from multiprocessing import Process, Queue
from multiprocessing.shared_memory import SharedMemory
from time import sleep
import numpy as np
import socket
import struct
import logging
import select
import sys
# import fabric

class StreamToLogger(object):
    """
    Fake file-like stream object that redirects writes to a logger instance.
    Taken from https://stackoverflow.com/questions/19425736/how-to-redirect-stdout-and-stderr-to-logger-in-python
    to catch lost output from separate process.
    """
    def __init__(self, logger, level):
       self.logger = logger
       self.level = level
       self.linebuf = ''

    def write(self, buf):
       for line in buf.rstrip().splitlines():
          self.logger.log(self.level, line.rstrip())

    def flush(self):
        pass

class dataThread:
    def __init__(self,
                 port=1001,
                 ip="192.168.1.100",
                 ):


        # Queues to pass data to and from main thread/GUI
        self.GUI_to_data_Queue = Queue()
        self.data_to_GUI_Queue = Queue()

        #State toggles and counters
        self.process = None
        self.process_isRun = False
        self.isRecord = False
        self.bytes_to_receive = 0
        self.trigger = False
        self.config_change = False
        self.record_request = False

        #FPGA config struct
        self.config = { "trigger":0,
                        "mode":0,
                        "CIC_divider":1250,
                        "param_a":5,
                        "param_b":0,
                        "param_c":0,
                        "param_d":0,
                        "param_e":0,
                        "param_f":0,
                        "param_g":0,
                        "param_h":0}

        #Socket config
        self.port = port
        self.ip = ip
        self.s = None


     # ************************ Process admin ****************************** #

    def start_Process(self):
        """Begin thread, toggle run state"""

        if self.process == None:
            self.process_isRun = True
            self.process = Process(target=self.backgroundThread)
            self.process.start()


     # **************** Local Inter-process Comms with GUI****************** #

    def fetch_instructions(self):
        """ Get and save instructions from GUI thread:

            config: New config struct for FPGA
            config_change: Config struct changed, request to send (could easily optimise out)
            record request: First request from GUI, initiates shared memory setup and handshake
            trigger: Shared memory set up, ready to receive & send to GUI.

        """

        try:
            self.trigger, self.config, self.config_change, [self.record_request, self.bytes_to_receive] = self.GUI_to_data_Queue.get(block=False)
            logging.debug("message received")
            return True
        except Exception as e:
            #logging.debug(str(e))
            return False

    def inform_GUI(self, event):
        """ Send message back to GUI thread, either:
            Allocated: Tell GUI new shared memory is allocated
            data_ready: Shared memory filled with requested data. """

        if event == "allocated":
            self.data_to_GUI_Queue.put([0, self.shared_memory_name], block=False)
            logging.debug(self.shared_memory_name + "sent to GUI")

        elif event == "data_ready":
            self.data_to_GUI_Queue.put([1, 0], block=False)

    def initiate_record(self):
        self.shared_mem = SharedMemory(size=self.bytes_to_receive, create=True)
        self.shared_memory_name = self.shared_mem.name

        #Tell GUI the memory is allocated
        self.inform_GUI("allocated")
        logging.debug("inform GUI executed")
     # ************************ TCP Comms with RP MCU  ********************* #

    def send_settings_to_FPGA(self):
        """Package FPGA_config attribute and send to server"""

        # Get config and package into c-readable struct
        format_ = "HHHiiiiiiii"

        config_send = struct.pack(format_,
                                  self.config["trigger"],
                                  self.config["mode"],
                                  self.config["CIC_divider"],
                                  self.config["param_a"],
                                  self.config["param_b"],
                                  self.config["param_c"],
                                  self.config["param_d"],
                                  self.config["param_e"],
                                  self.config["param_f"],
                                  self.config["param_g"],
                                  self.config["param_h"]
                                  )

        self.open_socket()
        if (self.initiate_transfer("config") < 1):
            logging.debug("Socket type (config) not acknowledged by server")
        else:
            try:
                self.s.sendall(config_send)
            except Exception as e:
                logging.debug("config send error")
                logging.debug(e)

        self.close_socket()
        logging.debug("FPGA settings sent")



    def record(self):
        self.open_socket()
        if (self.initiate_transfer("recording") < 1):
            logging.debug("Socket type (record) not acknowledged by server")

        #Create view of shared memory buffer

        view = memoryview(self.shared_mem.buf)
        logging.debug("memory view created")
        logging.debug("{} to receive".format(self.bytes_to_receive))

        # wait for trigger confirmation from server - process may get stuck in
        # this loop if trigger acknowledgement is lost
        if (self.wait_for_ack() != 1):
            logging.debug("Record acknowledge not received")
            return

        logging.debug("start receive")

        while (self.bytes_to_receive):
            #Load info into array in nbyte chunks
            nbytes = self.s.recv_into(view, self.bytes_to_receive)
            view = view[nbytes:]
            self.bytes_to_receive -= nbytes
            logging.debug(self.bytes_to_receive)

        self.purge_socket()
        self.close_socket()

        del view
        self.inform_GUI("data_ready")
        self.shared_mem.close()
        del self.shared_mem

    def wait_for_ack(self, ack_value=1):
        """ Wait for an acknowledge byte from MCU. If no byte or incorrect value
        received, log error and return -1. Returns 1 on ack. """
        ack = int.from_bytes(self.s.recv(4), "little", signed=False)
        logging.debug("Ack value received: {}, expected {}".format(ack, ack_value))
        
        if ack == ack_value:
            return 1
        else:
            logging.debug("Bad acknowledge")
            return -1

    def purge_socket(self):
        """Purge receive buffer if server is streaming naiively and likely to overshoot.
        . Ensure you have all the data you need first. Should only be required after record"""
        readers = [1]
        self.s.setblocking(False)
        sockets = [self.s]
            
        while readers != []:
            readers, writers, err = select.select(sockets,
                                                  sockets,
                                                  sockets,
                                                  2)
            try:
                purged = self.s.recv(16384)
                print("{} bytes received in purge, header = {} {} {} {}".format(len(purged), purged[0], purged[1], purged[3], purged[4]))
            except Exception as e:
                logging.debug("Purge recv error: {}".format(e))
                break

    def close_socket(self):
        """Close socket and wait for a 100ms. """
        # Close socket
        self.s.close()
        sleep(0.1)

    def open_socket(self):
        """Open generic client socket."""
        self.s = socket.socket(socket.AF_INET, socket.SOCK_STREAM, 0)
        try:
            self.s.connect((self.ip, self.port))
        except Exception as e:
            logging.debug(e)

    def initiate_transfer(self, type='config'):
        """ Send request info to MCU, await acknowledgement. Sends 4-byte 0 to initiate
        config packet send, expect ack value of 2. Sends "bytes to receive" value otherwise,
        expects ack value of bytes_to_receive. """
        if type == 'config':
            payload = np.uint32(0)
            ack_value = 2
        if type == 'recording':
            payload = np.uint32(self.bytes_to_receive)
            ack_value = np.uint32(self.bytes_to_receive)

        self.s.sendall(payload)
        if (self.wait_for_ack(ack_value) == 1):
            return 1
        else:
            return -1

    # ********************** MAIN LOOP ************************************* #


    def backgroundThread(self):    # retrieve data
        """
            """

        #Set up socketlog.log debug log
        logging.basicConfig(filename='socketlog.log',
                            level=logging.DEBUG,
                            format='%(asctime)s %(message)s',
                            datefmt='%m/%d/%Y %I:%M:%S %p')

        logging.debug('Logfile initialised')
        log = logging.getLogger('socket')
        sys.stdout = StreamToLogger(log, logging.DEBUG)
        sys.stderr = StreamToLogger(log, logging.DEBUG)


        while(True):
             # Check for instructions and dispatch accordingly
            if self.fetch_instructions():

                if self.config_change:
                    self.send_settings_to_FPGA()
                    self.config_change = 0

                elif self.record_request:
                    logging.debug("request received")
                    self.initiate_record()
                    self.record_request = False

                elif self.trigger:
                    #Trigger FPGA, start recording
                    self.config['trigger'] = 1
                    self.send_settings_to_FPGA()
                    logging.debug("{} to receive".format(self.bytes_to_receive))

                    self.record()

                    self.config['trigger'] = 0
                    self.send_settings_to_FPGA()
                    logging.debug("Trigger off sent")

                else:
                    sleep(0.1)





    def close(self):
        """End process and close socket when GUI is closed"""

        self.isrun = False
        logging.debug("Close called")
        logging.debug("{} s".format(self.s))

        if self.process_isRun:
            self.process.terminate()
            self.process_isRun = False

        if self.s != None:
            self.s.close()
            logging.debug("socket closed")

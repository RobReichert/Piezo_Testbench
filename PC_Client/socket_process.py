from multiprocessing import Process, Queue
from multiprocessing.shared_memory import SharedMemory
from time import sleep
import numpy as np
import socket
import struct
import logging
import select
import sys

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

        #FPGA config struct
        self.__config = { "load_mode":0,
                          "param_L1":0,
                          "param_L2":0,
                          "param_L3":0,
                          "param_L4":0,
                          "sample_mode":0,
                          "param_S1":0,
                          "param_S2":0,
                          "param_S3":0,
                          "param_S4":0,
                          "temp_mode":0,
                          "param_T1":0,
                          "param_T2":0,
                          "param_samp1":0,
                          "param_samp2":0,
                          "LI_amp_mode": 0,
                          "DDS_phase": 42,
                          "measure":0}

        #internal variables
        self.__process = None
        self.__bytes_to_receive = 0
        
        #Socket config
        self.__port = port
        self.__ip = ip
        self.__s = None #Socket instance
        self.__start_Process()

    def __del__(self):
        """End process and close socket when class been deleted"""
        logging.debug("Close called")
        try:
            self.__process.terminate()
        except:
            pass
        if self.__s != None:
            self.__s.close()
            logging.debug("socket closed")

        #close logging
        logging.shutdown()
     
     # ************************ Process admin ****************************** #

    def __start_Process(self):
        """Begin thread, toggle run state"""
        logging.debug('fetch_instrsssuction')
        if self.__process == None:
            #self.process_isRun = True
            self.__process = Process(target=self.backgroundThread)
            self.__process.start()
            
    
            
    # ************************ Handel Socket ****************************** #

    def __open_socket(self):
        """Open generic client socket."""
        self.__s = socket.socket(socket.AF_INET, socket.SOCK_STREAM, 0)
        try:
            self.__s.connect((self.__ip, self.__port))
        except Exception as e:
            logging.debug(e)
            
    def __close_socket(self):
        """Close socket and wait for a 100ms. """
        # Close socket
        self.__s.close()
        sleep(0.1)
            
    def __purge_socket(self):
        """Purge receive buffer if server is streaming naiively and likely to overshoot.
        . Ensure you have all the data you need first. Should only be required after record"""
        readers = [1]
        self.__s.setblocking(False)
        sockets = [self.__s]
            
        while readers != []:
            readers, writers, err = select.select(sockets,
                                                  sockets,
                                                  sockets,
                                                  2)
            try:
                purged = self.__s.recv(16384)
                logging.debug("{} bytes received in purge, header = {} {} {} {}".format(len(purged), purged[0], purged[1], purged[3], purged[4]))
            except Exception as e:
                logging.debug("Purge recv error: {}".format(e))
                break


     # **************** Communication ****************** #

    def __fetch_instructions(self):
        """ Get and save instructions from GUI thread:
            config: New config struct for FPGA
            bytes_to_receive: if 0 send config else recive x bytes from FPGA
        """

        try:
            self.__config, self.__bytes_to_receive = self.GUI_to_data_Queue.get(block=False)
            logging.debug('fetch_instruction')
            return True
        except:
            return False

    def __send_settings_to_FPGA(self):
        """Package FPGA_config attribute and send to server"""

        # Get config and package into c-readable struct see https://docs.python.org/3/library/struct.html#format-characters
        format_ = "i"

        config_send = struct.pack(format_,
                                    self.__config["LI_amp_mode"]
                                    )

        self.__open_socket()
        try:
            self.__s.sendall(np.uint32(0)) #send config request
            ack = int.from_bytes(self.__s.recv(4), "little", signed=False) #wait for acknowledge byte from MCU
            logging.debug("Ack value received: {}".format(ack))
            if ack == 1:
                self.__s.sendall(config_send)
            else:
                logging.debug("Socket type (config) not acknowledged by server")
        except Exception as e:
            logging.debug("config send error")
            logging.debug(e)

        self.__close_socket()
        logging.debug("FPGA settings sent")
        
    def __initiate_record(self):
        self.__shared_mem = SharedMemory(size=self.__bytes_to_receive, create=True)
        #Tell GUI the memory is allocated
        self.data_to_GUI_Queue.put([0, self.__shared_mem.name], block=False)
        logging.debug(self.__shared_mem.name + " sent to GUI")
        
    def __record(self):
        self.__open_socket()
        try:
            self.__s.sendall(np.uint32(self.__bytes_to_receive)) #send config request
            ack = int.from_bytes(self.__s.recv(4), "little", signed=False) #wait for acknowledge byte from MCU
            logging.debug("Ack value received: {}".format(ack))
            if ack == self.__bytes_to_receive:
                #Create view of shared memory buffer
                view = memoryview(self.__shared_mem.buf)
                logging.debug("memory view created")
                
                while (self.__bytes_to_receive):
                    #Load info into array in nbyte chunks
                    nbytes = self.__s.recv_into(view, self.__bytes_to_receive)
                    view = view[nbytes:]
                    self.__bytes_to_receive -= nbytes
                    logging.debug("{} Bytes received {} left".format(nbytes, self.__bytes_to_receive))
                    
                del view
                self.__shared_mem.close()
                del self.__shared_mem
                
                #Tell GUI the data is ready
                self.data_to_GUI_Queue.put([1, 0], block=False)
                logging.debug("data ready")
                
            else:
                logging.debug("Socket type (record) not acknowledged by server")
        except Exception as e:
            logging.debug("config send error")
            logging.debug(e)

        self.__purge_socket()
        self.__close_socket()            
            
        
    # ********************** MAIN LOOP ************************************* #


    def backgroundThread(self):    # retrieve data

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
            if self.__fetch_instructions():
                if self.__bytes_to_receive == 0:
                    logging.debug('config send')
                    self.__send_settings_to_FPGA()
                else:
                    logging.debug("{} bytes to receive".format(self.__bytes_to_receive))
                    self.__initiate_record()
                    self.__record()
            else:
                sleep(0.01)
                    
                    

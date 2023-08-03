import sys
import numpy as np
import os
import logging
from matplotlib.backends.backend_qtagg import (NavigationToolbar2QT as NavigationToolbar)
from PyQt5.QtCore import QTimer
from multiprocessing.shared_memory import SharedMemory


##Include clases from other files
from UI_unformated import Ui_MainWindow, QtWidgets
import socket_process as sp
from Canvas import MyFigureCanvas
from float_converter import NumpyFloatToFixConverter


    
class Window(QtWidgets.QMainWindow):
    def __init__(self,parent=None):
        # Set up debug log
        logging.basicConfig(filename='GUIlog.log',
                            level=logging.DEBUG,
                            format='%(asctime)s %(message)s',
                            datefmt='%m/%d/%Y %I:%M:%S %p')
        logging.debug('Logfile initialised')
        
        # setup ui
        QtWidgets.QWidget.__init__(self,parent=None)
        self.ui = Ui_MainWindow()
        self.ui.setupUi(self)
        
        # connect button press function
        self.ui.buttonSend.clicked.connect(self.ButtonPressSend)
        self.ui.buttonMeasurement.clicked.connect(self.ButtonPressMeasure)
        self.ui.buttonReload.clicked.connect(self.ButtonPressReload)
        self.ui.AmpPhase.released.connect(self.RadioButtonMode)
        self.ui.RC.released.connect(self.RadioButtonMode)
        self.ui.inputLoad.editingFinished.connect(self.LoadMode)
        self.ui.inputSample.editingFinished.connect(self.SampleMode)
        self.ui.inputTemp.editingFinished.connect(self.TempMode)
        
        # Create data processing thread
        self.data = sp.dataThread()
        
        # insert matplotlib graph
        self.layout = QtWidgets.QVBoxLayout(self.ui.MplWidget)
        self.canvas = MyFigureCanvas()
        self.layout.addWidget(self.canvas)
        self.layout.addWidget(NavigationToolbar(self.canvas,self.ui.MplWidget))
        
        # Calibration data
        self.FPGA_fclk = 125000000
        
        
        # init variables
        self.measurement = 0
        self.FPGA_config = {#"trigger": 0,
                            #"mode": 0,
                            #"CIC_divider":int(np.floor(self.FPGA_fclk / 100000)),#int(self.ui.inputData1.text()))),
                            "load_mode":int(self.ui.inputLoad.text()),
                            "param_L1":int(self.ui.inputL1.text()),
                            "param_L2":int(self.ui.inputL2.text()),
                            "param_L3":int(self.ui.inputL3.text()),
                            "param_L4":int(self.ui.inputL4.text()),
                            "sample_mode":int(self.ui.inputSample.text()),
                            "param_S1":int(self.ui.inputS1.text()),
                            "param_S2":int(self.ui.inputS2.text()),
                            "param_S3":int(self.ui.inputS3.text()),
                            "param_S4":int(self.ui.inputS4.text()),
                            "temp_mode":int(self.ui.inputTemp.text()),
                            "param_T1":int(self.ui.inputT1.text()),
                            "param_T2":int(self.ui.inputT2.text()),
                            "param_samp1":int(self.ui.inputSample1.text()),
                            "param_samp2":int(self.ui.inputSample2.text()),
                            "LI_amp_mode": 0,
                            "DDS_phase": 42,
                            "measure":0}
        
        # init float to fix conversion
        self.FloatToFix = NumpyFloatToFixConverter(signed=True, n_bits=32, n_frac=16)
        
        #init monitor timer
        self.timer = QTimer()
        self.timer.timeout.connect(lambda: self.monitor())

    def closeEvent(self, event):
        #stop running measurement
        self.FPGA_config["measure"] = 0
        self.ButtonPressSend()
        
        # Close shared memory
        try:
            self.shared_mem.close()
            self.shared_mem.unlink()
        except:
            pass
        # stop monitoring
        try:
            self.timer.stop()
        except:
            pass
        # Close data processing thread
        del self.data
        
        # close logging
        logging.shutdown()   
        
        
    def monitor(self): #data recording monitoring --> check for new data
        try:
            data_ready, memory_name = self.data.data_to_GUI_Queue.get(block=False)
            logging.debug ("Ready: {}, Memory name: {}".format(data_ready, memory_name))
            if memory_name:
                #intit shared memory
                self.shared_mem = SharedMemory(name=memory_name, size=self.num_bytes, create=False)
            elif data_ready:
                self.MeasureFinished()
        except:
            pass
        
    def MeasureFinished(self):
        # Stop data recording monitoring
        self.timer.stop()
        # Change Button mode
        self.measurement = 0
        self.ui.buttonMeasurement.setText("Start Measurement") # change button text
        #create array with view of shared mem
        logging.debug("data_ready recognised")
        #temp = np.ndarray((self.num_samples), dtype=np.dtype([('in', np.int16), ('out', np.int16)]), buffer=self.shared_mem.buf)
        temp = np.ndarray((self.num_samples), dtype=np.dtype([('in1', np.int32), ('in2', np.int16), ('in3', np.int16), ('in4', np.int16), ('in5', np.int16), ('in6', np.int16), ('in7', np.int16)]), buffer=self.shared_mem.buf)
        print(temp)
        #copy into permanent array
        self.recording = np.copy(temp)
        
        logging.debug("recording copied")
        # Delete view of shared memory (important, otherwise memory still exists)
        del temp
        
        # Store to *.csv
        if self.ui.checkBoxStore.isChecked():
            #Set up data directory
            datadir="./Data/"
            if (os.path.isdir(datadir) != True):
                os.mkdir(datadir)
            label = self.ui.inputFileName.text()
            i = 0
            while os.path.exists(datadir + '{}{}.csv'.format(label, i)):
                i += 1
            np.savetxt(datadir + '{}{}.csv'.format(label, i), 
                        np.transpose([self.recording['in1'], self.recording['in2']]), 
                        delimiter=";", fmt='%d',
                        header="Sampling Frequenzy [Hz]: {}".format(int(self.ui.inputSample2.text())))
        
        # Close shared memory
        self.shared_mem.close()
        self.shared_mem.unlink()
        
        
## Define button functions
    def ButtonPressSend(self):
        self.num_bytes = 0
        packet = [self.FPGA_config, self.num_bytes]
        try:
            self.data.GUI_to_data_Queue.put(packet, block=False)
            logging.debug("packet sent to socket process")
        except:
            logging.debug("Didn't send config to data process")
            
        
    def ButtonPressMeasure(self):
        if self.measurement==0: 
            print('measure')
            #Send config before measure
            self.FPGA_config["measure"] = 1
            self.ButtonPressSend()
            
            self.num_samples = int(self.ui.inputSample1.text())
            self.num_bytes = self.num_samples * 16
            # Send record request to server
            packet = [self.FPGA_config, self.num_bytes]
            logging.debug("{} samples requested".format(self.num_samples))
            try:
                self.data.GUI_to_data_Queue.put(packet, block=False)
                logging.debug("packet sent to socket process")
                #Switch button mode
                self.measurement = 1
                self.ui.buttonMeasurement.setText("Stop Measurement") # change button text
                # Start data recording monitoring
                self.timer.start(250)
            except:
                logging.debug("Didn't start measurement process")
                pass
        else:
            self.measurement = 0
            self.ui.buttonMeasurement.setText("Start Measurement") # change button text
            #stop running measurement
            self.FPGA_config["measure"] = 0
            self.ButtonPressSend() #send stop to FPGA
            # Stop data recording monitoring
            self.timer.stop()
            # Close shared memory
            try:
                self.shared_mem.close()
                self.shared_mem.unlink()
            except:
                pass
            
    def ButtonPressReload(self):
        print('ButtonPressReload')
        """# Update Canvas
        self.scale=[float(self.ui.inputScal0.text()),float(self.ui.inputScal1.text()),float(self.ui.inputScal2.text()),float(self.ui.inputScal3.text())]
        self.offset=[float(self.ui.inputOffset0.text()),float(self.ui.inputOffset1.text()),float(self.ui.inputOffset2.text()),float(self.ui.inputOffset3.text())]
        self.canvas.update_canvas([recording['in'],recording['out']],self.scale,self.offset)
        
        
        """
        
    def RadioButtonMode(self):
        if self.ui.AmpPhase.isChecked():
            logging.debug('toggel to AmpPhase')
            self.FPGA_config["LI_amp_mode"] = 0
            
        if self.ui.RC.isChecked():
            logging.debug('toggel to RC')
            self.FPGA_config["LI_amp_mode"] = 1
            
    def LoadMode(self):
        print('Load mode')
        
    def SampleMode(self):
        print('sample mode')
    
    def TempMode(self):
        print('temp mode')
            

## Main Loop

if __name__ == "__main__":
    #initialize variable
    
    #Open QT Window and import as ui
    app=QtWidgets.QApplication(sys.argv)
    self=Window()
    self.showMaximized() #setGeometry(300, 300, 800, 600) #X co-ordinate, Y co-ordinate, Width, Height
    self.show()
    
    #crate client
    #serverClient=Client()
    
    
    
    
    
    













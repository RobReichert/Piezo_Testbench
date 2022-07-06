import numpy as np
from matplotlib.backends.backend_qtagg import (FigureCanvas)
import matplotlib.figure as mpl_fig

## This is the FigureCanvas in which the plot is drawn. (according https://stackoverflow.com/questions/57891219/how-to-make-a-fast-matplotlib-live-plot-in-a-pyqt5-gui)
class MyFigureCanvas(FigureCanvas):
    def __init__(self) -> None:
        FigureCanvas.__init__(self, mpl_fig.Figure())
        
        # Store a figure and ax
        self.ax  = self.figure.subplots(2,1)
        self.line0, = self.ax[0].plot([], [],label='IN1')
        #self.line1, = self.ax[0].plot([], [],label='IN2')
        self.line2, = self.ax[1].plot([], [],label='OUT1')
        #self.line3, = self.ax[1].plot([], [],label='OUT2')
        
        # Set polt options
        self.ax[0].set_title("In Chart")
        self.ax[0].grid(True)  
        self.ax[0].legend(loc='upper right')
        self.ax[0].set_ylabel("ADC Counts")
        self.ax[0].set_xlabel("Sample Count")
        self.ax[1].set_title("Out Chart")
        self.ax[1].grid(True) 
        self.ax[1].legend(loc='upper right')
        self.ax[1].set_ylabel("DAC Counts")
        self.ax[1].set_xlabel("Sample Count")        
        
        # Set figure options
        self.figure.tight_layout()
        
    def update_canvas(self, data, scale, offset):
        
        # # 2 Line mode
        scaled_data=[data[0]*scale[0]+offset[0],data[1]*scale[2]+offset[2]]
        # ax 0
        self.line0.set_data(range(len(scaled_data[0])), scaled_data[0])
        self.ax[0].set_xlim((0,len(scaled_data[0])))
        self.ax[0].set_ylim((np.min(scaled_data[0]), np.max(scaled_data[0])))
        # ax 1
        self.line2.set_data(range(len(scaled_data[1])), scaled_data[1])
        self.ax[1].set_xlim((0,len(scaled_data[1])))
        self.ax[1].set_ylim((np.min(scaled_data[1]), np.max(scaled_data[1])))
        
        # 4 Line mode
        # scaled_data=[data[0]*scale[0]+offset[0],data[1]*scale[1]+offset[1],data[2]*scale[2]+offset[2],data[3]*scale[3]+offset[3]]
        # # ax 0
        # self.line0.set_data(range(len(scaled_data[0])), scaled_data[0])
        # self.line1.set_data(range(len(scaled_data[1])), scaled_data[1])
        # self.ax[0].set_xlim((0,np.max([len(scaled_data[0]),len(scaled_data[1])])))
        # self.ax[0].set_ylim((np.min([scaled_data[0],scaled_data[1]]), np.max([scaled_data[0],scaled_data[1]])))
        # # ax 1
        # self.line2.set_data(range(len(scaled_data[2])), scaled_data[2])
        # self.line3.set_data(range(len(scaled_data[3])), scaled_data[3])
        # self.ax[1].set_xlim((0,np.max([len(scaled_data[2]),len(scaled_data[3])])))
        # self.ax[1].set_ylim((np.min([scaled_data[2],scaled_data[3]]), np.max([scaled_data[2],scaled_data[3]])))
        
        # redraw canvas
        self.draw()
        
import numpy as np
from matplotlib.backends.backend_qtagg import (FigureCanvas)
import matplotlib.figure as mpl_fig

from matplotlib.backends.backend_qt5agg import FigureCanvasQTAgg
from matplotlib.figure import Figure

def make_patch_spines_invisible(ax):
    ax.set_frame_on(True)
    ax.yaxis.set_label_position('left')
    ax.yaxis.set_ticks_position('left')

## This is the FigureCanvas in which the plot is drawn. (according https://stackoverflow.com/questions/57891219/how-to-make-a-fast-matplotlib-live-plot-in-a-pyqt5-gui)
class MyFigureCanvas(FigureCanvas):      
    def __init__(self) -> None:
        FigureCanvas.__init__(self, mpl_fig.Figure())
        
    def change_canvas(self, activated, names):
        # Delate all axis 
        for index, _ in enumerate(activated):
            ax_name = f"ax_{index}"
            if hasattr(self, ax_name):  # if axis exist
                getattr(self, ax_name).remove()
                delattr(self, ax_name)  
        
        num_activated = 0
        spacing = 0.2  # spacing between axis
        
        first_activ = True
        
        colors = ['red', 'green', 'blue', 'purple', 'orange', 'magenta']  # axis color
        
        all_lines = []  # Array to collect all lines from different axes
        all_labels = []  # Array to collect all line labels
        
        for index, activ in enumerate(activated):
            ax_name = f"ax_{index}" # set variable for axis depending on index
            line_name = f"line_{index}" # set variable for line depending on index
            color = colors[index]  # take color according index
            name = names[index] # take name according index
            
            if activ: #check if axis is activated
                relative_position = -spacing * num_activated
                num_activated = num_activated + 1
                    
                if first_activ: 
                    setattr(self, ax_name, self.figure.subplots()) #set first axis
                    self.ax_first = getattr(self, ax_name) #store name of first axis
                    first_activ = False # set trigger for first axis false
                    getattr(self, ax_name).set_xlabel("Sample Count") # set labele of x axis
                    getattr(self, ax_name).grid(True)  # turn on grid
                    getattr(self, ax_name).set_title("Measurement Values") # name the chart
                else:
                    setattr(self, ax_name, self.ax_first.twinx()) #set further axis
                    getattr(self, ax_name).spines["left"].set_position(("axes", relative_position)) #set position of further axis depending on the number of already activated ones 
                    make_patch_spines_invisible(getattr(self, ax_name)) # configure the axis to the same look like the first one
                
                getattr(self, ax_name).spines["left"].set_color(color)  # set color of axis
                #getattr(self, ax_name).set_ylabel(name) # put axis lable to the axis
                setattr(self, line_name, getattr(self, ax_name).plot([], [], label=name, color=color)) #initialise the lines
                all_lines.append(getattr(self, line_name)[0])  # Append the line to the list
                all_labels.append(name)  # Append the label to the list
            
        if num_activated>0: #check if at least one axis is activated
            self.ax_first.legend(all_lines, all_labels, loc='upper right') # put a legend in the upper right corner
            #fit the diagam to window (why ever the needed space is nonlinear --> if statement)
            if num_activated<=4:
                self.figure.subplots_adjust(left=0.12 + (0.1*(num_activated-1)), right=0.93, top=0.93, bottom=0.07)
            else:
               self.figure.subplots_adjust(left=0.23+(0.056*(num_activated-1)), right=0.93, top=0.93, bottom=0.07) 
            
            self.draw() # Redraw the canvas
        
        
    def update_canvas(self, data, activated):
        #plot all activated lines
        for index, activ in enumerate(activated): 
            if activ:
                line_name = f"line_{index}" # set variable for line depending on index
                getattr(self, line_name)[0].set_data(range(len(data[index])), data[index])  # Update line data
                ax = getattr(self, line_name)[0].axes # get axis that belong to line
                ax.set_xlim((0, len(data[index])))  # Update x-axis limits
                ax.set_ylim(np.min(data[index]), np.max(data[index]))  # Update y-axis limits
    
        self.draw() # Redraw the canvas


# This canvas is for the temperature and heat flow curve        
class MyFigure_thermal(FigureCanvasQTAgg):
   def __init__(self,width=5,height=4,dpi = 100):
      # 1. Create a figure object to draw
      self.fig = Figure(figsize=(width,height),dpi=dpi) 
      # 2. Activate the figure window and inherit the parent class properties
      super(MyFigure_thermal, self).__init__(self.fig)
        
import tkinter as tk

row_max = 4
column_max = 10

class UIWindow:
    def __init__(self):
        self.current_window = "display_measurement"
        self.selected_window = ""
        # self.measurement_mode = "Impedance"
        self.measurement_mode = "RLs"

        self.display = tk.Tk()
        self.display.geometry("480x320")
        
        self.display.grid_columnconfigure(column_max, minsize=40)
        for col in range(column_max):
            self.display.grid_columnconfigure(col, minsize=40)

        for row in range(row_max):
            self.display.grid_rowconfigure(row, minsize=40)

        self.display_measurement()
        tk.mainloop()

    def set_measurement_mode(self, mode: str):
        self.measurement_mode=mode

    def display_measurement(self):
        
        display_measurement_tab_label = tk.Label(self.display, text="Display\nMeasurment")
        display_measurement_tab_label.grid(row = 0, column = column_max)

        display_measurement_tab_label = tk.Label(self.display, text="Measurement\nSetup")
        display_measurement_tab_label.grid(row = 1, column = column_max)
        
        display_measurement_tab_label = tk.Label(self.display, text="Waveform\nSetup")
        display_measurement_tab_label.grid(row = 2, column = column_max)

        if self.measurement_mode == "Impedance":
            measurement_label = tk.Label(self.display, text="Measured Impedance:", font=('Arial', 24))
            measurement_label.grid(row=0, column=0, columnspan=column_max-1, rowspan=1)

            impedance_reading_label = tk.Label(self.display, text=u"1.0000 + j9.0000 \u03A9", font=('Arial', 24))
            impedance_reading_label.grid(row=1, column=0, columnspan=column_max-1, rowspan=row_max-1)

        if self.measurement_mode == "RCs":
            measurement_label = tk.Label(self.display, text="Measured Rs, Cs:", font=('Arial',24))
            measurement_label.grid(row=0, column=0, columnspan=column_max-1, rowspan=1)

            rs_label = tk.Label(self.display, text= "Rs:", font=('Arial', 18))
            rs_label.grid(row=1, column=0, columnspan=1, rowspan=1)

            rs_reading_label = tk.Label(self.display, text=u"\u03A9", font=('Arial', 18))
            rs_reading_label.grid(row=1, column=1, columnspan=column_max-2, rowspan=1)


            cs_label = tk.Label(self.display, text= "Cs:", font=('Arial', 18))
            cs_label.grid(row=2, column=0, columnspan=1, rowspan=1)
            
            cs_reading_label = tk.Label(self.display, text= "F", font=('Arial', 18))
            cs_reading_label.grid(row=2, column=1, columnspan=column_max-2, rowspan=1)

        if self.measurement_mode == "RLs":
            measurement_label = tk.Label(self.display, text="Measured Rs, Ls:", font=('Arial',24))
            measurement_label.grid(row=0, column=0, columnspan=column_max-1, rowspan=1)

            rs_label = tk.Label(self.display, text= "Rs:", font=('Arial', 18))
            rs_label.grid(row=1, column=0, columnspan=1, rowspan=1)

            rs_reading_label = tk.Label(self.display, text=u"\u03A9", font=('Arial', 18))
            rs_reading_label.grid(row=1, column=1, columnspan=column_max-2, rowspan=1)


            cs_label = tk.Label(self.display, text= "Ls:", font=('Arial', 18))
            cs_label.grid(row=2, column=0, columnspan=1, rowspan=1)
            
            cs_reading_label = tk.Label(self.display, text= "H", font=('Arial', 18))
            cs_reading_label.grid(row=2, column=1, columnspan=column_max-2, rowspan=1)


        

    def setup_measurement(self):
        display_measurement_tab_label = tk.Label(self.display, text="Display\nMeasurment")
        display_measurement_tab_label.grid(row = 0, column = column_max)

        display_measurement_tab_label = tk.Label(self.display, text="Measurement\nSetup")
        display_measurement_tab_label.grid(row = 1, column = column_max)
        
        display_measurement_tab_label = tk.Label(self.display, text="Waveform\nSetup")
        display_measurement_tab_label.grid(row = 2, column = column_max)

    
def main():
    window = UIWindow()
    



if __name__ == '__main__':
    main()
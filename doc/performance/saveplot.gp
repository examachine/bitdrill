# File name: saveplot - saves a plot as a Post-Script file
set size 1.0, 0.5
set terminal postscript portrait enhanced "Helvetica" 14
replot
set size 1,1                  # return to normal size
set terminal x11
replot

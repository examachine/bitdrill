#-*-gnuplot-*-
set size 1.0, 0.8
set terminal postscript portrait enhanced "Helvetica" 16
set out "parallel/dissection.ps"

set ylabel "Parallel Runtime (%)"
set key invert reverse Left outside
set key autotitle columnheader 
set key box lw 0.25
set key over
set yrange [0:100]
set auto x
set style data histogram
set style histogram rowstacked
set style fill pattern 2 border
set boxwidth 0.75
set xtics rotate by -45

plot '../../perf/parallel/all.dissection' using 2:xtic(1) title 2, \
'' using 3, \
'' using 4

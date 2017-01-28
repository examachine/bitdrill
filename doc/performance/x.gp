# -*- gnuplot -*-

set size 1.0, 0.63
set terminal postscript portrait enhanced "Helvetica" 20
set out "parallel/all.imbl.ps"

set title ""
set xlabel "Processors"
set ylabel "Imbalance (%)"
set datafile missing '-'

set key box lw 0.25
#set key samplelen 3
set key spacing 1

if ("below" eq "nokey") print 'bok' ; set nokey ; else  set key  below

#set ylabel

set style data linespoints

#BUG:the following doesn't work!
set style data histogram
set style histogram cluster gap 1

if ("" ne "") set yrange [:]


if (4==1)  plot '/Users/malfunct/Code/projects/noclique_code/perf/parallel/all.imbl' using 1:2:xtic(1) ti col

if (4==2)  plot '/Users/malfunct/Code/projects/noclique_code/perf/parallel/all.imbl' using 1:2:xtic(1) ti col, \
   '' u 1:3:xtic(1) ti col

if (4==3)  plot '/Users/malfunct/Code/projects/noclique_code/perf/parallel/all.imbl' using 1:2:xtic(1) title col, \
   '' u 1:3:xtic(1) ti col, '' u 1:4:xtic(1) ti col

if (4==4)  plot '/Users/malfunct/Code/projects/noclique_code/perf/parallel/all.imbl' using 1:2:xtic(1) title col, \
   '' u 1:3:xtic(1) ti col, '' u 1:4:xtic(1) ti col, '' u 1:5:xtic(1) ti col

if (4==5)  plot '/Users/malfunct/Code/projects/noclique_code/perf/parallel/all.imbl' using 1:2:xtic(1) title col, \
   '' u 1:3:xtic(1) ti col, '' u 1:4:xtic(1) ti col, '' u 1:5:xtic(1) ti col, '' u 1:6:xtic(1) ti col

if (4==6)  plot '/Users/malfunct/Code/projects/noclique_code/perf/parallel/all.imbl' using 1:2:xtic(1) title col, \
   '' u 1:3:xtic(1) ti col, '' u 1:4:xtic(1) ti col, '' u 1:5:xtic(1) ti col, '' u 1:6:xtic(1) ti col, '' u 1:7:xtic(1) ti col

if (4==7)  plot '/Users/malfunct/Code/projects/noclique_code/perf/parallel/all.imbl' using 1:2:xtic(1) title col, \
'' u 1:3:xtic(1) ti col, '' u 1:4:xtic(1) ti col, '' u 1:5:xtic(1) ti  col, \
'' u 1:6:xtic(1) ti col, '' u 1:7:xtic(1) ti col, '' u 1:8:xtic(1) ti col

if (4==8)  plot '/Users/malfunct/Code/projects/noclique_code/perf/parallel/all.imbl' using 1:2:xtic(1) title col, \
'' u 1:3:xtic(1) ti col, '' u 1:4:xtic(1) ti col, '' u 1:5:xtic(1) ti  col, \
'' u 1:6:xtic(1) ti col, '' u 1:7:xtic(1) ti col, '' u 1:8:xtic(1) ti col, \
'' u 1:9:xtic(1) ti col

if (4>=9)  plot '/Users/malfunct/Code/projects/noclique_code/perf/parallel/all.imbl' using 1:2:xtic(1) title col, \
'' u 1:3:xtic(1) ti col, '' u 1:4:xtic(1) ti col, '' u 1:5:xtic(1) ti  col, \
'' u 1:6:xtic(1) ti col, '' u 1:7:xtic(1) ti col, '' u 1:8:xtic(1) ti col, \
'' u 1:9:xtic(1) ti col, '' u 1:10:xtic(1) ti col


#!/bin/sh

rm -rf parallel analysis serial
#python plot-par-time.py
#python plot-par-time-lcm.py
#python plot-par-time-aim.py
#python plot-par-time-fpgrowth.py
#python plot-imbalance-all.py 
#python plot-par-dissection.py
python plot-serial-time.py
#python plot-serial-time.py ../data/datafiles-scale
#python plot-par-time.py ../data/datafiles-scale
#python plot-scalability.py
#python plot-par-time.py ../data/datafiles-scale-old
#python plot-scalability2.py

python plot-quality.py $1
python plot-par-dissection2.py $1
python plot-par-time-all.py $1


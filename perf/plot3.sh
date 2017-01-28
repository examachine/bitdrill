#!/bin/sh

rm -rf parallel3 

python plot-par-time3.py --config=cypress benchmark 
python plot-quality3.py --config=cypress benchmark 
python plot-par-dissection3.py --config=cypress benchmark

python plot-par-time3.py --config=skynet benchmark2
python plot-quality3.py --config=skynet benchmark2 
python plot-par-dissection3.py --config=skynet benchmark2

#python plot-par-time-all.py $1


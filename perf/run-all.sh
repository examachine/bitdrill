#!/bin/sh

#python perf/serial-time2.py
#lamclean -v
python perf/par-time2.py
#lamclean -v
python perf/par-time2-pardci.py

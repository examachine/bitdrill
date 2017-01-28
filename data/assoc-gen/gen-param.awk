#! /usr/bin/awk -f
BEGIN { FS = "," } ; { print "echo " $2 " > " $1 ".exp"; }

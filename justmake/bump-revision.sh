#! /bin/bash
rev=`cvs status | grep revision | awk '{print $3;}' | sort -g | tail -1 | cut --fields=1 --delimiter=.`
expr $rev + 1

#! /usr/bin/gawk -f
BEGIN { FS = "," } ; { print $1 ".exp: " EXP ".exps-sh" ; print "	sh " EXP ".exps-sh"; }

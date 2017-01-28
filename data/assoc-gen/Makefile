# Makefile for gnu make
# assoc-gen benchmark suite
# Eray Ozkural aka exa

default: all

export PATH:=$(PATH):../../aux/ibm-datagen/ 

all:
	@echo "* Running make recursively to interpret experiment params"
	@$(MAKE) -f Makefile.interpret-exp
	@echo 
	@echo "* Running make recursively to generate data"
	@$(MAKE) -f Makefile.run-exp

clean: clean-links
	@$(MAKE) -f Makefile.interpret-exp clean
	@$(MAKE) -f Makefile.run-exp clean

realclean:
	@$(MAKE) -f Makefile.interpret-exp realclean
	@$(MAKE) -f Makefile.run-exp realclean

speedup:
	@echo "* Running make recursively to interpret experiment params"
	@$(MAKE) -f Makefile.interpret-exp
	@echo 
	@echo "* Running make recursively to measure speedup"
	@$(MAKE) -f Makefile.run-exp speedup

timing:
	@echo "* Running make recursively to interpret experiment params"
	@$(MAKE) -f Makefile.interpret-exp
	@echo 
	@echo "* Running make recursively to measure timing"
	@$(MAKE) -f Makefile.run-exp timing

link-exec:
	@echo Making symbolic links to executables for convenience
	@-ln -s `which gen` gen &>/dev/null
	@-ln -s `which init-pardb` init-pardb &>/dev/null
	@-ln -s `which part-db` part-db &>/dev/null
	@-ln -s `which parfreq` parfreq &>/dev/null
	@-ln -s `which freq` freq &>/dev/null
	@-ln -s `which nodebisect` nodebisect &>/dev/null

clean-links:
	rm -f gen init-pardb part-db parfreq freq nodebisect

.PHONY:	all clean link-exec clean-links

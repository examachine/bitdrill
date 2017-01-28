# Makefile to generate performance plots

ifndef top_srcdir
top_srcdir=../..
endif

perfdir=$(top_srcdir)/perf
perfdirs=$(notdir $(wildcard $(perfdir)/support-*))

all: plots

# run timing and speedup plots for each support dir

plots:
	@echo; echo "*** Plotting performance data"
	@echo "* Plotting serial performance"
	#$(MAKE) -f Makefile.plot-serial
	#@echo "* Plotting analysis"
	#$(MAKE) -f Makefile.plot-analysis
	@echo "* Plotting parallel performance"
	#$(MAKE) -f Makefile.plot-parallel

	$(MAKE) -f Makefile.plot-parallel3

clean:
	rm -rf *.ps support-* serial/* parallel/* benchmark2.skynet/*

test:
	@echo perfdirs = $(perfdirs)

.PHONY:	test

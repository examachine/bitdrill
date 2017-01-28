# Makefile for GNU make
# Eray Ozkural aka exa
# hacked a bit from edison's makefile

# dirs

subdirs		= .

builddir	= out

# files

progs = test-comms-model

# more dirs

VPATH		= $(subdirs) $(importdir) $(outdir)

fullsources     = $(foreach dir, $(subdirs), $(wildcard $(dir)/*.hs))
sources         = $(foreach file, $(fullsources), $(notdir $(file)))
objects         = $(foreach file, $(sources:.hs=.o), $(file))

# tools and options

ghc		= ghc
ghcflags	= -cpp -i$(importdir) -package data
# -O2 -fvia-C -O2-for-C 
default: all

$(progs): % : %.hs
	$(ghc) --make $(ghcflags) $< -o $@

$(progs): $(fullsources) # we use --make

all: checkdir $(progs)

checkdir:
	@-mkdir $(builddir)

clean:
	rm -f *.o *.hi $(progs)

.PHONY:	clean checkdir check


#CXXFLAGS =	-O0 -g -Wall --pedantic 
CXXFLAGS = -Wall --pedantic -O6 -finline-functions -DREENTRANT -fmessage-length=0

OBJS = findrules.o ifile.o utils.o

LIBS =

TARGET = findrules

$(TARGET): $(OBJS)
	$(CXX) -o $(TARGET) $(OBJS) $(LIBS)
	strip $(TARGET)

all:	$(TARGET)

clean:
	rm -f $(OBJS) $(TARGET).o

depend:
	makedepend $(CXXFLAGS) *.cc

pack:
	mkdir rules
	cp *.hh *.cc Makefile COPYING rules
	tar zcvf rules.tgz rules/
	rm -rf rules

# DO NOT DELETE

findrules.o: /usr/include/stdlib.h /usr/include/features.h
findrules.o: /usr/include/sys/cdefs.h /usr/include/bits/wordsize.h
findrules.o: /usr/include/gnu/stubs.h /usr/include/gnu/stubs-32.h
findrules.o: /usr/include/sys/types.h /usr/include/bits/types.h
findrules.o: /usr/include/bits/typesizes.h /usr/include/time.h
findrules.o: /usr/include/endian.h /usr/include/bits/endian.h
findrules.o: /usr/include/sys/select.h /usr/include/bits/select.h
findrules.o: /usr/include/bits/sigset.h /usr/include/bits/time.h
findrules.o: /usr/include/sys/sysmacros.h /usr/include/bits/pthreadtypes.h
findrules.o: /usr/include/alloca.h ifile.hh /usr/include/stdio.h
findrules.o: /usr/include/libio.h /usr/include/_G_config.h
findrules.o: /usr/include/wchar.h /usr/include/bits/stdio_lim.h
findrules.o: /usr/include/bits/sys_errlist.h
ifile.o: ifile.hh /usr/include/stdio.h /usr/include/features.h
ifile.o: /usr/include/sys/cdefs.h /usr/include/bits/wordsize.h
ifile.o: /usr/include/gnu/stubs.h /usr/include/gnu/stubs-32.h
ifile.o: /usr/include/bits/types.h /usr/include/bits/typesizes.h
ifile.o: /usr/include/libio.h /usr/include/_G_config.h /usr/include/wchar.h
ifile.o: /usr/include/bits/stdio_lim.h /usr/include/bits/sys_errlist.h
ifile.o: /usr/include/stdlib.h /usr/include/sys/types.h /usr/include/time.h
ifile.o: /usr/include/endian.h /usr/include/bits/endian.h
ifile.o: /usr/include/sys/select.h /usr/include/bits/select.h
ifile.o: /usr/include/bits/sigset.h /usr/include/bits/time.h
ifile.o: /usr/include/sys/sysmacros.h /usr/include/bits/pthreadtypes.h
ifile.o: /usr/include/alloca.h

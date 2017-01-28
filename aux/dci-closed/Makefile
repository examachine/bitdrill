CC  =g++
LFS_SUPP=
CPPFLAGS = -MP -MD
CFLAGS =`getconf LFS_CFLAGS` -Wall --pedantic -O6 -finline-functions -DREENTRANT -pthread $(CPPFLAGS)
#CFLAGS =`getconf LFS_CFLAGS` -Wall --pedantic -DREENTRANT -g
#CFLAGS = -Wall --pedantic -finline-functions -DREENTRANT -gdwarf-2 -g3 -O3
#CC = xlc++ -q32 -O3

HEADER  = database.hh first_scan.hh second_scan.hh items.hh   \
          my_defs.hh  tid_list_macros.hh  utils.hh writer.hh \
          memory.hh \
          stat.hh transaction.hh vertical.hh  \
          third_scan.hh	 outputunmap.hh \
          fourth_scan.hh print.hh 

OBJS	= database.o fim_all.o items.o memory.o utils.o \
          stat.o first_scan.o second_scan.o third_scan.o \
          outputunmap.o fourth_scan.o print.o 

LIBS = 

ADDFLAGS =

TARGET  = multi-closed

default: $(TARGET)

multi-closed: $(OBJS) $(HEADER) 
	$(CC) $(CFLAGS) -lpthread -o $(TARGET) $(OBJS) $(LIBS)
	strip $(TARGET)

all:
	make clean && make ssn
	make clean && make sdn
	make clean && make ddn

	make clean && make ssp
	make clean && make sdp
	make clean && make ddp
	make clean && make multi-closed

# ------------------------------------------
# various policies
# ssn
ssn: ADDFLAGS = -DPARTITIONING_POLICY=4
ssn: TARGET   = ssn
ssn: $(OBJS) $(HEADER) 
	$(CC) $(CFLAGS) -lpthread -o $(TARGET) $(OBJS) $(LIBS)
	strip $(TARGET)

sdn: ADDFLAGS = -DPARTITIONING_POLICY=3
sdn: TARGET   = sdn
sdn: $(OBJS) $(HEADER) 
	$(CC) $(CFLAGS) -lpthread -o $(TARGET) $(OBJS) $(LIBS)
	strip $(TARGET)

ddn: ADDFLAGS = -DPARTITIONING_POLICY=5
ddn: TARGET   = ddn
ddn: $(OBJS) $(HEADER) 
	$(CC) $(CFLAGS) -lpthread -o $(TARGET) $(OBJS) $(LIBS)
	strip $(TARGET)

ssp: ADDFLAGS = -DPARTITIONING_POLICY=6
ssp: TARGET   = ssp
ssp: $(OBJS) $(HEADER) 
	$(CC) $(CFLAGS) -lpthread -o $(TARGET) $(OBJS) $(LIBS)
	strip $(TARGET)

sdp: ADDFLAGS = -DPARTITIONING_POLICY=7
sdp: TARGET   = sdp
sdp: $(OBJS) $(HEADER) 
	$(CC) $(CFLAGS) -lpthread -o $(TARGET) $(OBJS) $(LIBS)
	strip $(TARGET)

ddp: ADDFLAGS = -DPARTITIONING_POLICY=8
ddp: TARGET   = ddp
ddp: $(OBJS) $(HEADER) 
	$(CC) $(CFLAGS) -lpthread -o $(TARGET) $(OBJS) $(LIBS)
	strip $(TARGET)

.SUFFIXES: .o .cc

.cc.o:
	$(CC) $(CFLAGS) $(ADDFLAGS) -c $<       

#depend:
#	makedepend $(CFLAGS) *.cc
clean:
	rm -f *~  $(OBJS) 
remove:
	rm -f *~  $(OBJS) $(TARGET) $(TARGET).exe ssn ssn.exe sdn sdn.exe ddn ddn.exe ssp ssp.exe sdp sdp.exe ddp ddp.exe

-include $(SRC:.cpp=.d)

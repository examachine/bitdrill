flags = -Wall --pedantic -O3 -finline-functions

all:: fim_closed

fim_closed: database.o fim_all.o items.o memory.o utils.o 
	g++ $(flags) database.o fim_all.o items.o memory.o utils.o -o fim_closed

database.o: database.cc database.hh transaction.hh items.hh
	g++ $(flags) -c database.cc -o database.o

fim_all.o: fim_all.cc items.hh transaction.hh database.hh candidates.hh frequents.hh utils.hh direct_count.hh vertical.hh closure.hh writer.hh tid_list_macros.hh
	g++ $(flags) -c fim_all.cc -o fim_all.o

items.o: items.cc items.hh direct_count.hh my_defs.hh
	g++ $(flags) -c items.cc -o items.o

memory.o: memory.cc memory.hh
	g++ $(flags) -c memory.cc -o memory.o

utils.o: utils.hh utils.cc database.hh
	g++ $(flags) -c utils.cc -o utils.o

clean:
	rm -f *.o fim_closed fim_closed.exe

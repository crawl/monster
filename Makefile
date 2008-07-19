
all:
	g++ -Icrawl-rel/latest/source/ -Wall -pedantic -DNDEBUG -DUNIX monster.cc -o monster
	strip -s monster

test: all
	./monster quasit

install: all
	cp monster $(HOME)/bin/

clean:
	rm monster


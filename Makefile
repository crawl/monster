CRAWL_PATH=crawl-rel/latest/source
all:
	g++ -I$(CRAWL_PATH) -Wall -pedantic -DNDEBUG -DUNIX monster.cc \
		 -o monster
	strip -s monster

test: all
	./monster quasit

install: all
	cp monster $(HOME)/bin/

clean:
	rm monster


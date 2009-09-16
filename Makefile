# Use 'make stable' to compile monster, 'make trunk' to compile monster-trunk.
# Use 'make install' to install monster, 'make install-trunk' to install
# monster-trunk.

# Branch names to target in git repo.
STABLE = dcss05
TRUNK = master

CRAWL_PATH=crawl-ref/crawl-ref/source

CXX = g++
LUASRC := $(CRAWL_PATH)/util/lua/src
LUALIB = lua
LUALIBA = lib$(LUALIB).a

SQLSRC := $(CRAWL_PATH)/util/sqlite
SQLLIB   := sqlite3
SQLLIBA  := lib$(SQLLIB).a
FSQLLIBA := $(SQLSRC)/$(SQLLIBA)

CFLAGS = -Wall -Wno-parentheses -pedantic -DNDEBUG -DUNIX \
	-I$(LUASRC) -I$(SQLSRC) -I$(CRAWL_PATH) -g

LFLAGS = -L$(SQLSRC)  -l$(SQLLIB) -L$(LUASRC) -l$(LUALIB) -lncurses

CRAWL_OBJECTS = \
	abl-show.o abyss.o arena.o artefact.o beam.o branch.cc \
	cio.o chardump.o clua.o cloud.o \
	command.o crash-u.o \
	database.o debug.o decks.o delay.o describe.o directn.o dgnevent.o \
	dungeon.o \
	effects.o fight.o files.o hiscores.o initfile.o \
	food.o format.o itemprop.o items.o itemname.o item_use.o it_use2.o \
	it_use3.o invent.o ghost.o Kills.o lev-pand.o \
	libutil.o libunix.o luadgn.o macro.o makeitem.o mapdef.o mapmark.o \
	maps.o menu.o message.o mgrow.o misc.o monplace.o monspeak.o \
	mon-pick.o mon-util.o monstuff.o mstuff2.o mt19937ar.o mtransit.o \
	mutation.o \
	newgame.o notes.o \
	ouch.o output.o overmap.o place.o player.o quiver.o \
	religion.o shopping.o skills.o skills2.o spells1.o \
	spells2.o spells3.o spells4.o \
	spl-book.o spl-cast.o spl-mis.o spl-util.o sqldbm.o state.o store.o \
	stash.o stuff.o tags.o terrain.o traps.o transfor.o travel.o tutorial.o \
	version.o view.o xom.o

OBJECTS = monster.o

OBJECTS += $(CRAWL_OBJECTS:%=$(CRAWL_PATH)/%)

.cc.o:
	${CXX} ${CFLAGS} -o $@ -c $<

all: stable

stable: monster

trunk: monster-trunk

checkout-stable:
	cd $(CRAWL_PATH) && \
	( test "$$(git branch | grep '^\*')" = "* $(STABLE)" || \
	  git checkout $(STABLE) && git clean -f -d -x)

checkout-trunk:
	cd $(CRAWL_PATH) && \
	( test "$$(git branch | grep '^\*')" = "* $(TRUNK)" || \
	  git checkout $(TRUNK) && git clean -f -d -x)

monster-trunk: checkout-trunk $(OBJECTS) $(LUASRC)/$(LUALIBA) $(FSQLLIBA)
	g++ $(CFLAGS) -o $@ $(OBJECTS) $(LFLAGS)
	strip -s $@

monster: checkout-stable $(OBJECTS) $(LUASRC)/$(LUALIBA) $(FSQLLIBA)
	g++ $(CFLAGS) -o $@ $(OBJECTS) $(LFLAGS)
	strip -s $@

$(LUASRC)/$(LUALIBA):
	echo Building Lua...
	cd $(LUASRC) && $(MAKE) crawl_unix

$(FSQLLIBA):
	echo Building SQLite
	cd $(SQLSRC) && $(MAKE)

test: monster
	./monster quasit

install: monster
	cp monster $(HOME)/bin/

install-trunk: monster-trunk
	cp monster-trunk $(HOME)/bin/

clean:
	rm -f *.o
	rm -f monster
	cd $(CRAWL_PATH) && git clean -f -d -x

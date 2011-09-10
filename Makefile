# Use 'make stable' to compile monster, 'make trunk' to compile monster-trunk.
# Use 'make install' to install monster, 'make install-trunk' to install
# monster-trunk.

# Use master
TRUNK = master

CRAWL_PATH=crawl-ref/crawl-ref/source

CXX = g++
LUASRC := $(CRAWL_PATH)/contrib/lua/src
LUALIB = lua
LUALIBA = lib$(LUALIB).a

PYTHON = python

SQLSRC := $(CRAWL_PATH)/contrib/sqlite
SQLLIB   := sqlite3
SQLLIBA  := lib$(SQLLIB).a
FSQLLIBA := $(SQLSRC)/$(SQLLIBA)

ifdef USE_MERGE_BASE
MERGE_BASE := $(shell cd $(CRAWL_PATH) ; git merge-base HEAD $(USE_MERGE_BASE))
endif

CFLAGS = -Wall -Wno-parentheses -DNDEBUG -DUNIX \
	-I$(LUASRC) -I$(SQLSRC) -I$(CRAWL_PATH) -I/usr/include/ncursesw -g

LFLAGS = $(FSQLLIBA) $(LUASRC)/$(LUALIBA) -lncursesw -lz

include $(CRAWL_PATH)/makefile.obj

CRAWL_OBJECTS := $(OBJECTS:main.o=)
CRAWL_OBJECTS += libunix.o crash-u.o

ALL_OBJECTS = monster-main.o vault_monster_data.o vault_monsters.o $(CRAWL_PATH)/version.o
ALL_OBJECTS += $(CRAWL_OBJECTS:%=$(CRAWL_PATH)/%)

all: vaults trunk

$(CRAWL_PATH)/compflag.h: $(CRAWL_OBJECTS:%.o=$(CRAWL_PATH)/%.cc)
	cd $(CRAWL_PATH) ; ./util/gen-cflg.pl compflag.h "$(CFLAGS)" "$(LFLAGS)"

$(CRAWL_PATH)/build.h: $(CRAWL_OBJECTS:%.o=$(CRAWL_PATH)/%.cc)
	cd $(CRAWL_PATH) ; ./util/gen_ver.pl build.h "$(MERGE_BASE)"

$(CRAWL_PATH)/version.cc: $(CRAWL_PATH)/build.h $(CRAWL_PATH)/compflag.h

$(CRAWL_PATH)/art-enum.h $(CRAWL_PATH)/art-data.h: $(CRAWL_OBJECTS:%.o=$(CRAWL_PATH)/%.cc)
	cd $(CRAWL_PATH) ; ./util/art-data.pl

$(CRAWL_PATH)/mon-mst.h: $(CRAWL_PATH)/mon-spll.h
	cd $(CRAWL_PATH) ; ./util/gen-mst.pl

$(CRAWL_PATH)/mon-util.o: $(CRAWL_PATH)/mon-mst.h

.cc.o:
	${CXX} ${CFLAGS} -o $@ -c $<

vault_monster_data.o: vaults

trunk: monster

vault_monster_data.o:
	${CXX} ${CFLAGS} -o vault_monster_data.o -c vault_monster_data.cc

vaults: | update-cdo-git
	rm -f vault_monster_data.cc vault_monster_data.o
	${PYTHON} parse_des.py --verbose

update-cdo-git:
	[ "`hostname`" != "ipx14623" ] || sudo -H -u git /var/cache/git/crawl-ref.git/update.sh

monster: vaults update-cdo-git $(CRAWL_PATH)/art-data.h $(ALL_OBJECTS) $(LUASRC)/$(LUALIBA) $(FSQLLIBA)
	g++ $(CFLAGS) -o $@ $(ALL_OBJECTS) $(LFLAGS)

$(LUASRC)/$(LUALIBA):
	echo Building Lua...
	cd $(LUASRC) && $(MAKE) all

$(FSQLLIBA):
	echo Building SQLite
	cd $(SQLSRC) && $(MAKE)

test: monster
	./monster quasit

install: monster tile_info.txt
	strip -s monster
	cp monster $(HOME)/bin/

tile_info.txt:
	${PYTHON} parse_tiles.py --verbose

clean:
	rm -f *.o
	rm -f monster monster-trunk
	rm -f *.pyc vault_monster_data.cc
	cd $(CRAWL_PATH) && git clean -f -d -x && git pull

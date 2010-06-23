# Use 'make stable' to compile monster, 'make trunk' to compile monster-trunk.
# Use 'make install' to install monster, 'make install-trunk' to install
# monster-trunk.

# Branch names to target in git repo.
STABLE = stone_soup-0.6
# Use master
TRUNK = master

CRAWL_PATH=crawl-ref/crawl-ref/source

CXX = g++
LUASRC := $(CRAWL_PATH)/contrib/lua/src
LUALIB = lua
LUALIBA = lib$(LUALIB).a

SQLSRC := $(CRAWL_PATH)/contrib/sqlite
SQLLIB   := sqlite3
SQLLIBA  := lib$(SQLLIB).a
FSQLLIBA := $(SQLSRC)/$(SQLLIBA)

ifdef USE_MERGE_BASE
MERGE_BASE := $(shell cd $(CRAWL_PATH) ; git merge-base HEAD $(USE_MERGE_BASE))
endif

CFLAGS = -Wall -Wno-parentheses -DNDEBUG -DUNIX \
	-I$(LUASRC) -I$(SQLSRC) -I$(CRAWL_PATH) -g

LFLAGS = $(FSQLLIBA) $(LUASRC)/$(LUALIBA) -lncurses

include $(CRAWL_PATH)/makefile.obj

CRAWL_OBJECTS := $(OBJECTS:main.o=)
CRAWL_OBJECTS := $(CRAWL_OBJECTS:dbg-asrt.o=) libunix.o

ALL_OBJECTS = monster-main.o $(CRAWL_PATH)/version.o
ALL_OBJECTS += $(CRAWL_OBJECTS:%=$(CRAWL_PATH)/%)

all: trunk

$(CRAWL_PATH)/compflag.h: $(CRAWL_OBJECTS:%.o=$(CRAWL_PATH)/%.cc)
	cd $(CRAWL_PATH) ; ./util/gen-cflg.pl compflag.h "$(CFLAGS)" "$(LFLAGS)"

$(CRAWL_PATH)/build.h: $(CRAWL_OBJECTS:%.o=$(CRAWL_PATH)/%.cc)
	cd $(CRAWL_PATH) ; ./util/gen_ver.pl build.h "$(MERGE_BASE)"

$(CRAWL_PATH)/version.cc: $(CRAWL_PATH)/build.h $(CRAWL_PATH)/compflag.h

$(CRAWL_PATH)/art-enum.h $(CRAWL_PATH)/art-data.h: $(CRAWL_OBJECTS:%.o=$(CRAWL_PATH)/%.cc)
	cd $(CRAWL_PATH) ; ./util/art-data.pl

.cc.o:
	${CXX} ${CFLAGS} -o $@ -c $<

trunk: monster-trunk

update-cdo-git:
	[ "`hostname`" != "ipx14623" ] || sudo -H -u git /var/cache/git/crawl-ref.git/update.sh

checkout-trunk:
	cd $(CRAWL_PATH) && \
	( test "$$(git branch | grep '^\*')" = "* $(TRUNK)" || \
		( git checkout $(TRUNK) && git clean -f -d -x && git pull ) )

monster-trunk: update-cdo-git checkout-trunk $(CRAWL_PATH)/art-data.h $(ALL_OBJECTS) $(LUASRC)/$(LUALIBA) $(FSQLLIBA)
	g++ $(CFLAGS) -o $@ $(ALL_OBJECTS) $(LFLAGS)

$(LUASRC)/$(LUALIBA):
	echo Building Lua...
	cd $(LUASRC) && $(MAKE) all

$(FSQLLIBA):
	echo Building SQLite
	cd $(SQLSRC) && $(MAKE)

test: monster
	./monster-trunk quasit

install-trunk: monster-trunk
	cp monster-trunk $(HOME)/bin/

clean:
	rm -f *.o
	rm -f monster
	cd $(CRAWL_PATH) && git clean -f -d -x && git pull

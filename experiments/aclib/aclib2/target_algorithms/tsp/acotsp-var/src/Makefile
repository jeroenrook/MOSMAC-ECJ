VERSION=1.05-tuning

DEBUG=0
RNODE=iridiacluster

ifneq ($(DEBUG),0)
DEBUGFLAGS=-g3 -DMALLOC_CHECK_=3
else
DEBUGFLAGS=
endif

OPT_FLAGS=-O2 -ffast-math
WARN_FLAGS=-Wall -Wextra -std=c99 -pedantic
CFLAGS=$(WARN_FLAGS) $(OPT_FLAGS) $(DEBUGFLAGS)
LDLIBS+=-lm
OS:=$(shell uname)
ifeq ($(OS),Linux)
TIMER=unix_timer
else
TIMER=timer
endif

acotsp: acotsp.o TSP.o utilities.o ants.o InOut.o $(TIMER).o ls.o parse.o adaptation.o

all: clean acotsp TAGS

clean:
	@$(RM) *.o acotsp

acotsp.o: acotsp.c

TSP.o: TSP.c TSP.h

ants.o: ants.c ants.h acotsp-parameters.def

InOut.o: InOut.c InOut.h

utilities.o: utilities.c utilities.h

ls.o: ls.c ls.h

parse.o: parse.c parse.h acotsp-parameters.def

$(TIMER).o: $(TIMER).c $(TIMER).h

adaptation.o : CFLAGS+=-DDEBUG=$(DEBUG)
adaptation.o : adaptation.h

TAGS: *.c *.h
	etags *.c *.h

rsync : all
ifdef RNODE
	rsync -rlp -CIzc -L --delete --copy-unsafe-links --exclude=.svn --progress --relative --exclude=acotsp                           \
	.                               \
	$(RNODE):~/src/acotsp-var-new/
else
	@echo "ERROR: You must specify a remote node (e.g., RNODE=b21)"
	@exit 1
endif

dist : DIST_SRC_FILES=*.c *.h README *.tsp Makefile gpl.txt tuning.patch *.def
dist : DISTFILENAME:=ACOTSPvar-$(VERSION)
dist : all
	@(mkdir -p ../$(DISTFILENAME)			\
	&& rsync -rlpC --exclude=.svn $(DIST_SRC_FILES) ../$(DISTFILENAME)/ \
        && cd .. 	\
	&& tar cf - $(DISTFILENAME) | gzip -f9 > $(DISTFILENAME).tar.gz \
	&& rm -rf ./$(DISTFILENAME)					\
	&& echo "$(DISTFILENAME).tar.gz created." && cd $(CWD) )


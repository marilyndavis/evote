# $Id: makefile,v 1.5 2003/01/25 22:04:36 marilyndavis Exp $ 
#/* * eVote - Software for online consensus development.
# * Copyright (C) 2015 Marilyn Davis <marilyn@deliberate.com>
# *
# * This file is part of eVote.
# *
# * eVote is free software: you can redistribute it and/or modify
# * it under the terms of the GNU Affero General Public License as
# * published by the Free Software Foundation, either version 3 of the
# * License, or (at your option) any later version.
# *
# * eVote is distributed in the hope that it will be useful,
# * but WITHOUT ANY WARRANTY; without even the implied warranty of
# * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# * GNU Affero General Public License for more details.
# *
# * You should have received a copy of the GNU Affero General Public License
# * along with eVote.  If not, see <http://www.gnu.org/licenses/>.
# */
CC = gcc

CFLAGS = -g  -DEDEBUG -D_GNU_SOURCE -Wall
DEMOFLAGS =  -g -DDEMO  -DTIME -DTITLE -Wall
LDFLAGS = 
YOUR_BIN = "."

LD = @gcc 
LIBS = testfns.o Clerkdummy eVotedummy ../demo/eVote_io.o 

INCS = ../demo/eVote.h  ../Clerklib/Clerkdef.h ../Clerklib/Clerk.h \
	../demo/eVoteui/eVoteui.h

LOADERS = ../Clerklib/Clerklib.a ../demo/eVote_io.o testfns.o 

all: ipctest porttest longtest whom eVote_test whotest

$(LIBS):$(INCS)  
porttest: evtstart fastvoter porttest.o $(INCS) $(LIBS) 
	$(LD) $(LDFLAGS) -o porttest porttest.o $(LOADERS)

longtest: longtest.o slowvoter $(INCS) $(LIBS)
	$(LD) $(LDFLAGS) -o longtest longtest.o $(LOADERS)

fastvoter: fastvoter.o $(LIBS) $(INCS)
	$(LD) $(LDFLAGS) -o fastvoter fastvoter.o $(LOADERS) \
	../demo/eVoteui/eVoteuilib.a

slowvoter: slowvoter.o $(LIBS) $(INCS)
	$(LD) $(LDFLAGS) -o slowvoter slowvoter.o $(LOADERS) \
	../demo/eVoteui/eVoteuilib.a

evtstart: evtstart.o $(LIBS) $(INCS)
	$(LD) $(LDFLAGS) -o evtstart evtstart.o $(LOADERS) \
	../demo/eVoteui/eVoteuilib.a

eVote.h: ../Clerklib/Clerkdef.h ../demo/eVoteui/eVoteui.h ../Clerklib/Clerk.h

whom: whom.o
	$(LD) $(LDFLAGS) whom.o -o whom
ipctest: ipctest.o
	$(LD) $(LDFLAGS) ipctest.o -o ipctest

eVote_test: eVote_test.o ../demo/eVote_io.o  testfns.o $(INCS) $(LIBS)
	$(LD) $(LDFLAGS) -o eVote_test eVote_test.o ../demo/eVote_io.o \
		testfns.o ../Clerklib/Clerklib.a \
		../demo/eVoteui/conf.o \
		../demo/eVoteui/item.o \
		../demo/eVoteui/voter.o \
		../demo/eVoteui/misc.o \
		../demo/eVoteui/input.o 
../demo/demo.h: ../demo/eVoteui/eVoteui.h \
	../Clerklib/Clerk.h ../Clerklib/Clerkdef.h

eVotedummy:
	@cd ../demo/eVoteui; make "CFLAGS=$(DEMOFLAGS)"

CLERKFLAGS = -g -c -D_XOPEN_SOURCE -DVERSION=2.52 -DNUMBER=1 -DEDEBUG -Wall
Clerkdummy:
	@cd ../Clerklib; make "CFLAGS=$(CLERKFLAGS)"

whotest: whotest.o $(INCS) $(LIBS)
	$(LD) $(LDFLAGS) -o whotest whotest.o ../Clerklib/Clerklib.a \

../demo.h: ../eVoteui/eVoteui.h \
	../Clerklib/Clerk.h ../Clerklib/Clerkdef.h

../demo/eVote_io.o: $(INCS)
	cd ../demo; ${MAKE} -k YOUR_BIN=\"${YOUR_BIN}\"
testfns.o: $(INCS)


clean:
	@touch porttest.o
	@rm *.o



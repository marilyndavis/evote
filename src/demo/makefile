# $Id: makefile,v 1.6 2003/01/25 22:04:34 marilyndavis Exp $ 
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
INCS = eVote.h ../Clerklib/Clerkdef.h ../Clerklib/Clerk.h

LIBS =  eVoteui/eVoteuilib.a ../Clerklib/Clerklib.a  

# YOUR_BIN = /home/eVote/bin
YOUR_BIN = /usr/bin
CFLAGS =  -g -c -DDEMO -Wall
LDFLAGS = 

LD = @gcc 

all: $(YOUR_BIN)/eVote_demo $(YOUR_BIN)/eVote

$(YOUR_BIN)/eVote_demo: eVote_demo.o eVote_io.o $(LIBS) $(INCS) 
	@touch $(YOUR_BIN)/eVote_demo
	@chmod a+rw $(YOUR_BIN)/eVote_demo
	$(LD) $(LDFLAGS) -o $(YOUR_BIN)/eVote_demo \
		eVote_demo.o eVote_io.o $(LIBS)  
	@chmod a-rw,a+x,u+r $(YOUR_BIN)/eVote_demo

eVote.h: eVoteui/eVoteui.h ../Clerklib/Clerk.h ../Clerklib/Clerkdef.h

eVoteui/eVoteuilib.a:$(INCS) eVoteui/blurbs.c \
		eVoteui/conf.o eVoteui/explain.c \
		eVoteui/input.c eVoteui/item.c \
		eVoteui/menu.c eVoteui/misc.c \
		eVoteui/poetry.c eVoteui/queries.c \
		eVoteui/ui.c eVoteui/voter.c
	@cd eVoteui; ${MAKE} "CFLAGS = $(CFLAGS)"; cd ..

# The eVote executable uses some files from the library
# but not the whole library.

$(YOUR_BIN)/eVote: eVote.o eVote_io.o eVoteui/command.o \
	eVoteui/conf.o eVoteui/input.o \
	eVoteui/misc.o eVoteui/voter.o \
	../Clerklib/Clerklib.a  $(INCS) 
	@touch $(YOUR_BIN)/eVote
	@chmod u+rw $(YOUR_BIN)/eVote
	$(LD) $(LDFLAGS) -o $(YOUR_BIN)/eVote \
		eVote.o eVote_io.o  eVoteui/command.o \
		eVoteui/conf.o eVoteui/input.o \
		eVoteui/misc.o eVoteui/voter.o \
		../Clerklib/Clerklib.a  
	@chmod a-rw,a+x,u+r $(YOUR_BIN)/eVote

eVoteui/command.o:eVoteui/command.c
	@cd eVoteui; ${MAKE} "CFLAGS = $(CFLAGS)"; cd ..

eVote.o eVote_demo.o eVote_io.o: eVoteui/eVoteui.h \
	../Clerklib/Clerk.h ../Clerklib/Clerkdef.h

clean:
	@touch eVote.o
	@rm *.o
	@cd eVoteui; ${MAKE} clean


# $Id: makefile,v 1.6 2003/01/25 22:04:34 marilyndavis Exp $ 
# makefile for the eVoteui directory -- telnet interface library
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
CFLAGS = -g -c -DDEMO -Wall

all: eVoteuilib.a command.o  # command.o is for the eVote executable

eVoteuilib.a: conf.o explain.o \
	input.o item.o menu.o misc.o \
	poetry.o queries.o ui.o voter.o blurbs.o
	@ar r $@ $?
	@ranlib eVoteuilib.a

command.o conf.o input.o item.o menu.o  \
	misc.o poetry.o queries.o ui.o voter.o blurbs.o: eVoteui.h \
	 ../eVote.h \
	../../Clerklib/Clerk.h ../../Clerklib/Clerkdef.h

explain.o : ../eVote.h explain.txt

clean:
	@touch poetry.o
	@rm *.o
	@touch eVoteuilib.a
	@rm *.a

# $Id: makefile,v 1.6 2003/01/25 22:04:34 marilyndavis Exp $ 
# makefile for the eVoteui directory -- telnet interface library
#########################################################
#    Copyright (c) 1994...2015 Deliberate.com Patented.
#    by Marilyn Davis
#  This program is free software; you can redistribute it and/or
#  modify it under the terms of the eVote(R)/Clerk License as
#  published by Deliberate.Com.
#
#  This program is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#  eVote(R)/Clerk License for more details.
#
#  You should have received a copy of the eVote(R)/Clerk License
#  along with this program in EVOTE_HOME_DIR/eVote/doc/eVote.doc. 
#  If not, write to Deliberate.Com 2555 W. Middlefield Rd. #150,
#  Mountain View, CA 94043 USA or office@deliberate.com.
##########################################################

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

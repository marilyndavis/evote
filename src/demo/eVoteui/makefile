# $Id: makefile,v 1.6 2003/01/25 22:04:34 marilyndavis Exp $ 
# makefile for the eVoteui directory -- telnet interface library
#########################################################
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

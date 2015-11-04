# $Id: makefile,v 1.8 2003/01/28 04:11:27 marilyndavis Exp $ 
# main makefile for the mail interface
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
##########################################################/

SERVER = majordomo
YOUR_BIN = /usr/local/majordomo
MAIL_BIN = $(YOUR_BIN)

# For mailman, use these instead:
# SERVER = mailman
# YOUR_BIN = /home/mailman/scripts
# MAIL_BIN = $(YOUR_BIN)/../bin

CC = gcc 
LIBS = mailui/mailuilib.a  message/messagelib.a ../Clerklib/Clerklib.a  
LIBDUMMYS =  mailuilib_o messagelib_o 
INCS = mailui/mailui.h message/message.h mailui/maildef.h ../Clerklib/Clerkdef.h ../tools/filters/filter.h
FILTER = ../tools/filters/filter.o
# on ROSA YOUR_BIN = /u1/eVote/src/mail
# CFLAGS = -g -c -Wall -DROSA
CFLAGS = -g -Wall
LDFLAGS = 

LD = @gcc
CD = @cd

all: $(YOUR_BIN)/eVote_insert $(MAIL_BIN)/eVote_mail $(YOUR_BIN)/eVote_petition 

$(YOUR_BIN)/eVote_insert: eVote_insert.o $(SERVER)/list.o $(LIBDUMMYS) $(INCS) eVote_insert.h $(FILTER)
	$(LD) $(LDFLAGS) -o $(YOUR_BIN)/eVote_insert \
		eVote_insert.o $(SERVER)/list.o $(LIBS) $(FILTER)

$(MAIL_BIN)/eVote_mail: eVote_mail.o $(SERVER)/list.o $(LIBDUMMYS) $(INCS) $(FILTER)
	$(LD) $(LDFLAGS) -o $(MAIL_BIN)/eVote_mail \
		eVote_mail.o $(SERVER)/list.o $(LIBS) $(FILTER)

$(YOUR_BIN)/eVote_petition: eVote_petition.o $(SERVER)/list.o $(LIBDUMMYS) $(INCS) $(FILTER)
	$(LD) $(LDFLAGS) -o $(YOUR_BIN)/eVote_petition \
		eVote_petition.o $(SERVER)/list.o $(LIBS) $(FILTER)

mailuilib_o:
	$(CD) mailui; ${MAKE} "CFLAGS = $(CFLAGS)"; cd ..

messagelib_o:
	$(CD) message; ${MAKE} "CFLAGS = $(CFLAGS)"; cd ..

eVote_insert.o eVote_petition.o eVote_mail.o:$(INCS)

$(FILTER): ../tools/filters/filter.c
	$(CD) ../tools/filters; ${MAKE} "CFLAGS=$(CFLAGS)"; cd ../../mail

$(SERVER)/list.o: $(SERVER)/list.c
	$(CC) ${CFLAGS} -DW_BIN=\"$(YOUR_BIN)\" $(SERVER)/list.c -o $(SERVER)/list.o -c

clean: 
	@touch eVote_insert.o
	@rm *.o; 
	$(CD) message; ${MAKE} clean 
	$(CD) mailui; ${MAKE} clean
	$(CD) ../tools/filters; ${MAKE} clean
	@touch $(SERVER)/list.o
	@rm $(SERVER)/list.o
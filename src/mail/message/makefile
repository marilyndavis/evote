# $Id: makefile,v 1.6 2003/01/25 22:04:35 marilyndavis Exp $ 
# makefile for mail/message/messagelib.a  -- functions that
# relate to reading and writing the messages.
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
CC = gcc 
YOUR_WBIN=/usr/local/majordomo
INCS = ../mailui/mailui.h message.h ../mailui/maildef.h \
       ../../Clerklib/Clerkdef.h ../../tools/filters/filter.h

CFLAGS = -Wall -g 
LDFLAGS = -v 
LD = @gcc
OBJS =  in_message.o out_message.o send.o token.o

messagelib.a: $(OBJS)
	@ar r $@ $?
	@ranlib messagelib.a

$(OBJS): $(INCS)

clean:
	@touch token.o
	@rm *.o
	@touch messagelib.a
	@rm *.a

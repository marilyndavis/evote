# $Id: makefile,v 1.6 2003/01/25 22:04:35 marilyndavis Exp $ 
# makefile for mail/message/messagelib.a  -- functions that
# relate to reading and writing the messages.
#########################################################
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

# $Id: makefile,v 1.6 2003/01/25 22:04:35 marilyndavis Exp $ 
# makefile for mail/message/messagelib.a  -- functions that
# relate to reading and writing the messages.
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

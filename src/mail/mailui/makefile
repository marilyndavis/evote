# $Id: makefile,v 1.5 2003/01/25 22:04:35 marilyndavis Exp $ 
# makefile for ../src/mail/mailui
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

# No one but deliberate.com wants -DROSA -- it's special language for
# our Kopilli Ketzalli petition.

# -DLINUX is for signal catching in signal.c  Non Linux has some other
#   signals

# CFLAGS = -g -DROSA -Wall
CFLAGS = -g -Wall 

OBJS = confirm.o do_eVote.o form.o help.o listm.o lock.o new_poll.o \
	petition.o poll.o poll_list.o \
	queriesm.o report.o signal.o\
	signatures.o spread.o subject.o text.o util.o voterm.o table.o \
	pet_out.o pet_out_es.o pet_out_fr.o 
INCS = mailui.h maildef.h ../../Clerklib/Clerkdef.h ../message/message.h \
	../../tools/filters/filter.h

all: mailuilib.a

table.o:table.cc table.h $(INCS)
	@echo ""
	@echo ""
	@echo "--> Warnings like \"table.cc:479: warning: taking address of temporary\" are ok."
	@echo ""
	@echo ""
	$(CC) -c -o table.o $(CFLAGS) -x c++ table.cc
	@echo ""
	@echo ""
	@echo "--> Warnings like \"table.cc:479: warning: taking address of temporary\" are ok."
	@echo ""
	@echo ""

mailuilib.a: $(OBJS)
	@ar r $@ $?
	@ranlib mailuilib.a

$(OBJS): $(INCS)

clean:
	@touch table.o
	@rm *.o
	@touch mailuilib.a
	@rm *.a

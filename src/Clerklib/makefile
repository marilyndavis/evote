# $Id: makefile,v 1.7 2003/04/07 18:31:21 marilyndavis Exp $ 
# makefile for Clerklib
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

CC = @gcc 

# -DLINUX is used in ipc_msg.c to not redefine struct msgbuf.

# If compiling for the port directory, you need -DEDEBUG

CFLAGS = -g -DVERSION=\"2.52\" -DNUMBER=1 -DEDEBUG -Wall -D_GNU_SOURCE

# if DNUMBER == 0, and -DEXP=02 or something, it's the demo and it 
# expires on 1/1/EXP
# and it drops dead on 1/1/EXP+1

all: Clerklib.a

Clerklib.a: confl.o \
	iteml.o maint.o \
	ipc_shm.o start.o stats.o \
	voterl.o  ipc_msg.o \
	queriesl.o 
	@ar r $@ $?
	@ranlib Clerklib.a

confl.o  \
	iteml.o maint.o  \
	stats.o voterl.o \
	queriesl.o: msgdef.h Clerkdef.h \
	ipc_msg.h Clerk.h

ipc_shm.o ipc_msg.o: msgdef.h \
	ipc_msg.h Clerkdef.h Clerk.h
ipc_msg.o: mtype.c
mtype.o: Clerkdef.h

ipc_msg.o ipc_shm.o start.o: eVote_defaults.h

clean:
	@touch start.o
	@rm *.o
	@touch Clerklib.a
	@rm *.a


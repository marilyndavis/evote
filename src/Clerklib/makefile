# $Id: makefile,v 1.7 2003/04/07 18:31:21 marilyndavis Exp $ 
# makefile for Clerklib
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


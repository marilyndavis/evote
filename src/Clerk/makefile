# $Id: makefile,v 1.8 2003/10/17 23:07:49 marilyndavis Exp $ 
# makefile -- for The Clerk
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

#/*********************************************************
# **********************************************************/
CPP = g++ 
CPPFLAGS =  -Wall -c -g -DEDEBUG -DTITLE -DTIME -DPETITION -I/usr/lib/g++-include -D_GNU_SOURCE -v

# -D_XOPEN_SOURCE  for egcs

# DEST = /home/eVote/bin 
DEST = /usr/bin
LD = @g++
LDFLAGS = 

OBJS =  main.o debug.o inq.o outq.o qlist.o instruct.o noq.o \
	hasq.o needsq.o respond1.o maybeq.o conflist.o conf.o \
	itemlist.o applist.o memseg.o memlist.o \
	grouplis.o dropord.o item.o titem.o sitem.o gitem.o \
	itemgrou.o voterlis.o voter.o \
	hash.o ballotbo.o ballot.o wholist.o util.o 

all:$(DEST)/eVote_Clerk 

$(DEST)/eVote_Clerk:$(OBJS) 
	@touch $(DEST)/eVote_Clerk
	@chmod u+wrx $(DEST)/eVote_Clerk
	$(LD) $(LDFLAGS) -o $(DEST)/eVote_Clerk $(OBJS)
#	touch ./eVote_Clerk
#	chmod u+wr ./eVote_Clerk
#	cp $(DEST)/eVote_Clerk ./eVote_Clerk 
#	chmod a-rwx,u+x $(DEST)/eVote_Clerk

conf.h:ballotbo.h itemlist.h voterlis.h 
instruct.h:itemid.h ballot.h conf.h
itemlist.h: itemid.h applist.h
applist.o: applist.h memlist.h memseg.h evotedef.h itemlist.h voterlis.h item.h conf.h conflist.h ../Clerklib/msgdef.h ../Clerklib/Clerk.h ../Clerklib/Clerkdef.h
memseg.o memlist.o: memlist.h memseg.h
ballot.o: evotedef.h ballot.h ../Clerklib/msgdef.h ../Clerklib/Clerk.h
ballotbo.o:	evotedef.h ballot.h conf.h itemlist.h conflist.h\
	voterlis.h ballotbo.h qlist.h ../Clerklib/msgdef.h ../Clerklib/Clerk.h
conf.o: applist.h evotedef.h conf.h voterlis.h itemlist.h ballotbo.h conflist.h ../Clerklib/msgdef.h ../Clerklib/Clerk.h ../Clerklib/Clerkdef.h
conflist.o: conflist.h conf.h itemlist.h evotedef.h ../Clerklib/msgdef.h ../Clerklib/Clerk.h ballotbo.h
debug.o: evotedef.h
gitem.o:evotedef.h itemgrou.h item.h itemlist.h ballotbo.h conf.h ../Clerklib/msgdef.h ../Clerklib/Clerk.h ../Clerklib/Clerkdef.h
grouplis.o: evotedef.h itemlist.h itemgrou.h conf.h item.h ../Clerklib/msgdef.h ../Clerklib/Clerk.h
hash.o: ballot.h conf.h ballotbo.h ../Clerklib/msgdef.h ../Clerklib/Clerk.h evotedef.h
instruct.o inq.o outq.o needsq.o qlist.o \
	maybeq.o hasq.o noq.o respond1.o:conflist.h \
	voterlis.h voter.h item.h itemlist.h \
	conf.h instruct.h ../Clerklib/msgdef.h ../Clerklib/Clerk.h \
	evotedef.h ../Clerklib/Clerkdef.h
item.o titem.o sitem.o: evotedef.h item.h itemlist.h ballotbo.h conf.h ../Clerklib/msgdef.h ../Clerklib/Clerk.h ../Clerklib/Clerkdef.h
itemgrou.o: evotedef.h conf.h itemgrou.h item.h itemlist.h ../Clerklib/msgdef.h ../Clerklib/Clerk.h ../Clerklib/Clerkdef.h
itemlist.o: applist.h evotedef.h item.h itemlist.h conf.h ballot.h ballotbo.h \
	voter.h voterlis.h itemgrou.h ../Clerklib/msgdef.h \
	../Clerklib/Clerk.h ../Clerklib/Clerkdef.h
dropord.o: evotedef.h itemlist.h item.h
qlist.o : evotedef.h ../Clerklib/msgdef.h ../Clerklib/Clerk.h instruct.h qlist.h voter.h inq.h outq.h
main.o:	conflist.h evotedef.h qlist.h inq.h wholist.h itemlist.h ../Clerklib/Clerk.h 
qlist.o: evotedef.h inq.h qlist.h voter.h 
util.o: evotedef.h inq.h ../Clerklib/mtype.c ../Clerklib/Clerkdef.h
voter.o: evotedef.h item.h ballotbo.h conf.h voter.h voterlis.h ballot.h \
	../Clerklib/msgdef.h ../Clerklib/Clerk.h ../Clerklib/Clerkdef.h
voterlis.o: qlist.h evotedef.h voter.h voterlis.h conf.h \
	ballot.h ../Clerklib/msgdef.h ../Clerklib/Clerk.h
wholist.o: wholist.h ../Clerklib/msgdef.h ../Clerklib/Clerkdef.h evotedef.h conflist.h ballotbo.h

evotedef.h: ../Clerklib/Clerkdef.h ../Clerklib/eVote_defaults.h
.SUFFIXES:
.SUFFIXES: .o .cc
.cc.o:
	$(CPP) $(CPPFLAGS) $< 
clean:
	@touch main.o
	@rm *.o



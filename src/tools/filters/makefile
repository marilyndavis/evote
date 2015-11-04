# $Id: makefile,v 1.8 2003/10/17 23:07:50 marilyndavis Exp $ 
##
#   Makefile for the family of mail security programs:
#   angel, shelter, fence; and the mail filters forward,
#   pick, and puppet.  See the DOC file.
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
#   Your domain:

DOMAIN = deliberate.com

#
#   Alter this alert address so that the alert messages go
#   where you want them:

ALERT_ADDRESS = majordom@deliberate.com

#   HOME  -- for majordomo programs

HOME = /home


#   GOODLISTS should point to your file that lists the email
#   lists that your protected addresses want to receive:

GOODLISTS = $(HOME)/majordomo/goodlists

#  BADGUYS should name the file that lists known attackers.
#  If an entry in this file matches a string in the headers
#  the message will be blocked and sent to the alert address

BADGUYS = $(HOME)/majordomo/badguys

#  If you are tired of alerts about the blocking of a certain
#  address, put it in this file and mail that has a string in
#  the headers with this address will quietly be deleted ...
#  no message will be sent to the alert address.

FORGETS_PATH = $(HOME)/majordomo/forgets

#  Filename for writing the log.  

LOG_FNAME = $(HOME)/majordomo/filters.log

#  Path to sendmail.  

SENDMAIL_PATH = /usr/lib/sendmail

# sendmail command to use:

SENDMAIL_COMMAND = "sendmail -t"

#  Path to your wrapper program and where other programs executed
#  from the command line are expected to reside.

WRAPPER_PATH = $(HOME)/majordomo

CFLAGS = -Wall -g

MYFLAGS = -DSENDMAIL_PATH=\"$(SENDMAIL_PATH)\" \
	-DSENDMAIL_COMMAND=\"$(SENDMAIL_COMMAND)\" \
	-DTHIS_DOMAIN=\"$(DOMAIN)\" -DLOG_FNAME=\"$(LOG_FNAME)\" \
	-DALERT_ADDRESS=\"$(ALERT)\" -DGOODLISTS_PATH=\"$(GOODLISTS)\" \
	-DBADGUYS_PATH=\"$(BADGUYS)\" -DFORGETS_PATH=\"$(FORGETS_PATH)\" \
	-DWRAPPER_PATH=\"$(WRAPPER)\" -DTIME_OUT=2000

CC = @gcc $(MYFLAGS)

LDFLAGS = 

LD = @gcc 

all: shelter angel fence forward pick puppet
shelter: shelter.o filter.o
	$(LD) $(LDFLAGS) -o shelter shelter.o filter.o
angel: angel.o filter.o
	$(LD) $(LDFLAGS) -o angel angel.o filter.o
fence: fence.o filter.o
	$(LD) $(LDFLAGS) -o fence fence.o filter.o
forward: forward.o filter.o
	$(LD) $(LDFLAGS) -o forward forward.o filter.o
pick: pick.o filter.o pick.h
	$(LD) $(LDFLAGS) -o pick pick.o filter.o
puppet: puppet.o filter.o 
	$(LD) $(LDFLAGS) -o puppet puppet.o filter.o

install: all
	mv shelter $(WRAPPER_PATH)
	mv angel $(WRAPPER_PATH)
	mv fence $(WRAPPER_PATH)
	mv forward $(WRAPPER_PATH)
	mv pick $(WRAPPER_PATH)
	mv puppet $(WRAPPER_PATH)

shelter.o filter.o angel.o fence.o forward.o pick.o puppet.o: filter.h


clean:
	@touch filter.o
	@rm *.o


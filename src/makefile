# $Id: makefile,v 1.20 2003/10/20 17:06:08 marilyndavis Exp $
VER = "2.52b2"
# main makefile for the eVote(R)/Clerk system
# make install for a new installation -- At the end of the make,
#   the *compilation* window gives instructions for su-ing 
#   to root to finish the installation.
# make clean - to force a total recompile with the next make
# make - to compile everything
# make config - to configure eVote/Clerk/listserver and fix up perms
# make clobber - to remove everything except the sources.  At the
#   end the *compilation* window gives instructions for su-ing
#   to root to finish the clobber.
# If you want to make the mail filters, you should alter
# the makefile in EVOTE_HOME_DIR/eVote/src/tools/filters
# If you want the translator, make it separately at
# EVOTE_HOME_DIR/eVote/src/tools/trans
#
# To make for the gdb, put "-g" on the CFLAGS in this file
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

# What listserver are you using?  It might be:

# LISTSERVER = majordomo

# or it could be:
LISTSERVER = mailman

# It must be one or the other, all in lower case.  

# Your listserver has its own login.  Mailman, for example, usually
# runs its programs from a "mailman" login.  Majordomo usually runs
# its programs from a "majordom" login.

# LOGIN = majordom

# or 

LOGIN = mailman

# If you are running Mailman, this is your mailman login's group name:

GROUP = exim

# What domain are you?

WHEREAMI = maildance.com

# To which login shall emergency messages from eVote be sent?

EVOTE_MAIL_TO = $(LOGIN)

# If you are running Majordomo, skip this one.
# If you are running Mailman, which directory has the mailman
# distribution directory structure?  This is the directory where you
# "make install" for mailman.  The login that is running "make install"
# here must have permission to write into $(MAILMAN_INSTALL)/..

MAILMAN_INSTALL = /b/dev/mm1

# It's good to have a special login for running The Clerk and keeping
# the data files.  This login should have an '*' in the encrypted
# password field of the password entry so that only root can gain
# access, and only by su-ing to clerk.

CLERK = clerk

# Which directory will have the executable eVote_Clerk, the vote-server
# daemon?  This should be on the default path.

EVOTE_BIN = /usr/local/bin

# Where will be eVote's home directory?  In here will go eVote's source
# files and data.  You must have permission to write in this directory!

EVOTE_HOME_DIR = /b/home

# What is your preferred mailer command?  You might like:

MAILER = "/usr/lib/sendmail -f\\\$$sender -t" 

# because it gives the result: MAILER =
# /usr/lib/sendmail -f\$sender -t in the eVote.cf file.

# What directory has the list-servers programs, i.e., resend for
# majordomo or post for mailman?  Maybe it's:
# PROGS = /b/home/majordomo
# for majordomo or

PROGS = /b/home/mm1/scripts

# What directory will contain eVote's mail user-interface executables,
# eVote_insert and eVote_petition, which are compiled from sources in
# $(EVOTE_HOME_DIR)/eVote/src/mail.  For majordomo lists, it should be
# the same directory that has majordomo's "resend" program. For
# mailman, it should be the same directory that has mailman's "post"
# program. (For regression tests, it is different from $(UI_BIN).) In
# either case, probably you want:

UI_BIN = $(PROGS)

# What directory has the list-server's email lists?  For majordomo,
# it might be:

# LISTDIR = /b/home/majordomo/lists

# For mailman, which doesn't have the lists in flat files, eVote still
# needs a place to store some documentation.  I suggest:

LISTDIR = $(EVOTE_HOME_DIR)/eVote/lists

# ----  You probably don't want to touch anything else

#################  THINGS FOR RUNNING REGRESSION TESTS ###############

# TEST = YES  
# or 
TEST = NO

# This is a test mailer, good for running the regression tests in
# EVOTE_HOME_DIR/src/mail/test for testing the mail interface.

# MAILER = $(EVOTE_HOME_DIR)/eVote/src/mail/test/smail_test -f\\\$$sender -t

# If you are running eVote's regression tests in the
# $(EVOTE_HOME_DIR)/eVote/src/mail/test directory, then you want:

# UI_BIN = /usr/local/majordomo

# And, if you are testing eVote's mail interface in the
# $(EVOTE_HOME_DIR)/eVote/src/mail/test directory, then you want to
# point to the post/resend simulators here:

# PROGS = $(EVOTE_HOME_DIR)/eVote/src/mail/test

####################################################################

DIR_PERMS = o-w,o+rx,ug+wrx 
FILE_PERMS = a-x,a+rw,o-w
WHO=`whoami`
ECHO = @echo 
IF = @if 
CD = @cd
CHMOD = @chmod
CHOWN = @chown

all: 
	${MAKE} Clerk_dummy eVote

install:
	${MAKE} all config		

Clerk_dummy: Clerklib/Clerklib.a
	@echo "----> Clerk happening"
	$(CD) Clerk;${MAKE} -k DEST="." CPPFLAGS="-Wall -c -DDEMO -DTITLE -DPETITION -DTIME -I/usr/lib/g++-include -D_GNU_SOURCE"
	$(CHMOD) a-rwx,u+xr Clerk/eVote_Clerk
	$(CD) demo;${MAKE} -k CFLAGS="-Wall -c -DDEMO" YOUR_BIN="."

Clerklib/Clerklib.a:
	@echo "Clerklib happening"
	${IF} [ $(TEST) = "YES" -a -e port ] ;\
            then \
		echo "----> Making Clerklib for port tests" ;\
	        cd Clerklib;${MAKE} clean; ${MAKE} -k CFLAGS="-Wall -c -DVERSION=\\\"$(VER)\\\" -DNUMBER=1 -D_GNU_SOURCE -DEDEBUG" ;\
	        cd ../port; ${MAKE} -k YOUR_BIN=".";\
        fi
	$(CD) Clerklib;${MAKE} clean; ${MAKE} -k CFLAGS="-Wall -c -DVERSION=\\\"$(VER)\\\" -DNUMBER=1 -D_GNU_SOURCE"

eVote: Clerk_dummy
	@echo "----> eVote mail interface happening"
	$(CD) mail; ${MAKE} -k YOUR_BIN="." SERVER=$(LISTSERVER) CFLAGS="-Wall"
	@$(IF) [ $(TEST) = "YES" -a -e mail/test ] ; \
	  then \
	  cd mail/test; ${MAKE} -k ;\
	fi    

config:
	@echo "----> config happening -- this takes a while"
	$(IF) [ ! -e $(EVOTE_HOME_DIR)/eVote ] ;\
		then \
		mkdir $(EVOTE_HOME_DIR)/eVote ;\
		cp -r .. $(EVOTE_HOME_DIR)/eVote ;\
	fi 
	$(IF) [ $(LISTSERVER) = "mailman" ] ;\
		then \
		echo "----> Fixing up Mailman" ;\
		mkdir $(MAILMAN_INSTALL)/../mailman-eVote ;\
		cp -r $(MAILMAN_INSTALL)/* $(MAILMAN_INSTALL)/../mailman-eVote ;\
		cp mail/mailman/mailman-2.0.13-eVote-diff.txt $(MAILMAN_INSTALL)/../mailman-eVote ;\
		cd $(MAILMAN_INSTALL)/../mailman-eVote; \
		patch -p0 < mailman-2.0.13-eVote-diff.txt ;\
		if [ -e $(MAILMAN_INSTALL)/../mailman-eVote/src/common.o ] ;\
			then \
			rm $(MAILMAN_INSTALL)/../mailman-eVote/src/*.o ;\
		fi ;\
		if [ -e $(MAILMAN_INSTALL)/../mailman-eVote/src/wrapper ] ;\
			then \
			rm $(MAILMAN_INSTALL)/../mailman-eVote/src/wrapper ;\
		fi ;\
	fi
	@echo "----> Preparing $(EVOTE_BIN)/eVote.cf in $(EVOTE_HOME_DIR)/eVote/eVote.cf"
	$(ECHO) "EVOTE_HOME_DIR = $(EVOTE_HOME_DIR)" > $(EVOTE_HOME_DIR)/eVote/eVote.cf
	$(ECHO) "EVOTE_BIN = $(EVOTE_BIN)" >> $(EVOTE_HOME_DIR)/eVote/eVote.cf
	$(ECHO) "TIME_OUT = 5" >> $(EVOTE_HOME_DIR)/eVote/eVote.cf
	$(ECHO) "IPCS = ipcs -c " >> $(EVOTE_HOME_DIR)/eVote/eVote.cf
	$(ECHO) "WHEREAMI = $(WHEREAMI)" >> $(EVOTE_HOME_DIR)/eVote/eVote.cf
	$(ECHO) "LISTDIR = $(LISTDIR)" >> $(EVOTE_HOME_DIR)/eVote/eVote.cf
	$(ECHO) "EVOTE_MAIL_TO = $(EVOTE_MAIL_TO)@$(WHEREAMI)" >> $(EVOTE_HOME_DIR)/eVote/eVote.cf
	$(ECHO) "MAILER = $(MAILER)" >> $(EVOTE_HOME_DIR)/eVote/eVote.cf
	$(ECHO) "PROGS = $(PROGS)" >> $(EVOTE_HOME_DIR)/eVote/eVote.cf
	$(IF) [ $(WHO) == root ] ;\
		then \
		chown -R $(LOGIN).$(GROUP) $(EVOTE_HOME_DIR)/eVote ;\
	else \
		touch ./root.commands ;\
		rm ./root.commands ;\
		if [ ! $(WHO) = $(LOGIN) ] ;\
			then \
			echo "chown -R $(LOGIN).$(GROUP) $(EVOTE_HOME_DIR)/eVote" > ./root.commands  ;\
		fi ;\
	fi 
	@echo "----> Setting up $(EVOTE_HOME_DIR)/eVote/data" 
	$(IF) [ ! -e $(EVOTE_HOME_DIR)/eVote/data ] ;\
		then \
		mkdir $(EVOTE_HOME_DIR)/eVote/data ;\
		chmod a-rwx,u+rwx $(EVOTE_HOME_DIR)/eVote/data ;\
	fi 
	$(ECHO) "----> Changing modes"
	$(CHMOD) -R $(DIR_PERMS) $(EVOTE_HOME_DIR)/eVote
	$(CHMOD) 644 $(EVOTE_HOME_DIR)/eVote/eVote.cf
	$(CHMOD) $(FILE_PERMS) $(EVOTE_HOME_DIR)/eVote/src/Clerk/*
	$(IF) [ -e $(EVOTE_HOME_DIR)/eVote/src/Clerk/CVS ] ;\
		then \
		chmod $(DIR_PERMS) $(EVOTE_HOME_DIR)/eVote/src/Clerk/CVS ;\
	fi
	$(CHMOD) ug+xrw,o-wx $(EVOTE_HOME_DIR)/eVote/src/Clerk/eVote_Clerk
	$(CHMOD) $(FILE_PERMS) $(EVOTE_HOME_DIR)/eVote/src/Clerklib/*
	$(IF) [ -e $(EVOTE_HOME_DIR)/eVote/src/Clerklib/CVS ] ;\
		then \
		chmod $(DIR_PERMS) $(EVOTE_HOME_DIR)/eVote/src/Clerklib/CVS ;\
	fi
	$(CHMOD) $(FILE_PERMS) $(EVOTE_HOME_DIR)/eVote/src/demo/eVoteui/*
	$(IF) [ -e $(EVOTE_HOME_DIR)/eVote/src/demo/eVoteui/CVS ] ;\
		then \
		chmod $(DIR_PERMS) $(EVOTE_HOME_DIR)/eVote/src/demo/eVoteui/CVS ;\
	fi
	$(CHMOD) $(FILE_PERMS) $(EVOTE_HOME_DIR)/eVote/src/demo/*
	$(IF) [ -e $(EVOTE_HOME_DIR)/eVote/src/demo/CVS ] ;\
		then \
		chmod $(DIR_PERMS) $(EVOTE_HOME_DIR)/eVote/src/demo/CVS ;\
	fi
	$(CHMOD) a+xr,a-w,u+w $(EVOTE_HOME_DIR)/eVote/src/demo/eVote
	$(CHMOD) $(DIR_PERMS) $(EVOTE_HOME_DIR)/eVote/src/demo/eVoteui
	$(IF) [ -e $(EVOTE_HOME_DIR)/eVote/src/port ] ;\
		then \
		chmod $(FILE_PERMS) $(EVOTE_HOME_DIR)/eVote/src/port/* ;\
	fi
	$(IF) [ -e $(EVOTE_HOME_DIR)/eVote/src/port/CVS ] ;\
		then \
		chmod $(DIR_PERMS) $(EVOTE_HOME_DIR)/eVote/src/port/CVS ;\
	fi
	$(IF) [ -e $(EVOTE_HOME_DIR)/eVote/src/test/lists ] ;\
		then \
		chmod $(FILE_PERMS) $(EVOTE_HOME_DIR)/eVote/src/mail/test/lists/* ;\
	fi
	$(IF) [ -e $(EVOTE_HOME_DIR)/eVote/src/mail/test/lists/CVS ] ;\
		then \
		chmod $(DIR_PERMS) $(EVOTE_HOME_DIR)/eVote/src/mail/test/lists/CVS ;\
	fi
	$(IF) [ -e $(EVOTE_HOME_DIR)/eVote/src/mail/test/big ] ;\
		then \
		chmod $(FILE_PERMS) $(EVOTE_HOME_DIR)/eVote/src/mail/test/* ;\
	fi
	$(IF) [ -e $(EVOTE_HOME_DIR)/eVote/src/mail/test/CVS ] ;\
		then \
		chmod $(DIR_PERMS) $(EVOTE_HOME_DIR)/eVote/src/mail/test/CVS ;\
	fi
	$(IF) [ -e $(EVOTE_HOME_DIR)/eVote/src/mail/test/lists ] ;\
		then \
		chmod $(DIR_PERMS) $(EVOTE_HOME_DIR)/eVote/src/mail/test/lists ;\
	fi
	$(IF) [ -e $(EVOTE_HOME_DIR)/eVote/src/mail/test ] ;\
		then \
		cd $(EVOTE_HOME_DIR)/eVote/src/mail/test;make config ;\
	fi
	$(CHMOD) $(FILE_PERMS) $(EVOTE_HOME_DIR)/eVote/src/mail/majordomo/*
	$(IF) [ -e $(EVOTE_HOME_DIR)/eVote/src/mail/majordomo/CVS ] ;\
		then \
		chmod $(DIR_PERMS) $(EVOTE_HOME_DIR)/eVote/src/mail/majordomo/CVS ;\
	fi
	$(CHMOD) $(FILE_PERMS) $(EVOTE_HOME_DIR)/eVote/src/mail/mailman/bin/*
	$(IF) [ -e $(EVOTE_HOME_DIR)/eVote/src/mail/mailman/bin/CVS ] ;\
		then \
		chmod $(DIR_PERMS) $(EVOTE_HOME_DIR)/eVote/src/mail/mailman/bin/CVS ;\
	fi
	$(CHMOD) $(FILE_PERMS) $(EVOTE_HOME_DIR)/eVote/src/mail/mailman/templates/*
	$(IF) [ -e $(EVOTE_HOME_DIR)/eVote/src/mail/mailman/templates/CVS ] ;\
		then \
		chmod $(DIR_PERMS) $(EVOTE_HOME_DIR)/eVote/src/mail/mailman/templates/CVS ;\
	fi
	$(CHMOD) $(FILE_PERMS) $(EVOTE_HOME_DIR)/eVote/src/mail/mailman/*
	$(IF) [ -e $(EVOTE_HOME_DIR)/eVote/src/mail/mailman/CVS ] ;\
		then \
		chmod $(DIR_PERMS) $(EVOTE_HOME_DIR)/eVote/src/mail/mailman/CVS ;\
	fi
	$(CHMOD) $(FILE_PERMS) $(EVOTE_HOME_DIR)/eVote/src/mail/mailui/*
	$(IF) [ -e $(EVOTE_HOME_DIR)/eVote/src/mail/mailui/CVS ] ;\
		then \
		chmod $(DIR_PERMS) $(EVOTE_HOME_DIR)/eVote/src/mail/mailui/CVS ;\
	fi
	$(CHMOD) $(FILE_PERMS) $(EVOTE_HOME_DIR)/eVote/src/mail/message/*
	$(IF) [ -e $(EVOTE_HOME_DIR)/eVote/src/mail/message/CVS ] ;\
		then \
		chmod $(DIR_PERMS) $(EVOTE_HOME_DIR)/eVote/src/mail/message/CVS ;\
	fi
	$(CHMOD) $(FILE_PERMS) $(EVOTE_HOME_DIR)/eVote/src/mail/*
	$(IF) [ -e $(EVOTE_HOME_DIR)/eVote/src/mail/CVS ] ;\
		then \
		chmod $(DIR_PERMS) $(EVOTE_HOME_DIR)/eVote/src/mail/CVS ;\
	fi
	$(CHMOD) $(DIR_PERMS) $(EVOTE_HOME_DIR)/eVote/src/mail/majordomo
	$(CHMOD) $(DIR_PERMS) $(EVOTE_HOME_DIR)/eVote/src/mail/mailman
	$(CHMOD) $(DIR_PERMS) $(EVOTE_HOME_DIR)/eVote/src/mail/mailman/templates
	$(CHMOD) $(DIR_PERMS) $(EVOTE_HOME_DIR)/eVote/src/mail/mailman/bin
	$(CHMOD) $(DIR_PERMS) $(EVOTE_HOME_DIR)/eVote/src/mail/mailui
	$(CHMOD) $(DIR_PERMS) $(EVOTE_HOME_DIR)/eVote/src/mail/message
	$(IF) [ -e $(EVOTE_HOME_DIR)/eVote/src/mail/test ] ;\
		then \
		chmod $(DIR_PERMS) $(EVOTE_HOME_DIR)/eVote/src/mail/test ;\
		chmod $(DIR_PERMS) $(EVOTE_HOME_DIR)/eVote/src/mail/test/lists;\
	fi
	$(CHMOD) $(FILE_PERMS) $(EVOTE_HOME_DIR)/eVote/src/tools/filters/*
	$(IF) [ -e $(EVOTE_HOME_DIR)/eVote/src/tools/filters/CVS ] ;\
		then \
		chmod $(DIR_PERMS) $(EVOTE_HOME_DIR)/eVote/src/tools/filters/CVS ;\
	fi
	$(CHMOD) $(FILE_PERMS) $(EVOTE_HOME_DIR)/eVote/src/tools/trans/*
	$(IF) [ -e $(EVOTE_HOME_DIR)/eVote/src/tools/trans/CVS ] ;\
		then \
		chmod $(DIR_PERMS) $(EVOTE_HOME_DIR)/eVote/src/tools/trans/CVS ;\
	fi
	$(CHMOD) $(FILE_PERMS) $(EVOTE_HOME_DIR)/eVote/src/tools/utils/*
	$(IF) [ -e $(EVOTE_HOME_DIR)/eVote/src/tools/utils/CVS ] ;\
		then \
		chmod $(DIR_PERMS) $(EVOTE_HOME_DIR)/eVote/src/tools/utils/CVS ;\
	fi
	$(CHMOD) $(FILE_PERMS) $(EVOTE_HOME_DIR)/eVote/src/tools/README
	$(IF) [ $(WHO) = root -o $(WHO) = $(LOGIN) ] ;\
		then \
		echo "----> Setting up $(LISTDIR), and poll directory." ;\
		if [ ! -e $(LISTDIR) -a $(LISTSERVER) = "mailman" ] ;\
			then \
			mkdir $(LISTDIR) ; \
		fi ;\
		if [ ! -e $(LISTDIR)/../polls ] ;\
			then \
			mkdir $(LISTDIR)/../polls ; \
		fi ;\
		cp ../doc/eVote.petition $(LISTDIR) ;\
		cp ../doc/eVote.poll $(LISTDIR) ;\
		cp ../doc/eVote.help $(LISTDIR) ;\
		cp ../doc/eVote_sample.info $(LISTDIR) ;\
		cp ../doc/eVote.list-owner-info $(LISTDIR) ;\
		chmod 00770 $(LISTDIR) ;\
		chmod 00770 $(LISTDIR)/../polls ;\
		chmod $(FILE_PERMS) $(LISTDIR)/eVote* ;\
		chown $(LOGIN).$(GROUP) $(LISTDIR) ;\
		chown $(LOGIN).$(GROUP) $(LISTDIR)/../polls ;\
		chown $(LOGIN).$(GROUP) $(LISTDIR)/eVote* ;\
		echo "----> Placing eVote in $(UI_BIN)" ;\
		mv mail/eVote_insert $(UI_BIN) ;\
		mv mail/eVote_petition $(UI_BIN) ;\
		if [ $(LISTSERVER) = "mailman" ] ;\
			then \
			ln -s $(EVOTE_BIN)/eVote.cf $(UI_BIN) ;\
			mv mail/eVote_mail $(UI_BIN)/../bin ;\
			cp mail/mailman/bin/eVote_queries.py $(UI_BIN)/../bin ;\
			cp mail/mailman/bin/* $(UI_BIN)/../bin ;\
			cp mail/mailman/templates/* $(UI_BIN)/../templates ;\
			chmod o-wx,ug+rx $(UI_BIN)/../bin/eVote* ;\
			chown $(LOGIN).$(GROUP) $(UI_BIN)/../bin/eVote* ;\
			chmod a+r,u+w,a-x $(UI_BIN)/../templates/eVote* ;\
			chown $(LOGIN).$(GROUP) $(UI_BIN)/../templates/eVote* ;\
			chown -R $(LOGIN).$(GROUP) $(MAILMAN_INSTALL)/../mailman-eVote; \
			cd $(MAILMAN_INSTALL)/../mailman-eVote; \
			if [ $(WHO) = root ] ;\
				then \
				su -c "make install" $(LOGIN); \
			else \
				make install; \
			fi ;\
		else \
			mv mail/eVote_mail $(UI_BIN) ;\
		fi ;\
		chmod o-wx,ug+rx $(UI_BIN)/eVote_insert ;\
		chmod o-wx,ug+rx $(UI_BIN)/eVote_petition ;\
		chown $(LOGIN).$(GROUP) $(UI_BIN)/eVote* ;\
	fi
	$(IF) [ $(WHO) = root ] ;\
		then \
		echo "----> Placing executables in $(EVOTE_BIN)" ;\
		mv Clerk/eVote_Clerk $(EVOTE_BIN) ;\
		mv demo/eVote $(EVOTE_BIN) ;\
		if [ -e $(EVOTE_BIN)/eVote.cf ] ;\
			then \
			rm $(EVOTE_HOME_DIR)/eVote/eVote.cf ;\
		else \
			mv $(EVOTE_HOME_DIR)/eVote/eVote.cf $(EVOTE_BIN) ;\
		fi ;\
		if [ $(TEST) = YES -a -e $(EVOTE_HOME_DIR)/eVote/src/mail/test/cscratch ] ;\
			then \
			cp $(EVOTE_HOME_DIR)/eVote/src/mail/test/cscratch $(EVOTE_BIN)/eVote_cscratch ;\
			cp $(EVOTE_HOME_DIR)/eVote/src/mail/test/csetup $(EVOTE_BIN)/eVote_csetup ;\
			cp $(EVOTE_HOME_DIR)/eVote/src/mail/test/cres $(EVOTE_BIN)/eVote_cres ;\
		fi ;\
		echo "----> Changing ownerships to $(CLERK)" ;\
		chown $(CLERK) $(EVOTE_BIN)/eVote* ;\
		chown -R $(CLERK) $(EVOTE_HOME_DIR)/eVote/data ;\
		chown -R $(CLERK) $(EVOTE_HOME_DIR)/eVote/src/Clerk ;\
		chown -R $(CLERK) $(EVOTE_HOME_DIR)/eVote/src/demo ;\
		if [ -e $(EVOTE_HOME_DIR)/eVote/src/port/CVS ] ;\
			then \
			chown -R $(CLERK) $(EVOTE_HOME_DIR)/eVote/src/port ;\
		fi ;\
	fi
	$(IF) [ ! $(WHO) = root ] ;\
		then \
		echo "----> Fixing up your ./root.commands" ;\
		echo "mv Clerk/eVote_Clerk $(EVOTE_BIN)" >> ./root.commands ;\
		echo "mv demo/eVote $(EVOTE_BIN)"  >> ./root.commands ;\
		if [ -e $(EVOTE_BIN)/eVote.cf ] ;\
			then \
			rm $(EVOTE_HOME_DIR)/eVote/eVote.cf ;\
		else \
			echo "mv $(EVOTE_HOME_DIR)/eVote/eVote.cf $(EVOTE_BIN)" >> ./root.commands ;\
		fi ;\
		if [ $(TEST) = YES -a -e $(EVOTE_HOME_DIR)/eVote/src/mail/test/cscratch ] ;\
			then \
			echo "cp $(EVOTE_HOME_DIR)/eVote/src/mail/test/cscratch $(EVOTE_BIN)/eVote_cscratch"  >> ./root.commands ;\
			echo "cp $(EVOTE_HOME_DIR)/eVote/src/mail/test/csetup $(EVOTE_BIN)/eVote_csetup"  >> ./root.commands ;\
			echo "cp $(EVOTE_HOME_DIR)/eVote/src/mail/test/cres $(EVOTE_BIN)/eVote_cres"  >> ./root.commands ;\
		fi ;\
		echo "chown $(CLERK) $(EVOTE_BIN)/eVote*"  >> ./root.commands ;\
		echo "chown -R $(CLERK) $(EVOTE_HOME_DIR)/eVote/data"  >> ./root.commands ;\
		echo "chown -R $(CLERK) $(EVOTE_HOME_DIR)/eVote/src/Clerk"  >> ./root.commands ;\
		echo "chown -R $(CLERK) $(EVOTE_HOME_DIR)/eVote/src/demo"  >> ./root.commands ;\
		if [ -e $(EVOTE_HOME_DIR)/eVote/src/port ] ;\
			then \
			echo "chown -R $(CLERK) $(EVOTE_HOME_DIR)/eVote/src/port"  >> ./root.commands ;\
		fi ;\
	fi 
	$(IF) [ ! $(WHO) = $(LOGIN) -a ! $(WHO) = root ] ;\
		then \
		if [ ! -e $(LISTDIR) ] ;\
			then \
			echo "mkdir $(LISTDIR)"  >> ./root.commands ; \
		fi ;\
		if [ ! -e $(LISTDIR)/../polls ] ;\
			then \
			echo "mkdir $(LISTDIR)/../polls " >> ./root.commands ; \
		fi ;\
		echo "cp ../doc/eVote.petition $(LISTDIR)"  >> ./root.commands ;\
		echo "cp ../doc/eVote.poll $(LISTDIR)"  >> ./root.commands ;\
		echo "cp ../doc/eVote.help $(LISTDIR)"  >> ./root.commands ;\
		echo "cp ../doc/eVote_sample.info $(LISTDIR)"  >> ./root.commands ;\
		echo "cp ../doc/eVote.list-owner-info $(LISTDIR)"  >> ./root.commands ;\
		echo "chmod 00770 $(LISTDIR)"  >> ./root.commands ;\
		echo "chmod 00770 $(LISTDIR)/../polls"  >> ./root.commands ;\
		echo "chmod $(FILE_PERMS) $(LISTDIR)/eVote*"  >> ./root.commands ;\
		echo "chown $(LOGIN).$(GROUP) $(LISTDIR)"  >> ./root.commands ;\
		echo "chown $(LOGIN).$(GROUP) $(LISTDIR)/../polls"  >> ./root.commands ;\
		echo "chown $(LOGIN).$(GROUP) $(LISTDIR)/eVote*"  >> ./root.commands ;\
		echo "mv mail/eVote_insert $(UI_BIN)"  >> ./root.commands ;\
		echo "mv mail/eVote_petition $(UI_BIN)"  >> ./root.commands ;\
		if [ $(LISTSERVER) = "mailman" ] ;\
			then \
			echo "mv mail/eVote_mail $(UI_BIN)/../bin"  >> ./root.commands ;\
			echo "ln -s $(EVOTE_BIN)/eVote.cf $(UI_BIN)" >> ./root.commands ;\
			echo "cp mail/mailman/bin/* $(UI_BIN)/../bin " >> ./root.commands ;\
			echo "cp mail/mailman/templates/* $(UI_BIN)/../templates " >> ./root.commands ;\
			echo "chmod o-wx,ug+rx $(UI_BIN)/../bin/eVote* " >> ./root.commands ;\
			echo "chown $(LOGIN).$(GROUP) $(UI_BIN)/../bin/eVote_* " >> ./root.commands ;\
			echo "chmod a+r,u+w,a-x $(UI_BIN)/../templates/eVote* " >> ./root.commands ;\
			echo "chown $(LOGIN).$(GROUP) $(UI_BIN)/../templates/eVote* " >> ./root.commands ;\
			echo "chown -R $(LOGIN).$(GROUP) $(MAILMAN_INSTALL)/../mailman-eVote" >> ./root.commands ; \
			echo "cd $(MAILMAN_INSTALL)/../mailman-eVote " >> ./root.commands ;\
			echo "make install" >> ./root.commands ;\
		else \
			echo "mv mail/eVote_mail $(UI_BIN)"  >> ./root.commands ;\
		fi ;\
		echo "chmod o-wx,ug+rx $(UI_BIN)/eVote_petition"  >> ./root.commands ;\
		echo "chmod o-wx,ug+rx $(UI_BIN)/eVote_insert"  >> ./root.commands ;\
		echo "chown $(LOGIN).$(GROUP) $(UI_BIN)/eVote*"  >> ./root.commands ;\
	fi
	$(IF) [ ! $(WHO) = root -a -s ./root.commands ] ;\
		then \
		chmod u+x ./root.commands ;\
		echo "" ;\
		echo " EVERYTHING IS DONE THAT CAN BE DONE FROM THE LOGIN $(WHO)" ;\
		echo "" ;\
		echo "" ;\
		echo "" ;\
		echo " - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -" ;\
		echo "Please su to root to perform the following:" ;\
		echo " - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -" ;\
		cat root.commands ;\
		echo " - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -" ;\
		echo "End of root commands.  These commands are in the file \"./root.commands\" " ;\
		echo "So, instead, you can do:  su -c ./root.commands root" ;\
		echo " - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -" ;\
	fi

clobber: clean remove_eVote 

remove_eVote:  
	@echo "----> Removing eVote"
	$(IF) [ $(WHO) = root -o $(WHO) = $(LOGIN) ] ;\
		then \
		if [ -e $(LISTDIR)/../polls ] ;\
			then \
			rm -r $(LISTDIR)/../polls ;\
		fi ;\
		if [ -e  $(LISTDIR)/eVote.petition ] ;\
			then \
			rm $(LISTDIR)/eVote* ;\
		fi ;\
		if [ -e $(UI_BIN)/eVote_insert ] ;\
			then \
			rm $(UI_BIN)/eVote_insert ;\
			rm $(UI_BIN)/eVote_petition ;\
			rm $(UI_BIN)/eVote_mail ;\
		fi ;\
		if [ -e $(MAILMAN_INSTALL)/../mailman-eVote ] ;\
			then \
			rm -r $(MAILMAN_INSTALL)/../mailman-eVote ;\
		fi ;\
		if [ $(LISTSERVER) = "mailman" ] ;\
			then \
			if [ -e $(LISTDIR) ] ;\
				then \
				rm -r $(LISTDIR) ;\
			fi ;\
			if [ -e  $(UI_BIN)/../bin/eVote_queries.py ] ;\
				then \
				chmod u+w $(UI_BIN)/../bin/eVote* ; \
				rm $(UI_BIN)/../bin/eVote* ; \
			fi ;\
			if [ -e  $(UI_BIN)/../templates/eVote.txt ] ;\
				then \
				chmod u+w $(UI_BIN)/../templates/eVote* ; \
				rm $(UI_BIN)/../templates/eVote* ; \
			fi ;\
			cd $(MAILMAN_INSTALL); make install ;\
		fi ;\
	fi 
	$(IF) [ $(WHO) = root ] ;\
		then \
		if [ -e ../data/nohup.out ] ;\
			then \
			chmod u+w ../data/nohup.out ;\
			rm ../data/nohup.out ;\
		fi ;\
		if [ -e $(EVOTE_BIN)/eVote.cf ] ;\
			then \
			chmod u+w $(EVOTE_BIN)/eVote* ;\
			rm $(EVOTE_BIN)/eVote* ;\
		fi ;\
		if [ ! $(EVOTE_HOME_DIR)/eVote/src = `pwd` -a -e $(EVOTE_HOME_DIR)/eVote ] ;\
			then \
			chmod -R u+w $(EVOTE_HOME_DIR)/eVote ;\
			rm -r $(EVOTE_HOME_DIR)/eVote ;\
		fi ;\
	else \
		touch ./root.commands ;\
		if [ -e ../data/nohup.out ] ;\
			then \
			echo "chmod u+w ../data/nohup.out" >> ./root.commands ;\
			echo "rm ../data/nohup.out" >> ./root.commands ;\
		fi ;\
		if [ -e $(EVOTE_BIN)/eVote.cf ] ;\
			then \
			echo "chmod u+w $(EVOTE_BIN)/eVote*" >> ./root.commands ;\
			echo "rm $(EVOTE_BIN)/eVote*" >> ./root.commands ;\
		fi ;\
		if [ ! $(EVOTE_HOME_DIR)/eVote/src = `pwd` -a -e $(EVOTE_HOME_DIR)/eVote ] ;\
			then \
			echo "rm -r $(EVOTE_HOME_DIR)/eVote"  >> ./root.commands ;\
		fi ;\
		if [ ! $(WHO) = $(LOGIN) ] ;\
			then \
			if [ -e $(LISTDIR)/../polls ] ;\
				then \
				echo "rm -r $(LISTDIR)/../polls"  >> ./root.commands ;\
			fi ;\
			if [ -e  $(LISTDIR)/eVote.petition ] ;\
				then \
				echo "rm $(LISTDIR)/eVote*"  >> ./root.commands ;\
			fi ;\
			if [ -e $(UI_BIN)/eVote_insert ] ;\
				then \
				echo "rm $(UI_BIN)/eVote*"  >> ./root.commands ;\
			fi ;\
			if [ -e $(MAILMAN_INSTALL)/../mailman-eVote ] ;\
				then \
				echo "rm -r $(MAILMAN_INSTALL)/../mailman-eVote"  >> ./root.commands ;\
			fi ;\
			if [ $(LISTSERVER) = "mailman" ] ;\
				then \
				if [ -e $(LISTDIR) ] ;\
					then \
					echo "rm -r $(LISTDIR)"  >> ./root.commands ;\
				fi ;\
				if [ -e  $(UI_BIN)/../bin/eVote_queries.py ] ;\
					then \
					echo "chmod u+w $(UI_BIN)/../bin/eVote* " >> ./root.commands ; \
					echo "rm $(UI_BIN)/../bin/eVote* " >> ./root.commands ; \
				fi ;\
				if [ -e  $(UI_BIN)/../templates/eVote.txt ] ;\
					then \
					echo "chmod u+w $(UI_BIN)/../templates/eVote* " >> ./root.commands ; \
					echo "rm $(UI_BIN)/../templates/eVote* " >> ./root.commands ; \
				fi ;\
				echo "cd $(MAILMAN_INSTALL);make install"  >> ./root.commands ;\
			fi ;\
		fi ;\
		if [ -s ./root.commands ] ;\
			then \
			chmod u+x ./root.commands ;\
			echo "" ;\
			echo " EVERYTHING IS DONE THAT CAN BE DONE FROM THE LOGIN $(WHO)" ;\
			echo "" ;\
			echo "" ;\
			echo "" ;\
			echo " - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -" ;\
			echo "Please su to root to perform the following:" ;\
			echo " - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -" ;\
			cat root.commands ;\
			echo " - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -" ;\
			echo "End of root commands.  These commands are in the file \"./root.commands\" " ;\
			echo "So, instead, you can do:  su -c ./root.commands root" ;\
			echo " - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -" ;\
		fi ;\
	fi
clean:
	@echo "----> Cleaning sources"
	@touch ./root.commands 
	@rm ./root.commands 
	$(CD) Clerk; ${MAKE} clean
	$(CD) Clerklib; ${MAKE} clean
	$(IF) [ $(TEST) = YES -a -e mail/test ] ;\
		then \
		if [ $(WHO) = root -o $(WHO) = $(LOGIN) ] ;\
			then \
			cd mail/test; ${MAKE} clean LISTDIR=$(LISTDIR) ;\
		else \
			echo "cd mail/test; ${MAKE} clean LISTDIR=$(LISTDIR)" >> ./root.commands ;\
		fi ;\
	fi
	$(CD) mail; ${MAKE} clean
	$(CD) demo; ${MAKE} clean

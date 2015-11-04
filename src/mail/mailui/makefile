# $Id: makefile,v 1.5 2003/01/25 22:04:35 marilyndavis Exp $ 
# makefile for ../src/mail/mailui
#########################################################
##########################################################/

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

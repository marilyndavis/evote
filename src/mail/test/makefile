# $Id: makefile,v 1.8 2003/10/20 19:41:41 marilyndavis Exp $ 
CC = gcc 
CFLAGS = -g -c -Wall
LDFLAGS = 
LISTDIR = /home/majordomo/lists
LD = @gcc 
CHMOD = @chmod
TOUCH = @touch
all: resend_test smail_test config

config: smail_test resend_test
	${CHMOD} u+x smail_test
	${CHMOD} u+x resend_test
	${CHMOD} u+x cres 
	${CHMOD} u+x cscratch 
	${CHMOD} u+x csetup 
	${CHMOD} u+x big
	${CHMOD} u+x con 
	${CHMOD} u+x drops 
	${CHMOD} u+x kop 
	${CHMOD} u+x kop2 
	${CHMOD} u+x mex 
	${CHMOD} u+x mclean
	${CHMOD} u+x mres 
	${CHMOD} u+x mscratch 
	${CHMOD} u+x msetup 
	${CHMOD} u+x par 
	${CHMOD} u+x pet 
	${CHMOD} u+x pet2 
	${CHMOD} u+x pet3 
	${CHMOD} u+x pet4 
	${CHMOD} u+x vacation
	${TOUCH} tmp1
	@if [ ! -e tmp ] ; \
		then \
		mkdir tmp ; \
	fi 
	@if [ ! -e bugdata ] ; \
		then \
		mkdir bugdata ; \
	fi 
	${CHMOD} u+rwx bugdata
	${CHMOD} u+rwx tmp
	${TOUCH} tmp/t.out
	${TOUCH} tmp/r.out
	${CHMOD} u+x *.pl

smail_test: smail_test.o 
	${LD} ${LDFLAGS} -o smail_test smail_test.o

resend_test: resend_test.o
	${LD} ${LDFLAGS} -o ./resend_test resend_test.o
clean:
	${TOUCH} resend_test.o
	@rm *.o
	${TOUCH} tmp1
	@rm tmp?*
	${TOUCH} t3
	@rm t3
	@if [ -e bugdata ] ; \
		then \
		chmod u+x bugdata;\
	fi
	${CHMOD} a+x mclean
	@./mclean ${LISTDIR}

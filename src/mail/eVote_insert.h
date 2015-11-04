/* $Id: eVote_insert.h,v 1.4 2003/01/15 18:54:08 marilyndavis Exp $ */ 
/**********************************************************
 *  eVote_insert.h  -  header for eVote_insert.c
 *********************************************************
 **********************************************************/
#include "mailui/maildef.h"
/* These are the only things needed in this directory */
extern char token[];     /* ./message/token.c */
extern char error_msg[]; /* ../tools/filters/filter.c */
extern char *from;       /* ../tools/filters/filter.c */
extern char *list;       /* ./mailui/listm.c */
                    /* cf.c */
extern char * whereami;         /* host name */
extern char * mailer;           /* mail command */
extern char * subject;          /* message subject - 
				   ../tools/filters/filter.c   */
/*  function declarations */
void dump_message(WHAT, YESorNO strip_subject); /* ../tools/filters/filter.c*/
int parse_header(int argc, char *argv[]);  /* ./message/in_message.c */
int get_token(void);                           /* ./message/token.c */
int save_token(int *);
int parse_cf();                            /* ./cf.c       */
void send(int towhom);                    /* ./message/send.c */
void subs(int argc, char** argv);         /* ./list.c         */
void do_eVote(int cc);                    /* ./mailui/do_eVote.c    */
YESorNO same(char*, char*);                /* ../tools/filters/filter.c */

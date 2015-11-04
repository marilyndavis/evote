/* * eVote - Software for online consensus development.
 * Copyright (C) 2015 Marilyn Davis <marilyn@deliberate.com>
 *
 * This file is part of eVote.
 *
 * eVote is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as
 * published by the Free Software Foundation, either version 3 of the
 * License, or (at your option) any later version.
 *
 * eVote is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with eVote.  If not, see <http://www.gnu.org/licenses/>.
 */

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

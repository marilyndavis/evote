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

/* $Id: message.h,v 1.5 2003/10/20 17:07:34 marilyndavis Exp $ */ 
/************************************************************
 *   message.h    
 *                has modules that relate to reading and writing
 *                the messages.  It has the tokenizer.  It depends
 *                heavily on the filter facility:
 *                EVOTE_HOME_DIR/eVote/src/tools/filters
 *********************************************************
 **********************************************************/
#ifdef TRACE
extern FILE* fptrace;  /* in eVote_petition.c */
#endif
#include "../../tools/filters/filter.h"
/* ../../tools/filters/filter.c */
/* ./in_message.c */
extern char **argv_copy;  /* used by send.c only */
extern int argc_copy;
extern char **env_copy; /* used by mailman's ../list.c */
extern char * petition_address;
extern int end_mark;
extern int sig_start;
int parse_headers(int argc, char* argv[], char *env[]);
void set_sig_start(int cc);
/* out_message.c */
extern YESorNO header_done;
void big_finish(int exit_no);
void bounce_error(int);
void finish(int exit_code);
void gen_header(int whom, const char * subject_prefix, YESorNO show_date);
/* send.c */
extern char * mailer;
void send(int whom);       
void transmit(char * address);
/* token.c */
extern char token[];        
extern int tokens_read_to;
int back_one_token(void);
void block_token(void);
int fix_first_token(void);
int get_token(void);              
void mark_token(void);
void print_first_line(void);
void print_tokens(YESorNO one_line_only);
void read_to_end(void);  
char * save_token(int *);
void write_from_mark(FILE *fp, YESorNO drop_last_token);

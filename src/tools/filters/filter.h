/* $Id: filter.h,v 1.5 2003/10/20 17:07:34 marilyndavis Exp $ */ 
/************************************************************
 *     filter.h
 *     Accompanies filter.c
 *********************************************************
 *    Copyright (c) 1994...2015 Deliberate.com Patented.
 *    by Marilyn Davis
 **********************************************************/
#ifndef _filter_h
#define _filter_h
#include<errno.h>
#include<limits.h>
#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include<sys/types.h>
#include<time.h>
#include<unistd.h>
#define SAME_LETTER(x,y) (x==y ? 1 : \
    x >= 'a' && x <= 'z' && y >= 'A' && y <= 'Z' && x + 'A' - 'a' == y ? 1 : \
    x >= 'A' && x <= 'Z' && y >= 'a' && y <= 'z' && x - 'A' + 'a' == y ? 1 : 0)
#define IS_WHITE(ch) (ch == ' ' || ch == '\t' || ch == '\n'? 1 : 0)
#define TOUPPER(X) (((X) <= 'z'&& (X) >= 'a') ? (X) + 'A' - 'a' : (X))
#ifndef Clerkdef_h 
typedef enum {NOT_OK, OK, UNDECIDED, PROBLEM, STOP, CANT} OKorNOT;
typedef enum {NO, YES, MAYBE, PUNT} YESorNO;
#endif
void alert(char *msg, YESorNO log_error_msg);
char * copy_line(int start, int * ends);
char * date_str(time_t when);
void dump_bad(void);
void dump_message(FILE * fp, YESorNO skip_headers, 
		  YESorNO skip_first_token, YESorNO provide_intro);
YESorNO fork_is_parent(void);
char * get_first_token(void);
char * get_guy(int start, int * ends);
char * keep_string(char*);
void look_for_trouble(int argc, char *argv[], char *env[]);
void mail_it(char * to_address, char * from_address, char * subject);
void parse_message(YESorNO for_filter);
void read_message(int argc, char *argv[], char *env[]);
YESorNO same(const char*, const char*);
YESorNO same_address(const char *one, const char* two);
YESorNO samen(const char*, const char*, int);
char * set_signal_name(int signo);
void set_signals(void (*fn)(int));
#define MAX_ERROR   500  /* maximum error message size */
/* buffer */
extern char * buffer;
extern int buff_up(void);
extern int bytes;
extern char error_msg[];
extern char * alert_address;
extern char * alias_address;
extern char * cc_address;
extern int date_start;
extern char * first_token;
extern char * for_address;
extern char * from;
extern int from_start;
extern time_t now;
extern int msg_start;
extern char * program;
extern char * reply_to;
extern char * return_path;
extern char * sender;
extern char * to_address;
extern int to_start;
extern char * subject;
extern int time_out;
extern char * whereami;
extern char * eVote_mail_to;
#endif

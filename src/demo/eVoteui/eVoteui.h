/* $Id: eVoteui.h,v 1.3 2003/01/15 18:54:08 marilyndavis Exp $ */ 
/**********************************************************
 *   ../eVote/src/eVoteui/eVoteui.h 
 *********************************************************
 *    Copyright (c) 1994...2015 Deliberate.com Patented.
 *    by Marilyn Davis
 **********************************************************/
#include<string.h>
#include<stdlib.h>
/* Questions that eVote asks and that have a help string. */
/* These are for the structure at the beginning of 
   input.c. */
#define NO_QUESTIONS (22)
typedef enum 
{
  NO_Q,
  CHANGEIT,
  DROP_DAYS,
  DROPS_OK,
  GROUPIT,
  HOW_WHO,
  JOIN,
  NO_TOGETHER,
  OKAY,
  MAX,
  MIN,
  PRIV_IF,
  PRIV_HOW,
  REALLY_QUIT,
  SAME_LIMITS,
  SEE_IT,
  SUM_LIMIT,
  TITLER,
  VOTE,
  VOTED_ON,
  WHO_WHAT,
  YN_VOTE
} QUESTION;  
struct questions_def
{
  QUESTION qnumber;
  char * qprompt;
  char * qhelp;
};
/* for collect_eVote_details() and related do_xxx() functions */
typedef enum {GET, SHOW, CHANGE, FALL} MODE; 
/* other possible replies when we are expecting a positive number */
#define EQUIT		(-1)
#define EQUESTION	(-2)
#define ENOREPLY	(-3)
#include "../../Clerklib/Clerk.h"
#define PATHLEN (128)
/* global */
extern char current_conf[];  /* space in Clerklib/confl.c
				maintained in Clerklib/voterl.c */
extern short current_drop_days;  /* not used in the demo
				    but available.  
				    maintained by
				    Clerklib/ipc_shm.c */
/* shared memory items */
extern short *p_no_items;    /* maintained by */
extern ITEM_INFO *item_info; /* Clerklib/ipc_msg.c */
/*  Some forward external declarations of the low-level 
    routines in	input.c */
char eVote_askyn(QUESTION question);
short eVote_asknum(QUESTION question, short min, 
		   short max, YESorNO yesno);
/*  In ../eVote_io.c */
char *GetArg(char *prompt);
short make_dropping(unsigned long local_id);
short printing_id(short dropping_id);
char * print_title(short dropping_id);
/* calls in command.c  These are used by the eVote executable only. */
int command(int argc, char *argv[]);
YESorNO have_permission(YESorNO verbose);
char* print_version(void);

/* calls in ui.c */
void get_args(int argc, char* argv[], char* this_conf, short* drop_days);
void hello_Clerk(void);
/* calls in menu.c */
void eVote_menu(ITEM_INFO* p_item, YESorNO* voted_yet);
char * process_eVote(ITEM_INFO* p_item, 
		     char* input, 
		     YESorNO come_back);
/* calls in conf.c */
OKorNOT check_conf(char* conf_name);
char *collect_conf_name(char* input);
void correct_conf_name(char* new_conf_name);
OKorNOT eVote_conf(char * conf_name, short drop_days);
char *get_conf_list(YESorNO finished);
void list_confs(void);
/* calls in misc.c */
short check_atoi(char *str);
OKorNOT fix_case(char *change, char* keep, short max);
char *get_name(unsigned long uid);
/* calls in item.c */
void change_vstatus(ITEM_INFO *p_item, char ch, YESorNO testing);
short process_drops(short *start, short *end, YESorNO testing);
/* calls in voter.c */
OKorNOT drop_voter(char * conf_name, 
		   unsigned long uid_to_drop);
OKorNOT i_am_entering(char * conf_name, YESorNO testing);
OKorNOT process_vote(ITEM_INFO *p_item, char *input);
/* in queries.c */
void how_menu(ITEM_INFO *p_item);
short how_voted(ITEM_INFO *p_item, 
		char *pwname, unsigned long how_uid);
OKorNOT process_how(ITEM_INFO* p_item, char *input);
OKorNOT process_who(ITEM_INFO *p_item, char *input);
void who_menu(ITEM_INFO* p_item);
/* in explain.c */
void explain(char *inp);
#define NO_LINES 23
#define NO_COLS 77
#ifdef DEMO
/* in blurbs.c */
char* do_blurb(ITEM_INFO* p_item);
void do_blurbette(void);
void toggle_blurbs(void); /* in blurbs.c */
#endif

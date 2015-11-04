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

/* $Id: voterl.c,v 1.4 2003/01/15 18:54:08 marilyndavis Exp $ */ 
/**********************************************************
 *	 ../eVote/src/Clerklib/voterl.c
 *********************************************************
 **********************************************************/
#include <stdio.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <pwd.h>
#include "Clerkdef.h"
#include "Clerk.h"
#include "msgdef.h"
#include "ipc_msg.h"
char * message(void);  /* in ipc_msg.c */
void dump_stats(void); /* in stats.c */
void fix_sum(short dropping_id, short new_sum);  /* in stats.c */
char time_str[40] = "";
unsigned long no_of_participants;
unsigned long who_am_i(void);
unsigned long mail_voter = 0L;
unsigned long voter(void);
extern short *p_no_items;
extern ITEM_STAT *item_stat;
ACTION current_action;  /* used by voterm.c */
static YESorNO is_valid_email(char *);
/* The conf the user is currently in - available for export */
extern char  current_conf[];
/*  declarations for calls in ipc_shm */
void drop_info(void);
OKorNOT start_info(int new_mid);
#ifdef EDEBUG
void output(char *);
extern char debugger[200];
extern int edebug;
#define FLOWER 8
#define STRESS 1
time_t start_time;
#endif
/********************************************************
 *      new_action can be 
 *         EVERYTHING - the initial state where the
 *                      voter reads the mail list and votes.
 *         VACATION   - For a voter on vacation who doesn't
 *                      want the email but we keep the ballot
 *                      and don't allow mods.
 *         VOTE_ONLY  - For a voter who doesn't
 *                      want the email -- possibly insecure.
 *         READ_ONLY  - For an archive that doesn't have
 *                      voting privileges.
 *         LOCK       - Not droppable *once* so that 
 *                      majordomo's attempt to drop the
 *                      voter will fail once when he goes
 *                      on vacation.
 *        These can be combined:  (LOCK | VOTE_ONLY)
 *********************************************************/
OKorNOT 
change_action(ACTION new_action)
{
  out_msg_buffer->mtype = CHANGE_ACTION;
  sprintf(out_msg_buffer->mtext, FNE_CHANGE_ACTION, NNE_CHANGE_ACTION);
  if (send_inst(0, YES) != OK)
    return NOT_OK;
  switch (in_msg_buffer->mtype)
    {
    case NO_CHANGE:
      return UNDECIDED;
    case GOOD:
      current_action = new_action;
      return OK;
    }
  return UNDECIDED;
}
/********************************************************/
YESorNO 
does_uid_exist(unsigned long uid_to_check)
{
  out_msg_buffer->mtype = PR_UID_EXIST;
  sprintf(out_msg_buffer->mtext, FNE_UID_EXIST, NNE_UID_EXIST);
  if (send_inst(0, YES) != OK)
    return MAYBE;
  switch (in_msg_buffer->mtype)
    {
    case NOT_GOOD:
      return NO;
      break;
    case GOOD:
      return YES;
      break;
    default:
      fprintf(stderr,"\nUnrecognized char %ld returned by does_uid_exist.\n",
	      in_msg_buffer->mtype);
      return MAYBE;
    }
  return MAYBE;
}
/*******************************************************/
YESorNO 
have_i_read(short dropping_id)
{
  int i;
  char *get_stats(short dropping_id);
  if (dropping_id > *p_no_items )
    return MAYBE;
  if (item_stat[dropping_id].text[0] == '\0' &&
      strcmp(get_stats(dropping_id), "Not Found!  ") == 0)
    return MAYBE;
  if (strcmp(item_stat[dropping_id].vote_str,"!ACC") == 0)
    return NO;
  for (i = 2; i < 5; i++)
    if (item_stat[dropping_id].vote_str[i] == '*')
      return NO;
  return YES;
}
/*******************************************************/
YESorNO 
have_i_voted(ITEM_INFO* p_item)
{
  short the_vote;
  short vote_sum;
  switch (send_how(p_item, voter(), &the_vote, &vote_sum) )
    {
    case GOOD:
      break;
    case NO_VOTER:
      fprintf(stderr, "\nYou have not participated in this conference!\n");
    case FAILURE:  /* message written to stderr */
      fprintf(stderr, "\nTrouble in have_i_voted:");
      return PUNT;
      break;
    case STRING_OUT:
      fprintf(stderr, "\nTrouble in have_i_voted:");
      fprintf(stderr, "\n%s\n", message());
      return PUNT;
      break;
    default:
      break;
    }
  switch (the_vote)
    {
    case NOT_READ:
    case READ:
      return NO;
      break;
    default:
      return YES;
      break;
    }
  return MAYBE;
}
/************************************************************/
YESorNO 
is_valid_email(char * add)
{
  int i;
  int at;
  for (i = 0, at = 0; add[i]; i++)
    {
      if (add[i] == '@')
	at++;
    }
  if (at != 1)
    return NO;
  return YES;
}
/***************************************************************
 *      Informs eVote that this admin is leaving so that eVote
 *      can close the queue.
 *      Also, this file needs to know when the conference is left.
 ****************************************************************/
void 
leave_admin(void)
{
#ifdef EDEBUG
  if (edebug & FLOWER)
    {
      time_t end_time;
      time(&end_time);
      sprintf(debugger,
	      "echo %lu:Appli: pid %d About to leave after %ld seconds.",
	      who_am_i(), getpid(), end_time - start_time);
      output(debugger);
    }
#endif
  /*  dump_stats(); */
  drop_info();
  current_conf[0] = '\0';
  out_msg_buffer->mtype = PR_LEAVE_ADMIN;
  sprintf(out_msg_buffer->mtext, FNE_LEAVE_ADMIN, NNE_LEAVE_ADMIN);
  send_inst(0, NO);
  time_str[0] = '\0';
}
/***************************************************************
 *      Informs eVote that this voter is leaving so that eVote
 *      can make a little space and close this voter's ballot.
 *      Also, this file needs to know when the conference is left.
 ****************************************************************/
void 
i_am_leaving(void)
{
#ifdef EDEBUG
  if (edebug & FLOWER)
    {
      time_t end_time;
      time(&end_time);
      sprintf(debugger,
	      "echo %lu:Appli: pid %d About to leave after %ld seconds.",
	      who_am_i(), getpid(), end_time - start_time);
      output(debugger);
    }
#endif
  dump_stats();
  drop_info();
  current_conf[0] = '\0';
  out_msg_buffer->mtype = PR_LEAVING;
  sprintf(out_msg_buffer->mtext, FNE_LEAVING, NNE_LEAVING);
  send_inst(0, NO);
  time_str[0] = '\0';
}
/*****************************************************************
 *     informs eVote that the user has read the item.
 *     No acknowledgement given.
 ******************************************************************/
OKorNOT 
i_just_read(ITEM_INFO* p_item)
{
  OKorNOT fix_stats(YESorNO grab_msg);
  char *fix_about_item(ITYPE itype, STATIC_ID* static_id);
  char * rest;
  OKorNOT cc;
  out_msg_buffer->mtype = PR_I_READ;
  rest = fix_about_item(I_READ, &(p_item->static_id));
  sprintf(rest, FNE_I_READ);
#ifdef EDEBUG
  if (edebug & FLOWER)
    {
      sprintf(debugger,
	      "echo %lu:Appli: pid %d About to call send_inst from i_just_read",
	      who_am_i(), getpid());
      output(debugger);
    }
#endif
  if (send_inst(0, NO) != OK)
    {
#ifdef EDEBUG
  if (edebug & FLOWER)
    {
      sprintf(debugger,
	      "echo %lu:Appli: pid %d Failed send_inst from i_just_read returning NOT_OK",
	      who_am_i(), getpid());
      output(debugger);
    }
#endif
      return NOT_OK;
    }
#ifdef EDEBUG
  if (edebug & FLOWER)
    {
      sprintf(debugger,
	      "echo %lu:Appli: pid %d About to call fix_stats(YES) from i_just_read",
	      who_am_i(), getpid());
      output(debugger);
    }
#endif
  cc = fix_stats(YES);
#ifdef EDEBUG
  if (edebug & FLOWER)
    {
      sprintf(debugger,
	      "echo %lu:Appli: pid %d fix_stats(YES) return %s to i_just_read",
	      who_am_i(), getpid(), cc ? "OK" : "NOT_OK");
      output(debugger);
    }
#endif
  return cc;
}
/************************************************************
 *      finds the time stamp on a TIMESTAMP item
 *************************************************************/
time_t 
pull_time(ITEM_INFO* p_item)
{
  time_t time_stamp;
  char *fix_about_item(ITYPE itype, STATIC_ID* static_id);
  char * rest;
  if (p_item->eVote.type != TIMESTAMP)
    {
      fprintf(stderr,"\nTried to pull time on from wrong vote type.\n");
      return (time_t) -1;
    }
  out_msg_buffer->mtype = PR_PULL_TIME;
  rest = fix_about_item(PULL_TIME, &(p_item->static_id));
  sprintf(rest, FNE_PULL_TIME);
  if (send_inst(0, YES) != OK)
    return (time_t)-2;
  sscanf(in_msg_buffer->mtext, FEN_PULL_TIME, ENN_PULL_TIME);
  return time_stamp;
}
/************************************************************
 *       Returns the old time stamp.
 ************************************************************/
time_t 
push_time(ITEM_INFO* p_item, time_t new_time)
{
  time_t old_time;
  char *fix_about_item(ITYPE itype, STATIC_ID* static_id);
  char * rest;
  if (p_item->eVote.type != TIMESTAMP)
    {
      fprintf(stderr,"\nTried to push time on from wrong vote type.\n");
      return (time_t) -1;
    }
  out_msg_buffer->mtype = PR_PUSH_TIME;
  rest = fix_about_item(PUSH_TIME, &(p_item->static_id));
  sprintf(rest, FNE_PUSH_TIME, NNE_PUSH_TIME);
  if (send_inst(0, YES) != OK)
    return (time_t)-2;
  sscanf(in_msg_buffer->mtext, FEN_PUSH_TIME, ENN_PUSH_TIME);
  return old_time;
}
/**************************************************************
 *       Sends the vote into the Clerk.
 **************************************************************/
RTYPE 
send_vote(ITEM_INFO* p_item, short vote, short *ret_old_vote)
{
  OKorNOT fix_stats(YESorNO grab_msg);
  char *fix_about_item(ITYPE itype, STATIC_ID* static_id);
  char ave_str[8];
  short my_sum;
  unsigned long readers, voters;
  RTYPE ret = FAILURE;
  short old_vote, return_vote;
  char * rest;
  char sum_str[8];
  char vote_str[8];
  YESorNO yesno = NO;
  OKorNOT cc;
  if (p_item->eVote.vstatus == CLOSED)
    {
      fprintf(stderr, 
	      "\nsend_vote:Voting is closed on item #%d.\n", 
	      p_item->dropping_id);
      return FAILURE;
    }
  if (p_item->eVote.type == PLAIN)
    {
      fprintf(stderr,
	      "\nsend_vote: There is no vote on item #%d.\n", 
	      p_item->dropping_id);
      return FAILURE;
    }
  if (p_item->eVote.type == TALLIEDY 
     || p_item->eVote.type == GROUPEDY)
    yesno = YES;
  if (vote != READ && (vote > p_item->eVote.max 
			|| vote < p_item->eVote.min))
    {
      if (yesno == YES)
	fprintf(stderr,
		"\nsend_vote: Your vote on item #%d must be 'y' for YES, or 'n' for NO.",
		p_item->dropping_id);
      else
	fprintf(stderr,
		"\nsend_vote: Your vote on item #%d must be from %d to %d.", 
		p_item->dropping_id, p_item->eVote.min, 
		p_item->eVote.max);
      return FAILURE;
    }
  rest = fix_about_item(SEND_VOTE, &(p_item->static_id));
  out_msg_buffer->mtype = PR_SEND_VOTE;
  sprintf(rest, FNE_SEND_VOTE, NNE_SEND_VOTE);
#ifdef EDEBUG
  if (edebug & FLOWER)
    {
      sprintf(debugger,
	      "echo %lu:Appli: pid %d About to call send_inst from send_vote",
	      who_am_i(), getpid());
      output(debugger);
    }
#endif
  cc = send_inst(0, NO);
#ifdef EDEBUG
  if (edebug & FLOWER)
    {
      sprintf(debugger,
	      "echo %lu:Appli: pid %d send_inst returned %s to send_vote",
	      who_am_i(), getpid(), cc ? "OK" : "NOT_OK");
      output(debugger);
    }
#endif
  if (cc != OK)
    return FAILURE;
#ifdef EDEBUG
  if (edebug & FLOWER)
    {
      sprintf(debugger,
	      "echo %lu:Appli: pid %d About to call fix_stats from send_vote",
	      who_am_i(), getpid());
      output(debugger);
    }
#endif
  cc = fix_stats(YES);
#ifdef EDEBUG
  if (edebug & FLOWER)
    {
      sprintf(debugger,
	      "echo %lu:Appli: pid %d fix_stats returned %s to send_vote",
	      who_am_i(), getpid(), cc ? "OK" : "NOT_OK");
      output(debugger);
    }
#endif
  if (cc != OK)
    return FAILURE;
  switch (ret = (RTYPE)in_msg_buffer->mtype)
    {
    case NEW_STAT:      /* all these send a stat anyway */
    case NO_MODS:
    case NO_CHANGE:
    case VIOLATES_SUM:
    case NOT_GOOD:
      if (p_item->eVote.type >= GROUPED && p_item->eVote.type != TIMESTAMP)
	{
	  if (yesno == YES)
	    sscanf(((ITEM_STAT*)(in_msg_buffer->mtext))->text, 
		   FNN_GSTATYV, ENN_GSTATYV);
	  else							
	    sscanf(((ITEM_STAT*)(in_msg_buffer->mtext))->text, 
		   FNN_GSTATV, ENN_GSTATV);
	  fix_sum(p_item->dropping_id, my_sum);
	}
      else  /* not grouped */
	{
	  if (yesno == YES)
	    sscanf(((ITEM_STAT*)(in_msg_buffer->mtext))->text, 
		   FNN_TSTATYV, ENN_TSTATYV);
	  else
	    sscanf(((ITEM_STAT*)(in_msg_buffer->mtext))->text, 
		   FNN_TSTATV, ENN_TSTATV);
	}
      break;
    case NOT_ALLOWED:
      return NOT_ALLOWED;
    case NO_ITEM:
      fprintf(stderr,"\nsend_vote:Item #%d not found.\n",
	      p_item->dropping_id);
      return FAILURE;
      break;
    default:
      fprintf(stderr,"\nsend_vote: impossible case %d.\n", ret);
      return FAILURE;
      break;
    }
  *ret_old_vote = (short)old_vote;
  if (ret == NEW_STAT)
    ret = GOOD;
  return ret;
}
/******************************************************/
RTYPE 
send_drop_voter(char * conf_name, unsigned long uid_to_drop,
		YESorNO only_if_non_voter)
{
  out_msg_buffer->mtype = PR_DROP_VOTER;
  sprintf(out_msg_buffer->mtext, FNE_DROP_VOTER, NNE_DROP_VOTER);
  if (send_inst(0, YES) != OK)
    return FAILURE;
  return (RTYPE)in_msg_buffer->mtype;
}
/*****************************************************************
 *  send_enter_admin - alerts the eVote DBS that there is activity
 *  here. The DBS readies stuff for sending back.
 *  Initialized the stat array.  -- No voter is expected, for
 *  admin tasks only.
 *****************************************************************/	
OKorNOT 
send_enter_admin(char *conf_name)
{
  int new_mid;
#ifdef EDEBUG
  time(&start_time);
#endif 
  strcpy(current_conf, conf_name);
  out_msg_buffer->mtype = PR_ENTER_ADMIN;
  sprintf(out_msg_buffer->mtext, FNE_ENTER_ADMIN, NNE_ENTER_ADMIN);  
  if (send_inst(0, YES) != OK)
    return NOT_OK;
  switch (in_msg_buffer->mtype)
    {
    case MQ_OK:
      sscanf(in_msg_buffer->mtext, FEN_ENTER_ADMIN, ENN_ENTER_ADMIN);
      if (start_info(new_mid) != OK)
	{
	  fprintf(stderr,"\nTrying again.\n");
	  get_msg(NO);
	  if (in_msg_buffer->mtype != NEW_MID)
	    exit(0);
	  fprintf(stderr,"Success!\n");
	}
      return OK;
      break;
    case NO_CONF:
      fprintf(stderr, "\neVote:  No eVoted conference named %s.\n",
	      conf_name);
      return NOT_OK;
    default:
      fprintf(stderr, "\neVote:  Odd return from i_am_entering on %s: %ld\n",
	      conf_name, in_msg_buffer->mtype);
      break;
    }
  return NOT_OK;
}	
/*****************************************************************
 *  send_entering - alerts the eVote DBS that there is activity
 *  here. The DBS readies stuff for sending back.
 *  Initialized the stat array.
 *****************************************************************/	
char * 
send_entering(char *conf_name, short *ret_drop_days, 
		     int time_limit, ACTION *p_action)
{
  short drop_days;
  int new_mid;
  long last_access;
  int times = 0;
  int current_action;
  int trouble = 0;
#ifdef EDEBUG
  time(&start_time);
#endif 
  strcpy(current_conf, conf_name);
  if (who_am_i() == 0L && voter() == 0L )
    return "\nSorry, root cannot participate.  Please use your personal login.\n";
  out_msg_buffer->mtype = PR_ENTERING;
  sprintf(out_msg_buffer->mtext, FNE_ENTERING, NNE_ENTERING);  
  do
    {
      if (send_inst(0, YES) != OK)
	return NULL;
      switch (in_msg_buffer->mtype)
	{
	case MQ_OK:
	  sscanf(in_msg_buffer->mtext, FEN_ENTERING, ENN_ENTERING);
	  if (start_info(new_mid) != OK)
	    {
	      fprintf(stderr,"\nTrying again.\n");
	      get_msg(NO);
	      if (in_msg_buffer->mtype != NEW_MID)
		exit(0);
	      fprintf(stderr,"Success!\n");
	    }
	  *p_action = (ACTION)current_action;
	  strcpy(time_str, ctime((time_t*)(&last_access)));
	  return time_str;
	  break;
	case NO_CONF:
	  fprintf(stderr, "\neVote:  No eVoted conference named %s.\n",
		  conf_name);
	  *ret_drop_days = -1;
	  return NULL;
	case NEW_VOTER:
	  sscanf(in_msg_buffer->mtext, FEN_NEW_VOTER, ENN_NEW_VOTER);
	  *ret_drop_days = drop_days;
	  break;
	case ON_TWICE:
	  sscanf(in_msg_buffer->mtext, FEN_ON_TWICE, ENN_ON_TWICE);
	  sleep(times += 15 * trouble);
	  /* trouble is the number of times this guy has tried to
	     enter the list while he's already in the list. */
	  break;
	default:
	  fprintf(stderr, "\neVote:  Odd return from i_am_entering on %s: %ld\n",
		  conf_name, in_msg_buffer->mtype);
	  break;
	}
    }
  while (in_msg_buffer->mtype == ON_TWICE && times/60 < time_limit);
  if (in_msg_buffer->mtype == ON_TWICE)
    return "On twice";
  return NULL;
}	
/*****************************************************************/
char * 
send_joining(int time_limit, YESorNO local)
{
  int new_mid;
  char *conf_name = current_conf;
  int times = 0;
  int trouble = 0;
  strcpy(time_str,"First time participating.\n");
  out_msg_buffer->mtype = PR_JOINING;
  sprintf(out_msg_buffer->mtext, FNE_JOINING, NNE_JOINING); 
  do
    {
      if (send_inst(0, YES) != OK)
	return NULL;
      switch (in_msg_buffer->mtype)
	{
	case MQ_OK:
	  sscanf(in_msg_buffer->mtext, FEN_NEW_MID, ENN_NEW_MID);
	  if (start_info(new_mid) != OK)
	    {
	      fprintf(stderr,"\nTrying again.\n");
	      get_msg(NO);
	      if (in_msg_buffer->mtype != NEW_MID)
		exit(0);
	      fprintf(stderr,"Success!\n");
	    }
	  return time_str;
	  break;
	case NO_CONF:
	  fprintf(stderr, "\neVote:  No eVoted conference named %s.\n",
		  conf_name);
	  return NULL;
	case ON_TWICE:
	  sscanf(in_msg_buffer->mtext, FEN_ON_TWICE, ENN_ON_TWICE);
	  sleep(times += 15 * trouble);
	  /* trouble is the number of times this guy has tried to
	     enter the list while he's already in the list. */
	  break;
	default:
	  fprintf(stderr, "\neVote:  Odd return from i_am_joining on %s: %ld\n",
		  conf_name, in_msg_buffer->mtype);
	  break;
	}
    }
  while (in_msg_buffer->mtype == ON_TWICE && times/60 < time_limit);
  if (in_msg_buffer->mtype == ON_TWICE)
    return "On twice";
  return NULL;
}
/*****************************************************************
 *   changes the ballots belonging to was are moved to is.
 *****************************************************************/
RTYPE 
send_move(char * was, char * is)
{
  if (was == NULL || was[0] == '\0')
    return NO_VOTER;
  if (is == NULL || is[0] == '\0')
    return NO_VOTER;
  if (!is_valid_email(is))
    return NO_VOTER;
  out_msg_buffer->mtype = PR_MOVE;
  sprintf(out_msg_buffer->mtext, FNE_MOVE, NNE_MOVE); 
  if (send_inst(0, YES) != OK)
    return FAILURE;
  return (RTYPE)in_msg_buffer->mtype;
}
/*****************************************************************
 *      returns a pointer to the name or email address
 *      associated with the who_id in the WhoList.
 *****************************************************************/
char * 
who_is(unsigned long who_id)
{
  out_msg_buffer->mtype = PR_WHO_IS;
  sprintf(out_msg_buffer->mtext, FNE_WHO_IS, NNE_WHO_IS); 
  if (send_inst(0, YES) != OK)
    return NULL;
  switch (in_msg_buffer->mtype)
    {
    case GOOD:
      break;
    case NO_VOTER:
      sprintf(in_msg_buffer->mtext,"No user associated with who_id = %lu", who_id);
    }
  return in_msg_buffer->mtext;
}
/*****************************************************************
 *  Provides the voter id for the address, if it exists.  If not,
 *  and if add == YES, it'll make a new id.
 *****************************************************************/
unsigned long 
who_num(char * name, YESorNO add)
{
  unsigned long id;
  if (name == NULL || name[0] == '\0')
    return 0L;
  out_msg_buffer->mtype = PR_WHO_NUM;
  sprintf(out_msg_buffer->mtext, FNE_WHO_NUM, NNE_WHO_NUM); 
  if (send_inst(0, YES) != OK)
    return 0L;
  sscanf(in_msg_buffer->mtext, FEN_WHO_NUM, ENN_WHO_NUM);	
  return id;
}
RTYPE 
who_drop(unsigned long id, YESorNO force)
{
  out_msg_buffer->mtype = PR_WHO_DROP;
  sprintf(out_msg_buffer->mtext, FNE_WHO_DROP, NNE_WHO_DROP); 
  if (send_inst(0, YES) != OK)
    return FAILURE;
  return (RTYPE)in_msg_buffer->mtype;
}
/*****************************************************************
 *  returns the id of the user
 *****************************************************************/
unsigned
long
who_am_i(void)
{
  struct passwd * pw;
  static unsigned long i_am = 0L;
  char *name;
  if (i_am > 0L)
    return i_am;
#ifdef EDEBUG
  if (edebug & STRESS) 
    {
      i_am = (unsigned long)getuid();
      return i_am;
    }
#endif
  name = getlogin();
#ifdef linux
  if (name == NULL || name[0] == '\0')
    /*      name = cuserid(NULL); */
    name = getpwuid(getuid())->pw_name;
#endif
  pw = getpwnam(name);
  i_am = (unsigned long)pw->pw_uid; 
  return i_am;
}
/**************************************************/
unsigned long 
voter(void)
{
  if (mail_voter > 0L)
    return mail_voter;
  mail_voter = who_am_i();
  return mail_voter;
}

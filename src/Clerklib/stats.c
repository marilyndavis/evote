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

/* $Id: stats.c,v 1.4 2003/01/15 18:54:08 marilyndavis Exp $ */ 
/**********************************************************
 *	  ../eVote/src/Clerklib/stats.c   
 *	The item_stat array is maintained here.
 *	ITEM_STAT is defined in Clerkdef.
 *********************************************************
 **********************************************************/
#include "Clerk.h"
#include "msgdef.h"
#include "ipc_msg.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#ifdef EDEBUG
void output(char *);
extern char debugger[];
extern int edebug;
extern YESorNO trace;
#define FLOWER 8
#define STRESS 1
#endif
/* maintained here but also visible to voterl.c */
ITEM_STAT* item_stat = NULL;
/* private to this file */
#define STAT_BUNCH (10)  /* amount to increase the array by */
static int stat_space = 0;
char *get_stats(short dropping_id);
#ifdef EDEBUG
extern YESorNO trace;
#endif
/*********************************************************
 *      Called from eVoteipc::get_msg when a
 *      DROP_STAT message is found there.
 *      Returns the number of stats dropped.
 *      If input == NULL, it returns the number of stats
 *      dropped the last time stats were dropped and doesn't
 *      do anything.
 *********************************************************/
int 
drop_stats(char* input)
{
  short to, from;
  static short last_drop = 0;
#ifdef EDEBUG
  if (trace == YES)
    {
      if (input == NULL)
	{
	  printf("\n%d: entering drop_stats with input = NULL",
		 getpid());
	}
      else
	{
	  printf("\n%d: entering drop_stats with input = %s",
		 getpid(), input);
	}
    }
#endif
  if (input == NULL)
    return last_drop;
  sscanf(input, FEN_DROP_STATS, ENN_DROP_STATS);	
#ifdef EDEBUG
  if (trace == YES)
    {
      printf("\n%d: Dropping from %d to %d",
	     getpid(), from, to);
    }
#endif
  last_drop = to - from + 1;
  if (to < stat_space)
    {
      while (++to <= stat_space)
	{
	  item_stat[from] = item_stat[to];
	  item_stat[from].dropping_id = from++;
	}
    }
  from--;
  while (++from <= stat_space)
    {
      item_stat[from].dropping_id = from;
      item_stat[from].text[0] = '\0';
    }
#ifdef EDEBUG
  if (trace == YES)
    {
      printf("\n%d: Leaving drop_stats. Dropped %d.",
	     getpid(), last_drop);
    }
#endif
  return last_drop;
}		
/****************************************************
 *   Called from i_am_leaving()
 *****************************************************/
void 
dump_stats(void)
{
#ifdef EDEBUG
  if (trace == YES)
    {
      printf("\n%d: Entering dump_stats.",
	     getpid());
    }
#endif
  if (item_stat != NULL)
    free (item_stat); 
  item_stat = NULL;
  stat_space = 0;
#ifdef EDEBUG
  if (trace == YES)
    {
      printf("\n%d: Leaving dump_stats.",
	     getpid());
    }
#endif
}
/************************************************
 *   fixes the item_stat[] array with the new stats
 *   that are in the msg buffer.
 *   called by i_just_read, send_vote, send_stats, 
 *   if grab_msg == NO, it was called by send_vote or
 *   i_just_read and there is only one stat but there
 *   is also other info in the msg buffer.
 **********************************************************/
OKorNOT 
fix_stats(YESorNO grab_msg)
{
  int i, j, bytes_in;
  int no_items_in;
  ITEM_STAT *p_stat;
  OKorNOT grow_stats();
  YESorNO break_out = NO;
#ifdef EDEBUG
  if (edebug & FLOWER)
    {
      sprintf(debugger,
	      "echo %lu:Appli: pid %d Fix stats grab_msg = %d called.",
	      who_am_i(), getpid(), grab_msg);
      output(debugger);
    }
  if (trace == YES)
    {
      printf("\n%d: Fix stats called. grab_msg = %d",
	     getpid(), grab_msg);
    }
#endif
  do
    {
      if (grab_msg == YES)
	{
#ifdef EDEBUG
	  if (edebug & FLOWER)
	    {
	      sprintf(debugger,
		      "echo %lu:Appli: pid %d Fix stats calling get_msg(YES)",
		      who_am_i(), getpid());
	      output(debugger);
	    }
#endif
	  if ((bytes_in = get_msg(YES)) == -1)
	    {
#ifdef EDEBUG
	      if (edebug & FLOWER)
		{
		  sprintf(debugger,
			  "echo %lu:Appli: pid %d Fix stats got -1 from get_msg. Returning NOT_OK.",
			  who_am_i(), getpid());
		  output(debugger);
		}
#endif
	      return NOT_OK;
	    }
	  no_items_in = bytes_in/sizeof(ITEM_STAT);
	}
      else
	no_items_in = 1;
#ifdef EDEBUG
      if (edebug & FLOWER)
	{
	  sprintf(debugger,
		  "echo %lu:Appli: pid %d Fix stats got msg with %d items",
		  who_am_i(), getpid(), no_items_in);
	  output(debugger);
	}
      if (trace == YES)
	{
	  printf("\n%d: Fix stats: grabbed msg %s with %d items.",
		 getpid(), get_rtype((RTYPE)in_msg_buffer->mtype), no_items_in);
	}
#endif
      switch (in_msg_buffer->mtype)
	{
	case NO_ITEM:
	  fprintf(stderr,"\nNo item returned in fix_stats!");
#ifdef EDEBUG
	  if (edebug & FLOWER)
	    {
	      sprintf(debugger,
		      "echo %lu:Appli: pid %d Fix stats NO_ITEM returning NOT_OK",
		      who_am_i(), getpid());
	      output(debugger);
	    }
#endif
	  return NOT_OK;
	  break;
	case NOT_ALLOWED: /* voter is on vacation */
	case NO_MODS:  /* item closed */
	case NOT_GOOD:  /* outside limits */
	case NO_CHANGE: /* same vote as old vote */
	case VIOLATES_SUM:  /* violates sum_limit */
	case NEW_STAT:  /* good vote */
	case MORE_STATS: /* get more stats after processing this */
	case REDUNDANT: /* tried to make an item that exists */
	  break;
	default:
	  fprintf(stderr,"\nImpossible case:%ld = %s in fix_stats.", in_msg_buffer->mtype, 
		  get_rtype((RTYPE)in_msg_buffer->mtype));
#ifdef EDEBUG
	  if (edebug & FLOWER)
	    {
	      sprintf(debugger,
		      "echo %lu:Appli: pid %d Fix stats message impossible. Returning NOT_OK",
		      who_am_i(), getpid());
	      output(debugger);
	    }
#endif	  
	  return NOT_OK;
	  break;
	}
      p_stat = (ITEM_STAT*)(in_msg_buffer->mtext);
      while (no_items_in-- > 0)
	{
	  while (p_stat->dropping_id > stat_space)
	    {
	      if (grow_stats() != OK)
		{
#ifdef EDEBUG
		  if (edebug & FLOWER)
		    {
		      sprintf(debugger,
			      "echo %lu:Appli: pid %d Fix stats grow_stats failed. Returning NOT_OK",
			      who_am_i(), getpid());
			output(debugger);
		    }
#endif	  
		  return NOT_OK;
		}
	    }
	  item_stat[p_stat->dropping_id] = *p_stat;
	  /* when returning from a vote, we need a '\0' */
	  i = -1;
	  break_out = NO;
	  for (j=0; j < 3 && break_out == NO;	j++)
	    {
	      if (j != 0 || item_stat[p_stat->dropping_id].text[0] == ' ')
		while (item_stat[p_stat->dropping_id].text[++i] == ' ')
		  if (item_stat[p_stat->dropping_id].text[i] == '\0')
		    {
		      break_out = YES;
		      break;
		    }
	      while (break_out == NO 
		    && item_stat[p_stat->dropping_id].text[++i] != ' ')
		if (item_stat[p_stat->dropping_id].text[i] == '\0')
		  {
		    break_out = YES;
		    break;
		  }
	    }
	  if (i < 12)
	    i = 12;
	  if (break_out == NO)
	    item_stat[p_stat->dropping_id].text[i] = '\0';
	  /*					item_stat[p_stat->dropping_id].text[STXTLEN] = '\0'; */
#ifdef EDEBUG
	  if (trace == YES)
	    {
	      printf("\n%d: Fix stats: fixed stat for dropping_id = %d. Space = %d",
		     getpid(), p_stat->dropping_id, stat_space);
	    }
#endif
	  p_stat++;
	}
      grab_msg = YES;
    }		
  while (in_msg_buffer->mtype == MORE_STATS);
#ifdef EDEBUG
  if (trace == YES)
    {
      printf("\n%d: Leaving fix_stats",
	     getpid());
    }
  if (edebug & FLOWER)
    {
      sprintf(debugger,
	      "echo %lu:Appli: pid %d Fix stats happy. Returning OK",
	      who_am_i(), getpid());
	output(debugger);
    }
#endif
  return OK;
}
/************************************************************
 *      When a voter votes on one item in the group, the
 *      sum changes.  But the new sum needs to be in
 *      in the my_sum member of the first item in the group.
 *      So, send_vote() calls this function to get it done.
 ************************************************************/
void 
fix_sum(short dropping_id, short new_sum)
{
#ifdef EDEBUG
  if (trace == YES)
    {
      printf("\n%d: Fix_sum on dropping_id = %d, new_sum = %d",
	     getpid(), dropping_id, new_sum);
    }
#endif
  item_stat[dropping_id + item_info[dropping_id].eVote.more_to_come
	   - item_info[dropping_id].eVote.no_in_group + 1].my_sum
    = (unsigned long)new_sum;
}
/**********************************************************
 *       Guarentees that the extra stats are in local memory.
 *********************************************************/
OKorNOT 
get_extras(short dropping_id)
{
#ifdef EDEBUG
  if (trace == YES)
    {
      printf("\n%d: Get_extras called on dropping_id = %d",
	     getpid(), dropping_id);
      fflush(stdout);
    }
#endif
  if (dropping_id > *p_no_items )
    return NOT_OK;
  if (dropping_id <= stat_space &&
     item_stat[dropping_id].text[0] != '\0')
    return OK;
  if (strcmp(get_stats(dropping_id), "Not Found!  ") == 0)
    return NOT_OK; ;
#ifdef EDEBUG
  if (trace == YES)
    {
      printf("\n%d: Get_extras returning",
	     getpid());
      fflush(stdout);
    }
#endif
  return OK;
}
/*************************************************************
 *     This is called during the printing of the contents
 *     screen when the last item in a group is printed.
 *     The sum of the user's votes for this group is kept
 *     in the my_sum member of the first item in the group.
 ************************************************************/
short 
get_my_sum(short dropping_id)
{
  unsigned long my_sum;
  short first = dropping_id + item_info[dropping_id].eVote.more_to_come
    - item_info[dropping_id].eVote.no_in_group + 1;
#ifdef EDEBUG
  if (trace == YES)
    {
      printf("\n%d: get_my_sum called on dropping_id = %d",
	     getpid(), dropping_id);
      fflush(stdout);
    }
#endif
  if (get_extras(first) != OK)
    return -1L;
  my_sum = item_stat[first].my_sum;
#ifdef EDEBUG
  if (trace == YES)
    {
      printf("\n%d: get_my_sum returning %lu",
	     getpid(), my_sum);
      fflush(stdout);
    }
#endif
  return my_sum;
}
/*****************************************************************
 *         Returns the ratio of positive to negative votes
 *         and well as the sum of the positives and negatives.
 ************************************************************/
float 
get_ratio(short dropping_id, float* pos, float*neg)
{
  if (get_extras(dropping_id) != OK)
    return -1.;
  *pos = item_stat[dropping_id].pos;
  *neg = item_stat[dropping_id].neg;
  if (*neg > .5)
    return *pos/(*neg);
  else
    return -999.;
}
/*****************************************************************
 *         Returns the ratio of yes-voters to no-voters.
 *         and well as the number of yeses and nos.
 ************************************************************/
float 
get_vratio(short dropping_id, unsigned long* pos, 
	   unsigned long*neg)
{
  float ratio;
  if (get_extras(dropping_id) != OK)
    return -1.;
  *pos = item_stat[dropping_id].pos_voters;
  *neg = item_stat[dropping_id].neg_voters;
  if (item_info[dropping_id].eVote.type == TALLIEDY)
    *neg = item_stat[dropping_id].voters - *pos;
  if (*neg > 0L)
    ratio = *pos/(float)(*neg);
  else
    ratio =  -999.;
  return ratio;
}
/*****************************************************************
 *     Returns a string containing the statistics to print for
 *     the item requested.
 *     Does not communicate directly with eVote but maintains
 *     the stat array and makes calls to eVote using send_stats.
 ************************************************************/ 	
char* 
get_stats(short dropping_id)
{
  int to_send;
  int i;
  OKorNOT grow_stats();
  OKorNOT send_stats(STATIC_ID *static_id, int no_to_send);
  void set_symbols(short dropping_id);
#ifdef EDEBUG
  if (trace == YES)
    {
      printf("\n%d: Get_stats called on dropping_id = %d",
	     getpid(), dropping_id);
    }
#endif
  if (dropping_id > *p_no_items )
    return "Not Found!  ";
  while (dropping_id > stat_space)
    {
      if (grow_stats() != OK)
	{
	  fprintf(stderr,"\nNo system resources for new item stats.\n");
#ifdef EDEBUG
	  if (trace == YES)
	    {
	      printf("\n%d: get_stats: Not Found!",
		     getpid());
	    }
#endif
	  return "Not Found!  ";
	}
    }
  if (item_stat[dropping_id].text[0] != '\0')
    {
      set_symbols(dropping_id);
#ifdef EDEBUG
      if (trace == YES)
	{
	  printf("\n%d: Get_stats returns %s",
		 getpid(), item_stat[dropping_id].text);
	}
#endif
      return item_stat[dropping_id].text;
    }
  /* find the # to send */
  for (i = dropping_id + 1; i <= *p_no_items; i++)
    if (item_stat[i].text[0] != '\0')
      break;		
  to_send = i - dropping_id;
  if (send_stats(&item_info[dropping_id].static_id, to_send) == NOT_OK)
    {
#ifdef EDEBUG
      if (trace == YES)
	{
	  printf("\n%d: get_stats: bad return from send_stats. Returning Not Found",
		 getpid());
	  fflush(stdout);
	}
#endif
      return "Not Found!  ";
    }
  if (item_stat[dropping_id].text[0] != '\0')
    {
      set_symbols(dropping_id);
#ifdef EDEBUG
      if (trace == YES)
	{
	  printf("\n%d: get_stats returns %s",
		 getpid(), item_stat[dropping_id].text);
	  fflush(stdout);
	}
#endif
      return item_stat[dropping_id].text;
    }
#ifdef EDEBUG
  if (trace == YES)
    {
      printf("\n%d: get_stats reached end of function.",
	     getpid());
    }
#endif
  return "Not Found!  ";
}
/*****************************************************
 *     unsigned long get_voters(short dropping_id)
 ******************************************************/
unsigned long 
get_voters(short dropping_id)
{
  if (item_info[dropping_id].eVote.type < GROUPED)
    return 0L;
  if (get_extras(dropping_id) != OK)
    return 0L;
  return item_stat[dropping_id - item_info[dropping_id].eVote.no_in_group
		  + item_info[dropping_id].eVote.more_to_come + 1].voters;
}
/**********************************************************/
OKorNOT 
grow_stats(void)
{
  int i;
  char * pt;
  int old_space = stat_space;
#ifdef EDEBUG
  if (trace == YES)
    {
      printf("\n%d: grow_stats called, old_space = %d",
	     getpid(), old_space);
    }
#endif
  if (item_stat == NULL)
    {		
      old_space = -1;
      stat_space = STAT_BUNCH + *p_no_items;
      pt = malloc((stat_space +1) * sizeof(ITEM_STAT) );
    }
  else
    {
      stat_space += STAT_BUNCH;
      pt = realloc((char*)item_stat,
			  ((stat_space + 1) * sizeof(ITEM_STAT) ));
    }
  if (pt == NULL)
    {
#ifdef EDEBUG
      if (trace == YES)
	{
	  printf("\n%d: grow_stats can't malloc more space. Returning.",
		 getpid());
	}
#endif
      fprintf(stderr,"\ngrow_stats:No system resources.  Need %d slots. pid = %d, user id = %d", stat_space, getpid(), getuid());
      return NOT_OK;
    }
  item_stat = (ITEM_STAT*)pt;
  for (i = ++old_space; i <= stat_space; i++)	
    {
      item_stat[i].dropping_id = i;
      item_stat[i].text[0] = '\0';
    }
#ifdef EDEBUG
  if (trace == YES)
    {
      printf("\n%d: grow_stats returning OK.",
	     getpid());
    }
#endif
  return OK;
}
/*******************************************************
 *   This is called from ipc_msg.c when an unsolicited
 *   message, NEW_VSTATS, comes in.  It wipes out the
 *   status on the indicated dropping_id stat, and on 
 *   all the items in its group, if it's grouped.
 *******************************************************/
void 
new_vstatus(char* input)
{
  int lid, start, end;
  sscanf(input, FEN_NEW_VSTATUS, ENN_NEW_VSTATUS);
  start = lid;
  end = lid;
  if (item_info[lid].eVote.type >= GROUPED)
    {
      end = lid + item_info[lid].eVote.more_to_come;
      start = end - item_info[lid].eVote.no_in_group + 1;
    }
  for (lid = start; lid <= end; lid++)
    item_stat[lid].text[0] = '\0';
}
/*********************************************************
 *    This is called directly from get_msg when it finds
 *    a redo_stats request waiting there.
 ******************************************************/
OKorNOT 
redo_stats(void)
{
  int i;
  if (!p_no_items)
    return OK;
  while (stat_space < *p_no_items)
    {
      if (grow_stats() != OK)
	return NOT_OK;
    }
  for (i=0; i <= stat_space; i++)
    {
      item_stat[i].text[0] = '\0';
    }
  return OK;
}
/*****************************************************/
void 
set_symbols(short dropping_id)
{
  int i, blank[2] = {3, 7};
  char a;
  for (i = 0; (a = item_stat[dropping_id].text[i]) ; i++)
    {
      if (a == 'I' || a == 'p' || a == 'P')
	return;
    }
  for (i = 0 ; i < 2; i++)
    {
      while (item_stat[dropping_id].text[blank[i]] != ' '
	    && item_stat[dropping_id].text[blank[i]])
	blank[i]++;
    }
  if (item_info[dropping_id].eVote.type != PLAIN)
    {
      switch (item_info[dropping_id].eVote.priv_type)
	{
	case PUBLIC:
	  item_stat[dropping_id].text[blank[0]] = 'P';
	  break;
	case PRIVATE:
	  item_stat[dropping_id].text[blank[0]] = 'p';
	  break;
	case IF_VOTED:
	  item_stat[dropping_id].text[blank[0]] = 'I';
	  break;
	}
    }
  if (item_info[dropping_id].eVote.vstatus != CLOSED 
     && item_info[dropping_id].eVote.type != PLAIN)
    {
      item_stat[dropping_id].text[blank[1]] = '*';
    }
  else
    {
      item_stat[dropping_id].text[blank[1]] = ' ';
    }
}
/**************************************************************
 *     Called by get_stats when we don't have the 
 *			requested stats in memory.
 *     Requests and receives ITEM_STATs
 *     about items, starting at item static_id and sending n 
 *     consecutive items stats.
 *     Places the stat strings in the stat array.
 ***************************************************************/	
OKorNOT 
send_stats(STATIC_ID* static_id, int n)
{
  OKorNOT fix_stats(YESorNO grab_msg);
  char* fix_about_item(ITYPE itype, STATIC_ID* static_id);
  char* rest;
  rest = fix_about_item(SEND_STATS, static_id);
  out_msg_buffer->mtype = PR_SEND_STATS;
  sprintf(rest, FNE_SEND_STATS, NNE_SEND_STATS);
  if (send_inst(0, NO) != OK)
    return NOT_OK;
  return fix_stats(YES);
}

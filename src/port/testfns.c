/* $Id: testfns.c,v 1.4 2003/01/15 18:54:12 marilyndavis Exp $ */ 
/********************************************************************
 *
 *   testfns.c  - this is linked into the test executables
 *        It contains some helper functions for the test and
 *        some stubs to appease the loader.
 *********************************************************************/
#include "../demo/eVote.h"
#include <unistd.h>
int time_out = 60;
extern int edebug;
extern short* p_no_items;
extern ITEM_INFO* item_info;
extern char current_conf[];

char pok[] = "OK";
char pnok[] = "NOT OK";
extern YESorNO debugging;

OKorNOT send_check(ITEM_INFO* item_list, short no_in_group, 
		   char* msg, YESorNO verbose);

/********************************************************************
 *
 *    make_group creates a group of items.  If same_limits == YES,
 *      all the items have the same min and max vote, min_start
 *      and max_start.  If not, the min grows and the max shrinks
 *      through the group until the min >= max and then it
 *      starts over.
 *      msg becomes part of each item's title.
 *********************************************************************/
OKorNOT make_group(short no_in_group, VOTE_TYPE vtype, short sum_limit,
		   short min_start, short max_start, YESorNO same_limits, 
		   char * msg)
{
  ITEM_INFO new_item[25];
  short min_copy = min_start;
  short max_copy = max_start;
  static PRIV_TYPE ptype = PRIVATE;
  int i;
  short sum_min = 0;
  
  i = (int)ptype;
  ptype = (PRIV_TYPE)(++i%3);
  
  for (i=0; i < no_in_group; i++)
    {
      new_item[i].eVote.author = getuid();
      new_item[i].eVote.more_to_come = no_in_group - i - 1;
      new_item[i].eVote.type = vtype;
      new_item[i].eVote.sum_limit = sum_limit;
      new_item[i].eVote.vstatus = OPEN;
      new_item[i].eVote.priv_type = ptype;
      new_item[i].eVote.no_in_group = no_in_group;
      new_item[i].static_id.local_id = 0l;
      new_item[i].static_id.network_id = MY_HOST_ID;
      new_item[i].dropping_id = 0;
      if(vtype == GROUPEDY || same_limits == YES)
	{
	  new_item[i].eVote.min = min_start;
	  new_item[i].eVote.max = max_start;
	}
      else
	{
	  new_item[i].eVote.min = min_start++;
	  new_item[i].eVote.max = max_start--;
	  if(min_start >= max_start)
	    {
	      min_start = min_copy;
	      max_start = max_copy;
	    }
	}
      sum_min += new_item[i].eVote.min;
    }
  
  if (sum_min > sum_limit)
    for(i = 0; i < no_in_group; i++)					
      new_item[i].eVote.sum_limit = sum_min +3;
  
  return send_check(new_item, no_in_group, msg, YES);
}

/********************************************************************
 *
 *   send_check sends in the request to The Clerk to create the
 *    item(s) and checks that everything is OK.
 *********************************************************************/

OKorNOT send_check(ITEM_INFO* item_list, short no_in_group, char* msg,
		   YESorNO verbose)
{
  OKorNOT cc;
  short old_no_items = *(p_no_items);
  int i;
  char moremsg[10];
  char* get_stats(short lid);		
  OKorNOT evcmp(eVOTE* e1, eVOTE* e2);
  
  for (i = 0; i < no_in_group; i++)
    {
      strcpy(item_list[i].eVote.title, msg);
      sprintf(moremsg,"-> #%d",*p_no_items+i+1);
      strcat(item_list[i].eVote.title, moremsg);
    }
  cc = eVote_new_items(item_list, no_in_group);
  if(verbose == YES || cc != OK)
    printf("\neVote_new_items returned %s on %s.",
	   (cc?pok:pnok),msg);
  if(cc != OK)
    return cc;
  if (*(p_no_items) != old_no_items + no_in_group)
    printf("\nGrowing fast: No of items should be %d, is %d.",
	   old_no_items+no_in_group, *(p_no_items));
  else
    for (i = 0; i < no_in_group; i++)
      {
	if(evcmp(&item_list[i].eVote, &item_info[old_no_items + i + 1].eVote)
	   != OK)
	  printf("\nTrouble in eVote struct on i = %d", i);
      }
  if(verbose == YES)
    for(i=old_no_items + 1; i <= *(p_no_items); i++)
      {
	printf("\n%d: %s %s",i, get_stats(i), item_info[i].eVote.title);
      }
  fflush(stdout);
  return OK;
}
/********************************************************************
 *
 *   evcmp is a function that looks at two eVOTE structures and
 *     tells if they are identical.
 *    OK is returned if they are identical.
 *    NOT_OK if they are not.
 *********************************************************************/

OKorNOT evcmp(eVOTE * e1, eVOTE * e2)
{
  if(e1->type != e2->type
     || e1->vstatus != e2->vstatus
     || e1->priv_type != e2->priv_type
     || e1->max != e2->max
     || e1->min != e2->min
     || e1->no_in_group != e2->no_in_group
     || e1->more_to_come != e2->more_to_come
     || e1->sum_limit != e2->sum_limit)
    return NOT_OK;
  return OK;
}
/********************************************************************
 *
 *     check_read sends a read instruction on the indicated item
 *        checks that it was received OK, and prints out the new
 *        stats.
 *********************************************************************/
void check_read(short lid)
{
  OKorNOT cc;
  OKorNOT i_just_read(ITEM_INFO * item);
  char * get_stats(short lid);
  char old_stats[40];
  char new_stats[40];

  strcpy(old_stats, get_stats(lid));
  
  cc =	i_just_read(&item_info[lid]);
  strcpy(new_stats, get_stats(lid));

  printf("\n%d: Reading %3d is %s: %3d %s %s",getuid(), 
	 lid, (cc?pok:pnok), lid, 
	 new_stats, item_info[lid].eVote.title);
  if(strcmp(new_stats, old_stats) == 0 )
    {
      printf("%d: Old stats = %s New stats = %s. Leaving.\n",getuid(),
	     old_stats, new_stats);
      i_am_leaving();
      exit(1);
    }
  fflush(stdout);
}
/********************************************************************
 *
 *    check_vote sends in a vote instruction on the item,
 *      voting the  char vote.  It checks that the vote was
 *      acknowledged and prints out the new stat.
 *
 *********************************************************************/
void check_vote(short lid, short vote)
{
  RTYPE cc;
  short old_vote;
  char *get_stats(short lid);
  char old_stats[40];
  char new_stats[40];

  strcpy(old_stats, get_stats(lid));
  
  cc = send_vote(&item_info[lid], vote, &old_vote);
  strcpy(new_stats, get_stats(lid));
  printf("\nVoting %d on %d is %s:\n   %d: %s %c%s%c\n", 
	 vote,lid, (cc != FAILURE ?pok:pnok),
	 lid, new_stats, 34,item_info[lid].eVote.title,34);
  /*  if(strcmp(new_stats, old_stats) == 0 
     && item_info[lid].eVote.type < GROUPED
     && item_info[lid].eVote.vstatus == OPEN)
    {
      printf("%d: Old stats = %s New stats = %s Leaving.", getuid(),
	     old_stats, new_stats);
      i_am_leaving();
      exit(1);
    }
  */
  fflush(stdout);
}



/* these are for the loader */

void helper(char *inp)
{  /* for the loader */
}

OKorNOT do_yours(int *pfield, YOUR_ITEM *p_your_item,
		 ITEM_INFO *p_eVote_item, 
		 MODE * pmode, int changeno)
{
  return OK;
}

OKorNOT add_group(YOUR_ITEM * your_list,ITEM_INFO * item_list)
{
  return OK;
}


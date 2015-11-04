/* $Id: slowvoter.c,v 1.4 2003/01/15 18:54:12 marilyndavis Exp $ */ 
/********************************************
  This test 
  expects there to be 
  a conf, argv[1].
  This test 
  It sleeps now and then for slowness seconds.  Slowness is
  argv[2]
  It enters and makes some items, reads some items, votes
  on some items, drops some items and leaves.
					     
****************************************/
#include<stdio.h>  
#include "../demo/eVote.h"
#include <pwd.h>
#include <unistd.h>
OKorNOT send_check(ITEM_INFO* item_list, short no_in_group, 
		   char* msg, YESorNO verbose);

extern short *p_no_items;
extern ITEM_INFO *item_info;
extern short edebug;

int 
main(int argc, char *argv[])
{
  char conf[50];
  ITEM_INFO new_item;
  ITEM_INFO item_copy;
  short slowness = 5;
  
  char * get_stats(short dropping_id);
  OKorNOT i_am_entering(char *,YESorNO);
  void i_am_leaving();
  OKorNOT make_group(short no_in_group, VOTE_TYPE vtype, short sum_limit,
		     short min_start, short max_start, YESorNO same_limits, 
		     char * msg);
  void drop_something();
  char msg[80];
  OKorNOT cc;
  RTYPE ccc = GOOD;
  char *pt;
  short old_vote;
  short number = 3;
  char title[TITLEN +1];
  unsigned int my_uid;
  int x = 0;
  edebug = 9;  /* set to 9 if there's trouble and let everyone
		  write to the data directory  -- otherwise 1 */
  
  start_up(NO);
  
  if(argc > 1)
    {
      strcpy(conf,argv[1]);
      slowness = atoi(argv[2]);
    }
  else
    {
      printf("\nUsage: slowvoter conf slowness\n");
      exit(0);
    }
  my_uid = who_am_i();
  
  /***  ENTERING ***/
  do
    {
      if(++x > 20) 
	exit(0);
    } 
  while (i_am_entering(conf,YES) == NOT_OK);
  /* That "YES" tells it not to wait for user input */
  
  printf("\nUser %ld is a slow voter in %s.",who_am_i(), conf);
  
  /*** concoct new items ****/
  
  sprintf(msg,"%d Group of %d",getuid(),number);
  if((cc = make_group(number, GROUPEDY, number-number/2,
		      0, 1, YES,  msg)) == NOT_OK)
    {
      printf("\nBad return from make_group - quitting %d.\n",getuid());
      exit(0);
    }
  new_item.static_id.network_id = MY_HOST_ID;
  new_item.static_id.local_id = 0l;
  new_item.dropping_id = 0;
  new_item.eVote.sum_limit = 0;
  new_item.eVote.max = 0;
  new_item.eVote.min = 0;
  new_item.eVote.author = getuid();
  new_item.eVote.vstatus = OPEN;
  new_item.eVote.priv_type = (PRIV_TYPE)(my_uid%3);
  new_item.eVote.no_in_group = 1;
  new_item.eVote.more_to_come = 0;
  
  /***  CREATE TALLIED YES/NO ITEM ***/
  /* tallied, not grouped */
  
  new_item.eVote.type = TALLIEDY;
  new_item.eVote.max = 1;
  new_item.eVote.min = 0;
  while(number-- > 0)
    {
      sprintf(title,"User %d:TALLIED Y/N : #%d",my_uid,*p_no_items + 1);
      if (send_check(&new_item, 1, title,YES) != OK)
	exit(0);
    }
  for(number = 1; number <= *p_no_items; number++)
    {
      item_copy = item_info[number];
      
      if((my_uid%3) && ( cc = 	i_just_read(&item_copy)) == OK)
	{
	  printf("\nJust read %d.",number);
	  if(my_uid%2)
	    {
	      switch(item_copy.eVote.type)
		{
		case PLAIN:
		  ccc = GOOD;
		  break;
		case TALLIEDN:
		case GROUPEDN:
		case GROUPEDn:
		  ccc = send_vote(&item_copy,item_copy.eVote.max, &old_vote);
		  break;
		case TALLIEDY:
		case GROUPEDY:
		  ccc = send_vote(&item_copy,1, &old_vote);
		  break;
		default:
		  break;
		}
	    }
	}
      if(cc == OK && ccc == GOOD)
	{
	  printf("\n%3d: %s",number, pt = get_stats(number));
	  if (strcmp(pt, "Not Found!  ") == 0)
	    cc = NOT_OK;
	}
      if(cc != OK || ccc != GOOD)
	printf("\nKeeping on.");
      sleep(slowness);
    }
  
  drop_something();
  i_am_leaving();
  return 0;
}

void 
drop_something(void)
{
  unsigned int me;
  short start, end;
  int i;
  short no_drops;
  
  me = getuid();
  for (i = 1; i <= *p_no_items; i++)
    {
      if(item_info[i].eVote.author == me)
	break;
    }
  if ( i > *p_no_items)
    {
      printf("\nCouldn't find an item to drop for %d\n",me);
      return;
    }
  start = i;
  end = i;
  no_drops = process_drops(&start,&end,YES);
  printf("\nDropped %d items for %d.\n",no_drops,me);
}



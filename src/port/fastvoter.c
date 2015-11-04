/* $Id: fastvoter.c,v 1.4 2003/01/15 18:54:12 marilyndavis Exp $ */ 
/********************************************
 *    This test 
 *    expects there to be 
 *     a conf, argv[1].
 *    It enters and makes some items, reads some items, votes
 *    on some items, drops some items and leaves.
 *
 **r**************************************/
#define TIME_OUT  4000

#include<stdio.h>
#include<sys/types.h>
#include<unistd.h>
#include<signal.h>
#include "../demo/eVote.h"
#include <pwd.h>
OKorNOT send_check(ITEM_INFO* item_list, short no_in_group, 
		   char* msg, YESorNO verbose);

void set_signals();
extern short *p_no_items;
extern ITEM_INFO *item_info;
extern short edebug;
unsigned int my_uid;

int 
main(int argc,char * argv[])
{
  char conf[50];
  ITEM_INFO new_item;
  ITEM_INFO item_copy;
  
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
  int x = 0;
  edebug = 9;  /* set to 9 if there's trouble and let everyone
		  write to the data directory  -- otherwise 1 */
  
  set_signals();
  start_up(NO);
  strcpy(conf,"ed");
  while(--argc > 0)
    {
      strcpy(conf,argv[argc]);
    }
  
  my_uid = who_am_i();
  
  /***  ENTERING ***/
  do
    {
      if(++x > 20) 
	{
	  printf("\nUser %ld failed to enter %s.", who_am_i(), conf);
	  exit(0);
	}
    } 
  while (i_am_entering(conf,YES) == NOT_OK);
  /* That "YES" tells it not to wait for user input */
  
  printf("\nUser %ld is a fast voter in %s.",who_am_i(), conf);
  
  /*** concoct new items ****/
  
  sprintf(msg,"%d Group of %d",getuid(),number);
  if((cc = make_group(number, GROUPEDY, number-number/2,
		      0, 1, YES,  msg)) == NOT_OK)
    {
      printf("\nBad return from make_group - quitting %d.\n",getuid());
      i_am_leaving();
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
	{
	  printf("\n Couldn't create a new item for user %d.\n", my_uid);
	  i_am_leaving();
	  exit(0);
	}
    }
  for(number = 1; number <= *p_no_items; number++)
    {
      item_copy = item_info[number];
      
      if((my_uid%3) && ( cc = 	i_just_read(&item_copy)) == OK)
	{
	  printf("\n%d:Just read %d.",my_uid, number);
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
	  printf("\n%d: %3d: %s",my_uid, number, pt = get_stats(number));
	  if (strcmp(pt, "Not Found!  ") == 0)
	    cc = NOT_OK;
	}
      if(cc != OK || ccc != GOOD)
	printf("\n%d:Keeping on.", my_uid);
      /*      sleep(4);*/
    }
  
  drop_something();
  i_am_leaving();
  return 0;
}

void drop_something()
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

void process_signal(int signo);

/********************************************************
 *
 *   void process_signal(int signo)
 *     Caught signals come here.
 *************************************************************/
void process_signal(int signo)
{
  char buf[200];
  signal(signo,process_signal);
  switch(signo)
    {
    case SIGALRM:
      printf("\nTimed out on %d, leaving\n", my_uid);
      sprintf(buf, "echo Timed out on %d. >> %s", my_uid, FLOW);
      system(buf);
      i_am_leaving();
      exit(0);
      break;
    }
  exit(1);
}
/*******************************************************
 *
 *    void set_signals()
 *      Sets every signal in my book to go to process_signal() 
 *************************************************************/     
void set_signals()
{
  signal(SIGALRM, process_signal);
  alarm(TIME_OUT);
}



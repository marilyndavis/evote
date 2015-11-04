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

/* $Id: eVote_test.c,v 1.4 2003/01/15 18:54:12 marilyndavis Exp $ */ 
/*********************************************
This test called by
test  [conf_name] [-dn]

n indicates a debug level.  This only has meaning if the source was
compiled with EDEBUG #defined.

If there is no conf_name, it assumes "ed".  

It tries to enter the conf.  That should fail because the conf
has not been created.  

It creates the conf with 'x' on the end of the name and with 
drop_days == 1.

It enters, puts in some topics, reads them, leaves and drops the user.
The dropping of the user should fail.

It creates the conf without 'x' on the end with drop_days == 0
and the then enters.

It puts in some items:

1   1.  plain to drop
2   1.  plain 
3   2.  tallied yes/no
4   3.  tallied <= 9
5   3.  tallied <=9 to drop
6   4   tallied >9
7   4.  tallied >9 to drop
8   5.  grouped yes/no to drop
9   6.  "              to drop
10  7.  "              to drop
11  5.  grouped yes/no
12  6.  "
13  7.  "
14  8.  grouped <=9
15  9.  "
16 10.  "
17 11.  grouped >9
18 12.  "
19 13.  "
20 - 319 big list.  Later it'll drop 260 in the center.
  
It reads them all.
It leaves and drops itself from the conf.  This should succeed.

It enters again, and lists the stats.  We should be down to
zero participants in the stats.

It reads them all.

It drops all the items.

It puts them all in again.

It reads them all again.

It votes on the big group.  The votes cycle from 0 to 99 three times.
On the first 19, not in the big group, it votes the max if the item
will be dropped, the min if it is staying.

It prints a sample of the stats.

Then it drops the items that should be dropped in the first
19 items.  The group stays a little longer.

It prints a sample of the stats.  The droppers should be gone
and the next ones should be moved up in the lists.

It votes '1' on the remaining items before the big group.

It prints a sample of the stats.

Then it leaves the conf.

It enters again.

It prints a sample of the stats.

It changes the vstatus of items #2, 5, & 3.

It prints a few stats.

Leaves, enters again, prints a few and votes on 2, 5, & 3.

Again it changes the vstatus on 2 and 5 and 5 again.

It prints a few stats and votes on 2, 5, and 3.

It changes the vstatus on 2, prints a few stats and votes on 2, 5 & 3 again.

leaves again.

Then it checks the conf's statistics.  This is not much of a check
since there is only one voter.

It adjourns the conf.

And it checks the conf's statistics again.  
enters - reconvenes the conf

It makes a new group of 11.

Then it finally drops 260 items out of the center of the big group
of not-grouped items.

It makes a new group of 12.

It leaves again.

It checks the conf.

It adjourns the conf.

Enters again, prints another sample of stats.

Makes a group of 11, of 12, and of 13.

It leaves again.

It checks again.

Adjourns again.

Enters one last time.

Prints a sample of the stats.

leaves again.
---
sends quit
*****************************************************/

#include "../demo/eVote.h"
#include <unistd.h>
extern int edebug;
extern short* p_no_items;
extern ITEM_INFO* item_info;
extern char current_conf[];

extern char pok[];
extern char pnok[];
YESorNO debugging;

void enter(char* name);
void make_group(short no_in_group, VOTE_TYPE vtype, short sum_limit,
		short min_start, short max_start, YESorNO same_limit, 
		char * msg);
void send_check(ITEM_INFO*, short, char*, YESorNO verbose);

long dropper[5][2];  /* [][0] stores the local_id of the item 
			to be dropped. [][1] holds the number of
			items to drop, starting with item [][0] */

int main(int argc,char *argv[])
{
  ITEM_INFO new_item;
  char conf[50];
  char confx[51];
  OKorNOT cc;
  short lid, starter, ender;
  int di = 0;
  int dou = 0;
  short order;
  short vote;
  
  OKorNOT eVote_conf(char *, short drop_day);
  OKorNOT i_am_entering(char *,YESorNO);
  void i_am_leaving();
  OKorNOT adjourn_conf(char *);
  char * get_stats(short i);
  
  void make_items(YESorNO verbose);
  void check_vote(short lid, short vote);
  void check_read(short lid);
  void make_group(short no_in_group, VOTE_TYPE vtype, short sum_limit,
		  short min_start, short max_start, 
		  YESorNO same_limits, char * msg);
  
  /**** COMMAND LINE ****/
  debugging = YES;
  start_up(NO);
  
  strcpy(conf,"ed");
  while(--argc > 0)
    {
      if(strncmp(argv[argc],"-d",2) == 0)
	edebug = atoi(&argv[argc][2]);
      else
	strcpy(conf,argv[argc]);
    }
  
  /***  Checking if the conf exists through an entering call ***/
  
  printf("\n About to enter non-existant conf, %s",conf);
  if ((cc = i_am_entering(conf,YES)) == OK)
    {
      printf("\nTo run this test, there should not be a %s conf.\n",conf);
      printf("\nDo:     eVote dropc %s\nand try again.\n", conf);
      exit(0);
    }
  
  /****  CREATE CONF WITH X ****/
  
  strcpy(confx,conf);
  strcat(confx,"x");
  
  printf("\nAbout to call eVote_conf %s.",confx);
  cc = eVote_conf(confx, 1);
  printf("\nAfter create %s: %s ",confx,(cc?pok:pnok));
  
  /***  ENTERING ***/
  enter(confx);
  
  make_items(NO);
  
  for (lid = 1; lid <= *p_no_items; lid+=12)
    {
      check_read(lid);
    }
  
  /* get out and delete myself */
  
  printf("\nLeaving and dropping myself from %s.\n",confx);
  i_am_leaving();
  
  /* fails at voter dropping here */
  drop_voter(confx,getuid());
  
  /***  Make a droppable conf. ***/
  
  printf("\nAbout to call eVote_conf %s.",conf);
  cc = eVote_conf(conf, 0);
  printf("\nAfter create %s: %s ",conf,(cc?pok:pnok));
  
  /***  ENTERING ***/
  enter(conf);
  
  make_items(NO);
  
  for (lid = 1; lid <= *p_no_items; lid+=20)
    {
      check_read(lid);
    }
  
  /* get out and delete myself */
  
  printf("\nLeaving and dropping myself from %s.\n",conf);
  i_am_leaving();
  drop_voter(conf,getuid());  /* should succeed */
  
  /***  ENTERING ***/
  enter(conf);
  /*** PRINT THE STATS ****/
  
  for (lid = 1; lid <= *p_no_items; lid += 10)		
    printf("\n%d: %s  %s",lid, get_stats(lid),item_info[lid].eVote.title);
  
  for (lid = 1; lid <= *p_no_items; lid+=13)
    {
      check_read(lid);
    }
  
  /* drop all the items */
  
  starter = 1;
  ender = *p_no_items;
  printf("\nDropping all items returns %d.\n",
         process_drops(&starter, &ender, YES));
  
  /* put them in again */
  make_items(NO);
  
  for (lid = 1; lid <= *p_no_items; lid++)
    {
      i_just_read(&item_info[lid]);
    }
  
  /* vote on the big group */
  
  for (vote = -1, lid = *p_no_items-300 + 1; lid <= *p_no_items; lid++)
    {
      vote = ++vote % 100;
      check_vote(lid,vote);
    }
  /*  vote low on keepers, high on droppers */
  for (lid = 1; lid <= 19; lid++)
    {
      if(item_info[lid].eVote.type == PLAIN)
	{
	  if (dropper[di][0] == item_info[lid].static_id.local_id)
	    di++;
	  continue;
	}
      if ( dropper[di][0] == item_info[lid].static_id.local_id)
	{
	  for (dou=0; dou < (short)dropper[di][1]; dou++)
	    {
	      check_vote(lid++,item_info[lid].eVote.max);
	    }
	  lid--;
	  di++;
	  continue;
	}
      /*else*/
      check_vote(lid,item_info[lid].eVote.min);
    }
  
  /*** PRINT THE STATS ****/
  
  for (lid = 3; lid <= *p_no_items; lid += 10)		
    printf("\n%d: %s  %s",lid, get_stats(lid),item_info[lid].eVote.title);
  
  /*** drop the droppers ***/
  fflush(stdout);
  for (dou = 0; dou <= 2; dou++)
    {
      order = (dou*2 + 1)%3;  /* results in 1,0,2 order */
      if(dou != 1)
	{
	  i_am_leaving();
	  sleep(2);  /* let it adjourn */
	  enter(conf);
	}
      lid = 1;
      while(item_info[lid].static_id.local_id != dropper[order][0])
        lid++;
      starter = lid;
      ender = (short)(dropper[order][1] +  lid - 1);
      printf("\nprocess_drops from %d to %d returns %d.\n",starter,ender,
	     process_drops(&starter, &ender, YES));
    }
  
  for (lid = 4; lid  <= *p_no_items; lid += 6)		
    printf("\n%d: %s  %s",lid, get_stats(lid),item_info[lid].eVote.title);
  
  /*** VOTE 1 on 2 - 13 ***/
  for(lid = 2; lid <= 13; lid++)
    check_vote(lid,1);
  
  /*** PRINT THE STATS ****/
  for (lid = 3; lid <= *p_no_items; lid+= 20)		
    printf("\n%d: %s  %s",lid, get_stats(lid),item_info[lid].eVote.title);
  
  /***  LEAVING  ***/
  printf("\nLeaving");
  i_am_leaving();
  sleep(2); /* let it adjourn */
  
  /*** ENTERING AGAIN ****/
  enter(conf);
  
  /*** PRINT THE STATS ****/
  for (lid = 2; lid <= *p_no_items; lid+= 20)		
    printf("\n%d: %s  %s",lid, get_stats(lid),item_info[lid].eVote.title);
  
  printf("\nTesting the changing of vstatus.\n");
  new_item = item_info[2];
  change_vstatus(&new_item, 'U', YES);
  new_item = item_info[5];
  change_vstatus(&new_item, 'L', YES);
  new_item = item_info[3];
  change_vstatus(&new_item, 'O', YES);
  for (lid = 1; lid <= 5; lid++)
    printf("\n%d: %s  %s",lid, get_stats(lid),item_info[lid].eVote.title);
  i_am_leaving();
  sleep(2);
  enter(conf);
  printf("\nAfter leaving and returning.\n");
  for (lid = 1; lid <= 5; lid++)
    printf("\n%d: %s  %s",lid, get_stats(lid),item_info[lid].eVote.title);
  check_vote(2,0);
  check_vote(5,0);
  check_vote(3,0);
  new_item = item_info[2];
  change_vstatus(&new_item, 'O', YES);
  new_item = item_info[5];
  change_vstatus(&new_item, 'U', YES);
  new_item = item_info[5];
  change_vstatus(&new_item, 'O', YES);
  for (lid = 1; lid <= 5; lid++)
    printf("\n%d: %s  %s",lid, get_stats(lid),item_info[lid].eVote.title);
  check_vote(2,1);
  check_vote(5,1);
  check_vote(3,1);
  new_item = item_info[2];
  change_vstatus(&new_item, 'L', YES);
  for (lid = 1; lid <= 5; lid++)
    printf("\n%d: %s  %s",lid, get_stats(lid),item_info[lid].eVote.title);
  check_vote(2,0);
  check_vote(5,0);
  check_vote(3,0);
  
  /*** LEAVING AGAIN ****/
  printf("\nLeaving again.");
  i_am_leaving();
  
  cc = check_conf(conf);
  adjourn_conf(conf);
  cc = check_conf(conf);
  
  printf("\nAdjourned %s.",conf);
  enter(conf);
  
  make_group(11, GROUPEDY, 5, 0, 1, YES, "11: GROUPED YES/NO");
  
  /* look for the big group to drop */
  for (lid = 1; lid <= *p_no_items; lid++)		
    {
      if (dropper[3][0] == item_info[lid].static_id.local_id)
	break;
    }
  starter = lid;
  ender = lid+259;
  process_drops(&starter,&ender,YES);
  
  make_group(12, GROUPEDY, 5, 0, 1, YES, "12: GROUPED YES/NO");
  
  i_am_leaving();
  cc = check_conf(conf);
  adjourn_conf(conf);
  enter(conf);
  
  make_group(11, GROUPEDY, 5, 0, 1, YES, "11:#39-49: GROUPED YES/NO");
  make_group(12, GROUPEDN, 50, 0, 50, NO, "12:#50-61:GROUPED NUMERIC");
  make_group(13, GROUPEDY, 10, 0, 1, YES, "13:#62-74:GROUPED YES/NO");
  
  i_am_leaving();
  cc = check_conf(conf);
  adjourn_conf(conf);
  
  enter(conf);
  
  for (lid = 1; lid <= *p_no_items; lid++)		
    printf("\n%d: %s  %s",lid, get_stats(lid),item_info[lid].eVote.title);
  i_am_leaving();
  
  send_quit();
  printf("\nThis test is finished and The Clerk has been brought down.\n");
  printf("\n\n\nIf you ran this test by issuing the command");
  printf("\n\neVote_test >& eVote_test.out & \n\n");
  printf("\ndiff eVote_test.out eVote_test.right  \n\n to see if everything is OK.\n");
  printf("\nThe only differences should be in the date and time in the line:");
  printf("\n> ed: Last participation on Mon Feb 21 11:22:37 1994 \n\n");
  return 0;
}

void enter(char * conf_name)
{
  OKorNOT cc;
  short count = 0;
  
  do
    {
      printf("\nAbout to enter");
      cc = i_am_entering(conf_name,YES);
      printf("\nBack from i_am_entering: %s. Current conf = %s.", 
	     (cc?pok:pnok), current_conf);
    }
  while(cc == NOT_OK && ++count < 20);
  if (cc == NOT_OK)
    {
      printf("\nYou must start The Clerk before you run this test!\n\n");
      exit(0);
    }
}
void make_items(YESorNO verbose)
{
  ITEM_INFO new_item;
  int i;
  
  /*** concoct new items ****/
  
  new_item.static_id.network_id = MY_HOST_ID;
  new_item.static_id.local_id = 0l;
  new_item.dropping_id = 0;
  new_item.eVote.sum_limit = 0;
  new_item.eVote.max = 0;
  new_item.eVote.min = 0;
  new_item.eVote.no_in_group = 1;
  new_item.eVote.author = getuid();
  new_item.eVote.vstatus = NOT_KNOWN;
  new_item.eVote.priv_type = PUBLIC;
  
  /*** CREATE PLAIN ITEM FOR DROPPING ***/
  /*  not tallied */
  new_item.eVote.type = PLAIN;
  new_item.eVote.no_in_group = 1;
  new_item.eVote.more_to_come = 0;
  send_check(&new_item,1, "#1 PLAIN to DROP",verbose);
  dropper[0][0] = item_info[*p_no_items].static_id.local_id;
  dropper[0][1] = 1l;
  
  /*** CREATE PLAIN ITEM ***/
  /*  not tallied */
  new_item.eVote.type = PLAIN;
  new_item.eVote.no_in_group = 1;
  new_item.eVote.more_to_come = 0;
  send_check(&new_item,1, "#1 PLAIN",verbose);
  
  
  /***  CREATE TALLIED YES/NO ITEM ***/
  /* tallied, not grouped */
  new_item.eVote.type = TALLIEDY;
  new_item.eVote.max = 1;
  new_item.eVote.min = 0;
  new_item.eVote.priv_type = PUBLIC; 
  new_item.eVote.vstatus = UNSEEN;
  new_item.eVote.no_in_group = 1;
  send_check(&new_item, 1, "#2 TALLIED YES/NO",verbose);
  
  /***  CREATE TALLIED <=9 ITEM ***/
  /* tallied, not grouped */
  new_item.eVote.type = TALLIEDN;
  new_item.eVote.max = 5;
  new_item.eVote.min = 1;
  new_item.eVote.vstatus = OPEN;
  new_item.eVote.priv_type = PUBLIC; 
  new_item.eVote.no_in_group = 1;
  send_check(&new_item, 1, "#3 TALLIED <=9",verbose);
  
  /***  CREATE TALLIED <=9 ITEM ***/
  /* tallied, not grouped */
  new_item.eVote.type = TALLIEDN;
  new_item.eVote.max = 5;
  new_item.eVote.min = 1;
  new_item.eVote.vstatus = OPEN;
  new_item.eVote.priv_type = PUBLIC; 
  new_item.eVote.no_in_group = 1;
  send_check(&new_item, 1, "#3 TO DROP TALLIED <=9",verbose);
  dropper[1][0] = item_info[*p_no_items].static_id.local_id;
  dropper[1][1] = 1l;
  
  /***  CREATE TALLIED >9 ITEM ***/
  /* tallied, not grouped */
  new_item.eVote.type = TALLIEDN;
  new_item.eVote.max = 99;
  new_item.eVote.min = 0;
  new_item.eVote.priv_type = PUBLIC; 
  new_item.eVote.no_in_group = 1;
  send_check(&new_item, 1, "#4 TALLIED > 9",verbose);
  
  /***  CREATE TALLIED >9 ITEM ***/
  /* tallied, not grouped */
  new_item.eVote.type = TALLIEDN;
  new_item.eVote.max = 99;
  new_item.eVote.min = 0;
  new_item.eVote.priv_type = PUBLIC; 
  new_item.eVote.no_in_group = 1;
  send_check(&new_item, 1, "DROP #4 TALLIED > 9",verbose);
  dropper[2][0] = item_info[*p_no_items].static_id.local_id;
  dropper[2][1] = 4l;
  
  /*** CREATE GROUP OF THREE, 5,6, & 7 yes/no ***/
  /* group of 3 */
  make_group(3, GROUPEDY, 2, 0, 1, YES, "DROP #5, 6, 7:GROUPED YES/NO");
  
  /*** CREATE GROUP OF THREE, 5,6, & 7 yes/no ***/
  /* group of 3 */
  make_group(3, GROUPEDY, 2, 0, 1, YES, "#5, 6, 7:GROUPED YES/NO");
  
  /*** CREATE GROUP OF THREE 8,9,10 <= 9***/
  /* group of 3 */
  make_group(3, GROUPEDN, 30, 0, 9, NO, "#8, 9, 10: GROUPED < 9");
  
  /*** CREATE GROUP OF THREE  > 9 11,12,13 ***/
  /* group of 3 */
  make_group(3, GROUPEDN, 30, 0, 99, NO, "#11, 12, 13: GROUPED > 9");
  
  /***  CREATE TALLIED >9 ITEMS - 300 to drop 260 ***/
  /* tallied, not grouped */
  new_item.eVote.type = TALLIEDN;
  new_item.eVote.max = 99;
  new_item.eVote.min = 0;
  new_item.eVote.no_in_group = 1;
  new_item.eVote.priv_type = PUBLIC; 
  for (i = 0; i < 20; i++)
    {
      send_check(&new_item, 1, "300 BIG TALLIED ",verbose);
    }
  dropper[3][0] = item_info[*p_no_items].static_id.local_id;
  dropper[3][1] = 260l;
  
  for(i = 0; i < 280; i++)
    {
      send_check(&new_item, 1, "300 BIG TALLIED ",verbose);
    }
}










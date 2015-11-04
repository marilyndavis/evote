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

/* $Id: item.c,v 1.3 2003/01/15 18:54:08 marilyndavis Exp $ */ 
/**********************************************************
 *	../eVote/src/eVote/eVoteui/item.c
 *********************************************************
 **********************************************************/
#include <pwd.h>
#include "../eVote.h"
#include "../../Clerklib/Clerk.h"
#include<stdlib.h>
/**********************************************************
 *   This is called when the user indicates that she wants
 *   to change the status of an item.  ch is the character
 *   entered by the user that is 'U','O', or 'L' indicating
 *   that she wants the item UNSEEN, OPENed or CLOSED.
 **********************************************************/
void
change_vstatus(ITEM_INFO *p_item, char ch, YESorNO testing)
{
  RTYPE cc;
  VSTATUS new_vstatus = NOT_KNOWN;
  char status_str[10];
  if (who_am_i() != p_item->eVote.author)
    {
      printf("\nOnly %s can change the status on item #%d.",
	     (getpwuid(p_item->eVote.author))->pw_name, 
	     p_item->dropping_id);
      return;
    }
  if (p_item->eVote.type == PLAIN)
    {
      printf("\nItem #%d is a PLAIN item.  It has no vote status.\n",
	     p_item->dropping_id);
      return;
    }
  switch (ch)
    {
    case 'U':
    case 'u':
      switch (p_item->eVote.vstatus)
	{
	case NOT_KNOWN:
	  printf("Impossible vote status!\n");
	  return;
	case CLOSED:
	  printf("\nItem #%d is CLOSED. It cannot be UNSEEN.\n",
		 p_item->dropping_id);
	  return;
	  break;
	case UNSEEN:
	  printf("\nItem #%d is already UNSEEN.\n",
		 p_item->dropping_id);
	  return;
	  break;
	case OPEN:
	  printf("\nWARNING: If you change the status on item #%d to UNSEEN, ", p_item->dropping_id);
	  printf("\nyou cannot change it back to OPEN.\n");
	  new_vstatus = UNSEEN;
	  strcpy(status_str, "UNSEEN");
	  break;
	}
      break;
    case 'O':
    case 'o':
      switch (p_item->eVote.vstatus)
	{
	case OPEN:
	  printf("\nItem #%d is already OPEN.\n", p_item->dropping_id);
	  return;
	  break;
	case CLOSED:
	  printf("\nOnce item #%d is CLOSED, it cannot be reOPENed.\n",
		 p_item->dropping_id);
	  return;
	case UNSEEN:
	  printf("\nItem #%d is UNSEEN. It cannot be OPENed again.",
		 p_item->dropping_id);
	  printf("\nIt can only be CLOSED to reveal the tally.");
	  return;
	default:
	  break;
	}
      new_vstatus = OPEN;
      strcpy(status_str, "OPEN");
      break;
    case 'L':
    case 'l':
      switch (p_item->eVote.vstatus)
	{
	case OPEN:
	case UNSEEN:
	  printf("\nWARNING: Once item #%d is CLOSED, it cannot be OPENed or UNSEEN again.\n", 
		 p_item->dropping_id);
	  new_vstatus = CLOSED;
	  strcpy(status_str, "CLOSED");
	  break;
	case CLOSED:
	  printf("\nItem #%d is already CLOSED.\n",
		 p_item->dropping_id);
	  return;
	  break;
	case NOT_KNOWN:
	  printf("Impossible return\n");
	  return;
	}
      break;
    default:
      return; /* can't happen */
    }
  if (p_item->eVote.type >= GROUPED)
    {
      short start, end;
      start = p_item->dropping_id + p_item->eVote.more_to_come 
	- p_item->eVote.no_in_group + 1;
      end = p_item->dropping_id + p_item->eVote.more_to_come;
      printf("\nItem #%d is part of a group.  The whole group of %d items,",
	     p_item->dropping_id, p_item->eVote.no_in_group);
      printf("\nitems #%d through #%d, will change status to %s.\n",
	     start, end, status_str);
    }
  else
    {
      printf("\n\nChanging vote status on %d:%c%s%c to %s. ",
	     p_item->dropping_id, 34,
	     print_title(make_dropping(p_item->static_id.local_id)), 
	     34, status_str);
				/* There we used make_dropping on the static_id just in
				   case the items have shifted */
    }
  if (testing == NO)
    {
      if (*GetArg("\nIs this OK?  ") != 'y')  /* user input */
	return;							
    }
  switch (cc = send_vstatus(p_item, new_vstatus))
    {
    case FAILURE:
      printf("\nUnable to send or receive message.");
      return;
    case NO_CHANGE:
      printf("\nchange_vstatus: Illegal change requested.");
      return;
    case NO_ITEM:
      printf("\nchange_vstatus: item #%d, %lu:%lu not found.",
	     p_item->dropping_id, p_item->static_id.network_id, 
	     p_item->static_id.local_id);
      return;
      break;
    case REDUNDANT:
      printf("\nVoting on Item #%d is %s.", p_item->dropping_id,
	     status_str);
      break;
    case NOT_GOOD:
      printf("\neVote was unable to store the new vstatus.");
      printf("\nPlease notify support.");
      return;
      break;
    case DONE:
      if (p_item->eVote.type >= GROUPED)
	{
	  printf("\nVoting on the group of %d items that includes",
		 p_item->eVote.no_in_group);
	  printf("\nitem #%d is now %s.", p_item->dropping_id, 
		 status_str);
	}
      else
	{
	  printf("\nVoting is now %s on item #%d.",
		 status_str, p_item->dropping_id);
	}
      p_item->eVote.vstatus = new_vstatus;
      break;
    default:
      fprintf(stderr,"\nUnexpected return:%d in change_vstatus()\n", cc);
    }
  return;
}		
/**********************************************************
 *   Drops items with dropping ids from start to end.  It 
 *		drops more if start or end is in the middle of the 
 *		group so that it always drops a whole group.
 *   Because it may decide to drop more items than the 
 *		user originally asked for, it offers a chance to
 *		back out.
 *   Returns the number of items dropped.
 *******************************************************/
short
process_drops(short *start, short *end, YESorNO testing)
{
  /* Note that the call to drop_items expects to receive
     a list of ITEM_INFO's.  It's best not to have drops
     point into the item_info array in shared memory because
     it could change out from under us and confuse things.*/
  unsigned int author = 0;
  YESorNO changed_ends = NO;
  ITEM_INFO * drops;
  short i, j;
  short no_drops;
  OKorNOT all_ok = OK;
  unsigned long my_uid;
  char answer;
  if (*start > *p_no_items || *start < 1 ||
      *end > *p_no_items || *end < 1)
    {
      printf("\nIllegal arguments in process_drop():\n   *start = %d, *end = %d : *p_no_items = %d", 
	     *start, *end, *p_no_items);
      return 0;
    }
  /*  be sure that if a member of a group is dropped, 
      the whole group is dropped  */
  if (item_info[*start].eVote.type >= GROUPED)
    {
      while (*start > 1 
	     && item_info[*start].eVote.no_in_group 
	     != item_info[*start].eVote.more_to_come +1)
	{
	  (*start)--;
	  changed_ends = YES;
	}
    }
  if (item_info[*end].eVote.type >= GROUPED)
    {
      while (item_info[*end].eVote.more_to_come != 0 
	    && *end < *p_no_items)
	{
	  (*end)++;
	  changed_ends = YES;
	}
    }
  no_drops = *end - *start + 1;
  my_uid = who_am_i();
  drops = malloc(no_drops * sizeof (ITEM_INFO ));
  if (drops == NULL)
    {
      printf("\nThere are no system resources to process drops right now.");
      printf("\nPlease try again later.");
      return 0;
    }
  for (i = 0, j= *start ; i < no_drops; i++, j++)
    {
      drops[i] = item_info[j];
      if (drops[i].static_id.network_id != MY_HOST_ID)
	{
	  printf("\nItem # %d can't be dropped from this host.",
		 drops[i].dropping_id);
	  all_ok = NOT_OK;
	  break;
	}
      if (drops[i].eVote.author != my_uid 
	  && author != drops[i].eVote.author
	  && does_uid_exist(author = drops[i].eVote.author) == YES )
	{
	  if (drops[i].eVote.type >= GROUPED)
	    {
	      printf("\nYou are not the author of the group: item #%d to #%d; so you can't drop it.",
		     drops[i].dropping_id, 
		     drops[i].dropping_id + drops[i].eVote.more_to_come);
	    }
	  else
	    printf("\nYou are not the author of Item # %d so you can't drop it.",
		   drops[i].dropping_id);
	  all_ok = NOT_OK;
	  break;
	}
    }
  if ((all_ok == OK && 
       item_info[*start].static_id.local_id != drops[0].static_id.local_id)
       || (item_info[*end].static_id.local_id 
	   != drops[no_drops-1].static_id.local_id))
    {
      printf("\nSomeone else is dropping items right now.  Please try again.");
      all_ok = NOT_OK;
    }
  if (all_ok == OK)
    {
      do
	{
	  if (no_drops > 1)
	    {
	      if (changed_ends == YES)
		{
		  printf("\nThe whole group must be dropped if any of the items in the ");
		  printf("\ngroup is dropped. \n\n");
		}
	      printf("\nDropping all items from \n   #%3d : %c%s%c \nto \n   #%3d : %c%s%c",
		     drops[0].dropping_id, 34,
		     print_title(make_dropping(drops[0].static_id.local_id)),
		     34, drops[no_drops-1].dropping_id, 34,
		     print_title(make_dropping(drops[no_drops-1].static_id.local_id)),
		     34);
	    }
	  else /* only one  to drop */
	    {
	      printf("\nDropping item #%d : %c%s%c",
		     drops[0].dropping_id, 34,
		     print_title(make_dropping(drops[0].static_id.local_id)),
		     34);
	    }
	  if (testing == YES)
	    answer = 'y';
	  else
	    if ((answer = eVote_askyn(DROPS_OK)) == 'n'
	       || answer == 'q')
	      all_ok = NOT_OK;
	}
      while (answer == '?');
    }
  if (all_ok != OK)
    {
      free(drops);
      return 0;
    }
  no_drops  = drop_items(drops, no_drops);
  free (drops);
  return no_drops;
}
/**************************************************************
 *   This group of functions is for querying the user about how 
 *   to set up the vote on an item.  The mother function is 
 *   OKorNOT collect_eVote_details(...). 
 *   These functions accept user input directly through 
 *   calls to GetArg(), eVote_askyn(), and eVote_asknum().
 *   GetArg(), eVote_askyn(), and eVote_asknum() are 
 *   defined in eVote_io.c.  You may want to change them
 *   there so that eVote's input stream is the same stream
 *   as the rest of the application.
 ***************************************************************
  * Asks the user a bunch of questions about the item pertaining
  * to the type of vote.  It places the info gathered into 
  * **pp_your_item and **pp_eVote_item.
  * Returns OK if everything is OK, and NOT_OK if user indicated 
  * that she wants to quit.
  * *pp_eVote_item and *pp_your_item can be arrays of items.  
  * Indeed they must be we are
  * asking this function to check a group.  In that case,
  * it was called from set_up_item_group(), below.
  * checking is YES if we already have values in p_eVote_item but
  * we want to check them.  It is NO if we are gathering
  * values for the first time.  Checking == YES only for a
  * group.
  * For a single item, checking happens automatically in this
  * function.
  *************************************************************/
OKorNOT
collect_eVote_details(YOUR_ITEM ** pp_your_item,
			      ITEM_INFO** pp_eVote_item, 
			      YESorNO checking,
			      short lines_printed)
{
  OKorNOT cc = OK;
  int changeno = 0;
  int field;
  int i, line = 0;
  MODE mode = GET;
  YESorNO need_show_answer = YES;
  ITEM_INFO* p_eVote_item = *pp_eVote_item;
  YOUR_ITEM* p_your_item = *pp_your_item;
  OKorNOT do_group(int *pfield, ITEM_INFO *p_eVote_item, 
		   MODE *pmode, int changeno);
  OKorNOT do_max(int *pfield, ITEM_INFO *p_eVote_item, 
		 MODE *pmode, int changeno);
  OKorNOT do_min(int *pfield, ITEM_INFO *p_eVote_item,
		 MODE *pmode, int changeno);
  OKorNOT do_number_in_group(int *pfield, YOUR_ITEM **pp_your_item,
			     ITEM_INFO **pp_eVote_item, 
			     MODE *pmode, int changeno);
  OKorNOT do_perm(int *pfield, ITEM_INFO *p_eVote_item, 
		  MODE *pmode, int changeno);
  OKorNOT do_status(int *pfield, ITEM_INFO *p_eVote_item,
		    MODE *pmode, int changeno);
  OKorNOT do_sum_limit(int *pfield, ITEM_INFO *p_eVote_item, 
		       MODE *pmode, int changeno);
  OKorNOT do_tally(int *pfield, ITEM_INFO *p_eVote_item, MODE * pmode, 
		   int changeno);
  OKorNOT do_yours(int *pfield, YOUR_ITEM* p_your_item,
		   ITEM_INFO *p_eVote_item, 
		   MODE * pmode, int changeno);
  short help_her(MODE pmode, YESorNO checking);
  if (checking == YES)
    mode = SHOW;
  if (mode == GET)  /* if it's GET, we don't know if 
		       it's a group yet */
    {
      p_eVote_item[0].eVote.more_to_come = 0;
      p_eVote_item[0].eVote.no_in_group = 1;
      p_eVote_item[0].eVote.author = who_am_i();
    }
  while (1)
    {
      if (cc == UNDECIDED)  /* some confusion down the line */
	{
	  if (mode == FALL)
	    mode = SHOW;
	  line = help_her(mode, checking);
	  cc = OK;
	}
      field = 0;
      /****
	   These do_xxx() functions return:
	   OK if the question was appropriately answered. 
	   UNDECIDED if the user was very confused. 
	   NOT_OK if there was a system resource problem.
	   STOP if the user indicated she wanted to quit.
      ****/
      do
	{
	  if ((cc = do_yours(&field, p_your_item, p_eVote_item, 
			      &mode, changeno)) == NOT_OK 
	      || (cc == STOP && eVote_askyn(REALLY_QUIT) == 'y' ))
	    {
	      return NOT_OK;
	    }
	}
      while (cc == STOP);
      if (cc == UNDECIDED)
	continue;
      do
	{
	  if ((cc = do_tally(&field, p_eVote_item, &mode, 
			      changeno)) == NOT_OK 
	      || (cc == STOP && eVote_askyn(REALLY_QUIT) == 'y' ))
	    {
	      return NOT_OK;
	    }
	}
      while (cc == STOP);
      if (cc == UNDECIDED)
	continue;
      if (p_eVote_item[0].eVote.type > PLAIN)
	{
	  do
	    {
	      if ((cc =do_status(&field, p_eVote_item, &mode, changeno)) 
		  == NOT_OK
		  || (cc == STOP && eVote_askyn(REALLY_QUIT) == 'y' ))
		{
		  return NOT_OK;
		}
	    }
	  while (cc == STOP);
	  if (cc == UNDECIDED)
	    continue;
	  do
	    {
	      if ((cc =do_perm(&field, p_eVote_item, &mode, changeno)) 
		  == NOT_OK
		  || (cc == STOP && eVote_askyn(REALLY_QUIT) == 'y' ))
		{
		  return NOT_OK;
		}
	    }
	  while (cc == STOP);
	  if (cc == UNDECIDED)
	    continue;
	  do
	    {
	      if ((cc = do_group(&field, p_eVote_item, 
				 &mode, changeno)) == NOT_OK
		  || (cc == STOP && eVote_askyn(REALLY_QUIT) == 'y' ))
		{
		  return NOT_OK;
		}
	    }
	  while (cc == STOP);
	  if (cc == UNDECIDED)
	    continue;
	  if (p_eVote_item[0].eVote.type >= GROUPED)
	    {							
	      do
		{
		  if ((cc = do_number_in_group(&field, pp_your_item, 
					       pp_eVote_item, &mode, changeno)) == NOT_OK
		      || (cc == STOP && eVote_askyn(REALLY_QUIT) == 'y' ))
		    {
		      return NOT_OK;
		    }
		}
	      while (cc == STOP);
	      if (cc == UNDECIDED)
		continue;
	      p_eVote_item = *pp_eVote_item;
	    }
	  if (p_eVote_item[0].eVote.type >= GROUPED)									
	    {
	      do
		{
		  if ((cc = do_sum_limit(&field, p_eVote_item, 
					 &mode, changeno)) == NOT_OK
		      || (cc == STOP && eVote_askyn(REALLY_QUIT) == 'y' ))
		    {
		      return NOT_OK;
		    }
		}
	      while (cc == STOP);
	      if (cc == UNDECIDED)
		continue;
	      if (mode == GET)
		return OK;
	    }
	  /*  Grouped items are collected in set_up_item_group(), below.
	      This is so that the text is collected along with the eVote 
	      details.  
	      set_up_item_group() calls collect_evote_details(pp_your_item,
	      pp_eVote_item, YES), this function,
	      after collecting all the items so that this
	      function can report the details and allow the user to
	      change them.  So if mode == SHOW or FALL, we go on. */
	  need_show_answer = YES;
	  line += (6 + lines_printed);
	  lines_printed = 0;
	  for (i = 0; i < p_eVote_item[0].eVote.no_in_group &&
		 cc != STOP; i++)
	    {
	      if ((line += 5) >= 23 && mode == SHOW)
		{
		  printf("\nAre these OK?  More coming... ");
		  if ((changeno = eVote_asknum(CHANGEIT, 1, field, NO))
		      != ENOREPLY)
		    {
		      need_show_answer = NO;
		      break;
		    }
		  line = 5;
		}
	      if (p_eVote_item[0].eVote.type >= GROUPED )
		{
		  if (mode != CHANGE && mode != FALL)
		    {
		      printf("\n\nItem %d of %d:", i+1,
			     p_eVote_item[0].eVote.no_in_group);
		    }
		  /* good to reshow first title here */
		  do
		    {
		      if ((cc = do_yours(&field, &p_your_item[i],
					 &p_eVote_item[i], 
					 &mode, changeno)) == NOT_OK
			  || (cc == STOP 
			      && eVote_askyn(REALLY_QUIT) == 'y' ))
			{
			  return NOT_OK;
			}
		    }
		  while (cc == STOP);
		  if (cc == UNDECIDED)
		    break;
		}
	      do
		{
		  if ((cc = do_min(&field, &p_eVote_item[i], 
				   &mode, changeno)) == NOT_OK
		      || (cc == STOP && eVote_askyn(REALLY_QUIT) == 'y' ))
		    {
		      return NOT_OK;
		    }
		}
	      while (cc == STOP);
	      if (cc == UNDECIDED)
		break;
	      do
		{
		  if ((cc = do_max(&field, &p_eVote_item[i], 
				   &mode, changeno)) == NOT_OK
		      || (cc == STOP && eVote_askyn(REALLY_QUIT) == 'y' ))
		    {
		      return NOT_OK;
		    }
		}
	      while (cc == STOP);
	      if (cc == UNDECIDED)
		break;
	    }  /* loop through items */
	}  /* if > PLAIN */
      if (cc == UNDECIDED)
	continue;
      switch (mode)
	{
	default:
	  /* impossible */
	  break;
	case GET:
	  mode = SHOW;
	  break;
	case FALL:  /* CHANGE is changed to FALL
		       in the do_xxx() where the change is found */
	  mode = SHOW;
	  line = 0;
	  break;
	case SHOW:
	  if (need_show_answer == YES)
	    changeno = eVote_asknum(CHANGEIT, 1, field, NO);
	  switch (changeno)
	    {
	    case EQUIT:
	      if (eVote_askyn(REALLY_QUIT) == 'y')
		return NOT_OK;
	    case EQUESTION:
	      cc = UNDECIDED;  /* flag to help user */
	      break;
	    case ENOREPLY:  /* all is OK */
	      return OK;
	      break;
	    default:
	      mode = CHANGE;
	      break;
	    }
	}  /* switch (mode) */
    }  /* while (1) */
  return OK;   /* never gets here */
}
/******************************************************/
OKorNOT
do_group(int *pfield, ITEM_INFO *p_eVote_item, MODE *pmode, 
		 int changeno)
{
  int mistakes = 0;
  char *report;
  ++(*pfield);	  
  while (mistakes++ < MISTAKES)
    {
      switch (*pmode)
	{
	case FALL:
	  return OK;
	case CHANGE:
	  if (changeno != *pfield)
	    return OK;
	  /* else fall into GET */
	  *pmode = FALL;  /*  so that it will fall 
			      through the other fields*/
	case GET:
	  switch (eVote_askyn(GROUPIT))
	    {
	    case 'n':
	      p_eVote_item->eVote.type = TALLIED;
	      p_eVote_item->eVote.more_to_come = 0;
	      p_eVote_item->eVote.no_in_group = 1;
	      return OK;
	    case 'y':
	      p_eVote_item->eVote.type = GROUPED;
	      return OK;
	      break;
	    case 'q':
	      return STOP;
	      break;
	    case '?':
	      printf("\nAre you entering a group of items that are related by a question?  The");
	      printf("\nquestion could be something like, ");
	      printf("'Vote YES on 1 of the next 3 items' ");
	      printf("\nor 'Distribute 100 votes over the next 15 items.'");
	      printf("\n\nIf so, answer 'y' for YES.  Otherwise, answer 'n' for NO.\n\n");
	      break;
	    default:
	      return OK;
	    }
	  break;
	case SHOW:
	  if (p_eVote_item->eVote.type >= GROUPED)
	    report = "Yes";
	  else
	    report = "No";
	  printf("\n   %d: %s %s", *pfield, 
		 ask[GROUPIT].qprompt, report);
	  return OK;
	  break;
	}
    }
  return UNDECIDED;
}				
/******************************************************
 * This expects that min and sum-limit(if it's grouped) 
 * are already in the structure.
 **********************************************************/
OKorNOT
do_max(int *pfield, ITEM_INFO *p_eVote_item, 
	       MODE *pmode, int changeno)
{
  short cc;
  int mistakes = 0;
  char show_str[4];
  ++(*pfield);
  while (mistakes++ < MISTAKES)
    {
      switch (*pmode)
	{
	case FALL:
	  return OK;
	case CHANGE:
	  if (changeno != *pfield)
	    return OK;
	  /* else fall into GET */
	  *pmode = FALL;  /*  so that it will fall 
			      through the other fields*/
	case GET:
	  if (*pmode != FALL && p_eVote_item->eVote.min == 0
	       && p_eVote_item->eVote.max == 1)  /* Yes/No question */
	    return OK;
	  switch (cc = eVote_asknum(MAX, 
				   (short)p_eVote_item->eVote.min + 1,
				   (p_eVote_item->eVote.type >= GROUPED 
				    ? p_eVote_item->eVote.sum_limit : 99), 
				   (p_eVote_item->eVote.min? NO : YES)))
	    {
	    case EQUIT:
	      return STOP;
	      break;
	    case ENOREPLY:
	    case EQUESTION:
	      printf("\nPlease enter the maximum vote allowed on this item.\n\n");
	      break;
	    default:
	      p_eVote_item->eVote.max = cc;
	      return OK;
	    }
	  break;
	case SHOW:
	  if (p_eVote_item->eVote.min == 0 
	      && p_eVote_item->eVote.max == 1)
	    strcpy(show_str, "YES");
	  else
	    sprintf(show_str,"%d",
		    (short)p_eVote_item->eVote.max);
	  printf("\n   %d: %s %s", 
		 *pfield, ask[MAX].qprompt,	show_str);
	  return OK;
	  break;
	}
    }
  return UNDECIDED;
}	
/******************************************************/
OKorNOT
do_min(int *pfield, ITEM_INFO *p_eVote_item, 
	       MODE *pmode, int changeno)
{
  short cc;
  int mistakes = 0;
  char show_str[4];
  short max_min = 98;
  ++(*pfield);
  while (++mistakes < MISTAKES)
    {
      switch (*pmode)
	{
	case FALL:
	  return OK;
	case CHANGE:
	  if (changeno != *pfield)
	    return OK;
	  /* else fall into GET */
	  *pmode = FALL;  /*  so that it will fall 
			      through the other fields*/
	  max_min = p_eVote_item->eVote.max - 1;
	case GET:
	  if (*pmode != FALL && p_eVote_item->eVote.more_to_come ==
	     p_eVote_item->eVote.no_in_group -1
	     && *pfield != 0 /* flag for same max/min */)
	    {
	      do
		{
		  switch (cc = eVote_askyn(YN_VOTE))
		    {
		    case 'y':
		      p_eVote_item->eVote.min = 0;
		      p_eVote_item->eVote.max = 1;
		      if (p_eVote_item->eVote.type < GROUPED)
			p_eVote_item->eVote.type = TALLIEDY;
		      else
			p_eVote_item->eVote.type = GROUPEDY;
		      return OK;
		      break;
		    case 'n':
		      break;
		    case 'q':
		      return STOP;
		      break;
		    case '?':
		      printf("\nEnter 'y' for YES if your item will be answered YES or NO.\nOtherwise enter 'n' for NO.\n");
		      break;
		    }
		}while (cc == '?');
	    }  /* if *pmode != FALL */
	  switch (cc = eVote_asknum(MIN, 0, max_min, YES))
	    {
	    case EQUIT:
	      return STOP;
	      break;
	    case ENOREPLY:
	    case EQUESTION:
	      if (mistakes + 1 < MISTAKES)
		printf("\nPlease enter the minimum vote allowed on the item.\n\n");
	      break;
	    default:
	      p_eVote_item->eVote.min = cc;
	      return OK;
	    }
	  break;
	case SHOW:
	  sprintf(show_str,"%d", p_eVote_item->eVote.min);
	  if (p_eVote_item->eVote.min == 0 
	      && p_eVote_item->eVote.max == 1)
	    {
	      if (p_eVote_item->eVote.type < GROUPED)
		p_eVote_item->eVote.type = TALLIEDY;
	      if (p_eVote_item->eVote.type == TALLIEDY
		 || p_eVote_item->eVote.type == GROUPEDY)
		strcpy(show_str, "NO");
	    }
	  else
	    {
	      if (p_eVote_item->eVote.type < GROUPED)
		p_eVote_item->eVote.type = TALLIEDN;
	    }
	  printf("\n   %d: %s %s", 
		 *pfield, ask[MIN].qprompt,	show_str);
	  return OK;
	  break;
	}
    }
  return UNDECIDED;
}				
/******************************************************/
OKorNOT
do_number_in_group(int *pfield, YOUR_ITEM **pp_your_item,
			   ITEM_INFO **pp_eVote_item, 
			   MODE *pmode, int changeno)
{
  OKorNOT do_min(int * pfield, ITEM_INFO *p_eVote_item,
		 MODE *pmode, int changeno);
  OKorNOT do_max(int * pfield, ITEM_INFO *p_eVote_item, 
		 MODE *pmode, int changeno);
  OKorNOT do_yours(int *pfield, YOUR_ITEM *p_your_item,
		   ITEM_INFO *p_eVote_item, 
		   MODE * pmode, int changeno);
  short answer;
  int mistakes = 0, i;
  short old_number;
  MODE pass_mode = GET;
  OKorNOT ret_code;
  ITEM_INFO *p_eVote_item = *pp_eVote_item;
  YOUR_ITEM *p_your_item = *pp_your_item;
  ++(*pfield);
  while (mistakes++ < MISTAKES)
    {
      switch (*pmode)
	{
	case FALL:
	  return OK;
	case CHANGE:  /* we have CHECKING == YES */
	  if (changeno != *pfield && changeno != 0)
	    return OK;
	  /* else fall into GET */
	  old_number = p_eVote_item->eVote.no_in_group;
	  *pmode = FALL;  /*  so that it will fall 
			      through the other fields*/
	case GET:
	  switch (answer = eVote_asknum(NO_TOGETHER, 
				       2, MAX_ITEMS_IN_GROUP, NO))
	    {
	    case EQUIT:
	      return STOP;
	      break;
	    case ENOREPLY:
	    case EQUESTION:
	      printf("\nYou have indicated that you want this item to be grouped with ");
	      printf("\nother items.  How many items will be in this group? \n\n");
	      break;
	    default:
	      p_eVote_item->eVote.no_in_group = answer;
	      p_eVote_item->eVote.more_to_come = answer - 1;
	      if (*pmode == FALL)
		{ /* this means she changed 
		     how many there are */
		  if (answer > old_number)
		    {
		      char *pt;
		      int dummy;
		      if ((pt = realloc((char*)p_your_item,
					       answer*sizeof(YOUR_ITEM)))
			  == NULL)
			{
			  printf("\nNo resources for more items right now.  Try later.");
			  return NOT_OK;
			}
		      p_your_item = (YOUR_ITEM *)pt;
		      *pp_your_item = p_your_item;
		      if ((pt = realloc((char*)p_eVote_item,
					       answer*sizeof(ITEM_INFO)))
			  == NULL)
			{
			  printf("\nNo resources for more items right now.  Try later.");
			  return NOT_OK;
			}
		      p_eVote_item = (ITEM_INFO *)pt;
		      *pp_eVote_item = p_eVote_item;
		      for (i=0; i < old_number; i++)
			{
			  p_eVote_item[i].eVote.more_to_come = answer-1-i;
			  p_eVote_item[i].eVote.no_in_group = answer;
			}
		      /* This function can be called from 
			 set_up_item_group() if a mistake has been
			 detected.  In this case, we don't want to
			 collect anything more.  If so, changeno == 0,
			 and we test for that and return. */
		      for (; i < answer; i++)
			{
			  p_eVote_item[i] = p_eVote_item[0];
			  p_eVote_item[i].eVote.more_to_come = answer-1-i;
			  p_eVote_item[i].eVote.no_in_group = answer;
			  if (changeno == 0)
			    continue;
			  printf("\nItem %d of %d:\n", i+1, answer);
			  if ((ret_code = do_yours(&dummy, 
						    &p_your_item[i], &p_eVote_item[i],
						    &pass_mode, 0)) != OK
			       || (ret_code = do_min(&dummy, 
						     &p_eVote_item[i], &pass_mode, 0)) != OK
			       || (ret_code = do_max(&dummy, 
						     &p_eVote_item[i], &pass_mode, 0)) != OK)
			    return ret_code;
			}
		    }
		  else if (answer < old_number)
		    {
		      for (i=0; i < answer; i++)
			{
			  p_eVote_item[i].eVote.more_to_come = answer -1 -i;
			  p_eVote_item[i].eVote.no_in_group = answer;
			}
		    }
		}
	      if (answer == 1)
		p_eVote_item[0].eVote.type = TALLIED;
	      return OK;
	    }
	  break;
	case SHOW:
	  printf("\n   %d: %s %d", *pfield, 
		 ask[NO_TOGETHER].qprompt,
		 p_eVote_item->eVote.no_in_group);
	  return OK;
	  break;
	}
    }
  return UNDECIDED;
}				
/******************************************************/
OKorNOT
do_perm(int *pfield, ITEM_INFO *p_eVote_item, 
		MODE *pmode, int changeno)
{
  char ans;
  int mistakes = 0;
  char *report;
  ++(*pfield);
  while (mistakes++ < MISTAKES)
    {
      switch (*pmode)
	{
	case FALL:
	  return OK;
	case CHANGE:
	  if (changeno != *pfield)
	    return OK;
	  /* else fall into GET */
	  *pmode = FALL;  /*  so that it will fall 
			      through the other fields*/
	case GET:
	  do
	    {
	      switch (ans = eVote_askyn(PRIV_HOW))
		{
		case 'q':
		  return STOP;
		  break;
		case '?':
		  printf("\nDo you want the vote on this item to be PUBLIC, like a show");
		  printf("\nof hands?  If so, all users will be able to see each others'");
		  printf("\nvotes.  Enter 'y' for YES or 'n' for NO.\n\n");
		  break;
		case 'y':
		  p_eVote_item->eVote.priv_type = PUBLIC;
		  return OK;
		case 'n':
		  /* else it's not public */
		  break;
		}
	    }
	  while (++mistakes < MISTAKES && ans == '?');
	  mistakes = 0;
	  if (ans == '?')
	    return UNDECIDED;  /* to many mistakes and still '?' */
	  do
	    {
	      switch (ans = eVote_askyn(PRIV_IF))
		{
		case 'q':
		  return STOP;
		  break;
		case '?':
		  printf("\nYou already indicated that the votes should not be PUBLIC.");
		  printf("\nDo you want users to be able to see IF other users have voted?\n\n");
		  break;
		case 'y':
		  p_eVote_item->eVote.priv_type = IF_VOTED;
		  return OK;
		  break;
		case 'n':
		  p_eVote_item->eVote.priv_type = PRIVATE;
		  return OK;
		  break;
		}
	    }
	  while (++mistakes < MISTAKES && ans == '?');
	  if (ans == '?')
	    return UNDECIDED;
	  break;
	case SHOW:
	  if (p_eVote_item->eVote.priv_type == PRIVATE)
	    report = "PRIVATE";
	  else if (p_eVote_item->eVote.priv_type == PUBLIC)
	    report = "PUBLIC";
	  else if (p_eVote_item->eVote.priv_type == IF_VOTED)
	    report = "IF-VOTED";
	  printf("\n   %d: %s %s", *pfield, "Permission type? ",
		 report);
	  return OK;
	  break;
	}
    }
  return UNDECIDED;
}
/***********************************************************
 *     This function is different from the other do_xxx()'s.
 *     Notice first that it returns a YES or NO.  It is
 *     called by set_up_item_group() in eVote_demo.c.
 **********************************************************/
YESorNO
do_same_limits(ITEM_INFO *p_eVote_item)
{
  YESorNO same_limits = MAYBE;
  short mistakes = 0;
  do
    {
      switch (eVote_askyn(SAME_LIMITS))
	{
	case 'q':
	  if (eVote_askyn(REALLY_QUIT) == 'y')
	    return PUNT;
	  break;
	case '?':
	  printf("\nIf you answer 'n' for NO, you will be asked for the maximum and");
	  printf("\nminimum vote for each of your %d items.  If you answer 'y' for YES,", p_eVote_item->eVote.no_in_group);
	  printf("\nyou will only be asked once for the maximum and minimum.  Then");
	  printf("\neach of your %d items will have the same maximum and minimum vote.\n\n", p_eVote_item->eVote.no_in_group);
	  break;
	case 'y':
	  same_limits = YES;
	  break;
	default:
	  same_limits = NO;
	  break;
	}
    }
  while (++mistakes < MISTAKES && same_limits == MAYBE);
  return same_limits;
}
/******************************************************/
OKorNOT
do_sum_limit(int *pfield, ITEM_INFO *p_eVote_item, 
		     MODE *pmode, int changeno)
{
  short answer;
  int mistakes = 0;
  ++(*pfield);
  while (mistakes++ < MISTAKES)
    {
      switch (*pmode)
	{
	case FALL:
	  return OK;
	case CHANGE:
	  if (changeno != *pfield && changeno != 0)
	    return OK;
	  /* else fall into GET */
	  *pmode = FALL;  /*  so that it will fall 
			      through the other fields*/
	case GET:
	  switch (answer = eVote_asknum(SUM_LIMIT, 1, 
				       99 * MAX_ITEMS_IN_GROUP, NO))
	    {
	    case EQUIT:
	      return STOP;
	      break;
	    case ENOREPLY:
	    case EQUESTION:
	      printf("\nIf the directions for this group will be:");
	      printf("\n  'Vote YES on 3 of the next 5 items.' ");
	      printf("\nthe sum-limit should be '3'.  If the instructions are:");
	      printf("\n  'Distribute 100 votes over the next 12 items.'");
	      printf("\nthe sum-limit should be '100'.\n\n");
	      break;
	    default:
	      p_eVote_item->eVote.sum_limit = answer;
	      return OK;
	    }
	  break;
	case SHOW:
	  printf("\n   %d: %s %d", *pfield, 
		 ask[SUM_LIMIT].qprompt,
		 p_eVote_item->eVote.sum_limit);
	  return OK;
	  break;
	}
    }
  return UNDECIDED;
}				
/******************************************************/
OKorNOT
do_status(int *pfield, ITEM_INFO *p_eVote_item, MODE * pmode, 
		  int changeno)
{
  char answer;
  int mistakes = 0;
  char *report;
  ++(*pfield);
  while (mistakes++ < MISTAKES)
    {
      switch (*pmode)
	{
	case FALL:
	  return OK;
	case CHANGE:
	  if (changeno != *pfield)
	    return OK;
	  /* else fall into GET */
	  *pmode = FALL;  /*  so that it will fall 
			      through the other fields*/
	case GET:
	  switch (answer = eVote_askyn(SEE_IT))
	    {
	    case 'q':
	      return STOP;
	      break;
	    case '?':
	      printf("\nAnswer `y' for YES if you want the vote tally to be visible on the");
	      printf("\ncontents screen while the vote is in progress.  Otherwise, the vote");
	      printf("\ntally will be hidden with *?* until you close the vote.\n\n");
	      break;
	    case 'y':
	      p_eVote_item->eVote.vstatus = OPEN;
	      return OK;
	    case 'n':
	      p_eVote_item->eVote.vstatus = UNSEEN;
	      return OK;
	    }
	  break;
	case SHOW:
	  if (p_eVote_item->eVote.vstatus == OPEN)
	    report = "OPEN";
	  else
	    report = "UNSEEN";
	  printf("\n   %d: Vote status?  %s", *pfield, report);
	  return OK;
	  break;
	}
    }
  return UNDECIDED;
}				
/******************************************************/
OKorNOT
do_tally(int *pfield, ITEM_INFO *p_eVote_item, MODE * pmode, 
		 int changeno)
{
  char answer;
  int mistakes = 0;
  char old_type;
  char *report;
  ++(*pfield);
  while (mistakes++ < MISTAKES)
    {
      switch (*pmode)
	{
	case FALL:
	  return OK;
	case CHANGE:
	  if (changeno != *pfield)
	    return OK;
	  /* else fall into GET */
	  *pmode = FALL;  /*  so that it will fall 
			      through the other fields*/
	  old_type = p_eVote_item->eVote.type;
	case GET:
	  switch (answer = eVote_askyn(VOTED_ON))
	    {
	    case 'q':
	      return STOP;
	      break;
	    case '?':
	      printf("\nDo you want the community to vote on this item?\n\n");
	      break;
	    case 'y':
	      p_eVote_item->eVote.type = TALLIED;
	      if (old_type == PLAIN && *pmode == FALL)
		{
		  p_eVote_item->eVote.more_to_come = 0;
		  p_eVote_item->eVote.no_in_group = 1;
		  *pmode = GET;
		}
	      return OK;
	    case 'n':
	      p_eVote_item->eVote.type = PLAIN;
	      p_eVote_item->eVote.more_to_come = 0;
	      p_eVote_item->eVote.no_in_group = 1;
	      p_eVote_item->eVote.sum_limit = 99;
	      return OK;
	    }
	  break;
	case SHOW:
	  if (p_eVote_item->eVote.type > PLAIN)
	    report = "Yes";
	  else
	    report = "No";
	  printf("\n   %d: %s %s", *pfield, ask[VOTED_ON].qprompt,
		 report);
	  return OK;
	  break;
	}
    }
  return UNDECIDED;
}				
/********************************************************
 *   Prints out a help message for a confused user.  Returns
 *   the number of lines it printed if we're in SHOW mode.
 *********************************************************/
short
help_her(MODE mode, YESorNO checking)
{
  short line = 0;
  if (mode == GET)
    {
      printf("\n\n* * * STARTING OVER * * *");
      printf("\n\nYou indicated that you want to add a new item to the %s", current_conf);
      printf("\nconference.  In order to do so, you must answer some questions");
      printf("\nabout the vote (or no vote) on your new item.\n\n");
    }
  else if (mode == SHOW)
    {
      printf("\n\nYou have composed ");
      if (checking == YES)
	printf("some new items ");
      else
	printf("a new item ");
      printf("for the %s conference.", current_conf);
      printf("\nNow, please check your answers:\n\n");
      line = 4;
    }
  printf("\nEnter '?' to receive more instructions or 'q' to quit.\n");
  printf("\nYou can enter '?' more than once to receive even more instructions.\n\n");
  return line;
}
/***********************************************************
 *   This is called from set_up_item_group() in eVote_demo.c
 *   when the sum_limit < the sum of the min votes.
 ************************************************************/
OKorNOT
limit_exceeded(YOUR_ITEM **p_your_item_list, 
		       ITEM_INFO **p_eVote_item_list, 
		       YESorNO same_limits, int sum_min)
{
  OKorNOT cc;
  int dummy;
  MODE pmode;
  ITEM_INFO *item_list = *p_eVote_item_list;
  OKorNOT do_number_in_group(int *pfield, YOUR_ITEM **p_your_item_list,
			     ITEM_INFO **pp_eVote_item_list, 
			     MODE *pmode, int changeno);
  OKorNOT do_sum_limit(int *pfield, ITEM_INFO *p_eVote_item, 
		       MODE *pmode, int changeno);
  printf("\n\nThere is some confusion here.  Your sum-limit, %d, is smaller than", item_list[0].eVote.sum_limit);
  if (same_limits == YES)
    {
      printf("\nthe sum of your minimum votes, %d = %d X %d.  Let's back up.\n\n", 
	     (sum_min = item_list[0].eVote.no_in_group
	      * item_list[0].eVote.min),
	     item_list[0].eVote.no_in_group, 
	     item_list[0].eVote.min);
    }
  else
    {
      printf("\nthe sum of your minimum votes, %d.  Let's back up.\n\n",
	     sum_min);
    }
  do
    {
      pmode = CHANGE;
      /* do_number_in_group needs the address of the lists
	 because, if the number grows, it reallocs them. */
      if ((cc = do_number_in_group(&dummy, p_your_item_list,
				   p_eVote_item_list,
				   &pmode, 0)) == NOT_OK 
	  || cc == UNDECIDED
	  || (cc == STOP && eVote_askyn(REALLY_QUIT) == 'y' ))
	{
	  return cc;
	}
    }
  while (cc == STOP);
  do
    {
      pmode = CHANGE;
      if ((cc = do_sum_limit(&dummy, &item_list[0], &pmode, 0)) 
	  == NOT_OK || cc == UNDECIDED
	  || (cc == STOP && eVote_askyn(REALLY_QUIT) == 'y' ))
	{
	  return cc;
	}
    }
  while (cc == STOP);
  return OK;
}
/**************************************************************
 *     This is called when adding a group of items.  p_eVote_item 
 *     has the info for the first item of the group except the min
 *     max votes.  We collect those limits here.
 *     This function saves up the info about each item in the 
 *	    group before it locks up the application database and
 *     enters all the items together.  This is accomplished by
 *	    mallocing space for two arrays: your_list[], for holding
 *	    the item's data that the application keeps; and 
 *	    item_list[], for holding the data that eVote keeps.  
 *	    The your_list[] array is an array of struct your_def's, 
 *	    or YOUR_ITEMs that is defined in eVote.h.  You want to
 *	    change the definition so that the members in the structure
 *	    are the things you need to make the call to your
 *	    application's function for storing a new item.
 *******************************************************/
OKorNOT
set_up_item_group(YOUR_ITEM *p_your_item,
			   ITEM_INFO *p_eVote_item)
{
  OKorNOT add_group(YOUR_ITEM *your_list, ITEM_INFO *item_list);
  /* add group is defined below this function 
     - out of alphabetic order */
  OKorNOT do_max(int* field, ITEM_INFO * p_eVote_item, 
		 MODE *pmode, int changeno);
  OKorNOT do_min(int* field, ITEM_INFO * p_eVote_item, 
		 MODE *pmode, int changeno);
  YESorNO do_same_limits(ITEM_INFO *p_eVote_item);
  OKorNOT do_yours(int *pfield, YOUR_ITEM* p_your_item,
		   ITEM_INFO *p_eVote_item, 
		   MODE * pmode, int changeno);
  OKorNOT limit_exceeded(YOUR_ITEM **p_your_item_list,
			 ITEM_INFO **p_eVote_item_list, 
			 YESorNO same_limits, int sum_min);
  OKorNOT verify_group(YOUR_ITEM **p_your_list, ITEM_INFO **p_item_list);
  OKorNOT cc = OK;
  int dummy = 0;
  short i;
  ITEM_INFO *item_list = NULL;
  MODE pmode;
  YESorNO same_limits;
  int sum_min = 0;
  short titles_in_to = 0;  /* where we discovered the user's 
			      mistake that the sum of the mins > sum_limit */
  YOUR_ITEM *your_list = NULL;
  item_list = malloc(p_eVote_item->eVote.no_in_group 
				  * sizeof(ITEM_INFO));
  if (item_list == NULL)
    {
      printf("\nNo system resources.  Try later.");
      return NOT_OK;
    }
  your_list = malloc(p_eVote_item->eVote.no_in_group 
				 * sizeof(YOUR_ITEM));
  if (your_list == NULL)
    {
      free(item_list);
      return NOT_OK;
    }
  do    /* until the sum-limit > sum of mins */
    {
      pmode = GET;
      sum_min = 0;
      if ((same_limits = do_same_limits(p_eVote_item)) == PUNT)
	return NOT_OK;
      if (same_limits == MAYBE)
	return UNDECIDED;
      for (i = 0; i < p_eVote_item->eVote.no_in_group ; i++)
	{
	  item_list[i] = *p_eVote_item;				
	  /*  The id's are set to 0 by this copy.  If you set
	      your own id's, you must fix that up here. */
	  if (i > 0 || same_limits == NO)
	    printf("\nItem %d of %d:\n", i+1, p_eVote_item->eVote.no_in_group);
	  if (i > 0)
	    {
	      /* We don't want to recollect the text and title
		 if we're cycling for a mistake in mins. */
	      if (titles_in_to < i)
		{
		  titles_in_to = i;
		  do
		    {
		      if ((cc = do_yours(&dummy, &your_list[i],
					 &item_list[i], &pmode, 0))
			  == NOT_OK || cc == UNDECIDED
			  || (cc == STOP && eVote_askyn(REALLY_QUIT) == 'y' ))
			{
			  free(your_list);
			  free(item_list);
			  if (cc == UNDECIDED)
			    return cc;
			  return NOT_OK;
			}
		    }
		  while (cc == STOP);
		} 
	      item_list[i].eVote.more_to_come 
		= p_eVote_item->eVote.no_in_group - 1 - i;
	      if (same_limits == YES)
		{	
		  item_list[i].eVote.min = item_list[0].eVote.min;
		  item_list[i].eVote.max = item_list[0].eVote.max;
		  item_list[i].eVote.type = item_list[0].eVote.type;
		}
	    } /* if i > 0 */
	  if (same_limits == NO || i == 0)
	    { 
	      if (same_limits == NO)
		dummy = -1;  /* flag to NOT ask for Yes/No vote */
	      do
		{
		  if ((cc = do_min(&dummy, &item_list[i], &pmode, 0)) 
		      == NOT_OK || cc == UNDECIDED
		      || (cc == STOP && eVote_askyn(REALLY_QUIT) == 'y' ))
		    {
		      free(your_list);
		      free(item_list);
		      if (cc == UNDECIDED)
			return cc;
		      return NOT_OK;
		    }
		}
	      while (cc == STOP);
	      /* - * - * - * - * - * - * - * - * - * - * - * - * - * - * - * 
	       *  The sum_min is the sum of all the min votes for the group.
	       *  We check to be sure that it isn't equal or greater than
	       *  the sum_limit because that doesn't make sense.
	       * - * - * - * - * - * - * - * - * - * - * - * - * - * - * - */
	      if ((same_limits == YES && i == 0
		  && (item_list[0].eVote.sum_limit 
		      <= item_list[0].eVote.min * item_list[0].eVote.no_in_group))
		  || (same_limits == NO 
		      && ((sum_min += item_list[i].eVote.min) 
			  >= item_list[0].eVote.sum_limit)))
		{
		  if ((cc = limit_exceeded(&your_list, &item_list, 
					  same_limits, sum_min)) != OK)
		    {
		      free(your_list);
		      free(item_list);
		      return cc;
		    }
		  *p_eVote_item = item_list[0];
		  sum_min = 32000;  /* trigger loop */
		  break;  /* from collecting loop into error loop */
		}
	      do
		{
		  if ((cc = do_max(&dummy, &item_list[i], &pmode, 0)) 
		      == NOT_OK || cc == UNDECIDED
		      || (cc == STOP && eVote_askyn(REALLY_QUIT) == 'y' ))
		    {
		      free(your_list);
		      free(item_list);
		      if (cc == UNDECIDED)
			return cc;
		      return NOT_OK;
		    }
		}
	      while (cc == STOP);
	    }
        }  /* end of for loop to collect 
	      all the info about an item. */
    }
  while (sum_min > item_list[0].eVote.sum_limit);
  /* - * - * - * - * - * - * - * - * - * - * - * - * - * - * - * 
   * Now all the items' details have been gathered from the user.
   * Here we check the sum_min and give the user a chance to 
   * change things.
   * - * - * - * - * - * - * - * - * - * - * - * - * - * - * - */
  if ((cc = verify_group(&your_list, &item_list))
     == OK)  
    cc = add_group(your_list, item_list);  /* add_group is defined
					      in eVote_demo.c */
  free(your_list);
  free(item_list);
  return cc;
}
/*********************************************************
 *    Checks various things.  Passes the lists back to
 *    collect_eVote_details() to allow the user to change
 *    things and rechecks everything.
 *******************************************************/      
OKorNOT
verify_group(YOUR_ITEM ** p_your_list, 
		     ITEM_INFO ** p_item_list)
{
  short big_max;
  short big_min;
  short i;
  ITEM_INFO *item_list = *p_item_list;
  short lines = 0;
  int sum_min = 0;
  YESorNO yesno;
  YOUR_ITEM *your_list = *p_your_list;
  do  /* until sum_min is good */
    {
      if (sum_min >= item_list[0].eVote.sum_limit)
	{
	  printf("\nThe sum of the minimum votes for this group is %d.",
		 sum_min);
	  printf("\nThe sum_limit for this group is %d.  The sum of the",
		 item_list[0].eVote.sum_limit);
	  printf("\nminimum votes must be less than the sum-limit.");
	  printf("\nPlease fix it.\n");
	  lines += 5;
	}
      /* This call checks the values.  It could come back with a plain
	 or tallied item in item_list[0] -- or it could grow the list */
      if (collect_eVote_details(p_your_list, p_item_list, YES, lines) != OK)
	{
	  return NOT_OK;
	}
      /* These might have changed if the list grew */
      item_list = *p_item_list;
      your_list = *p_your_list;
      lines = 0;
      sum_min = 0;
      yesno = YES;  /* check this again */
      big_max = 0;
      big_min = 0;
      for (i = 0; i < item_list[0].eVote.no_in_group; i++)
	{
	  /* some things the user might have changed were changed
	     only in the first item */
	  item_list[i].eVote.priv_type = item_list[0].eVote.priv_type;
	  item_list[i].eVote.sum_limit = item_list[0].eVote.sum_limit;
	  sum_min += item_list[i].eVote.min;
	  if (item_list[i].eVote.min != 0 
	     || item_list[i].eVote.max != 1)
	    {
	      yesno = NO;
	      if (item_list[i].eVote.max > big_max)
		big_max = item_list[i].eVote.max;
	      if (item_list[i].eVote.min > big_min)
		big_min = item_list[i].eVote.max;
	    }
	}
    }	
  while (sum_min >= item_list[0].eVote.sum_limit);
  for (i = 0 ; i < item_list[0].eVote.no_in_group; i++)
    {
      if (item_list[0].eVote.type >= GROUPED )
	{
	  if (yesno == YES 
	   || (big_max == 1 && big_min == 0))
	    {
	      item_list[i].eVote.type = GROUPEDY;
	    }
	  else
	    {
	      if (big_max > 9)
		{
		  item_list[i].eVote.type = GROUPEDN;
		}
	      else
		{
		  item_list[i].eVote.type = GROUPEDn;
		}
	    }
	}
    }
  return OK;
}

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

/* $Id: poll.c,v 1.4 2003/01/15 18:54:09 marilyndavis Exp $ */ 
/**********************************************************
 *  poll.c  Functions that deal with an established poll.
 *********************************************************
 **********************************************************/
#include<stdio.h>
#include"mailui.h"
   /* for export */
   char author_name[MAX_ADDRESS + 1];
YESorNO just_checking = NO;
/* private */
static void display_closing_stats(void);
static void display_mirror_stats(ITEM_INFO * p_item, YESorNO new_poll);
static void do_copy(int i);
#define AUTHOR_DAYS (28)   /* days that author must wait to drop a poll
			      after it has been closed */
#define OTHERS_DAYS (180) /* same for non-authors */
#define AUTHOR_WAIT  (AUTHOR_DAYS*24*60*60)  /* seconds that the poll  */
#define OTHERS_WAIT  (OTHERS_DAYS*(24*60*60)) /* must be closed before  
						 it is droppable   */
/**********************************************************
 *  Makes space for a copy of the item_info structure(s)
 *  for the subject, copies the current version into
 *  this space and points p_item_copy to it.
 *  The item_info array is maintained in shared memory
 *  and can change from participation from another
 *  process.  So we make a stable copy.
 *  If fussy == NO, we're looking
 *  for a petition that matches the first three chars.
 **********************************************************/
void
copy_poll(YESorNO fussy)
{
  short i, j;
  char * key;
  if (!p_no_items)
    exit(1);
  for (i = 1; i <= *p_no_items; i++)
    {
      if ((same(subject, item_info[i].eVote.title) 
	   && !(item_info[i].eVote.type >= GROUPED
		&& item_info[i].eVote.type <= GROUPEDN)))
	{
	  do_copy(i);
	  if (petition != NO)
	    {
	      if (item_info[i].eVote.type == PLAIN)
		{
		  extra_subject = malloc(strlen(subject) + 1);
		  strcpy(extra_subject, subject);
		  if (same(EXTRA, &subject[strlen(subject)- strlen(EXTRA)]))
		    {
		      extra_subject[strlen(subject) - strlen(EXTRA) - 1] = '\0';
		      for (j = 1; j <= *p_no_items; j++)
			{
			  if (same(extra_subject, item_info[j].eVote.title)
			      && item_info[j].eVote.type == TIMESTAMP)
			    {
			      no_pet_votes = item_info[i+1].eVote.no_in_group;
			      break;
			    }
			}
		    }
		  free(extra_subject);
		  extra_subject = NULL;
		}
	      if (item_info[i].eVote.type == TALLIEDY
		  || item_info[i].eVote.type == TIMESTAMP)
		{
		  petition = YES;
		  no_pet_votes = 0;
		  extra_subject = malloc(strlen(subject) + strlen(EXTRA) + 2);
		  sprintf(extra_subject,"%s %s", subject, EXTRA);
		  for (j = 1; j <= *p_no_items; j++)
		    {
		      if (same(extra_subject, item_info[j].eVote.title)
			  && item_info[j].eVote.type == PLAIN)
			{
			  no_pet_votes = item_info[j+1].eVote.no_in_group;
			  p_first_vote_item = &item_info[j+1];
			  break;
			}
		    }
		  if (!no_pet_votes)
		    {
		      free(extra_subject);
		      extra_subject = NULL;
		    }
		}
	      else
		{
		  petition = NO;
		}
	    }
	  return;
	}
    }
  if (fussy == YES)
    return;
  if (petition == MAYBE)
    {
      char p_s[MAX_KEY_LEN + 1];
      YESorNO eVote_flag = NO;
      strcpy(p_s, pet_file_key(subject));
      for (i = 1; i <= *p_no_items; i++)
	{
	  key = pet_file_key(item_info[i].eVote.title);
	  if (samen(key, p_s,  strlen(key)))
	    {
	      if (!eVote_flag)
		{
		  int len = strlen(item_info[i].eVote.title) - 6;
		  if (same(item_info[i].eVote.title+len, "-eVote"))
		    {
		      eVote_flag = YES;
		      continue;
		    }
		}
	      do_copy(i);
	      petition = PUNT;
	      return;
	    }
	}
    }
  return;
}
/**********************************************************
 * Displays the statistics for the poll.
 * This is only called when the poll is closed.
 **********************************************************/
void
display_closing_stats(void)
{
  unsigned long readers = 0L, voters = 0L, yeses = 0L;
  char result[10] = "  - ";
  int i;
  char vote_str[10] = "  - ";
  char Choice_str[10]="Choice";
  switch (p_item_copy->eVote.type)
    {
    case PLAIN:  /* really GROUPED */
      get_mail_stats((p_item_copy+1)->dropping_id,
		     &readers, vote_str, result);
      voters = get_voters((p_item_copy+1)->dropping_id);
      if ((p_item_copy+1)->eVote.priv_type == PUBLIC
	  && no_of_participants <= CLOSING_SPREAD 
	  && voters > 0 )
	{
	  if (fill_spread() == OK)
	    {
	      gen_spread();
	      finish(0);
	    }
	}
      switch ((p_item_copy+1)->eVote.type)				
	{
	case GROUPEDY:
	  printf("  Yes    No     On      \n"
		 "  Votes  Votes  %s  \n", Choice_str);
	  for (i = 1; i <= (p_item_copy+1)->eVote.no_in_group; i++)
	    {
	      get_mail_stats((p_item_copy+i)->dropping_id,
			     &readers, vote_str, result);
	      yeses = atoul(result);
	      printf("\n%4lu   %4lu    %2d. %s ", yeses, 
		     voters-yeses, i, (p_item_copy+i)->eVote.title);
	    }
	  break;
	case GROUPEDN:
	case GROUPEDn:
	  printf("  Min   Max   Average   On      \n"
		 "  Vote  Vote  Vote      %s  \n", Choice_str);
	  for (i = 1; i <= (p_item_copy+1)->eVote.no_in_group; i++)
	    {
	      get_mail_stats((p_item_copy+i)->dropping_id,
			     &readers, vote_str, result);
	      printf("\n %4d  %4d  %5s    %3d. %s ", 
		     (p_item_copy+i)->eVote.min,
		     (p_item_copy+i)->eVote.max,
		     result,
		     i, (p_item_copy+i)->eVote.title);
	    }
	  break;
	default:
	  /*impossible */
	  break;
	}
      break;
    case TIMESTAMP:
    case TALLIEDY:
    case TALLIEDN:
      fprintf(stderr,"Programming error!  display_closing_stats() called on Tallied Item.\n");
      break;
    default:
      /*impossible */
      break;
    }
}
/********************************************************
 *   Displays information about the poll.
 *   This function cannot rely on the global p_item_copy
 *   because we may be displaying info about a poll
 *   request that is "check" -- not really in the 
 *   data base;
 *********************************************************/
void
display_info(ITEM_INFO *p_item, YESorNO new_poll)
{
  YESorNO isauthor = NO;
  char is[4] = "is";
  int i, v1, v2, j;
  short int item;
  PRIV_TYPE local_ptype;
  char choice_str[10] = "choice";
  if (p_item == NULL)
    {
      p_item = p_item_copy;
      local_ptype = ptype;
    }
  else
    {
      local_ptype = p_item->eVote.priv_type;
      if (p_item->eVote.type == PLAIN)
	local_ptype = (p_item+1)->eVote.priv_type;
    }
  if (mail_voter == p_item->eVote.author)
    isauthor = YES;
  start_displays(p_item);
  display_stats(p_item, new_poll, NO);
  switch (local_ptype)
    {
    case PRIVATE:
      printf("\n");
      highlight("PRIVATE POLL");
      printf("\nThis %s a \"private\" poll; ballots are secret.\n",
	     is);
      break;
    case IF_VOTED:  
      printf("\n");
      highlight("IF_VOTED POLL");
      printf("\nThis %s an \"if-voted\" poll.  This means you can send email to", is);
      printf("\n\n\t%s@%s", list, whereami);
      printf("\n\nwith the subject, \"%s\", \nand with a message that says: ", 
	     subject);
      printf("\n\n         eVote who");
      printf("\n\nto receive a list of those who voted.  Of course, everyone else"
	     "\non the %s list can see if you have voted too.\n", list);
      break;
    case PUBLIC:
      printf("\n");
      highlight("PUBLIC POLL");
      printf("\nThis %s a \"public\" poll.  This means you can send email to ", is);
      printf("\n\n\t%s@%s", list, whereami);
      printf("\n\nwith the subject, \"%s\", \nand with a message that says: ", 
	     subject);
      printf("\n\n         eVote who");
      if (p_item->eVote.type == PLAIN) /* really a group */
	{
	  printf("\n\nto receive a spreadsheet showing everyone's votes.");
	  printf("\n\n         eVote who w=140");
	  printf("\n\nwill produce a spreadsheet 140 columns wide.  The default is 80.");
	}
      else
	{
	  printf("\n\nto receive a list of everyone's votes." );
	}
      printf("\n\nOf course, everyone else on the %s list"
	     "\ncan monitor your votes too.\n", list);
      break;
    }
  if (local_ptype != PUBLIC)
    {
      printf("\nWarnings: Although eVote will not reveal your vote to the other"
	     "\nmembers of the voting community, the system administrator of the"
	     "\ncomputer that stores your vote can quite easily see the voting"
	     "\nrecords of individuals. Also, there is the possibility that your"
	     "\nballot can be seen by a \"snooper\", someone who intercepts your "
	     "\nballot in transit.\n"
	     "\nThe integrity of the poll, i.e., the accuracy of the count, is"
	     "\nsusceptible to being tampered with by the system administrator"
	     "\nof the computer running the eVote(R)/Clerk software."
	     "\n\nBoth the privacy of your vote and the integrity of the poll are"
	     "\nsusceptible to attack at your own computer.\n");
    }
  if (p_item->eVote.vstatus != CLOSED)
    {
      printf("\n");
      highlight("TO VOTE");
      printf("\n | 1.  Send a message to %s@%s.", list, whereami);
      printf("\n |");
      printf("\n | 2.  Your subject must be ");
      printf("\"%s\".", subject);
      printf("\n |     (Don't worry about extra words in the subject line that");
      printf("\n |      reply-to produces.)");
      printf("\n |");
      printf("\n |      * * * * * * * * * * * * * * * * * * * * * * * *"
	     "\n  ----> *  NOTE:  These two steps are easy.  Just use *"
	     "\n        *         your reply-to key on this message!  *"
	     "\n        * * * * * * * * * * * * * * * * * * * * * * * *");
      printf("\n\n   3.  Your message *must* start with the word, \"eVote\", or your"
	     "\n       vote will be sent to the entire %s list and it won't"
	     "\n       be counted!", list);
      switch (p_item->eVote.type)
	{
	case TIMESTAMP:
	  printf("\nProgramming error!");
	  break;
	case TALLIEDY:
	  printf("\n\n       To vote YES, your message should say:\n"
		 "\n         eVote yes"
		 "\n\n       To vote NO, your message should say:\n"
		 "\n         eVote no\n\n");
	  break;
	case TALLIEDN:
	  i = (p_item->eVote.min + p_item->eVote.max)/2;
	  if (i == 0)
	    i = p_item->eVote.max/2;
	  printf("\n\n       To vote %d, your message should say:", i);
	  printf("\n\n          eVote %d", i);
	  printf("\n\n       Your vote can be any number from %d to %d.\n\n",
		 p_item->eVote.min, p_item->eVote.max);
	  break;
	case PLAIN:
	  item = (p_item+1)->eVote.no_in_group/2;
	  if (item == 1)
	    item = 2;
	  switch ((p_item+1)->eVote.type)
	    {
	    case GROUPEDY:  /* just 1 choice not allowed */
	      if ((p_item+1)->eVote.sum_limit > 1)
		{
		  printf("\n\n       To vote yes on choices 1 and %d, your message should say:\n"
			 "\n          eVote "
			 "\n          1. y"
			 "\n          %d. y", item, item);
		  printf("\n\n       You may make your list of votes as long as you wish.");
		}
	      else
		{
		  printf("\n\n       To vote yes on choice %d, your message should say:\n"
			 "\n          eVote "
			 "\n          %d. y", item, item);
		}
	      printf("\n\n       Every choice you don't vote \"yes\" on will receive an automatic"
		     "\n        \"no\" vote.\n\n");
	      break;
	    default:  /* numeric grouped */
	      if ((p_item+1)->eVote.sum_limit == NO_LIMIT)
		strcpy(choice_str,"item");
	      switch (i = (p_item+1)->eVote.no_in_group)
		{
		case 1:
		  printf("\n\n       To vote %d on item 1, your message should say:\n", 
			 v1 = ((p_item+1)->eVote.max 
			       + (p_item+1)->eVote.min)/2);
		  printf("\n          eVote ");
		  printf("\n          1. %d\n", v1);
		  break;
		case 2:
		  v1 = ((p_item+1)->eVote.max + (p_item+1)->eVote.min)/2;
		  if (v1 <= 0)
		    v1 = (p_item+1)->eVote.max/2;
		  v2 = ((p_item+2)->eVote.max + (p_item+2)->eVote.min)/2;
		  if (v2 <= 0)
		    v2 = (p_item+2)->eVote.max/2;
		  if (v2 + v1 <= (p_item+2)->eVote.sum_limit)
		    {
		      printf("\n\n       To vote %d on %s 1 and %d on %s 2, your message should say:\n", 
			     v1, choice_str, v2, choice_str);
		      printf("\n          eVote ");
		      printf("\n          1. %d", v1);
		      printf("\n          2. %d\n", v2);
		    }
		  else
		    {
		      printf("\n\n       To vote %d on %s 2, your message should say:\n",
			     v2, choice_str);
		      printf("\n          eVote ");
		      printf("\n          2. %d\n", v2);
		    }
		  break;
		default:
		  j = 2;
		  do
		    {
		      v1 = ((p_item+2)->eVote.max 
			    + (p_item+2)->eVote.min)/j;
		      if (v1 <= 0)
			v1 = (p_item+2)->eVote.max/j;
		      v2 = ((p_item+i)->eVote.max 
			    + (p_item+i)->eVote.min)/j;  /* problem */
		      if (v2 <= 0)
			v2 = (p_item+i)->eVote.max/j;
		    }
		  while (j++ < 10 && (v1 + v2) 
			 > (p_item+1)->eVote.sum_limit);
		  printf("\n\n       To vote %d on %s 2 and %d on %s %d, your message should say:\n", v1, choice_str, v2, choice_str, i);
		  printf("\n          eVote ");
		  printf("\n          2. %d", v1);
		  printf("\n          %d. %d\n", i, v2);
		  break;
		}
	      if ((p_item+1)->eVote.no_in_group != 1)
		{
		  printf("\n       You can make your list of votes as long as necessary.");
		  printf("\n       Each %s that you don't vote on will automatically receive \n       the minimum vote for that %s.\n\n",
			 choice_str, choice_str);
		}
	      else
		printf("\n");
	      break;
	    }
	  break;
	default:
	  /* impossible */
	  break;
	}
      printf("   4.  If your message has a signature, or any other text"
	     "\n       below your vote, make a line that says, \"end\" just"
	     "\n       after your vote.\n");
      highlight("CHANGING YOUR VOTE");
      printf("\nYou can change your %s while the poll is open by voting again.",
	     (p_item->eVote.type == PLAIN ? "votes" : "vote"));
      printf("\n\n");
      highlight("REMOVING YOUR VOTE");
      printf("\nTo remove your %s on \"%s\", \nsend the message:",
	     (p_item->eVote.type == PLAIN ? "votes" : "vote"),
	     subject);
      printf("\n\n\teVote remove");
      printf("\n\n");
      highlight("SEEING THE RESULTS OF THE POLL");
      if (p_item->eVote.vstatus == UNSEEN)
	{
	  printf("\nYou cannot see the vote tally until the vote closes.\n");
	  printf("Only %s, the originator of this poll, \ncan close it.",
		 author_name);
	}
      else
	{
	  printf("\nTo see the current vote tally on \"%s\", \nsend the message:",
		 subject);
	  printf("\n\n\teVote info");
	}
      printf("\n");
    }
  printf("\n");
  highlight("MORE INFORMATION");
  printf("\nTo receive more information about \"%s\":", subject);
  printf("\n\n1.  Send a message to:\n");
  printf("\n\t%s@%s\n", list, whereami);
  printf("\n2.  Your subject must be:");
  printf("\n\n\t%s", subject);
  switch (local_ptype) 
    {
    case PRIVATE:
      printf("\n\n3.  ");
      break;
    case PUBLIC:
    case IF_VOTED:
      printf("\n\n3.  Your message should say:");
      printf("\n\n\teVote help who\n\nto learn details about the WHO command.\n");
      printf("\nIf your message says:");
      printf("\n\n\teVote how Charlie@somewhere.com\n\nyou will receive a message reporting ");
      if (local_ptype == PUBLIC)
	printf("Charlie's vote.\n");
      else
	printf("whether or not Charlie has voted.\n\n");
      break;
    }
  printf("To see your own vote");
  if (p_item->eVote.vstatus != UNSEEN)
    {
      printf(", the %s vote tally,",
	     (p_item->eVote.vstatus == CLOSED ? "final" : "current"));
    }
  printf(" and this%sinformation again, send the command:\n\n",
	 (p_item->eVote.vstatus != UNSEEN ?	"\n" : " "));
  printf("\teVote info");
  printf("\n\nFor a general explanation of eVote/Majordomo, use any "
	 "\nsubject line, and send the message:\n\n");
  printf("\teVote help\n\n");
  if (isauthor == YES && new_poll == NO && p_item->eVote.vstatus != CLOSED)
    {
      address("TO", from);
      printf("\n\nBecause you are the author of this poll, you, and only you, \ncan send the message:");
      printf("\n\n\teVote close");
      printf("\n\nSending this message will close the poll to voting");
      if (p_item->eVote.vstatus == UNSEEN )
	printf(" and the \nresults will be posted to the %s list.\n",
	       list);
      else
	printf(".\n");
    }
}
/************************************************************
 * Displays the stats on a group where all the min's == -max's
 *************************************************************/
void
display_mirror_stats(ITEM_INFO * p_item, YESorNO new_poll)
{
  int i;
  char result[10] = "-";
  char vote_str[10] = "-";
  unsigned long pos = 0;
  unsigned long neg = 0;
  unsigned long voters = 0L, readers = 0L;
  float ratio;
  char * ratio_string(float ratio);
  voters = (new_poll? 0 : get_mail_voters((p_item+1)->dropping_id));
  printf("   Pos    Neg    Zero  Pos/  Your   On      \n"
	 "   Votes  Votes  Votes  Neg  Vote   Choice  \n");
  for (i = 1; i <= (p_item+1)->eVote.no_in_group; i++)
    {
      if (new_poll == NO)
	{
	  get_mail_stats((p_item+i)->dropping_id,
			 &readers, vote_str, result);
	  ratio = get_vratio((p_item+i)->dropping_id, &pos, &neg);
	}
      printf("\n%6ld %6ld %6ld   %4s%5s  %2d. %s ", 
	     pos, neg, voters - pos - neg, 
	     (neg ? ratio_string(ratio) : "  - "), vote_str,
	     i, (p_item+i)->eVote.title);
    }
  printf("\n");
  return;
}
/**********************************************************
 *  displays the statistics for the item or group.
 **********************************************************/
void
display_stats(ITEM_INFO *p_item, YESorNO new_poll, YESorNO closing)
{
  unsigned long readers = 0L, voters = 0L, yeses = 0L;
  char result[10] = "-";
  int i;
  YESorNO negs = NO;
  char participants[30] = "Participants";
  char vote_str[10] = "-";
  VSTATUS vstat;
  YESorNO mirror;
  int lim;
  if (p_item == NULL)
    p_item = p_item_copy;
  vstat = p_item->eVote.vstatus;
  if (new_poll == YES)
    sprintf(participants,"%lu participants", no_of_participants);
  if (p_item->eVote.vstatus == CLOSED)
    {
      printf("\n");
      highlight("RESULTS");
    }
  else if (new_poll == YES)
    {
      printf("\n");
      highlight("POLL INSTRUCTIONS");
    }
  else
    {
      printf("\n");
      highlight("RESULTS SO FAR");
    }
  switch (p_item->eVote.type)
    {
    case PLAIN:  /* really GROUPED */
      vstat = (p_item+1)->eVote.vstatus;
      if (new_poll == NO)
	voters = get_mail_voters((p_item+1)->dropping_id);
      if (no_pet_votes)  /* accompanies a petition */
	{
	  /*					printf("\n%s%lu %s voted on %s.\n",
						(vstat == CLOSED ? "" : "So far, "),
						voters, (voters == 1 ? "person has" : "people have"),
						p_item->eVote.title); */
	}
      else
	if (vstat == CLOSED)
	  {
	    printf("\nOf the %lu people subscribed to the %s list \nwhen this subject was closed, ", 
		   (p_item+1)->eVote.participants_when_closed, list);
	    if (voters > 1L)
	      printf("%lu of them voted.\n", voters);
	    else if (voters == 1L)
	      printf("one of them voted.\n");
	    else /* voters == 0L */
	      printf("none of them voted.\n");
	  }
	else if (new_poll == NO)
	  {
	    printf("\nOf the %lu people currently subscribed to the %s \nlist, ",
		   no_of_participants, list);
	    if (voters > 1L)
	      printf("%lu have voted so far.\n", voters);
	    else if (voters == 1L)
	      printf("one has voted so far.\n");
	    else  /* voters == 0L */
	      printf("none have voted on this poll yet.\n");
	  }
      switch ((p_item+1)->eVote.type)				
	{
	case GROUPEDY:
	  if ((p_item+1)->eVote.sum_limit != NO_LIMIT)
	    printf("\n%s %s asked to vote YES on %d of the following choices:\n\n",
		   participants,
		   (vstat == CLOSED ? "were" : "are"),
		   (p_item+1)->eVote.sum_limit);
	  else
	    printf("\n%s %s asked to vote YES or NO on the following items:\n\n",
		   participants,
		   (vstat == CLOSED ? "were" : "are"));
	  if (closing == YES)
	    {
	      display_closing_stats();
	      break;
	    }
	  if (vstat == CLOSED || vstat == OPEN)
	    {
	      printf("  Your  Yes    No     On      \n");
	      printf("  Vote  Votes  Votes  Choice  \n");
	      for (i = 1; i <= (p_item+1)->eVote.no_in_group; i++)
		{
		  if (new_poll == NO)
		    {
		      get_mail_stats((p_item+i)->dropping_id,
				     &readers, vote_str, result);
		    }
		  yeses = atoul(result);
		  printf("\n %4s %4lu   %4lu    %2d. %s ", vote_str, yeses, 
			 voters-yeses, i, (p_item+i)->eVote.title);
		}
	    }
	  else if (vstat == UNSEEN)
	    {
	      printf("  Your  On      \n");
	      printf("  Vote  Choice  \n");
	      for (i = 1; i <= (p_item+1)->eVote.no_in_group; i++)
		{
		  if (new_poll == NO)
		    {
		      get_mail_stats((p_item+i)->dropping_id,
				     &readers, vote_str, result);
		    }
		  printf("\n   %3s %2d. %s ", vote_str, i,
			 (p_item+i)->eVote.title);
		}
	    }
	  if ((p_item+1)->eVote.sum_limit != NO_LIMIT)
	    printf("\n\nYou have used %d of your %d YES votes.\n",
		   (new_poll? 0 : get_my_sum((p_item+1)->dropping_id)),
		   (p_item+1)->eVote.sum_limit);
	  else
	    printf("\n");
	  break;
	case GROUPEDN:
	case GROUPEDn:
	  mirror = YES;
	  lim = (p_item+1)->eVote.max;
	  if (lim != -(p_item+1)->eVote.min)
	    mirror = NO;
	  for (i = 1; i <= (p_item+1)->eVote.no_in_group; i++)
	    {
	      if (new_poll == NO)
		{
		  get_mail_stats((p_item+i)->dropping_id,
				 &readers, vote_str, result);
		}
	      if ((p_item+i)->eVote.max != lim
		  || ((p_item+i)->eVote.min != -lim))
		{
		  mirror = NO;
		  if (negs == YES)
		    break;
		}
	      if ((p_item+i)->eVote.min < 0)
		{
		  negs = YES;
		  if (mirror == NO)
		    break;
		}
	    }
	  if (mirror && ((p_item+1)->eVote.sum_limit == NO_LIMIT
			 || ((p_item+1)->eVote.sum_limit >=
			     (p_item+1)->eVote.no_in_group * (p_item+1)->eVote.max))
	      && (vstat == OPEN || vstat == CLOSED))
	    {
	      voters = (new_poll ? 0 : 
			get_mail_voters((p_item+1)->dropping_id));
	      printf("\n%s%lu have voted from %d to %d on the following items:\n\n",
		     (vstat == CLOSED ? "" : "So far, "), voters,
		     (p_item+1)->eVote.min, (p_item+1)->eVote.max);
	      display_mirror_stats(p_item, new_poll);
	      break;
	    }
	  if ((p_item+1)->eVote.sum_limit != NO_LIMIT)
	    printf("\n%s %s asked to distribute %d %s over \nthe following choices:\n\n",
		   participants, 
		   (vstat == CLOSED ? "were" : "are"),
		   (p_item+1)->eVote.sum_limit,
		   (negs ? "positive votes" : "votes"));
	  else
	    printf("\n%s %s asked to vote on the following items:\n\n",
		   participants,
		   (vstat == CLOSED ? "were" : "are"));
	  if (closing == YES)
	    {
	      display_closing_stats();
	      break;
	    }
	  if (vstat == CLOSED || vstat == OPEN)
	    {
	      printf("  Min   Max   Your  Average   On      \n"
		     "  Vote  Vote  Vote  Vote      Choice  \n");
	      for (i = 1; i <= (p_item+1)->eVote.no_in_group; i++)
		{
		  if (new_poll == NO)
		    {
		      get_mail_stats((p_item+i)->dropping_id,
				     &readers, vote_str, result);
		    }
		  printf("\n %4d   %3d  %4s  %5s     %2d. %s ", 
			 (p_item+i)->eVote.min,
			 (p_item+i)->eVote.max,
			 vote_str, result,
			 i, (p_item+i)->eVote.title);
		}
	      if ((p_item+1)->eVote.sum_limit == NO_LIMIT)
		printf("\n");
	    }
	  else if (vstat == UNSEEN)
	    {
	      printf("  Min   Max   Your    On      \n"
		     "  Vote  Vote  Vote    Choice  \n");
	      for (i = 1; i <= (p_item+1)->eVote.no_in_group; i++)
		{
		  if (new_poll == NO)
		    {
		      get_mail_stats((p_item+i)->dropping_id,
				     &readers, vote_str, result);
		    }
		  printf("\n %4d   %3d   %3s    %2d. %s ", 
			 (p_item+i)->eVote.min,
			 (p_item+i)->eVote.max,
			 vote_str, i,
			 (p_item+i)->eVote.title);
		}
	    }
	  i = 0;
	  if ((p_item+1)->eVote.sum_limit == NO_LIMIT)
	    break;
	  if (new_poll == NO && (i = get_my_sum((p_item+1)->dropping_id)) ==
	      (p_item+1)->eVote.sum_limit)
	    {
	      printf("\n\nYou %sused all of the %d %s you %s allotted \nfor these choices. \n ", 
		     (vstat == CLOSED ? "" : "have "),
		     (p_item+1)->eVote.sum_limit,
		     (negs ? "positive votes" : "votes"),
		     (vstat == CLOSED ? "were" : "are"));
	    }
	  else
	    printf("\n\nYou %sused %d of the %d %s you %s allotted \nfor these choices.\n", 
		   (vstat == CLOSED ? "" : "have "),
		   i, (p_item+1)->eVote.sum_limit,
		   (negs ? "positive votes" : "votes"),
		   (vstat == CLOSED ? "were" : "are"));
	  break;
	default:
	  /* impossible */
	  break;
	}
      break;
      if ((p_item+1)->eVote.vstatus == UNSEEN)
	{
	  printf("\nThe tally is not available until %s \ncloses the poll.\n",
		 author_name);
	}
      break;
    case TIMESTAMP:
      printf("\nProgramming error\n");
    case TALLIEDY:
    case TALLIEDN:
      if (vstat == CLOSED)
	{
	  printf("\nWhen this poll closed, there were %lu participants.",
		 p_item->eVote.participants_when_closed);
	  printf("\nThe participants of the %s list were asked\n",
		 list);
	}
      else
	{
	  printf("\nThe %lu participant%sof the %s list %s asked\n",
		 no_of_participants, (no_of_participants == 1L ? " " : "s "),
		 list, (no_of_participants == 1L ? "is" : "are"));
	}
      if (p_item->eVote.type == TALLIEDY)
	{
	  printf("to vote yes or no.\n");
	}
      else
	printf("to vote from %d to %d.\n",
	       p_item->eVote.min, p_item->eVote.max);
      if (new_poll == YES)
	break;
      get_mail_stats(p_item->dropping_id,
		     &readers, vote_str, result);
      voters= atoul(vote_str);
      printf("\n%lu subscriber%sto the %s list %s \n",
	     voters, 
	     (voters == 1L ? " " : "s "), list,
	     (vstat == CLOSED ?	"voted." : 
	      (voters == 1L ? "has voted so far." : "have voted so far.")));
      if (new_poll == NO && closing == NO)
	{
	  (void)how_voted(NULL, 0);
	}
      if (p_item->eVote.vstatus != UNSEEN)
	{
	  if (p_item->eVote.type == TALLIEDY)
	    {
	      unsigned long pos;
	      unsigned long neg;
	      float ratio;
	      char * ratio_string(float ratio);
	      ratio = get_vratio(dropping_id, &pos, &neg);
	      printf("\n%10lu --> = YES Votes\n", pos);
	      printf("\n%10lu --> = NO Votes\n\n", neg);
	      printf("\n%10s --> = Ratio of YES/NO Votes\n", 
		     (neg ?  ratio_string(ratio): " - "));
	    }
	  else
	    {
	      if (voters == 0L)
		strcpy(result, " - ");
	      if (p_item->eVote.max == -p_item->eVote.min)
		{
		  printf("\n%10s --> Average Vote\n", result);
		  report_ratios(p_item->dropping_id);
		}
	      else
		printf("\nAverage vote = %s.\n", result);
	    }
	}
      else   /* UNSEEN */
	{
	  printf("\nThe tally is not available until %s \ncloses the poll.\n",
		 author_name);
	}
      break;
    default:
      /* impossible */
      break;
    }
}		
/************************************************************/
void
do_copy(int i)
{
  int j;
  strcpy(author_name, who_is(item_info[i].eVote.author));
  dropping_id = i;
  if (item_info[i].eVote.type == PLAIN)
    {
      if ((p_item_copy = malloc(sizeof(ITEM_INFO) *
					    (item_info[i+1].eVote.no_in_group
					     + 1))) == NULL)
	{
	  sprintf(error_msg,"\n%s is out of resources.  Please try your eVote command later.\n", whereami);
	  bounce_error(SENDER | OWNER | ADMIN);
	}
      for (j = 0; j < item_info[i+1].eVote.no_in_group + 1; j++)
	{
	  memcpy(p_item_copy + j, &(item_info[i+j]), sizeof(ITEM_INFO));
	}
      ptype = (p_item_copy+1)->eVote.priv_type;
    }
  else
    {
      if ((p_item_copy = malloc(sizeof(ITEM_INFO)))
	  == NULL)
	{
	  sprintf(error_msg,"\n%s is out of resources.  Please try your eVote command later.\n", whereami);
	  bounce_error(SENDER | OWNER | ADMIN);
	}
      memcpy(p_item_copy, &(item_info[i]), sizeof(ITEM_INFO));
      ptype = p_item_copy->eVote.priv_type;
    }
}
/**********************************************************
 *   returns stats for the subject.
 **********************************************************/
void
get_mail_stats(short dropping_id,
	       unsigned long* readers, char *str1, char *str2)
{
  char mail_stats[STXTLEN + 1];
  int i;
  strcpy(mail_stats, get_stats(dropping_id));
  for (i = 0; mail_stats[i] != '\0'; i++)
    {
      switch (mail_stats[i])
	{
	case '*':
	  if (mail_stats[i+1] == '?' || mail_stats[i-1] == '?')
	    break;
	case 'I':
	case 'P':
	case 'p':
	  mail_stats[i] = ' ';
	  break;
	}
    }
  sscanf(mail_stats, "%lu %s %s", readers, str1, str2);
  if (strcmp(str1,"ACC") == 0)
    strcpy(str1,"-");
}
/**********************************************************
 *  get_voters(short dropping_id) only provides
 *   the number of voters for grouped polls.  
 *   This provides that information for all polls.
 **********************************************************/
unsigned long
get_mail_voters(short dropping_id)
{
  unsigned long readers;
  char vote_str[10];
  char result[10];
  if (item_info[dropping_id].eVote.type == PLAIN)
    return get_voters(dropping_id + 1);
  if (item_info[dropping_id].eVote.type >= GROUPED 
      && item_info[dropping_id].eVote.type <= GROUPEDN)
    return get_voters(dropping_id);
  get_mail_stats(dropping_id,
		 &readers, vote_str, result);
  return (atoul(vote_str));
}
/**********************************************************
 *     Sends a message displaying the voting instructions
 *     for the poll.
 **********************************************************/
void
instruct(int whom, ITEM_INFO* p_item, YESorNO new_poll)
{
  if (p_item == NULL)
    p_item = p_item_copy;
  gen_header(whom, (new_poll ? "Poll:" : "Re:"), YES);
  if (just_checking == YES)
    check_message();
  display_info(p_item, new_poll);
  if (new_poll)
    finish(0);
  big_finish(0);
}
/************************************************************
 *     Returns a static string containing the polls directory.
 *************************************************************/
char *
pollsdir(void)
{
  static char fname[FNAME + 1];
  int i;
  if (fname[0] != '\0')
    return fname;
  strcpy(fname, listdir);
  i = strlen(fname) + 1;
  do
    {
      while (fname[--i] != '/')
	;
    }
  while (strncmp(&fname[i], "/lists", 6) != 0);
  strcpy(&fname[i],"/polls");
  return fname;
}
/**********************************************************
 *   Prints the poll title and a result of some sort.
 **********************************************************/
void
print_list_line(int i)
{
  int len;
  unsigned long readers;
  char vote_str[10];
  char result[10];
  char tmp[12];
  unsigned long voters;
  int j = i;
  len = 38 - strlen(item_info[i].eVote.title);
  printf("\n%4lu ", voters = get_mail_voters(i));
  while (len-- > 0)
    printf(" ");
  printf("%s  ", item_info[i].eVote.title);
  if (item_info[i].eVote.type == PLAIN)
    j = i+1;
  switch (item_info[j].eVote.priv_type)
    {
    case PUBLIC:
      printf("Public   ");
      break;
    case PRIVATE:
      printf("Private  ");
      break;
    case IF_VOTED:
      if (petition == MAYBE 
	  && (item_info[j].eVote.type == TALLIEDY
	      || item_info[j].eVote.type == TIMESTAMP))
	printf("Private  ");
      else
	printf("If-Voted ");
      break;
    }
  switch (item_info[i].eVote.type)
    {
    case PLAIN:  /* some kind of group */
      switch (item_info[j].eVote.type)
	{
	case GROUPEDN:
	case GROUPEDn:
	  printf("[#-#] %3d choices ", item_info[j].eVote.no_in_group);
	  break;
	case GROUPEDY:
	  printf("[Y/N] %3d choices ", item_info[j].eVote.no_in_group);
	  break;
	default:
	  /* impossible */
	  break;
	}
      switch (item_info[j].eVote.vstatus)
	{
	case OPEN:
	  printf("Visible");
	  break;
	case UNSEEN:
	  printf(" Hidden");
	  break;
	default:  /* closed */
	  printf(" Closed");
	  break;
	}
      break;
    case TIMESTAMP:
      printf("   Petition   %3lu signers", voters );
      break;
    case TALLIEDY:
      if (item_info[i].eVote.vstatus == UNSEEN)
	{
	  printf("[Y/N]              Hidden");
	}
      else /* CLOSED or OPEN */
	{
	  float ratio;
	  unsigned long yeses;
	  get_mail_stats(i, &readers, vote_str, result);
	  yeses = atoul(result);
	  if (voters - yeses > 0)  /* NO's */
	    {
	      ratio = yeses/(float)(voters - yeses);
	    }
	  len = 13 - sprintf(tmp, "[%lu/%lu]", yeses, voters - yeses);
	  printf("[Y/N]");
	  for (i = 0; i < len; i++)
	    printf(" ");
	  printf("%s = %s", tmp, (voters - yeses ? ratio_string(ratio)
				  : "  - ") );
	}
      break;
    case TALLIEDN:
      len = printf("[%d,%d]", item_info[i].eVote.min, 
		   item_info[i].eVote.max);
      if (item_info[i].eVote.vstatus == UNSEEN)
	{
	  for (i = len; i < 19; i++)   /* was 20 */
	    printf(" ");
	  printf("Hidden");
	}
      else /* CLOSED or OPEN */
	{
	  if (item_info[i].eVote.max != -item_info[i].eVote.min)
	    {
	      get_mail_stats(i, &readers, vote_str, result);
	      if (strlen(result) > 5)
		len += strlen(result);
	      else
		len += 5;
	      for (i = len; i < 19; i++)
		printf(" ");
	      printf(" AVE =%5s", result);
	    }
	  else
	    {
	      float ratio;
	      float pos;
	      float neg;
	      float vratio;
	      unsigned long vpos;
	      unsigned long vneg;
	      vratio = get_vratio(i, &vpos, &vneg);
	      sprintf(tmp,"[%lu/%lu]", vpos, vneg);
	      ratio = get_ratio(i, &pos, &neg);
	      len += strlen(tmp) + 2;
	      for (i = len; i < 19; i++)
		printf(" ");
	      if (neg == 0)
		printf(" %s =   -", tmp);
	      else
		printf(" %s %s:1", tmp, ratio_string(ratio));
	    }
	}
      break;
    default:
      /* impossible */
      break;
    }
}
/**********************************************************
 *        processes a request to close a poll
 **********************************************************/
void
process_close(YESorNO force)
{
  RTYPE cc;
  int i;
  char *p_word = "poll";
  if (petition == YES)
    p_word = "petition";
  if (p_item_copy->eVote.vstatus == CLOSED)
    {
      sprintf(error_msg, "\nThe %s attached to \"%s\" \nis already closed.  \n\nSend the command \"eVote stats\" to see the results.\n",
	      p_word, subject);
      bounce_error(SENDER);
    }
  if (mail_voter != p_item_copy->eVote.author && force == NO)
    {
      strcpy(author_name, who_is(p_item_copy->eVote.author));
      sprintf(error_msg, "\nOnly %s, the initiator of the %s attached to \n\"%s\" can close it.\n", 
	      author_name, p_word, subject);
      bounce_error(SENDER);
    }
  if (confirm_this.status != VERIFIED)
    {
      send_confirm(FOR_CLOSE);
    }
  get_stats(p_item_copy->dropping_id);  /* send_vstatus expects stats
					   to be in memory */
  i = -1;
  do
    {
      switch (cc = send_vstatus(p_item_copy+(++i), CLOSED))
	{
	case FAILURE:
	  sprintf(error_msg, "\nUnable to send or receive message.");
	  bounce_error(SENDER);
	  break;
	case NO_CHANGE:
	  sprintf(error_msg,
		  "\nprocess_close: Illegal change requested.");
	  bounce_error(SENDER | ADMIN | OWNER);
	  break;
	case NO_ITEM:
	  sprintf(error_msg,
		  "\nprocess_close: Subject \"%s\": not found.",
		  subject);
	  bounce_error(SENDER | ADMIN | OWNER);
	  break;
	case REDUNDANT:
	  sprintf(error_msg, "\nVoting on \"%s\" is already closed.  Programmer error?", subject);
	  bounce_error(SENDER | ADMIN | OWNER);
	  break;
	case DONE:
	  break;
	default:
	  sprintf(error_msg,"\nUnexpected return:%d in process_close()\n", cc);
	  bounce_error(SENDER | ADMIN | OWNER);
	  break;
	}
    }
  while ((p_item_copy+i)->eVote.type == PLAIN);
  gen_header(LIST, "Closed:", YES);
  if (force == YES)
    {
      printf("\n%s has forced the closing of the %s on",
	     from, p_word);
      printf("\n\n\t%s\n", subject);
      printf("\nThis %s was initiated by %s \non %s", p_word,
	     who_is(p_item_copy->eVote.author),
	     date_str((time_t)p_item_copy->eVote.open));
    }
  else
    {
      printf("\n%s has closed the %s on",
	     from, p_word);
      printf(" \n\n\t%s.\n", subject);
      printf("\nThis %s was initiated on %s", p_word,
	     date_str((time_t)p_item_copy->eVote.open));
    }
  if (petition == YES)
    {
      translate(DEFAULT_LANGUAGE, DISPLAY_PETITION_TEXT, p_item_copy,
		NO, NO, NO, NULL, NULL, MAYBE);
      translate(default_language, DISPLAY_PETITION_RESULTS,
		&item_info[dropping_id], MAYBE, MAYBE, MAYBE, 
		NULL, NULL, MAYBE);
      if (item_info[dropping_id].eVote.priv_type != PRIVATE)
	{
	  printf("\n");
	  highlight("SIGNATURES");
	  printf("\n\nYou can retrieve the signatures for this petition by"
		 "\nsending email to %s@%s with the subject \n\"%s\".", list, 
		 whereami, subject);
	  printf("\n\nYour message should say:");
	  printf("\n\n     eVote names\n");
	}
    }
  else
    {
      display_poll_text(p_item_copy, NO);
      display_stats(&item_info[dropping_id], NO, YES);
      printf("\n\nYou can see your own vote, and this information again by "
	     "\nsending email to %s@%s with the subject \n\"%s\".", list, 
	     whereami, subject);
      printf("\n\nYour message should say:");
      printf("\n\n     eVote stats\n");
      if (p_item_copy->eVote.priv_type != PRIVATE)
	{
	  printf("\nYou can still use the \"eVote how\" and \"eVote who\" commands.");
	  printf("\nSend a message saying:"
		 "\n\n     eVote help who"
		 "\n\n          -or-  "
		 "\n\n     eVote help how"
		 "\n\nto learn how to use these commands.\n");
	  printf("\nThe \"eVote stats\", \"eVote info\", \"eVote who\", and"
		 "\n\"eVote how\" commands are available until this poll is"
		 "\ndropped from the database.");
	}
      else
	{
	  printf("\nThe \"eVote stats\" and \"eVote info\" commands are available until"
		 "\nthis poll is dropped from the database.");
	}
    }
  printf("\n");
  highlight("DELETING THE DATA");
  printf("\nThe originator of this %s, %s, can"
	 "\ndrop this %s from the database after it has been"
	 "\nclosed for %d days by sending the command:",
	 p_word, author_name, p_word, AUTHOR_DAYS);
  printf("\n\n     eVote drop\n\n");
  printf("Anyone can drop this %s after it has been closed for %d days.\n",
	 p_word, OTHERS_DAYS);
  big_finish(0);
}		
/**********************************************************
 *   processes a request to drop a poll
 *   Anyone can drop the item if there have been no votes.
 *   Using the approve feature and adding the word "quiet"
 *   drops the poll without sending a notice to the list.
 **********************************************************/
void
process_drop(YESorNO force, YESorNO silent)
{
  int items;
  char *p_word = "poll";
  if (petition == YES)
    p_word = "petition";
  if (get_mail_voters(dropping_id) == 0)
    {
      /* No one has voted so just drop it. */
      confirm_this.status = VERIFIED;	
    }
  else  /* Someone has voted.  Poll is on. */
    {
      if (p_item_copy->eVote.vstatus != CLOSED && force == NO)
	{
	  if (mail_voter == p_item_copy->eVote.author)
	    {
	      sprintf(error_msg, "\nYou must close this %s and then wait %d days before you can drop it.\n", p_word, AUTHOR_DAYS);
	    }
	  else
	    {
	      sprintf(error_msg,"\nThis %s must be closed by %s, its author, \nand then you must wait %d days before you can drop it.  \n%s can drop it %d days after it is closed.\n", 
		      p_word, author_name, OTHERS_DAYS, 
		      author_name, AUTHOR_DAYS);
	    }
	  bounce_error(SENDER);
	}
      if (mail_voter == p_item_copy->eVote.author && force == NO)
	{
	  float left;
	  if ((left = (AUTHOR_WAIT - (now - p_item_copy->eVote.close))/
	       (24. * 60. * 60.))
	      > 0)
	    {
	      sprintf(error_msg,"\nYou must wait %d days after you close a %s before you can drop it.  \nYou have %2.0f days left to wait on this %s, \n\"%s\".\n",
		      AUTHOR_DAYS, p_word, left, p_word, subject);
	      bounce_error(SENDER);
	    }
	}
      if (mail_voter != p_item_copy->eVote.author && force == NO)
	{
	  float left_me, left_author;
	  if ((left_me = (OTHERS_WAIT - (now - p_item_copy->eVote.close))/
	       (24*60*60)) > 0)
	    {
	      if ((left_author = (AUTHOR_WAIT 
				  - (now - p_item_copy->eVote.close))
		   /(24*60*60)) > 0)
		{
		  sprintf(error_msg,"\nThis %s: \n\n\t\"%s\" \n\n"
			  "can't be dropped until %d days have passed since"
			  "it was closed.  \nThere are %2.0f days left and"
			  "then only %s, \nthe %s's originator, can drop it."
			  "You can drop it after it \nhas been closed for %d"
			  "days.  There are %3.0f days left to wait \n"
			  "before you can drop it.",
			  p_word, subject, AUTHOR_DAYS, left_author, 
			  author_name, p_word, OTHERS_DAYS, left_me);
		}
	      else
		{
		  sprintf(error_msg,"\nAt this time the %s attached to \"%s\" \ncan only be dropped by %s, the %s's originator.  \nYou can drop it after it has been closed for %d days.  \nThere are %3.0f days left to wait before you can drop it.",
			  p_word, subject, author_name, p_word, 
			  OTHERS_DAYS, left_me);
		}
	      bounce_error(SENDER);
	    }
	}
    }
  if (confirm_this.status != VERIFIED)
    {
      send_confirm(FOR_DROP);
    }
  items = 1;
  if (p_item_copy->eVote.type == PLAIN)
    items = 1 + (p_item_copy+1)->eVote.no_in_group;
  if (drop_items(p_item_copy, items) != items)
    {
      sprintf(error_msg, "\nSome difficulty in dropping \"%s\".\n",
	      subject);
      bounce_error(SENDER | OWNER | ADMIN | DEVELOPER);
    }
  if (silent)
    gen_header(SENDER | OWNER, "Re:", YES);
  else
    gen_header(LIST, "Re:", YES);
  if (force == YES)
    {
      printf("\n%s has forced the deletion of the %s on",
	     from, p_word);
      printf("\n\n\t%s\n", subject);
      printf("\nThis %s was initiated by %s.\n", p_word, author_name);
    }
  else
    {
      printf("\n%s has dropped the %s on \n\n\t", from, (p_word));
      printf("%s\n", subject);
    }
  if (strncmp("petitionxyz", lclist, 8) == 0)
    {
      forget_petition(0, p_item_copy, YES);
      drop_confirm_dir();
      finish(0);
    }
  drop_poll_text(p_item_copy);
  drop_confirm_dir();
  big_finish(0);
}
/************************************************************/
void
report_ratios(short dropping_id)
{
  float pos;
  float neg;
  float ratio;
  unsigned long posv;
  unsigned long negv;
  ratio = get_vratio(dropping_id, &posv, &negv);
  printf("\n %9ld --> Number of Positive Voters\n", posv);
  printf("\n %9ld --> Number of Negative Voters\n", negv);
  printf("\n %9s --> Ratio of Positive to Negative Voters\n",
	 ratio_string(ratio));
  ratio = get_ratio(dropping_id, &pos, &neg);
  printf("\n %9ld --> Sum of Positive Votes\n", (long)pos);
  printf("\n %9ld --> Sum of Negative Votes\n", (long)neg);
  printf("\n %9s --> Ratio of Sums, Positive to Negative\n",
	 ratio_string(ratio));
}
/**********************************************************/
void
send_stats_only(void)
{
  gen_header(SENDER, "Re:", YES);
  start_displays(p_item_copy);
  display_stats(p_item_copy, NO, NO);
  big_finish(0);
}
/**********************************************************/
void
start_displays(ITEM_INFO * p_item)
{
  printf("\n\nOn %s%s attached a poll to this subject:",
	 date_str((time_t)p_item->eVote.open), author_name);
  printf("\n\n\t%s\n", subject);
  if (p_item->eVote.vstatus == CLOSED) 
    {
      printf("\nThis poll was "
	     "opened for voting on %s",
	     date_str((time_t)p_item->eVote.open));
      printf("but has been closed since %s",
	     date_str((time_t)p_item->eVote.close));
    }
  display_poll_text(p_item, just_checking);
}


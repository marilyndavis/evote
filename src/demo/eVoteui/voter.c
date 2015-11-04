/* $Id: voter.c,v 1.3 2003/01/15 18:54:08 marilyndavis Exp $ */ 
/**********************************************************
 *	../eVote/src/demo/eVoteui/voter.c
 *********************************************************
 *    Copyright (c) 1994...2015 Deliberate.com Patented.
 *    by Marilyn Davis
 **********************************************************/
#include "../eVote.h"
#include "../../Clerklib/Clerk.h"
/**********************************************************/
OKorNOT
drop_voter(char * conf_name, 
		   unsigned long uid_to_drop)
{
  RTYPE cc;
  switch (cc = send_drop_voter(conf_name, uid_to_drop, NO))
    {
    case GOOD:
      return OK;
    case DONE:
      fprintf(stderr,"\nCould not drop voter %s from %s.  \nBallot was locked, try again.\n", get_name(uid_to_drop), conf_name);
      return NOT_OK;
    case NO_CONF:
      fprintf(stderr,"\nCould not drop voter %s: No conf %s.\n",
	      get_name(uid_to_drop), conf_name);
      return NOT_OK;
    case NOT_GOOD:
      printf("\nCould not drop voter %s from %s: Participation has not expired.\n",
	     get_name(uid_to_drop), conf_name);
      return NOT_OK;
    case NO_VOTER:
      printf("\nCould not drop voter %s from %s: nonexistant voter.\n",
	     get_name(uid_to_drop), conf_name);
      return NOT_OK;
    case ON_LINE:
      printf("\nCould not drop voter %s: voter online.\n",
	     get_name(uid_to_drop));
      return NOT_OK;
    case FAILURE:
      return NOT_OK;
    default:
      fprintf(stderr, "\nUnexpected return in drop_voter: %d\n", cc);
      return NOT_OK;
    }
  return UNDECIDED;
}
/***********************************************************
 *      YESorNO testing should always be NO when real
 *      people are using the program.  This function protects
 *      their privacy.
 ************************************************************/
OKorNOT
i_am_entering(char * conf_name, YESorNO testing)
{
  short ret_drop_days;
  char *time_str;
  ACTION action_dummy;  /* vacation feature for email version */
  if ((time_str = send_entering(conf_name, &ret_drop_days, 0, 
				&action_dummy)) 
      != NULL)
    {
      if (strncmp(time_str,"\nSorry", 5) == 0  /* root tried to enter*/
	  || strncmp(time_str, "\nYou are", 8) 
	  == 0) /* no_voter tried to enter*/
	{
	  printf("%s", time_str);
	  return NOT_OK;
	}
      if (strcmp(time_str, "On twice") == 0)
	{
	  printf("\nYou still have an active eVote session in progress.  Please end the \nexisting session before you try to enter eVote again.\n");
	  return NOT_OK;
	}			
      printf("\n%s: Last participation on %s",
	     current_conf, time_str);
      return OK;
    }
  /* NULL is returned if the voter has never entered this conf 
     before.  NULL can also be returned if the conf doesn't
     exist. In that case (programmer error), ret_drop_days == -1 */
  if (ret_drop_days == -1)
    {
      printf("\nProgrammer error.  No %s conf.\n", conf_name);
      return NOT_OK;
    }
  printf("\nWarning: The %s conference is eVoted.  This means that there will",
	 conf_name);
  printf("\nbe a public record of your participation in this conference.");
  switch (ret_drop_days)
    {
    case 0:
      printf("\n\nHowever, you can remove the record of your participation at");
      printf("\nany time by entering: ");
      printf("\n\n   r %s", conf_name);
      printf("\n\n to (R)emove Me from the %s conference.", conf_name);
      break;
    case 9999:
      printf("\n\nThe public record of your participation will exist as long");
      printf("\nas the %s conference exists.", conf_name);
      break;
    default:
      printf("\n\nHowever, if you do not participate in this conference for");
      if (ret_drop_days == 1)
	printf("\n1 day, you can remove the record of your");
      else
	printf("\n%d consecutive days, you can remove the record of your", ret_drop_days);
      printf("\nparticipation by entering: ");
      printf("\n\n   r %s", conf_name);
      printf("\n\n to (R)emove Me from the %s conference.", conf_name);
      break;
    }
  printf("\n\nDo you really want to join the %s conference? (yes/no) ", 
	 conf_name);
  if (testing == NO)
    {
      if (eVote_askyn(JOIN) != 'y')
	{
	  return NOT_OK;
	}
    }
  /* YES in send_joining marks the ballot as a user
     with a login, not email */
  if ((time_str = send_joining(TRY, YES)) != NULL)
    {
      if (strcmp(time_str, "On twice") == 0)
	{
	  printf("\nYou still have an active eVote session in progress.  Please end the \nexisting session before you try to enter eVote again.\n");
	  return NOT_OK;
	}			
      printf("\n%s: First time participating.\n", current_conf);
      return OK;
    }
  return NOT_OK;
}
/**********************************************************
 *    p_item is a copy of the item_info.  input is the rest
 *    of the user's input string.
 ***********************************************************/
OKorNOT
process_vote(ITEM_INFO *p_item, char *input)
{
  short old_vote;
  short mistakes = 0;
  short vote;
  RTYPE rtype;
  char vote_pt[6];
  YESorNO yesno = NO;
  if (p_item->eVote.type == PLAIN)
    {
      printf("\nItem #%d is not available for voting.", 
	     p_item->dropping_id);
      return OK;
    }
  if (p_item->eVote.vstatus == CLOSED)
    {
      printf("\nVoting is closed on item #%d.", 
	     p_item->dropping_id);
      return OK;
    }
  /* if 'y' or 'n' was in input, it was changed to '1' or '0' */
  /* in process_eVote() */
  if ((vote = check_atoi(input)) == EQUIT)
    return OK;
  if (p_item->eVote.type == TALLIEDY 
      || p_item->eVote.type == GROUPEDY)
    yesno = YES;
  while ((vote < p_item->eVote.min 
	  || vote > p_item->eVote.max) && vote != READ)
    {
      if (yesno == YES)
	printf("\nPlease enter 'y' for YES or 'n' for NO. \n");
      else
	printf("\nPlease enter a number from %d to %d. \n",
	       p_item->eVote.min, p_item->eVote.max);
      switch (vote = eVote_asknum(VOTE, 
				 p_item->eVote.min, 
				 p_item->eVote.max, yesno))
	{
	case EQUIT:
	  return OK;
	case ENOREPLY:
	  return UNDECIDED;
	case EQUESTION:
	  if (++mistakes > MISTAKES)
	    return UNDECIDED;
	  break;
	}
      return OK;
    }
  if (yesno == YES)
    {
      if (vote == 1)
	strcpy(vote_pt, "YES");
      else
	strcpy(vote_pt, "NO");
    }
  else
    sprintf(vote_pt,"%d", vote);
  switch (rtype = send_vote(p_item, vote, &old_vote))
    {
    case FAILURE:
      printf("\nSystem troubles: Your vote was not recorded.\n");
      return NOT_OK;
      break;
    case NO_MODS:
      printf("\nVoting is closed on item #%d:%c%s%c.\n",
	     p_item->dropping_id, 34, print_title(p_item->dropping_id), 34);
      return OK;
      break;
    case NO_CHANGE:
      printf("\nThank you.  You already voted %s on item #%d:%c%s%c.\n",
	     vote_pt, p_item->dropping_id, 34,
	     print_title(p_item->dropping_id), 34);
      break;
    case NOT_GOOD:
      printf("\nYour vote of %s on item #%d:%c%s%c \nis outside the vote limits.\n", 
	     vote_pt, p_item->dropping_id, 34,
	     print_title(p_item->dropping_id), 34);
      break;
    case VIOLATES_SUM:
      printf("\nYour vote of %s on item #%d:%c%s%c \nviolates the sum-limit.  It is not accepted.\n", 
	     vote_pt, p_item->dropping_id, 34,
	     print_title(p_item->dropping_id), 34);
      return OK;
    case GOOD:
      if (vote == READ)  /* here the voter erased old vote */
	{
	  if (yesno == YES)
	    {
	      if (old_vote == 1)
		strcpy(vote_pt, "YES");
	      else
		strcpy(vote_pt, "NO");
	    }
	  else
	    sprintf(vote_pt,"%d", old_vote);
	  printf("\nYour old vote of %s has been removed on item #%d.\n",
		 vote_pt, p_item->dropping_id);
	}
      else
	{
	  if (old_vote == READ 
	     || old_vote == NOT_READ)
	    {
	      if (yesno == YES)
		{
		  printf("\nThank you for your '%c' on item #%d.\n",
			 (vote?'Y':'N'), p_item->dropping_id);
		}
	      else
		printf("\nThank you for your %d on item #%d.\n",
		       vote, p_item->dropping_id);
	    }
	  else
	    {
	      if (yesno == YES)
		{
		  printf("\nThank you.  Your vote has changed from '%c' to '%c' on item #%d.\n",
			 (old_vote?'Y':'N'), (vote?'Y':'N'), 
			 p_item->dropping_id);
		}
	      else
		{
		  printf("\nThank you.  Your vote has changed from %d to %d on item #%d.\n",  
			 old_vote, vote, p_item->dropping_id);
		}
	    }
	}
    default:
      /* impossible */
      break;
    }
  if (p_item->eVote.type >= GROUPED)
    printf("\nThe sum of your votes for this group is %d.\n",
	   get_my_sum(p_item->dropping_id));
  return OK;
}

/* $Id: queries.c,v 1.3 2003/01/15 18:54:08 marilyndavis Exp $ */ 
/**********************************************************
 *	../eVote/src/demo/eVoteui/queries.c  -
 *      Menu and processing for how-voted and who-voted
 *      commands.
 *********************************************************
 *    Copyright (c) 1994...2015 Deliberate.com Patented.
 *    by Marilyn Davis
 **********************************************************/
#include <pwd.h>
#include <stdio.h>
#include "eVoteui.h"
#include "../../Clerklib/Clerk.h"
extern struct questions_def *ask;
/*  ask is defined in input.c */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *   These next two declarations are listed here rather than in
 *   in the Clerk.h file because they are only used by this
 *   file.
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
char *message(void); 
char *uid_report(unsigned long* p_uid);
/*************  void how-menu(...)   **********************
 *		void how_menu(ITEM_INFO *p_item)
 *			Context sensitive menu for how-voted questions.
 **********************************************************/
void
how_menu(ITEM_INFO *p_item)
{
  if (p_item->eVote.type == PLAIN)
    {
      printf("\nEnter a user's login name to see if that user ");
      printf("\nhas accessed item #%d.", p_item->dropping_id);
      return;
    }
  switch (p_item->eVote.priv_type)
    {
    case PUBLIC:
      printf("\nEnter a user's login name to see how that user voted");
      printf("\non item #%d: ", p_item->dropping_id);
      break;
    case PRIVATE:
      printf("\nHow-voted statistics are not available for item #%d.",
	     p_item->dropping_id);
      printf("\nIt is a PRIVATE item.");
      break;
    case IF_VOTED:
      printf("\nEnter a user's login name to see if that user has");
      printf("\nvoted on item #%d. ", p_item->dropping_id);
      break;
    }
}				
/*****************************************************************
 *  short how_voted  - returns the vote for the given pwname.  If
 *  pwname == NULL, the caller's vote is returned.  If not, the 
 *  vote is returned only if the p_item->eVote.priv_type allows it.
 *****************************************************************/	
short
how_voted(ITEM_INFO* p_item, char *pwname, unsigned long how_uid)
{
  short the_vote;
  char starter[35];
  short vote_sum;
  char vote_str[10];
  if (pwname == NULL)
    {
      how_uid = who_am_i();
      strcpy(starter,"You have");
      pwname = "You";
    }
  else
    {
      sprintf(starter,"%s has", pwname);
    }
  switch (send_how(p_item, how_uid, &the_vote, &vote_sum) )
    {
    case NO_VOTER:
      printf("\n%s not participated in this conference.\n", starter);
    case FAILURE:  /* message written to stderr */
      return (short)(-1);
      break;
    case GOOD:
      break;
    case STRING_OUT:
      printf("\n%s\n", message());
      return (short)(-1);
      break;
    default:
      /* impossible */
      break;
    }
  switch (the_vote)
    {
    case NOT_READ:
      printf("\n%s not accessed item #%d.\n", starter, p_item->dropping_id);
      break;
    case READ:
      printf("\n%s accessed item #%d.\n", starter, p_item->dropping_id);
      break;
    case LATE:
      printf("\n%s joined too late to vote on item #%d.\n",
	     pwname, p_item->dropping_id);
      break;
    case NOT_VALID:
      printf("\n%s left before the poll on item #%d closed",
	     pwname, p_item->dropping_id );
      if (strcmp(pwname, "You") == 0)
	printf("\nor you are a non-voting member.\n");
      else
	printf("\nor %s is a non-voting member.\n", pwname);
      break;
    default:
      if (p_item->eVote.priv_type == IF_VOTED && how_uid != who_am_i())
	{
	  printf("\n%s voted on item #%d.\n", starter, p_item->dropping_id);
	  break;
	}
      if (p_item->eVote.type == TALLIEDY || p_item->eVote.type == GROUPEDY)
	{
	  if (the_vote == 0)
	    strcpy(vote_str, "NO");
	  else
	    strcpy(vote_str, "YES");
	}
      else /*  numeric vote */
	{
	  sprintf(vote_str, "%d", the_vote);
	}
      printf("\n%s voted %s on item #%d.\n", starter, vote_str,
	     p_item->dropping_id);
    }
  if (p_item->eVote.type >= GROUPED
      && p_item->eVote.priv_type == PUBLIC)
    {
      if (p_item->eVote.type == GROUPEDY)
	{
	  printf("\n%s used %d of the %d YESes possible.\n", starter,
		 vote_sum, p_item->eVote.sum_limit);
	}
      else  /* GROUPEDN  or GROUPEDn */
	{
	  strcpy(starter, "your");
	  if (pwname != NULL)
	    sprintf(starter, "%s's", pwname);
	  printf("\nThe sum of %s votes on this group is %d.  The sum-limit is %d.\n",
		 starter,	vote_sum, p_item->eVote.sum_limit);
	}
    }
  return the_vote;
}
/*************************************************
 *  Called from process_eVote when 'h' is entered for a how-voted 
 *  question.
 *  input points to the unprocessed user input.
 *  An UNDECIDED return triggers the eVote menu.
 *  A NOT_OK return quits eVote.
 *  OK stays in eVote but only gives the eVote prompt.
 ***************************************************************/
OKorNOT
process_how(ITEM_INFO* p_item, char *input)
{
  short confusion = 0;
  unsigned long how_uid;
  short i;
  char *msg = ask[HOW_WHO].qprompt;
  char *pwname;
  struct passwd *pwstruct;
  YESorNO redo = NO;
  if (p_item->eVote.priv_type == PRIVATE)
    {
      printf("\nHow-voted statistics are not available for item #%d.",
	     p_item->dropping_id);
      printf("\nIt is a PRIVATE item.\n");
      return UNDECIDED;
    }
  do
    {
      redo = NO;
      if (*input == '\0')
	input = GetArg(msg);
      while (*input == '\0'|| *input == '?')
	{
	  if (++confusion > 2)
	    {
	      if (confusion > 4)
		return UNDECIDED;
	      how_menu(p_item);
	      continue;
	    }
	  input = GetArg(msg);
	}
      confusion = 0;
      while (*input == ' ')
	input++;
      pwname = input;
      for (i = 0; pwname[i] != '\0'; i++)
	{
	  switch (pwname[i])
	    {
	    case 'q':
	    case 'Q':
	      if ((i == 0 && pwname[i+1] == '\0')
		  || strcmp(pwname,"quit") == 0)
		return NOT_OK;
	      break;
	    case '?':
	      redo = YES;
	      break;
	    case ' ':
	      pwname[i]='\0';
	      break;
	    default:
	      break;
	    }
	}
      if (redo == YES)
	{
	  how_menu(p_item);
	  *input = '\0';
	}
    }
  while (redo == YES);
  if ((pwstruct = getpwnam(pwname)) == NULL)
    {
      printf("\n%s is not a user of this conferencing system.", pwname);
      return OK;
    }
  how_uid = pwstruct->pw_uid;
  (void) how_voted(p_item, pwname, how_uid);
  return OK;
}
/*****************************************************************
 *   OKorNOT process_who(
 *   Called from eVote_menu() to processes a who-voted request.
 *   Returns OK if everything made sense and we want to return to 
 *   the eVote prompt.
 *   Returns NOT_OK if we want to quit eVote.
 *   Returns UNDECIDED if we want to see the eVote menu again.
 ***********************************************************/
OKorNOT
process_who(ITEM_INFO *p_item, char *input)
{
  short confusion = -1;
  short i;
  char *question;
  YESorNO redo = NO;
  if (p_item->eVote.priv_type == PRIVATE)
    {
      printf("\nWho-voted statistics are not available for item #%d.",
	     p_item->dropping_id);
      printf("\nIt is a PRIVATE item.\n");
      return UNDECIDED;
    }
  do
    {
      redo = NO;
      while (*input == '\0')
	{
	  if (++confusion > 2)
	    {
	      if (confusion > 4)
		return UNDECIDED;
	      who_menu(p_item);
	      continue;
	    }
	  input = GetArg(ask[WHO_WHAT].qprompt);
	}
      confusion = 0;
      question = input;					
      for (i = 0; question[i] != '\0'; i++)
	{
	  switch (question[i])
	    {
	    case 'n':
	    case 'N':
	      if (question[i+1] == '\0' 
		  && (p_item->eVote.type == TALLIEDY 
		      || p_item->eVote.type == GROUPEDY))
		question[i] = '0';
	      break;
	    case 'y':
	    case 'Y':
	      if (question[i+1] == '\0' 
		  && (p_item->eVote.type == TALLIEDY 
		      || p_item->eVote.type == GROUPEDY))
		question[i] = '1';
	      break;
	    case 'q':
	    case 'Q':
	      if (question[i+1] == '\0')
		return NOT_OK;
	      break;
	    case '?':
	      redo = YES;
	      break;
	    default:
	      break;
	    }
	}
      if (redo == YES)
	{
	  who_menu(p_item);
	  *input = '\0';
	}
    }
  while (redo == YES);
  switch (who_voted(p_item, question))
    {
    default:
      /* impossible */
      break;
    case FAILURE:  /* message written to stderr */
      return NOT_OK;
    case TOO_LONG:
      printf("\nwho_voted:question too long.\n");
      return UNDECIDED;
      break;
    case STRING_OUT:
      printf("\n%s\n", message());
      break;
    case UID_LIST:
    case UID_LIST_MORE:
      {  /* here we use uid_report() to iterate through
	    the list of voters the Clerk returned */
	short i = -1;
	unsigned long uid;
	char *vote_str;
	while ((vote_str = uid_report(&uid)) != NULL)
	  printf("%c%-11s->   %s", (++i%3?'\t':'\n'), get_name(uid), vote_str);
	if (uid == 0)  /* flag that we ran out of resources */
	  {
	    printf("\nwho_voted: ran out of resources.\n");
	    return NOT_OK;
	  }
      }
      break;
    }		
  return OK;
}
/*************  void who_menu(...)   **********************
 *Draws the context-sensitive who-voted menu.
 **********************************************************/
void
who_menu(ITEM_INFO* p_item)
{
  if (p_item->eVote.priv_type == PRIVATE)
    {
      printf("\nWho-voted statistics are not available for item #%d.",
	     p_item->dropping_id);
      printf("\nIt is a PRIVATE item.");
      return;
    }
  printf("\nEnter a 'who-voted' question. "); 
  printf("\nThe question for item #%d can be:\n", p_item->dropping_id);
  printf("\n  '!ACC' to see who has not accessed item #%d.",
	 p_item->dropping_id);
  printf("\n   'ACC' to see who has accessed item #%d.", p_item->dropping_id);
  if (p_item->eVote.type == PLAIN)
    {
      printf("\n");
      return;
    }
  printf("\n  'Vote' to see who has voted on item #%d. ", p_item->dropping_id);
  printf("\n  '!Vote' to see who has not voted on item #%d. ",
	 p_item->dropping_id);
  if (p_item->eVote.priv_type == IF_VOTED)
    {
    }
  else
    /* only PUBLIC is left in the logic */
    {			
      if (p_item->eVote.type == TALLIEDY || p_item->eVote.type == GROUPEDY)
	{
	  printf("\n     'y' to see who has voted YES on item #%d.", p_item->dropping_id);
	  printf("\n     'n' to see who has voted NO on item #%d.", p_item->dropping_id);
	}
      else
	{
	  printf("\n   '< n' to see who has voted less than the number 'n'.");
	  printf("\n   '> n' to see who has voted more than the number 'n'.");
	  printf("\n   '= n' to see who has voted exactly n on item #%d.",
		 p_item->dropping_id);
	  printf("\nThe votes must be from %d to %d", p_item->eVote.min,
		 p_item->eVote.max);
	}
    }
  printf("\n");
  return;
}

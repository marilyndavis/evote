/* $Id: voterm.c,v 1.6 2003/01/24 22:15:19 marilyndavis Exp $ */ 
/**********************************************************
 *	../eVote/src/mail/mailui/voterm.c
 *     Functions relating to the voter.  'm' is for mail.
 *********************************************************
 *    Copyright (c) 1994...2015 Deliberate.com Patented.
 *    by Marilyn Davis
 **********************************************************/
#include<stdio.h>
#include "mailui.h"
static int mail_vote(int item_no, short the_vote, char *report);
/*********************************************************
 *         Adds the mail_voter to The Clerks version
 *         of the list of subscribers.
 **********************************************************/
#define TRY 20
OKorNOT
add_mail_voter(char * list, ACTION action)
{
  char guy[MAX_ADDRESS + 1];
  char *str;
  OKorNOT ret = OK;
  strcpy(guy, who_is(mail_voter));
  switch (enter_Clerk())
    {
    case OK:  /* already there */
      switch (current_action)
	{
	  /*	case (READ_ONLY | VOTE_ONLY | LOCK):
	  change_action(EVERYTHING);
	  i_am_leaving();
	  time_str[0] = '\0';    // suppress call to i_am_leaving 
	  return UNDECIDED;
	  case (READ_ONLY | VOTE_ONLY):
	  change_action(EVERYTHING); */
	case EVERYTHING:
	  sprintf(error_msg,"\n%s's subscription to %s is redundant.\n",
		  guy, list);
	  ret = PROBLEM;
	  break;
	case VACATION:
	  vacation_back_msg(guy);
	  ret = CANT;
	  break;
	case SIGNER:
	  sprintf(error_msg, "\n%s's status has been changed from being a"
		  "\n\"signature-only-participant to a regular reader/voter.\n\n", guy);
	  change_action(EVERYTHING);
	  ret = PROBLEM;
	  break;
	case VOTE_ONLY:
	  sprintf(error_msg,"\n%s's subscription to %s is redundant."
		  "\nHowever, %s has been a VOTE_ONLY member of %s.\n"
		  "\n%s's status has been changed to be a regular member.\n",
		  guy, list, guy, list, guy);
	  change_action(EVERYTHING);
	  ret = PROBLEM;
	  break;
	case READ_ONLY:
	  sprintf(error_msg,"\n%s is already a READ_ONLY member of %s.\n"
		  "\nFor %s to become a voting member, the list's owner,"
		  "\n%s-owner@%s must send the following to %s@%s:"
		  "\n\neVote approve < password > vote %s"
		  "\n\nwith the appropriate password filled in.",
		  guy, list, guy, list, whereami, list, whereami,
		  guy);
	  ret = CANT;
	  break;
	default:
	  /* impossible */
	  break;
	}
      i_am_leaving();
      time_str[0] = '\0';    /* suppress call to i_am_leaving */
      return ret;
    case NOT_OK:      
      return NOT_OK;
    case UNDECIDED:  /* This guy is not in the list yet */
      break;
    default:
      /* impossible */
      break;
    }
  str = send_joining(TRY, NO);
  /* NO = not local, no login, it's a mail voter */
  if (strcmp(str, "On twice") == 0)
    {
      sprintf(error_msg,"\n%s is on twice in %s for %d minutes!.\n",
	      guy, list, TRY);
      return NOT_OK;
    }
  if (action != EVERYTHING)
    change_action(action);
  i_am_leaving();
  return OK;
}
/****************************************************
 *        called from voterm.c functions
 *        and queriesm.c when there is trouble.
 *    good_part is the response to the good parts of the
 *    vote attempt.  The error is in error_msg.
 *****************************************************/
void
bad_vote_message(char * good_part)
{
  gen_header(SENDER, "Error:", YES);
  printf("\nYour command to eVote generated some confusion at this point:\n");
  print_tokens(NO);
  printf("%s", error_msg);
  if (good_part != NULL && good_part[0] != '\0')
    {
      printf("\nYour command was partially completed:\n");
      printf("%s", good_part);
      free(good_part);
      printf("\nPerhaps the instructions for \"%s\" \nwill help with the error.\n",
	     subject);
    }
  else
    {
      printf("\nPerhaps the instructions for \"%s\" \nwill help.\n",
	     subject);
    }
  display_info(NULL, NO);
  printf("\n\nYour entire message follows:\n");
  big_finish(0);
}	
/*********************************************************
 *       Drops the voter from the list and removes her
 *       vote from the statistics on all the open polls in
 *       the list.
 **********************************************************/
OKorNOT
drop_mail_voter(char *list, YESorNO only_if_non_voter)
{
  RTYPE cc;
  char *pv;
  /*	name = who_is(mail_voter); */
  switch (cc = send_drop_voter(list, mail_voter, only_if_non_voter))
    {
    case GOOD:
      return OK;
      break;
      /*		case DONE:
			fprintf(stderr,"\nCould not drop voter %s from %s.  Locked ballot, try again.\n", who_is(mail_voter), list);
			return PROBLEM;
			break; */
    case NO_CONF:
      fprintf(stderr,"\nCould not drop voter %s: No %s list.\n",
	      who_is(mail_voter), list);
      break;
    case NOT_GOOD:  /* won't happen in mail */
      fprintf(stderr,"\nCould not drop voter %s from %s: Participation has not expired.\n",
	      who_is(mail_voter), list);
      break;
    case NO_VOTER:
      /*      fprintf(stderr,"\nCould not drop voter %s from %s: nonexistant voter.\n",
	      pv = who_is(mail_voter), list);*/
      sprintf(error_msg,"\nCould not drop voter %s from %s: nonexistant voter.\n",
	      pv, list);
      break;
    case ON_LINE:  /* won't happen in mail */
      fprintf(stderr,"\nCould not drop voter %s: voter online.\n",
	      who_is(mail_voter));
      break;
    case FAILURE:
      fprintf(stderr,"\nMessage calls failed.\n");
      break;
    default:
      fprintf(stderr, "\neVote: Unexpected return in drop_voter: %d\n", cc);
      break;
    }
  return NOT_OK;
}
/*********************************************************
 *      Puts the mail_voter online to the Clerk.
 *      UNDECIDED means that the voter is not online because
 *         the voter hasn't joined the list.
 *      OK means that the voter is online and an i_am_leaving
 *         call is needed to clear the queue. current_action
 *         has been set.
 *      NOT_OK means that the voter is not online.  The reason
 *         is in error_msg.
 *********************************************************/
OKorNOT
enter_Clerk(void)
{
  char *str;
  short dummy_drop_days;
  if ((str = send_entering(lclist, &dummy_drop_days, TRY,
			  &current_action)) == NULL)
    {
      return UNDECIDED;
    }
  if (strcmp(str, "Sorry") == 0)
    {
      sprintf(error_msg, "\nRoot can't use eVote's interface.  Please use another login.\n");
      return NOT_OK;
    }
  if (strcmp(str, "On twice") == 0)
    {
      sprintf(error_msg,"\n%s is on twice in %s for %d minutes!.\n",
	      who_is(mail_voter), list, TRY);
      return NOT_OK;
    }
  return OK;
}
/************************************************************
 *    Returns YES if this voter has voted on anything.
 ************************************************************/
YESorNO
ever_voted(void)
{
  int i;
  for (i = 1; i <= *p_no_items; i++)
    {
      if (item_info[i].eVote.type == PLAIN
	  || item_info[i].eVote.vstatus == CLOSED)
	continue;
      if (have_i_voted(&item_info[i]))
	return YES;
    }
  return NO;
}
/*********************************************************
 *       Communicates the vote to The Clerk and prints
 *       some acknowledgement that it happened.
 *       Anything it has to say is stored at char* report.
 *       The number of chars written there is returned.
 *       Errors are stored in error_msg and a -1 is returned.
 **********************************************************/
int
mail_vote(int item_no, short the_vote, char *report)
{
  short old_vote;
  RTYPE rtype;
  char vote_pt[6];
  char * started = report;
  static YESorNO new_vote = NO;  /* If the user sends in several votes
				    for one group in one message, we
				    want to print the "thank you"
				    response for each vote. Without
				    this flag, it'll report that the
				    votes were changed from the min
				    votes after the first call since
				    The Clerk will see those calls
				    as changes to the first vote. 
				    This is because The Clerk 
				    automatically sets all the
				    user's votes to the minimum
				    on the first call.  */
  if ((p_item_copy + item_no)->eVote.type == GROUPEDY 
      || (p_item_copy +item_no)->eVote.type == TALLIEDY
      || (p_item_copy + item_no)->eVote.type == TIMESTAMP )
    {
      if (the_vote == 1)
	strcpy(vote_pt, "Yes");
      else
	strcpy(vote_pt, "No");
    }
  else
    sprintf(vote_pt,"%d", the_vote);
  switch (rtype = send_vote(p_item_copy + item_no, the_vote, &old_vote))
    {
    case FAILURE:
      sprintf(error_msg, "\nSystem troubles at %s: Your vote was not recorded.\n", whereami);
      bounce_error(SENDER | ADMIN | OWNER);
      break;
    case NOT_ALLOWED:
      vacation_voter();
      bounce_error(SENDER);
      return -1;
    case NO_MODS:
      sprintf(error_msg,"\nVoting is closed on \"%s\".", subject);
      return -1;  /* bad returns trigger bad_vote_message to SENDER */
      break;
    case NO_CHANGE:
      /*      if ((item_no == 0 || new_vote == NO) && !header_done)
	{   tried to vote twice 
	  gen_header(SENDER, "Error:", YES);   force error in subject
	  }*/
      if (item_no == 0)
	{
	  report += sprintf(report, "\nThank you.  You already voted \"%s\" on \n\"%s\".\n",
			    vote_pt, subject);
	  break;
	}
      if (new_vote == NO)
	{
	  report += sprintf(report, "\nThank you.  You already voted \"%s\" \non choice #%d, \"%s\".\n", vote_pt, item_no, (p_item_copy+item_no)->eVote.title);
	  break;
	}
    case GOOD:
      /* GOOD also new_vote == YES or NO_CHANGE on continuation
	 of a group */
      if (the_vote == READ)  /* here the voter erased old vote */
	{
	  if (item_no == 0)  /* Not a group */
	    {
	      if ((p_item_copy + item_no)->eVote.type == TALLIEDY
		  || (p_item_copy + item_no)->eVote.type == TIMESTAMP)
		{
		  if (old_vote == 1)
		    strcpy(vote_pt, "Yes");
		  else
		    strcpy(vote_pt, "No");
		}
	      else
		sprintf(vote_pt,"%d", old_vote);
	      report += sprintf(report, "\nYour old vote of \"%s\" has been removed on \n\"%s\".\n",
				vote_pt, subject);
	    }
	  else
	    report += sprintf(report, "\nYour old votes on \"%s\" \nhave been removed from all the choices.\n", subject);
	  break;
	}
      if (new_vote == YES || (old_vote == READ 
			      || old_vote == NOT_READ))  
				/* if the old_vote was
				   READ or NOT_READ, it's
				   the first vote for this
				   item. */
	{
	  new_vote = YES;
	  if (item_no == 0)
	    {
	      report += sprintf(report, "\nThank you for your \"%s\" on \"%s\".\n",
				vote_pt, subject);
	    }
	  else
	    report += sprintf(report, "\nThank you for your \"%s\" on choice #%d. \"%s\".\n",
			      vote_pt, item_no, (p_item_copy+item_no)->eVote.title);
	  break;
	}
      report += sprintf(report, "\nYour vote has changed from ");
      switch ((p_item_copy+item_no)->eVote.type)
	{
	case TALLIEDY:
	case TIMESTAMP:
	  report += sprintf(report, "\"%s\" to \"%s\" \non \"%s\".\n",
			    (old_vote? "Yes" : "No"), vote_pt, subject);
	  break;
	case TALLIEDN:
	  report += sprintf(report, "\"%d\" to \"%s\" \non \"%s\".\n",
			    old_vote, vote_pt, subject);
	  break;
	case GROUPEDY:
	  report += sprintf(report, "\"%s\" to \"%s\" \non choice #%d. \"%s\".\n",
			    (old_vote? "Yes" : "No"), vote_pt, item_no,
			    (p_item_copy+item_no)->eVote.title);
	  break;
	case GROUPEDN:
	case GROUPEDn:
	  report += sprintf(report, "\"%d\" to \"%s\" \non choice #%d. \"%s\".\n",
			    old_vote, vote_pt, item_no,
			    (p_item_copy+item_no)->eVote.title);
	  break;
	default:
	  /* impossible */
	  break;
	}
      break;
    case NOT_GOOD:
      if (item_no == 0)
	sprintf(error_msg,
		 "\nYour vote of %d on \"%s\" \nis outside the vote limits.\n",
		 the_vote, subject);
      else
	sprintf(error_msg,
		"\nYour vote, %d, on \"%s\" \nis outside the vote limits for choice #%d:\n\"%s\".\n",
		the_vote, subject, item_no, (p_item_copy+item_no)->eVote.title);
      return -1;
      break;
    case VIOLATES_SUM:
      sprintf(error_msg, "\nYour vote of \"%s\" on  choice #%d. \"%s\"\nviolates the sum-limit for \"%s\".  \nIt is not accepted.\n\nNote that you are forced to vote the minimum on all the choices.\n", 
	      vote_pt, item_no, (p_item_copy+item_no)->eVote.title,
	      subject);
      return -1;
      break;
    default:
      /* impossible */
      break;
    }
  return report - started;
}
/****************************************************/
void
move_voter(char *was, char *is)
{
  switch (send_move(was, is))
    {
    case NO_VOTER:
      sprintf(error_msg,"\n%s is not on the %s list or %s is invalid.\n", 
	      was, list, is);
      break;
    case NO_CONF:  // meaningless flag
      sprintf(error_msg,"\nProgrammer error!\n");
      bounce_error(SENDER | ADMIN);
      break;
    case FAILURE:
      sprintf(error_msg, "\nCan't communicate with the Clerk.\n");
      bounce_error(SENDER | ADMIN);
      break;
    case DONE:
      sprintf(error_msg, "\nThe ballots for %s are now associated with \n%s.\n",
	      was, is);
      return;
      break;
    case NOT_GOOD:
      sprintf(error_msg, "\nBoth email addresses %s and \n%s already exist.\n",
	      was, is);
      break;
    case REDUNDANT:
      sprintf(error_msg, "\n%s is already in the data and \n%s is not.\n",
	      is, was);
      break;
    default:
      /* impossible */
      break;
    }
  /*	from = is; */
  bounce_error(SENDER);
}
/**********************************************************/
void
on_vacation(YESorNO going)
{
  OKorNOT cc;
  char * from_report;
  int dummy;

  if (going)
    {
      if (current_action & VACATION)
	cc = UNDECIDED;
      else
	cc = change_action(VACATION);
    }
  else  /* back */
    {
      if (current_action == EVERYTHING 
	 || current_action == (VOTE_ONLY | READ_ONLY))
	cc = UNDECIDED;
      else
	{
	  if (confirm_this.status == VERIFIED)  /* || from_mailman? */
	    cc = change_action(EVERYTHING);
	  else
	    {
	      send_confirm(FOR_BACK);
	    }
	}
    }
  switch (cc)
    {
    case OK:
      gen_header(SENDER | OWNER, "Re:", YES);
      break;
    case UNDECIDED:
      gen_header(SENDER, "Re:", YES);
      vacation_redundant_msg(going);
      break;
    case NOT_OK:
      gen_header(SENDER | OWNER | ADMIN, "Error:", YES);
      printf("\nYour command was not totally processed because there is a problem"
	     "communicating with the Clerk.\n");
      break;
    default:
      /* impossible */
      break;
    }
  if (cc != OK)
    {      
      if (going)
	{
	  printf("\nRegardless, you will not receive mail from %s ", list);
	}
      else
	{
	  printf("\nRegardless, you will receive mail from %s ",
		 list);
	}
      printf("\nand your ballot is in tact.\n");
    }
  if (going)
    {
      drop_reader(from, lclist);
    }
  else 
    {
      from_report = copy_line(from_start, &dummy); 
      /* copy_line gives 1 extra byte*/
      strcat(from_report, "\n");
      add_reader(from, from_report, lclist);
      drop_reader(from, "bounces");
    }
  if (cc == UNDECIDED)
    {
      printf("\nAll is fixed.\n");
      big_finish(0);
    }
  
  if (going)
    {
      vacation_msg();
      big_finish(0);
    }
  if (cc != UNDECIDED)
    printf("\nWelcome back to %s %s.\n", list, from);
  big_finish(0);
} 
/**********************************************************/
char
*print_action(ACTION action)
{
  switch (action)
    {
    case EVERYTHING:
    case (VOTE_ONLY | READ_ONLY):
      return "a regular voter/reader";
      break;
    case VOTE_ONLY:
      return "a voter, but does not receive the list email";
      break;
    case VACATION:
      return "on vacation";
      break;
      /*    case LOCK:
    case (VOTE_ONLY | LOCK):
      return "on vacation";
      break; */
    case READ_ONLY:
      return "a non-voter";
      break;
    default:
      return "impossible";
      break;
    }
}
/*********************************************************/
OKorNOT
process_action(ACTION action, char * who, ACTION * old_action)
{
  if (does_conf_exist(lclist) == NO)
    {
      sprintf(error_msg, "\nNo eVoted list named %s. \n", list);
      return NOT_OK;
    }
  if ((mail_voter = who_num(who, NO)) == 0)
    {
      sprintf(error_msg, "\n%s is not on any list at %s.\n", who, whereami);
      return NOT_OK;
    }
  switch (enter_Clerk())
    {
    case OK:
      break;
    case NOT_OK:
      return NOT_OK;
      break;
    case UNDECIDED:
      sprintf(error_msg, "\n%s is not subscribed to %s.", who, lclist);
      return PROBLEM;
      break;
    default:
      /* impossible */
      break;
    }
  *old_action = current_action;
  if (action == *old_action)
    {
      sprintf(error_msg,"\n%s in %s is already marked as \n%s.\n",
	      who, list, print_action(action));
      i_am_leaving();
      return UNDECIDED;
    }
  switch (*old_action)
    {
    case EVERYTHING:
    case VOTE_ONLY:
      /*    case VOTE_ONLY + LOCK:
      switch (action)
	{
	case EVERYTHING:
	  break;
	case VACATION:
	  //	  sprintf(error_msg,"\n%s in %s is on vacation.  \nPlease send an \"eVote back\" command before changing to NO_VOTE.\n", who, list); 
	  i_am_leaving;
	  return NOT_OK;
	  break;
	default:
	  break;
	  }*/
      break;
    case READ_ONLY:
      switch (action)
	{
	case EVERYTHING:
	  break;
	case VOTE_ONLY:
	case VACATION:
	case SIGNER:
	  sprintf(error_msg,"\n%s in %s is a read-only member, perhaps \nan archive.  Please unsubscribe %s if you \nwish to stop email to %s.\n", 
		  who, list, who, who);
	  i_am_leaving();
	  return NOT_OK;
	  break;
	default:
	  /* impossible */
	  break;
	}
      break;
    default:
      /* impossible */
      break;
    }
  switch (change_action(action))
    {
    case UNDECIDED:
      sprintf(error_msg,"\n%s in %s is already marked as %s.\n",
	      who, list, print_action(action));
      i_am_leaving();
      return UNDECIDED;
      break;
    case OK:
      sprintf(error_msg, "\nAs directed by %s, \n%s in %s \nwas %s and is now %s.\n", 
	      from, who, list, 
	      print_action(*old_action), print_action(action));
      i_am_leaving();
      return OK;
      break;
    case NOT_OK:
      sprintf(error_msg,"\nCan't send an instruction.  Request failed.\n");
      i_am_leaving();
      return NOT_OK;
    default:
      /* impossible */
      break;
    }
  return OK;
}
/**********************************************************
 *        cc is the char after the token in the file.
 *        Figures out the vote instruction from the 
 *        tokens and then makes it happen.
 ***********************************************************/
void
process_mail_vote(int cc)
{
  YESorNO yesno = NO;
  int item = 0;
  int i;
  YESorNO first_time = YES;
  short the_vote = UNKNOWN;
  char * report, *buffer;
  int chunks;
  int chars_written;
  YESorNO empty_line;
  YESorNO no_good = NO;
  i = strlen(subject) - strlen(EXTRA) - 1;
  if (same(&subject[i+1], EXTRA) )
    {
      no_good = YES;
      subject[i] = '\0';
    }
  if (no_good || (petition == YES && p_item_copy->eVote.vstatus != CLOSED))
    {
      sprintf(error_msg, "\nThe poll on \"%s\" is a petition"
	      "\nand needs your signature along with your vote.  "
	      "\nTo receive information about it:", subject);
      strcat(error_msg,	"\n\n1.  Send a message to:\n\n\teVote@");
      strcat(error_msg, whereami);
      strcat(error_msg,	"\n\n2.  Your subject must be:\n\n\t");
      strcat(error_msg, subject);
      strcat(error_msg, "\n\n3.  Your message should say:\n\n\t");
      strcat(error_msg, "info");
      strcat(error_msg, "\n");
      bounce_error(SENDER);
    }
  if (p_item_copy->eVote.vstatus == CLOSED)
    {
      sprintf(error_msg, "\nVoting is closed on \"%s\".\n", 
	      subject);
      bad_vote_message(NULL);
    }
  if (p_item_copy->eVote.type == TALLIEDY
      || p_item_copy->eVote.type == TIMESTAMP
      || (p_item_copy->eVote.type == PLAIN 
	  && (p_item_copy + 1)->eVote.type == GROUPEDY))
    yesno = YES;
  if ((buffer = malloc((chunks = 2) * MAX_LINE))
     == NULL)
    {
      sprintf(error_msg,"\n%s is out of resources.  Please try your eVote command later.\n", whereami);
      bounce_error(SENDER | OWNER | ADMIN);
    }
  buffer[0] = '\0';
  report = buffer;
  do
    {
      empty_line = YES;
      do
	{
	  if (first_time == NO)
	    {
	      cc = get_token();
	      if (token[0] == '\0' || same(token, "end"))
		{
		  token[0] = '\0';
		  break;
		}
	    }
	  else
	    first_time = NO;
	  empty_line = NO;
	  if (yesno == NO && (token[0] == 'y'          /* yes */
			     || token[0] == 'Y'       /* yes */
			     || token[0] == 'n'       /* no */
			     || (token[0] == 'N' && yesno == NO)))  /* no */
	    {
	      sprintf(error_msg, "\nThe poll attached to \"%s\" is numeric.  \nA yes or no vote is not accepted.", subject);
	      bad_vote_message(buffer);
	    }
	  if (token[0] == 'y'            /* yes */
	     || token[0] == 'Y')        /* Yes */
	    {
	      the_vote = 1;
	      continue;
	    }
	  if (token[0] == 'n'            /* no */
	     || token[0] == 'N')        /* No */
	    {
	      the_vote = 0;
	      continue;
	    }
	  if ((token[0] <= '9' && token[0] >= '0') || token[0] == '-'
	     || token[0] == '+' )
	    {
	      i = 0;
	      if (token[0] == '-' || token[0] == '+')
		i = 1;
	      for ( ; token[i] != '\0'; i++)
		{
		  if (token[i] != '.' && token[i] > '9' && token[i] < '0')
		    {
		      sprintf(error_msg,"\neVote doesn't understand the word \"%s\".\n", token);
		      bad_vote_message(buffer);
		    }
		}
	      for (i = 0; token[i] != '\0'; i++)
		{
		  if (token[i] == '.')
		    {
		      break;
		    }
		}
	      if (token[i] == '.')
		{
		  token[i] = '\0';
		  item = atoi(token);
		  if (item > (p_item_copy+1)->eVote.no_in_group)
		    {
		      sprintf(error_msg,"\nThere are only %d choices for \"%s\".\n",
			      (p_item_copy+1)->eVote.no_in_group, subject);
		      bad_vote_message(buffer);
		    }
		  if (item == 0)
		    {
		      sprintf(error_msg,"\nThere is no choice numbered 0.\n");
		      bad_vote_message(buffer);
		    }
		  continue;
		}
	      the_vote = (short)atoi(token);
	      if (the_vote == 0)
		{
		  for (i = 0 ; token[i] != '\0'; i++)
		    {
		      if (token[i] != '0')
			{
			  sprintf(error_msg,
				  "\neVote can't understand the word \"%s\" in this context.", 
				  token);
			  bad_vote_message(buffer);
			}
		    }
		}
	      continue;
	    }
	  sprintf(error_msg, "\neVote can't understand the word \"%s\" in this context.", 
		  token);
	  bad_vote_message(buffer);
	}
      while (cc != '\n' && cc != EOF);
      if (empty_line)
	break; 
      if (the_vote == UNKNOWN)
	{
	  sprintf(error_msg,"\neVote can't determine how you want to vote.\n");
	  bad_vote_message(buffer);
	}
      if (p_item_copy->eVote.type == PLAIN
	 && item == 0)
	{
	  sprintf(error_msg, "\nYou must specify a choice for your vote on \"%s\".\n",
		  subject);
	  bad_vote_message(buffer);
	}
      if (the_vote  < (p_item_copy + item)->eVote.min 
	 || the_vote > (p_item_copy +item)->eVote.max)
	{
	  if (item == 0)
	    {
	      sprintf(error_msg,
		      "\nYour vote, %d, on \"%s\" \nis outside the vote limits.\n",
		      the_vote, subject);
	    }
	  else
	    {
	      sprintf(error_msg,
		      "\nYour vote, %d, on \"%s\" \nis outside the vote limits for choice #%d:\n\"%s\".\n",
		      the_vote, subject, item, (p_item_copy+item)->eVote.title);
	    }
	  bad_vote_message(buffer);
	}
      if (report - buffer >= (chunks - 1) * MAX_LINE)
	{
	  char * tmp;
	  if ((tmp = realloc(buffer, MAX_LINE * ++chunks))
	     != NULL)
	    {
	      report = tmp - buffer + report;
	      buffer = tmp;
	    }
	  else 
	    {
	      sprintf(error_msg,"\n%s is out of resources.  Please try your eVote command later.\nGot this far:\n%s", whereami, buffer);
	      bounce_error(SENDER | OWNER | ADMIN);
	    }
	}
      if ((chars_written = mail_vote(item, the_vote, report)) == -1)
	{
	  bad_vote_message(buffer);
	}
      else
	report += chars_written;
    }
  while (cc != EOF && token[0]);  /* token[0] == 0 at 'end' */
  gen_header(SENDER, "eVote Rcpt:", YES);
  printf("%s", buffer);
  display_poll_text(p_item_copy, NO);
  display_stats(p_item_copy, NO, NO);
  /*	if (p_item_copy->eVote.type == PLAIN)
	{
	printf("\nYour sum = %4d of %d allowed on the %d choices for",
	get_my_sum((p_item_copy+item)->dropping_id), 
	(p_item_copy+item)->eVote.sum_limit,
	(p_item_copy+item)->eVote.no_in_group);
	printf("\n\"%s\".", subject);
	}
  */
  big_finish(0);
}
/*********************************************************
 *         Removes the voter's vote from the poll attached
 *         to the current subject.
 **********************************************************/
void
process_remove(YESorNO from_petition)
{
  int i = 0;
  char report[MAX_LINE];
  if (p_item_copy->eVote.vstatus == CLOSED)
    {
      if (from_petition == YES)
	{
	  sprintf(error_msg, 
		  "\nThe petition on \"%s\" is closed.\nIt's too late to remove your signature.\n", 
		  subject);
	}
      else
	sprintf(error_msg, "\nVoting is closed on \"%s\".\nIt's too late to remove your vote.\n", 
		subject);
      bounce_error(SENDER);
    }
  if (petition == YES && from_petition == NO)
    {
      sprintf(error_msg, "\nThis poll is a petition.  To remove your signature:"
	      "\n\n\t1.  Send a message to:"
	      "\n\n\t\teVote@%s"
	      "\n\n\t2.  Your subject must be:"
	      "\n\n\t\t%s"
	      "\n\n\t3.  Your message should say:"
	      "\n\n\t\tremove\n", whereami, subject);
      bounce_error(SENDER);
    }
  if (p_item_copy->eVote.type == PLAIN)
    i = 1;
  if (have_i_voted(p_item_copy+i) == NO)
    {
      if (petition == YES)
	{
	  sprintf(error_msg, "\nYou have not signed the petition on \"%s\".\n",
		  subject);
	}
      else
	{
	  sprintf(error_msg,
		  "\nYou have not voted on \"%s\".  \nYou have no vote to remove.\n",
		  subject);
	}
      bounce_error(SENDER);
    }
  if (from_petition == YES)
    return;
  gen_header(SENDER, "eVote Rcpt:", YES);
  mail_vote(i, READ, report);
  printf("%s", report);
  display_stats(p_item_copy, NO, NO);
  big_finish(0);
}
/*********************************************************/
void
sub_to_eVote(char *address, char *dowhat, ACTION action)
{
  YESorNO new_list = NO;
  make_lclist();

  if (initialize() != OK)
    {
      bounce_error(OWNER | ADMIN);
    }
  if ((mail_voter = who_num(address, YES)) == 0)
    {
      sprintf(error_msg, "\nThe Clerk rejected the address: %s\n", address);
      bounce_error(OWNER | ADMIN);
    }
  if (does_conf_exist(lclist) == NO)
    {
      if (strcmp(dowhat, "VACATION") == 0) /* mailman only */
	exit(0);  /* let mailman handle this */
      if (strcmp(dowhat, "SUBSCRIBE") == 0 )
	{
	  if (start_list(lclist) != OK)
	    bounce_error(OWNER | ADMIN);
	  new_list = YES;
	}
      if (strcmp(dowhat, "UNSUBSCRIBE") == 0) 
	  exit(0);
    }
  
  if (strcmp(dowhat, "SUBSCRIBE") == 0)
    {
      /* for new_list, the voter was already subbed in start_list */
      if (new_list)
	return;
      switch (add_mail_voter(lclist, action))
	{
	case OK:        /* New subscription */
	case UNDECIDED: /* Was a VOTE_ONLY member */
	  break;
	case NOT_OK:    /* system trouble */
	case PROBLEM:   /* Redundant or something */
	  bounce_error(APPROVAL);
	  break;
	case CANT:
	  from = address;
	  bounce_error(APPROVAL | SENDER);
	  break;
	default:
	  /* impossible */
	  break;
	}
      return;
    }
  else
    {
      if (confirm_this.status != VERIFIED
	  || strcmp(dowhat, "VACATION") == 0)  /* gets mailman requests too */
	{
	  if (try_entering(NO) == UNDECIDED)
	    {
	      if (strcmp(dowhat, "VACATION") == 0) /*mailman*/
		{
		  exit(0);  /* let mailman handle this */
		}

	      sprintf(error_msg,"\nCould not drop voter %s from %s: nonexistant voter.\n",
		      address, list);
	      bounce_error(APPROVAL);
	    }

	  if (strcmp(dowhat, "VACATION") == 0)  /* mailman */
	    {
	      from = address;
	      on_vacation(action == VACATION ? YES : NO);
	    }

	  if (ever_voted())  /* unsubbing */
	    {
	      from = address;
	      if (current_action != VACATION)
		switch (change_action(VACATION))
		  {
		  case UNDECIDED:
		  case OK:
		    break;
		  case NOT_OK:
		    sprintf(error_msg, "\nYour command was not processed because there is a problem communicating \nwith the Clerk.\n");
		    bounce_error(SENDER | OWNER | ADMIN);
		  default:
		    /*impossible */
		    break;
		  }
	      i_am_leaving();
	      send_confirm(FOR_UNSUB);
	    }
	  i_am_leaving();
	}
      if (drop_mail_voter(lclist, NO) == NOT_OK)
	bounce_drop_error();
      if (confirm_this.status == VERIFIED)
	{
	  from = address;
	  gen_header(SENDER | OWNER, "Re:", YES);
	  printf("\nThanks for being with us %s.\n", address);
	  printf("\nYour ballot has been removed.\n");
	  big_finish(0);
	}
    }
  exit(0);
}
/*********************************************************
 *         Identifies the from field and tries to enter.
 *         force == YES when it's a petition and we'll
 *         call send_joining after this call.
 **********************************************************/
OKorNOT
try_entering(YESorNO force)
{
  OKorNOT cc;
  if (mail_voter != 0)
    ;
  else if (return_path != NULL 
	  && (mail_voter = who_num(return_path, force)) != 0)
    from = return_path;
  else if (reply_to != NULL
	  && (mail_voter = who_num(reply_to, force)) != 0)
    from = reply_to;
  else if (from != NULL
	  && (mail_voter = who_num(from, force)) != 0)
    ;
  else
    {
      mail_voter = 0;
      if (force == YES)
	return PROBLEM; /* Didn't accept email address */
      return UNDECIDED;  /* hasn't joined any list */
    }
  switch (cc = enter_Clerk())  /* initializes current_action */
    {
    case NOT_OK:   /* on twice */
      bounce_error(SENDER | ADMIN);
      break;
    case OK:         /* member of this list */
    case UNDECIDED:  /* hasn't joined this list */
      return cc;
      break;
    default:
      /* impossible */
      return cc;
      break;
    }
  return NOT_OK; /* impossible */
}
/************************************************************/
OKorNOT
unsign(char *name)
{
  time_t when;
  short old_vote;
  if ((mail_voter = who_num(name, NO)) == 0)
    {
      sprintf(error_msg,"\n%s is not at %s.\n", name, whereami);
      return NOT_OK;
    }
  switch (enter_Clerk())
    {
    case OK:
      break;
    case NOT_OK:
      sprintf(error_msg,"\nCan't communicate with the Clerk.\n");
      return NOT_OK;
      break;
    case UNDECIDED:
      sprintf(error_msg, "\n%s has not participated in %s.\n",
	      name, lclist);
      return NOT_OK;
      break;
    default:
      /* impossible */
      break;
    }
  if (have_i_voted(p_item_copy) == NO)
    {
      sprintf(error_msg,"\n%s has not signed %s.\n",
	      name, subject);
      return NOT_OK;
    }
  when = pull_time(p_item_copy);
  switch (send_vote(p_item_copy, READ, &old_vote))
    {
    case FAILURE:
      sprintf(error_msg, "\nSystem troubles at %s: Your vote was not recorded.\n", whereami);
      return NOT_OK;
      break;
    case NO_MODS:
    case NO_CHANGE:
      sprintf(error_msg, "\nProgramming error on remove_signature.\n");
      return NOT_OK;
      break;
    case GOOD:  
      break;
    default:
      /* impossible */
      break;
    }
  i_am_leaving();
  drop_mail_voter(lclist, YES);
  printf("\n%s has been unsigned.\nThe text removed was:\n", name);
  drop_signature(mail_voter, stdout, when);
  return OK;
}

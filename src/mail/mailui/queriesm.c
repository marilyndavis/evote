/* $Id: queriesm.c,v 1.3 2003/01/15 18:54:09 marilyndavis Exp $ */ 
 /**********************************************************
 *  queriesm.c  Functions for vote data queries.
 *      Info and processing for how-voted and who-voted
 *      commands.
 **********************************************************
 *    Copyright (c) 1994...2015 Deliberate.com Patented.
 *    by Marilyn Davis
 **********************************************************
 *    Translations notes:
 *     The Upper case VOTED, !VOTED, ACC, etc are for
 *     communicating with the Clerk and don't get translated.
 *     They never show in stdout.
 *     The lower case versions do need translations.
 *********************************************************/
#include<stdio.h>
#include"mailui.h"
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *   This next  declaration is listed here rather than in
 *   in the Clerk.h file because it is only used by this
 *   file.
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
char *message(void); 
static void how_info(void);
static void who_info(void);
/****************************************************************
 *   void failed_uid_report(void)
 *    Called from fill_spread() or process_who(void) if a uid_report
 *    ran out of memory.
 ****************************************************************/
void
failed_uid_report(void)
{
  sprintf(error_msg,"\nOut of resources in eVote.  A spreadsheet request has bounced.\n");
  printf("\nuid_report: insufficient resources.\n");
  printf("\n\nYour request cannot be completed at this time.");
  printf("\nPlease try again later.\n");
  big_finish(-2);
  bounce_error(OWNER | ADMIN);
}
/**********************************************************
 *    void gen_info(char *type)
 *        generates a return to sender message with
 *        some info.  Used for errors.
 **********************************************************/
void
gen_info(char *kind)
{
  printf("\nIn response to your command which starts:\n\n");
  print_first_line();
  if (error_msg[0] != '\0')
    printf("%s", error_msg);
  if (same(kind, "who"))
    who_info();
  else
    how_info();
  display_poll_text(p_item_copy, NO);
  display_stats(p_item_copy, NO, NO);
  big_finish(0);
}
/*************  void how-menu(...)   **********************
 *	void how_info(void)
 *	Context sensitive instructions for how-voted questions.
 **********************************************************/
void
how_info(void)
{
  char how_or_if[4] = "how";
  switch (ptype)
    {
    case IF_VOTED:
      strcpy(how_or_if, "if");
      printf("\nThe \"eVote how\" command for \"%s\" \ntells you if a particular person has voted yet.", subject);
    case PUBLIC:
      if (petition == YES)
	{
	  printf("\n\nTo see Charlie@deliberate.com's message, send email to ");
	  printf("\n%s@%s with the same subject, \n\"%s\".",
		 list, whereami, subject);
	  printf("\n\nYour message should say:");
	  printf("\n\n\teVote how ");
	  printf("Charlie@deliberate.com");
	  printf("\n\nIf you want to see several people's messages, list all their email \naddresses:");
	  printf("\n\n\teVote how \n\t");
	  printf("Charlie@deliberate.com");
	  printf("\n\tpres@whitehouse.gov");
	  printf("\n\tAnn@maclane.org");
	  printf("\n\nIf you want to see everybody's messages, use the \"eVote who\" command.");
	}
      else
	{
	  if (ptype != IF_VOTED)
	    printf("\nThe \"eVote how\" command for \"%s\" \ntells you how a particular person voted.", subject);
	  printf("\n\nTo see %s Charlie@deliberate.com voted, send email to ", how_or_if);
	  printf("\n%s@%s with the same subject, \n\"%s\".",
		 list, whereami, subject);
	  printf("\n\nYour message should say:");
	  printf("\n\n\teVote how ");
	  printf("Charlie@deliberate.com");
	  printf("\n\nIf you want to see %s several people voted, list all their email \naddresses:", how_or_if);
	  printf("\n\n\teVote how \n\t");
	  printf("Charlie@deliberate.com");
	  printf("\n\tpres@whitehouse.gov");
	  printf("\n\tAnn@maclane.org");
	  printf("\n\nIf you want to see %s everybody voted, use the \"eVote who\" command.", how_or_if);
	  printf("\n\nSend email to %s@%s with the same subject \n\"%s\", and with the message:",
		 list, whereami, subject);
	  printf("\n\n\teVote help who");
	  printf("\n\nto learn how the \"eVote who\" command works.");
	}
      break;
    case PRIVATE:
      printf("\nThis subject, \"%s\", has \na \"private\" poll attached.",
	     subject);
      printf("\n\nYou cannot use the \"eVote how\" command on a private poll.");
      break;
    }
}				
/*****************************************************************
 *  short how_voted  - returns the vote for the given guy.  If
 *  guy == NULL, the caller's vote is returned.  If not, the 
 *  vote is returned only if the ptype allows it.
 *****************************************************************/	
short
how_voted(char * guy, unsigned long how_uid)
{
  short the_vote;
  char he_has[35];
  short vote_sum;
  char vote_str[10];
  ITEM_INFO *p_item = p_item_copy;
  if (guy == NULL)
    {
      how_uid = mail_voter;
      strcpy(he_has,"You have");
      if (p_item_copy->eVote.vstatus == CLOSED)
	strcpy(he_has,"You");
      guy = "You";
    }
  else
    {
      sprintf(he_has,"%s has", guy);
      if (p_item_copy->eVote.vstatus == CLOSED)
	sprintf(he_has,"%s", guy);
    }
  if (p_item_copy->eVote.type != PLAIN)  /* the GROUP follows */
    {
      switch ((RTYPE)send_how(p_item_copy, how_uid, &the_vote, &vote_sum) )
	{
	case NO_VOTER:
	  if (petition == YES)
	    printf("\n%s not signed the petition.\n", he_has);
	  else
	    printf("\n%s not participated in this list.\n", he_has);
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
	case LATE:
	  if (petition == YES)
	    printf("\n%s was too late to sign the petition.\n", guy);
	  else
	    printf("\n%s joined too late to vote in this poll.\n", guy);
	  break;
	case NOT_VALID:
	  printf("\n%s left before this poll closed", guy);
	  if (strcmp(guy,"You") == 0)
	    printf("\nor you are a non-voting member.\n");
	  else
	    printf("\nor %s is a non-voting member.\n", guy);
	  break;
	case NOT_READ:
	case READ:
	  if (p_item_copy->eVote.vstatus == CLOSED)
	    printf("\n%s did not vote.\n", he_has);
	  else
	    printf("\n%s not yet voted.\n", he_has);
	  return the_vote;
	default:
	  if (ptype == IF_VOTED && how_uid != mail_voter)
	    {
	      printf("\n%s voted.\n", he_has);
	      return the_vote;
	    }
	  if (petition == YES)
	    {
	      time_t when = 0;
	      if (p_item->eVote.type == TIMESTAMP)
		{
		  when = (time_t)send_timestamp(p_item_copy, how_uid);
		}
	      if (print_signature(guy, how_uid, when) != OK)
		return (short)(-1);
	      return the_vote;
	    }
	  /* PUBLIC  or inquiring about one's own vote */
	  if (p_item_copy->eVote.type == TALLIEDY)
	    {
	      if (the_vote == 0)
		strcpy(vote_str, "NO");
	      else
		strcpy(vote_str, "YES");
	    }
	  else if (p_item_copy->eVote.type == TALLIEDN) /*  numeric vote */
	    {
	      sprintf(vote_str, "%d", the_vote);
	    }
	  printf("\n%s voted ", he_has);
	  printf("%s.\n", vote_str);
	}
      return the_vote;
    }
  do   /* grouped vote */
    {
      switch ((RTYPE)send_how(++p_item, how_uid, &the_vote, &vote_sum) )
	{
	case NO_VOTER:
	  printf("\n%s not participated in this list.\n", he_has);
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
	case LATE:
	  printf("\n%s joined too late to vote in this poll.\n", guy);
	  break;
	case NOT_VALID:
	  printf("\n%s left before this poll closed", guy);
	  if (strcmp(guy,"You") == 0)
	    printf("\nor you are a non-voting member.\n");
	  else
	    printf("\nor %s is a non-voting member.\n", guy);
	  break;
	case NOT_READ:
	case READ:
	  if (p_item->eVote.vstatus == CLOSED)
	    printf("\n%s did not vote.\n", he_has);
	  else
	    printf("\n%s not yet voted.\n", he_has);
	  return the_vote;
	default:
	  if (p_item->eVote.priv_type == IF_VOTED && how_uid != mail_voter)
	    {
	      printf("\n%s voted.\n", he_has);
	      return the_vote;
	    }
	  break;
	}
      /* PUBLIC  or inquiring about one's own vote */
      if (p_item->eVote.type == GROUPEDY)
	{
	  if (the_vote == 0)
	    strcpy(vote_str, "NO");
	  else
	    strcpy(vote_str, "YES");
	}
      else  /*  numeric vote */
	{
	  sprintf(vote_str, "%d", the_vote);
	}
      printf("\n%4s on %2d.  ", vote_str,
	     p_item->eVote.no_in_group 
	     - p_item->eVote.more_to_come);
      printf("\"%s\".", 
	     p_item->eVote.title);
    }
  while (p_item->eVote.more_to_come != 0);
  if (p_item->eVote.type == GROUPEDY
      && p_item->eVote.sum_limit != NO_LIMIT)
    {
      printf("\n%s used %d of the %d YESes possible.\n", he_has,
	     vote_sum, p_item->eVote.sum_limit);
    }
  /* GROUPEDN  or GROUPEDn */
  else if (p_item->eVote.sum_limit != NO_LIMIT)
    {
      strcpy(he_has, "your");
      if (guy != NULL)
	sprintf(he_has, "%s's", guy);
      printf("\nThe sum of %s votes on this group is %d.\n",
	     he_has,	vote_sum);
    }
  else
    printf("\n");
  return the_vote;
}
/*********************************************************
 *   void process_how(void)
 *     Processes a how voted command from the tokens.
 **********************************************************/
void
process_how(void)
{
  int cc;
  int whoid;
  void gen_info(char*);
  char * what = "poll";
  if (petition == YES)
    what = "petition";
  gen_header(SENDER, "Re:", YES);
  if (ptype == PRIVATE)
    {
      gen_info("how");
    }
  cc = get_token();
  if (same(token, "help") || same(token, "info") || same(token, "end"))
    gen_info("how");
  if (token[0] == '\0')
    {
      if (petition == YES)
	{
	  sprintf(error_msg, "\nYou need to specify an email address to receivethat person's message.");
	}
      else
	sprintf(error_msg, "\nYou need to specify an email address to learn %s that person voted.", (ptype == PUBLIC ? "how" : "if"));
      gen_info("how");
    }
  printf("\nIn response to your \"eVote how\" command regarding the %s:\n", what);
  printf("\n\t\"%s\"\n", subject);
  if (p_item_copy->eVote.type == PLAIN 
     && (p_item_copy+1)->eVote.priv_type == PUBLIC
     && (p_item_copy+1)->eVote.sum_limit != NO_LIMIT)
    printf("\nThe sum-limit for this group of %d choices is %d.\n",
	   (p_item_copy+1)->eVote.no_in_group, 
	   (p_item_copy+1)->eVote.sum_limit);
  do
    {
      if ((whoid = who_num(token, NO)) == 0)
	{
	  if (petition == YES)
	    printf("\n%s has not signed the petition.\n", token);
	  else
	    printf("\n%s is not a subscriber to any list at %s.\n",
		   token, whereami);
	}
      else
	{
	  if (p_item_copy->eVote.type == PLAIN
	     && (p_item_copy+1)->eVote.priv_type == PUBLIC)
	    printf("\n%s:\n", token);
	  cc = how_voted(token, whoid);
	}
      do
	{
	  cc = get_token();
	  if (token[0] == '\0' || same(token, "end"))
	    break;
	}
      while (same(token,"eVote") || same(token,"how"));
    }
  while (token[0] != '\0');
  if (petition == YES)
    {
      translate(DEFAULT_LANGUAGE, DISPLAY_PETITION_TEXT, p_item_copy,
		NO, NO, NO, NULL, NULL, MAYBE);
    }
  else
    {
      display_poll_text(p_item_copy, NO);
      display_stats(p_item_copy, NO, NO);
    }
  big_finish(0);
}
/*****************************************************************
 *   void process_who(cc)
 *        Processes a who voted request from the tokens.
 *        cc is the character that delimited the last token.
 *        On entry, token is either "who" or "spreadsheet".
 ***********************************************************/
void
process_who(int cc)
{
  short  number, item_no = 0;
  int ret_code;
  char question[10] = "";
  ITEM_INFO *new_p_item;
  char command[MAX_LINE + 1];
  YESorNO spreadsheet = NO;
  YESorNO just_one = NO;
  YESorNO first_line = YES;
  int real_participants = no_of_participants;
  unsigned long voters;
  if (petition == YES)
    process_names();
  gen_header(SENDER, "Re:", YES);
  if (ptype == PRIVATE)
    {
      gen_info("who");
    }
  if ((spreadsheet = is_spread()) == YES)
    {
      strcpy(question,">=!ACC");
    }
  sprintf(command,"eVote %s ", token);
  start_displays(p_item_copy);
  printf("\n");
  highlight("RESULTS OF YOUR EVOTE WHO COMMAND");
  if (p_item_copy->eVote.vstatus == CLOSED)
    {
      real_participants = p_item_copy->eVote.participants_when_closed;
    }
  else
    real_participants = no_of_participants;
  voters = get_mail_voters(p_item_copy->dropping_id);
  printf("\n%d participant%s  %ld voter%s \n", 
	 real_participants,
	 (real_participants == 1? "" : "s"),
	 voters, (voters == 1 ? "" : "s"));
  if (samen(lclist,"petition", 8))
    {
#ifdef ROSA
      if (!same(subject,"Mexico March"))
#endif
      printf("\nPetition signers' addresses are suppressed.\n");
    }
  do   /* for each line */
    {
      if (first_line == NO)
	{
	  strcpy(question,"");
	  strcpy(command,"");
	}
      new_p_item = p_item_copy;
      number = -1;
      item_no = 0;
      if (cc == '\n')
	cc = 1;
      while (cc != '\n' && cc != EOF)  /* parse the line */
	{
	  cc = get_token();
	  if (same(token, "end") || !token[0])
	    {
	      cc = EOF;
	      break;
	    }
	  if (same(token, "eVote") || same(token, "who"))
	    {
	      if (first_line == NO)
		{
		  strcat(command, token);
		  strcat(command," ");
		}
	      continue;
	    }
	  strcat(command, token);
	  strcat(command," ");
	  if (same(token, "all") || is_spread() == YES)
	    {
	      spreadsheet = YES;
	      continue;
	    }
	  if (same(token, "voted") || same(token,"vote"))
	    {
	      if (cc == '\n' || cc == EOF)
		{
		  strcpy(question,"VOTE");
		}
	      continue;
	    }
	  if (same(token, "help") || same(token, "info"))
	    gen_info("who");
	  if (same(token, "has"))
	    continue;
	  if (same(token, "=="))
	    continue;
	  if (same(token, "not"))
	    {
	      cc = get_token();
	      strcat(command, token);
	      if (same(token,"voted") || same(token, "vote"))
		{
		  strcpy(question,"!VOTE");
		  continue;
		}
	      sprintf(error_msg,
		      "\neVote can't interpret your \"eVote who\" command.");
	      gen_info("who");
	    }
	  if (same(token,"!voted") || same(token,"!vote"))
	    {
	      strcpy(question,"!VOTE");
	      continue;
	    }
	  if (token[0] == 'y'            /* yes */
	     || token[0] == 'n'         /* no */
	     || token[0] == 'Y'         /* Yes */
	     || token[0] == 'N')        /* No */
	    {
	      if (p_item_copy->eVote.type != TALLIEDY 
		 && p_item_copy->eVote.type != TIMESTAMP
		 && (p_item_copy->eVote.type != PLAIN
		     && (p_item_copy+1)->eVote.type != GROUPEDY))
		{
		  sprintf(error_msg,
			  "\nThis subject has a numeric poll attached.  You can't ask about 'yes' and 'no'.");
		  gen_info("who");
		}
	      if (ptype == IF_VOTED)
		{
		  int j;
		  j = strlen(command);
		  command[j-1]= '\0';
		  sprintf(error_msg,
			  "\nThis subject, \"%s\", has an \n\"if-voted\" poll attached.  You can't ask who has voted \"%s\".",
			  subject, &command[10]);
		  gen_info("who");
		}
	      if (token[0] == 'y'      /* yes */
		 || token[0] == 'Y')  /* Yes */
		{
		  strcat(question,"1");
		  continue;
		}
	      if (token[0] == 'n'     /* no */
		 || token[0] == 'N') /* No */
		{
		  strcat(question,"0");
		  continue;
		}
	    }
	  switch (token[0])
	    {
	    case 'w':   /* who */
	    case 'W':   /* Who */
	      if (get_cols(&cc) == -1)
		gen_info("who");
	      spreadsheet = YES;
	      break;
	    case '>':
	    case '<':
	    case '=':
	      strcpy(question, token);
	      {
		int j=0;
		while (token[j] != '\0' && (token[j] < '0' || token[j] > '9'))
		  j++;
		if (token[j] != '\0')
		  number= atoi(&token[j]);
	      }
	      continue;
	      break;
	    default:
	      break;
	    }
	  if (token[0] <= '9' && token[0] >= '0')
	    {
	      int i = -1;
	      while (token[++i] != '\0')
		{
		  if (token[i] == '.')
		    {
		      break;
		    }
		}
	      if (token[i] == '.')  /* we have an item number here */
		{
		  if (p_item_copy->eVote.type != PLAIN)
		    {
		      sprintf(error_msg,
			      "\nThe poll attached to \"%s\" is not a \ngrouped poll.\n", subject);
		      gen_info("who");
		    }
		  token[i] = '\0';
		  if ((item_no = atoi(token)) == 0)
		    {
		      sprintf(error_msg,"\neVote can't interpret your \"eVote who\" command.\n");
		      gen_info("who");
		    }
		  if (item_no > (p_item_copy + 1)->eVote.no_in_group)
		    {
		      sprintf(error_msg,
			      "\nThere are not %d choices in the poll attached to \n\"%s\".\n",
			      item_no, subject);
		      bad_vote_message(NULL);
		    }
		  continue;
		}
	      /* not item number but number */
	      if (ptype == IF_VOTED)
		{
		  int j;
		  j = strlen(command);
		  command[j-1]= '\0';
		  sprintf(error_msg,
			  "\nThis subject, \"%s\", has an \n\"if-voted\" poll attached.  You can't ask who has voted \"%s\".",
			  subject, &command[10]);
		  gen_info("who");
		}
	      number = atoi(token);
	      strcat(question, token);
	      continue;
	    }
	  if (same(token,"on"))
	    {
	      if (p_item_copy->eVote.type != PLAIN)
		{
		  sprintf(error_msg,
			  "\neVote doesn't understand the word, \"on\", in an \"eVote who\" command \nfor this poll.");
		  gen_info("who");
		}
	      continue;
	    }
	}
      /* done parsing a line */
      if (cc == EOF && !question[0])
	{
	  if (!first_line)
	    break;
	  strcpy(question, ">=!ACC");
	  if (p_item_copy->eVote.type == PLAIN 
	     && (p_item_copy+1)->eVote.priv_type == PUBLIC)
	    spreadsheet = YES;
	}
      if (get_mail_voters(p_item_copy->dropping_id) == 0)
	{
	  printf("\nIn response to your command:\n");
	  printf("%s\n", command);
	  printf("\nNo one has voted yet on \"%s\".\n",
		 subject);
	  display_stats(p_item_copy, NO, NO);
	  big_finish(0);
	}
      new_p_item = p_item_copy + item_no;
      if (question[0] == '\0' && spreadsheet == NO)
	{
	  if (cc != EOF)
	    continue;
	  sprintf(error_msg, "\neVote can't interpret your command:\n\n\t");
	  strcat(error_msg, command);
	  strcat(error_msg,"\n");
	  gen_info("who");
	}
      if (p_item_copy->eVote.type == PLAIN )
	{
	  if (same(question, ">=!ACC") || ptype == IF_VOTED
	     || same(question, "!VOTED")
	     || same(question, "VOTED")
	     || same(question, "VOTE")
	     || same(question, "!VOTE"))
	    {
	      new_p_item = p_item_copy +1;
	      just_one = YES;
	    }
	  else if (item_no == 0)
	    {
	      sprintf(error_msg,"\nOn a grouped poll like \"%s\", \nyou must specify which choice you wish your \"eVote who\" command \nto be applied to.\n",
		      subject);
	      gen_info("who");
	    }
	}
      if (spreadsheet == YES)
	{
	  if (p_item_copy->eVote.type != PLAIN 
	     || p_item_copy->eVote.priv_type != PUBLIC)
	    {
	      sprintf(error_msg,"\nYou can only ask \"%s\" on PUBLIC GROUPED polls.\n", command);
	      gen_info("who");
	    }
	  if (fill_spread() == NOT_OK)
	    {
	      char extra[100];
	      sprintf(extra,"\nPlease try your \"%s\" command later.",
		      command);
	      strcat(error_msg, extra);
	      bounce_error(SENDER | ADMIN | OWNER);
	    }
	  gen_spread();
	  big_finish(0);
	}
      if ((number != -1 && (number < new_p_item->eVote.min 
			    && question[0] != '>'))
	  || ((number > new_p_item->eVote.max
	       && question[0] != '<')))
	{
	  if (new_p_item->eVote.type == GROUPEDY
	     || new_p_item->eVote.type == TALLIEDY
	     || p_item_copy->eVote.type == TIMESTAMP )
	    {
	      sprintf(error_msg, "\nThe poll attached to \"%s\" allows \nyes and no votes only.  You can't ask \"%s\".\n", subject, command);
	      gen_info("who");
	    }
	  if (p_item_copy->eVote.type == PLAIN)
	    sprintf(error_msg,
		    "\n%d is out of range for votes on choice #%d, \"%s\".\n\nVotes must be from %d to %d.\n",
		    number, item_no, new_p_item->eVote.title,
		    new_p_item->eVote.min, new_p_item->eVote.max);
	  else
	    sprintf(error_msg,
		    "\n%d is out of range for votes on \"%s\".\n\nVotes must be from %d to %d.\n",
		    number, subject,
		    new_p_item->eVote.min, new_p_item->eVote.max);
	  gen_info("who");
	}
      switch (ret_code = who_voted(new_p_item, question))
	{
	case FAILURE:  /* message written to stderr */
	  sprintf(error_msg,"\nThere is some difficulty at %s right now.  \nPlease try your \"eVote who\" command later.", whereami);
	  bounce_error(SENDER | OWNER | ADMIN);
	  break;
	case TOO_LONG:
	  sprintf(error_msg, "\neVote can't make sense of your \"eVote who\" command.");
	  gen_info("who");
	  break;
	case STRING_OUT:
	case UID_LIST:
	case UID_LIST_MORE:
	  if (just_one == NO && first_line == YES
	     && new_p_item->eVote.type == TALLIEDN)
	    printf("\nPossible votes are from %d to %d.\n", new_p_item->eVote.min, new_p_item->eVote.max);
	  if (just_one == NO 
	     && new_p_item->eVote.vstatus != UNSEEN
	     && !(new_p_item->eVote.type >= GROUPED
		  && new_p_item->eVote.type <= GROUPEDN)
	     && first_line == YES)
	    {
	      unsigned long readers;
	      char vote_str[8];
	      char result[8];
	      void report_ratios(short dropping_id);
	      get_mail_stats(new_p_item->dropping_id,
			     &readers, vote_str, result);
	      /*	      printf("\n\n"); */
	      if (new_p_item->eVote.type == TALLIEDN)
		{
		  if (new_p_item->eVote.min == 
		     -new_p_item->eVote.max)
		    {
		      report_ratios(dropping_id);
		    }
		  printf("\n %9s --> Average of All Votes\n", result);
		}
	      else if (new_p_item->eVote.type == TALLIEDY
		      || p_item_copy->eVote.type == TIMESTAMP )
		{
		  float ratio = 0;
		  unsigned long yeses;
		  char *ratio_string(float ratio);
		  yeses = atoul(result);
		  if (voters - yeses > 0)
		    ratio = (float)yeses/(voters - yeses);
		  printf("\n %9s --> Total YES", result);
		  printf("\n %9s --> Ratio YES/NO", ratio_string(ratio));
		}
	    }
	  printf("\n\nYOUR COMMAND: %s\n", command);
	  if (ptype == PUBLIC && new_p_item->eVote.type >= GROUPED
	     && new_p_item->eVote.type <= GROUPEDN
	     && just_one == NO)
	    {
	      int number;
	      number =  new_p_item->eVote.no_in_group 
		- new_p_item->eVote.more_to_come;
	      printf("\nChoice %d.  \"%s\"  \n", number,
		     new_p_item->eVote.title);
	      if (new_p_item->eVote.type != GROUPEDY)
		printf("\nVotes are from %d to %d.\n", new_p_item->eVote.min, new_p_item->eVote.max);
	    }
	  if (ret_code == STRING_OUT)
	    {
	      char out_str[100];
	      strncpy(out_str, message(), 99);
	      if (samen(out_str,"\nNo one", 7))
		{
		  int number;
		  number =  new_p_item->eVote.no_in_group -
		    new_p_item->eVote.more_to_come;
		  if (new_p_item->eVote.type == TALLIEDY
		     || p_item_copy->eVote.type == TIMESTAMP 
		     || new_p_item->eVote.type == GROUPEDY)
		    {
		      int k = -1;
		      while (question[++k] != '\0')
			{
			  if (question[k] == '1')
			    {
			      strcpy(&question[k], "yes");
			      break;
			    }
			  else if (question[k] == '0')
			    {
			      strcpy(&question[k], "no");
			      break;
			    }
			}
		    }
		  printf("\nNo one has voted \"%s\" on",
			 question);
		  if (new_p_item->eVote.type >= GROUPED
		     && new_p_item->eVote.type <= GROUPEDN)
		    printf(" choice %d. \"%s\".\n", number,
			   new_p_item->eVote.title);
		  else
		    printf(" \"%s\".\n", subject);
		}
	      else
		printf("%s", message());
	      break;
	    }
	  /* Now STRING_OUT is out of the switch */
	  /* Only UID_LIST is left */
	  /* here we user uid_report() to iterate through
	     the list of voters the Clerk returned */
	  {
	    unsigned long uid;
	    char *vote_str;
	    while ((vote_str = uid_report(&uid)) != NULL)
	      {
		if (samen(vote_str, "!ACC", 4)
		   || samen(vote_str, "ACC", 3))
		  vote_str="not voted";
		if (just_one == YES)
		  {
		    if (same(question,"VOTE"))
		      vote_str =  "voted";
		    if (same(question, "!VOTE"))
		      vote_str =  "not voted";
		  }
		printf("\n %9s --> %s", vote_str, who_is(uid));
	      }
	    printf("\n");
	    if (uid == 0)
	      {
		failed_uid_report();
	      }  
	  }/* short i */
	  break;  
	default:
	  sprintf(error_msg, "\nUnexpected return in process_who: %d = %s\n",
		  ret_code, get_rtype((RTYPE)ret_code));
	  bounce_error(SENDER | ADMIN | OWNER);
	  break;
	} /* end of switch */
      if (just_one == NO 
	 && new_p_item->eVote.vstatus != UNSEEN
	 && new_p_item->eVote.type >= GROUPED
	 && new_p_item->eVote.type <= GROUPEDN)
	{
	  unsigned long readers;
	  char vote_str[8];
	  char result[8];
	  get_mail_stats(new_p_item->dropping_id,
			 &readers, vote_str, result);
	  if (new_p_item->eVote.type == GROUPEDN
	     || new_p_item->eVote.type == GROUPEDn)
	    {
	      printf("\n\n %9s --> Average of All Votes\n", result);
	    }
	  else if (new_p_item->eVote.type == GROUPEDY)
	    {
	      printf("\n %9s --> Total YES", result);
	    }
	}
      first_line = NO;
    }
  while (cc != EOF);  /* outer loop to parse message */
  big_finish(0);
}
/*************  void who_info(...)   **********************
 *		void who_info(void);
 *			Draws the context-sensitive who-voted instructions.
 **********************************************************/
void
who_info(void)
{
  switch (ptype)
    {
    case PRIVATE:
      printf("\nThe \"eVote who\" command cannot be applied to this subject, \n");
      printf("\"%s\".  The poll is a \"private\"\npoll so the votes are secret.",
	     subject);
      return;
    case IF_VOTED:
      printf("\nThe \"eVote who\" command tells you who has voted and who has not ");
      printf("\nvoted on the poll attached to \"%s\".", subject);
      printf("\n\nTo use the \"eVote who\" command:");
      printf("\n\n  1.  Send email to %s@%s.", list, whereami);
      printf("\n\n  2.  Your subject must be ");
      printf("\"%s\".", subject);
      printf("\n\n  3.  Your message can say:");
      printf("\n\n        eVote who has voted");
      printf("\n\n      to receive a list of the members of %s who ", list);
      printf("\n      have voted so far; or");
      printf("\n\n        eVote who has not voted");
      printf("\n\n      to receive a list of the members of %s who ", list);
      printf("\n      have NOT voted so far; or");
      printf("\n\n        eVote who");
      printf("\n\n      with no qualifier to receive a list of all the members"
	     "\n      of %s and whether or not each member"
	     "\n      has voted.\n", list);
      return;
    case PUBLIC:
      printf("\nThe \"eVote who\" command tells you who has voted a certain way\n");
      printf("on ");
      printf("\"%s\".\n", subject);
      if (p_item_copy->eVote.type == TALLIEDN)
	printf("\nThe votes on \"%s\" \nmust be from %d to %d\n",
	       subject, p_item_copy->eVote.min,
	       p_item_copy->eVote.max);
      printf("\nTo use the \"eVote who\" command on this poll, \n\"%s\":",
	     subject);
      printf("\n\n  1.  Send email to %s@%s.", list, whereami);
      printf("\n\n  2.  Your subject must be:");
      printf("\n\n        %s", subject);
      printf("\n\n  3.  Your message can say:");
      if (p_item_copy->eVote.type == TALLIEDY
	  || p_item_copy->eVote.type == TIMESTAMP )
	{
	  printf("\n\n        eVote who voted yes");
	  printf("\n\n      to receive a list of everyone who has voted ");
	  printf("YES.  Or:");
	  printf("\n\n        eVote who voted no");
	  printf("\n\n      to receive a list of everyone who has voted ");
	  printf("NO.");
	  printf("\n\n        eVote who");
	  printf("\n\n      to receive a list of everyone on the %s list "
		 "\n      and their votes.", list);
	}
      else if (p_item_copy->eVote.type == TALLIEDN)
	{
	  int num = (p_item_copy->eVote.min + p_item_copy->eVote.max)/2;
	  printf("\n\n        eVote who ");
	  printf("\n\n      to receive a list of everyone on the %s list "
		 "\n      and their votes.", list);
	  printf("\n\n        eVote who voted < %d", num);
	  printf("\n\n      to receive a list of everyone who has voted ");
	  printf("less than %d.", num);
	  printf("\n\n        eVote who voted > %d", num);
	  printf("\n\n      to receive a list of everyone who has voted ");
	  printf("more than %d,", num);
	  printf("\n\n        eVote who voted %d", num);
	  printf("\n\n      to receive a list of everyone who has voted ");
	  printf("exactly %d", num);
	  printf("\n\n      You can also use \"<=\" and \">=\".");
	}
      /*  now only PLAIN items -- really a group, is left */
      else if ((p_item_copy+1)->eVote.type == GROUPEDY)
	{
	  printf("\n\n        eVote who ");
	  printf("\n\n            or");
	  printf("\n\n        eVote spreadsheet ");
	  printf("\n\n      to receive a spreadsheet of everyone on the %s", list);
	  printf("\n      list and their votes on each of the choices.");
	  printf("\n\n         eVote who voted yes on 1.");
	  printf("\n\n      to receive a list of participants who have voted yes on the"
		 "\n      the first choice, ");
	  printf("\"%s\"; or:", (p_item_copy+1)->eVote.title);
	  printf("\n\n         eVote who voted no on 2.");
	  printf("\n\n      to receive of list of participants who have voted no on the ");
	  printf("\n      the second choice, \"%s\".", (p_item_copy+2)->eVote.title);					
	}
      else if ((p_item_copy+1)->eVote.type == GROUPEDN 
	      || (p_item_copy+1)->eVote.type == GROUPEDn)
	{
	  int num1, item, num2;
	  num1 = ((p_item_copy+1)->eVote.min + (p_item_copy+1)->eVote.max)/2;
	  item = (p_item_copy+1)->eVote.no_in_group/2;
	  if (item == 1)
	    item = 2;
	  num2 = ((p_item_copy+item)->eVote.min + (p_item_copy+item)->eVote.max)/2;
	  printf("\n\n         eVote who ");
	  printf("\n\n             or");
	  printf("\n\n         eVote spreadsheet ");
	  printf("\n\n      to receive a spreadsheet of everyone on the %s"
		 "\n      list and their votes on each of the choices."
		 "\n\n         eVote who voted < %d on 1.", list, num1);
	  printf("\n\n      to receive a list of everyone who has voted ");
	  printf("less than %d on", 
		 num1);
	  printf("\n      the first choice, ");
	  printf("\"%s\"",  (p_item_copy+1)->eVote.title);
	  printf("; or:");
	  printf("\n\n         eVote who voted > %d on %d.", num2, item);
	  printf("\n\n      to receive a list of everyone who has voted ");
	  printf("more than %d on", num2);
	  printf("\n      choice number %d, ", item);
	  printf("\"%s\"",  (p_item_copy+item)->eVote.title);
	  printf("; or:");
	  num1 = (p_item_copy+1)->eVote.no_in_group;
	  num2 = ((p_item_copy+num1)->eVote.min + (p_item_copy+num1)->eVote.max)/2;
	  printf("\n\n         eVote who voted %d on %d.", num2, num1);
	  printf("\n\n      to receive a list of everyone who has voted ");
	  printf("exactly %d on ", num2);
	  printf("\n      the last choice, \"%s\".", (p_item_copy+num1)->eVote.title);
	  printf("\n\n      You can also use \"<=\" and \">=\".");
	}
      printf("\n\nNOTE:  Your message can have several \"eVote who\" commands.\n");
    }
}

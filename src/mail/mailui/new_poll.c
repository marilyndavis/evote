/* $Id: new_poll.c,v 1.4 2003/01/15 18:54:09 marilyndavis Exp $ */ 
/**********************************************************
 *  new_poll.c  Functions that accept and check a new poll.
 *********************************************************
 **********************************************************/
#include<stdio.h>
#include"mailui.h"
extern char * good_strip;  /* bad extern in subject.c and only used here */
static void blank_item(ITEM_INFO* p_item);
static void collect_limits(ITEM_INFO *p_item);
/**********************************************************
 *        puts initial values in the p_item.
 **********************************************************/
void
blank_item(ITEM_INFO* p_item)
{
  p_item->eVote.type = ZIP;
  p_item->eVote.vstatus = NOT_KNOWN;
  p_item->eVote.priv_type = (PRIV_TYPE)9;
  p_item->eVote.max = 0;
  p_item->eVote.min = 0;
  p_item->eVote.close = (time_t)0;
  p_item->eVote.open = (time_t)0;
  p_item->eVote.no_in_group = 1;
  p_item->eVote.more_to_come = 0;
  p_item->eVote.sum_limit = NO_LIMIT;
  p_item->dropping_id = 0;
  p_item->static_id.network_id = 0;
  p_item->static_id.local_id = 0;
  strcpy(p_item->eVote.title, "none yet");
}
/**********************************************************
 *  Prints out a little message acknowledging
 *  that this is just a test poll.  
 **********************************************************/
void
check_message(void)
{
  printf("\nDear %s, \n", from);
  printf("\nThank you for initializing a poll/petition.  And thank you for"
	 "\nchecking the results of your command before sending to the \n"
	 "%s list.  Here's what the %s community would have"
	 "\nreceived if you didn't say \"check\".\n", list, list);
  printf("\n - - - - - - - - - - - - - - - - - - - - - - - - - - - - \n");
}
/**********************************************************
 *  Collects the vote limits for the item.
 *  Extracts them from the current token.
 **********************************************************/
static void
collect_limits(ITEM_INFO *p_item)
{
  char word[MAX_TOKEN + 1];
  char lims[MAX_TOKEN + 1];
  char *p_t;
  int num1, num2;
  char *p_num2 = NULL;
  int cc = 0;
  strcpy(lims, token);
  while (cc != EOF && lims[strlen(lims)-1] != ']')
    {
      cc = get_token();
      strcat(lims, token);
    }
  if (same(token,"[y/n]") || same(token,"[y-n]")
     || same(token,"[y, n]") || same(token,"[n, y]")
     || same(token,"[n/y]") || same(token,"[n-y]"))
    {
      p_item->eVote.type = TALLIEDY;
      p_item->eVote.min = 0;
      p_item->eVote.max = 1;
      return;
    }
  strcpy(word, lims+1);
  p_t = word;
  while (*p_t != ']' && *p_t != ',' && *p_t != '\0')
    p_t++;
  if (*p_t == ',')
    *p_t = '\0';
  p_num2 = p_t+1;
  if ((num1 = atoi(word)) == 0)
    {
      p_t = word-1;
      while (*(++p_t) != '\0')
	{
	  if (*p_t != '0' && *p_t != ' ')
	    {
	      return;
	    }
	}
    }
  while (*(++p_t) != ']' && *p_t != '\0')
    ;
  if (*p_t == '\0')
    return;
  *p_t = '\0';
  if ((num2 = atoi(p_num2)) == 0)
    {
      if (strlen(p_num2) == 0)
	return;
      p_t = p_num2 - 1;
      while (*(++p_t) != '\0')
	{
	  if (*p_t != '0' || *p_t != ' ')
	    {
	      return;
	    }
	}
    }
  if (num1 >= num2)
    {
      sprintf(error_msg,"\nThe minimum vote must be less than the maximum vote.\n");
      send_help("poll", SENDER, "Error:");
    }
  p_item->eVote.min = num1;
  p_item->eVote.max = num2;
  if (p_item->eVote.max > MAX_POS)
    {
      sprintf(error_msg,"\nThe maximum vote must be %d or less.\n", 
	      MAX_POS);
      send_help("poll", SENDER, "Error:");
    }
  if (p_item->eVote.min < MIN_NEG)
    {
      sprintf(error_msg,"\nThe minimum vote must be %d or more.\n", 
	      MIN_NEG);
      send_help("poll", SENDER, "Error:");
    }
  if (p_item->eVote.min == 0 && p_item->eVote.max == 1)
    {
      if (p_item->eVote.type == GROUPED)
	p_item->eVote.type = GROUPEDY;
      else
	p_item->eVote.type = TALLIEDY;
    }
  else
    {
      if (p_item->eVote.type == GROUPED)
	p_item->eVote.type = GROUPEDN;
      else
	p_item->eVote.type = TALLIEDN;
    }
  return;
}
/**********************************************************
 *  Processes an "eVote poll" instruction.
 **********************************************************/
void
process_poll(void)
{
  int cc;
  ITEM_INFO new_item;
  ITEM_INFO *gitem;
  char this_subject[TITLEN+1];
  short i = 0;
  int here = 0;
  YESorNO negs = NO;
  int min_tot;
  int item_no;
  YESorNO have_yn = NO;
  YESorNO have_num = NO;
  YESorNO item_no_found = NO;
  int new_index;
  YESorNO confirm_petition = NO;
  if (good_strip != NULL)
    {
      sprintf(error_msg,"Please don't use %s as the first word in your subject line."
	      "\nIt has special meaning to eVote.\n"
	      "\nPlease submit your poll or petition with a different subject line.\n\n", good_strip);
      bounce_error(SENDER);
    }
  strcpy(author_name, who_is(mail_voter));
  new_index = *p_no_items + 1;
  blank_item(&new_item);
  if (petition == MAYBE || petition == YES )
    {
      if (same(token,"petition"))
	{
	  new_item.eVote.vstatus = OPEN;
	  new_item.eVote.type = TALLIEDY;
	  new_item.eVote.min = 0;
	  new_item.eVote.max = 1;
	}
    }
  do
    {
      cc = get_token();
      if (token[0] == '\0')
	{
	  sprintf(error_msg, "\nThere's not enough information in your message to establish a poll.\n");
	  send_help("poll", SENDER, "Error:");
	}
      if (token[0] == '[')
	{
	  collect_limits(&new_item);
	  continue;
	}
      if (same(token,"check") || same(token, "checking"))
	{
	  just_checking = YES;
	  continue;
	}
      if (petition == MAYBE || petition == YES )
	{
	  if (same(token,"confirm"))
	    {
	      confirm_petition = YES;
	      continue;
	    }
	}
      if (same(token,"public"))
	{
	  new_item.eVote.priv_type = PUBLIC;
	  continue;
	}
      if (same(token, "private"))
	{
	  new_item.eVote.priv_type = PRIVATE;
	  continue;
	}
      if (same(token, "IF_VOTED") || same(token,"if-voted"))
	{
	  new_item.eVote.priv_type = IF_VOTED;
	  continue;
	}
      if (same(token, "visible"))
	{
	  new_item.eVote.vstatus = OPEN;
	  continue;
	}
      if (same(token, "hidden"))
	{
	  new_item.eVote.vstatus = UNSEEN;
	  continue;
	}
      if (same(token, "group") || same(token, "grouped"))
	{
	  new_item.eVote.type = GROUPED;
	  continue;
	}
      if ((i = atoi(token)) > 0 
	 && new_item.eVote.type == GROUPED)
	{
	  new_item.eVote.sum_limit = i;
	  continue;
	}
      sprintf(error_msg,"\neVote cannot understand the word, \"%s\".",
	      token);
      send_help(strncmp(lclist, "petition", 8) == 0 ? "petition" : "poll", 
		SENDER, "Error:");
    }
  while (cc != EOF && cc != '\n' && cc != '\r');
  /* first line read */
  if (petition == MAYBE)
    {
      copy_poll(NO);   /* look for 1st three letters */
      if (new_item.eVote.type == TALLIEDY 
	 ||  new_item.eVote.type == TIMESTAMP)
	{
	  new_item.eVote.type = TIMESTAMP;
	  if (dropping_id != 0)
	    { 
	      if (petition == PUNT)
		{
		  sprintf(error_msg,"The %s list already has a petition with the first"
			  "\nword of the subject the same as \"%s\".  This is not allowed"
			  "\nPlease submit your petition with a different subject line.\n\n",
			  list, subject);
		}
	      else /* exactly the same */
		{
		  sprintf(error_msg,"The %s list already has a petition with the name \"%s\"."
			  "\nPlease submit your petition with a different subject line.\n\n",
			  list, subject);
		}
	      bounce_error(SENDER);
	    }
	  petition = YES;
	}
      else  /* petition list but not a new petition */
	{
	  if (dropping_id != 0) /* might be ok */
	    { 
	      if (petition == PUNT)
		dropping_id = 0;
	    }
	  petition = NO;
	}
    }
  if (dropping_id != 0)
    {
      sprintf(error_msg, "The %s list already has a poll named \n\"%s\".\n\nPlease submit your poll with a different subject line.\n",
	      list, subject);
      bounce_error(SENDER);
    }
  new_item.eVote.author = mail_voter;
  strcpy(new_item.eVote.title, subject);
  if (new_item.eVote.type == ZIP)
    {
      if (error_msg[0]  == '\0')
	sprintf(error_msg, "\neVote can't determine the minimum and maximum votes for your poll.\n");
      send_help("poll", SENDER, "Error:");
    }
  if (new_item.eVote.vstatus == NOT_KNOWN)
    {
      sprintf(error_msg, "\neVote can't determine if the tally should be VISIBLE or HIDDEN while the \npoll is open.\n");
      send_help("poll", SENDER, "Error:");
    }
  if (new_item.eVote.priv_type == 9)
    {
      sprintf(error_msg, "\neVote can't determine if the poll is \"public\", \"private\", or \"if-voted\".\n");
      send_help("poll", SENDER, "Error:");
    }
  if (!(new_item.eVote.type >= GROUPED && new_item.eVote.type <= GROUPEDN))
    {
      /* find the message command, if it's there */
      while (cc != EOF && !same(token,"message") && !same(token,"message:")
	    && !same(token,"form") && !same(token,"form:")
	    && !same(token,"report") && !same(token,"report:"))
	{
	  cc = get_token();
	}
      if (petition != NO
	 && new_item.eVote.type == TALLIEDY)
	{
	  petition = YES;
	  new_item.eVote.type = TIMESTAMP;
	}
      while (same(token,"form") || same(token,"form:")
	    || same(token,"report") || same(token,"report:"))
	{
	  if (!petition)
	    {
	      sprintf(error_msg,"\nThis poll is not a petition so forms and reports are inappropriate.  \nPetitions must be initialized as YES/NO votes.\n");
	      send_help("petition", SENDER, "Error:");
	    }
	  if (same(token,"form:")|| same(token, "form"))
	    {
	      parse_form(cc);
	    }
	}
      if (just_checking == NO && eVote_new_items(&new_item, 1) != OK)
	{
	  sprintf(error_msg, "\neVote has experienced an internal problem."
		  "\n Please try your message again later.\n");
	  bounce_error(SENDER | ADMIN | OWNER | DEVELOPER);
	}
      if (petition == YES)
	{
	  if (just_checking)
	    {
	      new_item.eVote.open = now;
	      finish_petition(SENDER, &new_item, YES, confirm_petition);
	    }
	  finish_petition(LIST, &item_info[new_index], NO, 
			  confirm_petition);
	}
      /* finish_petition never returns */
      if (just_checking == NO)
	{
	  store_poll_text(&item_info[new_index]);
	}
      if (just_checking == YES)
	{
	  new_item.eVote.open = now;
	  /* time(&new_item.eVote.open); */
	  instruct(SENDER, &new_item, YES); /* instruct finishes */
	}
      instruct(LIST, &item_info[new_index], YES);
    }
  /* Now we have a grouped item */
  if ((gitem = malloc(3* sizeof(ITEM_INFO)))
     == NULL)
    {
      sprintf(error_msg,"\n%s is out of resources right now.  Please try your message later.\n", whereami);
      fprintf(stderr,"\nCan'd allocate space for a group.\n");
      bounce_error(SENDER | OWNER | ADMIN);
    }
  memcpy(gitem, &(new_item), sizeof(ITEM_INFO));
  item_no = 0;
  while (cc != EOF && !same(token,"message") && !same(token,"message:")
	&& !same(token,"form") && !same(token, "form:"))
    {
      YESorNO line_start;
      int left;
      line_start = YES;
      do
	{
	  cc = get_token();
	  if (cc == EOF && token[0] == '\0')
	    break;
	  if (line_start == YES)
	    {
	      if (same(token,"message") || same(token,"message:")
		  || cc == EOF)
		 /*		 && (cc == '\n' || cc == EOF)) */
		break;
	      if (++item_no > 2)
		{
		  ITEM_INFO* tmp;
		  tmp = realloc(gitem, sizeof(ITEM_INFO)*(item_no + 1));
		  if (tmp == NULL)
		    {
		      sprintf(error_msg,"\n%s is out of resources right now.  Please try your message later.\n", whereami);
		      fprintf(stderr,"\nCan't realloc more space for group of %d.\n",
			      item_no);
		      bounce_error(SENDER | OWNER | ADMIN);
		    }
		  gitem = tmp;
		}
	      line_start = NO;
	      blank_item(gitem + item_no);
	      here = 0;
	    }
	  /* check for item number -- ignore it */
	  if (token[0] >= '0' && token[0] <= '9' && item_no_found == NO)
	    {
	      i = 0;
	      while ((token[++i] != '\0' 
		      && (token[i] >= '0' && token[i] <= '9'))
		     || token[i] == '.')
		{
		  if (token[i] == '.' && token[i+1] == '\0')
		    {
		      item_no_found = YES;
		    }
		}
	      if (item_no_found == YES)
		continue;
	    }
	  if (token[0] == '[' && (cc == '\n' || cc == EOF))
	    {
	      collect_limits(gitem + item_no);
	      continue;
	    }
	  left = TITLEN - here - 1;
	  if (left > 0)
	    {
	      if (strlen(token) > (unsigned)left)
		token[left] = '\0';
	      here += sprintf(&this_subject[here],
			      "%s ", token);
	    }
	}
      while (cc != '\n' && cc != EOF);
      item_no_found = NO;
      if ((cc == EOF && token[0] == '\0') 
	 || same(token,"message") || same(token,"message:")
	 || same(token,"form") || same(token,"form:"))
	break;
      this_subject[here - 1] = '\0';
      if (strlen(this_subject) > TITLEN)
	{
	  strncpy((gitem+item_no)->eVote.title, this_subject, TITLEN);
	  (gitem+item_no)->eVote.title[TITLEN] = '\0';
	}
      else
	strcpy((gitem+item_no)->eVote.title, this_subject);
    }
  /* now we have all the items */
  min_tot = 0;
  if (item_no == 1 && (gitem+1)->eVote.type == TALLIEDY)
    { /* we allow only one on tallied for inclusion in petitions */
      sprintf(error_msg, "\nWe need more choices than one!\n");
      send_help("poll", SENDER, "Error:");
    } 
  if (item_no == 0)
    {
      sprintf(error_msg, "\nWe need some choices to vote on!\n");
      send_help("poll", SENDER, "Error:");
    }
  for (i = 1; i <= item_no; i++)
    {
      (gitem + i)->eVote.author = mail_voter;
      if (	(gitem + i)->eVote.type == TALLIEDY)
	{
	  have_yn = YES;
	  (gitem + i)->eVote.type = GROUPEDY;
	}
      else if ((gitem + i)->eVote.type == TALLIEDN)
	{
	  have_num = YES;
	  (gitem + i)->eVote.type = GROUPEDN;
	  if ((gitem + i)->eVote.max < 10 && (gitem + i)->eVote.min >= 0)
	    (gitem + i)->eVote.type = GROUPEDn;
	}
      else
	{
	  sprintf(error_msg, "\neVote can't determine the vote limits for all the choices in your poll.\n");
	  send_help("poll", SENDER, "Error:");
	}
      (gitem + i)->eVote.vstatus = new_item.eVote.vstatus;
      (gitem + i)->eVote.priv_type = new_item.eVote.priv_type;
      (gitem + i)->eVote.no_in_group = item_no;
      (gitem + i)->eVote.more_to_come = item_no - i;
      (gitem + i)->eVote.sum_limit = new_item.eVote.sum_limit;
      min_tot += ((gitem +i)->eVote.min < 0 ? 0 : (gitem + i)->eVote.min);
      if ((gitem+i)->eVote.min < 0)
	negs = YES;
    }
  if (have_yn == YES && have_num == YES)
    {
      for (i = 1; i <= item_no; i++)
	{
	  if (	(gitem + i)->eVote.type == GROUPEDY)
	    (gitem + i)->eVote.type = GROUPEDN;
	  if ((gitem + i)->eVote.max < 10 
	     && (gitem + i)->eVote.min >= 0)
	    (gitem + i)->eVote.type = GROUPEDn;
	}
    }
  if ((gitem+1)->eVote.type == GROUPEDY 
     && (gitem+1)->eVote.sum_limit != NO_LIMIT
     && ((gitem+1)->eVote.no_in_group < (gitem+1)->eVote.sum_limit))
    {
      sprintf(error_msg,"\nVoters have more yeses to distribute than there are choices. \nThis doesn't make sense.");
      send_help("poll", SENDER, "Error:");
    }
  if (min_tot > new_item.eVote.sum_limit)
    {
      sprintf(error_msg, "\nThe sum of the %s votes on your choices is %d.  \nThis exceeds %d, the number of %s votes each voter is allowed.  \nThis is an impossible set of choices.\n", 
	      (negs ? "positive minimum" : "minimum"),
	      min_tot, new_item.eVote.sum_limit,
	      (negs ? "positive votes" : "votes"));
      send_help("poll", SENDER, "Error:");
    }
  if (min_tot == new_item.eVote.sum_limit)
    {
      sprintf(error_msg, "\nThe sum of the %s votes on your choices, %d, \nequals the number of votes each voter is allowed.  This doesn't leave \nany decision for the list members to make.\n", 
	      (negs ? "positive minimum" : "minimum"), min_tot);
      send_help("poll", SENDER, "Error:");
    }
  gitem->eVote.type = PLAIN;
  if (just_checking == NO)
    {
      if (eVote_new_items(gitem, 1) == OK)
	if (eVote_new_items(gitem + 1, item_no) != OK)
	  {
	    sprintf(error_msg, "\neVote has experienced an internal problem. \n Please try your message again later.\n");
	    bounce_error(SENDER | OWNER | ADMIN | DEVELOPER);
	  }
    }
  if (just_checking == NO)
    {
      store_poll_text(&item_info[new_index]);
    }
  if (just_checking == YES)
    {
      new_item.eVote.open = now;
      instruct(SENDER, gitem, YES);  /* instruct finishes */
    }
  instruct(LIST, &item_info[new_index], YES);
}

/* $Id: do_eVote.c,v 1.6 2003/10/20 17:07:34 marilyndavis Exp $ */ 
/**********************************************************
 *  ../eVote/src/mail/mailui/do_eVote.c
 *  do_eVote.c  - driver for processing eVote requests
 ***********************************************************
 *    Copyright (c) 1994...2015 Deliberate.com Patented.
 *    by Marilyn Davis
 **********************************************************/
#include<stdio.h>
#include "mailui.h"
/*******************************************************
 *    void do_eVote(int cc)
 *     Processes a message when "eVote" is the first
 *     word.  "eVote" is the current token.
 *     int cc is the character that delimited the
 *     "eVote".
 *******************************************************/
void
do_eVote(int cc)
{
  if (cc == EOF)
    send_help("help", SENDER, "Error:");
  cc = get_token();
  if (same(token,"help"))
    {
      get_help();
    }
  if (set_up_eVote() != OK) /* Identifies the list, and user */
    {
      err_need_to_subscribe();
      bounce_error(SENDER);
    }
  /* if we get this far, communication to the list is working
     the list exists and, if mail_voter != 0, we're into
     the database.  */
  if (same(token,"list") || same(token,"lists"))
    list_polls(NO);    /* Lists the subjects with polls attached */
  if (current_action == READ_ONLY)
    {
      sprintf(error_msg,"\nYou can not participate in eVoting.  \nYou are a read_only member of %s.\n", list);
      bounce_error(SENDER | OWNER);
    }
  if (same(token,"approve"))
    approve();
  if (same(token,"vacation"))
    process_vacation(YES);     /* in ../list.c -- listserve dependent */
  if (same(token,"back"))
    process_vacation(NO);
  if (current_action == VACATION)
    {
      vacation_voter();  /* message depends on listserve */
      bounce_error(SENDER);
    }      
  if (same(token,"members"))
    members();
  if (subject == NULL || subject[0] == '\0')
    {
      sprintf(error_msg, "\nYour eVote command requires a subject.\n");
      send_help("help", SENDER, "Error:");
    }
  else
    {
      copy_poll(YES); /* If dropping_id != 0, we have a polled
			 subject */
    }
  if (dropping_id == 0 && same(token,"info"))
    {
      list_polls(YES);  /* YES == it's for an error */
    }
  if (same(token,"info"))  /* a poll exists */
    {
      if (petition == YES)
	{
	  /* NULL copies p_item_copy */
	  send_petition_info(SENDER, NULL, NO, YES, NO); 
	}
      instruct(SENDER, NULL, NO);  /* NO, it's not a new poll */
    }
  if (same(token,"poll") == YES
     || same(token,"petition") == YES)
    {
      process_poll(); 
    }
  if (dropping_id == 0)
    {
      list_polls(YES);  /* YES == it's for an error */
    }
  if (same(token,"names"))   /* for petitions only */
    {
      process_names();
    }
  if (same(token,"stats"))
    {
      if (petition == YES)
	{
	  send_petition_info(SENDER, NULL, NO, YES, NO);
	}
      send_stats_only();
    }
  if (same(token,"who"))
    process_who(cc);
  if (is_spread() == YES)
    process_spread(cc);
  if (same(token,"how"))
    process_how();
  if (same(token,"close"))
    process_close(NO);
  if (same(token,"remove"))
    process_remove(NO);
  if (same(token,"drop"))
    process_drop(NO, NO);
  process_mail_vote(cc);
}
/************************************************************/
OKorNOT
initialize(void)
{
  start_up(NO);
  if (say_hello() != OK)
    {
      /* error already handled */
      /*      char no_Clerk[] ="\neVote is unable to communicate with its vote-server, The Clerk.\n";
      sprintf(error_msg, "%sPlease try your message again later.\n", no_Clerk);
      fprintf(stderr, "%sPlease start The Clerk as 'clerk' by giving the command 'eVote'.\n", no_Clerk); */
      return NOT_OK;
    }
  return OK;
}
/*********************************************************
 *   parse_cf   -  Parses the eVote.cf file for the host
 *             name (whereami), listdir (directory with
 *             lists), mailer (command to send mail).
 *             returns 0 if everything is OK.
 *             returns a list of addressees if there
 *             is a problem.
 *********************************************************/
int
parse_cf(void)
{
  eVote_mail_to = find_default("EVOTE_MAIL_TO", YES, NO);
  mailer = find_default("MAILER", YES, NO);
  listdir = find_default("LISTDIR", YES, NO);
  whereami = find_default("WHEREAMI", YES, NO);
  time_out = atoi(find_default("TIME_OUT", NO, NO));
  if (mailer == NULL || listdir == NULL || mailer == NULL)
    return (SENDER | OWNER | ADMIN);  
  return 0;
}
/*******************************************************/
OKorNOT
set_up_eVote(void)
{
  if (initialize() != OK)
    {
      bounce_error(SENDER | ADMIN | OWNER);
    }
  if (does_conf_exist(lclist) == NO)
    {
      /* A new list is generated when the first subscription
	 notice arrives. */
      fprintf(stderr, "\nNo eVoted list named %s.\n", list);
      fprintf(stderr, "This can't happen!\n");
      sprintf(error_msg, "\nNo eVoted list named %s.\n", list);
      bounce_error(SENDER | ADMIN | OWNER);
    }
  return try_entering(NO);
}

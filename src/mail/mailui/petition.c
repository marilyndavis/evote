/* $Id: petition.c,v 1.3 2003/01/15 18:54:09 marilyndavis Exp $ */ 
/**********************************************************
 *  ../eVote/src/mail/mailui/petition.c
 *   Functions for a petition 
 *********************************************************
 **********************************************************/
#include<stdio.h>
#include<unistd.h>
#include<sys/stat.h>
#include "mailui.h"
#include <errno.h>
/* These are exported into pet_out_xx.c's only */
YESorNO give_email;  /* also exported to signatures.c */
YESorNO into_eVote = NO;  /* this keeps do_petition from
			     entering twice if it switches
			     languages */
YESorNO new_signer = NO;  
YESorNO petition = NO;
void leave(void);
void too_late(void);
/* available for general export */
int no_pet_votes;
ITEM_INFO* p_first_vote_item;
char * extra_subject;
YESorNO remove_sig = NO;
/* local */
static void parse_and_store_translations(ITEM_INFO * p_item, 
					 YESorNO just_checking,
					 YESorNO confirm);
static OKorNOT remove_pet_dir(ITEM_INFO *p_item, int whom);
/* static OKorNOT unsign(char *name); I think it's obsolete */
/************************************************************
 *     Puts a new email address into the database as a
 *     signer of the petition.
 ************************************************************/
void
enter_signer(void)
{
  char * str;
  if (mail_voter == 0)
    {
      switch (try_entering(YES))
	{
	case UNDECIDED:
	  break;
	case PROBLEM:
	  sprintf(error_msg,"\nEmail address rejected by Clerk.\n");
	  bounce_error(SENDER | OWNER);
	  break;
	default:
	  sprintf(error_msg,"\nProgrammer error!\n");
	  bounce_error(SENDER | OWNER);
	  break;
	}
    }
  str = send_joining(TRY, NO);
  new_signer = YES;
  change_action(SIGNER);
}
/************************************************************
 *  Called from new_poll: process_poll to finish the new petition.
 ************************************************************/
void
finish_petition(int whom, ITEM_INFO *p_item, 
		YESorNO just_checking,
		YESorNO confirm_petition)
{
  if (!just_checking)
    make_sig_dir(p_item, lclist);
  parse_and_store_translations(p_item, just_checking, confirm_petition);
  if (store_form_template(p_item) != OK)
    forget_petition(whom, p_item, just_checking);
  send_petition_info(whom, p_item, YES, YES, just_checking);
}
/************************************************************/
void
forget_petition(int whom, ITEM_INFO * p_item, YESorNO dont_clerk)
{  
  if (!dont_clerk)
    if (drop_items(p_item, 1) != 1)
      {
	strcat(error_msg, "\n\nPetition is in eVote but petition text is not stored!\n");
      }
  drop_translations(lclist, p_item);
  remove_pet_dir(p_item, whom);
  if (whom)
    bounce_error(whom);
}
/**********************************************************
 *    void leave();  Only linked to pet_out_xx.c
 **********************************************************/
void
leave(void)
{
  if (current_action == UNSET)
    return;
  if (new_signer == YES)
    {
      i_am_leaving();
      drop_mail_voter(list, NO);
      current_action = UNSET;
    }
  else if (current_action != SIGNER && current_action != EVERYTHING)
    {
      change_action(SIGNER);
    }
}
/************************************************************
 *       Returns the key for a file name that depends on
 *       the string -- which should be a petition title.
 *       The key will be the first word of the petition title,
 *       if it is at least 5 characters long.  If it is shorter,
 *       the second word is concatonated, etc.  The key is 
 *       truncated to be no more than 10 characters long.
 *************************************************************/
char
*pet_file_key(char * str)
{
  static char key[MAX_KEY_LEN + 1];
  int j, k;
  for (k = 0, j = 0; str[k] != '\0'
	 && j < MAX_KEY_LEN; k++)
    {
      if (str[k] != ' ' 
	  && str[k] != '\t')
	key[j++] = str[k];
      else
	if (j >= MIN_KEY_LEN)
	  break;
    }
  key[j] = '\0';
  return key;
}
/*********************************************************
 *      Returns the directory for the petition data
 *      with the name attached.
 **********************************************************/ 
char *
pet_fname(char * name, ITEM_INFO * p_item)
{
  static char fname[PATHLEN + 1];
  int i;
  char * title;
  strcpy(fname, listdir);
  i = strlen(fname) + 1;
  while (fname[--i] != '/')
    ;
  if (p_item == NULL)
    {
      title = subject;
    }
  else
    {
      title = p_item->eVote.title;
    }
  if (name[0] == '\0')
    sprintf(&fname[i], "/polls/%s/%s", lclist,
	    pet_file_key(title));
  else
    sprintf(&fname[i], "/polls/%s/%s/%s", lclist,
	    pet_file_key(title), name);
  return fname;
}
/**********************************************************
 *   Sends the text file of petition messages.
 **********************************************************/
void
process_names(void)
{
  char list_start[9];
  strncpy(list_start, list, 8);
  list_start[8] = '\0';
  if (!same(list_start,"petition"))
    {
      sprintf(error_msg,"\nThe %s@%s list does not handle petitions.\n",
	      list, whereami);
    }
  if (p_item_copy->eVote.type != TALLIEDY
      && p_item_copy->eVote.type != TIMESTAMP)
    {
      sprintf(error_msg,"\nThe poll attached to \"%s\" is not a petition.\n",
	      subject);
      bounce_error(SENDER);
    }
  if ((p_item_copy->eVote.priv_type == PRIVATE 
       || p_item_copy->eVote.priv_type == IF_VOTED)
      && p_item_copy->eVote.author != mail_voter)
    {
      sprintf(error_msg,"\nThe petition signatures for %s are only \naccessible by %s.\n",
	      subject, who_is(p_item_copy->eVote.author));
      bounce_error(SENDER);
    }
  petition = YES;
  gen_header(SENDER, "Re:", YES);
  translate(the_language, START_PETITION_DISPLAYS, p_item_copy, 
	    NO, NO, NO, NULL, NULL, MAYBE);
  translate(the_language, DISPLAY_PETITION_TEXT, p_item_copy,
	    NO, NO, NO, NULL, NULL, MAYBE);
  display_signatures(p_item_copy);
  big_finish(0);
}
/*********************************************************
 *     Creates a file for storage of the petition's text.
 **********************************************************/ 
void
parse_and_store_translations(ITEM_INFO * p_item, 
			     YESorNO just_checking, 
			     YESorNO confirm_petition)
{
  FILE * fp;
  char *fname;
  char subject[TITLEN + 1];
  char trans[TITLEN + 1];
  char default_subject[TITLEN + 1];
  char working_flag[4] = "-df";
  char found_list[MAX_LINE + 1];
  YESorNO found_message = NO;
  LANGUAGE new_language;
  LANGUAGE working_language = DEFAULT_LANGUAGE;
  int cc, old_cc;
  int tries = 0;
  YESorNO drop_last_token = YES; /* for call to write_from_mark() */
  strncpy(&working_flag[1], tongue[working_language].name, 2);
  if (forced_language)
    {
      working_language = the_language;
      strncpy(&working_flag[1], tongue[working_language].name, 2);
    }
  strcpy(subject, p_item->eVote.title);
  if (table_exists(subject, found_list, default_subject, working_flag))
    {
      sprintf(error_msg,"\n\"%s\" already exists as the %s translation of the\n"
	      "\"%s\" petition on the %s list.\n",
	      subject, whole_name(working_flag), 
	      default_subject, found_list);
      forget_petition(SENDER, p_item, just_checking);
    }
  table_add(list, subject, subject, working_flag,
	    p_item->eVote.priv_type, confirm_petition);
  fname = pet_fname("text/", p_item);
  strcat(fname, tongue[working_language].name);
  while ((fp = fopen(fname, "w")) == NULL)
    {
      if (++tries < 2)
	{
	  fname = pet_fname("text", p_item);
	  make_path(fname);
	  fname = pet_fname("text/", p_item);
	  strcat(fname, tongue[working_language].name);
	  continue;
	}
      sprintf(error_msg, "\neVote is unable to store your petition text in %s now.\nPlease try later", fname);
      forget_petition(SENDER | OWNER | ADMIN, p_item, just_checking);
    }
  mark_token();
  while (1)
    {
      old_cc = cc;
      cc = get_token();
      if (old_cc  == '\n' && (cc  == '\n' || cc == EOF)
	  && same(token, "end"))
	{
	  break;
	}
      if (token[0] == '\0')
	{
	  drop_last_token = NO;
	  break;
	}
      new_language = the_language;
      if (token[0] == '-' && strlen(token) == 3 
	  && is_flag(token, &new_language))
	{   /* new language starting */
	  char * t;
	  if ((t = get_trans(subject, new_language, NO)) != NULL)
	    {
	      sprintf(error_msg, 
		      "\nYou already have a %s translation, \"%s\".\n", 
		      tongue[new_language].name, t);
	      forget_petition(SENDER, p_item, just_checking);
	    }
	  if (!found_message)
	    {
	      break;
	    }
	  write_from_mark(fp, YES);  /* drop_last_token == YES */
	  fclose(fp);
	  chmod(fname, 00440);
	  working_language = new_language;
	  strncpy(&working_flag[1], tongue[working_language].name, 2);
	  fname = pet_fname("text/", p_item);
	  strcat(fname, tongue[working_language].name);
	  if ((fp = fopen(fname, "w")) == NULL)
	    {
	      sprintf(error_msg, "\neVote is unable to store your %s petition text in %s now.\nPlease try later.", tongue[working_language].name, fname);
	      forget_petition(SENDER | OWNER | ADMIN, p_item, just_checking);
	    }
	  found_message = NO;
	  /* collect the translation */
	  trans[0] = '\0';
	  while (cc != '\n' && cc != EOF && cc != '\r')
	    {
	      cc = get_token();
	      strcat(trans, token);
	      if (cc != EOF && cc != '\n')
		sprintf(trans, "%s%c", trans, cc);
	    }
	  if (trans[0] == '\0')
	    {
	      sprintf(error_msg,
		      "\nYou didn't provide a %s translation of \"%s\".\n",
		      tongue[working_language].name, p_item->eVote.title);
	      forget_petition(SENDER, p_item, just_checking);
	    }
	  /* store the translation in the table */
	  if (!same(trans, p_item->eVote.title)  
	      && table_exists(trans, found_list, 
			      default_subject, working_flag))
	    {
	      sprintf(error_msg,
		      "\n\"%s\" already exists as a petition title "
		      "\nfor \"%s\" in %s on the %s list.\n", 
		      trans, default_subject, whole_name(working_flag), 
		      found_list);
	      forget_petition(SENDER, p_item, just_checking);
	    }
	  table_add(list, subject, trans, working_flag,
		    p_item->eVote.priv_type, confirm_petition);
	  mark_token();
	  continue;
	}
      found_message = YES;
    }
  if (!found_message)
    {
      sprintf(error_msg,"\nYou didn't give a translation for the poll text in \"%s\".\n", tongue[working_language].name);
      forget_petition(SENDER, p_item, just_checking);
    }
  write_from_mark(fp, drop_last_token);
  fclose(fp);
  chmod(fname, 00440);
  write_translations();
  return;
}
/********************************************************
 *   void process_unsign(void)  -- called for multiple unsigns
 *     from the owner.  Only called from listm.c
 *********************************************************/
void
process_unsign(void)
{
  int cc;
  unsigned long me = mail_voter;
  unsigned long  readers, voters;
  char vote_str[10] = "  - ";
  char result[10] = "  - ";
  int reports;
  if (p_item_copy->eVote.vstatus == CLOSED)
    {
      sprintf(error_msg, "The poll is closed on %s.  You can't remove %s.\n",
	      original_subject, (no_pet_votes ? "votes" : "signatures"));
      bounce_error(SENDER);
    }
  gen_header(SENDER, "Re:", YES);
  i_am_leaving();
  do
    {
      if ((mail_voter = who_num(token, NO)) == 0)
	{
	  printf("\n%s is not at %s.\n", token, whereami);
	  continue;
	}
      switch (enter_Clerk())
	{
	case OK:
	  break;
	case NOT_OK:
	  printf("\nCan't communicate with the Clerk.\n");
	  continue;
	  break;
	case UNDECIDED:
	  printf("\n%s has not participated in %s.\n",
		 token, lclist);
	  continue;
	  break;
	default:
	  /* impossible */
	  break;
	}
      translate(default_language, FINISH_UNSIGN, p_item_copy, MAYBE,
		NO, NO, token, &cc, YES);
      if (cc != OK)
	{
	  printf("%s", error_msg);
	}
    }
  while ((cc = get_token()) && token[0] != '\0');
  if ((reports = read_report_instructions(p_item_copy)))
    {
      time_str[0] = '\0';
      big_finish(0);
    }
  mail_voter = me;
  if (enter_Clerk() != OK)
    {
      printf("\nTerrible trouble, can't reenter to update report after unsign\n on %s.\n", original_subject);
      big_finish(0);
    }
  get_mail_stats(p_item_copy->dropping_id, &readers, vote_str, result);
  voters = atoul(vote_str);
  ship_reports(voters);
  big_finish(0);
}
/************************************************************
 *     Removes the directory -- recursively
 *************************************************************/
OKorNOT
remove_pet_dir(ITEM_INFO *p_item, int whom)
{
  char *fname;
  char command[200];
  int cc;
  struct stat ss;
  fname = pet_fname("", p_item);
  if (stat(fname, &ss) != 0)
    return OK;
  sprintf(command, "chmod -R u+w %s", fname= pet_fname("", p_item));
  if ((cc = system(command)) == 0) 
    {
      sprintf(command, "rm -r %s", fname);
      cc = system(command);
    }
  if (cc)
    {
      sprintf(&error_msg[strlen(error_msg)], 
	      "\nCan't drop petition directory for %s, %s", 
	      subject, fname); 
      perror(error_msg);
      if (whom == 0)
	whom = ADMIN | SENDER | OWNER;
      bounce_error(whom);
    }	
  return OK;
}
/********************************************************
 *     Displays information about the poll.
 *   This function cannot rely on the global p_item_copy
 *   because we may be displaying info about a poll
 *   request that is "check" -- not really in the 
 *   data base;
 *********************************************************/
void
send_petition_info(int whom, ITEM_INFO *p_item, YESorNO new_pet,
		   YESorNO from_insert, YESorNO checking)
{
  int i;
  YESorNO isauthor = NO;
  YESorNO did_author_line = NO;
  if (p_item == NULL)
    {
      p_item = p_item_copy;
    }
  if (p_item != NULL  &&
      mail_voter == p_item->eVote.author)
    isauthor = YES;
  if (new_pet || from_insert)
    gen_header(whom, "Re:", YES);
  else
    gen_header(whom, "Re:", NO);
  if (just_checking)
    check_message();
  translate(the_language, DISPLAY_PETITION_INFO, 
	    p_item, just_checking, new_pet, NO, NULL, NULL, MAYBE);
  if (!p_item)
    {
      leave();
      translate(the_language, BIG_FINISH,
		p_item, just_checking, 
		new_pet, MAYBE, NULL, NULL, MAYBE);
    }
  if (new_pet)
    {
      for (i = 0; i < no_languages; i++)
	{
	  if ((LANGUAGE)i == the_language)
	    continue;
	  if (get_trans(subject, (LANGUAGE)i, NO) == NULL 
	      && (field == NULL || field[0].name[i][0] == '\0'))
	    continue;
	  translate((LANGUAGE)i, DISPLAY_PETITION_INFO, 
		    p_item, just_checking, 
		    new_pet, YES, NULL, NULL, MAYBE);
	}
    }			
  if (p_item->eVote.priv_type != PRIVATE && from_insert)
    {
      printf("\n");
      highlight("RETRIEVING THE SIGNATURES");
      printf("\nYou can retrieve the signatures for this petition:\n");
    }
  else if (isauthor == YES && new_pet == NO)
    {
      address("TO", from);
      printf("\n\nBecause you are the author of this private petition, you, and"
	     "\nonly you can retrieve the signatures for this petition:\n");
      did_author_line = YES;
    }
  if ((isauthor == YES && new_pet == NO) 
      || (p_item->eVote.priv_type != PRIVATE && from_insert))
    {
      printf("\n1.  Send a message to:");
      printf("\n\n\t%s@%s", list, whereami);
      printf("\n\n2.  Your subject must be:");
      printf("\n\n\t%s", subject);
      printf("\n\n3.  Your message should say:");
      printf("\n\n\teVote names\n");
    }
  if (new_pet == YES && did_author_line == NO)
    {
      address("TO", from);
    }
  if (isauthor)
    {
      printf("\n\nBecause you are the author of this petition, you, and only you, \ncan send the message:");
      printf("\n\n\teVote close");
      printf("\n\nSending this message will close the petition to new signatures.");
    }
  printf("\n");
  if (checking)
    forget_petition(0, p_item, YES); /* drops the translations */
  if (!from_insert)
    leave();
  translate(the_language, BIG_FINISH,
	    p_item, just_checking, 
	    new_pet, MAYBE, NULL, NULL, MAYBE);
}
/************************************************************
 *   Called from in_message.c:fix_subject:strip_subject 
 *   Determines if this is a confirmation message, if a 
 *   confirmation message is need, and which list and language.
 *   Coming in status is either CHECK for Confirm: found in subject
 *                        or STARTING if not.
 *   returns NOT_OK if it find_and_switch couldn't find
 *   the translations file.  On return:
 *    CHECK -- key in confirm_this.key
 *    STARTING -- needed, but this is not it.
 *    NOT_NEEDED
 ************************************************************/
OKorNOT
sort_out_petition(int argc, char *argv[])
{
  YESorNO needs_confirm;
  OKorNOT cc = OK;
  if ((cc = find_and_switch (argc, argv, &needs_confirm)) != OK)
    return cc;  /* table.cc couldn't open translation file */
  if (!needs_confirm)
    {
      /*      sprintf(error_msg,""); in case FAILED */
      confirm_this.status = NOT_NEEDED; /* in case subject had Confirm: */
    }
  return OK;
}
/************************************************************
 *     Syncronizes the Clerk's data to match the data in
 *     the signature file for each of the petitions in
 *     lclist.
 *************************************************************/
OKorNOT
sync_petitions(void)
{
  OKorNOT cc = OK, ret_cc = OK;
  int i;
  if (send_enter_admin(lclist) != OK)
    return UNDECIDED;
  for (i= 1; i <= *p_no_items; i++)
    {
      if (item_info[i].eVote.vstatus == CLOSED
	  || (item_info[i].eVote.type != TALLIEDY
	      && item_info[i].eVote.type != TIMESTAMP))
	{
	  continue;
	}
      if ((cc = sync_sigs(&item_info[i])) != OK)
	{
	  ret_cc = cc;
	}
    }
  leave_admin();
  return ret_cc;
}
/**********************************************************
 *      Tells them what they missed.
 *      Only exported to pet_out_xx.c files.
 **********************************************************/
void
too_late(void)
{
  gen_header(SENDER, "Re:", YES);
  if (error_msg[0] != '\0')
    {
      printf("%s", error_msg);
    }
  translate(the_language, DISPLAY_PETITION_RESULTS, p_item_copy, MAYBE,
	    MAYBE, MAYBE, NULL, NULL, MAYBE);
  leave();
  big_finish(0);
}
/*************************************************************
 *      Makes the call into the translation table.
 ************************************************************/
void
translate(LANGUAGE which_language, WHICH_FUNCTION function, 
	  ITEM_INFO * p_item, YESorNO just_checking, 
	  YESorNO new_pet, YESorNO do_header, char * name,
	  int *pcc, YESorNO from_owner)
{
  tongue[which_language].translate_fn(which_language, function, p_item, 
				      just_checking, new_pet, do_header,
				      name, pcc, from_owner);
}

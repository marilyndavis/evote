/* $Id: pet_out.c,v 1.7 2003/10/20 17:07:34 marilyndavis Exp $ */ 
/**********************************************************
 *  ../eVote/src/mail/mailui/pet_out.c
 *         has code that needs translating
 *********************************************************
 *    Copyright (c) 1994...2015 Deliberate.com Patented.
 *    by Marilyn Davis
 ***********************************************************
 *     pet_out.c  default petition output functions.
 *       To provide a Spanish translation, for example,
 *       1.  In mailui.h, be sure the new language is one of the
 *           LANGUAGEs listed in the enumerated type.  Add it.
 *       2.  If the number of languages is greater than
 *           MAX_LANGUAGES, raise MAX_LANGUAGES.
 *       3.  Add a new function declaration in mailui.h:
 *           void translate_es(WHICH_FUNCTION ...
 *           Make the arguments just like translate_en(...) and
 *           declare it just after translate_en(...).
 *       4.  In table.cc, add an entry for your language to the
 *           table of tongues near the top of the file.  Be sure
 *           to keep your table of tongues in the same order as
 *           your list of LANGUAGEs in mailui.h 
 *           If your language is already there, check the trans-
 *           lations and change the translate_xx entry to be
 *           translate_es if ESPANOL is your language.      
 *       5.  Translate this file to Spanish.
 *             1.   trans en es pet_out.c
 *             2.   Edit the en-es.new file that results so that
 *                  all the phrases are translated.  Rename the
 *                  file en_es.
 *             3.   Rerun trans en es pet_out.c.  Check that
 *                  the new en_es.new file is empty.
 *       6.  Change the LOCAL_LANGUAGE below to be your language,
 *           all in caps -- in the translated version of this file.
 *       7.  Change the names of the function in this file:
 *           translate_en   --> translate_es  (also translated version)
 *       8.  Fix up the makefile in this directory to include 
 *           pet_out_es.o in the OBJS.  Note that there is a tab
 *           at the beginning of the lines in the list of OBJS,
 *           and that each line (except the last) ends with a '\'
 *       9.  Fix up the makefile in the .. directory 
 *           (EVOTE_HOME_DIR/src/mail) to include mailui/pet_out_es.o
 *           in the list at the end.
 *      10.  make the new executables.
 ************************************************************/
#include<stdio.h>
#include <sys/stat.h>
#include "mailui.h"
static void do_petition(void);
static void start_petition_displays(ITEM_INFO * p_item, YESorNO new_pet,
				    LANGUAGE which);
static void store_signature(ITEM_INFO * p_item);
static void display_petition_info(LANGUAGE which_language, 
				  ITEM_INFO * p_item, 
				  YESorNO just_checking, 
				  YESorNO new_pet, YESorNO do_header);
static void display_petition_results(ITEM_INFO *p_item);
static void display_petition_text(ITEM_INFO * p_item, YESorNO just_checking,
				  LANGUAGE which);
static void finish_unsign(char * name, int * pcc, YESorNO from_owner);
static void big_petition_finish(int exit_code);
static void no_pet_message(void);
static void vote_the_petition(void);
extern YESorNO give_email;
extern YESorNO into_eVote; /* petition.c but worked here */
/*************************************************************
 *  Change this LOCAL_LANGUAGE from ENGLISH to your LANGUAGE *
 *                       \|/                               ***/
#define LOCAL_LANGUAGE ENGLISH
/*************************************************************
 *              |     After translating this file, change the 
 *              |     name of this function to be translate_xx 
 *              |     where xx is the flag for your language.
 ************* \|/ ******************************************/
void
translate_en(LANGUAGE which_language, WHICH_FUNCTION function, 
		  ITEM_INFO * p_item, YESorNO just_checking, 
		  YESorNO new_pet, YESorNO do_header, char *name,
		  int * pcc, YESorNO from_owner)
{
  switch (function)
    {
    case DO_PETITION:
      do_petition();
      break;
    case FINISH_UNSIGN:
      finish_unsign(name, pcc, from_owner);
      break;
    case DISPLAY_PETITION_RESULTS:
      display_petition_results(p_item);
      break;
    case DISPLAY_PETITION_INFO:
      display_petition_info(which_language, p_item, just_checking, 
			    new_pet, do_header);
      break;
    case DISPLAY_PETITION_TEXT:
      display_petition_text(p_item, just_checking, which_language);
      break;
    case START_PETITION_DISPLAYS:
      start_petition_displays(p_item, new_pet, which_language);
      break;
    case BIG_FINISH:
      big_petition_finish(0);
      break;
    default:
      /* impossible */
      break;
    }
}
extern YESorNO new_signer;
static void bounce_info_error(void);
static OKorNOT check(RTYPE rtype);
static int check_form(int cc);
static OKorNOT check_sig_confirm(void);
static void display_howto(LANGUAGE);
static void display_priv_type(ITEM_INFO *);
static void display_remove(ITEM_INFO *, LANGUAGE);
static int explain_form(LANGUAGE);
static void list_petitions(void);
extern void leave();  /* in petition.c */
static int parse_vote(int * cc, char * *, int vote_item);
static void remove_signer(int reports);
static void report_vote(void);
extern void too_late();  /* in petition.c */
extern char (*answer)[FIELD_LEN + 1];  /* in form.c */
extern int no_petitions;
extern int no_publics;
extern int no_privates;
static int * pet_vote;
/************************************************************/
void
big_petition_finish(int exit_code)
{
  printf("\n  ================    START OF MESSAGE RECEIVED ===================\n\n");
  dump_message(stdout, NO, NO, NO);
  printf("\n\n  =====================    END OF MESSAGE   =======================\n");
  finish(exit_code);
}
/*******************************************************/
static void
bounce_petition_error(int whom)
{
  leave();
  /* This call to send splits the process and comes
     back the parent with the child's stdin sucking
     from this stdout */
  /* generate a message header on stdout */
  gen_header(whom, "Error:", YES);
  printf("\nIn response to your message which starts:\n");
  /*  print_tokens(YES); */
  print_first_line();
  printf("\n----\n");
  printf("%s", error_msg);
  if (list != NULL && list[0] != '\0')
    {
      if ((whom & OWNER && whom != OWNER) 
	 || (whom & APPROVAL && whom != APPROVAL))
	{
	  printf("\nThis message is also being sent to owner-%s", list);
	  if (whereami != NULL && whereami[0] != '\0')
	    printf("@%s\n", whereami);
	}
    }
  if (whom & ADMIN)
    {
      printf("\nThis message is also being sent to the listserver administrator.");
      if (whereami != NULL && whereami[0] != '\0')
	printf("@%s\n", whereami);
    }
#ifdef EVOTE_ERRORS
  if (whom & DEVELOPER)
    printf("\nThis message is also being sent to %s.\n", 
	   EVOTE_ERRORS);
#endif
  big_finish(0);
}
/************************************************************
 *    Checks for a confirm file from this voter with the
 *    right key.  confirm_this.key has the key from the
 *    subject line.  confirm_this.status = CHECK when
 *    called.  Returns VERIFIED or sends an error message.
 *    If VERIFIED, it has also switched stdin to be the
 *    confirmed message.
 ************************************************************/
OKorNOT
check_sig_confirm(void)
{
  FILE *fp;
  char buf[500];
  char * file_key;
  YESorNO cc;
  CONFIRM_WHAT what = FOR_SIG;
  switch (cc = read_confirm(&what, &file_key, &fp))
    {
    case YES:
      return OK;
      break;
    case NO:
    case MAYBE:
      sprintf(error_msg, "\nYour message about:\n\n\t%s\n\n"
	      "looks like a confirmation reply for \"%s\" \n"
	      "with the confirmation key \"%s\".  However, we have\n"
	      "no record of this confirmation key for you for\n"
	      "\"%s\".\n", original_subject, subject, confirm_this.key, 
	      subject);
      if (cc == NO)
	{
	  bounce_info_error();
	  break;
	}
      /* case MAYBE: found a file_key but wrong one */
      sprintf(buf, "Confirm: %s ", file_key);
      gen_header(SENDER, buf, NO);
      printf(error_msg);
      printf("\nHowever, you do have a confirmation key \"%s\" pending\n"
	     "on \"%s\".\n\n"
	     "Note that eVote only keeps your most recent attempt.  The following\n"
	     "is the message that awaits confirmation.  To confirm it, reply to \n"
	     "this message.\n"
	     "\nIf you wish to send in a new message for %s,"
	     "\ntry again, but don't put \"Confirm: %s\" in your subject.\n",
	     file_key, subject, subject, confirm_this.key);
      printf("\n\n ----- YOUR MESSAGE AWAITING CONFIRMATION -----\n\n");
      while (fgets(buf, 400, fp))
	fputs(buf, stdout);
      fclose(fp);
      big_petition_finish(0);
      break;
    default:
      return NOT_OK;  /* never happens */
    }
  /* Never happens */
  return NOT_OK;
}
/************************************************************
 *  Returns the last cc of get_token or -1 on error
 *************************************************************/
static int
collect_votes(void)
{
  int no = 0;
  int end;
  int i, cc;
  YESorNO some_vote = NO;
  YESorNO no_good = NO;
  char *answer;
  char local_token[100];
  if ((pet_vote = malloc(sizeof(int) * no_pet_votes))
     == NULL)
    {
      sprintf(error_msg,"\n%s is out of resources right now.  Please try your message later.\n", whereami);
      return -1;
    }
  for (no = 0; no < no_pet_votes; no++)
    pet_vote[no] = UNKNOWN;
  while (1)
    {
      no_good = NO;
      end = strlen(token)-1;
      if (cc == EOF || end == 0 
	 || (token[end] != ':' && token[end] != '.' && token[end] != '-'
	     && token[end] != ','))
	{
	  no_good = YES;
	}
      strcpy(local_token, token);
      local_token[end] = '\0';
      if (no_good || (no = atoi(local_token)) == 0 )
	{
	  if (!some_vote)
	    {							
	      sprintf(error_msg,"\neVote expects a vote response like \"1. Yes\" at this point.\n");
	      return -1;
	    }
	  for (i = 0; i < no_pet_votes; i++)
	    {
	      if (pet_vote[i] == UNKNOWN)
		pet_vote[i] = 0;
	    }
	  return cc;
	}
      if (no > no_pet_votes)
	{
	  sprintf(error_msg,"There are only %d questions in this poll.\n",
		  no_pet_votes);
	  return -1;
	}
      if (pet_vote[no-1] != UNKNOWN)
	{
	  sprintf(error_msg,"\nYou can only vote once on question %d.\n",
		  no);
	  return -1;
	}
      if ((pet_vote[no-1] = parse_vote(&cc, &answer, no-1)) == UNKNOWN)
	{ /* parse_vote puts an error_msg in */
	  if ((p_first_vote_item+no)->eVote.min == -1
	     && (p_first_vote_item+no)->eVote.max == 1)
	    {
	      sprintf(error_msg, "\neVote doesn't recognize your response for question %d."
		      "\nYou responded \"%s\".  You must answer"
		      "\n\"Yes\", \"No\", or \"Don't Know.\"\n", 
		      no, answer);
	    }
	  else
	    {
	      sprintf(error_msg, "\neVote doesn't recognize your response for question %d."
		      "\nYou responded \"%s\".  You must answer "
		      "a number between %d and %d.\n",
		      no, answer,
		      (p_first_vote_item+no)->eVote.min,
		      (p_first_vote_item+no)->eVote.max);
	    }
	  return -1;
	}
      some_vote = YES;
      cc = get_token();
    }
  return cc;
}
/********************************************************
 *     Displays information about how to sign.
 *********************************************************/
void
display_howto(LANGUAGE which)
{
#ifdef ROSA
  if (same(subject,"Mexico March"))
    {
      int mexico_form(LANGUAGE);
      printf("\n");
      highlight("TO REGISTER");
      printf("\n1.  Send a message to:");
      printf("\n\n\tmexico_registration@%s", whereami);
      printf("\n");
      printf("\n2.  Your subject must be:");
      printf("\n\n\tMexico March");
      printf("\n");
      mexico_form(which);
      return;
    }
#endif
  printf("\n");
  highlight((no_pet_votes ? "TO VOTE": "TO SIGN THIS PETITION"));
  printf("\n1.  Send a message to:");
  printf("\n\n\teVote@%s", whereami);
  printf("\n");
  printf("\n2.  Your subject must be:");
  printf("\n\n\t%s", get_trans(subject, which, YES));
  printf("\n");
  if (!form_exists && !no_pet_votes)
    {
      printf("\n3.  If your message is blank, only your email address will be recorded."
	     "\n"
	     "\n    or"
	     "\n"
	     "\n    Type your name, affiliation and location.\n");
    }
  else  /* also explains votes */
    {
#ifdef ROSA
      if (same("Mexico March", subject))
      {
	int mexico_form(LANGUAGE);
	mexico_form(which);
	return;
      }
#endif
      explain_form(which);
    }
  printf("\n4.  If your message has a signature file, or any other text that you"
	 "\n    don't want included, make a line that just says \"end\".\n");
}
/************************************************************/
void
display_petition_info(LANGUAGE which_language,
			   ITEM_INFO * p_item, YESorNO just_checking, 
                           YESorNO new_pet, YESorNO do_header)
{
  int reports;
  int fields;   /* also in no_of_fields, external */
  if (p_item == NULL)
    {
      no_pet_message();
      return;
    }
  if (do_header)
    {
      char head[200];
      char uppers[100];
      printf("\n");
      sprintf(head, "INSTRUCTIONS FOR %s", 
	      raiseup(uppers, tongue[which_language].name));
      highlight(head);
    }
  start_petition_displays(p_item, new_pet, which_language);
  display_petition_text(p_item, just_checking, which_language);
  reports  = read_report_instructions(p_item);
  fields = read_form_template(p_item);
  display_howto(which_language);
#ifdef ROSA
  if (!same("Mexico March", subject))
#endif    
    display_priv_type(p_item);
  display_remove(p_item, which_language);
  /*	if (just_checking || do_header || new_pet) */
  return;
  /*	big_petition_finish(0); */
}
/*******************************************************/
void
display_petition_results(ITEM_INFO *p_item)
{
  unsigned long voters;
  voters = get_mail_voters(p_item->dropping_id);
  printf("\n");
  highlight("RESULTS");
  if (no_pet_votes)
    {
      printf("\n%lu voter%s participated in this poll.\n",
	     voters, (voters == 1L? "" : "s"));
    }
  else
    printf("\nThis petition collected %lu signature%s.\n",
	   voters, (voters == 1L? "" : "s"));
}
/*********************************************************
 *    Prints the petition text on stdout.
 **********************************************************/ 
void
display_petition_text(ITEM_INFO * p_item, YESorNO just_checking,
			   LANGUAGE which)
{
  FILE *fp;
  char fname[PATHLEN + 1];
  int ch;
  char * default_name;
  int tries = 0;
  /*  printf("\n"); */
#ifdef ROSA
  if (same(subject,"Mexico March"))
    {
      highlight("MARCH WITH XOKONOSCHTLETL");
    }
  else
#endif
  highlight((no_pet_votes ? "POLL TEXT" : "PETITION TEXT"));
  default_name = pet_fname("text", p_item);
  sprintf(fname, "%s/%s", default_name, tongue[which].name);
  while ((fp = fopen(fname, "r")) == NULL)
    {
      sprintf(fname, "%s/%s", default_name, tongue[DEFAULT_LANGUAGE].name);
      if (++tries < 2)
	continue;
      printf("\n%s", file_error);
      printf("\n\t%s \n\n", fname);
      printf("to read this petition's text.\n\nPlease forward this to %s.\n\n",
	     eVote_mail_to);
      fprintf(stderr,"%s\n\t%s \n\nto read this petition's text.\n\n", 
	      file_error, fname);
      perror("");
      return;
    }
  if ((ch = fgetc(fp)) == EOF)
    {
      char er[] = "\n\nERROR! Empty file: \n\n";
      char er2[] = "\n\nCan't read this poll/petition's text.\n\n";
      printf("%s%s%sPlease forward this to %s.\n\n", 
	     er, fname, er2, eVote_mail_to);
      fprintf(stderr,"%s%s%s", er, fname, er2);
      fclose(fp);
      return;
    }
  if (ch != '\n')
    putchar('\n');
  putchar(ch);
  while ((ch = fgetc(fp)) != EOF)
    {
      putchar(ch);
    }
  fclose(fp);
}
/*******************************************************
 *    Displays messages about the privacy type.
 *******************************************************/
void
display_priv_type(ITEM_INFO *p_item)
{
  switch (p_item->eVote.priv_type)
    {
    case PRIVATE:
    case IF_VOTED:
#ifdef ROSA
      if (same(subject,"Mexico March"))
	return;
#endif
      printf("\n");
      highlight("PRIVATE PETITION");
      printf("\nThe signatures for this petition, and the email addresses, can only be"
	     "\nretrieved by %s and by the system administrator of"
	     "\nthis site. They alone are responsible for how the information is used.\n",
	     author_name);
      break;
    default:
      printf("\n");
      highlight((no_pet_votes ? "PUBLIC POLL" : "PUBLIC PETITION"));
#ifdef ROSA
      if (same(subject,"Kopilli Ketzalli"))
	{
	  printf("\nThis is a *public petition*.  We will publish your name, country, and"
		 "\ncomments (but not your email address) on the web page.  In the"
		 "\nfuture, your friends will be able to query this system (if they know"
		 "\nyour email address) to see your petition message."
		 "\n"
		 "\nWe reserve the right to remove any contributions we consider in-"
		 "\nappropriate.  You can remove your own contribution at any time."
		 "\nHowever, if your signature remains for one week you can remove it"
		 "\nfrom the data in cyberspace but it may already have been counted"
		 "\nin Austria."
		 "\n"
		 "\nWe use your email address to ensure that there is only one signature"
		 "\nfor each email address and for no other purpose.\n");
	}
      else
#endif
	{
	  printf("\nThis is a *public* petition.  The information you contribute"
		 "\ncan be retrieved by anyone who signs the petition and by anyone"
		 "\non the %s@%s email list.\n", list, whereami);
	  printf("\nThis means that the information is quite public and that there"
		 "\nare no controls on its use.\n");
	  printf("\nYour email address, however, can only be seen by the site"
		 "\nadministrator at %s.\n"
		 "\nYour email address is used to ensure that there is only one"
		 "\nsignature for each email address and for no other purpose.\n", whereami);
	}
    }
}
/*******************************************************/
void
display_remove(ITEM_INFO *p_item, LANGUAGE lang)
{
  /*  printf("\n"); */
#ifdef ROSA
  if (same(subject,"Mexico March"))
    {
      highlight("CANCELLING YOUR RESERVATION");
      printf("\nIf you wish to cancel your reservation:");
      printf("\n\n1.  Send a message to:");
      printf("\n\n\tmexico_registration@%s", whereami);
      printf("\n\n2.  Your subject must be:");
      printf("\n\n\t%s", "Mexico March");
      printf("\n\n3.  Your message should say:");
      printf("\n\n\tremove\n");
      printf("\nYour deposit will be returned, minus a $25 processing fee.\n");
      printf("\nIf you cancel your reservation after October 1, 2001, your deposit\n"
	     "will be returned minus a $100 inconvenience fee.\n");
      printf("\nIf you need to reach a person, write to:"
	     "\n\n\tmexico-info@deliberate.com\n");
      return;
    }
#endif
  highlight((no_pet_votes ? "REMOVING YOUR VOTE" : "REMOVING YOUR SIGNATURE"));
  printf("\nIf you feel that you %s in error, please remove your %s:",
	 (no_pet_votes ? "voted" : "signed"),
	 (no_pet_votes ? "vote" : "signature"));
  printf("\n\n1.  Send a message to:");
  printf("\n\n\teVote@%s", whereami);
  printf("\n\n2.  Your subject must be:");
  printf("\n\n\t%s", get_trans(subject, lang, YES));
  printf("\n\n3.  Your message should say:");
  printf("\n\n\tremove\n");
#ifdef ROSA
  if (same(subject,"Kopilli Ketzalli"))
    {
      printf("\nIf you wish to remove your signature, please do so promptly.  After "
	     "\none week, your signature may be forwarded to Vienna to be counted.\n");
      printf("\n");
      highlight("NEWSLETTER");
      printf("\nTo receive the email Newsletter for the Yanakuikanahuak please:\n");
      printf("\n1.  Send a message to:");
      printf("\n\n\tmajordomo@deliberate.com");
      printf("\n\n2.  The subject line doesn't matter.");
      printf("\n\n3.  Your message should say:");
      printf("\n\n\tsubscribe");
      printf(" anahuak-en");
      printf("\n\tend");
      printf("\n\nThere are also newsletters in Spanish, \"anahuak-es\", "
	     "German, \"anahuak-de\","
	     "\nand French, \"anahuak-fr\" which you are welcome to subscribe to as well.\n");
      printf("\n");
      highlight("GATHERING");
      printf("\nTo join the email gathering for discussing issues surrounding \nKopilli Ketzalli and its return to Mexico:\n");
      printf("\n1.  Send a message to:");
      printf("\n\n\tmajordomo@deliberate.com");
      printf("\n\n2.  The subject line doesn't matter.");
      printf("\n\n3.  Your message should say:");
      printf("\n\n\tsubscribe");
      printf(" kopilli-en");
      printf("\n\tend");
      printf("\n\nThere are also gatherings in Spanish, \"kopilli-es\", "
	     "German, \"kopilli-de\","
	     "\nand French, \"kopilli-fr\" which you are welcome to subscribe to as well.\n");
      printf("\n");
      highlight("BILINGUAL?");
      printf("\nIf you write in German, Spanish or French as well as English and"
	     "\nif you would like to help with translating for the Yankuikanahuak:");
      printf("\n\n1.  Send a message to:");
      printf("\n\n\tmajordomo@%s", whereami);
      printf("\n\n2.  The subject line doesn't matter.");
      printf("\n\n3.  Your message should say:");
      printf("\n\n\tsubscribe");
      printf(" es");
      printf("\n\tend");
      printf("\n\nto meet with Spanish language translators.  There are also"
	     "\ntranslator meetings for German, \"de\", and French, \"fr\" "
	     "\nwhich you are welcome to join as well.\n");
      printf("\nYou can send many commands to majordomo@deliberate.com in one message:");
      printf("\n\n\tsubscribe");
      printf(" anahuak-en");
      printf("\n\tsubscribe");
      printf(" kopilli-en");
      printf("\n\tsubscribe");
      printf(" anahuak-es");
      printf("\n\tsubscribe");
      printf(" kopilli-es");
      printf("\n\tsubscribe");
      printf(" es");
      printf("\n\tend");
      printf("\n\nwill subscribe you to five lists.\n");
    }
#endif
}
/*******************************************************
 *     Processes a message when it came into the system
 *     through the petition facility. When called:
 *     confirm_this.status = 
 *                           VERIFIED - this message confirmed
 *                           NOT_NEEDED - no confirm necessary
 *                           STARTING - this needs error or
 *                                      to be confirmed.
 *******************************************************/
void
do_petition(void)
{
  int cc;
  unsigned long  readers, voters;
  char vote_str[10] = "  - ";
  char result[10] = "  - ";
  short old_vote;
  int reports = 0;
  int fields;
  int i;
  void register_mexico(void);
  if (confirm_this.status != CHECK)
    {
      cc = fix_first_token();
      if (same(token,"eVote"))
	{
	  sprintf(error_msg,"\nTo use eVote commands you must be a member of the \n\n\t%s@%s \n\nlist and the eVote commands must be sent directly to the list\naddress.\n",
	      list, whereami);
	  bounce_error(SENDER);
	}
      for (i = 0; i < no_languages; i++)
	{
	  if (same(token, tongue[(LANGUAGE)i].remove))
	    {
	      remove_sig = YES;
	      break;
	    }
	}
    }
#ifdef ROSA
  if (same(subject, "Mexico"))
     subject = "Mexico March";
  if (same(subject, "Mexico March"))
    register_mexico();
#endif
  if (!into_eVote && set_up_eVote() != OK)  /* Identifies the list, and user */
    /* if we get this far, communication to the list is working
       the list exists and, if mail_voter != 0, we're into
       the database.  */
    {
      if (remove_sig)
	{
	  sprintf(error_msg, "\nYou have not participated in eVoting at %s.\n",
		  whereami);
	}
      enter_signer();
    }
  if (subject == NULL || subject[0] == '\0')
    {
      sprintf(error_msg, "\nYou must specify a petition title as the subject of your message.\n");
      list_petitions();
    }
  if (current_action == READ_ONLY)
    {
      sprintf(error_msg,"\nYou can not participate in eVoting.  \nYou are a read_only member of %s.\n", list);
      bounce_petition_error(SENDER | OWNER);
    }
  if (!into_eVote)
    {
      copy_poll(YES); /* If dropping_id != 0, we have a polled
			 subject */
      into_eVote = YES;
    }
  if (confirm_this.status == CHECK)
    /* Confirm: found in subject */
    {
      check_sig_confirm();  /* places VERIFIED in status */
      /* it starts over with the waiting message */
      do_petition();
      /* or it generates an error message and quits */
    }  
  if (dropping_id == 0 || petition != YES 
     || (p_item_copy->eVote.type != TALLIEDY 
	 && p_item_copy->eVote.type != TIMESTAMP))
    {
      gen_header(SENDER, "ERROR:", NO);
      no_pet_message();
    }
  /* Check for remove message and switch to correct language */
  for (i = 0; i < no_languages; i++)
    {
      if (same(token, tongue[(LANGUAGE)i].remove))
	{
	  remove_sig = YES;
	  if (!same(tongue[the_language].remove, tongue[(LANGUAGE)i].remove))
	    {
	      the_language = (LANGUAGE)i;
	      back_one_token();
	      translate(the_language, DO_PETITION, NULL, 
			MAYBE, MAYBE, MAYBE, NULL, NULL, MAYBE);
	    }
	  break;
	}
    }  
  if (error_msg[0] != '\0')  /* remove_sig and never eVoted */
    {
      bounce_info_error();
    }
  if (same(tongue[the_language].help, token) 
     || same(tongue[the_language].info, token))
    send_petition_info(SENDER, p_item_copy, NO, NO, NO);
  /* Switch to the right language ??? */
  for (i = 0; i < no_languages; i++)
    {
      if (same(token, tongue[(LANGUAGE)i].help)
	 || same(token, tongue[(LANGUAGE)i].info))
	{
	  the_language = (LANGUAGE)i;
	  send_petition_info(SENDER, p_item_copy, NO, NO, NO);
	}
    }
  if (p_item_copy->eVote.vstatus == CLOSED)
    {
      if (remove_sig == YES)
	{
	  sprintf(error_msg,"\nYou are too late to remove your %s on:\n\n\t%s"
		  "\n\nThe %s is closed.\n", 
		  (no_pet_votes ? "votes" : "signature"),
		  original_subject,
		  (no_pet_votes ? "poll" : "petition"));
	}
      else
	{
	  sprintf(error_msg,"\nYou are too late to %s on:\n\n\t%s"
		  "\n\nThe %s is closed.\n",
		  (no_pet_votes ? "vote" : "sign the petition"),
		  original_subject,
		  (no_pet_votes ? "poll" : "petition"));
	}
      too_late();
    }
  reports  = read_report_instructions(p_item_copy);
  fields = read_form_template(p_item_copy);
  /* remove_sig is set in strip_subject() if [R] in subject
     line -- good for javascript */
  if (remove_sig == YES)
    {
      remove_signer(reports); /* collects confirm */
    }
  if (no_pet_votes && (cc = collect_votes()) == -1)
    bounce_info_error(); 
  if (form_exists)         /* check that the user filled in the form */
    {
      cc = check_form(cc);
    }
  if (no_pet_votes || form_exists)
    {
      set_sig_start(cc);
    }
  switch (send_vote(p_item_copy, 1, &old_vote))
    {
    case FAILURE:
      sprintf(error_msg, "\nSystem troubles at %s: Your %s was not recorded.\n", 
	      whereami,
	      (no_pet_votes ? "vote" : "signature"));
      bounce_petition_error(SENDER | ADMIN | OWNER);
      break;
    case NO_MODS:
      sprintf(error_msg,"\nYou are too late to %s on:\n\n\t%s"
	      "\n\nThe %s is closed.\n",
	      (no_pet_votes 
	       ? (confirm_this.status == VERIFIED ? "confirm your vote" : "vote")
	       : (confirm_this.status == VERIFIED ? "confirm your signature" :"sign the petition")),
	      original_subject,
	      (no_pet_votes ? "poll" : "petition"));
      list_petitions();   /* never returns */
      break;
    case NO_CHANGE:
      sprintf(error_msg, "\nThank you.  You already %s on "
	      "\n\n\t%s\n\nYou can't %s twice.\n", 
	      (no_pet_votes ? "voted" : "signed the petition"),
	      original_subject, (no_pet_votes ? "vote": "sign"));
      bounce_info_error();
      break;
    case GOOD:  
      get_mail_stats(p_item_copy->dropping_id,
		     &readers, vote_str, result);
      if (confirm_this.status != STARTING)  /* NOT_NEEDED or VERIFIED */
	{
	  store_signature(p_item_copy);
	  if (no_pet_votes)
	    vote_the_petition();
	  gen_header(SENDER, "eVote Rcpt:", NO);
	}
      else
	{
	  char buf[100];
	  check(send_vote(p_item_copy, READ, &old_vote));
	  get_mail_stats(p_item_copy->dropping_id,
			 &readers, vote_str, result);
	  if (collect_confirm(FOR_SIG) == NULL)  /* sets key */
	    {
	      bounce_petition_error(SENDER | OWNER | ADMIN);
	    }
	  sprintf(buf, "Confirm: %s", confirm_this.key);
	  gen_header(SENDER, buf, NO);
	  printf("\nYour %s on \"%s\" has NOT been"
		 "\nregistered yet.",
	     (no_pet_votes == 0 ? "signature" : (no_pet_votes == 1 ?
						 "vote" : "votes")),
	     original_subject);
	  printf("\n\nTo verify that your message really came from you, please confirm"
		 "\nthis message by using \"Reply-To\" and do not alter the subject"
		 "\nline.  You don't need to type anything new in your reply, it's"
		 "\njust a confirmation message.  This ensures the integrity of all"
		 "\neVote petitions.");
	  printf("\n\nYou must send in your reply-to before the petition is closed or"
		 "\nit won't be counted.\n");
	}
      printf("\nThank you for your %s on\n\n\t%s", 
	     (no_pet_votes == 0 ? "signature" : (no_pet_votes == 1 ?
						     "vote" : "votes")),
	     original_subject);
      voters= atoul(vote_str);
      printf("\n\nTo date, %lu %s %s.\n", voters,
	     (voters == 1? "person has" : "people have"),
	     (no_pet_votes ? "voted" : "signed this petition"));
      if (confirm_this.status == STARTING)
	{
	  printf("\nAfter you reply-to this message, the following will be recorded:\n\n");
	}
      highlight(no_pet_votes == 0 ? "YOUR SIGNATURE" : 
		 (no_pet_votes == 1 ? "YOUR VOTE" : "YOUR VOTES"));
      if (no_pet_votes)
	{
	  printf("\nYour votes %s recorded as:\n", (confirm_this.status == STARTING ? "will be" : "are"));
	  report_vote();
	  printf("\n");
	  highlight("YOUR COMMENTS");
	}
      if (confirm_this.status == STARTING)
	{
	  printf("\nYour %s recorded as:", (no_pet_votes ? "comments will be" :
					    "signature will be"));
	  printf("\n\n - - - - start of signature - - - - \n");
	  cc = write_signature(stdout, NO, NO);
	  printf("\n - - - - end of signature - - - - \n");
	}
      else
	{
	  printf("\nYour %s recorded as:", (no_pet_votes ? "comments are" :
					    "signature is"));
	  cc = write_signature(stdout, NO, YES);
	  printf("\n - - - - end of signature - - - - ");
	}
      if (cc != OK)
	{
	  sprintf(error_msg, "\neVote is unable to store your %s for %s now.\n\nPlease try later\n", 
		  (no_pet_votes? "vote" : "signature"), original_subject);
	  bounce_petition_error(SENDER | OWNER | ADMIN);
	}
      display_priv_type(p_item_copy);
      if (confirm_this.status != STARTING)
	display_remove(p_item_copy, the_language);
      display_petition_text(p_item_copy, NO, the_language);
      break;
    case NOT_ALLOWED:
      sprintf(error_msg, "\nHow can a petition signer be on vacation?");
      bounce_petition_error(SENDER | OWNER | ADMIN);
    default:
      /* impossible */
      break;
    }
  /*   For testing the timeout
       {
       long i;
       for (i = 0; i >= 0; i++)
       fprintf(stderr, "%d", i);
       }
  */
  if (reports > 0 && confirm_this.status != STARTING)
    {
      big_petition_finish(-2);  /* This puts the end of the email message
				   on stdout and returns, doesn't finish
				   with an exit.  This is so that ship_reports
				   can fork and exec an ftp process and
				   close stdout in the parent and get the
				   message send */
      ship_reports(voters);
      exit(0);
    }
  else
    {
      big_petition_finish(0);
    }
}
/**********************************************************
 * Sends the list of open petitions to the sender.  
 **********************************************************/
void
list_petitions(void)
{
  gen_header(SENDER, "Re:", YES);
  if (error_msg[0] != '\0')
    {
      printf("%s", error_msg);
    }
  printf("\nThe petitions currently available at %s are listed below.", 
	 whereami);
  printf("\n\nFor information about a particular petition:");
  printf("\n\n1.  Send a message to:");
  printf("\n\n\teVote@%s\n\n", whereami);
  printf("2.  Your subject line should match the petition title.");
  printf("\n\n3.  Your message should say: ");
  printf("\n\n\tinfo");
  printf("\n\n    to receive the text of the petition and instructions for signing.\n");
  lowlight("Privacy");
  printf("\nIn the list below:\n\n"
	 "PUBLIC  petitions allow anyone on the %s@%s\n"
	 "        list to retrieve the signatures, but not the email addresses,  \n"
	 "        attached to the petition.  Also, anyone who signs the petition\n"
	 "        can retrieve the signatures.  There are no controls on the use\n"
	 "        of your signature.  Your email address is used to ensure that there\n"
	 "        is only one signature per address and for no other purpose.\n\n"
	 "PRIVATE petitions allow only the petition's initiator and the system\n"
	 "        administrator at %s to retrieve the signatures.\n"
	 "        The email addresses are available to both people as well.\n",
	 list, whereami, whereami);
  if (no_petitions == -1)
    read_translations();
  if (no_publics == 0)
    printf("\nThere are no PUBLIC petitions.\n\n");
  else if (no_publics == 1)
    {
      printf("\nThere is one PUBLIC petition:\n\n");
      print_petitions(YES);
    }
  else
    {
      printf("\nPUBLIC petitions:\n\n");
      print_petitions(YES);
    }
  if (no_privates == 0)
    printf("\nThere are no PRIVATE petitions.\n\n");
  else if (no_privates == 1)
    {
      printf("\nThere is one PRIVATE petition:\n\n");
      print_petitions(NO);
    }
  else
    {
      printf("\nPRIVATE petitions:\n\n");
      print_petitions(NO);
    }
  leave();
  big_petition_finish(0);
}
/************************************************************/
void
no_pet_message(void)
{
  printf("\nThere is no petition attached to the subject, "
	 "\"%s\".\n"
	 "\nPlease check the spelling on the subject line of your message.\n", 
	     original_subject);
  list_petitions();
}
/*************************************************************/
static int
parse_vote(int * pcc, char **ans, int item_offset)
{
  static char answer[200];
  char delimiter[2] = " ";
  int cc, i, j;
  int vote;

  answer[0] = '\0';
  *ans = answer;
  while (1)
    {
      cc = get_token();
      strcat(answer, token);
      if (cc == EOF || cc == '\n')
	break;
      delimiter[0] = cc;
      strcat(answer, delimiter);
    }
  *pcc = cc;
  for (i = 0, j = 0; answer[i]; i++)
    {
      if (answer[i] == '\'' || answer[i] == '"' || answer[i] == '.'
	 || answer[i] == ',' || answer[i] == ';' || answer[i] == ':'
	 || answer[i] == '!')
	continue;
      answer[j++] = answer[i];
    }
  answer[j] = '\0';
  if ((p_first_vote_item+item_offset)->eVote.min == -1
     && (p_first_vote_item+item_offset)->eVote.max == 1)
    {
      for (i = 0; i < no_of_answers; i++)
	{
	  if (same(answer_list[i].str, answer))
	    return answer_list[i].answer;
	}
    }
  if (sscanf(answer,"%d", &vote) != 1)
    return UNKNOWN;
  if ((p_first_vote_item+item_offset)->eVote.min <= vote
     || (p_first_vote_item+item_offset)->eVote.max >= vote)
    return vote;
  return UNKNOWN;
}
/************************************************************/
static OKorNOT
check(RTYPE rtype)
{
  switch (rtype)
    {
    case FAILURE:
      sprintf(error_msg, "\nSystem troubles at %s: Your %s was not removed.\n", 
	      whereami, (no_pet_votes ? "vote" : "signature"));
      return NOT_OK;
      break;
    case NO_MODS:
      sprintf(error_msg, "\nProgramming error on pet_out.c:check.\n");
      return NOT_OK;
      break;
    case NO_CHANGE:  /* first vote makes all others 0 */
    case GOOD:  
      return OK;
    default:
      /* impossible */
      return NOT_OK;
      break;
    }
}
/************************************************************/
static void
remove_signer(int reports)
{
  unsigned long  readers, voters;
  char vote_str[10] = "  - ";
  char result[10] = "  - ";
  int cc;
  if (reports > 0)
    {
      get_mail_stats(p_item_copy->dropping_id,
		     &readers, vote_str, result);
      voters= atoul(vote_str) -1;
    }
  finish_unsign(from, &cc, NO);
  if (cc != OK)
    bounce_petition_error(SENDER);
  if (reports > 0)
    {
      big_petition_finish(-2);
      ship_reports(voters);
      exit(0);
    }
  else
    {
      big_petition_finish(0);
    }
}
/**********************************************************/
void
start_petition_displays(ITEM_INFO * p_item, YESorNO new_pet,
			     LANGUAGE which)
{
  unsigned long voters;
#ifdef ROSA
  void start_mexico();
  if (same(subject,"Mexico March"))
    {
      start_mexico();
      return;
    }
#endif
  printf("\nOn %s%s attached a %s to this subject:\n\n\t \"%s\"\n",
	 date_str(p_item->eVote.open), author_name, 
	 (no_pet_votes? "poll" : "petition"),
	 get_trans(subject, which, YES));
  if (new_pet)
    voters = 0L;
  else
    voters = get_mail_voters(p_item->dropping_id);
  if (p_item->eVote.vstatus == CLOSED) 
    {
      printf("\nThis %s was initiated on %s but has been closed since ",
	     (no_pet_votes ? "poll" : "petition"),
	     date_str(p_item->eVote.open));
      printf(date_str(p_item->eVote.close));
      printf("\n%lu %s%s collected.\n",
	     voters, (no_pet_votes ? "vote" : "signature"),
	     (voters == 1L ? " was" : "s were"));
    }
  else if (!new_pet)
    {
      printf("\n%lu %s%s been collected so far.\n",
	     voters, (no_pet_votes ? "vote" : "signature"),
	     (voters == 1L ? " has" : "s have"));
    }
}
/**********     FORMS  ******************/
/****************************************************
 *  called from check_form when the user doesn't
 *  fill it in properly.
 *****************************************************/
void
bounce_info_error(void)
{
  /* generate a message header on stdout */
  gen_header(SENDER, "Error:", NO);
  printf("\nIn response to your message which starts:\n");
  print_tokens(NO);
  /*  print_first_line(); */
  printf("%s", error_msg);
  error_msg[0] = '\0';
  if (dropping_id && item_info)
    {
#ifdef ROSA
      if (same("Mexico March", subject))
	printf("\nHere are the instructions for registering:\n");
      else
#endif
	printf("\nPerhaps the instructions for \"%s\" will help.\n",
	       original_subject);
      send_petition_info(SENDER, NULL, NO, NO, NO);
    }  
  leave();
  printf("\n\nYour entire message follows:\n");
  big_finish(0);
}
/************************************************************
 *  This checks the answers to the form that are supplied by the 
 *  signer.  If there is an error, it bounces the message.
 *  The int returned and cc are the delimiter for the
 *  current token.
 ************************************************************/
static	YESorNO could_match(char * right_name, char * try_name, int len);
static	char *force_name(int i);
int
check_form(int cc)
{
  YESorNO possible;
  int len, missing_fields;
  char delimit[2];
  char name[FIELD_LEN +1];
  int i, j, l;
  YESorNO nines, xes;
  extern char (*answer)[FIELD_LEN + 1];  /* in form.c */
  if (no_of_fields == 0)
    return cc;
  if ((answer = (char (*)[FIELD_LEN + 1])calloc((FIELD_LEN +1), no_of_fields)) 
     == NULL)
    {
      sprintf(error_msg, "\nThere are no resources available at this time to process your %s\non \"%s\".  Please try later.\n",
	      (no_pet_votes? "vote" : "signature"), original_subject);
      bounce_petition_error(ADMIN | OWNER | SENDER);
    }
  while (cc != EOF)
    {
      name[0] =  '\0';
      do
	{
	  if (name[0] != '\0')
	    {
	      possible = NO;
	      sprintf(delimit, "%c", cc);
	      strcat(name, delimit);
	      cc = get_token();
	    }
	  strncat(name, token, FIELD_LEN);
	  if (name[strlen(name) - 1] == ':')
	    name[strlen(name) -1] = '\0';
	  possible = NO;
	  len = strlen(name);
	  for (i = 0; i < no_of_fields; i++)
	    {
	      if (could_match(field[i].name[the_language], name, len))
		{
		  possible = YES;
		  break;
		}
	    }
	  if (possible == NO)  /* try other languages */
	    {
	      for (i = 0; i < no_of_fields; i++)
		{
		  for (l = 0; l < no_languages; l++)
		    {
		      if ((LANGUAGE)l == the_language
			 || field[i].name[(LANGUAGE)l][0] == '\0')
			continue;
		      if (could_match(field[i].name[l], name, len))
			{
			  possible = YES;
			  break;
			}
		    }
		  if (possible)
		    break;
		}
	    }
	  if (!possible)
	    break;
	}
      while (cc != '\n' && cc != EOF 
	    && token[strlen(token) -1] != ':');
      if (possible == NO  && !is_comment(name))
	{
	  /* unrecognized line coming in */
	  for (i = 0; i < no_of_fields; i++)
	    {
	      if (field[i].required == YES && answer[i][0] == '\0'
		 && !same(field[i].name[the_language], 
			  tongue[the_language].comment))
		{
		  sprintf(error_msg, 
			  "\nYou must give your \"%s:\""
			  "\nbefore you can give a comment when signing the petition for"
			  "\n\n\t\"%s\"\n", force_name(i),
			  original_subject);
		  bounce_info_error();
		}
	    }
	  /* assume it's an unmarked comment at the end */
	  cc = back_one_token();
	  found_comment = MAYBE;
	  break;
	}
      for (i = 0; i < no_of_fields; i++) 
	{
	  if (same(field[i].name[the_language], name))
	    {
	      break;
	    }
	}
      if (i == no_of_fields)  /* try other languages */
	{
	  for (i = 0; i < no_of_fields; i++)
	    {
	      for (l = 0; l < no_languages; l++)
		{
		  if ((LANGUAGE)l == the_language
		     || field[i].name[(LANGUAGE)l][0] == '\0')
		    continue;
		  if (could_match(field[i].name[l], name, len))
		    {
		      break;
		    }
		}
	      if (l < no_languages)
		break;
	    }
	}
      if (i >= no_of_fields)
	{
	  if (is_comment(name))
	    {
	      cc = back_one_token(); 
	      found_comment = YES;
	      break;
	    }
	  else
	    {
	      sprintf(error_msg, "\n\"%s\" is not part of the form for:\n\n\t\"%s\".\n",
		      name, original_subject);
	      bounce_info_error();
	    }
	}
      if (answer[i][0] != '\0')
	{
	  sprintf(error_msg, "\nPlease give your %s only once when signing the petition for \n\"%s\".\n", force_name(i), original_subject);
	  bounce_info_error();
	}
      if (same(field[i].name[the_language], tongue[the_language].comment) )
	{
	  back_one_token();
	  found_comment = YES;
	  break;
	}
      while (cc != '\n' && cc != EOF)
	{
	  cc = get_token();
	  if (strlen(token) + strlen(answer[i]) + 2 > FIELD_LEN)
	    {
	      sprintf(error_msg, "\nPlease limit your response for \"%s:\" to %d characters.\n", field[i].name[the_language], FIELD_LEN);
	      bounce_info_error();
	    }
	  strcat(answer[i], token);
	  if (cc != '\n' && cc != EOF)
	    {
	      sprintf(delimit, "%c", cc);
	      strcat(answer[i], delimit);
	    }
	}
      if (cc != EOF)
	cc = get_token();
    }
  /*  The form has been read to the EOF or to a comment: field */
  for (i = 0; i < no_of_fields; i++)
    {
      if (same("email", field[i].name[ENGLISH]) 
	 && answer[i][0] != '\0'
	 && !same(answer[i], "No") && !same(answer[i], "Non"))
	{
	  give_email = YES;
	}
      if ((field[i].required == YES && answer[i][0] == '\0')
	 && (!same(field[i].name[the_language], tongue[the_language].comment) 
	     || found_comment == NO))
	{
	  missing_fields = 1;
	  for (j = i+1; j < no_of_fields; j++)
	    {
	      if ((field[j].required == YES && answer[j][0] == '\0')
		 && (!same(field[j].name[the_language],
			   tongue[the_language].comment) 
		     || found_comment == NO))
		missing_fields++;
	    }
	  sprintf(error_msg, "\nTo %s on \n\n\t\"%s\"\n\nyou must include your \"%s\"", 
		  (no_pet_votes ? "vote" : "sign the petition"),
		  original_subject, force_name(i));
	  if (missing_fields == 1)
	    strcat(error_msg, ".\n");
	  else
	    {
	      for (j = i+1; j < no_of_fields; j++)
		{
		  if ((field[j].required == YES && answer[j][0] == '\0')
		     && (!same(field[j].name[the_language],
			       tongue[the_language].comment) 
			 || found_comment == NO))
		    {
		      switch (--missing_fields)
			{
			case 0:
			  break;
			case 1:
			  strcat(error_msg, " and ");
			  strcat(error_msg, force_name(j));
			  strcat(error_msg, ".\n");
			  break;
			default:
			  strcat(error_msg, ", ");
			  strcat(error_msg, force_name(j));
			  break;
			}
		    }
		}
	    }
	  bounce_info_error();
	}
      if (field[i].required == YES && check_format(i) != OK)
	{
	  char instruction[400];
	  sprintf(instruction, "\nWhen supplying your \"%s,\" please make your \nresponse look like:\n\n\t%s: %s\n\n", field[i].name[the_language], field[i].name[the_language], field[i].format);
	  nines = NO;
	  xes = NO;
	  for (j=0; field[i].format[j]; j++)
	    {
	      if (field[i].format[j] == '9')
		nines = YES;
	      if (field[i].format[j] == 'X')
		xes = YES;
	    }
	  if (nines == NO)
	    {
	      sprintf(error_msg,"%swhere you replace all X's with the correct letters.\n", instruction);
	    }
	  else if (xes == NO)
	    {
	      sprintf(error_msg,"%swhere you replace all 9's with the correct numbers.\n", instruction);
	    }
	  else
	    {
	      sprintf(error_msg,"%swhere you replace all 9's with the correct numbers and all X's \nwith the correct letters.\n", instruction);
	    }
	  bounce_info_error();
	}
    }
  return cc;
}
/************************************************************/
static YESorNO
could_match(char * right_name, char * try_name, int len)
{
  if (strNcmp(right_name, try_name, len) == (unsigned)0
     && (len == (int)strlen(right_name) || right_name[len] == ' '))
    {
      if (len == (int)strlen(right_name))
	{
	  if (right_name[len] == '\0' && token[strlen(token)-1] == ':')
	    return YES;
	  else
	    return NO;
	}
      return YES;
    }
  return NO;
}
/************************************************************/
static char
*force_name(int j)
{
  return (field[j].name[the_language][0] == '\0'?
	  field[j].name[default_language] :
	  field[j].name[the_language]);
}
/************************************************************
 *  Called from display_howto when there is a form.
 *  Returns the number of notes printed.
 ************************************************************/
int
explain_form(LANGUAGE which)
{
  int i, j;
  YESorNO numbers = NO;
  YESorNO letters = NO;
  int len;
  int notes = 0;
  int notes_printed = 0;
  int last_required = -1;
  int required = 0;
  int last_optional = -1;
  int optional = 0;
  YESorNO comment = NO, name_field = NO;
  LANGUAGE do_this = which;
  printf("\n3.  Please make your message look like: \n");
  printf("\n---  cut here --- \n");
  for (i = 1; i <= no_pet_votes; i++)
    {
      printf("\n%d. Your vote", i);
    }
  if (no_of_fields > 0)
    {
      if (field[0].name[which][0] == '\0')
	do_this = DEFAULT_LANGUAGE;
      for (i = 0; i < no_of_fields; i++)
	{
	  if (strNcmp(field[i].name[do_this],
		     tongue[do_this].comment, 
		     strlen(tongue[do_this].comment)) == 0)
	    comment = YES;
	  if (strNcmp(field[i].name[do_this],
		     "name", 4) == 0)
	    name_field = YES;
	  printf("\n%s: %s",  field[i].name[do_this], field[i].format);
	  if (field[i].required)
	    {
	      last_required = i;
	      required++;
	    }
	  else
	    {
	      last_optional = i;
	      optional++;
	    }
	  for (j = 0; field[i].format[j]; j++)
	    {
	      if (field[i].format[j] == 'X')
		letters = YES;
	      if (field[i].format[j] == '9')
		numbers = YES;
	    }
	}
    }
  if (!comment && name_field)
    {
      printf("\nYou may add a %s here.",
	     tongue[do_this].comment);
    }
  else if (!comment && !name_field)
    {
      printf("\nYou may give your name and/or a %s here.",
	     tongue[do_this].comment);
    }
  else if (comment && !name_field)
    {
      printf("\nYou may give your name here.");
    }
  printf("\n\n---  cut here --- \n");
#ifdef ROSA
  if (!same("Mexico March", subject))
#endif    
  if (no_pet_votes)
    {
      printf("\n    where each \"1.\", \"2.\", etc., represents a question number.  Instead"
	     "\n    of \"Your vote\", you can answer \"Yes\", \"No\", or \"Don't Know.\"\n");
    } 
  notes = (required ? 1 : 0) + (optional ? 1 : 0)
    + letters + numbers;
  if (notes == 0)
    return notes;
  printf("\n      %s:\n", (notes > 1 ? "NOTES" : "NOTE"));
  if (required)
    {
      ++notes_printed;
      printf("\n      *  ");
      len = 39;
      printf("You must give %s for ", (required == 1?"an answer":"answers"));
      for (i = 0; i < no_of_fields; i++)
	{
	  if (field[i].required == NO)
	    continue;
	  if ((len += strlen(field[i].name[do_this]) + 5) >= 75) /* colon +
								   comma +
								   space */
	    {
	      printf("\n         ");
	      len = 10 + strlen(field[i].name[do_this]) + 5;
	    }
	  printf("\"%s:\"", field[i].name[do_this]);
	  if (i == last_required)
	    {
	      printf(".\n");
	      break;
	    }
	  else 
	    printf(", ");
	  if (required > 1 && i + 1 == last_required)
	    {
	      if ((len += 4) > 75)
		{
		  if (notes > 1)
		    {
		      printf("\n          ");
		    }
		  else
		    {
		      printf("\n      ");
		    }
		}
	      printf("and ");
	    }
	}
    }
  if (optional)
    {
      ++notes_printed;
      printf("\n      *  ");
      len = 39;
      printf("%s optional for ",
	     (no_of_fields - required == 1?"An answer is":"Answers are"));
      for (i = 0; i < no_of_fields; i++)
	{
	  if (field[i].required == YES)
	    continue;
	  if ((len += strlen(field[i].name[do_this]) + 5) >= 75) /* colon +
								   comma +
								   space */
	    {
	      printf("\n         ");
	      len = 10 + strlen(field[i].name[do_this]) + 5;
	    }
	  printf("\"%s:\"", field[i].name[do_this]);
	  if (i == last_optional)
	    {
	      printf(".\n");
	      break;
	    }
	  else 
	    printf(", ");
	  if (optional > 1 && i + 1 == last_optional)
	    {
	      if ((len += 4) > 75)
		{
		  if (notes > 1)
		    {
		      printf("\n          ");
		    }
		  else
		    {
		      printf("\n      ");
		    }
		}
	      printf("and ");
	    }
	}
    }
  if (numbers)
    {
      printf("\n      *  ");
      printf("Please replace the 9's with the correct numbers.\n");
    }
  if (letters)
    {
      printf("\n      *  ");
      printf("Please replace the X's with the correct letters.\n");
    }
  return notes_printed;
}
/************************************************************/
void
report_vote(void)
{
  int i;
  for (i = 0; i < no_pet_votes; i++)
    {
      printf("\n%d. %s", i+1, (pet_vote[i] == 0 ? "Don't know" : 
			       (pet_vote[i] == 1
				? "Yes" : "No")));
    }
}
/*********************************************************
 *     Stores message text and email address.
 **********************************************************/ 
void
store_signature(ITEM_INFO * p_item)
{
  FILE * fp, *fp_tmp;
  char *fname;
  time_t when;
  if (now == 0L)
    {
      fprintf(stderr,"0 stamp in store_signature");
      time(&now);
    }
  push_time(p_item, now);
  when = pull_time(p_item);
  if (when == 0L)
    {
      fprintf(stderr,"0 from pull after push in store_signature");
    }
  fname = pet_signers_fname(p_item, now);
  if ((fp_tmp = lock_to_tmp(fname)) == NULL)
    {
      sprintf(error_msg,"\n\nERROR! %s \n\nCan't open temporary "
	      "file lock to store \nyour signature for \"%s\".\n"
	      "Please try later.\n",
	      fname, subject);
      bounce_petition_error(SENDER | OWNER | ADMIN);
    }
  if ((fp = fopen(fname, "a")) == NULL 
     || write_signature(fp, YES, YES) != OK)
    {
      sprintf(error_msg, "\neVote is unable to store your signature"
	      "for %s now.\n\nPlease try later\n", subject);
      bounce_petition_error(SENDER | OWNER | ADMIN);
    }
  if (unlock_to_tmp(NO) != OK)
    {
      fprintf(stderr,"\nERROR! Unable to remove tmp file: %sT.\n",
	      fname);
    }			
  fclose(fp);
  chmod(fname, 00660);
  return;
}
/************************************************************/
static void
vote_the_petition(void)
{
  int i;
  char * tmp;
  short old_vote;
  tmp = subject;
  subject = extra_subject;
  copy_poll(YES);
  if (p_item_copy->eVote.vstatus == CLOSED)
    {
      sprintf(error_msg,"\nYou are too late to %s on:\n\n\t%s"
	      "\n\nThe %s is closed.\n",
	      (no_pet_votes 
	       ? (confirm_this.status == VERIFIED ? "confirm your vote" : "vote")
	       : (confirm_this.status == VERIFIED ? "confirm your signature" :"sign the petition")),
	      original_subject,
	      (no_pet_votes ? "poll" : "petition"));
      bounce_info_error();
    }
  for (i = 0; i < no_pet_votes; i++)
    {
      check(send_vote(p_item_copy + 1 + i, pet_vote[i], &old_vote));
    }
  subject = tmp;
  copy_poll(YES);
  petition = YES;
}
/************************************************************/
static void
finish_unsign(char * name, int * pcc, YESorNO from_owner)
{
  time_t when;
  short old_vote;
  char t_str[40];
  unsigned long  readers;
  char vote_str[10] = "  - ";
  char result[10] = "  - ";
  int i;
#ifdef ROSA
  void mexico_unsign(char * name, int * pcc, YESorNO from_owner);
  if (same(subject,"Mexico March"))
    {
      mexico_unsign(name, pcc, from_owner);
      return;
    }
#endif
  if (have_i_voted(p_item_copy) == NO)
    {
      sprintf(error_msg,"\n%s has not %s \"%s\".\n",
	      name, (no_pet_votes ? "voted in" : "signed" ), subject);
      i_am_leaving();
      *pcc = NOT_OK;
      return;
    }
  if (confirm_this.status == STARTING && !from_owner)
    {
      char buf[1000];
      if (collect_confirm(FOR_SIG) == NULL)
	{
	  bounce_petition_error(SENDER | OWNER | ADMIN);
	}
      sprintf(buf, "Confirm: %s", confirm_this.key);
      gen_header(SENDER, buf, NO);
      printf("Thank you for your \"remove\" message on \n\n\t%s", 
	     original_subject);
      printf("\n\nTo verify that your message really came from you, please confirm"
	     "\nthis message by using \"Reply-To\" and do not alter the subject"
	     "\nline.  You don't need to type anything new in your reply, it's"
	     "\njust a confirmation message.  This ensures the integrity of all"
	     "\neVote petitions.");
      printf("\n\nYou must send in your reply-to before the petition is closed or"
	     "\nyour %s won't be removed.\n", (no_pet_votes ? "votes" : "signature"));
      big_petition_finish(0);
    }
  when = pull_time(p_item_copy);
  if ((*pcc = check(send_vote(p_item_copy, READ, &old_vote))) == NOT_OK)
    return;
  if (no_pet_votes)
    {
      char * hold;
      hold = subject;
      subject = extra_subject;
      copy_poll(YES);
      pet_vote = malloc(sizeof(int) * no_pet_votes);
      if (have_i_voted(p_item_copy+1) != NO)
	{
	  if (pet_vote != NULL)
	    {
	      for (i = 1; i <= no_pet_votes ; i++)
		{
		  get_mail_stats((p_item_copy+i)->dropping_id,
				 &readers, vote_str, result);
		  pet_vote[i-1] = atoi(vote_str);
		}
	    }
	}
      check(send_vote(p_item_copy + 1, READ, &old_vote));
      subject = hold;
      petition = YES;
      copy_poll(YES);
    }
  strcpy(t_str, time_str);  /* i_am_leaving wipes it out */
  i_am_leaving();
  if (current_action == SIGNER)
    {
      drop_mail_voter(lclist, YES);
    }
  if (!from_owner)
    {
      strcpy(time_str, t_str);  /* for message to user */
      gen_header(SENDER, "eVote Rcpt:", YES);
      time_str[0] = '\0';  /* wipe it out to prevent another i_am_leaving */
      printf("\nYour %s been removed on \"%s\".",
	     (no_pet_votes? "votes have" : "signature has"), original_subject);
      if (pet_vote != NULL)
	{
	  printf("\n\nThe votes that were removed are:\n");
	  report_vote();
	}
      printf("\n\nThe comment that was removed is:\n");
    }
  if (from_owner)
    {
      printf("\n%s has been unsigned.\nThe text removed was:\n", name);
    }
  drop_signature(mail_voter, stdout, when);
  *pcc = OK;
  return;
}

#ifdef ROSA
/************************************************************/
void
register_mexico(void)
{
  int cc;
  short old_vote;
  int reports = 0;
  int fields;
  int i;
  unsigned long  readers, voters;
  char vote_str[10] = "  - ";
  char result[10] = "  - ";
  int mexico_votes(void);
  int mexico_form2(int cc);
  if (!into_eVote && set_up_eVote() != OK)  /* Identifies the list, and user */
    /* if we get this far, communication to the list is working
       the list exists and, if mail_voter != 0, we're into
       the database.  */
    {
      enter_signer();
    }
  if (current_action == READ_ONLY)
    {
      sprintf(error_msg,"\nYou can not register by email.  \nYou are a read_only member of %s.\n", list);
      bounce_petition_error(SENDER | OWNER);
    }
  if (!into_eVote)
    {
      copy_poll(YES); /* If dropping_id != 0, we have a polled
			 subject */
      into_eVote = YES;
    }
  if (confirm_this.status == CHECK)
    /* Confirm: found in subject */
    {
      check_sig_confirm();  /* places VERIFIED in status */
      /* it starts over with the waiting message */
      do_petition();
      /* or it generates an error message and quits */
    }  
  if (dropping_id == 0 || petition != YES 
     || (p_item_copy->eVote.type != TALLIEDY 
	 && p_item_copy->eVote.type != TIMESTAMP))
    {
      gen_header(SENDER, "ERROR:", NO);
      no_pet_message();
    }
  /* Check for remove message and switch to correct language */
  for (i = 0; i < no_languages; i++)
    {
      if (same(token, tongue[(LANGUAGE)i].remove))
	{
	  remove_sig = YES;
	  if (!same(tongue[the_language].remove, tongue[(LANGUAGE)i].remove))
	    {
	      the_language = (LANGUAGE)i;
	      back_one_token();
	      translate(the_language, DO_PETITION, NULL, 
			MAYBE, MAYBE, MAYBE, NULL, NULL, MAYBE);
	    }
	  break;
	}
    }  
  if (error_msg[0] != '\0')  /* remove_sig and never eVoted */
    {
      bounce_info_error();
    }
  if (same(tongue[the_language].help, token) 
     || same(tongue[the_language].info, token))
    send_petition_info(SENDER, p_item_copy, NO, NO, NO);
  /* Switch to the right language ??? */
  for (i = 0; i < no_languages; i++)
    {
      if (same(token, tongue[(LANGUAGE)i].help)
	 || same(token, tongue[(LANGUAGE)i].info))
	{
	  the_language = (LANGUAGE)i;
	  send_petition_info(SENDER, p_item_copy, NO, NO, NO);
	}
    }
  if (p_item_copy->eVote.vstatus == CLOSED)
    {
      if (remove_sig == YES)
	{
	  sprintf(error_msg, "\nYou are too late to cancel your reservation.\n");
	}
      else
	{
	  sprintf(error_msg, "\nYou are too late to register for the March with Xokonoschtletl.\n");
	}
      too_late();
    }
  reports  = read_report_instructions(p_item_copy);
  fields = read_form_template(p_item_copy);
  /* remove_sig is set in strip_subject() if [R] in subject
     line -- good for javascript */
  if (remove_sig == YES)
    {
      remove_signer(reports); /* collects confirm */
    }
  if (no_pet_votes && (cc = mexico_votes()) == -1)
    bounce_info_error(); 
  if (form_exists)         /* check that the user filled in the form */
    {
      cc = mexico_form2(cc);
    }
  if (no_pet_votes || form_exists)
    {
      set_sig_start(cc);
    }
  switch (send_vote(p_item_copy, 1, &old_vote))
    {
    case FAILURE:
      sprintf(error_msg, "\nSystem troubles at deliberate.com.  You reservation has not been recorded.\n");
      bounce_petition_error(SENDER | ADMIN | OWNER);
      break;
    case NO_MODS:
      sprintf(error_msg, "\nYou are too late to register for the March with Xokonoschtletl.\n");
      break;
    case NO_CHANGE:
      sprintf(error_msg, "\nYou have already registered for the March."
	      "\n\nIf you wish to change your registration:\n"
	      "\n1.  Remove your old registration:"
	      "\n\n\ta.  Reply-to this message."
	      "\n\n\tb.  Your message should say:"
	      "\n\n\t\tremove"
	      "\n\n2.  Send in a new registration.\n"
	      "\nIf you have already sent in money, please also write to:"
	      "\n\n\tmexico-info@deliberate.com.\n");
      bounce_info_error();
      break;
    case GOOD:  
      get_mail_stats(p_item_copy->dropping_id,
		     &readers, vote_str, result);
      if (confirm_this.status != STARTING)  /* NOT_NEEDED or VERIFIED */
	{
	  store_signature(p_item_copy);
	  if (no_pet_votes)
	    vote_the_petition();
	  gen_header(SENDER, "Mexico March Rcpt:", NO);
	  printf("Thank you for registering for the Mexico March with"
		 "\nXokonoschtletl.  You have a reservation for %d %s.", pet_vote[0], (pet_vote[0] == 1 ?
							       "person" : "people"));
	  printf("\n\nTo hold your reservation please send $%d to:",
		 425 * pet_vote[0]);
	}
      else
	{
	  char buf[100];
	  check(send_vote(p_item_copy, READ, &old_vote));
	  get_mail_stats(p_item_copy->dropping_id,
			 &readers, vote_str, result);
	  if (collect_confirm(FOR_SIG) == NULL)  /* sets key */
	    {
	      bounce_petition_error(SENDER | OWNER | ADMIN);
	    }
	  sprintf(buf, "Confirm: %s", confirm_this.key);
	  gen_header(SENDER, buf, NO);
	  printf("Your reservation for the March in Mexico with Xokonoschtletl"
		 "\nfor Kopilli Ketzalli has not been recorded yet.  To complete your"
		 "\nreservaton process, please:");
	  printf("\n\n1.  To verify that your message really came from you, please confirm"
		 "\n    this message by using \"Reply-To\" and do not alter the subject"
		 "\n    line.  You don't need to type anything new in your reply, it's"
		 "\n    just a confirmation message.\n");
	  printf("\n2.  To hold your reservation please send $%d to:",
		 425 * pet_vote[0]);
	} /* needs confirm */
      printf("\n\n\tYankuikanahuak"
	     "\n\t2555 W. Middlefield Rd, #150"
	     "\n\tMountain View, CA 94043\n");
      highlight("YOUR RESERVATION");
      if (confirm_this.status == STARTING)
	{
	  printf("\nAfter you reply-to this message, the following will be recorded\n"
		 "with your reservation for %d %s:\n",
		 pet_vote[0], pet_vote[0] == 1? "person" : "people");
	}
      else
	printf("\nYour reservation for %d %s is recorded as:\n",
	       pet_vote[0], pet_vote[0] == 1? "person" : "people");
      cc = write_signature(stdout, NO, YES);
      if (cc != OK)
	{
	  sprintf(error_msg, "\nWe are unable to process your registration now.\n\nPlease try later\n");
	  bounce_petition_error(SENDER | OWNER | ADMIN);
	}
      if (confirm_this.status != STARTING)
	display_remove(p_item_copy, the_language);
      display_petition_text(p_item_copy, NO, the_language);
      break;
    case NOT_ALLOWED:
      sprintf(error_msg, "\nHow can a petition signer be on vacation?");
      bounce_petition_error(SENDER | OWNER | ADMIN);
    default:
      /* impossible */
      break;
    }
  if (reports > 0 && confirm_this.status != STARTING)
    {
      big_petition_finish(-2);  /* This puts the end of the email message
				   on stdout and returns, doesn't finish
				   with an exit.  This is so that ship_reports
				   can fork and exec an ftp process and
				   close stdout in the parent and get the
				   message send */
      ship_reports(voters);
      exit(0);
    }
  else
    {
      big_petition_finish(0);
    }
}
/************************************************************/
void 
start_mexico(void)
{
  printf("\nThis facility is for processing registrations for the March and tour"
	 "\nin Mexico with Xokonoschtletl for the return of Kopilli Ketzalli."
	 "\n\nFor information, see these web sites:"
	 "\n\n\t       About the March      http://www.deliberate.com/xoko"
	 "\n\n\tAbout Kopilli Ketzalli      http://www.deliberate.com/aztec\n\n");
}
/************************************************************
 *    This checks the answers to the form that are supplied
 *    by the signer.  If there is an error, it bounces the
 *    message.
 *    The int returned and cc are the delimiter for the
 *    current token.
 ************************************************************/
int
mexico_form2(int cc)
{
  YESorNO possible;
  int len, missing_fields;
  char delimit[2];
  char name[FIELD_LEN +1];
  int i, j, l;
  YESorNO nines, xes;
  extern char (*answer)[FIELD_LEN + 1];  /* in form.c */
  if (no_of_fields == 0)
    return cc;
  if ((answer = (char (*)[FIELD_LEN + 1])calloc((FIELD_LEN +1), no_of_fields)) 
     == NULL)
    {
      sprintf(error_msg, "\nThere are no resources available at this time to process your registration.  Please try later.\n");
      bounce_petition_error(ADMIN | OWNER | SENDER);
    }
  while (cc != EOF)
    {
      name[0] =  '\0';
      do
	{
	  if (name[0] != '\0')
	    {
	      possible = NO;
	      sprintf(delimit, "%c", cc);
	      strcat(name, delimit);
	      cc = get_token();
	    }
	  strncat(name, token, FIELD_LEN);
	  if (name[strlen(name) - 1] == ':')
	    name[strlen(name) -1] = '\0';
	  possible = NO;
	  len = strlen(name);
	  for (i = 0; i < no_of_fields; i++)
	    {
	      if (could_match(field[i].name[the_language], name, len))
		{
		  possible = YES;
		  break;
		}
	    }
	  if (possible == NO)  /* try other languages */
	    {
	      for (i = 0; i < no_of_fields; i++)
		{
		  for (l = 0; l < no_languages; l++)
		    {
		      if ((LANGUAGE)l == the_language
			 || field[i].name[(LANGUAGE)l][0] == '\0')
			continue;
		      if (could_match(field[i].name[l], name, len))
			{
			  possible = YES;
			  break;
			}
		    }
		  if (possible)
		    break;
		}
	    }
	  if (!possible)
	    break;
	}
      while (cc != '\n' && cc != EOF 
	    && token[strlen(token) -1] != ':');
      if (possible == NO  && !is_comment(name))
	{
	  /* unrecognized line coming in */
	  for (i = 0; i < no_of_fields; i++)
	    {
	      if (field[i].required == YES && answer[i][0] == '\0'
		 && !same(field[i].name[the_language], 
			  tongue[the_language].comment))
		{
		  sprintf(error_msg, 
			  "\nYou must give your \"%s:\""
			  "\nwhen registering for the March in Mexico.\n",
			  force_name(i));
		  bounce_info_error();
		}
	    }
	  /* assume it's an unmarked comment at the end */
	  cc = back_one_token();
	  found_comment = MAYBE;
	  break;
	}
      for (i = 0; i < no_of_fields; i++) 
	{
	  if (same(field[i].name[the_language], name))
	    {
	      break;
	    }
	}
      if (i == no_of_fields)  /* try other languages */
	{
	  for (i = 0; i < no_of_fields; i++)
	    {
	      for (l = 0; l < no_languages; l++)
		{
		  if ((LANGUAGE)l == the_language
		     || field[i].name[(LANGUAGE)l][0] == '\0')
		    continue;
		  if (could_match(field[i].name[l], name, len))
		    {
		      break;
		    }
		}
	      if (l < no_languages)
		break;
	    }
	}
      if (i >= no_of_fields)
	{
	  if (is_comment(name))
	    {
	      cc = back_one_token(); 
	      found_comment = YES;
	      break;
	    }
	  else
	    {
	      sprintf(error_msg, "\n\"%s\" is not part of the form for registering.",
		      name);
	      bounce_info_error();
	    }
	}
      if (answer[i][0] != '\0')
	{
	  sprintf(error_msg, "\nPlease give your %s only once when registering for the March.\n", force_name(i));
	  bounce_info_error();
	}
      if (same(field[i].name[the_language], tongue[the_language].comment) )
	{
	  back_one_token();
	  found_comment = YES;
	  break;
	}
      while (cc != '\n' && cc != EOF)
	{
	  cc = get_token();
	  if (strlen(token) + strlen(answer[i]) + 2 > FIELD_LEN)
	    {
	      sprintf(error_msg, "\nPlease limit your response for \"%s:\" to %d characters.\n", field[i].name[the_language], FIELD_LEN);
	      bounce_info_error();
	    }
	  strcat(answer[i], token);
	  if (cc != '\n' && cc != EOF)
	    {
	      sprintf(delimit, "%c", cc);
	      strcat(answer[i], delimit);
	    }
	}
      if (cc != EOF)
	cc = get_token();
    }
  /*  The form has been read to the EOF or to a comment: field */
  give_email = YES;
  for (i = 0; i < no_of_fields; i++)
    {
      if ((field[i].required == YES && answer[i][0] == '\0')
	 && (!same(field[i].name[the_language], tongue[the_language].comment) 
	     || found_comment == NO))
	{
	  missing_fields = 1;
	  for (j = i+1; j < no_of_fields; j++)
	    {
	      if ((field[j].required == YES && answer[j][0] == '\0')
		 && (!same(field[j].name[the_language],
			   tongue[the_language].comment) 
		     || found_comment == NO))
		missing_fields++;
	    }
	  sprintf(error_msg, "\nTo register for the March you must include your \"%s\"", 
		  force_name(i));
	  if (missing_fields == 1)
	    strcat(error_msg, ".\n");
	  else
	    {
	      for (j = i+1; j < no_of_fields; j++)
		{
		  if ((field[j].required == YES && answer[j][0] == '\0')
		     && (!same(field[j].name[the_language],
			       tongue[the_language].comment) 
			 || found_comment == NO))
		    {
		      switch (--missing_fields)
			{
			case 0:
			  break;
			case 1:
			  strcat(error_msg, " and ");
			  strcat(error_msg, force_name(j));
			  strcat(error_msg, ".\n");
			  break;
			default:
			  strcat(error_msg, ", ");
			  strcat(error_msg, force_name(j));
			  break;
			}
		    }
		}
	    }
	  bounce_info_error();
	}
      if (field[i].required == YES && check_format(i) != OK)
	{
	  char instruction[400];
	  sprintf(instruction, "\nWhen supplying your \"%s, \" please make your \nresponse look like:\n\n\t%s: %s\n\n", field[i].name[the_language], field[i].name[the_language], field[i].format);
	  nines = NO;
	  xes = NO;
	  for (j=0; field[i].format[j]; j++)
	    {
	      if (field[i].format[j] == '9')
		nines = YES;
	      if (field[i].format[j] == 'X')
		xes = YES;
	    }
	  if (nines == NO)
	    {
	      sprintf(error_msg,"%swhere you replace all X's with the correct letters.\n", instruction);
	    }
	  else if (xes == NO)
	    {
	      sprintf(error_msg,"%swhere you replace all 9's with the correct numbers.\n", instruction);
	    }
	  else
	    {
	      sprintf(error_msg,"%swhere you replace all 9's with the correct numbers and all X's \nwith the correct letters.\n", instruction);
	    }
	  bounce_info_error();
	}
    }
  return cc;
}
/************************************************************/
int
mexico_form(LANGUAGE which)
{
  int i, j;
  YESorNO numbers = NO;
  YESorNO letters = NO;
  int notes = 0;
  int last_required = -1;
  int required = 0;
  int last_optional = -1;
  int optional = 0;
  YESorNO comment = NO, name_field = NO;
  LANGUAGE do_this = which;
  printf("\n3.  Please make your message look like: \n");
  printf("\n---  cut here --- \n");
  printf("\nNo_of_reservations:");
  if (no_of_fields > 0)
    {
      if (field[0].name[which][0] == '\0')
	do_this = DEFAULT_LANGUAGE;
      for (i = 0; i < no_of_fields; i++)
	{
	  if (strNcmp(field[i].name[do_this],
		     tongue[do_this].comment, 
		     strlen(tongue[do_this].comment)) == 0)
	    comment = YES;
	  if (strNcmp(field[i].name[do_this],
		     "name", 4) == 0)
	    name_field = YES;
	  printf("\n%s: %s",  field[i].name[do_this], field[i].format);
	  if (field[i].required)
	    {
	      last_required = i;
	      required++;
	    }
	  else
	    {
	      last_optional = i;
	      optional++;
	    }
	  for (j = 0; field[i].format[j]; j++)
	    {
	      if (field[i].format[j] == 'X')
		letters = YES;
	      if (field[i].format[j] == '9')
		numbers = YES;
	    }
	}
    }
  if (!comment && name_field)
    {
      printf("\nYou may add a %s here.",
	     tongue[do_this].comment);
    }
  else if (!comment && !name_field)
    {
      printf("\nYou may give your name and/or a %s here.",
	     tongue[do_this].comment);
    }
  else if (comment && !name_field)
    {
      printf("\nYou may give your name here.");
    }
  printf("\n\n---  cut here --- \n");
  notes = (required ? 1 : 0) + (optional ? 1 : 0)
    + letters + numbers;
  printf("\n      NOTE:\n");
  printf("\n      *  You must give an answer for each question, except"
	 "\n         \"comment\".  \"comment\" is optional.\n");
  return 1;
}
/************************************************************/
int 
mexico_votes(void)
{
  int no = 0;
  int end;
  int i, cc;
  YESorNO some_vote = NO;
  YESorNO no_good = NO;
  char *answer;
  char local_token[100];
  if ((pet_vote = malloc(sizeof(int) * no_pet_votes))
     == NULL)
    {
      sprintf(error_msg,"\n%s is out of resources right now.  Please try your message later.\n", whereami);
      return -1;
    }
  for (no = 0; no < no_pet_votes; no++)
    pet_vote[no] = UNKNOWN;
  while (1)
    {
      no_good = NO;
      end = strlen(token)-1;
      if (cc == EOF || end == 0 
	 || (token[end] != ':' && token[end] != '.' && token[end] != '-'
	     && token[end] != ','))
	{
	  no_good = YES;
	}
      strcpy(local_token, token);
      local_token[end] = '\0';
      if (same(local_token,"No_of_reservations"))
	{
	  strcpy(local_token,"1.");
	}
      if (no_good || (no = atoi(local_token)) == 0 )
	{
	  if (!some_vote)
	    {							
	      sprintf(error_msg,"\nThe first line of your message should look something like:"
		      "\n\nNo_of_reservations:  3\n");
	      return -1;
	    }
	  for (i = 0; i < no_pet_votes; i++)
	    {
	      if (pet_vote[i] == UNKNOWN)
		pet_vote[i] = 0;
	    }
	  return cc;
	}
      if (no > no_pet_votes)
	{
	  sprintf(error_msg,"This program is very confused by your response.\n");
	  return -1;
	}
      if (pet_vote[no-1] != UNKNOWN)
	{
	  sprintf(error_msg,"\nYou can only vote once on question %d.\n",
		  no);
	  return -1;
	}
      if ((pet_vote[no-1] = parse_vote(&cc, &answer, no-1)) == UNKNOWN)
	{ /* parse_vote puts an error_msg in */
	  if ((p_first_vote_item+no)->eVote.min == -1
	     && (p_first_vote_item+no)->eVote.max == 1)
	    {
	      sprintf(error_msg, "\neVote doesn't recognize your response for question %d."
		      "\nYou responded \"%s\".  You must answer"
		      "\n\"Yes\", \"No\", or \"Don't Know.\"\n", 
		      no, answer);
	    }
	  else
	    {
	      sprintf(error_msg, "\neVote doesn't recognize your response for question %d."
		      "\nYou responded \"%s\".  You must answer"
		      "a number between %d and %d.\n",
		      no, answer,
		      (p_first_vote_item+no)->eVote.min,
		      (p_first_vote_item+no)->eVote.max);
	    }
	  return -1;
	}
      some_vote = YES;
      cc = get_token();
    }
  return cc;
}
/************************************************************/
void 
mexico_unsign(char * name, int * pcc, YESorNO from_owner)
{
  time_t when;
  short old_vote;
  char t_str[40];
  unsigned long  readers;
  char vote_str[10] = "  - ";
  char result[10] = "  - ";
  int i;
  if (have_i_voted(p_item_copy) == NO)
    {
      sprintf(error_msg,"\nWe have no reservation for %s for the Mexico March.\n",  name);
      /*      i_am_leaving(); */
      *pcc = NOT_OK;
      return;
    }
  if (confirm_this.status == STARTING && !from_owner)
    {
      char buf[1000];
      if (collect_confirm(FOR_SIG) == NULL)
	{
	  bounce_petition_error(SENDER | OWNER | ADMIN);
	}
      sprintf(buf, "Confirm: %s", confirm_this.key);
      gen_header(SENDER, buf, NO);
      printf("Thank you for your \"remove\" message on your reservation for the"
	     "\nMexico March.");
      printf("\n\nTo verify that your message really came from you, please confirm"
	     "\nthis message by using \"Reply-To\" and do not alter the subject"
	     "\nline.  You don't need to type anything new in your reply, it's"
	     "\njust a confirmation message.\n");
      big_petition_finish(0);
    }
  when = pull_time(p_item_copy);
  if ((*pcc = check(send_vote(p_item_copy, READ, &old_vote))) == NOT_OK)
    return;
  if (no_pet_votes)
    {
      char * hold;
      hold = subject;
      subject = extra_subject;
      copy_poll(YES);
      pet_vote = malloc(sizeof(int) * no_pet_votes);
      if (have_i_voted(p_item_copy+1) != NO)
	{
	  if (pet_vote != NULL)
	    {
	      for (i = 1; i <= no_pet_votes ; i++)
		{
		  get_mail_stats((p_item_copy+i)->dropping_id,
				 &readers, vote_str, result);
		  pet_vote[i-1] = atoi(vote_str);
		}
	    }
	}
      check(send_vote(p_item_copy + 1, READ, &old_vote));
      subject = hold;
      petition = YES;
      copy_poll(YES);
    }
  strcpy(t_str, time_str);  /* i_am_leaving wipes it out */
  i_am_leaving();
  if (current_action == SIGNER)
    {
      drop_mail_voter(lclist, YES);
    }
  if (!from_owner)
    {
      strcpy(time_str, t_str);  /* for message to user */
      gen_header(SENDER, "Registration Rcpt:", YES);
      time_str[0] = '\0';  /* wipe it out to prevent another i_am_leaving */
      printf("\nYour reservation for %d %s for the Mexico March has"
	     "\nbeen cancelled.\n"
	     "\nIf you have sent in money, please write to:"
	     "\n\n\tmexico-info@deliberate.com"
	     "\n\nto have your money refunded.  You will be charged a $25"
	     "\nprocessing fee.  After October 1, 2001, the fee goes up to $100.\n", pet_vote[0], pet_vote[0] == 1? "person": "people");
      printf("\n\nThe information removed was:\n");
    }
  if (from_owner)
    {
      printf("\n%s has been unregistered for %d people.\nThe text removed was:\n", name, pet_vote[0]);
    }
  drop_signature(mail_voter, stdout, when);
  *pcc = OK;
  return;
}
/************************************************************/
void vitals_store(FILE *fp, SIG *here);
OKorNOT 
write_mexico(FILE *fp, YESorNO show_time, 
	     YESorNO display_date)
{
  int ch;
  char oldch = '\n';
  YESorNO some_message = NO;
  static char first_time[MAX_LINE + 1] = "";
  int fields = 0;
  int blank_lines = 0;
  int spaces = 0;
  int tabs = 0;
  YESorNO line_started = NO;
  int start = sig_start;
  SIG sig;
  if (start == 0)
    start = msg_start;
  if (first_time[0] == '\0')
    {
      read_to_end();
      strcpy(first_time, date_str(now));
      first_time[strlen(first_time) - 1] = '\0';
    }
  if (display_date)
    {
      fprintf(fp,"\n\n- - - %s - - - \n",
	      first_time);
    }
  if (show_time)
    {
      sig.uid = mail_voter;
      sig.tstamp = now;
      vitals_store(fp, &sig);
    }
  else
    {
      fprintf(fp, "\n");
    }
  if (give_email)
    {
      fprintf(fp, "From: %s\n", from);
    }
  fprintf(fp, "No_of_reservations: %d\n", pet_vote[0]);
  fields = print_fields(fp);
  fprintf(fp, "\nPaid: $ 0.00     \n\n");
  for (ch = start; ch <= end_mark ; ch++)
    {
      if (some_message == NO)
	{
	  if (buffer[ch] == '\n')
	    continue;
	}
      if (buffer[ch] != ' ' && buffer[ch] != '\t'
	 && buffer[ch] != '\n')
	{
	  if (blank_lines > 2)
	    blank_lines = 2;
	  while (blank_lines-- > 0)
	    fputc('\n', fp);
	  blank_lines = 0;
	  while (tabs-- > 0)
	    fputc('\t', fp);
	  while (spaces-- > 0)
	    fputc(' ', fp);
	  some_message = YES;
	  line_started = YES;
	}
      else if (buffer[ch] == '\n')
	{
	  spaces = 0;
	  tabs = 0;
	  blank_lines++;
	  line_started = NO;
	}
      else if (line_started == NO && buffer[ch] == ' ')
	{
	  spaces++;
	}
      else if (line_started == NO && buffer[ch] == '\t')
	{
	  tabs++;
	}
      if (line_started == NO)
	continue;
      /*  Here we're checking that the signature delimiter isn't part
	  of the signature file.  If it is, we change the signature 
	  file a tad.  */
      if (ch > start + 5  
	 && strncmp(&buffer[ch-5], "- - - ", 6) == 0)
	buffer[ch] = '.';
      fputc(buffer[ch], fp);
      oldch = buffer[ch];
    }
  if (oldch != '\n')
    fputc('\n', fp);
  if (some_message == NO && fields == 0)
    fprintf(fp,"\n(No signature message sent.)\n");
  return OK;
}
#endif

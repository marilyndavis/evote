/* $Id: help.c,v 1.4 2003/01/15 18:54:09 marilyndavis Exp $ */ 
/**********************************************************
 *	../eVote/src/mail/mailui/help.c
 *    Runs the help facility for eVote/Majordomo.
 *********************************************************
 *    Translations notes.
 *    You must also translate the help files:
 *    LISTDIR/eVote.help and LISTDIR/eVote.poll
 *    and LISTDIR/eVote.petition
 *    You should also change their names to match your
 *    language: LISTDIR/eVote.ayuda  etc. -- or this
 *    translated code won't find them.
 *********************************************************
 *    Copyright (c) 1994...2015 Deliberate.com Patented.
 *    by Marilyn Davis
 **********************************************************/
#include<stdio.h>
#include "mailui.h"
/*********************************************************
 *   Parses the next token for the type of help
 *   requested.  token's is guaranteed not to be EOF.
 *   In other words, another token is waiting.
 **********************************************************/
void
get_help(void)
{
  get_token();          /* message.c */
  if (!same(token, "who") && !same(token,"how"))
    send_help(token, SENDER, "Re:");
  /* who and how requests only require membership and a 
     subject with a poll. */
  if (set_up_eVote() != OK)
    {
      err_need_to_subscribe();
      send_help("help", SENDER, "Error:");
    }
  copy_poll(YES);
  if (dropping_id == 0)
    {
      sprintf(error_msg,"\nA request for help on the \"eVote how\" or \"eVote who\" command can not \nbe processed unless the subject has a poll attached.");
      send_help("help", SENDER, "Error:");
    }
  gen_header(SENDER, "Re:", YES);
  gen_info(token);   /* in queriesm.c */
}
/*********************************************************
 *   Processes the help request and responds.
 *   Reads a help file and performs substitutions
 *   for \whereami and \list.
 *   The help is sent to whom and the subject
 *   prefix is prefix.
 **********************************************************/
void
send_help(char *category, int whom, char *prefix)
{
  char help_file[FNAME];
  int ch;
  FILE* open_file;
  YESorNO dump_mess = NO;
  strcpy(help_file, listdir);
  strcat(help_file, "/eVote.");
  if (strlen(category) > 20)
    {
      category = "help";
    }
  strcat(help_file, category);
  if ((open_file = fopen(help_file,"r")) == NULL)
    {
      strcpy(help_file, listdir);
      strcat(help_file, "/eVote.help");
      if ((open_file = fopen(help_file,"r")) == NULL)
	{
	  gen_header(SENDER | OWNER | ADMIN, "Error:", YES);
	}
    }
  gen_header(whom, prefix, YES);
  printf("\nIn response to your message:\n");
  print_tokens(NO);
  printf("\n----\n");
  if (error_msg[0] != '\0')
    {
      printf("%s", error_msg);
      error_msg[0] = '\0';
      dump_mess = YES;
      if (open_file != NULL)
	printf("\nPerhaps eVote's instructions will help:\n");
    }
  if (open_file == NULL)
    {
      fprintf(stderr, "\nCan't open eVote's help file, %s.  A message requiring the help file\nis being bounced. listdir = %s\n", help_file, listdir);
      dump_message(stderr, NO, NO, NO);
      printf("\neVote's help file is not available.  \nPlease try your message later.\n");
      printf("\n%s-owner is being notified of the problem.\n", list);
      printf("\nYour unprocessed message follows:\n\n");
      big_finish(5);
    }
  while ((ch = fgetc(open_file)) != EOF)
    {
      if (ch == '\\')
	{
	  int i = 0;
	  char buf[10];
	  for (i = 0; i < 8; i++)
	    {
	      if ((ch = fgetc(open_file)) == EOF)
		{
		  fprintf(stderr, "In the eVote help file, %s, don't put any decorative back-slashes, '\\', \nwithin 10 characters of the end of the file.\n",
			  help_file);
		  printf("\neVote's help file is jumbled.\n");
		  printf("\nPlease try again later.\n");
		  finish(1);
		}
	      if (ch == '\\')
		{
		  ungetc(ch, open_file);
		  break;
		}
	      buf[i] = ch;
	    }
	  if (samen(buf, "list", 4))
	    {
	      int j;
	      printf("%s", list);
	      for (j = 4; j < i; j++)
		putchar(buf[j]);
	    }
	  else if (i == 8 && strncmp(buf, "whereami", 8) == 0)
	    {
	      printf("%s", whereami);
	    }
	  else
	    {
	      buf[i] = '\0';
	      putchar('\\');
	      printf("%s", buf);
	    }
	  continue;
	}
      putchar(ch);
    }
  if (dump_mess == YES)
    {
      printf("\nYour unprocessed message follows:\n\n");
      big_finish(0);
    }
  finish(0);
}


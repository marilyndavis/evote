/* $Id: signal.c,v 1.3 2003/01/15 18:54:09 marilyndavis Exp $ */ 
/* signal.c -- functions to react to a caught signal.
 *********************************************************
 *    Copyright (c) 1994...2015 Deliberate.com Patented.
 *    by Marilyn Davis
 **********************************************************/
#define EMERGENCY_FILE "/tmp/em.tmp"
#define EMERGENCY_TO "root owner-majordomo"
#include<stdio.h>
#include<signal.h>
#include"mailui.h"
#define TIME_OUT 0  /* was 250 */
/************************************************************
 *       Mails an emergency message. 
 *       stdout is already closed.
 *       message appears in the subject and the message
 *       body of the emergency email.
 *       cat_file is concatonated into the mail if it's
 *       not NULL.
 *************************************************************/
void
emergency(char * message, char * cat_file)
{
  FILE *fe;
  char command[300];
  if ((fe = fopen(EMERGENCY_FILE,"w")) == NULL)
    {
      fprintf(stderr,"\nCan't send emergency message.\n");
      fprintf(stderr,"%s: %s", message, error_msg);
      return;
    }
  fprintf(fe,
	  "%s%s for %s.\n",
	  error_msg,  message, subject);
  fclose(fe);
  if (cat_file != NULL)
    {
      sprintf(command,"cat %s >> %s", cat_file, EMERGENCY_FILE);
      system(command);
    }
  if ((fe = fopen(EMERGENCY_FILE,"a")) == NULL)
    {
      fprintf(stderr,"\nCan't send emergency message.\n");
      fprintf(stderr,"%s: %s", message, error_msg);
      return;
    }
  dump_message(fe, NO, NO, NO);
  fclose(fe);
  sprintf(command,"mail -s \"Trouble for '%s' %s\" %s < %s",
	  subject,	message, EMERGENCY_TO, EMERGENCY_FILE);
  system(command);
  sprintf(command, "rm %s", EMERGENCY_FILE);
  system(command);
}
/********************************************************
 *     Caught signals come here.
 *************************************************************/
void
react_to_signal(int signo)
{
  static int count = 0;
  char *signame;
  if (++count > 1)
    exit(1);
  signal(signo, react_to_signal);
  signame = set_signal_name(signo);
  unlock_to_tmp(YES);
  sprintf(error_msg, "Signal %d:%s received.\n", signo, signame);
  fprintf(stderr, "Signal %d:%s received.\n", signo, signame);
  if (header_done)
    {
      printf("\nNOTE: %seVote is experiencing some trouble.  \nPlease check that your command was processed as expected.\n", error_msg);
      big_finish(-2);
      fflush(stdout);
      fclose(stdout);
    }
  dump_message(stderr, NO, NO, NO);
  emergency("PROCESS FAILED", NULL);
  exit(1);
}

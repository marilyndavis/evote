/* $Id: list.c,v 1.3 2003/10/17 23:07:49 marilyndavis Exp $ */ 
/**********************************************************
 *   list.c  Functions that relate to the listserver.
 *      Also, ../Clerklib/start.c has 2 functions that
 *      are listserver-dependent: find_default() and
 *      eVote_cf_path().
 *********************************************************
 *    Copyright (c) 1994...2015 Deliberate.com Patented.
 *    by Marilyn Davis
 **********************************************************/
#define MAILMAN_BIN "/home/mailman/bin"  /* handy for testing */
#define RELATIVE_MAILMAN_BIN "../bin"
#ifndef W_BIN  /* Bin with mailman scripts: Maybe /home/mailman/scripts*/
#define W_BIN "."
#endif
#include<stdio.h>
#include<stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include<errno.h>
#include<time.h>
#include<string.h>
#include "../mailui/mailui.h"
char* listdir = NULL; 
#define MAX_PASSWD  200  /* max length of majordomo password */
static char ** collect_mailman(int *p_no_line);
static char* find_cheat_list(int to_start, YESorNO sub);
static void free_up(char ** these, int no);
static char * find_mailman_script_path(void);
static OKorNOT milk_mailman(char * command_line, int skip_lines);
static void no_space(void);
/**********************************************************
 *    Adds the whole_line or name to the majordomo list
 *    for list_name.  It adds the whole_line if the other
 *    entries have more than just the address, it adds a
 *    comma if the other entries have a comma.
 *    Called by mailui/voterm.c:vacation() and adjust_reader_list()
 *    Should not delete in mailman!
 **********************************************************/
void
add_reader(char *name, char * whole_line, char *list_name)
{
  return;
}
/************************************************************/
void
bounce_drop_error(void) /* don't do it! */
{
  exit(0);
}
/************************************************************
 *  Called by listm.c:start_list and sync_list
 *************************************************************/
OKorNOT
check_reader_files(void)
{
  char line[MAX_LINE + 1];
  char answer[MAX_LINE + 1];
  OKorNOT cc = NOT_OK;
  
  if (milk_mailman("list_lists", 0) != OK 
      || (fgets(answer, MAX_LINE, stdin) == NULL))
    {
      sprintf(error_msg, "Can't milk mailman.list_lists for %s.\n", lclist);
      bounce_error(SENDER | ADMIN | OWNER);
    }
  while(fgets(line, MAX_LINE, stdin) != NULL)
    {
      char list[MAX_LINE + 1];
      sscanf(line, "%s", list);
#ifdef MAILMAN_DEBUG
      fprintf(stderr,"%s\n", line);
#endif
      if (samen(lclist, list, strlen(lclist)))
	{
	  cc = OK;
	}
    }
  if (cc != OK)
    {
      return cc;
    }
  if (make_poll_dir(lclist) == NOT_OK)
    {
      return UNDECIDED;
    }
  return OK;
}
/************************************************************
 *  collects the output from mailman calls
 ************************************************************/
#define BUNCH 5
char **
collect_mailman(int *p_no_lines)
{
  char ** line;
  int no_lines = 0, space;
  char buffer[MAX_LINE];

  if ((line = malloc(sizeof(char*) * (space = BUNCH))) == NULL)
    no_space();
  while (fgets(buffer, MAX_LINE, stdin) != NULL)
    {
      if ( no_lines == space )
	{
	  if((line = realloc(line, (space += BUNCH) * sizeof(char*)))
	     == NULL)
	    no_space();
	}
      if ((line[no_lines] = malloc(strlen(buffer) + 1)) == NULL)
	no_space();
      strcpy(line[no_lines++], buffer);
#ifdef MAILMAN_DEBUG
      fprintf(stderr,"%s\n", buffer);
#endif
    }
  *p_no_lines = no_lines;
  return line;
}
/**********************************************************
*  returns the line from the majordomo file 
*  In mailman this should not drop the reader!
************************************************************/
char *
drop_reader(char *was, char *list_name)
{
  return NULL;
}
/************************************************************/
void
err_need_to_subscribe(void)
{
  sprintf(error_msg, "\nYou can't participate in the %s list until you \n"
	  "have subscribed to it.\n\n"
	  "To subscribe, send email to %s-request@%s.  No \n"
	  "subject is necessary; the message should say \n\n"
	  "subscribe \n\n ",
	      list, list, whereami);
}

/************************************************************
 *        This function deletes all the addresses in the
 *        bounces list that are old days old.  It drops
 *        them from all the lists on the site, both
 *        the majordomo versions and the eVote versions.
 *        We leave bouncing up to mailman.
 *************************************************************/
int
expire_bounces(long old)
{
  return 0;
}
/************************************************************
 *  This reads the list from the To: header 
 *   It is called whenever the list wasn't found in the
 * 	 command line arguments -- must be a sub 
 *   Or it is called to determine the petition address,
 *   i.e. the address that a petition signature was sent to.
 *************************************************************/
char *
find_cheat_list(int to_start, YESorNO sub)
{
  static char cheat_list[MAX_LINE + 1];
  int i = to_start-1;
  int j = 0;
  int len;
  while (buffer[++i] && buffer[i] != '\n' &&
	 (buffer[i] == ' ' || buffer[i] == '\t' ))
    ;
  if (buffer[i] == '<')
    i++;
  i--;
  while (buffer[++i] && buffer[i] != '\n' 
	&& buffer[i] != ' ' && buffer[i] != '\t'
	&& buffer[i] != '@')
    {
      cheat_list[j++] = buffer[i];
    }
  cheat_list[j] = '\0';
  if (sub && (len = strlen(cheat_list)) > 9 &&
     strcmp("-approval", &cheat_list[len - 9]) == 0)
    {
      cheat_list[len - 9] = '\0';
    }
  if (sub && strcmp(cheat_list, "eVote-notify") == 0) /*mailman*/
    {
      for (i = strlen(subject);i >=0 ; i--)
	{
	  if (subject[i] == ' ')
	    {
	      strcpy(cheat_list, &subject[i+1]);
	      return cheat_list;
	    }
	}
    }
  return cheat_list;
}
/************************************************************
 *  This finds Mailman's bin directory
 */
char *
find_mailman_script_path(void)
{
  static char path[PATHLEN];
  struct stat statbuf;
#ifdef MAILMAN_DEBUG
  FILE *fp;

  fp = fopen("/tmp/debug_list", "a");
#endif

  if (path[0])
    {
#ifdef MAILMAN_DEBUG
      fprintf(fp, "Returning: %s\n", path);
      fclose(fp);
#endif
      return path;
    }
  strcpy(path, find_default("PROGS", NO, NO));

  if (path[strlen(path)-1] == '/')
    path[strlen(path)-1] = '\0';

#ifdef MAILMAN_DEBUG
  fprintf(fp, "statting %s\n", path);
#endif
  if (stat(path, &statbuf) != 0)
    {
      char path2[PATHLEN + 1];
      strcpy(path2, path);
      strcpy(path, W_BIN);
      if (stat(path, &statbuf) != 0)  /* no file */
	{
	  fprintf(stderr,"\nPlease put the path to your Mailman scripts\nin the eVote.cf file:\n\n"
		  "\tPROGS = /home/mailman/scripts\n\n"
		  "\tor whatever is correct for your mailman setup. \n"
		  "\teVote looked in\n\n\t%s\n\n\tand %s.\n\n", 
		  path2, path);
	  dump_message(stderr, NO, NO, NO);
	  exit(2);
	}
    }
  strcat(path,"/");
#ifdef MAILMAN_DEBUG
  fprintf(fp, "Now returning %s.\n", path);
  fclose(fp);
#endif
  return path;
}
/************************************************************
 *  Called by message/send.c to find the path to the next
 *  program in the argument list -- this is built into mail-wrapper */
void
find_path(char *path)
{
  struct stat statbuf;
  strcpy(path, argv_copy[1]);
  if (stat(path, &statbuf) != 0)
    {
      fprintf(stderr, "Can't find python path.\n");
      fprintf(stderr, "Tried %s, which came in from $config/mail/wrapper.\n",
	      argv_copy[1]);
      dump_message(stderr, NO, NO, NO);
      exit(2);
    }
}
/************************************************************/
void
free_up(char ** line, int no_lines)
{
  int i;
  for (i = 0; i < no_lines; i++)
    {
      free(line[i]);
    }
  free(line);
}
/**********************************************************
 *         Returns the value of looking_for in the
 *            ../listdir/list_name.config file
 *   looking_for is "subject_prefix" or "approved_passwd"
 *   For mailman, it's only "subject_prefix".
 **********************************************************/
char
*get_config(char * looking_for)
{
  static char answer[MAX_LINE +1];

  if (strcmp(looking_for, "subject_prefix") != 0)
    {
      sprintf(error_msg, "Unexpectedly in mailman:\n"
	      "list.c:get_config called on %s.\n", looking_for);
      bounce_error(SENDER | ADMIN | OWNER);
    }
  sprintf(answer, "withlist -r eVote_queries.PrintSubjectPrefix %s", lclist);
  if (milk_mailman(answer, 3) != OK 
      || (fgets(answer, MAX_LINE, stdin) == NULL))
    {
      sprintf(error_msg, "Can't determine subject_prefix for %s from mailman.withlist.\n", lclist);
      bounce_error(SENDER | ADMIN | OWNER);
    }
  while (fgets(answer, MAX_LINE, stdin))
    {
      if (!(samen(answer, "Running", 7) 
	    || samen(answer, "Finalizing", 10)
	    || samen(answer, "Importing", 9)
	    || samen(answer, "Loading", 7)))
	break;
    }
  answer[strlen(answer)-1] = '\0';
  return answer;
}
/**********************************************************
 * parses the command line arguments for the list name.  
 * If that fails, it finds the list from the To: line.
 **********************************************************/
int
get_list(int argc, char* argv[], YESorNO * sub)
{
  /* get_list needs to happen before get_subject */
  int i;
  *sub = NO;
  if (argc > 4)
    {
      for (i = 1; i < argc; i++)
	{
	  if (same(argv[i], "-S"))
	    {
	      if(i + 2 < argc)
		{
		  list = argv[i+2];
		  break;
		}
	    }
	}
      if (list == NULL)
	{
	  fprintf(stderr,"\nThe list name should appear "
		  "in the alias for the list's address.\n");
	  sprintf(error_msg, 
		  "Something is wrong with the set up of this list.\n");
	  strcat(error_msg, "Please try later.\n");
	  return (SENDER | ADMIN | OWNER);
	}
    }
  else
    {
      int i, j;
      *sub = YES;  
      /* look for incoming mail from myself that should be dropped.
	 Error messages sent to the list-approval address will be
	 aliased to here too and we should ignore them. */

      for (i = strlen(from) -1, j = strlen(whereami)-1; 
	  i >= 0 && j >= 0; i--, j--)
	{
	  if (whereami[j] != from[i])
	    break;
	}
      if (j == -1)  /* matched the domain name! 
		      i should point to the '@' */
	{
	  if (strncmp(&from[i-6], "-eVote@", 7) == 0)
	    {  /* bingo!  drop it */
	      exit(0);
	    }
	}
    }
  /* subscription requests don't have the list in the args */
  if (list == NULL || list[0] == '\0')
    {
      list = find_cheat_list(to_start, YES);
    }
  make_lclist();
  if (strncmp(lclist,"petition", 8) == 0)
    {
      petition = MAYBE;
      if (*sub == NO)
	{
	  petition_address = find_cheat_list(to_start, NO);
	  if (!same(petition_address, list))
	    petition = YES;
	}
    }
  return PASS;
}
/**********************************************************
 *      Returns the name of each list kept by the listserver,
 *      iterating one at a time.  Usually call with finished
 *      == NO, but call an extra time when you are finished
 *      with finished == YES to reset.  If it passes back
 *      a NULL, it has reset itself and you don't need to
 *      call with finished == YES.
 **********************************************************/
char
*get_next_list(YESorNO finished)
{
  static char ** give;
  static int no_lists = 0;
  static int gave = 0;
  static char answer[MAX_LINE];

  if (finished || (no_lists > 0 && gave == no_lists))
    {
      if (no_lists > 0)
	free_up(give, no_lists);
      gave = 0;
      no_lists = 0;
      return NULL;
    }
  if (no_lists == 0)  /* set up the list of lists */
    {
      if (milk_mailman("list_lists", 1) != OK)
	{
	  sprintf(error_msg, "Can't run mailman.list_lists.\n");
	  bounce_error(SENDER | ADMIN | OWNER);
	}
      give = collect_mailman(&no_lists);
    }
  sscanf(give[gave++], "%s", answer);
  return answer;
}
/**********************************************************
 *      Returns the name of each member in the listserver's
 *      version of the list, iterating one at a time.
 **********************************************************/	
char*
get_next_name(char * list_name, YESorNO finished,
		    char ** whole_line)
{
  static char ** line;
  static int no_lines = 0;
  static char name[MAX_LINE + 1];
  static char nomail[MAX_LINE + 1];
  static int gave;
  int cc;

  if (finished || (no_lines > 0 && gave == no_lines))
    {
      if (no_lines > 0)
	free_up(line, no_lines);
      no_lines = 0;
      gave = 0;
      return NULL;
    }
  if (no_lines == 0) /* set up the list of names */
    {
      sprintf(name, "withlist -r eVote_queries.ListMembers %s", lclist);
      if (milk_mailman(name, 4) != OK)
	{
	  sprintf(error_msg, "Can't find members for %s from mailman.list_members.\n", lclist);
	  bounce_error(SENDER | ADMIN | OWNER);
	}
      line = collect_mailman(&no_lines);
    }
  do
    {
      if (gave == no_lines)
	{
	  free_up(line, no_lines);
	  no_lines = 0;
	  gave = 0;
	  return NULL;
	}	  
      cc = sscanf(line[gave++],"%s %s", name, nomail);
    }
  while (cc != 2 || same(name, "Finalizing"));

  *whole_line = nomail;
  return name;
}
/**********************************************************
 * called by mail/mailui/listm.c:bounce_them
 * Not useful in mailman
 ************************************************************/
YESorNO
is_listed(char *name, char *list_name)
{
  sprintf(error_msg,"Unexpected call to is_listed on %s, %s\n",
	  name, list_name);
  bounce_error(SENDER|ADMIN|OWNER);
  return NO;
}
/************************************************************
 *  returns YES if pw is either the list admin password or the
 *  or the site password
 ************************************************************/
YESorNO
is_password(char * pw)
{
  char line[MAX_LINE];

  sprintf(line, "withlist -r eVote_queries.IsAdminPassword %s %s", lclist, pw);
  if (milk_mailman(line, 4) != OK 
      || (fgets(line, MAX_LINE, stdin) == NULL))
    {
      sprintf(error_msg, "Can't determine passwords for %s from mailman.withlist.\n", lclist);
      bounce_error(SENDER | ADMIN | OWNER);
    }
  if(same("yes\n", line))
    {
      return YES;
    }
  return NO;
}
/**********************************************************/
int
move_reader(char *was, char *is)
{
  char owns[MAX_ADDRESS + 1];
  char time_s;
  char * really_from;
  char line[MAX_LINE + 1];
  char list_name[MAX_LINE + 1];
  int found = 0;
  time_s = time_str[0];
  really_from = from;

  sprintf(line, "eVote_queries.py  %s %s", was, is);
  if (milk_mailman(line, 0) != OK)
    {
      sprintf(error_msg, "Couldn't milk mailman.clone_member to move from %s to %s.\n", was, is);
      bounce_error(ADMIN|OWNER);
    }
  while (fgets(line, MAX_LINE, stdin) != NULL)
    {
      if (strstr(line, "processing mailing list:"))
	{
	  strcpy(list_name, &line[25]);
	  list_name[strlen(list_name) - 1] = '\0';
	  continue;
	}
      if (strstr(line, "lone address added") != NULL)
	{
	  found++;
	  sprintf(owns,"owner-%s", list_name);
	  /* jimmying 'from' so that gen_header(SENDER) will go to
	     the list owner  */
	  list = list_name;
	  from = owns;
	  time_str[0] = '\0';
	  header_done = NO;
	  gen_header(SENDER, "Re:", NO);
	  printf("\nOn the %s list, %s \nhas been moved to %s \nby %s.\n",
		 list_name, was, is, really_from);
	  fflush(stdout);
	}
    }
  time_str[0] = time_s;
  from = really_from;
  return found;
}
/************************************************************
 *  Forks and execs a call to mailman.withlist.eVote_queries with the
 *  command_line arranged as the argv's.  Copies this processes env.
 *  It reads the first few lines of output overhead from the call
 *  and is ready for the caller to read stdin for the answer.
 *************************************************************/
OKorNOT
milk_mailman(char * command_line, int skip_lines)
{
  char line[MAX_LINE];
  int pfd[2];
  char *argv[20];
  int i, len;
  char * path;

#ifdef MAILMAN_DEBUG
  FILE * fp;  /* for debug */
  fprintf(stderr,"In milk_mailman\n");
  fp = fopen("/tmp/debug_parent","a");

  fprintf(fp,"milk_mailman called with %s %d\n", command_line, skip_lines);
#endif

  path = find_mailman_script_path();
  for (i = 0; 1 ; i++)
    {
      if(sscanf(command_line, "%s", line) != 1)
	break;
      len = (strlen(line) + 1+ (i==0 ? strlen(path)+7 : 0));
      argv[i] = malloc(len);
      if (i == 0)
	{
	  sprintf(argv[0], "%s../bin/%s", path, line);
	}
      else
	{
	  strcpy(argv[i], line);
	}
      while(*command_line && !IS_WHITE(*command_line))
	command_line++;
      while (command_line[0] == ' ')
	command_line++;
    }
  argv[i] = NULL;

#ifdef MAILMAN_DEBUG
  fprintf(fp, "Call will be %s", argv[0]);
  i = 0;
  while(argv[++i])
    {
      fprintf(fp," argv[%d] = %s", i, argv[i]);
    }
  fprintf(fp,"\n");
#endif

  if (pipe(pfd) == -1)
    {
      fprintf(stderr, "\nCan't establish stdout pipe.\n");
#ifdef MAILMAN_DEBUG
      fclose(fp);
#endif
      return NOT_OK;
    }
#ifdef MAILMAN_DEBUG
  fprintf(fp, "Pipe established.\n");
#endif

  switch (fork())
    {
    case -1:
      fprintf(stderr,"\nCan't fork a new process.\n");
#ifdef MAILMAN_DEBUG
      fclose(fp);
#endif
      return NOT_OK;
    case 0:  /* child */
#ifdef MAILMAN_DEBUG
      fp = fopen("/tmp/debug_child","a");
      fprintf(fp,"Child process starting:\n");
#endif
      if (close(1) == -1)  /* close stdout */
	{
	  fprintf(stderr,"\nCan't close stdout of child process.\n");
#ifdef MAILMAN_DEBUG
      fclose(fp);
#endif
	  return NOT_OK;
	}
#ifdef MAILMAN_DEBUG
      fprintf(fp,"Closed stdout of child.\n");
#endif
      if (dup(pfd[1]) != 1) /* attach stdout to write of pipe */
	{
	  fprintf(stderr,"\nCan't attach child's stdout to pipe.\n");
#ifdef MAILMAN_DEBUG
      fclose(fp);
#endif
	  return NOT_OK;
	}
#ifdef MAILMAN_DEBUG
      fprintf(fp,"\nAttached child's stdout to pipe.\n");
#endif
      if (close(2) == -1)  /* close stderr */
	{
	  fprintf(stderr,"\nCan't close stderr of child process.\n");
#ifdef MAILMAN_DEBUG
      fclose(fp);
#endif
	  return NOT_OK;
	}
#ifdef MAILMAN_DEBUG
      fprintf(fp,"\nClosed stderr of child process.\n");
#endif
      if (dup(pfd[1]) != 2) /* attach stderr to write of pipe */
	{
	  fprintf(stderr,"\n%s: Can't attach child stderr to pipe.\n", 
		  strerror(errno));
#ifdef MAILMAN_DEBUG
      fclose(fp);
#endif
	  return NOT_OK;
	}
#ifdef MAILMAN_DEBUG
      fprintf(fp,"Attached child stderr to pipe.\n");
#endif
      close(pfd[0]);  /* close extra descriptors */
      close(pfd[1]);
#ifdef MAILMAN_DEBUG
      fprintf(fp,"extra descriptors closed.\n");
      fprintf(fp, "Calling %s", argv[0]);
      i = 0;
      while(argv[++i])
	{
	  fprintf(fp," argv[%d] = %s", i, argv[i]);
	}
      fprintf(fp,"\n");
      fflush(fp);
      fclose(fp);
#endif
      execve(argv[0], argv, env_copy);
      fprintf(stderr, "Call to %s didn't work: %s.\n",
	      argv[0], strerror(errno));
#ifdef MAILMAN_DEBUG
      fclose(fp);
#endif
      return NOT_OK;
    }
  /* parent */

#ifdef MAILMAN_DEBUG
  fprintf(fp, "Parent:\n\nCalling %s", argv[0]);
  i = 0;
  while(argv[++i])
    {
      fprintf(fp," argv[%d] = %s", i, argv[i]);
    }
  fprintf(fp,"\n");
  fflush(fp);
#endif

  i = 0;
  while(argv[i])
    {
      free(argv[i++]);
    }

  if (close(0) == -1)  /* close stdin of parent */
    {
      fprintf(stderr,"\nCan't close stdin for ipcs.\n");
#ifdef MAILMAN_DEBUG
      fclose(fp);
#endif
      return NOT_OK;
    }
#ifdef MAILMAN_DEBUG
  fprintf(fp,"stdin of parent closed.\n");
#endif
  if (dup(pfd[0]) != 0)  /* attach reading end to stdin */
    {
      fprintf(stderr,"\nCan't attach pipe to stdin.\n");
#ifdef MAILMAN_DEBUG
      fclose(fp);
#endif
      return NOT_OK;
    }
#ifdef MAILMAN_DEBUG
  fprintf(fp,"pipe reading attached to stdin of parent.\n");
#endif

  if (close(pfd[1]) == -1 || close(pfd[0]) == -1 )
    {
      fprintf(stderr,"\nCan't close extra pipe pfd[]s for parent.\n");
#ifdef MAILMAN_DEBUG
      fclose(fp);
#endif
      return NOT_OK;
    }
#ifdef MAILMAN_DEBUG
  fprintf(fp,"extra descriptors closed in parent.\n");
#endif

  for (i = 0; i < skip_lines; i++)
    {
      if (fgets(line,MAX_LINE,stdin) == NULL)
	{
	  sprintf(error_msg, "There aren't %d lines to milk with your call to mailman:\n"
		  "%s\n", skip_lines, command_line);
	  bounce_error(SENDER | ADMIN | OWNER);
	}
#ifdef MAILMAN_DEBUG
      fprintf(fp,"Read line %d: %s", i, line);
#endif
    }
#ifdef MAILMAN_DEBUG
  fprintf(fp, "Read %d lines ok and returning from milk_mailman.\n", 
	  skip_lines);
  fclose(fp);
#endif
  return OK;
}
/************************************************************/
void
no_space(void)
{
  sprintf(error_msg, "No memory for processing.\n");
  bounce_error(ADMIN);
}
/************************************************************
 *  We'll leave mailman to process vacation requests --
 *  It's the "nomail" feature. */
void
process_vacation(YESorNO on)
{
  sprintf(error_msg, "\nTo %s vacation, please use your mailman configuration web page \nto turn \"nomail\" to \"%s\".  Or:\n\n"
	  "1.  Write to %s-request@%s\n\n"
	  "2.  The subject doesn't matter.\n\n"
	  "3.  Your message should say:\n\n"
	  "      set nomail %s <password>\n\n"
	  "--- end of message ---\n\n",
	  on ? "go on" : "come back from", on ? "on" : "off",
	  lclist, whereami, on ? "on" : "off");
  bounce_error(SENDER);
}
/*********************************************************
 *      When the listserve sends a notice to the list's
 *      owner that someone has subscribed or unsubscribed,
 *      the message also comes to eVote_insert and eVote adds 
 *      the new person to its list.
 *      -r option enables registration by making the 
 *         ballot read_only.
 *********************************************************/
#define WHAT_LEN 15
void
subs(int argc, char* argv[])
{
  char dowhat[WHAT_LEN + 1];
  int len, dummy;
  char *address;
  int i;
  ACTION action = EVERYTHING;

  if (argc > 1 && same(argv[1], "-r"))
    action = READ_ONLY;

  /*  for (i = 0; i < argc; i++)
      fprintf(stderr, "argv[%d] = %s\n", i, argv[i]);*/
  for (i = 0; subject[i] != ' ' && subject[i] != '\0'; i++)
    {
      if (i >= WHAT_LEN)
	break;
      dowhat[i] = subject[i];
    }
  dowhat[i] = '\0';
  if (strcmp(dowhat, "SUBSCRIBE") != 0 
      && strcmp(dowhat, "VACATION") != 0
      && strcmp(dowhat, "UNSUBSCRIBE") != 0)  /* nothing to do */
    {
      exit(0);
    }
  if (list == NULL || same(list, "eVote-notify"))
    {
      len = strlen(&subject[i+1]) + 1;
      if ((list = malloc(len)) == NULL)
	{
	  fprintf(stderr,"\nCan't allocate space for list name.\n");
	  if (error_msg[0] == '\0')
	    {
	      sprintf(error_msg, "\n%s's host, %s, is out of resources right now.\nPlease try your eVote command again later.\n", 
		      (list == NULL ? "unknown" : list),
		      (whereami == NULL ? "unknown" : whereami));
	    }
	  bounce_error(OWNER | ADMIN);
	}
      strcpy(list, &subject[++i]);  /* expects global list */
      for (i = 0; list[i]; i++)
	{
	  if (list[i] == ' ')
	    {
	      list[i] = '\0';
	      break;
	    }
	}
    }
  if (strncmp(list,"petition", 8) == 0)
    petition = YES;
  if ((address = get_guy(msg_start, &dummy)) == NULL)
    {
      gen_header(OWNER, "Re:", NO);
      if (strcmp(dowhat, "SUBSCRIBE") == 0 )
	{
	  printf("eVote can't find a valid address here:\n\n%.80s\n\nYou should unsubscribe it from the list.\n", &buffer[msg_start]);
	}
      else
	{
	  printf("This is eVote acknowledging the deletion of the bad address in this:\n\n%.80s\n\nThanks.\n", &buffer[msg_start]);
	}
      big_finish(0);
    }
  else
    {
      if (strcmp(dowhat, "VACATION") == 0)
	{
	  int on;
	  sscanf(&buffer[msg_start], "%*s %d",  &on);
	  action = on ? VACATION : EVERYTHING;
	}
      confirm_this.status = VERIFIED;
      sub_to_eVote(address, dowhat, action);
      /* Only subscriptions return */
      if (strcmp(dowhat, "SUBSCRIBE") == 0 )
	{
	  gen_header(OWNER, "Re:", NO);
	  printf("eVote has made a ballot for %s's new subscription.\n",
		 address);
	  big_finish(0);
	}
    }
  exit(0);
}
/************************************************************/
void
vacation_back_msg(char * guy)
{
  confirm_this.status = VERIFIED;
  on_vacation(NO);
  
  /*  sprintf(error_msg,"\n%s's subscription to %s is redundant."
	  "\nHowever, %s has 'nomail' turned on.  To fix this:\n\n"
	  "1.  Write to %s-request@%s\n\n"
	  "2.  The subject doesn't matter.\n\n"
	  "3.  Your message should say:\n\n"
	  "      set nomail off <password>\n\n"
	  "--- end of message ---\n",
	  guy, lclist, guy, lclist, whereami);*/
}
/************************************************************/
void
vacation_msg(void)
{
      printf("\nThank you, %s, for your 'nomail' request to the \n%s list.\n",
	     from, list);
      printf("\nYour command was processed successfully.  You will not receive");
      printf("\nmail and your ballot is being held for you by eVote.\n");
      printf("\nWhen you return, change youir 'nomail' option to OFF.\n");
      printf("\nHave a nice vacation.\n");
}
/************************************************************/
void
vacation_redundant_msg(YESorNO going)
{
  printf("\neVote thinks your 'nomail' feature was already %s.\n", 
	 (going ? "on" : "off"));
}
/************************************************************
 *  Don't allow people on vacation to vote.  They can't see
 *  their receipt so it's not secure */
void
vacation_voter(void)
{
  sprintf(error_msg, "\nYou must change the \"nomail\" option on your Mailman page \n"
	  "to \"off\" before you can vote.  Or:\n\n"
	  "1.  Write to %s-request@%s\n\n"
	  "2.  The subject doesn't matter.\n\n"
	  "3.  Your message should say:\n\n"
	  "      set nomail off <password>\n\n"
	  "--- end of message ---\n",
	  lclist, whereami);
}


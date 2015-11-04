/* $Id: list.c,v 1.2 2003/01/31 21:32:50 marilyndavis Exp $ */ 
/**********************************************************
 *   list.c  Functions that relate to the listserver.
 *      Also, ../Clerklib/start.c has 2 functions that
 *      are listserver-dependent: find_default() and
 *      eVote_cf_path().
 *********************************************************
 **********************************************************/
#include<stdio.h>
#include <dirent.h>
#include<sys/types.h>
#include <sys/stat.h>
#include<errno.h>
#include<time.h>
#include<string.h>
#include<fcntl.h>
#include "../mailui/mailui.h"
char* listdir = NULL; 
#define MAX_PASSWD  200  /* max length of admin password */
static OKorNOT find_address(char *line, char *address);
static char* find_cheat_list(int to_start, YESorNO sub);
static int get_passwds(char ** where[]);
static FILE * open_reader_list(char *list, char * how);
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
  FILE *fp, *fp_tmp;
  int filedes;
  char fname[FNAME + 1];
  char first_line[MAX_LINE + 1];
  int len, i = 0;
  YESorNO comma = NO;
  YESorNO strip = YES;
  if (is_listed(name, list_name))
    return;
  sprintf(fname, "%s/%s", listdir, list_name);
  /* This is eVote's own locking device to be sure eVote doesn't
     mess itself up. */
  if ((fp_tmp = lock_to_tmp(fname)) == NULL)
    {
      char tmp[MAX_LINE];
      strcpy(tmp, error_msg);
      sprintf(error_msg,"\n\nERROR! %s \nCan't add %s to Majordomo's %s list.",
	      tmp, name, list);
      strcat(error_msg,"\nPlease try later.\n");
      bounce_error(SENDER | ADMIN);
    }
  while ((filedes = open(fname, O_RDWR)) == -1)
    {
      if ((fp = open_reader_list(list_name,"rw")) == NULL)
	{
	  unlock_to_tmp(NO);
	  bounce_error(SENDER | ADMIN);
	}
      fclose(fp);
    }
  /* This is a system lock to try to keep Majordomo from messing
     us up. */
  if (lockf(filedes, F_LOCK, 0) != 0)
    {
      int cc;
      switch (cc = errno)
	{
	case EBADF:
	  fprintf(stderr,"\nThis is not a valid open descriptor.\n");
	  break;
	case EDEADLK:
	  fprintf(stderr,"\nThis would create deadlock.\n");
	  break;
	}
      sprintf(error_msg,"\n\nCan't lock Majordomo's list for %s.\n"
	      "Can't add %s.\n",
	      list, name);
      strcat(error_msg,"Please try later.\n");
      unlock_to_tmp(NO);
      bounce_error(SENDER | ADMIN);
    }
  if ((fp = fdopen(filedes, "rw")) == NULL)
    {
      sprintf(error_msg, "\n\nCan't attach a FILE * to %s.\nCan't add %s.\n",
	      fname, name);
      strcat(error_msg,"Please try later.\n");
      unlock_to_tmp(NO);
      bounce_error(SENDER | ADMIN);
    }
  /* study the first line to see if there should be a comma
     at the end of each line */
  if (fgets(first_line, MAX_LINE, fp) != NULL)
    {
      len = strlen(first_line);
      for (i = len - 1; i >= 0; i--)
	{
	  if (first_line[i] == '\n' || first_line[i] == ' '
	     || first_line[i] == '\t')
	    continue;
	  if (first_line[i] == ',')
	    comma = YES;
	  break;
	}
    }
  do  /* study the whole file to see if we're stripping */
    {
      for (i = 0; i < len ; i++)
	{
	  if (first_line[i] == '"' || first_line[i] == '<'
	     || first_line[i] == '(')
	    {
	      strip = NO;
	      break;
	    }
	}
    }
  while (strip == YES 
	&& (fgets(first_line, MAX_LINE, fp) != NULL));
  if (fseek(fp, 0L, SEEK_END) != 0)
    {
      sprintf(error_msg, "\n\nCan't seek end of %s.\nCan't add %s.\nPlease try later.\n",
	      fname, name);
      unlock_to_tmp(NO);
      bounce_error(SENDER | ADMIN);
    }
  if (same(list_name,"bounces"))
    {
      char comment[100];
      /*			time_t now; */
      struct tm * tms;
      int year;
      /*			time(&now); */
      tms = localtime(&now);
      strip = NO;
      comma = YES;
      year = tms->tm_year + 1900;
      sprintf(comment," (%4d/%2d/%2d: %s) ",
	      year, tms->tm_mon + 1, tms->tm_mday, list);
      write(filedes, comment, strlen(comment));
    }
  len = strlen(whole_line);
  while (--len > 0)
    {
      if (whole_line[len] == '\n' || whole_line[len] == '\t'
	 || whole_line[len] == ' ')
	continue;
      if (comma == YES && whole_line[len] != ',')
	{
	  whole_line[len + 1] = ',';
	  whole_line[len + 2] = '\n';
	  whole_line[len + 3] = '\0';
	}
      if (comma == NO && whole_line[len] == ',')
	{
	  whole_line[len] = '\n';
	  whole_line[len + 1] = '\0';
	}
      break;
    }
  if (strip == YES)
    {
      write(filedes, name, strlen(name));
      write(filedes, ",\n", 2);
    }
  else
    write(filedes, whole_line, strlen(whole_line));
  if (lockf(filedes, F_ULOCK, 0) != 0)
    {
      sprintf(error_msg,"\n\nCan't unlock Majordomo's list for %s.\n",
	      list);
      unlock_to_tmp(NO);
      bounce_error(SENDER | ADMIN);
    }			
  close(filedes);
  if (unlock_to_tmp(NO) != OK)
    {
      fprintf(stderr,"\nERROR! Unable to remove tmp file: %sT.\n",
	      fname);
    }			
}
/************************************************************/
void
bounce_drop_error(void)
{
  bounce_error(APPROVAL);
}
/************************************************************
 *  Called by listm.c:start_list and sync_list
 *************************************************************/
OKorNOT
check_reader_files(void)
{
  FILE * fp;

  if ((fp = open_reader_list(lclist, "r")) == NULL
      || make_poll_dir(lclist) == NOT_OK)
    {
      return UNDECIDED;
    }
  fclose(fp);
  return OK;
}
/**********************************************************
*  returns the line from the majordomo file 
*  In mailman this should not drop the reader!
************************************************************/
char *
drop_reader(char *was, char *list_name)
{
  FILE *fp;
  FILE *fp_tmp;
  char stored_name[MAX_ADDRESS + 1];
  static char found_line[MAX_LINE + 1];
  char line[MAX_LINE + 1];
  YESorNO found = NO;
  OKorNOT cc;
  char fname[FNAME +1];
  int filedes;
  sprintf(fname, "%s/%s", listdir, list_name);
  if ((fp_tmp = lock_to_tmp(fname)) == NULL)
    {
      char tmp_msg[MAX_LINE +1];
      strcpy(tmp_msg, error_msg);
      sprintf(error_msg,"\nERROR! %s\nCan't remove %s from %s.\n",
	      tmp_msg, was, list_name);
      bounce_error(SENDER | ADMIN);
    }
  sprintf(stored_name, "cp %s %s.backup_from_eVote", fname, fname);
  system(stored_name);
  if ((filedes = open(fname, O_RDWR)) == -1)
    {
      sprintf(error_msg, "\nCan't open %s to drop %s.\n", 
	      fname, was);
      unlock_to_tmp(NO);
      if (strcmp(list,"bounces"))
	{
	  strcat(error_msg,"If you are using the \"eVote back\" command to return from vacation\n"
		 "your command was successful and you can ignore this error message.\n");
	}
      bounce_error(SENDER | ADMIN);
    }
  if (lockf(filedes, F_LOCK, 0) != 0)
    {
      sprintf(error_msg,"\n\nCan't lock Majordomo's list for %s.\n"
	      "Can't remove %s.\n",
	      list, was);
      strcat(error_msg,"Please try later.\n");
      unlock_to_tmp(NO);
      bounce_error(SENDER | ADMIN);
    }
  if ((fp = fdopen(filedes, "rw")) == NULL)
    {
      sprintf(error_msg, "\n\nCan't attach a FILE * to %s.\nCan't remove %s.",
	      fname, was);
      strcat(error_msg,"\nPlease try later.\n");
      unlock_to_tmp(NO);
      bounce_error(SENDER | ADMIN);
    }
  while (fgets(line, MAX_LINE, fp) != NULL)
    {
      if ((cc = find_address(line, stored_name)) == UNDECIDED)
	continue;
      if (cc != OK)
	{
	  unlock_to_tmp(NO);
	  bounce_error(OWNER | ADMIN);
	}
      if (!same_address(stored_name, was))
	fprintf(fp_tmp, "%s", line);
      else 
	{
	  found = YES;
	  strcpy(found_line, line);
	}
    }
  fflush(fp);
  fflush(fp_tmp);
  sprintf(stored_name, "cp %s %s/%s", tmp_file_name, listdir, list_name);
  if (lockf(filedes, F_ULOCK, 0) != 0)
    {
      sprintf(error_msg,"\n\nCan't unlock Majordomo's list for %s.\n",
	      list);
      unlock_to_tmp(NO);
      bounce_error(SENDER | ADMIN);
    }			
  fclose(fp);
  fp = NULL;
  if (found == YES)
    {
      system(stored_name);
      chmod(fname, 00644);
    }
  if (unlock_to_tmp(NO) != OK)
    {
      fprintf(stderr,"\nERROR! Unable to remove tmp file: %sT.\n",
	      fname);
    }			
  if (found == YES)
    return found_line;
  return NULL;
}

/************************************************************/
void
err_need_to_subscribe(void)
{
      sprintf(error_msg, "\nYou can't participate in the %s list until you \nhave subscribed to it.\n\nTo subscribe, send email to majordomo@%s.  No \nsubject is necessary; the message should say \n\n   subscribe %s\n\n ",
	      list, whereami, list);
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
  char * name;
  char * line;
  /*	time_t now; */
  struct tm tms;
  struct tm * now_tms;
  char keep_name[MAX_ADDRESS + 1];
  char keep_line[MAX_LINE + 1];
  int count = 0;

  subject = "Dropping old bad addresses";
  from = "owner-eVote";
  /*	time(&now); */
  now_tms = localtime(&now);
  if (list == NULL)
    list = malloc(MAX_LINE + 1);
  while ((name = get_next_name("bounces", NO, &line))
	!= NULL)
    {
      sscanf(line," (%4d/%2d/%2d: %s) ",
	     &tms.tm_year, &tms.tm_mon, &tms.tm_mday, list);
      tms.tm_year -= 1900;
      tms.tm_mon--;
      if (diff_time(now_tms, &tms) >= old)
	{
	  if (!count++)
	    {
	      printf("\nDeleting from bounces:\n");
	    }
	  strcpy(keep_name, name);
	  strcpy(keep_line, line);
	  printf("\n%s", keep_line);
	  get_next_name("bounces", YES, &line);
	  mail_voter = who_num(keep_name, NO);
	  if ((mail_voter = who_num(keep_name, NO)) == 0
	     || drop_mail_voter("ALL", NO) == OK)
	    {
	      move_reader(keep_name, NULL);
	    }
	}
    }
  return count;
}
/************************************************************
 *      This finds the email address in the line buffer
 *      and copies it to the address buffer.
 ************************************************************/
OKorNOT
find_address(char *line, char *address)
{
  int i = 0;
  int end, start = -1;
  int j;
  while (line[i] == ' ')
    i++;
  i--;
  while (line[++i] && line[i] != '\n'  && line[i] != '\r')
    {
      switch (line[i])
	{
	case '<':
	  j = 0;
	  while (line[++i] != '>')
	    {
	      if (line[i] == '\"' ||
		 line[i] == '\n' || line[i] == '\r' ||
		 line[i] == '\0'|| line[i] == EOF)
		{
		  sprintf(error_msg, "\nCan't figure email address from this line in %s:\n\n\t%s\n", 
			  list, line);
		  return NOT_OK;
		}
	      address[j++] = line[i];
	    }
	  address[j] = '\0';
	  return OK;
	case '(':
	  while (line[++i] != ')')
	    {
	      if (line[i] == '\n' || line[i] == '\r' ||
		 line[i] == '\0'|| line[i] == EOF)
		{
		  sprintf(error_msg, "\nCan't figure email address from this line in %s:\n\n\t%s\n", 
			  list, line);
		  return NOT_OK;
		}
	    }
	  break;
	case '"':
	  while (line[++i] != '"')
	    {
	      if (line[i] == '\n' || line[i] == '\r' ||
		 line[i] == '\0'|| line[i] == EOF)
		{
		  sprintf(error_msg, "\nCan't figure email address from this line in %s:\n\n\t%s\n", 
			  list, line);
		  return NOT_OK;
		}
	    }
	  break;
	case '@':
	  j = 0;
	  while (line[i] != ' ' && line[i] != '\n' && line[i] != ','
		&& line[i] != '\t' && i > 0)
	    i--;
	  if (i == 0)
	    i = -1;
	  while (line[++i])
	    {
	      if (line[i] == ' ' || line[i] == '\t' || line[i] == ','
		 || line[i] == '\n' || line[i] == '\r')
		break;
	      if (line[i] == '\'' || line[i] == '"')
		{
		  sprintf(error_msg, "\nCan't figure email address from this line in %s:\n\n\t%s\n", 
			  list, line);
		  return NOT_OK;
		}
	      address[j++] = line[i];
	    }
	  address[j] = '\0';
	  return OK;
	case ' ':
	case '\t':
	  break;
	default:
	  if (start == -1)
	    start = i;
	  break;
	}
    }
  /* start over */
  if (start == -1)
    {
      for (i = 0; line[i] != '\n' && line[i] != '\0'; i++)
	{
	  if (line[i] != ' ' && line[i] != '\t')
	    {
	      sprintf(error_msg, "\nCan't figure email address from this line in %s:\n\n\t%s\n", 
		      list, line);
	      return NOT_OK;
	    }
	}
      return UNDECIDED;  /* only white-space on the line */
    }
  end = i;
  j = 0;
  i = start - 1;
  while (++i < end && line[i] != ' ' && line[i] != ','
	&& line[i] != '"' && line[i] != '(')
    {
      address[j++] = line[i];
    }
  address[j] = '\0';
  if (j == 0)
    {
      sprintf(error_msg, "\nCan't figure email address from this line in %s:\n\n\t%s\n", 
	      list, line);
      return NOT_OK;
    }
  return OK;
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
  return cheat_list;
}
/************************************************************
 *  Called by message/send.c to find the path to the next
 *  program in the argument list */
void
find_path(char *path)
{
  struct stat statbuf;

  strcpy(path, find_default("PROGS", NO, NO));
  if (path[strlen(path)-1] != '/')
    strcat(path,"/");
  strcat(path, argv_copy[1]);

  if (lstat(path, &statbuf) != 0)
    {
      char path2[PATHLEN + 1];
      strcpy(path2, path);
      strcpy(path, W_BIN);
      if (path[strlen(path)-1] != '/')
	strcat(path,"/");
      strcat(path, argv_copy[1]);
      if (lstat(path, &statbuf) != 0)  /* no file */
	{
	  fprintf(stderr,"\nPlease put the path to your Majordomo's executables \nin the eVote.cf file:\n\n"
		  "\tPROGS = /usr/local/majordomo\n\n"
		  "\tor whatever.  Looked in\n\n\t%s\n\n\tand %s.\n\n", 
		  path2, path);
	  dump_message(stderr, NO, NO, NO);
	  exit(2);
	}
    }
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
  char line[MAX_LINE + 1];
  static char answer[MAX_LINE +1];
  char * start;
  FILE *fp;
  char fname[FNAME + 1];
  int len, i, j;
  YESorNO found = NO;
  len = strlen(looking_for);
  strcpy(fname, listdir);
  strcat(fname, "/");
  strcat(fname, lclist);
  strcat(fname, ".config");
  if ((fp=fopen(fname,"r")) == NULL)
    {
      sprintf(error_msg,"\nCan't open config file %s.\n", fname);
      return NULL;
    }
  while (fgets(line, MAX_LINE, fp) != NULL)
    {
      for (i = -1; line[++i] == ' ' || line[i] == '\t'; i++)
	;
      if (strncmp(&line[i], looking_for, len) == 0)
	{
	  found = YES;
	  break;
	}
    }
  fclose(fp);
  if (found == NO)
    {
      sprintf(error_msg,"\nCan't find %s in %s.\n", looking_for, fname);
      fprintf(stderr,"%s", error_msg);
      return NULL;
    }
  for (i = len-1; line[i] != '='; i++)
    {
      if (line[i] == '\n')
	return "";
    }
  while (line[++i] == ' ' || line[i] == '\t')
    ;
  if (line[i] == '\n')
    return "";
  start = &line[i];
  for (i = 0, j = 0; start[i] && start[i] != '\n'; i++, j++)
    {
      if (strncmp(&start[i], "$LIST", 5) == 0)
	{
	  strcpy(&answer[j], list);
	  j += strlen(list) - 1;
	  i += strlen("$LIST") -1;
	  continue;
	}
      answer[j] = start[i];
    }
  answer[j] = '\0';
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
  if (argc > 2)
    {
      for (i = 1; i < argc; i++)
	{
	  if (strcmp(argv[i], "-l") == 0)
	    {
	      list = argv[++i];
	      break;
	    }
	}
      if (list == NULL)
	{
	  fprintf(stderr,"\nThe list name should appear as  '-l listname' "
		  "in the alias for \nthe list's address.\n");
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
  static DIR *dirp = NULL;
  struct dirent *dp;
  int len;
  static char name[PATHLEN + 1];
  if (dirp == NULL)
    {
      dirp = opendir(listdir);
      if (dirp == NULL)
	{
	  sprintf(error_msg,"\nYou don't have permission to search Majordomo's data directory.  \nYour move command can't be completed.");
	  bounce_error(SENDER|ADMIN);
	}
    }
  if (finished != YES)
    {
      while ((dp = readdir(dirp)) != NULL)
	{
	  strcpy(name, dp->d_name);
	  len = (int)strlen(name);
	  if (strcmp(name+len-6,"config") != 0)
	    continue;
	  name[len-7] = '\0';
	  return name;
	}
    }
  if (dirp != NULL)
    closedir(dirp);
  dirp = NULL;
  return NULL;
}
/**********************************************************
 *      Returns the name of each member in the listserver's
 *      version of the list, iterating one at a time.
 *      whole_line points to the address line comments in
 *      majordomo.  In mailman, whole_line has the nomail
 *      (vacation) option for the reader : 1 or 0
 *      Usually call with finished
 *      == NO, but call an extra time when you are finished
 *      with finished == YES to reset.  If it passes back
 *      a NULL, it has reset itself and you don't need to
 *      call with finished == YES.
 *      Feature added by Dustin Marquess:  This ignores
 *      addresses that start with a one of the following 
 *      "special" characters:
 *      ~ - Pathname (using sh homedir expansion)
 *      # - Comment
 *      + - List seperator (Used in "+list" to mark the start of addresses)
 *      . - Pathname (relative to current directory)
 *      / - Pathname (absolute)
 *      | - Program pipe
 *      Also addresses that start with an & have the & eaten-up first.  
 *      The main purpose of this patch is to allow eVote to coexist 
 *      peacefully with qmail-extended lists.
 **********************************************************/	
char*
get_next_name(char * list_name, YESorNO finished,
		    char ** whole_line)
{
  static FILE *fp;
  static char line[MAX_LINE + 1];
  static char name[MAX_ADDRESS + 1];
  OKorNOT cc;
  char * return_name = name;
  if (fp == NULL)
    {
      if ((fp = open_reader_list(list_name,"r")) == NULL)
	{
	  bounce_error(SENDER | ADMIN);
	}
    }
  if (finished != YES)
    {
      while (fgets(line, MAX_LINE, fp) != NULL)
	{
	  if ((cc = find_address(line, name)) == UNDECIDED)
	    continue;
	  switch (name[0])
	    {
	    case '~' :
	    case '#' :
	    case '+' :
	    case '.' :
	    case '/' :
	    case '|' :
	      continue;
	    case '&' :
	      return_name++;
	    }
	  if (cc != OK)
	    {
	      if (fp != NULL)
		fclose(fp);
	      fp = NULL;
	      bounce_error(OWNER | ADMIN);
	    }
	  *whole_line = line;
	  return return_name;
	}
    }
  if (fp != NULL)
    fclose(fp);
  fp = NULL;
  return NULL;
}
/**********************************************************
 *         Returns the number of passwords
 *            where returns a list of pointers to the 
 *            passwords.  For majordomo, there are two:
 *            ../lists/list_name.passwd file
 *            ../lists/list_name.config's approve_passwd
 *         Only called by is_passwd() in this file.
 *         Not useful in mailman.
 **********************************************************/
int
get_passwds(char ** where[])  
{
  static char pw[2][MAX_PASSWD + 1] = {"",""};
  static char *points[2] = {pw[0], pw[1]};
  FILE *fp;
  static int found;
  char fname[FNAME + 1];
  int sl;
  char * pconfig;
  *where = points;
  if (pw[0][0] != '\0')
    {
      return found;
    }
  strcpy(fname, listdir);
  strcat(fname, "/");
  strcat(fname, lclist);
  strcat(fname, ".passwd");
  if ((fp=fopen(fname,"r")) == NULL)
    {
      sprintf(error_msg,"\nCan't open passwd file %s.\n", fname);
    }
  else
    {
      fgets(pw[0], 99, fp);
      fclose(fp);
      while ((sl = strlen(pw[0])) != 0 && pw[0][sl-1] == '\n')
	pw[0][sl-1] = '\0';
      found++;
    }
  if ((pconfig = get_config("approve_passwd")) != NULL)
    {
      strcpy(pw[1], pconfig);
      found++;
    }
  return found;
}
/**********************************************************
 * called by mail/mailui/listm.c:bounce_them
 * Not useful in mailman
 ************************************************************/
YESorNO
is_listed(char *name, char *list_name)
{
  char *stored_name;
  char *whole_line;
  while ((stored_name = get_next_name(list_name, NO, &whole_line)) 
	!= NULL)
    {
      if (same_address(name, stored_name))
	{
	  get_next_name(list_name, YES, &whole_line);
	  return YES;
	}
    }
  return NO;
}
/************************************************************
 *  returns YES if pw is either the list admin password or the
 *  approval password.
 ************************************************************/
YESorNO
is_password(char * pw)
{
  int no_passwds, i;
  char ** passwds;

  if ((no_passwds = get_passwds(&passwds)) == 0)
    {
      bounce_error(SENDER | OWNER | ADMIN);
    }
  if (no_passwds == 1)
    {
      fprintf(stderr, error_msg);
      error_msg[0] = '\0';
    }
  for (i=0; i < no_passwds; i++)
    {
      if (same(pw, passwds[i]))
	{
	  return YES;
	}
    }
  return NO;
}
/**********************************************************/
int
move_reader(char *was, char *is)
{
  char *name;
  char *whole_line;
  char owns[MAX_ADDRESS + 1];
  char time_s;
  char * really_from;
  char *list_name;
  int found = 0;
  YESorNO drop = NO;
  if (is == NULL)
    drop = YES;
  time_s = time_str[0];
  really_from = from;
  while ((list_name = get_next_list(NO)) != NULL)
    {
      while ((name = get_next_name(list_name, NO, &whole_line)) 
	    != NULL)
	{
	  if (same_address(name, was))
	    {
	      char line_copy[MAX_LINE +1];
	      char *ptr;
	      found++;
	      /* close this list */
	      get_next_name(list_name, YES, &whole_line); 
	      /* manufacture new sub line */
	      if (drop == NO)
		{
		  ptr = strstr(whole_line, name);
		  *ptr = '\0';
		  strcpy(line_copy, whole_line);
		  strcat(line_copy, is);
		  strcat(line_copy, ptr + strlen(was));
		}
	      if (drop_reader(was, list_name) != NULL)
		{
		  if (drop == NO)
		    add_reader(is, line_copy, list_name);
		  else
		    fprintf(stderr,"\n%s has been dropped from %s.\n",
			    was, list_name);
		}
	      if (!same(list_name, "bounces"))
		{
		  sprintf(owns,"owner-%s", list_name);
		  /* jimmying 'from' so that gen_header(SENDER) will go to
		     the list owner  */
		  list = list_name;
		  from = owns;
		  time_str[0] = '\0';
		  header_done = NO;
		  gen_header(SENDER, "Re:", NO);
		  if (drop == NO)
		    {
		      printf("\nOn the %s list, %s \nhas been moved to %s \nby %s.\n",
			     list_name, was, is, really_from);
		    }
		  else
		    {
		      printf("\n%s has been dropped from %s.\n",
			     was, list_name);
		    }
		  fflush(stdout);
		}
	      break;
	    }
	}
    }
  time_str[0] = time_s;
  from = really_from;
  return found;
}
/**********************************************************
 *  Opens majordomo's file of email names for the list.
 **********************************************************/
FILE *
open_reader_list(char *list, char * how)
{
  char fname[FNAME + 1];
  static FILE *fp;
  int times = 0;
  sprintf(fname, "%s/%s", listdir, list);
  while ((fp = fopen(fname, how)) == NULL)
    {
      if ((fp = fopen(fname, "wr")) == NULL || ++times > 2)
	{
	  sprintf(error_msg,"\nCan't find or open subscription file for %s: \n\t%s\n",
		  list, fname);
	  return fp;
	}
      if (fp != NULL)
	fclose(fp);
      fp = NULL;
      chmod(fname, 00644);
      fprintf(stderr, "\neVote created subscription file %s.\n", fname);
    }
  return fp;
}
/************************************************************
 *  Listserve-dependent feature. */
void
process_vacation(YESorNO on)
{
  on_vacation(on);
}
/*********************************************************
 *      When majordomo sends a notice to the list's
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
  for (i = 0; subject[i] != ' ' && subject[i] != '\0'; i++)
    {
      if (i >= WHAT_LEN)
	break;
      dowhat[i] = subject[i];
    }
  dowhat[i] = '\0';
  if (strcmp(dowhat, "SUBSCRIBE") != 0 
      && strcmp(dowhat, "UNSUBSCRIBE") != 0)  /* nothing to do */
    {
      exit(0);
    }

  if (list == NULL)
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
      sub_to_eVote(address, dowhat, action);
      exit(0);
    }
}
void
vacation_back_msg(char * guy)
{
  gen_header(SENDER, "Re:", YES);
  sprintf(error_msg,"\n%s's subscription to %s is redundant."
	  "\nHowever, %s is on vacation from %s."
	  "\n\nTo activate voting, %s must"
	  "\n\n\t1.  Write to \n\n\t\t%s@%s"
	  "\n\n\t2.  The subject doesn't matter."
	  "\n\n\t3.  The message should say:"
	  "\n\n\t\teVote back\n\n", 
	  guy, list, guy, list, guy, list, whereami);
}

/************************************************************/
void
vacation_msg(void)
{
  printf("\nThank you, %s, for your vacation request to the \n%s list.\n",
	 from, list);
  printf("\nYour \"eVote vacation\" command was processed successfully.  You");
  printf("\nhave been unsubscribed from Majordomo's list for %s", list);
  printf("\nbut your ballot is being held for you by eVote.\n");
  printf("\nWhen you return, send an \"eVote back\" command to \n%s@%s.\n",
	 list, whereami);
  printf("\nHave a nice vacation.\n");
}
/************************************************************/
void
vacation_redundant_msg(YESorNO going)
{
  printf("\neVote thinks you were already %s vacation.\n", 
	 (going ? "on" : "back from"));
}
/************************************************************
 *  Don't allow people on vacation to vote.  They can't see
 *  their receipt so it's not secure */
void
vacation_voter(void)
{
  sprintf(error_msg, "\nYour ballot is on \"vacation\" and can't be changed."
	  "\n\nBefore you can vote again:"
	  "\n\n\t1.  Write to \n\n\t\t%s@%s"
	  "\n\n\t2.  The subject doesn't matter."
	  "\n\n\t3.  Your message should say:"
	  "\n\n\t\teVote back\n\n", list, whereami);
}

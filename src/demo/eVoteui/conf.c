/* $Id: conf.c,v 1.3 2003/01/15 18:54:08 marilyndavis Exp $ */ 
/***********************************************************
 *      ../eVoteui/conf.c  
 *   User-interface calls that relate to whole conferences.
 ************************************************************
 *    Copyright (c) 1994...2015 Deliberate.com Patented.
 *    by Marilyn Davis
 **********************************************************/
#include <stdio.h>
#include <dirent.h>
#include "eVoteui.h"
#include "../../Clerklib/Clerk.h"
/************************************************************
 *   The named conf is checked.  The ballots are resummed and 
 *   the item totals are checked.  Discrepancies are fixed, if
 *   possible, and reported in Clerk.log.
 ************************************************************/
OKorNOT
check_conf(char * conf_name)
{
  RTYPE cc;
  switch (cc = send_check_conf(conf_name))
    {		  
    case GOOD:
      printf("\nConf %s is OK.\n", conf_name);
      return OK;
    case NO_CONF:
      fprintf(stderr,"\nNo such conf: %s.\n", conf_name);
      return NOT_OK;
    case NOT_GOOD:
      printf("\nSome discrepancies in check of %s. \nPlease see Clerk.log.\n",
	     conf_name);
      return NOT_OK;
    case FAILURE:
      return NOT_OK;
    default:
      printf("\nUnexpected return %d in check_conf(%s).\n",
	     cc, conf_name);
      return NOT_OK;
    }
}
/***********************************************************
 *     Determines if the user keyed-ahead the name of the next
 *     conf.  If so, passes back a pointer to that name.
 *     If not, it prompts the user for a new conf name.
 ***********************************************************/
char
*collect_conf_name(char* input)
{
  int i;
  void correct_conf_name(char*);
  /*  find the delimiting blank in the old input, if it's there */
  for (i = 1; input[i] != ' ' && input[i] != '\0' ; i++)
    ;
  /* get out all the blanks, if they're there */
  while (input[i] == ' ')
    i++;
  /* if nothing is left, prompt for a new conference */
  if (input[i] == '\0')
    {
      input = GetArg("\nConference name: ");
      i = 0;
    }
  /* get out all the blanks, if they're there */
  while (input[i] == ' ')
    i++;
  switch (input[i])
    {
    case 'q':
      if (input[i+1] != ' ' && strncmp(&input[i],"quit", 4) != 0)
	break;
    case '\0':
    case '?':
      i = -1;
      break;
    default:
      break;
    }
  if (i == -1)
    return NULL;
  /* truncate to CONFLEN chars */
  if (strlen(&input[i]) > CONFLEN - 1)
    input[CONFLEN] = '\0';
  correct_conf_name(&input[i]);
  return &input[i];
}
/**************************************************************
 *    If changing the case in conf_name will make it a valid 
 *    conf, the case is changed.
 **************************************************************/
void
correct_conf_name(char* new_conf_name)
{
  OKorNOT fix_case(char *change, char* keep, short max);
  char *real_conf_name;
  char keep_name[CONFLEN +1];
  char* get_conf_list(YESorNO finished);
  strcpy(keep_name, new_conf_name);
  while ((real_conf_name = get_conf_list(NO)) != NULL)
    {
      if (fix_case(new_conf_name, real_conf_name, CONFLEN) == OK)
	{
	  get_conf_list(YES);
	  return;
	}
    }
  get_conf_list(YES);
  strcpy(new_conf_name, keep_name);
}
/***************************************************************
 *       This function starts a new conference.
 ************************************************************/
OKorNOT
eVote_conf(char * conf_name, short drop_days)
{
  switch (send_eVote_conf(conf_name, drop_days))
    {
    case NOT_GOOD:
      printf("\nFailed: Can't open a new file for %s.\n", conf_name);
      return NOT_OK;
      break;
    case DONE:
      printf("\nCreated eVoted conference: %s.\n", conf_name);
      break;
    case REDUNDANT:
      printf("\nA %s conference already exists.\n", conf_name);
      break;
    case FAILURE:
      return NOT_OK;
      break;
    default:
      return NOT_OK;
      break;
    }
  return OK;
}
/***************************************************
 *     The first time it is called, it returns the first
 *     (alphabetically) eVoted conference.  Then it
 *     returns subsequent conferences until it runs out.
 *     If there are no more, it reinitializes in case
 *     it is called again and it returns a NULL.
 *****************************************************/
char
*get_conf_list(YESorNO finished)
{
  static DIR *dirp;
  struct dirent *dp;
  int len;
  static char path[PATHLEN + 1];
  static char name[PATHLEN + 1];
  static YESorNO path_in = NO;
  static YESorNO dir_open = NO;
  if (path_in == NO)
    {
      strcpy(path, find_default("EVOTE_HOME_DIR", NO, NO));
      strcat(path,"eVote/data");
      path_in = YES;
    }
  if (dir_open == NO)
    {
      dirp = opendir(path);
      if (dirp == NULL)
	{
	  printf("\nYou don't have permission to search eVote's data directory.");
	  printf("\nAsk your system administrator to give everyone permission");
	  printf("\nto read the data directory.\n");
	  printf("\nTerminating\n");
	  exit(0);
	}
      dir_open = YES;
    }
  if (finished != YES)
    {
      while ((dp = readdir(dirp)) != NULL)
	{
	  strcpy(name, dp->d_name);
	  if (name[0] == '~')
	    continue;
	  len = (int)strlen(name);
	  if (strcmp(name+len-3,"dat") != 0)
	    continue;
	  name[len-4] = '\0';
	  return name;
	}
    }
  closedir(dirp);
  dir_open = NO;
  return NULL;
}
/************************************************************/
void
list_confs(void)
{
  char *name;
  short hit = 0;
  char* get_conf_list(YESorNO finished);
  printf("\nAvailable conferences are:\n");
  name = get_conf_list(NO);
  while (name != NULL)
    {
      printf("%c%-12s", (hit++%4? '\t':'\n'), name);
      name = get_conf_list(NO);  
    }
  get_conf_list(YES);
  if (hit == 0)
    printf("\n There are no conferences.");
  else
    {
      printf("\n");
      printf("\nYou can visit any of these conferences by (g)o-ing to it:");
      printf("\n\n    g MovieClub           ");
      printf("\n\nto visit the 'MovieClub' conference. ");
    }
  printf("\n\nYou can create a new conference by (g)o-ing to it:\n");
  printf("\n\n    g MyNewConf      ");
  printf("\n\nto create a new conference named 'MyNewConf'.\n");
}

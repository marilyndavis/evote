/* * eVote - Software for online consensus development.
 * Copyright (C) 2015 Marilyn Davis <marilyn@deliberate.com>
 *
 * This file is part of eVote.
 *
 * eVote is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as
 * published by the Free Software Foundation, either version 3 of the
 * License, or (at your option) any later version.
 *
 * eVote is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with eVote.  If not, see <http://www.gnu.org/licenses/>.
 */

/* $Id: start.c,v 1.6 2003/01/23 19:54:27 marilyndavis Exp $ */ 
/*********************************************************
 *    start.c 
 *    This file contains functions that are needed by both
 *    the eVote command executable and by any user interface.
 *********************************************************
 **********************************************************/
#include<sys/stat.h>
#include<errno.h>
#include "Clerkdef.h"
#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include "eVote_defaults.h"
int msgmax;
int msgtql;
char version[50];
/* #define TESTPATH "/home/mailman" */
LISTSERVE listserve; 
/*******************************************************
 *   This function looks along root's (or the caller's)
 *   $PATH to find the path name to eVote.cf.  
 ********************************************************/
char * 
eVote_cf_path(YESorNO verbose)
{
  static char cf_path[PATHLEN + 1];
  char *env_PATH;
  static YESorNO done = NO;
  int i = -1, j, cc_stat;
  struct stat stat_buf;

  if (done == YES)
    return cf_path;
  if ((env_PATH = getenv("PATH")) != NULL)
    {
      listserve = MAJORDOMO;
#ifdef TESTPATH
      env_PATH = TESTPATH;
      listserve = MAILMAN;
#endif
    }
  else if ((env_PATH = getenv("PYTHONPATH")) != NULL)
    {
      listserve = MAILMAN;
    }
  else
    {
      fprintf(stderr, "\nNo PATH (for Majordomo) or PYTHONPATH (for Mailman)"
	      "in the environment.\n");
      exit(2);
    }
  i=-1;
  if (verbose)
    printf("\nFinding the path to eVote.cf");
  do   /* path to eVote.cf */
    {
      j= -1;
      while (env_PATH[++i] != ':' && env_PATH[i] != '\0')
	{
	  cf_path[++j] = env_PATH[i];
	}
      cf_path[++j] = '\0';
      if (listserve == MAJORDOMO)
	{
	  sprintf(cf_path,"%s/eVote.cf", cf_path);
	}
      else if(listserve == MAILMAN)
	{
	  sprintf(cf_path, "%s/scripts/eVote.cf", cf_path);
	}
      if (verbose)
	printf("\nTrying %s ...", cf_path);
      if ((cc_stat = stat(cf_path, &stat_buf) > -1))
	{
	  done = YES;
	  return cf_path;
	}
      if (errno != ENOENT)
	{
	  if (verbose)
	    printf("\nCould not stat the path to %s,"
		   "error # %d.\n", cf_path, errno);
	}
    }
  while (env_PATH[i] != '\0');
  fprintf(stderr, "\n\nPath to eVote.cf was not found.\n");
  if (listserve == MAILMAN)
    {
      fprintf(stderr,"\n\nBe sure that eVote.cf is soft-linked into"
	      "\n%s/scripts.\n", PYTHONPATH);
      fprintf(stderr,
	      "\nNote that this path is derived from \"PYTHONPATH\" which is inherited\n"
	      "from Mailman's wrapper.  PYTHONPATH seems to be %s.\n", 
	      PYTHONPATH);
    }
  else
    {
      fprintf(stderr,"\n\nBe sure:");
      fprintf(stderr,"\n    1.  The eVote.cf file is in EVOTE_BIN.");
      fprintf(stderr,"\n    2.  EVOTE_BIN is on your PATH.");
      fprintf(stderr,"\n\nNote that the expected path is inherited from the wrapper"
	      "\nprogram and is %s.\n", env_PATH);
    }
  exit(1);
  return "impossible";
}
/*************************************************************
 *      finds the eVote.cf directory, using the path name
 *      of the executable that calls this - eVote_cf_path().
 *      It reads the eVote.cf file and returns a pointer
 *      to the default we are looking_for.
 *      If make_space == YES, the answer is in malloced space,
 *      if not, it's in static space and is only temporary.
 **************************************************************/
char *
find_default(char * looking_for, YESorNO make_space, 
	     YESorNO verbose)
{
  static char answer[PATHLEN + 1];
  int ch;
  int counter;
  FILE* fp;
  char left[40];
  int len;
  char file_name[PATHLEN + 1];
  YESorNO need_slash = NO;
  YESorNO on_answer = NO;
  char *pt;
  /* set up default answers */
  answer[0] = '\0';
  if (strcmp(looking_for, "BLOCK_SIZE") == 0)
    sprintf(answer,"%d", BLOCK_SIZE);
  else if (strcmp(looking_for, "CRASH_COMMAND") == 0)
    answer[0] = '\0';
  else if (strcmp(looking_for, "EVOTE_HOME_DIR") == 0)
    {
      strcpy(answer, EVOTE_HOME_DIR);
      need_slash = YES;
    }
  else if (strcmp(looking_for, "EVOTE_MAIL_TO") == 0)
    strcpy(answer, EVOTE_MAIL_TO);
  else if (strcmp(looking_for, "EVOTE_MSG_KEY") == 0)
    sprintf(answer,"%s", EVOTE_MSG_KEY);
  else if (strcmp(looking_for, "IPCS") == 0)
    sprintf(answer,"%s", IPCS);
  else if (strcmp(looking_for, "LISTDIR") == 0)
    sprintf(answer,"%s", LISTDIR);
  else if (strcmp(looking_for, "MAILER") == 0)
    sprintf(answer,"%s", MAILER);
  else if (strcmp(looking_for, "MSGMNI") == 0)
    sprintf(answer,"%d", MSGMNI);
  else if (strcmp(looking_for, "MSGMAX") == 0)
    sprintf(answer,"%d", MSGMAX);
  else if (strcmp(looking_for, "MSGTQL") == 0)
    sprintf(answer,"%d", MSGTQL);
  else if (strcmp(looking_for, "PROGS") == 0)
    sprintf(answer,"%s","");
  else if (strcmp(looking_for, "SHMMNI") == 0)
    sprintf(answer,"%d", SHMMNI);
  else if (strcmp(looking_for, "STARTING_NICE") == 0)
    sprintf(answer,"%d", 20);
  else if (strcmp(looking_for, "TIME_OUT") == 0)
    sprintf(answer,"%d", TIME_OUT);
  else if (strcmp(looking_for, "WHEREAMI") == 0)
    sprintf(answer,"%s", WHEREAMI);
  strcpy(file_name, eVote_cf_path(verbose));
  /* try to open it */
  if ((fp = fopen(file_name, "r")) == NULL)
    {
      if (verbose)
	printf("Did not find %s, accepting default %s = %s.\n", 
	       file_name, looking_for, answer);
      return answer;
    }
  counter = 0;
  pt = left;
  while (1)
    {
      ch = fgetc(fp);
      switch (ch)
	{
	case '#':
	  while ((ch = fgetc(fp) != EOF)
		&& ch != '\n'
		&& ch != '\r')
	    ;  /* go around again */
	case '\r':
	case '\n':
	case EOF:
	  *pt = '\0';
	  if (left[0] == '\0' && ch != EOF)
	    break;
	  fclose(fp);
	  if (left[0] != '\0' && need_slash == YES)
	    {
	      len = (int)strlen(answer);
	      if (answer[len-1] != '/')
		strcat(answer,"/");
	    }
	  if (verbose)
	    printf("Found %s = %s.\n", looking_for, answer);
	  if (make_space)
	    {
	      char * pt;
	      if ((pt = malloc(strlen(answer) + 1)) == NULL)
		{
		  fprintf(stderr,"\nCan't malloc space in find_default.\n");
		  return NULL;
		}
	      strcpy(pt, answer);
	      return pt;
	    }
	  return answer;
	  break;
	case '=':
	  *pt = '\0';
	  if (strcmp(left, looking_for) == 0)
	    {
	      on_answer = YES;
	      pt = answer;
	      break;
	    }
	  /*  not a hit - dump the line and try again */
	  while ((ch = fgetc(fp) != EOF)
		&& ch != '\n' && ch != '\r')
	    ;
	  pt = left;
	  counter = 0;
	  break;
	case 34:
	  break;
	case ' ':
	case '\t':
	  if (!on_answer || pt == answer)
	    break;
	default:
	  if (on_answer != YES && (char)ch != looking_for[counter++])
	    {
	      while ((ch = fgetc(fp)) != EOF
		    && ch != '\n' && ch != '\r')
		{
		}
	      pt = left;
	      counter = 0;
	    }
	  else  
	    {
	      if (on_answer && pt - answer >= PATHLEN)
		{
		  fprintf(stderr,"\n%s in %s \nmust be only %d characters.\n",
			  looking_for, file_name, PATHLEN);
		  *pt = '\0';
		  while ((ch = fgetc(fp)) != EOF
			&& ch != '\n' && ch != '\r')
		    ;   /* finish line */
		  break;
		}
	      *pt = (char)ch;
	      pt++;
	    }
	  break;
	} /* end of switch */
    } /* end of while */
}
/**********************************************************************
 *      prints a big eVote to stdout.
 ******************************************************************/
void 
show_opening_screen(void)
{
  int i, len;
  printf("\n");
  printf("\n");
  printf("\n");
  printf("\n                                         VV");
  printf("\n                                        VV");
  printf("\n                                       VV                         ");
  printf("\n                                      VV                          ");
  printf("\n                                     VV                           ");
  printf("\n              VVV                   VV            VV              ");
  printf("\n                VV                 VV             VV              ");
  printf("\n       VVVVVV    VV               VV  VVVVVV    VVVVVV    VVVVVV  ");
  printf("\n      VV    VV    VV             VV  VV    VV     VV     VV    VV ");
  printf("\n     VV      VV    VV           VV  VV      VV    VV    VV      VV");
  printf("\n     VVVVVVVVVV     VV         VV   VV      VV    VV    VVVVVVVVVV");
  printf("\n     VV              VV       VV    VV      VV    VV    VV        ");
  printf("\n      VV    VV        VV     VV      VV    VV     VV     VV    VV ");
  printf("\n       VVVVVV          VV   VV        VVVVVV      VV      VVVVVV  ");
  printf("\n                        VV VV                                     ");
  printf("\n                         VVV                                      ");
  printf("\n                          V                    The Clerk is ready.");
  len = 65 - strlen(version);
  printf("\n");
  for (i=0; i < len; i++)
    {
      printf(" ");
    }
  printf("\n     %s", version);
  printf("\n                                                                  ");
  printf("\n     Copyright (c) 2003 by Deliberate.Com.       Patented.\n\n");
}

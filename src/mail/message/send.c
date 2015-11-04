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

/* $Id: send.c,v 1.5 2003/01/15 18:54:09 marilyndavis Exp $ */ 
/**********************************************************
 * send.c   Functions to fork and exec a new process.
 **********************************************************
 **********************************************************/
#include<stdio.h>
#include "../mailui/mailui.h"
#include<errno.h>
#include<sys/types.h>
#include<sys/stat.h>
#include <string.h>
#include <pwd.h>
#include <unistd.h>
char* mailer = NULL;
static void bounce_it(void);
/*#define TRACE "/tmp/eVote.child.trace" // traces child process 
  #define SEND_TRACE "/tmp/send.trace" */
#ifdef TRACE
FILE * tracefp;
#endif
#ifdef SEND_TRACE
FILE * send_tracefp;
#endif
FILE* fpout;
/**********************************************************
 *    This forks and execs a process:
 *        If whom == LIST it execs argv[1] and copies 
 *                        the other arguments.
 *        Otherwise it execs the mailer listed in 
 *                        eVote.cf
 *        The parent's stdout is attached to the child's
 *        stdin.
 ***********************************************************/
void
send(int whom)
{
  int i;
  char path[PATHLEN + 1];
  fpout = stderr;
#ifdef SEND_TRACE
    if ((send_tracefp = fopen(SEND_TRACE, "a")) == NULL)
      {
	fprintf(stderr,"\nCan't open send_trace file for send: %s\n",
		SEND_TRACE);
      }
    else
      {
	fpout = send_tracefp;
	fprintf(fpout,"\nSend_Trace started\n");
      }
    fprintf(fpout,"whom = %s\n", whom & LIST ? "LIST" : "NOT LIST");
    fprintf(fpout,"mailer = %s\n", mailer);
    fflush(fpout);
#endif

  if ((!(whom & LIST)) && (mailer == NULL || mailer[0] == '\0'
			   || from == NULL || from[0] == '\0'))
    {
      if (from == NULL)   /* processing in real time with
				    eVote_mail */
	{
	  fprintf(stderr, "%s", error_msg);
	  exit(2);
	}
      fprintf(stderr, "\nUnable to mail message.  The lost message follows:\n");
      dump_message(stderr, NO, NO, NO);
      exit(2);
    }
  if (whom & LIST)   /* figure path to next program */
    {
      find_path(path);
#ifdef SEND_TRACE
      fprintf(fpout, "Path worked: %s", path);
      fflush(fpout);
#endif
    }
  if (fork_is_parent())
    return;
  {  /* child process */ 
#ifdef TRACE
    if ((tracefp = fopen(TRACE, "a")) == NULL)
      {
	fprintf(stderr,"\nCan't open trace file for child: %s\n",
		TRACE);
	fpout = stderr;
      }
    else
      {
	fpout = tracefp;
	fprintf(fpout,"\nTrace started\n");
	fflush(fpout);
      }
#endif
    if (whom & LIST)
      {
#ifdef TRACE
	{
	  fprintf(fpout,"\nexecve called on %s, args are :\n", 
		  path);
	  for (i = 0; (argv_copy+1)[i] != NULL; i++)
	    fprintf(fpout," %s", (argv_copy+1)[i]);    
	  fprintf(fpout,"\nEnv passed is:\n");
	  for (i=0; (env_copy)[i] != NULL; i++)
	    fprintf(fpout, " %s", env_copy[i]);
	  fflush(fpout);
	}
#endif
	execve(path, argv_copy+1, env_copy);  
	fprintf(fpout,"\nERROR = %d\n", errno);
	fprintf(fpout,strerror(errno));
	fprintf(fpout,"%s\n", error_msg);
	{
	  struct passwd * pw;
	  pw = getpwuid(getuid());
	  fprintf(fpout,"\nRunning as %s", pw->pw_name);
	}
	fprintf(fpout,"\nExec failed. Message lost.\n");
	fprintf(fpout,"\nCall was \"%s\".  Args were:", path);
	for (i = 0; (argv_copy+1)[i] != NULL; i++)
	  fprintf(fpout," %s", (argv_copy+1)[i]);
	fprintf(fpout,"\n");
      }
    else
      {
	bounce_it();  /* should never return */
#ifdef TRACE
	fprintf(fpout,"%s", error_msg);
	fprintf(fpout, "Couldn't send return mail from eVote. execv failed.  Message lost.\n");
#endif
	fprintf(stderr,"%s", error_msg);
	fprintf(stderr, "Couldn't send return mail from eVote. execv failed.  Message lost.\n");
      }
#ifdef TRACE
    dump_message(fpout, NO, NO, NO);
#endif
    dump_message(stderr, NO, NO, NO);
    exit(2);
  }
}  
/*********************************************************
 *  Executes the mailer command for messages going to the sender.
 *********************************************************/
void
bounce_it(void)
{
  int i, j = 0, k, len;
  char path[FNAME + 1];
  char *argv_to_exec[MAX_ARGS];
  char fromstr[MAX_ADDRESS + 1];
#ifdef TRACE  
  fprintf(fpout,"\nIn bounceit, mailer = %s.\n", mailer); 
#endif
  len = strlen(mailer);
  for (i = 0; mailer[i] != ' '; i++)
    {
#ifdef TRACE
      fprintf(fpout,"%d:%c ", i, mailer[i]);  
#endif
      path[i] = mailer[i];
    }
  path[i] = '\0';
#ifdef TRACE
  fprintf(fpout,"\npath = %s\n", path);  
#endif  
  while (path[--i] != '/')
    {
#ifdef TRACE
      fprintf(fpout,"%d:%c ", i, path[i]);  
#endif
    }
  argv_to_exec[0] = &mailer[++i];
  while (mailer[++i] != ' ')
    {
#ifdef TRACE
      fprintf(fpout,"%d:%c ", i, mailer[i]); 
#endif
    }
  mailer[i] = '\0';
#ifdef TRACE
  fprintf(fpout,"\nargv_to_exec[0] = %s", argv_to_exec[0]); 
#endif
  for (++i, k = 1, j = 0; i < len; k++, j = 0)
    {
      argv_to_exec[k] = &mailer[i];  
#ifdef TRACE
      fprintf(fpout,"\nk = %d, starts with %c%c.\n", k,
	      mailer[i], mailer[i+1]);  
#endif
      while (mailer[++i] != ' ' && mailer[i] != '\0')
	{
#ifdef TRACE
	  fprintf(fpout,"%d:%c ", i, mailer[i]); 
#endif
	}
      mailer[i++] = '\0';
#ifdef TRACE
      fprintf(fpout,"\nargv_to_exec[%d] = %s", k, argv_to_exec[k]); 
#endif
      if (strncmp(argv_to_exec[k], "-f", 2) == 0)
	{
#ifdef TRACE
	  fprintf(fpout,"\nfound -f on k=%d, was %s.\n", 
		  k, argv_to_exec[k]);
#endif
	  argv_to_exec[k] = fromstr;
	  if (list == NULL || list[0] == '\0')
	    sprintf(fromstr, "-feVote");
	  else
	    sprintf(fromstr, "-f%s-eVote", list);
	  if (whereami != NULL && whereami[0] != '\0'
	      && strcmp(whereami,"localhost") != 0)
	    {
	      strcat(fromstr,"@");
	      strcat(fromstr, whereami);
	    }
#ifdef TRACE
	  fprintf(fpout,"\nk= %d, %s\n", k, argv_to_exec[k]);
#endif
	}
    }
  argv_to_exec[k] = NULL;
#ifdef TRACE
  fprintf(fpout,"\nexecv called on %s %s %s %s ...\n", path, argv_to_exec[0], argv_to_exec[1], argv_to_exec[2]);
  fflush(fpout);
#endif
  execv(path, argv_to_exec);
#ifdef TRACE
  fprintf(fpout, "\nERROR = %d.\n", errno);
  fprintf(fpout,"execv called on %s with args:\n", path);
  for (k = 0; argv_to_exec[k] != NULL; k++)
    fprintf(fpout,"argv_to_exec[%d] = %s\n", k, argv_to_exec[k]);
#endif
  fprintf(stderr, "\nERROR = %d.\n", errno);
  fprintf(stderr,"execv called on %s with args:\n", path);
  for (k = 0; argv_to_exec[k] != NULL; k++)
    fprintf(stderr,"argv_to_exec[%d] = %s\n", k, argv_to_exec[k]);
}

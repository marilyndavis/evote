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

/* $Id: report.c,v 1.3 2003/01/15 18:54:09 marilyndavis Exp $ */ 
 /**********************************************************
 *   report.c  Functions that control the report for a petition.   
 *********************************************************
 **********************************************************/
#define FTP_COMMAND "ftp -i"
#define FTP_ERROR   "/tmp/ftp.err"
#define FTP_TIMEOUT 100
#define FTP_TIMES 3
#include<stdio.h>
#include<errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include<sys/wait.h>
#include"../../Clerklib/Clerkdef.h"
#include"../../Clerklib/Clerk.h"
#include"mailui.h"
#include <signal.h>
#include<unistd.h>
#define FTP_DEBUG "/tmp/ftp.debug"
#ifdef FTP_DEBUG
 YESorNO debug = YES;
FILE * ftp_debug;
#endif
#define REPORT_LEN 100
#define REPORT_BUNCH 2
#define VAR_LEN 12
#define WHERELEN 80
typedef struct
{
  char file_name[FNAME + 1];
  char var[VAR_LEN + 1];
  char where[WHERELEN + 1];
} REPORT;
static REPORT *report;
static short no_of_reports;
static OKorNOT gen_report(char *variable, unsigned long number, 
			  FILE* fp_in, FILE *fp_out, FILE * fp_tmp);
static int timed_system(char * command, char * input, 
			char *output, int seconds);
static OKorNOT try_ftp(char * file, char * where);
static OKorNOT update_report(unsigned long number, REPORT *report);
/************************************************************
 *   Generates an html report in fp_out copying fp_in except it 
 *   replaces variables with number.
 *   fp_in is updated so that the variable knows its current value.
 *   fp_tmp is a temporary work file that is also the lock 
 *   for the maneuver.
 ************************************************************/
OKorNOT
gen_report(char *variable, unsigned long number, 
	   FILE* fp_in, FILE *fp_out, FILE* fp_tmp)
{
  int i = 0, j;
  int len = strlen(variable);
  int ch;
  YESorNO found = NO;
  YESorNO special = NO;
  int var_start, num_start, var_end;
  YESorNO brace, equal, wrong;
  char buf[MAX_LINE + 1];
  while ((ch = getc(fp_in)) != EOF)
    {
      if (ch != '{')
	{
	  fputc(ch, fp_out);
	  fputc(ch, fp_tmp);
	  continue;
	}
      special = NO;
      wrong = NO;
      var_start = -1;
      var_end = -1;
      brace = NO;
      equal = NO;
      num_start = -1;
      buf[i= 0] = '{';
      while (++i < MAX_LINE  && ((ch = fgetc(fp_in)) != EOF))
	{
	  switch (buf[i] = ch)
	    {
	    case ' ':
	    case '\t':
	      if (var_start > -1 && var_end == -1)
		var_end = i;
	      break;
	    case '{':
	      ungetc(ch, fp_in);
	      i--;
	      wrong = YES;
	      break;
	    case '}':
	      brace = YES;
	      break;
	    case '=':
	      if (var_end < 0)
		var_end = i;
	      if ((ch = fgetc(fp_in)) == '=')
		equal = YES;
	      else
		wrong = YES;
	      buf[++i] = ch;
	      break;
	    default:
	      if (equal == YES)
		{
		  if (special)
		    wrong = YES;
		  if (buf[i] >= '0' && buf[i] <= '9')
		    {
		      if (num_start == -1)
			num_start = i;
		    }
		  else
		    {
		      sprintf(error_msg, 
			      "\nCan't read the number for %s.\n",
			      variable);
		      return NOT_OK;
		    }
		}
	      else
		{
		  if (var_start == -1)	
		    {
		      var_start = i;
		      if (buf[i] == '$')
			special = YES;
		    }
		}
	      break;
	    }
	  if (wrong == YES || brace == YES)
	    break;
	}
      if (brace == NO || var_start == -1 
	  || (num_start == -1 && !special)
	  || wrong == YES || i == MAX_LINE)
	{
	  if (i == MAX_LINE)
	    i--;
	  for (j = 0; j <= i ; j++)
	    {
	      fputc(buf[j], fp_out);
	      fputc(buf[j], fp_tmp);
	    }
	  continue;
	}
      /*  found my variable, is it the right one? */
      if (special)
	{
	  if (strncmp(&buf[var_start], "$DATE", 5) == 0)
	    {
	      fprintf(fp_out,"%s", date_str(0));
	      for (j = 0; j <= i; j++)
		{
		  fputc(buf[j], fp_tmp);
		}
	      continue;
	    }
	}
      if (strncmp(&buf[var_start], variable, len) != 0
	  || len != var_end - var_start)
	{   /* wrong variable - updated from different petition */
	  /*  propagate the old answer */
	  for (j = 0; j <= i; j++)
	    fputc(buf[j], fp_tmp);
	  for (j = num_start; buf[j] >= '0' && buf[j] <= '9'; j++)
	    {
	      fputc(buf[j], fp_out);
	    }
	  continue;
	}
      /* right variable */
      for (j = 0; j < num_start; j++)
	fputc(buf[j], fp_tmp);
      fprintf(fp_tmp, "%lu", number);
      fprintf(fp_out, "%lu", number);
      fputc('}', fp_tmp);
      found = YES;
    }
  if (found)
    return OK;
  return NOT_OK;
}
/************************************************************
 *    Reads the report instructions from the report_instructions file
 *    into memory, if there are any.
 *    Returns the number of reports to write.
 ************************************************************/
int
read_report_instructions(ITEM_INFO *p_item)
{
  int i;
  char buf[2 * MAX_LINE + 2];
  char * fname;
  FILE *fp;
  fname = pet_fname("report_instructions", p_item);
  if ((fp = fopen(fname, "r")) == NULL
      || 	fgets(buf, MAX_LINE, fp) == NULL
      || 	sscanf(buf, "Reports: %hd\n", &no_of_reports) != 1
      || no_of_reports == 0)
    {
      if (fp != NULL)
	fclose(fp);
      return no_of_reports = 0;
    }
  if ((report = malloc(no_of_reports * sizeof(REPORT)))
      == NULL)
    {
      sprintf(error_msg,
	      "\nThere are no resources to read report requests at this time.  Please try later.\n");
      bounce_error(SENDER | ADMIN);
    }
  for (i = 0; i < no_of_reports; i++)
    {
      if (fgets(buf, 2*MAX_LINE + 2, fp) != buf
	  || sscanf(buf, " %s %s %s ", report[i].file_name, 
		    report[i].var, report[i].where) != 3)
	{
	  sprintf(error_msg,"\nStored report is messed up for %s.\n",
		  subject);
	  bounce_error(ADMIN);
	}
    }
  return no_of_reports;
}	
/*************************************************************/
void
ship_reports(unsigned long signers)
{
  int i;
  for (i = 0; i < no_of_reports; i++)
    {
      update_report(signers, &report[i]);
    }
}
/************************************************************
 *        Runs the program at path in the shell
 *        Kills it after seconds seconds.
 *        Returns 0 if the program finishes
 *              251 if it was killed.
 ***********************************************************/
void onalarm(int);
int pid;
int
timed_system(char * command, char * input, char *output, int seconds)
{
  char * argvv[10];
  int argcc = -1;
  int i = -1, cc, cc2, cc3;
  int start = 0;
  void(* old_alarm)(int);
  int status;
  char call[400];
  char program[100];
  int fd_input;
  int fd_output;
  int left;
  strcpy(call, command);
  while (call[++i] != '\0')
    {
      switch (call[i])
	{
	case ' ':
	case '\t':
	  start = 0;
	  call[i] = '\0';
	  break;
	default:
	  if (start == 1)
	    break;
	  argvv[++argcc] = &call[i];
	  start = 1;
	  break;
	}
    }
  argvv[++argcc] =	NULL;
  strcpy(program, argvv[0]);
  /* argvv[0] should only have the name, not the path */
  for (i = strlen(argvv[0]) - 1; i > 0; i--)
    {
      if (argvv[0][i] == '/')
	{
	  argvv[0] = &argvv[0][i+1];
	  break;
	}
    }
  /* Now argvv has the call and args */
  fflush(stdout);   /* so we can fork stdout too */
  switch (pid = fork())
    {
    case -1:
#ifdef FTP_DEBUG
      if (debug)
	fprintf(ftp_debug, 
		"\ntimed_system: fork failed on %s.\n", command);
#endif
      fprintf(stderr, "\ntimed_system: fork failed on %s.\n", command);
      return -1;
      break;
    case 0:     /* child */
      if (input != NULL)
	{
	  if ((fd_input = open(input, O_RDONLY)) == -1)
	    {
#ifdef FTP_DEBUG
	      if (debug)
		fprintf(ftp_debug, 
			"\ntimed_system: Can't open %s for input.\n", input);
#endif
	      fprintf(stderr, "\ntimed_system: Can't open %s for input.\n", input);
	      return -1;
	    }
#ifdef FTP_DEBUG
	  if (debug)
	    fprintf(ftp_debug, "\ntimed_system: Opened %s for input.\n", input);
#endif
	  close(0);
	  dup(fd_input);
	  close(fd_input);
	}
      if (output != NULL)
	{
	  /* O_TRUNC or O_APPEND */
	  if ((fd_output = open(output, O_WRONLY|O_CREAT|O_TRUNC, 0600)) == -1)
	    {
#ifdef FTP_DEBUG
	      if (debug)
		fprintf(ftp_debug, 
			"\ntimed_system: Can't open %s for output.\n", output);
#endif
	      fprintf(stderr, "\ntimed_system: Can't open %s for output.\n", output);
	      return -1;
	    }
	  close(1);
	  dup(fd_output);
	  close(2);
	  dup(fd_output);
	  close(fd_output);
	}
      if (i == 0)
	{
#ifdef FTP_DEBUG
	  if (debug)
	    fprintf(ftp_debug, "\ntimed_system: calling execvp on %s.\n", command);
#endif					
	  cc = execvp(program, &argvv[0]);
	}
      else
	{
#ifdef FTP_DEBUG
	  if (debug)
	    fprintf(ftp_debug, "\ntimed_system: calling execv on %s.\n", command);
#endif
	  cc = execv(program, &argvv[0]);
	}
      switch (errno)
	{
	case ENOENT:
#ifdef FTP_DEBUG
	  if (debug)
	    fprintf(ftp_debug, "\ntimed_system: One or more components of the new process path name of the file do not exist.");
#endif
	  fprintf(stderr, "timed_system: One or more components of the new process path name of the file do not exist.");
	  break;
	default:
#ifdef FTP_DEBUG
	  if (debug)
	    fprintf(ftp_debug, "timed_system: %d received from failed exec.\n", cc);
#endif
	  fprintf(stderr, "timed_system: %d received from failed exec.\n", cc);
	  break;
	}
#ifdef FTP_DEBUG
      if (debug)
	fprintf(ftp_debug,"\ntimed_system: Can't exec %s.\n", command);
#endif
      fprintf(stderr, "\ntimed_system: Can't exec %s.\n", command);
      return -1;
      break;
    default:    /* parent */
      break;
    }
  fclose(stdout);
  old_alarm = signal(SIGALRM, onalarm);
  left = alarm(seconds);
  /*	cc2 = wait(&status); */
  while ((cc2 = wait(&status)) != pid)
    {
      if (cc2 == -1 || (status & 0177) != 0)
	{
	  /*					fprintf(stderr,"timed_system: %s killed", command); */
#ifdef FTP_DEBUG
	  if (debug)
	    {
	      fprintf(ftp_debug,"\ntimed_system: %s killed!!", command);
	      fprintf(ftp_debug,"\n              Return from wait = %d.", 
		      cc2);
	      fprintf(ftp_debug,"\n              status & 0177 = %d", 
		      status & 0177);
	    }
#endif
	}
    }
  signal(SIGALRM, old_alarm);
  left = alarm(left);
#ifdef FTP_DEBUG
  if (debug)
    {
      fprintf(ftp_debug,"\ntimed_system: Parent is pid %d.", getpid());
      fprintf(ftp_debug,"\n              Child is pid %d.", pid);
      fprintf(ftp_debug,"\n              It took %d seconds.", FTP_TIMEOUT - left);
      fprintf(ftp_debug,"\n              wait returned %d, status = %d.\n", 
	      cc2, status);
      fprintf(ftp_debug,"\n              WIFEXITED = %d.", cc3 = WIFEXITED(status));
      if (cc3 == 0)
	fprintf(ftp_debug,"\n              WEXITSTATUS = %d.", WEXITSTATUS(status));
      fprintf(ftp_debug,"\n              WIFSIGNALED = %d.", cc3 = WIFSIGNALED(status));
      if (cc3 == 0)
	fprintf(ftp_debug,"\n              WTERMSIG = %d.", WTERMSIG(status));
    }
#endif
  if (cc2 == -1 || status & 0377)
    return -2;   /* killed the child */
  return ((status >> 8) & 0177);
}
/************************************************************/
void
onalarm(int x)
{
  kill(pid, SIGKILL);
}
/************************************************************
 * ftp's the file to where using the commands in file.ftp
 ************************************************************/
OKorNOT
try_ftp(char * file, char * where)
{
  int cc, len, ftp_count = 0;
  char command[5 * FNAME + 1];
  struct stat buf;
  char path[FNAME +1];
  char input_file[FNAME + 1];
#ifdef FTP_DEBUG
  /*		time_t now; */
  if (debug)
    {
      if ((ftp_debug = fopen(FTP_DEBUG,"a")) == NULL)
	{
	  fprintf(stderr,"\nCan't open ftp_debug file: %s.\n", 
		  FTP_DEBUG);
	  debug = NO;
	}
      else
	{
	  fprintf(ftp_debug,"\n%s", ctime(&now));
	}
    }
#endif
  strcpy(path, file);
  len = strlen(path);
  while (path[len] != '/')
    len--;
  path[len] = '\0';
  cc = chdir(path);
  /*		sprintf(command,"%s %s < %s.%s 2 > %s", 
		FTP_COMMAND, where, file, "ftp", FTP_ERROR); */
  sprintf(command,"%s %s", FTP_COMMAND, where);
  sprintf(input_file, "%s.%s", file, "ftp");
#ifdef FTP_DEBUG
  if (debug)
    {
      fprintf(ftp_debug,"try_ftp: Here's the ftp command: \n%s\n", command); 
      fprintf(ftp_debug,"         The input file is %s\n", input_file);
      fprintf(ftp_debug,"         The error file is %s\n", FTP_ERROR);
      fprintf(ftp_debug,"         The timeout is set for %d seconds.\n",
	      FTP_TIMEOUT);
    }
#endif
  while (++ftp_count < FTP_TIMES)
    {
      cc = -2;
      if ((cc = timed_system(command, input_file, FTP_ERROR, FTP_TIMEOUT)) 
	  == 0) 
	{
#ifdef FTP_DEBUG
	  if (debug)
	    {
	      fprintf(ftp_debug,
		      "\ntry_ftp: Ftp succeeded on try # %d.\n",
		      ftp_count);
	    }
#endif
	  break;
	}
      else
	{
	  sprintf(error_msg,
		  "\ntry_ftp: This command failed: %s\n \tError # %d.  Try # %d - Timeout is %d.\n", 
		  command, cc, ftp_count, FTP_TIMEOUT);
	  switch (cc)
	    {
	    case 256:
	      strcat(error_msg,"\tFailed to connect.\n");
	      break;
	    case 127:
	      strcat(error_msg, "\tExec failed\n");
	      break;
	    case -1:
	      strcat(error_msg,"\tSome system trouble\n");
	      break;
	    case -2:
	      strcat(error_msg,"\tFtp timed out.\n");
	      break;
	    case 2:
	      strcat(error_msg,"\tNULL command.\n");
	      break;
	    default:
	      sprintf(&error_msg[strlen(error_msg)],
		      "\tUnknown error: %d.\n", cc);
	      break;
	    }
	  fprintf(stderr,"%s", error_msg);
#ifdef FTP_DEBUG
	  if (debug)
	    fprintf(ftp_debug,"%s", error_msg);
#endif
	}
    }
  if (ftp_count >= FTP_TIMES && cc != 0)
    return NOT_OK;
  if (stat(FTP_ERROR, &buf) == 0)
    {
      if (buf.st_size > 0)
	{
	  sprintf(error_msg, "timed_system: Error in ftp on %s.\n", 
		  file);
	  fprintf(stderr, "%s", error_msg);
#ifdef FTP_DEBUG
	  if (debug)
	    fprintf(ftp_debug, "%s", error_msg);
#endif
	  return NOT_OK;
	}
    }
  return OK;
}
/************************************************************
 *  Updates the html report and ftp's it to the site.
 ************************************************************/
OKorNOT
update_report(unsigned long number, REPORT *report)
{
  char full_name[FNAME + 1];
  char var_name[FNAME + 1];
  char command[2 * FNAME + 20];
  FILE *fp_tmp, * fp_in, * fp_out;
  OKorNOT cc = OK;
  sprintf(full_name, "%s/%s", pet_fname("reports", p_item_copy),
	  report->file_name);
  while ((fp_tmp = lock_to_tmp(full_name)) == NULL )
    {
      /*			unlock_to_tmp(YES); */
      /*			if (++times > 2)     */
      {
	emergency("Can't update report", FTP_ERROR);
	return NOT_OK;
      }
    }
  sprintf(var_name, "%s.var", full_name);
  if ((fp_in = fopen(var_name, "rw")) == NULL)
    {
      unlock_to_tmp(NO);
      sprintf(error_msg,"\nCan't open %s to update report for %s.\n",
	      var_name, subject);
      emergency("Can't update report", FTP_ERROR);
      return NOT_OK;
    }
  if ((fp_out = fopen(full_name, "w")) == NULL)
    {
      unlock_to_tmp(NO);
      sprintf(error_msg,"\nCan't open %s to update report for %s.\n",
	      full_name, subject);
      emergency("Can't update report", FTP_ERROR);
      return NOT_OK;
    }
  cc = gen_report(report->var, number, fp_in, fp_out, fp_tmp);
  if (fp_in != NULL)
    {
      fclose(fp_in);
      fp_in = NULL;
    }
  if (fp_out != NULL)
    {
      fclose(fp_out);
      fp_out = NULL;
    }
  fflush(fp_tmp);
  sprintf(command,"cp %s %s", tmp_file_name, var_name);
  system(command);
  if (cc == OK && !same(report->where, "x"))
    {
      if (try_ftp(full_name, report->where) != OK)
	{
	  emergency("Can't update report", FTP_ERROR);
	  cc = NOT_OK;
	}
    }
  unlock_to_tmp(NO);
  return cc;
}

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

/* $Id: tryagain.c,v 1.3 2003/01/15 18:54:13 marilyndavis Exp $ */ 
/********************************************************
*   tryagain.c 
*   Crash_command for the Clerk.
*   When it is called from the dying Clerk, it restarts it
*   unless it's been less than TOO_SOON seconds since the
*   last time the Clerk died.
******************************************************/
/*********************************************************
 **********************************************************/
#include<stdio.h>
#include<pwd.h>
#include<time.h>
#define FNAME "./down.time"
#define TIMELEN 30
#define BLOCK 2
typedef enum{NO, YES, UNKNOWN} YESorNO;
typedef enum{NOT_OK, OK, WHAT} OKorNOT;
#define TOO_SOON 1200   /* 20 minutes */
typedef struct
{
  time_t time;
  char time_str[TIMELEN + 1];
} ENTRY;
OKorNOT check(char *file_name);
ENTRY time_report(void);
OKorNOT write_and_start(FILE *fp, ENTRY now);
int
main(int argc, char *argv[])
{
  ENTRY now, then, forever = { 0L, ""};
  FILE * fp;
  now = time_report();
  if ((fp = fopen(FNAME,"r+")) == NULL)
    {
      fprintf(stderr,"\nCan't open %s.\n", FNAME);
      return NOT_OK;
    }
  if (fread(&then, sizeof(ENTRY), 1, fp) != 1)  /* no data yet - first time */
    then = forever;
  if (now.time - then.time < TOO_SOON || write_and_start(fp, now) != OK)
    {
      system("/root/killem");
      system("mail -s \"Clerk crashing, bringing down the modem\" owner-eVote < ./down.time");
      exit(0);
    }
  else
    {
      system("mail -s \"Clerk crashed -- restarting.\" owner-eVote < ./eVote.out");
    }
  fclose(fp);
  return 0;
}
OKorNOT
write_and_start(FILE *fp, ENTRY now)
{
  if (fwrite(&now, sizeof(ENTRY), 1, fp) != 1)
    {
      fprintf(stderr,"\nCan't write to %s.\n", FNAME);
      fclose(fp);
      return NOT_OK;
    }
  system("eVote > eVote.out");
  return check("eVote.out");
}
/************************************************************
 *  char *time_report(void)  Returns a string containing the
 *                           current date and time.
 ************************************************************/
ENTRY
time_report(void)
{
  ENTRY answer;
  answer.time = time((long*)(0));    /* returns seconds since 1/1/70 */
  strcpy(answer.time_str, ctime(&answer.time));
  return answer;
}
OKorNOT
check(char *file_name)
{
  char * good = "ready.";
  char * bad = "Failed";
  int igood = 0, ibad = 0;
  FILE * fp;
  int ch;
  YESorNO bad_found = NO;
  YESorNO good_found = NO;
  if ((fp = fopen(file_name,"r")) == NULL)
    return NOT_OK;
  while ((ch = fgetc(fp)) != EOF)
    {
      if (!good_found && good[igood] == ch)
	{
	  if (good[++igood] == '\0')
	    {
	      good_found = YES;
	    }
	}
      if (!bad_found && bad[ibad] == ch)
	{
	  if (bad[++ibad] == '\0')
	    {
	      bad_found = YES;
	    }
	}
    }
  fclose(fp);
  if (bad_found)
    return NOT_OK;
  if (good_found)
    return OK;
  return NOT_OK;
}

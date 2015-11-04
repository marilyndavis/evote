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

/* $Id: longtest.c,v 1.4 2003/01/15 18:54:12 marilyndavis Exp $ */ 
/****************************************************
 * 
 *      longtest speed length_of_time
 *         exercises the IPCs and voter dropping.
 *
 ********************************************************/
#define CHILD 0
#define MAX_ARGS 20
#define NO_CONFS 3
#include<stdio.h>
#include<unistd.h>
#include<time.h>
#include "../demo/eVote.h"
#include "../Clerklib/msgdef.h"
#include <sys/types.h>

char conf[NO_CONFS][CONFLEN + 1];  /* holds conf names*/
char dummy_arg[MAX_ARGS][20];      /* holds args for go_dummy */

/**********
	   The dums array holds info about each dummy user.
	   YESorNO, whether or not the dummy is online.
	   conf *  the conference the dummy participates in
*************/

struct dum_def
{
  short on;
  char * conf;
} dums[101];

short debugging;

int main(int argc,char *argv[])
{
  char *args[MAX_ARGS];
  char command[200];
  
  short users = 10;
  short i, j, guy;
  short slowness = 5;
  long duration = 60;
  int times = 0;
  
  
  void go_dummy(char* args[]); /* defined below */
  /* send_drop_voter is in ../../demo/Clerk/voterl.c */
  
  start_up(NO);
  
  strcpy(dummy_arg[0],"dummy");
  
  for(i=0;i<MAX_ARGS;i++)  /* initialize to nulls, except [0] is "dummy" */
    {
      args[i] = dummy_arg[i];
    }
  
  duration = 3600l + time(0);
  
  if(argc > 1)  
    {
      slowness = 10 - atoi(argv[1]);
      duration = (long)(atoi(argv[2]) * 60) + time(0);
    }
  
  srand(users);  /* seed the random number generator */
  
  sprintf(command,"echo \"starting longtest \" > %s",FLOW);
  system(command);
  sprintf(command,"chmod uog+w %s",FLOW);
  system(command);
  
  for (i = 0; i < NO_CONFS; i++) /* concoct NO_CONFS names */
    {
      sprintf(conf[i],"ed%d",i);
    }
  
  for (i = 0; i < NO_CONFS; i++)
    {
      /*  start the confs -- evtstart makes drop_days == 0 */
      sprintf(dummy_arg[1],"%d",1);
      sprintf(dummy_arg[2],"%d",1);
      strcpy(dummy_arg[3],"evtstart");
      strcpy(dummy_arg[4], conf[i]);
      args[5] = NULL;
      go_dummy(args);
      args[5] = dummy_arg[5];
    }
  
  sleep(slowness);
  /*  now get the users active */
  
  while(time(0) < duration)
    {
      time_t left;
      sprintf(command,"echo \"%d time; %d seconds left. \" >> %s",
	      ++times, (int)left = duration - time(0), FLOW);
      system(command);
      printf("\n%d time: %d seconds left.\n",times, (int)left);
      if(times > 10) /* out of guys */
	{
	  for (guy = 1; guy < 101 ; guy++)
	    {
	      if (dums[guy].on == times - 10)
		dums[guy].on = 0;
	    }
	}
      
      for(j = 1; j <= users; j++)
	{
	  do   /* find one that isn't online yet */
	    {
	      guy = rand()%100 + 1;
	    }
	  while(dums[guy].on != 0);
	  dums[guy].on = times;
	  dums[guy].conf = conf[(rand()%NO_CONFS)];
	  printf("\nguy %d on %s",guy, dums[guy].conf);
	  fprintf(stderr,"\nguy %d on %s",guy, dums[guy].conf);
	  sprintf(dummy_arg[1],"%d",guy);
	  sprintf(dummy_arg[2],"%d",guy);
	  strcpy(dummy_arg[3],"slowvoter");
	  strcpy(dummy_arg[4],dums[guy].conf);
	  sprintf(dummy_arg[5],"%d",slowness);
	  args[6] = NULL;
	  go_dummy(args);
	  args[6] = dummy_arg[6];
	  sleep(slowness);
	}
      
      sleep(slowness * users * 3);  /* let them do their stuff */
      
      /* assume they're all done */
      
      for (j = 1; j <= users; j++)
	{
	  dums[j].on = NO;
	}
    }
  return 0;
}
/************************************************
 *
 *   go_dummy execs a call to dummy and passes along
 *     some arguments.
 *     dummy changes its uid to be the indicated dummy
 *       and runs the program indicated under that
 *       uid.
 **********************************************/

void go_dummy(char* args[])
{
  int pid;
  int i;
  
  while((pid = fork()) == -1)
    sleep(1);
  printf("\ngo_dummy called with:\n");
  for(i = 0; args[i] != NULL && args[i][0]!='\0';i++)
    printf("%s ",args[i]);;
  
  if(pid == CHILD)
    {
      execvp("./dummy",args);
      printf("\nexec failed:\n");
      for(i = 0; args[i][0]!='\0';i++)
	printf("%s ",args[i]);;
      printf("\n");
    }
}

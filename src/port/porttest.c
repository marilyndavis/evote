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

/* $Id: porttest.c,v 1.4 2003/01/15 18:54:12 marilyndavis Exp $ */ 
/****************************************************
 * 
 *      porttest [number] 
 *         exercises the IPCs and voter dropping.
 *         number is the number of dummy users to put
 *           online simultaneously.
 *
 ********************************************************/
#define CHILD 0
#define MAX_ARGS 20
#define NO_CONFS 3
#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include "../demo/eVote.h"

char conf[NO_CONFS][CONFLEN + 1];  /* holds conf names*/
char dummy_arg[MAX_ARGS][20];      /* holds args for go_dummy */

/**********
	   The dums array holds info about each dummy user.
	   YESorNO, whether or not the dummy is online.
	   conf *  the conference the dummy participates in
*************/

struct dum_def
{
  YESorNO on;
  char * conf;
} dums[101];

short debugging;

int main(int argc,char *argv[])
{
  char *args[MAX_ARGS];
  unsigned long users = 100;
  short i, j;
  unsigned long guy;
  
  void go_dummy(char* args[]); /* defined below */
  /* send_drop_voter is in ../Clerklib/voterl.c */
  RTYPE send_drop_voter(char *conf_name, unsigned long uid_to_drop,
			YESorNO only_if_non_voter);
  
  start_up(NO);
  strcpy(dummy_arg[0],"dummy");
  
  for(i=0;i<MAX_ARGS;i++)  /* initialize to nulls, except [0] is "dummy" */
    {
      args[i] = dummy_arg[i];
    }
  
  
  if(argc > 1)  /* number of simultaneous users specified */
    {
      users = atoi(argv[1]);
    }
  
  srand(users);  /* seed the random number generator */

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

  sleep(5);
  printf("\nConferences are started\n");
  system("eVote flush");

  /*  now get the users active */
  
  for(j = 1; j <= users; j++)
    {
      do   /* find one that isn't online yet */
	{
	  guy = rand()%100 + 1;
	}
      while(dums[guy].on == YES);
      dums[guy].on = YES;
      dums[guy].conf = conf[(rand()%NO_CONFS)];
      printf("\nguy %ld on %s",guy, dums[guy].conf);
      fprintf(stderr,"\nguy %ld on %s",guy, dums[guy].conf);
      sprintf(dummy_arg[1],"%ld",guy);
      sprintf(dummy_arg[2],"%ld",guy);
      strcpy(dummy_arg[3],"fastvoter");
      strcpy(dummy_arg[4],dums[guy].conf);
      args[5] = NULL;
      go_dummy(args);
      args[5] = dummy_arg[5];
    }

  for(j = 0; j< 10; j++)
    {
      sleep(users);

      printf("\nslept %ld", (users * j ));
    }

  /*  drop the users from the system */
  exit(0);
  for (j = 1; j <= users; j++)
    {
      do   /* find one that has been online */
	{
	  guy = rand()%100 + 1;
	}
      while(dums[guy].on == NO);
      dums[guy].on = NO;
      printf("\nDropping %ld from %s.",guy, dums[guy].conf);
      fprintf(stderr,"\nDropping %ld from %s.",guy, dums[guy].conf);
      send_drop_voter(dums[guy].conf, guy+2000, NO);
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
  
  if(pid == CHILD)
    {
      execvp("./dummy",args);
      printf("\nexec failed:\n");
      for(i = 0; args[i]!=NULL;i++)
	printf("%s ",args[i]);;
      printf("\n");
      exit(1);
    }
  
  printf("\nFrom parent: ");
  for(i = 0; args[i]!=NULL;i++)
    printf("%s ",args[i]);
  printf("\n");
}


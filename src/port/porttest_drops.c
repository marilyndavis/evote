/* $Id: porttest_drops.c,v 1.3 2003/01/15 18:54:12 marilyndavis Exp $ */ 
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
  char command[200];
  
  unsigned long users = 100;
  short i, j;
  unsigned long guy;
  short last;
  
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
      printf("\nguy %d on %s",guy, dums[guy].conf);
    }

  /*  drop the users from the system */

  for (j = 1; j <= users; j++)
    {
      do   /* find one that has been online */
	{
	  guy = rand()%100 + 1;
	}
      while(dums[guy].on == NO);
      dums[guy].on = NO;
      printf("\nDropping %d from %s.",guy, dums[guy].conf);
      fprintf(stderr,"\nDropping %d from %s.",guy, dums[guy].conf);
      send_drop_voter(dums[guy].conf, guy+2000, NO);
    }
  
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


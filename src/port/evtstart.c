/* $Id: evtstart.c,v 1.3 2003/01/15 18:54:12 marilyndavis Exp $ */ 
/**************************************
     This test evtstart
     creates a conf, argv[1].
     drop_days == 0.
***********************************/
#include<stdio.h>  
#include<unistd.h>
#include<sys/types.h>

#include "../demo/eVote.h"

int main(int argc,char *argv[])
{
  OKorNOT cc;
  
  char conf[CONFLEN + 1];
  OKorNOT eVote_conf(char* name, short drop_days);
  
  start_up(NO);
  strcpy(conf,argv[1]);
  
  /****  CREATE CONF ****/
  printf("\nAbout to call eVote_conf %s.",conf);
  cc = eVote_conf(conf, 0);
  if(cc == OK)
    printf("\n\nUser %d created %s.",getuid(), conf);
  else
    printf("\n\nUser %d failed to create %s.",getuid(),conf);
  return 0;
}

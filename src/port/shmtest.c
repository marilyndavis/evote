/* $Id: shmtest.c,v 1.3 2003/01/15 18:54:12 marilyndavis Exp $ */ 
extern "C" {
#include <stdio.h>
}
#include<sys/msg.h>
#include<errno.h>


int main(void)
{
  int qid;
  int pid = 0;

  fprintf(stderr,"MSGMNI = %d", MSGMNI);

  while( (qid = msgget((key_t)++pid, IPC_CREAT|0600)) != -1)
    {
      fprintf(stderr, "Got qid %d with key %d.\n", qid, pid);
    }

  perror("Failed");
  fprintf(stderr,"\nFailed on qid %d with key %d.\n", qid,pid);
}

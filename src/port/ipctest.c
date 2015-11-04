/* $Id: ipctest.c,v 1.5 2003/01/15 18:54:12 marilyndavis Exp $ */ 
/************************************************************
 *   ipctest.c  This is a test of ipc for porting
 ************************************************************/
#include <stdio.h>
#include"../Clerklib/eVote_defaults.h"
#include<errno.h>

int max_msgs;
void clean_up(void);
int test_shmmni(int **, int);
int test_msgmni(int **);
void drop_queues(int * qids, int no_qs);
void drop_mems(int * qids, int no);
void drop_queue(int qid);
void drop_mem(int id);
int test_no_messages(int msg_len);
int test_max_msg_len(void);
#define LEN 4096

int 
main(int argc, char *argv[])
{
  int * qids;
  int no_qs;
  int * mids;
  int no_ms;
  int big_ms;
  int max_1messages;
  int max_10messages;
  int max_msg_len;

  if(argc > 1)
    {
      clean_up();
    }
  /* for(i = 0, no_ms = 32768; i < 128; no_ms++, i++)
     drop_mem(no_ms); */
  max_1messages = test_no_messages(1);
  max_10messages = test_no_messages(10);
  max_msg_len = test_max_msg_len();

  no_ms = test_shmmni(&mids, 1);
  drop_mems(mids, no_ms);  
  big_ms = test_shmmni(&mids, LEN);
  drop_mems(mids, no_ms);
  no_qs = test_msgmni(&qids);
  drop_queues(qids, no_qs); 
  printf("MSGTQL max 1 byte messages  = %d.\n", max_1messages);
  printf("       max 10 byte messages = %d.\n", max_10messages);
  printf("MSGMAX max message length = %d.\n", max_msg_len);
  printf("MSGMNI max number of message queues = %d.\n", no_qs);
  printf("SHMMNI max number of memsegs = %d.\n", no_ms);
  printf("       max number of %d memsegs = %d.\n", LEN, big_ms);
  return 0;
}
/************************************************************
 * void clean_up(void)
 *  Cleans up all the existing queues and memory segments
 *************************************************************/
void clean_up(void)
{
  int id;
  int the_key = 1;
  int failed = 0;
  
  do
    {
      if((id = msgget((key_t)(the_key), IPC_PRIVATE)) != -1)
	{
	  printf( "Got msg qid %d with key %d.\n", id, the_key);
	  drop_queue(id);
	  failed = 0;
	}
      else
	failed++;
      the_key++;
    }
  while(failed < 2);
  the_key = 1;
  failed = 0;
  do
    {
      if((id = shmget(the_key,
		     LEN,
		     IPC_PRIVATE)) != -1)
	{
	  printf( "Got shmid %d with key %d.\n", id, the_key);
	  drop_mem(id);
	  failed = 0;
	}
      else
	failed++;
      the_key++;
    }
  while(failed < 2);
  return;
}

/************************************************************
 * int test_msgmni(int ** p_ids)
 *  This makes as many message queues as it can.  It returns
 *  the successful keys in a list at p_id.  It returns the
 *  number of successful queues.
 ************************************************************/
int test_msgmni(int ** p_ids)
{
  static int qid[LEN];
  int the_key = 1;
  
  while( (qid[the_key -1] = msgget((key_t)(the_key), IPC_CREAT|0600)) != -1)
    {
      printf( "Got qid %d with key %d.\n", qid[the_key - 1], the_key);
      the_key++;
    }
  perror("Failed to make a queue");
  printf("Errno = %d.\n", errno);
  printf("\nFailed on q and key %d.\n", the_key);
  *p_ids = qid;
  return the_key -1;
}
/************************************************************
 * int test_no_messages(int size)
 *  Establishes one queue and sends messages, each size bytes,
 *  until no more are allowed.
 *  It removes the queue and returns the number of messages
 *  allowed.
 * 
 ************************************************************/
int test_no_messages(int size)
{
  struct 
  {
    int mtype;
    char mtext[LEN];
  }spacer;
  int the_key = 1;
  struct msgbuf * sysmsg = (struct msgbuf *)&spacer;
  int i, qid;
  sysmsg->mtype = 10;
  
  if( (qid = msgget((key_t)(the_key), IPC_CREAT|0600)) == -1)
    {
      perror("Couldn't make one queue for max_no_messages");
      printf("Errno = %d.\n", errno);
      return 0;
    }
  printf("Got qid %d with key %d.\n", qid, the_key);
  
  for(i = 0; i < size; i++)
    sysmsg->mtext[i] = '0'+ i;
  
  for(i = 0; i < LEN * 10; i++) 
    {
      if(msgsnd(qid, sysmsg, size, IPC_NOWAIT) == -1)
	{
	  perror("Failed to send a 1 byte message.");
	  printf("Errno = %d.\n", errno);
	  break;
	}
      printf("Sent message # %d of %d chars.\n",
	      i+1, size);
    }
  
  drop_queue(qid);
  return i;
}
/************************************************************
 *
 *  int test_max_msg_len(void)
 *   Sets up a queue and sends and receives messages, each
 *   time doubling the size, until it's not allowed.  It 
 *   returns the maximum number of bytes allowed.
 ************************************************************/
int test_max_msg_len(void)
{
  long priority = - 1000l;
  struct 
  {
    int mtype;
    char mtext[LEN];
  }spacer;
  int the_key = 1;
  struct msgbuf * sysmsg = (struct msgbuf *)&spacer;
  int i, j, k, qid, len;
  sysmsg->mtype = 10;
  
  if( (qid = msgget((key_t)(the_key), IPC_CREAT|0600)) == -1)
    {
      perror("Couldn't make queue for test_max_msg_len.");
      printf("errno = %d\n", errno);
      return -1;
    }
  printf( "Got qid %d with key %d.\n", qid, the_key);

  for(i = 1, j= 1; i < LEN * 10; i*=2, j++)
    {
      for(k = 0; k < i; k++)
	{
	  sysmsg->mtext[k] = '0'+ k % 10;
	}
      
      if(msgsnd(qid, sysmsg, i, IPC_NOWAIT) == -1)
	{
	  perror("Failed to send");
	  printf("errno = %d\n", errno);
	  break;
	}
      printf("Sent message # %d of %d chars.\n",
	      j, i);
      if ((len = msgrcv(qid, sysmsg, LEN + sizeof(long),
			priority, IPC_NOWAIT)) == -1)
	{
	  perror("Failed to receive");
	  printf("errno = %d\n", errno);
	  break;
	}
      printf("Received message # %d of %d chars.\n",
	      j, i);
    } 
  drop_queue(qid);
  return i/2;
}
/************************************************************
 *  void drop_queues(int * qids, int no)
 *    This drops the no queues whose id's are in the list.
 ************************************************************/
void drop_queues(int * qids, int no)
{
  int i;
  for(i = 0; i < no; i++, qids++)
    {
      drop_queue(*qids);
    }
}
/************************************************************
 *void drop_queue(int qid)
 *  Drops one message queue.
 *************************************************************/
void drop_queue(int qid)
{
  struct msqid_ds buf;
  
  if (msgctl(qid, IPC_STAT, &buf) == -1)
    {
      perror("Failed msgctl ");
    }
   if ( msgctl(qid, IPC_RMID, &buf) == -1)
    {
      perror("Failed remove ");
    }
  printf("msg queue id %d removed.\n", qid);
}
/************************************************************
 *  int test_shmmni(int ** pid, int len)
 *   Makes as many shared memory segments as it can of length = len
 *   and returns all the id's int a list at pid.
 *   Returns the number of segments created.
 ************************************************************/
int test_shmmni(int ** pid, int len)
{
  key_t the_key = 1;
  static int id[LEN];
  
  while((id[the_key-1] = shmget(the_key,
				len,
				IPC_CREAT|0666)) != -1)
    {
      printf( "Got shmid %d with key %d and length %d.\n", id[the_key-1], the_key, len);
      the_key++;
    }
  perror("Failed: ");
  *pid = id;
  return the_key -1;
}
/************************************************************
 *void drop_mems(int * memids, int no)
 * drops all the shared message segments in the list
 *************************************************************/
void drop_mems(int * memids, int no)
{
  int i;
  for(i = 0; i < no; i++, memids++)
    {
      drop_mem(*memids);
    }
}
/************************************************************
 * void drop_mem(int id)
 *   drops one shared memory with id
 *************************************************************/
void drop_mem(int id)
{
  struct shmid_ds control;

  if(shmctl(id, IPC_STAT, &control) == -1)
    {
      perror("shmctl failed");
      return;
    }

  if(shmctl(id, IPC_RMID, NULL) == -1)
    {
      perror("shm remove failed");
      return;
    }
  printf("Removed mem id %d.\n", id);
}

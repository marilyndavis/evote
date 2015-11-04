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

/* $Id: ipctest.c,v 1.3 2003/01/15 18:54:13 marilyndavis Exp $ */ 
/************************************************************
 *  ipctest.c
 *    gcc -o ipctest ipctest.c
 *    Run this test to find the parameters of your ipc setup.
 *********************************************************
 **********************************************************/
#include <stdio.h>
#include<sys/types.h>
#include<sys/ipc.h>
#include<sys/msg.h>
#include<sys/shm.h>
#include<errno.h>
#define LEN 4096
int max_msgs;
int test_shmmni(int **);
int test_msgmni(int **);
void drop_queues(int * qids, int no_qs);
void drop_mems(int * qids, int no);
void drop_queue(int qid);
void drop_mem(int id);
int test_no_messages(int msg_len);
int test_max_msg_len(void);
int
main(void)
{
  int * qids;
  int no_qs;
  int * mids;
  int no_ms;
  int i;
  int max_1messages;
  int max_10messages;
  int max_msg_len;
  /* for (i = 0, no_ms = 32768; i < 128; no_ms++, i++)
     drop_mem(no_ms); */
  max_1messages = test_no_messages(1);
  max_10messages = test_no_messages(10);
  max_msg_len = test_max_msg_len();
  no_ms = test_shmmni(&mids);
  no_qs = test_msgmni(&qids);
  drop_queues(qids, no_qs); 
  drop_mems(mids, no_ms);  
  printf("MSGTQL max 1 byte messages  = %d.\n", max_1messages);
  printf("       max 10 byte messages = %d.\n", max_10messages);
  printf("MSGMAX max message length = %d.\n", max_msg_len);
  printf("MSGMNI max number of message queues = %d.\n", no_qs);
  printf("SHMMNI max number of memsegs = %d.\n", no_ms);
  return 0;
}
int
test_msgmni(int ** p_ids)
{
  static int qid[LEN];
  int the_key = 0;
  while ((qid[the_key] = msgget((key_t)(the_key), IPC_CREAT|0600)) != -1)
    {
      fprintf(stderr, "Got qid %d with key %d.\n", qid, the_key);
      the_key++;
    }
  perror("Failed to make a queue");
  fprintf(stderr,"Errno = %d.\n", errno);
  fprintf(stderr,"\nFailed on qid %d with key %d.\n", qid[the_key], the_key);
  *p_ids = qid;
  return the_key;
}
int
test_no_messages(int size)
{
  long priority = - 1000l;
  struct 
  {
    int mtype;
    char mtext[LEN];
  }spacer;
  int the_key = 0;
  struct msgbuf * sysmsg = (struct msgbuf *)&spacer;
  int i, qid;
  sysmsg->mtype = 10;
  if ((qid = msgget((key_t)(the_key), IPC_CREAT|0600)) == -1)
    {
      perror("Couldn't make one queue for max_no_messages");
      fprintf(stderr,"Errno = %d.\n", errno);
      return;
    }
  fprintf(stderr, "Got qid %d with key %d.\n", qid, the_key);
  for (i = 0; i < size; i++)
    sysmsg->mtext[i] = '0'+ i;
  for (i = 0; i < LEN * 10; i++) 
    {
      if (msgsnd(qid, sysmsg, size, IPC_NOWAIT) == -1)
	{
	  perror("Failed to send a 1 byte message.");
	  fprintf(stderr,"Errno = %d.\n", errno);
	  break;
	}
      fprintf(stderr,"Sent message # %d of %d chars.\n",
	      i+1, size);
    }
  drop_queue(qid);
  return i;
}
int
test_max_msg_len(void)
{
  long priority = - 1000l;
  struct 
  {
    int mtype;
    char mtext[LEN];
  }spacer;
  int the_key = 0;
  struct msgbuf * sysmsg = (struct msgbuf *)&spacer;
  int i, qid, len;
  sysmsg->mtype = 10;
  if ((qid = msgget((key_t)(the_key), IPC_CREAT|0600)) == -1)
    {
      perror("Couldn't make queue for test_max_msg_len.");
      fprintf(stderr,"errno = %d\n", errno);
      return -1;
    }
  fprintf(stderr, "Got qid %d with key %d.\n", qid, the_key);
  for (i = 0; i < LEN * 10; i++)
    {
      sysmsg->mtext[i] = '0'+ i % 10;
      if (msgsnd(qid, sysmsg, i + 1, IPC_NOWAIT) == -1)
	{
	  perror("Failed to send");
	  fprintf(stderr,"errno = %d\n", errno);
	  break;
	}
      fprintf(stderr,"Sent message # %d of %d chars.\n",
	      i+1, i+1);
      if ((len = msgrcv(qid, sysmsg, LEN + sizeof(long),
			priority, IPC_NOWAIT)) == -1)
	{
	  perror("Failed to receive");
	  fprintf(stderr,"errno = %d\n", errno);
	  break;
	}
      fprintf(stderr,"Received message # %d of %d chars.\n",
	      i+1, i+1);
    } 
  drop_queue(qid);
  return i;
}
void
drop_queues(int * qids, int no)
{
  int i;
  for (i = 0; i < no; i++, qids++)
    {
      drop_queue(*qids);
    }
}
void
drop_queue(int qid)
{
  struct msqid_ds buf;
  if (msgctl(qid, IPC_STAT, &buf) == -1)
    {
      perror("Failed msgctl ");
    }
   if (msgctl(qid, IPC_RMID, &buf) == -1)
    {
      perror("Failed remove ");
    }
  fprintf(stderr,"msg queue id %d removed.\n", qid);
}
int
test_shmmni(int ** pid)
{
  key_t the_key = 0;
  static int id[LEN];
  while ((id[the_key] = shmget(the_key,
			LEN,
			IPC_CREAT|0666)) != -1)
    {
      fprintf(stderr, "Got shmid %d with key %d.\n", id[the_key], the_key);
      the_key++;
    }
  perror("Failed: ");
  *pid = id;
  return the_key;
}
void
drop_mems(int * qids, int no)
{
  int i;
  for (i = 0; i < no; i++, qids++)
    {
      drop_mem(*qids);
    }
}
void
drop_mem(int id)
{
  struct shmid_ds control;
  if (shmctl(id, IPC_STAT, &control) == -1)
    {
      perror("shmctl failed");
      return;
    }
  if (shmctl(id, IPC_RMID, NULL) == -1)
    {
      perror("shm remove failed");
      return;
    }
  fprintf(stderr,"Removed mem id %d.\n", id);
}

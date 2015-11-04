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

/* $Id: msgtest.c,v 1.3 2003/01/15 18:54:12 marilyndavis Exp $ */ 

#include <stdio.h>
#include<sys/types.h>
#include<sys/ipc.h>
#include<sys/msg.h>
#include<sys/shm.h>
#include<errno.h>
#define LEN 4096

int test_shmmni(int **);
int test_msgmni(int **);
void test_msgtql(int qid);
void drop_queues(int * qids, int no_qs);
void drop_mems(int * qids, int no);
void drop_queue(int qid);
void drop_mem(int id);
int main(void)
{
  int * qids;
  int no_qs;
  int * mids;
  int no_ms;

  no_ms = test_shmmni(&mids);
  no_qs = test_msgmni(&qids);
  drop_queues(qids, no_qs); 
  drop_mems(mids, no_ms);
  return 0;
}

int test_msgmni(int ** p_ids)
{
  static int qid[LEN];
  int the_key = 0;
  
  while( (qid[the_key] = msgget((key_t)(the_key), IPC_CREAT|0600)) != -1)
    {
      fprintf(stderr, "Got qid %d with key %d.\n", qid, the_key);
      if(the_key == 0)
	{
	  test_msgtql(qid[the_key]);
	}
      the_key++;
    }
  perror("Failed");
  fprintf(stderr,"\nFailed on qid %d with key %d.\n", qid[the_key],the_key);
  *p_ids = qid;
  return the_key;
}
void test_msgtql(int qid)
{
  struct msgbuf sysmsg;
  int i;
  sysmsg.mtype = 10;

  for(i = 0; i < LEN; i++)
    {
      sysmsg.mtext[i] = '0'+ i % 10;

      if(msgsnd(qid, &sysmsg, i + 1, IPC_NOWAIT) == -1)
	{
	  perror("Failed");
	  return;
	}
      fprintf(stderr,"Sent message # %d of %d chars.\n",
	      i+1, i+1);
    }
}
void drop_queues(int * qids, int no)
{
  int i;
  for(i = 0; i < no; i++, qids++)
    {
      drop_queue(*qids);
    }
}

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
  fprintf(stderr,"msg queue id %d removed.\n", qid);
}

int test_shmmni(int ** pid)
{
  key_t the_key = 0;
  static int id[LEN];
  
  while((id[the_key] = shmget(the_key,
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
void drop_mems(int * qids, int no)
{
  int i;
  for(i = 0; i < no; i++, qids++)
    {
      drop_mem(*qids);
    }
}

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
  fprintf(stderr,"Removed mem id %d.\n", id);
}

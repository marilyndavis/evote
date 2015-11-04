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

/* $Id: ipc_shm.c,v 1.6 2003/01/15 18:54:08 marilyndavis Exp $ */ 
/**********************************************************
 *	 ../eVote/src/Clerklib/ipc_shm.c
 *   This is the info class.  It is responsible for
 *   maintaining the item_info array kept in shared
 *   memory.
 *
 *   Makes calls to the Clerk.  
 **********************************************************/ 
/*********************************************************
 *
 *    Copyright (c) 1994...2015 Deliberate.com
 *		Patented.
 *
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the eVote(R)/Clerk License as
 *  published by Deliberate.Com.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  eVote(R)/Clerk License for more details.
 *
 *  You should have received a copy of the eVote(R)/Clerk License
 *  along with this program in Chapter VII of ../../doc/eVote.doc. 
 *  If not, write to Deliberate.Com 2555 W. Middlefield Rd., Mountain
 *  View, CA 94043 USA or office@deliberate.com.
 **********************************************************/
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include "eVote_defaults.h"
#include "Clerkdef.h"
#include "Clerk.h"
#include "ipc_msg.h" 
#include <sys/stat.h>
#include <unistd.h>
#include <stdlib.h>
#include <time.h>
#include "msgdef.h"
#ifdef EDEBUG
extern char debugger[];
extern int edebug;
#define FLOWER 8
extern YESorNO trace;
void output(char *);
#endif

/*  These are maintained here but shared throughout */

ITEM_INFO *item_info;
short * p_no_items;
short current_drop_days;
extern unsigned long no_of_participants;

/* maintained by ipc_msg.c */

int * p_shmids = NULL;
extern char * pw_name(int);

/* PRIVATE to this file */

static char* p_where = NULL;
static int memid;

void 
drop_info(void)
{
#ifdef EDEBUG
  if (trace == YES)
    {
      printf("\n%d: dropping info at %p. memid = %d",
	     getpid(), p_where, memid);
    }
#endif
  if (p_where != NULL)
    {
#ifdef EDEBUG
      if (edebug & FLOWER)
	{
	  sprintf(debugger,
		  "echo %lu:Appli: detaching memory",
		  who_am_i());
	  output(debugger);
	}
#endif					

      shmdt(p_where);
    }
  p_where = NULL;
  p_no_items = NULL;
  item_info = NULL;
}
OKorNOT 
start_info(int new_mid)
{
  int er, count = 0;
  
#ifdef EDEBUG
  if (edebug & FLOWER)
    {
      sprintf(debugger,
	      "echo %lu:%d:Appli: start_info at %p. old memid = %d. New_mid = %d",
	      who_am_i(), getpid(), p_where, memid, new_mid);
      output(debugger);
    }
  if (trace == YES)
    {
      printf("\n%d: entering start_info at %p. memid = %d. New_mid = %d",
	     getpid(),p_where, memid, new_mid);
    }
#endif
  memid = new_mid;

#ifdef EDEBUG
  if (edebug & FLOWER)
    {
      sprintf(debugger,
	      "echo %lu:%d:Appli: attach memid = %d",
	      who_am_i(), getpid(), new_mid);
      output(debugger);
    }
#endif
  p_where = (char*)shmat(new_mid,(char*)0,0);
  if ((int)p_where == -1)
    {
      if ((er = errno) != EINVAL)
	fprintf(stderr,"\nUnable to attach shared memory:");
#ifdef EDEBUG
      if (edebug & FLOWER)
	{
	  sprintf(debugger,
		  "echo %lu:%d:Appli: attach memid = %d failed errno = %d",
		  who_am_i(), getpid(), new_mid, errno);
	  output(debugger);
	}
#endif
      switch(errno)
	{
	case EACCES:
	  fprintf(stderr," forbidden by protection.\n");
	  break;
	case EMFILE:
	  fprintf(stderr," too many online.\n");
	  break;
	case EINVAL:
	  if (get_msg(NO) == 0 || in_msg_buffer->mtype != NEW_MID)
	    {
	      fprintf(stderr,"\nUnable to attach shared memory:");
	      fprintf(stderr," identifier %d is not valid.\n",new_mid);
	    }
	  else /* probably OK and set up redundantly */
	    { 
	      return OK;
	    }
	  break;
	default:
	  fprintf(stderr," errno = %d", er);
	  break;
	}
      return NOT_OK;
    }
  while (*(short*)p_where == 1 && ++count < 10)
    {
      sleep(1);
    }

  if (*(short*)p_where == 1)
    {
      fprintf(stderr,"Can't get into memory ... locked.\n");

#ifdef EDEBUG
      if (edebug & FLOWER)
	{
	  sprintf(debugger,
		  "echo %lu:%d:Appli: Can't get into memory %d ... locked.",
		  who_am_i(), getpid(), memid);
	  output(debugger);
	}
#endif
      return NOT_OK;
    }

  p_no_items = (short*)(p_where + sizeof(short));
  current_drop_days = *(short*)(p_where+2*sizeof(short));
  item_info = (ITEM_INFO*)(p_where + 3*sizeof(short));
  
#ifdef EDEBUG
  if (edebug & FLOWER)
    {
      sprintf(debugger,
	      "echo %lu:%d:Appli: Exiting start_info at %p. memid = %d no_items = %d",
	      who_am_i(), getpid(), p_where, memid, *p_no_items);
      output(debugger);
    }
  if (trace == YES)
    {
      printf("\n%d: exiting start_info at %p. memid = %d.",
	     getpid(), p_where, memid);
      printf("\n%d:   No items = %d.",getpid(),*p_no_items);
    }
#endif
  
  return OK;
}
/**********************************************
 *
 *   void new_memory(char* input)
 *
 *     This is called directly from get_msg when it
 *     finds a NEW_MID instruction.  It means that
 *     the item list has outgrown its old memory slot
 *     and a new one has been set up.
 *****************************************************/
void 
new_memory(char* input)
{
  int len;
  int new_mid;
  
  sscanf(input,FEN_NEW_MID, ENN_NEW_MID);
#ifdef EDEBUG
  if (edebug & FLOWER)
    {
      sprintf(debugger,
	      "echo %lu:%d:Appli: new_memory: memid = %d, old is %d",
	      who_am_i(), getpid(), new_mid, memid);
      output(debugger);
    }
  if (trace == YES)
    {
      printf("\n%d: entering new_mid = %d. old memid = %d. ",
	     getpid(),  new_mid, memid);
    }
#endif
  if (p_where != NULL)
    {
#ifdef EDEBUG
      if (edebug & FLOWER)
	{
	  sprintf(debugger,
		  "echo %lu:%d:Appli: detaching memid = %d",
		  who_am_i(), getpid(), memid);
	  output(debugger);
	}
#endif
      shmdt(p_where);
      out_msg_buffer->mtype = PR_MID_DROPPED;
      (void)sprintf(out_msg_buffer->mtext,FNE_MID_DROPPED,NNE_MID_DROPPED);
      len = (int)strlen(out_msg_buffer->mtext);
      send_inst(len,NO);
    }
  if (start_info(new_mid) == NOT_OK)
    {
      fprintf(stderr,"\nCan't open memory for id = %d on pid = %d for user = %ld",
	      new_mid, getpid(), who_am_i());
      fprintf(stderr,"\nMust exit eVote.\n");
      exit(0);
    }
#ifdef EDEBUG
  if (edebug & FLOWER)
    {
      sprintf(debugger,
	      "echo %lu:%d:Appli: set with new memory at %p and id %d",
	      who_am_i(), getpid(), p_where, memid);
      output(debugger);
    }
  if (trace == YES)
    {
      printf("\n%d: exiting new_mid = %d. memid = %d. ",
	     getpid(),  new_mid, memid);
    }
#endif
}		
/*********************************************************
 *   void report_mems(int this_Clerk_pid, int other_Clerk_pid)
 *
 *   Slow function to report all shared memory segments.
 *   Let me know if you know a faster way.
 *********************************************************/
void 
report_mems(int this_Clerk_pid, int other_Clerk_pid)
{
  int  cc;
  int count = 0;
  int memid;
  struct shmid_ds membuf;
  int get_id(int ** pp_ids);
  
  while ((memid = get_id(&p_shmids)) != -1)
    {
      if ((cc = shmctl(memid, IPC_STAT, &membuf)) == -1)	
	{
	  continue;
	}
      printf("\nInspecting memory segment %d, key %d, %d bytes",
	     memid, membuf.shm_perm.key, membuf.shm_segsz);
      printf("\n  created by user %d = %s, group %d, perm = %o:",
	     membuf.shm_perm.cuid, pw_name(membuf.shm_perm.cuid),
	     membuf.shm_perm.cgid, 
	     membuf.shm_perm.mode);
      if (membuf.shm_cpid == this_Clerk_pid)
	{
	  count++;
	}
      if (membuf.shm_cpid == other_Clerk_pid)
	{
	  printf("\n  This segment was created by another Clerk!\n");
	}
      printf("\n  Last attach at %s",ctime(&membuf.shm_atime));
      printf("  Last detach at %s",ctime(&membuf.shm_dtime));
      printf("  Last change at %s",ctime(&membuf.shm_ctime));
    }
  if (count == 1)
    printf("\nThere is 1 shared memory segment created by this Clerk.\n\n");
  else
    printf("\nThere are %d shared memory segments created by this Clerk.\n\n",count);
}
/*********************************************************
 *   void stop_mems(int this_Clerk_pid, int other_Clerk_pid)
 *
 *   Slow function to drop all shared memory segments.
 *   Let me know if you know a faster way.
 *********************************************************/
void 
stop_mems(int this_Clerk_pid, int other_Clerk_pid)
{
  int cc;
  int count = 0;
  int memid;
  int get_id(int **pp_ids);
  struct shmid_ds membuf;
  
  /*	printf("\nDropping all superfluous shared memory segments!\n");	*/
  while ((memid = get_id(&p_shmids)) != -1)
    {
      if ((cc = shmctl(memid, IPC_STAT, &membuf)) == -1)	
	{
	  if (errno == EACCES
	     || errno == EPERM)
	    {
	      printf("\nYou do not have appropriate permission to see memid = %d.\n", memid);
	      exit(0);
	    }
	  continue;
	}
      if (membuf.shm_cpid == other_Clerk_pid 
	  || (this_Clerk_pid != 0  /* 0 if dead and no queues */
	      && membuf.shm_cpid != this_Clerk_pid ))
	{
	  continue;
	}

      cc = shmctl(memid, IPC_RMID, NULL);
      /*      printf("\n  There are %ld processes attached!",
	     (long)membuf.shm_nattch);
      printf("\n  Last attach at %s",ctime(&membuf.shm_atime));
      printf("  Last detach at %s",ctime(&membuf.shm_dtime));
      printf("  Last change at %s",ctime(&membuf.shm_ctime));*/
      if (cc == -1)
	{
	  switch (errno)
	    {
	    case EINVAL:
	      printf("  Can't remove memory %d -- must be gone.",memid);
	      printf("\n  The memid %d is not a valid identifier.",memid);
	      break;
	    case EPERM:
	      /*	      printf("  Leaving this one alone.\n");*/
	      break;
	    case EFAULT:
	      printf("  Needs better than a NULL as last argument.");
	      break;
	    }
	}
      else
	count++;
    }
  if (count == 1)
    printf("\nRemoved 1 shared memory segment.\n");
  else if (count > 0)
    printf("\nRemoved %d shared memory segments.\n",count);
}


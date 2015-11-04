/* $Id: memlist.cc,v 1.4 2003/01/15 18:54:07 marilyndavis Exp $ */ 
// memlist.cc -- manages the one list of memory segments for
// the entire application.
/*********************************************************
 *    Copyright (c) 1994...2015 Deliberate.com Patented.
 *    by Marilyn Davis
 **********************************************************/
#include <stdlib.h>
#include <iostream.h>
#include <fstream.h>
#include <sys/types.h>
#include <sys/shm.h>
#include <stdio.h>
#include <errno.h>
#include "evotedef.h"
#include"memlist.h"
#include"memseg.h"
MemList memlist;
#ifdef EDEBUG
#include"debug.h"
#endif
extern int shmmni;
extern long now;
extern ofstream *logger;
//************************************************************
OKorNOT
MemList::deactivate(MemSeg * memseg)
{
  MemSeg * seg = active, *last = active;
  static int count;
  count++;
#ifdef EDEBUG
  if (edebug & FLOWER && edebug & SMEM)
    {
      sprintf(debugger,
	      "echo Clerk: Deactivating memseg id = %d.", memseg->_memid);
      doutput(debugger);
      list_segs();
    }
#endif
  while (seg)
    {
#ifdef EDEBUG
      if (edebug & FLOWER && edebug & SMEM)
	{
	  sprintf(debugger,
		  "echo Clerk: This one has id = %d.", seg->_memid);
	  doutput(debugger);
	}
#endif
      if (seg == memseg)
	{
	  break;
	}
      last = seg;
      seg = seg->next;
    }
  if (!seg)
    {
#ifdef EDEBUG
      if (edebug & FLOWER && edebug & SMEM)
	{
	  sprintf(debugger,
		  "echo Clerk: Didn't find memseg id = %d.", memseg->_memid);
	  doutput(debugger);
	}
#endif
      return NOT_OK;
    }
  if (seg == active)
    active = seg->next;
  else
    last->next = seg->next;
  seg->next = inactive;
  inactive = seg;
#ifdef EDEBUG
  if (edebug & FLOWER && edebug & SMEM)
    {
      sprintf(debugger,
	      "echo Clerk: Deactivate finished.");
      doutput(debugger);
      list_segs();
    }
#endif
  return mid_dropped(seg->_memid);
}
//************************************************************
// Very public function that tries to drop all the old
// inactive mem_segs.
// Returns the number of queues dropped.
int
MemList::drop_old_segs(void)
{
  MemSeg * seg = inactive, *next;
  int count = 0;
#ifdef EDEBUG
  if (edebug & FLOWER && edebug & SMEM)
    {
      sprintf(debugger,
	      "echo Clerk: Dropping old memsegs.");
      doutput(debugger);
    }
#endif
  while (seg)
    {
#ifdef EDEBUG
      if (edebug & FLOWER && edebug & SMEM)
	{
	  sprintf(debugger,
		  "echo Clerk: Trying %d.", seg->_memid);
	  doutput(debugger);
	}
#endif
      next = seg->next;
      if (mid_dropped(seg->memid()) == OK)
	{
	  count++;
	}
      seg = next;
    }
  return count;
}
// ************************************************************
// Gets a new id for the MemSeg with room for for_items items
OKorNOT
MemList::getid(MemSeg * seg, short for_items)
{
  key_t the_key = ++last_key;
  struct shmid_ds control;
  int tries = 0, newid;
  int cc, er;
#ifdef EDEBUG
  if (edebug & FLOWER && edebug & SMEM)
    {
      sprintf(debugger,"echo Clerk: Creating shared memory for %d bytes.", 
	      ITEMS_TO_BYTES(for_items));
      doutput(debugger);
    }
#endif
  do
    {
      if ((newid = shmget(the_key,
			  (cc = ITEMS_TO_BYTES(for_items)),
			  IPC_CREAT|0666)) != -1)
	{
	  ++mem_segs;
	  seg->next = active;
	  seg->space = for_items;
	  active = seg;
	  seg->_memid = newid;
	  seg->key = the_key;
#ifdef EDEBUG
	  if (edebug & FLOWER && edebug & SMEM)
	    {
	      sprintf(debugger,"echo Clerk: Creating id = %d key = %d mem_segs = %d",
		      newid, the_key, mem_segs);
	      doutput(debugger);
	      list_segs();
	    }
	  if (edebug & LIMITS || edebug & ADJOURNS)
	    {
	      dlog << "New memory segment with "
		   << newid << ".";
	      dlog << "   mem_segs = " << mem_segs 
		   << ". Limit is " << shmmni;
	    }
	  if (edebug & MEMS)
	    {
	      char mes[200];
	      sprintf(mes, "Created shared memory of %d bytes. id = %d", 
		      ITEMS_TO_BYTES(for_items),
		      newid);
	      vmem(mes, NO);
	    }
#endif
	  return OK;
	}
      er = errno;
#ifdef EDEBUG
      if (edebug & FLOWER && edebug & SMEM)
	{
	  sprintf(debugger,"echo Clerk: Failed with errno = %d", er);
	  doutput(debugger);
	}
#endif
      switch (er)
	{
	case EEXIST:
	  clerklog << "MemList::getid:Memory already exists for key = " << the_key;
	  break;
	case EINVAL:
	  clerklog << "MemList::getid on " << the_key 
		   << " returned bad for_items = " << cc 
		   << "\n  perhaps for existing shared memory segment.";
	  clerklog << "Start the Clerk with no old memory segments existing.";
	  exit(0);
	  break;
	case ENOSPC:
	  clerklog << "The system tunable parameter, SHMMNI needs to be higher.";
	  exit(0);
	  break;
	case EAGAIN:
	case ENOMEM:
	  clerklog << "MemList::getid:No system resources.  Coming down.";
	  exit(0);
	  break;
	default:
	  clerklog << "MemList::getid: Unable to get shared memory, errno = " << errno;
	  exit(0);
	  break;
	}
#ifdef EDEBUG
      if (edebug & FLOWER && edebug & SMEM)
	{
	  sprintf(debugger,"echo Clerk: Trying again.");
	  doutput(debugger);
	}
#endif
      //  here, memory already exists
      newid = shmget(the_key, 0, 0);
      if (newid == -1)
	continue;
      if (shmctl(newid, IPC_RMID, &control) != 0)
	{
	  newid = -1;
	  continue;
	}
    }
  while (newid == -1 && ++tries < 3);
#ifdef EDEBUG
  if (edebug & FLOWER && edebug & SMEM)
    {
      sprintf(debugger,"echo Clerk: Giving up.");
      doutput(debugger);
    }
#endif
  return NOT_OK;
}
// ************************************************************
// Gets a new bigger memory segment for the MemSeg and copies the
// the data to the new segment
MemSeg *
MemList::grow(MemSeg * oldseg, 
	      short new_no_items)
{
  char * old_p_where, * new_p_where;
  MemSeg * newseg;
  int new_space;
  new_space = new_no_items + (BUNCH - new_no_items % BUNCH);
#ifdef EDEBUG
  if (edebug & FLOWER && edebug & SMEM)
    {
      sprintf(debugger,"echo Clerk: Growing id = %d to newspace = %d",
	      oldseg->_memid, new_space);
      doutput(debugger);
      list_segs();
    }
#endif
  if ((old_p_where = oldseg->attach()) == NULL)
    {
      return NULL;
    }
#ifdef EDEBUG
  if (edebug & FLOWER && edebug & SMEM)
    {
      sprintf(debugger,"echo Clerk: Growing id = %d from  %d to newspace = %d",
	      oldseg->_memid, oldseg->no_items, new_space);
      doutput(debugger);
    }
#endif
  newseg = new MemSeg(new_space);
  if ((new_p_where = newseg->attach()) == NULL)
    {
      return NULL;
    }
  memcpy(new_p_where, old_p_where, ITEMS_TO_BYTES(oldseg->no_items));
  oldseg->close();
  newseg->close();
  // newseg is now old and inactive 
  deactivate(oldseg);
#ifdef EDEBUG
  if (edebug & FLOWER && edebug & SMEM)
    {
      sprintf(debugger,"echo Clerk: grow and swap finished.");
      doutput(debugger);
      list_segs();
    }
#endif
  return newseg;
}
//************************************************************
// This is called when the user switches from the
// old memseg to the new one, or when the user goes
// off line.
// It tries checks if there are no attaches left and
// drops it if it can.
OKorNOT
MemList::mid_dropped(int shmid)
{
  MemSeg * seg = inactive, *last = inactive;
  struct shmid_ds control;
  int er;
  static int count;
  count++;
#ifdef EDEBUG
  if (edebug & FLOWER && edebug & SMEM)
    {
      sprintf(debugger, "echo Clerk: MemList::mid_dropped called on %d",
	      shmid);
      doutput(debugger);
      list_segs();
    }
#endif
  while (seg)
    {
#ifdef EDEBUG
      if (edebug & FLOWER && edebug & SMEM)
	{
	  sprintf(debugger,"echo Clerk: See %d", seg->_memid);
	  doutput(debugger);
	}
#endif
      if (seg->_memid == shmid)
	{
	  break;
	}
      last = seg;
      seg = seg->next;
    }
  if (!seg)
    {
#ifdef EDEBUG
      if (edebug & FLOWER && edebug & SMEM)
	{
	  sprintf(debugger,"echo Clerk: Couldn't find %d", shmid);
	  doutput(debugger);
	}
#endif
      return NOT_OK;
    }
  if (seg->attaches == -1  /* first drop */
      || --seg->attaches <= 0)
    {
#ifdef EDEBUG
      if (edebug & FLOWER && edebug & SMEM)
	{
	  sprintf(debugger,"echo Clerk: ipc_stating %d", shmid);
	  doutput(debugger);
	}
#endif
      if (shmctl(shmid, IPC_STAT, &control) == -1)
	{
	  er = errno;
#ifdef EDEBUG
	  if (edebug & FLOWER && edebug & SMEM)
	    {
	      sprintf(debugger,
		      "echo Clerk: Clerk mid %d failed inspection, errno = %d", 
		      shmid, er);
	      doutput(debugger);
	    }
#endif
	  if (er == EINVAL)
	    return UNDECIDED;
	  clerklog << "Shared memory unexpectedly gone on id = " << shmid;
	  switch (er)
	    {
	    default:
	      clerklog << "Errno = " << er;
	      break;
	    }
	  return NOT_OK;
	}
      if (control.shm_nattch > 0)
	{
	  seg->attaches = control.shm_nattch;
#ifdef EDEBUG
	  if (edebug & FLOWER && edebug & SMEM)
	    {
	      sprintf(debugger,
		      "echo Clerk: Clerk postponing %d -- still %ld attached.", 
		      shmid, (long)control.shm_nattch);
	      list_segs();
	      doutput(debugger);
	    }
#endif
	  return UNDECIDED;
	}
    }
#ifdef EDEBUG
  if (edebug & FLOWER && edebug & SMEM)
    {
      sprintf(debugger,
	      "echo Clerk: Clerk removing mid %d", 
	      shmid);
      doutput(debugger);
    }
#endif
  if (shmctl(shmid, IPC_RMID, NULL) == -1)
    {
      er = errno;
#ifdef EDEBUG
      if (edebug & FLOWER && edebug & SMEM)
	{
	  sprintf(debugger,
		  "echo Clerk: Clerk removing mid %d failed errno = %d", 
		  shmid, er);
	  doutput(debugger);
	}
#endif
      switch (er)
	{
	case EINVAL:
	  break;
	default:
	  clerklog << "Unable to drop shared memory: shmid = " << shmid
		   << ". Errno = " << er;
	  break;
	}
      return NOT_OK;
    }
  else
    {
      mem_segs--;
#ifdef EDEBUG
      if (edebug & FLOWER && edebug & SMEM)
	{
	  sprintf(debugger,
		  "echo Clerk: Clerk removing mid %d succeeded, now mem_segs == %d", 
		  shmid, mem_segs);
	  doutput(debugger);
	  list_segs();
	}
      if (edebug & LIMITS || edebug & ADJOURNS)
	{
	  dlog << "Dropped memory segment for "
	       << " id = " << shmid << ".";
	  dlog << "   mem_segs = " << mem_segs 
	       << ". Limit is " << shmmni;
	}
#endif
      if (seg == inactive)
	{
	  inactive = seg->next;
	}
      else
	{
	  last->next = seg->next;
	}
      delete seg;
      return OK;
    }
}
//************************************************************
void
MemList::swapsegs(MemSeg * pinactive, MemSeg * pactive)
{
  char tmp[sizeof(MemSeg)];
#ifdef EDEBUG
  if (edebug & FLOWER && edebug & SMEM)
    {
      sprintf(debugger,"Before swapsegs");
      doutput(debugger);
      list_segs();
    }
#endif
  memcpy(tmp, pinactive, sizeof(MemSeg));
  memcpy(pinactive, pactive, sizeof(MemSeg));
  memcpy(pactive, tmp, sizeof(MemSeg));
#ifdef EDEBUG
  if (edebug & FLOWER && edebug & SMEM)
    {
      sprintf(debugger,"After swapsegs");
      doutput(debugger);
      list_segs();
    }
#endif
}
#ifdef EDEBUG
void MemList::list_segs(void)
{
  MemSeg* seg = active;
  sprintf(debugger,
	  "echo Clerk: Active Segs:");
  doutput(debugger);
  while (seg)
    {
      sprintf(debugger,"  %d attaches= %d",
	      seg->memid(), seg->attaches);
      doutput(debugger);
      seg = seg->next;
    }
  sprintf(debugger,
	  "echo Clerk: Inactive Segs:");
  doutput(debugger);
  seg = inactive;
  while (seg)
    {
      sprintf(debugger,"  %d attaches= %d",
	      seg->memid(), seg->attaches);
      doutput(debugger);
      seg = seg->next;
    }
}
#endif

//
// eVote - Software for online consensus development.
// Copyright (C) 2015 Marilyn Davis <marilyn@deliberate.com>
//
// This file is part of eVote.
//
// eVote is free software: you can redistribute it and/or modify
// it under the terms of the GNU Affero General Public License as
// published by the Free Software Foundation, either version 3 of the
// License, or (at your option) any later version.
//
// eVote is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License
// along with eVote.  If not, see <http://www.gnu.org/licenses/>.
//

/* $Id: memseg.cc,v 1.4 2003/01/15 18:54:07 marilyndavis Exp $ */ 
// memseg.cc - manages a memory segment
/*********************************************************
 **********************************************************/
#include <iostream.h>
#include <fstream.h>
#include<stdio.h>
#include<sys/types.h>
#include <sys/shm.h>
#include <errno.h>
#include "evotedef.h"
#include"memlist.h"
#include"memseg.h"
#ifdef EDEBUG
#include"debug.h"
#endif
extern long now;
extern ofstream *logger;
//************************************************************
MemSeg::MemSeg(short space_in = 0): attaches(-1), _memid(-1),
				    next(NULL), space(space_in)
{
  if (space != 0) 
    memlist.getid(this, space);
}
//************************************************************
ITEM_INFO*
MemSeg::access(YESorNO lock, short **pp_no_items, 
	       short **pp_drop_days)
{
  short *value;
  //  value[0] == 1 if locked
  //              0 if not
  //  value[1] == no_items
  //  value[2] == drop_days
#ifdef EDEBUG
  if (edebug & FLOWER && edebug & SMEM)
    {
      sprintf(debugger,
	      "echo Clerk accessing mid %d ", _memid);
      doutput(debugger);
    }
#endif
  p_where = attach();
  value = (short*)p_where;
  p_lock = &value[0];
  p_no_items = &value[1];
  *pp_no_items = &value[1];
  *pp_drop_days = &value[2];
  if (lock)
    {
      *p_lock = (short)YES;
      locked = YES;
    }
  else
    {
      *p_lock = (short)NO;
      locked = NO;
    }
#ifdef EDEBUG
  if (edebug & FLOWER && edebug & SMEM)
    {
      sprintf(debugger,
	      "echo Clerk accessed mid %d %s p_where = %p no_items = %d",
	      _memid,
	      locked ? "locked" : "not locked",
	      p_where, value[1]);
      doutput(debugger);
    }
#endif
  return (ITEM_INFO*)(p_where + 3*sizeof(short));
  //if ((i = shmctl(_memid, SHM_LOCK, &control)) != 0)
  //		clerklog << "Can't lock shared memory.";
}
//************************************************************
//  Attach this process to the memory.
char *
MemSeg::attach(void)
{
  int er;
  p_where = (char*)shmat(_memid, 0, 0);
  locked = NO;
  if ((long)p_where == -1)
    {
      er = errno;
#ifdef EDEBUG
      if (edebug & FLOWER && edebug & SMEM)
	{
	  sprintf(debugger,
		  "echo Clerk can't attach mid %d errno = %d", _memid, 
		  er);
	  doutput(debugger);
	}
#endif
      clerklog << "Can't attach memory.  _memid = " << _memid 
	       << ". Errno = " << er;
      return NULL;
    }
  p_no_items = (short*)p_where + sizeof(short);  /* for close() */
  return p_where;
}
//************************************************************
void
MemSeg::close(void)
{
  int er;
  // if ((i = shmctl(_memid, SHM_UNLOCK, &control)) != 0)
  //  clerklog << "Can't unlock shared memory.";
  if (locked)
    {
      *p_lock = (short)NO;
      locked = NO;
    }
  no_items = *p_no_items;
#ifdef EDEBUG
  if (edebug & FLOWER && edebug & SMEM)
    {
      sprintf(debugger,
	      "echo Clerk detaching mid %d at %p no_items = %d.", 
	      _memid, p_where, no_items);
      doutput(debugger);
    }
#endif
  if (shmdt(p_where) != 0)
    {
      er = errno;
      clerklog << "Can't detach shared memory:";
#ifdef EDEBUG
      if (edebug & FLOWER && edebug & SMEM)
	{
	  sprintf(debugger,
		  "echo Clerk could not detach mid %d at %p.  Errno = %d", 
		  _memid, p_where, errno);
	  doutput(debugger);
	}
#endif
      switch (er)
	{
	default:
	  clerklog << "Errno = " << er;
	  break;
	}
      return;
    }
  return;
}
//************************************************************
MemSeg *
MemSeg::grow_by(short no_new_items)
{
  if (no_new_items + no_items < space)
    return this;
  if (no_new_items == 0)
    no_new_items = no_items + BUNCH;
  return memlist.grow(this, no_new_items + no_items);
}
//************************************************************
OKorNOT
MemSeg::setup(short no_items)
{
  if (_memid == -1)
    {
      if (memlist.getid(this,
			no_items + BUNCH - (no_items % BUNCH)) != OK)
	return NOT_OK;
    }
  if (space <= no_items)
    {
      if (grow_by(no_items - space) == NULL)
	{
	  return NOT_OK;
	}
    }
  return OK;
}

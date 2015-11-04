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

/* $Id: memlist.h,v 1.3 2003/01/15 18:54:07 marilyndavis Exp $ */ 
// memlist.h -- definition of the MemList class for keeping the
//              one list of memory segments.
/*********************************************************
 **********************************************************/
#ifndef _MEMLIST_H
#define _MEMLIST_H
#define BUNCH 50
class MemSeg;
#define ITEMS_TO_BYTES(X) (X+1)* sizeof(ITEM_INFO) + 3*sizeof(short)
class MemList
{
 public:
  
  MemList(void):last_key(0), inactive(NULL), active(NULL),
    mem_segs(0){}
  int drop_old_segs(void);
  OKorNOT mid_dropped(int mid);
 private:
  key_t last_key;
  MemSeg * inactive;
  MemSeg * active;
  int mem_segs;
#ifdef EDEBUG
  void list_segs(void);
#endif
  OKorNOT deactivate(MemSeg *);
  OKorNOT getid(MemSeg * seg, short for_items);
  MemSeg * grow(MemSeg* seg, short new_no_items);
  void swapsegs(MemSeg * pinactive, MemSeg * pactive);
  friend class MemSeg;
};
#endif

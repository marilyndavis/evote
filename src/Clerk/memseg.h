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

/* $Id: memseg.h,v 1.3 2003/01/15 18:54:07 marilyndavis Exp $ */ 
// memseg.h defines MemSeg class for maintaining a shared memory
//          segment for a Conference
/*********************************************************
 **********************************************************/
#ifndef _MEMSEG_H
#define _MEMSEG_H
extern MemList memlist;
class MemSeg
{
 public:
  MemSeg(short start_items = 0);  /* for how many items? */
  ITEM_INFO * access(YESorNO lock, short ** pp_no_items,
		    short ** pp_drop_days);
  void close(void);
  void deactivate(void){memlist.deactivate(this);}
  MemSeg * grow_by(short no_new_items = 0);
  int memid(void) {return _memid;}
  OKorNOT setup(short no_items);
 private:
  int attaches;
  key_t key;
  YESorNO locked;
  int _memid;
  MemSeg * next;
  short no_items;
  short * p_lock;
  short * p_no_items;
  char * p_where;
  short space;
  char * attach(void);
  friend class MemList;
};  
#endif

/* $Id: memseg.h,v 1.3 2003/01/15 18:54:07 marilyndavis Exp $ */ 
// memseg.h defines MemSeg class for maintaining a shared memory
//          segment for a Conference
/*********************************************************
 *    Copyright (c) 1994...2015 Deliberate.com Patented.
 *    by Marilyn Davis
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

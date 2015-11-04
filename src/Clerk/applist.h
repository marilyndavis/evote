/* $Id: applist.h,v 1.3 2003/01/15 18:54:07 marilyndavis Exp $ */ 
/************************************************************
 *    applist.h  -- Header for functions that manage the shared memory
 ************************************************************/
/*********************************************************
 *    Copyright (c) 1994...2015 Deliberate.com Patented.
 *    by Marilyn Davis
 **********************************************************/
#ifndef _APPLIST_H
#define _APPLIST_H
#include"memlist.h"
#include"memseg.h"
class Conf;
class ItemList;
class VoterList;
class AppList
{
 public:
  AppList(Conf* conf, ItemList* itemlist, VoterList* voterlist, 
	  short drop_day, YESorNO really_new);
  ~AppList(void);
  OKorNOT add(ITEM_INFO* p_new, short no_items);
  void change_vstatus(short did);
  OKorNOT drop(short id1, short id2);
  int get_memid(void){return p_memseg->memid();}
  OKorNOT set_up(short no_items, short drop_day);
 private:
  Conf * _p_conf;
  ItemList* _items;
  VoterList* _voters;
  ITEM_INFO *item;
  short * p_no_items;
  short * p_drop_day;
  MemSeg * p_memseg;
  OKorNOT fill_memory(short no_items, short drop_day);
  OKorNOT grow_by(short no_new_items);
  friend class ItemList;
};
#endif

/* $Id: applist.cc,v 1.3 2003/01/15 18:54:07 marilyndavis Exp $ */ 
/************************************************************
 *    applist.cc  -- Functions that manage the shared memory
 *********************************************************
 **********************************************************/
#include <iostream.h>
#include <fstream.h>
#include <strstream.h>
#include <stdio.h>
#include "evotedef.h"
GLOBAL_INCS
#include "applist.h"
#include "itemlist.h"
#include "voterlis.h"
#include "item.h"
#include "conf.h"
#include "conflist.h"
#include "memseg.h"
#include "memlist.h"
extern ConfList conferences;
#ifdef EDEBUG
#include"debug.h"
#endif
//  ************************************************
AppList::AppList(Conf* p_conf, ItemList* itemlist, VoterList* voterlist, 
		 short drop_day, YESorNO really_new)
  :_p_conf(p_conf), _items(itemlist), _voters(voterlist)
{
  p_memseg = new MemSeg;
  if (really_new)
    {
      set_up(0, drop_day);
    }
#ifdef EDEBUG
  if (edebug & ADJOURNS)
    dlog << "Making applist for " << p_conf->name();
#endif
}
//  ************************************************
AppList::~AppList(void)
{
  p_memseg->deactivate();
#ifdef EDEBUG
  if (edebug & ADJOURNS)
    {
      dlog << "Destroying applist for " << _p_conf->name();
    }
  if (edebug & MEMS)
    {
      vmem("Applist dropped mid.", NO);
    }
#endif  
}
//  ************************************************
OKorNOT 
AppList::add(ITEM_INFO* p_new, short no_new_items)
{
  int i;
  i = p_new->dropping_id;  // debug trick
  grow_by(no_new_items);
  if ((item = p_memseg->access(YES, &p_no_items, &p_drop_day)) 
      == NULL)
    return NOT_OK;
  for (i = 0; i < no_new_items; i++)
    {
      if (p_new[i].dropping_id != *p_no_items + 1)
	return NOT_OK;
      item[p_new[i].dropping_id] = p_new[i];
      (*p_no_items)++;
    }
  p_memseg->close();
  return OK;
}
//  *************************************************
//      Changes the vstatus in the item, and its
//      group members if it's grouped.  It also sends
//      a message to all on-line voters that the 
//      vstatus has changed so they can fix up their
//      stats.
void 
AppList::change_vstatus(short lid)
{
  int i, start, end;
  int len;
  char msg[20];
  if ((item = (ITEM_INFO*)p_memseg->access(YES, &p_no_items, &p_drop_day))
      == NULL)
    return;
  start = lid;
  end = lid;
  if (item[lid].eVote.type >= GROUPED)
    {
      end = lid + item[lid].eVote.more_to_come;
      start = end - item[lid].eVote.no_in_group + 1;
    }
  for (i = start; i <= end; i++)
    {
      item[i].eVote.vstatus 
	= (VSTATUS)(_items->dropping_order[i]->vstatus());
      if (item[i].eVote.vstatus == CLOSED)
	{
	  item[i].eVote.close = _items->dropping_order[i]->close();
	  item[i].eVote.participants_when_closed 
	    = _items->dropping_order[i]->participants_when_closed();
	}
    }
  p_memseg->close();
  (void)sprintf(msg, FEN_NEW_VSTATUS, EEN_NEW_VSTATUS);
  len = (int)strlen(msg);
  _voters->send_all(NEW_VSTATUS, msg, len);
}
//  ************************************************
OKorNOT 
AppList::drop(short id1, short id2)
{
  ITEM_INFO *item;
  short current_no_items;
  short dropping = id2 -id1 +1;
  if ((item = (ITEM_INFO *)p_memseg->access(YES, &p_no_items, &p_drop_day)) 
      == NULL)
    return NOT_OK;			
  current_no_items = *p_no_items;
  *p_no_items -= dropping;
  memcpy(&item[id1], &item[id2+1], 
	 (current_no_items - id2) * sizeof(ITEM_INFO));
  while (++id2 <= *p_no_items)
    {
      item[id1].dropping_id = id1++;
    }
  p_memseg->close();
  return OK;
}
//  ************************************************
OKorNOT 
AppList::fill_memory(short no_items, short drop_day)
{
  int i;
  if ((item = (ITEM_INFO *)p_memseg->access(YES, &p_no_items, &p_drop_day)) 
      == NULL)
    return NOT_OK;			
  if (drop_day == -2)
    drop_day = *p_drop_day;
  *p_drop_day = drop_day;
  for (i = 1; i <= no_items; i++)
    {
      if (_items->dropping_order[i] == NULL)
	return NOT_OK;
      _items->dropping_order[i]->put_info(&item[i]);
    }
  *p_no_items = no_items;
  p_memseg->close();
  if (i > 1 && _items->dropping_order[i] != NULL)
    return NOT_OK;
  _voters->send_all(REDO_STATS, 1);	
  return OK;
}
//  ************************************************
OKorNOT 
AppList::grow_by(short no_new_items)
{
  unsigned long no_of_participants;
  MemSeg * new_p_memseg;
  if ((new_p_memseg = p_memseg->grow_by(no_new_items)) == NULL)
    return NOT_OK;
  if (new_p_memseg != p_memseg)
    {
      char msg[300];
      int len;
      int _memid;
      p_memseg = new_p_memseg;
      _memid = p_memseg->memid();
      no_of_participants = _p_conf->no_of_participants();
      (void)sprintf(msg, FEN_NEW_MID, EEN_NEW_MID);
      len = (int)strlen(msg);
      _voters->send_all(NEW_MID, msg, len);
    }
  return OK;
}
//  ************************************************
OKorNOT 
AppList::set_up(short no_items, short drop_day)
{
  p_memseg->setup(no_items);
  return fill_memory(no_items, drop_day);
}

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

/* $Id: itemlist.cc,v 1.4 2003/01/15 18:54:07 marilyndavis Exp $ */ 
// itemlist.cc -- Controls the list of items in a conf.
/*********************************************************
 **********************************************************/
#include <iostream.h>
#include <fstream.h>
#include <strstream.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/msg.h>
#include <stdlib.h>
#include "evotedef.h"
#include "ballotbo.h"
#include "item.h"
#include "itemlist.h"
#include "conf.h"
#include "ballot.h"
#include "voter.h"
#include "inq.h"
#include "voterlis.h"
#include "itemgrou.h"
#include "qlist.h"
extern QList qlist;
GLOBAL_INCS
extern InQ inq;
extern int msgmax;
#ifdef EDEBUG
#define dumplog (*(_p_conf->ballot_box().dumper))
extern int edebug;
void vmem(char* msg, YESorNO free = NO, int size = 0);
#endif
unsigned long ItemList::_last_id = 0l;
//********************************************************
ItemList::ItemList(Conf* cf, short drop_day, YESorNO really_new)
  :_p_conf(cf), _applist(cf, this, &(cf->community()), 
			 drop_day, really_new),
  _first(NULL), _groups(cf), _end_offset((streampos)0),
  _fill_byte((streampos)sizeof(Header)-(streampos)1),
  _next_byte(sizeof(Header)), _next_bit(8),
  _to_delete(0){}
//********************************************************
ItemList::~ItemList(void)
{
  Item* checker = _first;
  Item* next;
  while (checker != NULL)
    {
      next = checker->_next;
      delete_item(checker);
      checker = next;
    }
  //  delete _groups;
  //  delete _applist;
  //  delete dropping_order;
}
//  *******************************************
//   attach_after   - attaches a new item to the list
//   Items are kept in two orders, ItemID order, for the network,
//   and dropping_order for the home system.  Both lists need this
//   item attached.
void ItemList::
attach_after(Item *p_new_item, Item *&after_item)
{
  if (after_item == NULL)  // first in conf
    {
      if (_first != NULL)
	p_new_item->_next = _first;
      _first = p_new_item;
#ifdef EDEBUG
      if (edebug & CONFS)
	{
	  dlog << "attach to " << _p_conf->name() << " _first:"
	       << p_new_item->_dropping_id << ":" << p_new_item->_item_id;
	}
#endif
    }
  else
    {
      p_new_item->_next = after_item->_next;
      after_item->_next = p_new_item;
#ifdef EDEBUG
      if (edebug & CONFS)
	{
	  dlog << "attach to " << _p_conf->name() 
	       << " new item:" << p_new_item->_dropping_id  << ":"
	       << p_new_item->_item_id;
	  dlog << "   after " << after_item->_dropping_id;
	  if (p_new_item->_next != NULL)
	    dlog << "   before " << (p_new_item->_next)->_dropping_id;
	}
#endif
    }
  // attach to the dropping_order list
  dropping_order.attach(p_new_item);
}
//  *****************************************************
//     Makes zero all the bytes in the ballot that are
//     for storing plain votes.
void
ItemList::blank_ballot(Ballot* p_ballot)
{
  Item* item = _first;
  while (item != NULL)
    {
      item->blank(p_ballot);
      item = item->_next;
    }
}
//  *********************************************
OKorNOT
ItemList::change_vstatus(short did, VSTATUS new_vstatus,
			 unsigned long uid_asking)
{
  OKorNOT cc;
  cc = dropping_order[did]->change_vstatus(new_vstatus, uid_asking);
  if (cc == OK)
    _applist.change_vstatus(did);
  return cc;
}
//  **********************************************
//   consider everyone on-line too
OKorNOT
ItemList::check_stats(void)
{
  Item *p_item;
  int i = 0;
  Ballot *p_ballot;
  OKorNOT all_ok = OK;
  while ((p_item = dropping_order[++i]) != NULL)
    {
      p_item->check(NULL);
    }
  p_ballot = _p_conf->ballot_box().iterator(START);
  while (p_ballot != NULL)
    {				
      i = 0;
      while ((p_item = dropping_order[++i]) != NULL)
	{
	  if (p_item->check(p_ballot) != OK && p_item->type() == 'G')
	    _p_conf->ballot_box().write_out();
	  // This means that the stored
	}      // sum for the group was off
      p_ballot = _p_conf->ballot_box().iterator(NEXT);
    }
  i = 0;
  while ((p_item = dropping_order[++i]) != NULL)
    {		
      if (p_item->check(NULL) != OK)
	all_ok = NOT_OK;
    }
  return all_ok;
}
//  ***************************************************************
//     Sees if there is space in the ballot for another few items.
//     If space is low, returns > 0 indicating priority of grow_request
//     that should be sent.
long
ItemList::check_space(int no_bytes)
{
  long priority = 0l;
  int left = _p_conf->ballot_box().ballot_size() - _next_byte;
  if (left == 0)
    {
      _p_conf->grow_ballot();
      return priority;
    }
  while (left < no_bytes)
    {
      _p_conf->grow_ballot(YES);
      left = _p_conf->ballot_box().ballot_size() - _next_byte;
    }
  if (strncmp(_p_conf->name(),"petition", 8) == 0)
    {
      return priority;
    }
  if (left <= 3)
    priority = PR_EMERGENCY1;
  else if (left <= 6)
    priority = PR_GROW_CONF;
  return priority;
}	
//  ***************************************************
//     input is the msg buffer with the item id.  This function
//     creates new items.
OKorNOT
ItemList::create(char* input, int len, unsigned long uid, 
		 int *no_return)
{
  long grow_priority = 0l;
  ItemID iid;
  int out_len = 0;
  ITEM_INFO* new_list;
  int no_items;
  short no_new = 0;
  short no_stats = 0;
  Item  *p_new_item, *belongs_after, *last_item = NULL;
  ITEM_STAT* p_stat;    
  ITEM_INFO* p_app_item;
  Item ** send_item;
  int i, need = 0;                
  YESorNO trouble = NO;
  i = start_list(input);
  no_items = (len - i)/sizeof(ITEM_INFO);
  new_list = new ITEM_INFO[no_items];
#ifdef EDEBUG
  if (edebug & MEMS)
    {
      char msg[100];
      sprintf(msg,"Made new Item Info new_list for %d items, %d bytes.",
	      no_items, no_items * sizeof(ITEM_INFO));
      vmem(msg, NO, no_items * sizeof(ITEM_INFO));
    }
#endif  
  if (new_list == NULL)
    return PROBLEM;
  send_item = new Item*[no_items];
#ifdef EDEBUG
  if (edebug & MEMS)
    {
      char msg[100];
      sprintf(msg,"Made send_item list for %d items, %d bytes.",
	      no_items, no_items * sizeof(Item*));
      vmem(msg, NO, no_items * sizeof(Item*));
    }
#endif  
  if (send_item == NULL)
    {
      delete[] new_list;
#ifdef EDEBUG
      if (edebug & MEMS)
	{
	  vmem("Deleted new_list", NO, -no_items * sizeof(ITEM_INFO));
	}
#endif  
      return PROBLEM;
    }
  // point the lists to the right places.
  p_app_item = (ITEM_INFO*)(input+i);
  p_stat = (ITEM_STAT*)qlist.output;
  // If it's the first item in a group, or if it's a
  // solo item, be sure the old group is finished
  // and that there is room
  if (p_app_item->eVote.no_in_group 
      == p_app_item->eVote.more_to_come + 1)
    {
      if (_groups.last_group_complete() == NO)
	{
	  if (++_resends > 3)
	    _groups.mark_delete(_groups.last_group(), uid);
	  else
	    {
	      delete[] new_list;
#ifdef EDEBUG
	      if (edebug & MEMS)
		{
		  vmem("Deleted new_list", NO, -no_items * sizeof(ITEM_INFO));
		}
#endif  
	      delete[] send_item;
#ifdef EDEBUG
	      if (edebug & MEMS)
		{
		  vmem("Deleted send_item", NO, -no_items * sizeof(Item*));
		}
#endif  
  	      return UNDECIDED;
	    }
	}
      _resends = 0;
      need = 1;
      if (p_app_item->eVote.type >= GROUPED 
	  && p_app_item->eVote.type <= GROUPEDN )
	{
	  need = p_app_item->eVote.no_in_group + 2;
	}
      else if (strncmp(_p_conf->name(),"petition", 8) == 0)
	{
	  if (p_app_item->eVote.type == TIMESTAMP)
	    {
	      need = 1 + sizeof(time_t) +  sizeof(streampos)
		+ 7; /* filler */
	    }
	}
      grow_priority = check_space(need);
    }
  // make the items
  for (i = 0; i < no_items; i++)
    {
      p_app_item[i].eVote.open = (time_t)get_local_id();
      p_app_item[i].eVote.close = (time_t)0;
      if (p_app_item[i].dropping_id == 0)
	p_app_item[i].dropping_id = dropping_order.next_did();
      if (p_app_item[i].static_id.local_id == (unsigned long)0)
	p_app_item[i].static_id.local_id = get_local_id();
      iid.set(&(p_app_item[i].static_id));
      if ((p_new_item = wheres(iid, belongs_after)) != NULL)
	{
	  if (p_new_item->_dropping_id != p_app_item[i].dropping_id)
	    clerklog << "\nTried to recreate item #"
		     << p_new_item->_dropping_id << ":" 
		     << p_new_item->_item_id << " as item # " 
		     << p_app_item[i].dropping_id;
	  if (last_item != NULL && last_item->_type == 'G')
	    ((GroupedItem*)last_item)->p_group()->mark_delete_group(uid);
	  trouble = YES;
	}
      else
	{
	  switch (p_app_item[i].eVote.type)
	    {
	    case PLAIN:
	      p_new_item = new Item(_p_conf, &(p_app_item[i]), _end_offset);	
#ifdef EDEBUG
	      if (edebug & MEMS)
		{
		  vmem("Made new Item", NO, sizeof(Item));
		}
#endif
  	      break;
	    case TALLIED:
	    case TALLIEDY:
	    case TALLIEDN:
	      p_new_item = new TalliedItem(_p_conf, &(p_app_item[i]), _end_offset);
#ifdef EDEBUG
	      if (edebug & MEMS)
		{
		  vmem("Made new TalliedItem", NO, sizeof(TalliedItem));
		}
#endif
	      break;
	    case TIMESTAMP:
	      p_new_item = new TimeStampItem(_p_conf, &(p_app_item[i]), _end_offset);
#ifdef EDEBUG
	      if (edebug & MEMS)
		{
		  vmem("Made new TimeStampItem", NO, sizeof(TimeStampItem));
		}
#endif
	      break;
	    case GROUPED:
	    case GROUPEDY:
	    case GROUPEDN:
	    case GROUPEDn:
	      p_new_item = new GroupedItem(_p_conf, &(p_app_item[i]), _end_offset);
#ifdef EDEBUG
	      if (edebug & MEMS)
		{
		  vmem("Made new GroupedItem", NO, sizeof(GroupedItem));
		}
#endif
	      break;
	    default:
	      break;
	    }
	  switch (finish_create(p_new_item, uid)) 
	    {
	    case OK:
	      break;
	    case PROBLEM: // group out of order - deleted
	      while (new_list[no_stats].eVote.more_to_come != 0)
		{
		  (no_stats)--;
		}
	      trouble = YES;
	      continue;
	      break;
	    case UNDECIDED:  // not finished with old group
	      delete[] new_list;
#ifdef EDEBUG
	      if (edebug & MEMS)
		{
		  vmem("Deleted new_list", NO, -no_items * sizeof(ITEM_INFO));
		}
#endif  
	      return UNDECIDED;
	    default:
	      break;
	    }
	  attach_after(p_new_item, belongs_after);
#ifdef EDEBUG
	  if (edebug & ITEMS)
	    {
	      show_map(dumplog);
	    }
#endif
	  new_list[no_new++] = p_app_item[i];
	}  // end of make the items
      send_item[no_stats++] = p_new_item;
    }
  _applist.add(new_list, no_new);
  if (grow_priority > 0l)
    request_grow(grow_priority);
  for (i = 0 ; i < no_stats; i++)
    {
      out_len += send_item[i]->report_stat(p_stat++);
    }
  delete[] send_item;
#ifdef EDEBUG
  if (edebug & MEMS)
    {
      vmem("Deleted send_item", NO, -no_items * sizeof(Item*));
    }
#endif  
  delete[] new_list;
#ifdef EDEBUG
  if (edebug & MEMS)
    {
      vmem("Deleted new_list", NO, -no_items * sizeof(ITEM_INFO));
    }
#endif  
  *no_return = no_stats;
  if (trouble == NO)
    return OK;						
  else
    return NOT_OK;
}
//  *****************************************************
void
ItemList::detach(Item* p_item)
{
  Item* comes_after;
  Item* x;
  if ((x = wheres(p_item->item_id(), comes_after)) != p_item)
    {
      clerklog << "Can't detach item: " << p_item->item_id();
      return;
    }
  if (comes_after != NULL)
    {
      (comes_after)->_next = p_item->_next;
    }
  else
    {
      _first = p_item->_next;
#ifdef EDEBUG
      if (edebug & CONFS)
	{
	  if (_first != NULL)
	    dlog << "_first replaced with " << _first->_dropping_id
		 << ":" << _first->item_id();
	}
#endif
    }
#ifdef EDEBUG
  if (edebug & CONFS)
    {
      dlog << "Detached from " << _p_conf->name()
	   << " item " << p_item->_dropping_id << ":"
	   << p_item->item_id();
      if (comes_after != NULL)
	{
	  dlog << "   " << (comes_after)->_dropping_id
	       << ":" << comes_after->item_id() ;
	  if (comes_after->_next != NULL)
	    dlog << "; then " << ((p_item)->_next)->_dropping_id
		 << comes_after->_next->item_id();
	}
    }
#endif
  if (p_item->_dropping_id > 0)
    dropping_order[p_item->_dropping_id] = NULL;
}
//  *************************************
//   delete_item  - Deletes the item from memory.
void
ItemList::delete_item(Item* checker)
{
#ifdef EDEBUG
  if (edebug & MEMS)
    {
      switch (checker->type())
	{
	case 'P':
	  vmem("Deleting Item", NO, -sizeof(Item));
	  break;
	case 'S':
	  vmem("Deleting TimeStampItem", NO, -sizeof(TimeStampItem));
	  break;
	case 'T':
	  vmem("Deleting TalliedItem", NO, -sizeof(TalliedItem));
	  break;
	case 'G':
	  vmem("Deleting GroupedItem", NO, -sizeof(GroupedItem));
	  break;
	}
    }
#endif
  switch (checker->type())
    {
    case 'P':
      delete checker;
      break;
    case 'S':
      delete (TimeStampItem*)checker;
      break;
    case 'T':
      delete (TalliedItem*)checker;
      break;
    case 'G':
      delete (GroupedItem*)checker;
      break;
    }
}
//  *************************************
//   drop_items  - finds the item, or group of items
//           and asks it to mark itself for deletion.
//           Returns the time that the item that closed
//           earliest closed.
time_t
ItemList::drop_items(char* input, int len, unsigned long uid)
{
  int i;
  int no_items;
  STATIC_ID* p_drop;
  ItemID iid;
  Item * p_item;
  YESorNO trouble = NO;
  short no_in_group;
  short new_dels;
  short low = 9999, high = 0;
  time_t early_closer;
  Ballot* p_author_ballot;
  long block;
  BallotBox& ballot_box = _p_conf->ballot_box();
  time(&early_closer);
  i = start_list(input);
  no_items = (len-i)/sizeof(STATIC_ID);
  // point the lists to the right places.
  p_drop = (STATIC_ID*)(input+i);
  for (i = 0; i < no_items; i++)
    {
      iid.set(p_drop++);
      p_item = wheres(iid);				
      if (p_item == NULL)
	{
	  trouble = YES;
	  continue;
	}
      if (p_item->_close != 0l && p_item->_close < early_closer)
	early_closer = p_item->_close;
      if (i == 0)  // first use
	{
	  low = p_item->_dropping_id;
	  high = low - 1;
	}
      else
	// each item should have the same dropping_id, low, because
	// the previous one(s) were dropped already
	if (p_item->_dropping_id != low)
	  {
	    clerklog << "Drop items: items not consecutive. Called by " 
		     << uid << ".";
	    trouble = MAYBE;
	    break;
	  }
      if (p_item->_type == 'G')
	{
	  GroupedItem* gitem = (GroupedItem*)p_item;
	  no_in_group = gitem->_p_group->_no_in_group;
	  high += no_in_group;
	  new_dels = gitem->_p_group->mark_delete_group(uid);
	  // mark_delete deletes from dropping order
	  if (new_dels != no_in_group)
	    trouble = YES;
	  _to_delete += new_dels;
	}
      else
	{
	  high++;
	  p_item->mark_delete(uid);
	  _to_delete++;
	  dropping_order.delete_from(low);
	}
      //  Here we fiddle with the last mod time of the author if
      //  the author dropped out of the conf before the poll closed.  
      //  This is because we
      //  have been keeping his ballot and his name in the who.list
      //  as long as this poll exists.  Since it's being dropped
      //  now, we're updating the ballot date so that the ballot
      //  will be dropped.
      p_author_ballot = ballot_box.find_ballot(p_item->_author, block);
      if (p_author_ballot->action() & DROPPING 
	  && p_author_ballot->mod_date() < p_item->_close)
	{
	  p_author_ballot->set_date(p_item->_close);
	  if (isalive(p_author_ballot) == NO)
	    ballot_box.delete_ballot(p_author_ballot, block);
	  else
	    ballot_box.write_ballot(block, 
				    (streampos)((char*)p_author_ballot
						-ballot_box._buffer), 
				    p_author_ballot);
	}
#ifdef EDEBUG
      if (edebug & ITEMS)
	{
	  if (_p_conf->ballot_box().dump_up == NO)
	    _p_conf->ballot_box().start_dump();
	  dumplog << "Dropping an item: " << p_item->_title;
	  dumplog << " type = " << p_item->_type << " dropping_id = ";
	  dumplog << p_item->_dropping_id
		  << " byte is " << p_item->_byte;
	  dumplog << "To_delete = " << _to_delete;
	}
#endif
    }
  if (trouble == YES)
    {
      _applist.set_up(dropping_order._next_did -1, _p_conf->drop_day());
    }
  else
    {
      _applist.drop(low, high);			
      (void)sprintf(qlist.output, FEN_DROP_STATS, EEN_DROP_STATS);
      len = (int)strlen(qlist.output);
      _p_conf->community().send_all(DROP_STATS, len);
    }
  return early_closer;
}
//  **********************************************
//  Subtract the votes for the ballot out of the statistics
OKorNOT
ItemList::drop_votes(Ballot *p_ballot)
{
  int i=0;
  short vote = NOT_READ;
  RTYPE cc;
  Item* p_item;
  if (p_ballot->action() == READ_ONLY)
    vote = WAS_ARCHIVE;
#ifdef EDEBUG
  if (edebug & DUMP2)
    {
      dumplog << "\nUncounting ballot:\n";
      show_votes(p_ballot, dumplog);
      show_packing(p_ballot, dumplog);
    }
#endif
  while (dropping_order[++i] != NULL)
    {
      p_item = dropping_order[i];
      if (p_item->_vstatus == CLOSED)
	continue;
      if (p_item->_type == 'G')
	{
	  i += ((GroupedItem*)p_item)->p_group()->_no_in_group - 1;
	}
      p_item->mark_vote(p_ballot, vote, &cc);
    }
  return OK;
}
//   ***********************************
//   fetch  - fetches the item info from the fstream.
OKorNOT
ItemList::fetch(istream &strm)
{
  Item *p_item = NULL;
  GroupedItem* last_grouped_item = NULL;
  char ch=0;
  Item* belongs_after, *checker;
  while (ch != EOF)
    {
      switch (ch = strm.peek())
	{
	case EOF:
	  if (last_grouped_item != NULL 
	      && last_grouped_item->more_to_come != 0)
	    {     // last group not finished      
	      _to_delete += 
		last_grouped_item->_p_group->mark_delete_group(0);
	    }
	  break;
	case 'P':
	  p_item = new Item(_p_conf, strm);
#ifdef EDEBUG
	  if (edebug & MEMS)
	    {
	      vmem("Made new Plain Item.", NO,
		   sizeof(Item));
	    }
#endif
	  //  This Byte and bit stuff fixes things up if
	  //  the Clerk went down without a save and there
	  //  were new items added since the last time the
	  //  itemlist stored itself.
	  if (p_item->_byte > _fill_byte)
	    {		
	      _fill_byte = p_item->_byte;
	      _next_byte = _fill_byte + 1;
	      _next_bit =  p_item->_bit + 1;
	    }
	  else if (p_item->_byte == _fill_byte
		   && p_item->_bit >= _next_bit)
	    {
	      _next_bit = p_item->_bit + 1;
	    }
	  break;
	case 'S':     
	  p_item = new TimeStampItem(_p_conf, strm);
#ifdef EDEBUG
	  if (edebug & MEMS)
	    {
	      char msg[100];
	      sprintf(msg,"Made new TimeStamp Item  %d bytes.",
		      sizeof(TimeStampItem));
	      vmem(msg, NO, sizeof(TimeStampItem));
	    }
#endif
	  break;
	case 'T':
	  p_item = new TalliedItem(_p_conf, strm);
#ifdef EDEBUG
	  if (edebug & MEMS)
	    {
	      char msg[100];
	      sprintf(msg,"Made new Tallied Item  %d bytes.",
		      sizeof(TalliedItem));
	      vmem(msg, NO, sizeof(TalliedItem));
	    }
#endif
	  break;
	case 'G':   // grouped item needs this pointer to establish
	  // link into the GroupList object
	  p_item = new GroupedItem(_p_conf, strm);
#ifdef EDEBUG
	  if (edebug & MEMS)
	    {
	      char msg[100];
	      sprintf(msg,"Made new GroupedItem  %d bytes.",
		      sizeof(GroupedItem));
	      vmem(msg, NO, sizeof(GroupedItem));
	    }
#endif
	  switch (p_item->_status)
	    {
	    case TROUBLE:
	      if (((GroupedItem*)p_item)->_p_group != NULL)
		{
		  _to_delete += _groups.mark_delete(((GroupedItem*)p_item)->_p_group, 0);
		}
	      else
		{
		  delete_item(p_item);
		  _to_delete++;
		}
	      ch = ' ';
	      break;
	    case DUPLICATE:
	      break;
	    default:
	      last_grouped_item = ((GroupedItem*)p_item);
	      break;
	    }
	  break;
	default:
	  if (ch != ' ')	
	    {
	      clerklog << "Unrecognized item type " << ch << " in " << _p_conf->name();
	    }
	  strm.get(ch);
	  ch = ' ';    
	  break;
	}
      if (ch != EOF && ch != ' ')  // if we made a new item
	{
	  if (p_item->_byte >= _next_byte)
	    _next_byte = p_item->_byte + p_item->ballot_bytes();
	  checker = wheres(p_item->item_id(), belongs_after);
	  switch (p_item->_status)
	    {
	    case DUPLICATE:
	      *checker = *p_item;
	      checker->_status = (unsigned char)ACTIVE;
	      delete_item(p_item);
	      break;
	    case DELETE:
	      _to_delete++;
	      // fall into default because attach_after
	      // attaches to item list but not dropping order.
	    default:
	      attach_after(p_item, belongs_after);
	      break;
	    }
	}
    }	
  _applist.set_up(dropping_order.next_did() - 1, _p_conf->drop_day());
  if (last_grouped_item != NULL)
    _groups.set_max_id(last_grouped_item->_p_group->_id);
#ifdef EDEBUG
  if (edebug & CONFS)
    {
      if (_p_conf->ballot_box().dump_up == NO)
	_p_conf->ballot_box().start_dump();
      dumplog << "Just fetched " << _p_conf->name() << ":";
      show_map(dumplog);
    }
#endif
  return OK;
}
// *********************************************************
//     called by create functions to finish the job.
OKorNOT
ItemList::finish_create(Item* p_new_item, unsigned long uid)
{
  if (p_new_item->_type == 'G')
    {
      switch (p_new_item->_status)
	{
	case TROUBLE: // items out of order for this group
	  _groups.mark_delete(((GroupedItem*)p_new_item)->_p_group, uid);
	  return PROBLEM;
	  break;
	case ACTIVE:
	  _resends = 0;
	  break;
	}
    }
  p_new_item->find_spot(this);
#ifdef EDEBUG
  if (edebug & ITEMS)
    {
      if (_p_conf->ballot_box().dump_up == NO)
	_p_conf->ballot_box().start_dump();
      dumplog << "\nCreated a new item: " << p_new_item->_title;
    }
#endif
  if ((p_new_item->_type == 'P' && p_new_item->_bit == 0 )
      || p_new_item->_type == 'S')
    {
      switch (p_new_item->_type)
	{
	case 'P':
	  _p_conf->ballot_box().blank_ballots(p_new_item->_byte, 
					      p_new_item->ballot_bytes());
	  _p_conf->community().blank_voters(p_new_item->_byte,
					    p_new_item->ballot_bytes());
#ifdef EDEBUG
	  if (edebug & ITEMS)
	    {
	      if (_p_conf->ballot_box().dump_up == NO)
		_p_conf->ballot_box().start_dump();
	      dumplog << "\nBlanked ballots at _byte " << (int)p_new_item->_byte;
	      dumplog << "\nNew plain item: _byte = " << p_new_item->_byte;
	      dumplog << "\n_bit = " << p_new_item->_bit;
	      show_map(dumplog);
	    }
#endif
#ifdef EDEBUG
	  if (edebug & DUMP2)
	    {
	      dumplog << "\nBlanked ballots at vote " << (int)p_new_item->_byte - sizeof(Header);
	      _p_conf->ballot_box().dump_file();	
	      show_onlines(dumplog);  
	    }
#endif
	  break;
	case 'S':
	  /* leave first byte alone! Like tallied item */
	  _p_conf->ballot_box().blank_ballots(p_new_item->_byte + 1, 
					      p_new_item->ballot_bytes()- 1);
	  _p_conf->community().blank_voters(p_new_item->_byte + 1,
					    p_new_item->ballot_bytes() - 1);
#ifdef EDEBUG
	  if (edebug & ITEMS)
	    {
	      if (_p_conf->ballot_box().dump_up == NO)
		_p_conf->ballot_box().start_dump();
	      dumplog << "\nBlanked ballots at _byte + 1 = " << (int)p_new_item->_byte + 1 << " for " << p_new_item->ballot_bytes() - 1 << " bytes." ;
	      show_map(dumplog);
	    }
#endif
	  break;
	}
    }
  p_new_item->store_me();  // comes before attach or trouble with groups!
  return OK;
}
// *************************************************
// manages the static _last_id member.
unsigned long
ItemList::get_local_id(void)
{
  unsigned long id;
  id = (unsigned long)time(NULL);
  if (id <= _last_id)
    id = ++_last_id;
  _last_id = id;
  return id;
}
//****************************************************
//     Checks if the ballot, which is under consideration
//     for deletion, has any reason to stay around.
YESorNO
ItemList::isalive(Ballot * p_ballot)
{
  int i = 0;
  Item * p_item;
  ACTION action;
  short vote;
  if (!((action = p_ballot->action()) & DROPPING)
      && !(action & DROP))
    return YES;
  while (dropping_order[++i] != NULL)
    {
      p_item = dropping_order[i];
      if (p_item->_type == 'G')
	{
	  i += ((GroupedItem*)p_item)->p_group()->_no_in_group - 1;
	}
      if (p_item->vstatus() == CLOSED
	  && p_item->close() <= p_ballot->mod_date()
	  && ((vote = p_item->report_fixed_vote(p_ballot)) != LATE)
	  && vote != WAS_ARCHIVE)  /* && vote != NOT_READ) */
	return YES;
      if (*(unsigned long*)p_ballot == p_item->_author)
	return YES;
    }      
  return NO;
}
// ******************************************
//  This function deletes the items
//  that have previously been marked for deleting and
//  figures which bytes in the vote record are no longer used.
//  It reports the dead bytes in the dead_bytes array.
OKorNOT
ItemList::kill_old_items(streampos dead_bytes[])
{
  streampos i;
  int j;
  streampos *byte;
  YESorNO continue_bit_byte = NO;
  Item* last_good_plain = NULL;
  short last_x_bit = 0;
  streampos last_x_fill_byte = 0;
  streampos no_bytes = _next_byte ;
  int no_of_deletes = 0;
  Item* p_item = _first;
  Item* next = NULL;
  YESorNO group_flag = NO;
  ItemGroup* the_group;
#ifdef EDEBUG
  if (edebug & CONFS)
    {
      dumplog << "Killing old bytes for " << _p_conf->name() << ":";
      dumplog << "_to_delete = " << _to_delete << " _next_byte = "
	      << _next_byte << " _next_bit = " << _next_bit 
	      << " fill_byte = " << _fill_byte;
      show_map(dumplog);
    }
#endif
  if (_to_delete == 0)
    {
      dead_bytes[0] = -1L;
      return OK;
    }
  byte = new streampos [no_bytes];
#ifdef EDEBUG
  if (edebug & MEMS)
    {
      char msg[100];
      sprintf(msg,"Made new byte array for %ld entries,  %ld bytes.",
	      (long)no_bytes, (long)(no_bytes * sizeof(streampos)));
      vmem(msg, NO, no_bytes * sizeof(streampos));
    }
#endif
  for (i = 0; i < no_bytes; i++)
    byte[i] = 0;
  while (p_item != NULL)
    {
      next = p_item->_next;
      group_flag = NO;
      if ((STATUS)p_item->_status != DELETE)
	{
	  p_item->mark_bytes(byte);
	  if (p_item->_type == 'P')
	    last_good_plain = p_item;
	}
      else
	{
	  if (p_item->_type == 'G')
	    {
	      group_flag = YES;
	      the_group = ((GroupedItem*)p_item)->p_group();
	      the_group->_no_in_group--;
	    }
	  if (p_item->_type == 'P')
	    {
	      last_x_fill_byte = p_item->_byte;
	      last_x_bit = p_item->_bit;
	    }
	  detach(p_item);
	  delete_item(p_item);
	  _to_delete--;
	  if (group_flag == YES && the_group->isempty() == YES)
	    {
	      _groups.detach(the_group);
	      delete the_group;
#ifdef EDEBUG
	      if (edebug & MEMS)
		{
		  vmem("Deleted a group list", NO, sizeof(GroupList));
		}
#endif
	      group_flag = NO;
	    }
	}			
      p_item = next;
    }
#ifdef EDEBUG
  if (edebug & ITEMS)
    {
      if (_p_conf->ballot_box().dump_up == NO)
	_p_conf->ballot_box().start_dump();
      i = sizeof(Header) -1;
      while (++i < _next_byte)
	{
	  dumplog << " byte[" << i << "] is "
		  << byte[i] ;
	}
    }
#endif
  j = -1;
  i = sizeof(Header) -1;
  while (++i < _next_byte)
    {
      if (byte[i] == 0)  // not used any more
	{
	  dead_bytes[++j] = i;
	  no_of_deletes++;
	}
    }
  dead_bytes[++j] = -1;
#ifdef EDEBUG
#define MAX  500
  if (edebug & ITEMS)
    {
      int k;
      if (_p_conf->ballot_box().dump_up == NO)
	_p_conf->ballot_box().start_dump();
      dumplog << "\nDead bytes are :\n    ";
      for (k = 0; dead_bytes[k] != -1 && k < 499 ; k++)
	{
	  dumplog << dead_bytes[k] << " " ;
	}
      dumplog << "\nno_of_deletes = " << no_of_deletes
	      << " _next_byte = " << _next_byte;
      dumplog << "\n_next_bit = " << _next_bit 
	      << " _fill_byte = " << _fill_byte ;
      if (last_good_plain != NULL)
	{
	  dumplog << "\nlast_x_fill_byte = " << last_x_fill_byte
		  << " last_good_plain->_byte = " << last_good_plain->_byte;
	  dumplog << "\nlast_x_bit = " << last_x_bit 
		  << "last_good_plain->_bit" << last_good_plain->_bit;
	}
      else
	{
	  dumplog << " last_good_plain is null.";
	}
    }
#endif
  _next_byte -= no_of_deletes;
  // On the plain items, we don't want to use a bit that has been used
  // before - even if it is deleted because the ballots will still
  // have data in that bit.
  // if _next_bit == 8, it could be that the last reorder started the
  //  next_fill_byte beyond the last_good_plain's _byte because 
  //  a later item in the last_good_plain's _byte was dropped.  In
  //  that case we want to still skip the rest of the last_good_plain's
  //  _byte because it may already have data in it.
  if (_next_bit != 8 && last_good_plain != NULL 
      && ((last_x_fill_byte < last_good_plain->_byte)
	  || (last_x_fill_byte == last_good_plain->_byte
	      && last_x_bit < last_good_plain->_bit)))
    {
      continue_bit_byte = YES;
    }
  else
    continue_bit_byte = NO;
  if (_to_delete != 0)
    {
      clerklog << "\n_to_delete = " << _to_delete;
    }
  // fix the bytes on the remaining items
  for (i = 1; dropping_order[i] != NULL; i++)
    {
      // first, find the biggest dead_byte that is less than
      // the byte for this item.
      j = no_of_deletes;
      while (--j >= 0 && dead_bytes[j] > dropping_order[i]->_byte)
	;
      if (j > -1 && dead_bytes[j] == dropping_order[i]->_byte)
	{
	  clerklog << "\nSome confusion with j = " << j << "; and i = " 
		   << i;
	}
      // Now, 0 through j th dead_byte is below this so this byte
      // collapses down j+1 bytes.
      dropping_order[i]->_byte -= j+1;
#ifdef EDEBUG
      if (edebug & ITEMS)
	{
	  dumplog << "\ndropping_order[" << i << "]->_byte = " 
		  << dropping_order[i]->_byte << " after -" << j+1;
	}
#endif
      // But, if it's a grouped item, we have to decrement the
      // the group bytes too - but only once, the first time
      // the group is encountered.
      if (dropping_order[i]->_type == 'G' )
        {
          the_group = ((GroupedItem*)dropping_order[i])->_p_group;
          if (the_group->first() == dropping_order[i])
            the_group->_byte -= j+1;
        }
    }
  delete[] byte;
#ifdef EDEBUG
  if (edebug & MEMS)
    {
      vmem("Deleted byte array", NO, -no_bytes * sizeof(streampos));
    }
#endif
  if (continue_bit_byte == YES)
    {
      _fill_byte = last_good_plain->_byte;
      _next_bit = last_good_plain->_bit + 1;
    }
  /*  else
      {
      _fill_byte = _next_byte;
      _next_bit = 8;
      }
  */
#ifdef EDEBUG
  if (edebug & ITEMS)
    {
      if (_p_conf->ballot_box().dump_up == NO)
	_p_conf->ballot_box().start_dump();
      dumplog << "\ncontinue_bit_byte = "
	      << (continue_bit_byte ? "yes" : "no");
      dumplog << " _fill_byte = " << _fill_byte
	      << " _next_bit = " << _next_bit;
      dumplog << "\nKilled old bytes.";
      show_map(dumplog);
    }
#endif
  return OK;
}
// *****************************************************
// sends a request to grow this conf.
void
ItemList::request_grow(long priority)
{
  char msg[30];
  sprintf(msg, FNE_GROW_CONF, GROW_CONF, (unsigned long)0, _p_conf->name());
  inq.send_myself(msg, priority);
#ifdef EDEBUG
  if (edebug & DUMP2)
    dumplog << "\nSending a priority " << priority << " grow request: "
	    << (_p_conf->ballot_box().ballot_size() - _next_byte)
	    << " item bytes left.";
#endif
}
//  *****************************************************
//     finds the '>' in the input string and reports
//     where that is.
//     This is used for messages that send in lists.
//     The > marks the spot just before the list.
//     Called by sync_conf(), drop_items(), and create().
short
ItemList::start_list(char* input)
{
  int i = 0;
  while (input[++i] != '>' && i < msgmax)
    ;   // keep going
  if (input[i] == '>')
    {
      return i+2;
    }
  return -1;
}
//  ****************************************************
//    packs a list of item stats in the output buffer, returning 
//    the number of bytes packed 
unsigned long
ItemList::send_stats(Voter* p_voter, Item* starting,
		     int no_to_send)
{
  int i;
  unsigned long len = 0L;
  ITEM_STAT *pt = (ITEM_STAT*)qlist.output;
  Ballot *p_ballot = p_voter -> p_ballot();
  for (i = starting->_dropping_id; no_to_send > 0; i++, pt++, no_to_send--)
    {
      if (dropping_order[i] == NULL)
	break;  // asking for too many
      if ((len += (unsigned long)sizeof(ITEM_STAT)) > (unsigned long)msgmax)
	{
	  len -= (unsigned long)sizeof(ITEM_STAT);
	  break;
	}
      dropping_order[i]->report_stat(pt, LIST, p_ballot);
    }
  return len;
}
//  **************************************************
//   store stores all the items in the _p_item list.
OKorNOT
ItemList::store(ostream &strm)
{
  Item* p_item = _first;
  OKorNOT all_ok = OK;
  while (p_item != NULL)
    {
      if (p_item->store(strm) != OK)
	{
	  all_ok = NOT_OK;
	}
      p_item = p_item->_next;
    }
#ifdef EDEBUG
  if (edebug & CONFS)
    {
      if (_p_conf->ballot_box().dump_up == NO)
	_p_conf->ballot_box().start_dump();
      dumplog << "\nStoring " << _p_conf->name() << ":\n";
      show_map(dumplog);
    }
#endif
  return all_ok;
}
// ***********************************************************
// unsigned long uid, short sends)
// Forces the conf to have a itemlist that matches the
// one sent in.
OKorNOT
ItemList::sync_conf(char * input, short len, unsigned long uid,
		    short sends)
{
  static short sends_left = 0;
  short no_items_in;
  static YESorNO perfect = YES;
  int list_off;
  static int tot_items = 0;
  static int new_order = 1;
  char msg[100];
  Item* p_item, *belongs_after;
  ItemID iid;
  STATIC_ID* item_list;
  ITEM_INFO item_info;
  int i;
  OKorNOT cc;
  list_off = start_list(input);
  item_list = (STATIC_ID*)(input + list_off);
  no_items_in = (len - list_off)/sizeof(STATIC_ID);
  if (sends_left == 0)  // first group
    {
      clerklog << "Sync_conf called by user " << uid 
	       << " on conf " << _p_conf->name() << ".";
      sprintf(msg, "Backed up for sync_conf by %lu", uid);
      _p_conf->backup(msg);
      perfect = YES;
      new_order = 1;
      tot_items = no_items_in;
    }
  else
    {
      tot_items += no_items_in;
      if (sends_left != sends)
	return  NOT_OK;
    }
  for (i = 0 ; i < no_items_in; i++, new_order++)
    {
      iid.set(&item_list[i]);
      if ((p_item = wheres(iid, belongs_after)) == NULL)
	{   // new item - must be PLAIN
	  if (dropping_order[new_order] != NULL)
	    {
	      clerklog << dropping_order[new_order]->_item_id
		       << " out of order in sync. \n Was "
		       << new_order << ", now -1.";
	      dropping_order[new_order]->_dropping_id = -1;
	    }
	  item_info.static_id = item_list[i];
	  item_info.dropping_id = new_order;
	  item_info.eVote.type = PLAIN;
	  p_item = new Item(_p_conf, &item_info, _end_offset);
#ifdef EDEBUG
	  if (edebug & MEMS)
	    {
	      char msg[100];
	      sprintf(msg,"Made new Item  %d bytes.",
		      sizeof(Item));
	      vmem(msg, NO, sizeof(Item));
	    }
#endif
	  if ((cc = finish_create(p_item, uid)) != OK)
	    return NOT_OK;
	  attach_after(p_item, belongs_after);
	  perfect = NO;
	  continue;
	}
      if (p_item->_dropping_id != new_order)
	{
	  clerklog << p_item->_item_id 
		   << " was out of order in sync.  Was # "
		   << p_item->_dropping_id << ".  Now # " << new_order << ".";
	  if ((STATUS)p_item->_status != DELETE)
	    dropping_order.delete_from(p_item);
	  else
	    p_item->_status = (unsigned char)ACTIVE;
	  perfect = NO;
	  p_item->_dropping_id = new_order;
	  dropping_order.attach(p_item);
	}
      // else OK as is
    }
  if ((sends_left = --sends) == 0)
    {  // end of list
      Item* next_item = _first;
      p_item = _first;
      while (p_item != NULL)
	{
	  next_item = p_item->_next;
	  if (p_item->_dropping_id == -1 
	      || p_item->_dropping_id > tot_items)
	    {
	      perfect = NO;
	      if (p_item->_type == 'G')
		{
		  GroupedItem* p_gitem = (GroupedItem*)p_item;
		  clerklog << "Whole group of items dropped:"
			   << p_gitem->_p_group->_no_in_group
			   << " items dropped, starting with " 
			   << p_item->_item_id;
		  _to_delete += _groups.mark_delete(p_gitem->_p_group, uid);
		  // deletes from dropping ord
		}
	      else
		{
		  p_item->mark_delete(uid);
		  _to_delete++;
		  // not necessary to delete -1 from dropping ord
		}
	    }
	  p_item = next_item;
	}  // loop through all items
      if (perfect == NO)
	_applist.set_up(tot_items, _p_conf->drop_day());
    }							
  return OK;
}
// **************************************************
Item*
ItemList::wheres(ItemID& tid, Item *&belongs_after)
{
  Item* checker = _first;
  belongs_after = NULL;
  while (checker != NULL)
    {
      if (checker->item_id() >= tid)
	break;
      belongs_after = checker;
      checker = checker->_next;
    }
  if (checker == NULL)
    return NULL;
  if (checker->item_id() == tid)
    return checker;
  return NULL;
}
// **************************************************
#ifdef EDEBUG
void
ItemList::show_map(ofstream& strm)
{
  int i;
  int done_group = -1;
  GroupedItem* gt;
  strm << "\nMAP of " << _p_conf->name() << ":\n";
  strm << "_next_byte = " << _next_byte << " _fill_byte = "
       << _fill_byte << " _next_bit = " << _next_bit << "\n";
  for (i = 1; dropping_order[i] != NULL ; i++)
    {
      strm << '\n' << dropping_order[i]->_type << ':' << i << ":"
	   << dropping_order[i]->item_id() << " -> "
	   << dropping_order[i]->_byte;
      if (dropping_order[i]->_type == 'P')
	strm << ':' << dropping_order[i]->_bit;
      if (dropping_order[i]->_type == 'G' )
	{
	  gt = (GroupedItem*)dropping_order[i];
	  if (done_group != gt->_p_group->_id)
	    {
	      done_group = gt->_p_group->_id;
	      strm << "\nGroup of " << gt->_p_group->_no_in_group
		   << ", #" << done_group << " -> " << 
		gt->_p_group->_byte;
	    }
	}					
    }
  strm.flush();
}
// **************************************************
void
ItemList::show_onlines(ofstream &strm)
{
  Voter* voter = _p_conf->community().first();
  long timer;
  dumplog << "\nOnline voter records are:";
  while (voter != NULL)
    {
      timer = voter->p_ballot()->mod_date();
      strm << "\n" << *(unsigned long*)(voter->p_ballot()) << ' ' 
	   << ctime(&timer);
      show_packing(voter->p_ballot(), strm);
      voter = voter->next();
    }
  strm.flush();
}
// **************************************************
void
ItemList::show_packing(Ballot* pb, ofstream &strm)
{
  int i;
  streampos byte_done = -1;
  char buf[40];
  int count = 70;
  GroupedItem* gt;
  int group_done = -1;
  void show_bits(unsigned char* byte, ofstream& strm);
  for (i = 1; dropping_order[i] != NULL; i++)
    {
      if (count >= 70)
	{
	  strm << "\n";
	  count = 0;
	}
      if (dropping_order[i]->_type == 'G')
	{
	  gt = (GroupedItem*)dropping_order[i];
	  if (gt->p_group()->_id != group_done)
	    {
	      group_done = gt->p_group()->_id;
	      (void)sprintf(buf," G:%d:%4d",
			    gt->p_group()->_no_in_group,
			    *(short*)((char*)pb + gt->p_group()->_byte));
	      count += (int)strlen(buf);
	      strm << buf;
	    }
	}
      if (count >= 70)
	{
	  strm << "\n";
	  count = 0;
	}
      if (dropping_order[i]->_type == 'P')
	{
	  if (dropping_order[i]->_byte != byte_done)
	    {
	      byte_done = dropping_order[i]->_byte;
	      show_bits((unsigned char*)(pb + byte_done), strm);
	      count += 9;
	    }
	}
      else
	{
	  sprintf(buf,"%4d", *((char*)pb + dropping_order[i]->_byte));
	  count += 4;
	  strm << buf;
	  if (dropping_order[i]->_type == 'S')  /*time Stamp */
	    {
	      if (count >= 54)
		{
		  strm << "\n";
		  count = 0;
		}
	      sprintf(buf, " S:%11ld/%11ld", 
		      (long)*((time_t *)((char*)pb 
					 + (dropping_order[i]->_byte+1))),
		      (long)*((streampos *)((char*)pb 
					    + (dropping_order[i]->_byte
					       + 1 + sizeof(time_t)))));
	      strm << buf;
	      count += 26;
	    }
	}
    }
  strm.flush();
}		
// **************************************************
void
show_bits(unsigned char *x, ofstream& strm)
{
  char n;
  int i;
  char mask = ~(~0 << 1);
  strm << ' ';
  for (i = 0 ; i < 8 ; i++)
    {
      n = *x;
      if ((n >> (7-i)) & mask)
	{
	  strm << '1';
	}
      else
	{
	  strm << '0';
	}
    }
}
// **************************************************
void
ItemList::show_votes(Ballot* pb, ofstream& strm)
{
  int i;
  short vote;
  int group_done = -1;
  int sum;
  GroupedItem *gt;
  for (i = 1; dropping_order[i] != NULL ; i++)
    {
      if (i%2)
	strm << "\n";
      else
	strm << "\t";
      vote = dropping_order[i]->report_vote(pb);
      strm << dropping_order[i]->_type << ' ' << dropping_order[i]->_dropping_id 
	   << ":" << dropping_order[i]->item_id()
	   << "-> " << vote;
      if (dropping_order[i]->_type == 'G')
	{
	  gt = (GroupedItem*)dropping_order[i];
	  if (gt->_p_group->_id != group_done)
	    {
	      group_done = gt->_p_group->_id;
	      sum = gt->_p_group->report_sum(pb);
	      strm << "\nGroup " << group_done << ", " <<
		gt->_p_group->_no_in_group << " items. Sum = "
		   << sum;
	    }
	}
      if (dropping_order[i]->_type == 'S')
	{
	  time_t time_stamp;
	  streampos offset;
	  time_stamp =  ((TimeStampItem*)dropping_order[i])->pull_time(pb, &offset);
	  strm << "/" << time_stamp;
	}
    }
  strm.flush();
}
#endif
#ifdef PETITION
// ***************************************************
// For Conf::drop_non_signers() - decides if
// the ballot has any votes in it.
YESorNO
ItemList::signed_anything(Ballot *p_ballot)
{
  int i = 0;
  Item * p_item;
  while (dropping_order[++i] != NULL)
    {
      p_item = dropping_order[i];
      if (p_item->_type == 'G')
	{
	  i += ((GroupedItem*)p_item)->p_group()->_no_in_group - 1;
	}
      switch (p_item->report_vote(p_ballot))
	{
	case READ:
	case NOT_READ:
	  continue;
	  break;
	case UNREAD_MIN:
	default:
	  return YES;
	  break;
	}
    }
  return NO;
}
#endif

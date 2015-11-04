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

/* $Id: itemgrou.cc,v 1.4 2003/01/15 18:54:07 marilyndavis Exp $ */ 
// itemgrou.cc controls a group of items 
/*********************************************************
 **********************************************************/
#include <iostream.h>
#include <fstream.h>
#include <strstream.h>
#include <stdio.h>
#include <stdlib.h>
#include "evotedef.h"
#include "conf.h"
#include "itemgrou.h"
#include "item.h"
#include "itemlist.h"
GLOBAL_INCS
extern int msgmni;
extern int msgmax;
#ifdef EDEBUG
void vmem(char* msg, YESorNO free = NO, int size = 0);
#endif
//  *************************************************
//   ITEMGROUP member functions.
ItemGroup::ItemGroup(Conf *p_conf, istream & strm, short id):
  _min_sum(-1), _id(id), _in_so_far(0), _p_conf(p_conf)
{
  strm.read((char*)(&long_byte), (char*)(&_sum_limit) - (char*)(&long_byte)
	    + sizeof(_sum_limit));
  _byte = (streampos)long_byte;
  _p_item = new GroupedItem*[_no_in_group];
  _space = _no_in_group;
#ifdef EDEBUG
  if (edebug & MEMS)
    {
      char msg[100];
      sprintf(msg,"Made new Grouped Item pointers, size = %d",
	      _no_in_group * sizeof(GroupedItem*));
      vmem(msg, NO, _no_in_group * sizeof(GroupedItem*));
    }
#endif  
  p_conf->items().groups().attach(this);
}
ItemGroup::ItemGroup(Conf *p_conf, short limit, short no):
  
  _min_sum(-1), _no_in_group(no), _sum_limit(limit), _in_so_far(0),
  _p_conf(p_conf)
{
  _p_item = new GroupedItem*[no];
#ifdef EDEBUG
  if (edebug & MEMS)
    {
      char msg[100];
      sprintf(msg,"Made new Grouped Item pointers, size = %d",
	      no * sizeof(GroupedItem*));
      vmem(msg, NO, no * sizeof(GroupedItem*));
    }
#endif  
  _space = no;
  _id = p_conf->items().groups().new_id();
  find_spot(&(p_conf->items()));
  p_conf->items().groups().attach(this);
}
// ************************************************************
ItemGroup::~ItemGroup(void)
{
  delete[] _p_item;
#ifdef EDEBUG
  if (edebug & MEMS)
    {
      vmem("ItemGroup deleted its list of pointers to items.", NO,
	   -sizeof(GroupedItem*)* _space);
    }
#endif
}
// ************************************************************
void
ItemGroup::attach(GroupedItem *p_new_item)
{
  int i;
  for (i = 0; i < _in_so_far; i++)
    {
      if (_p_item[i] == p_new_item)
	{
	  clerklog << "Tried to attach an already attached item "
		   << p_new_item->item_id() << " in conf " 
		   << _p_conf->name();
	  return;
	}
    }
  if (i >= _space)
    {
      int j;
      GroupedItem** old_list;
      old_list = _p_item;
      _p_item = new GroupedItem*[_space + IGBUNCH];
      if (_p_item == NULL)
	{
	  clerklog << "ItemGroup::attach:No system resouces to make more items.";
	  return;
	}
      for (j = 0; j < _in_so_far; j++)
	_p_item[j] = old_list[j];
      delete[] old_list;
      _space += IGBUNCH;
#ifdef EDEBUG
      if (edebug & MEMS)
	{
	  char msg[100];
	  sprintf(msg,"Remade new Grouped Item pointers, size  was = %d, now %d.",
		  (_space - IGBUNCH)  * sizeof(GroupedItem*),
		  _space * sizeof(GroupedItem*));
	  vmem(msg, NO, IGBUNCH * sizeof(GroupedItem*));
	}
#endif  
    }
  _p_item[i] = p_new_item;
  if (++_in_so_far >	_no_in_group)
    {
      clerklog << _p_conf->name() << " group " << _id 
	       << " tried to attach more items than it thought it should.";
      p_new_item->_status = (unsigned short)TROUBLE;
    }
  return;
}
// ************************************************************
short
ItemGroup::calc_sum(Ballot *p_ballot)
{ 
  int i;
  short vote;
  short sum = 0;
  for (i = 0; i < _no_in_group; i++)
    {
      if ((vote = _p_item[i]->report_fixed_vote(p_ballot)) == READ
	  || vote == NOT_READ || vote == LATE || vote == NOT_VALID
	  || vote == WAS_ARCHIVE)
	{
	  continue;
	}
      sum += (short)vote;
    }
  return sum;
}
//  ******************************
//     All the items in the group change their vstatus together.
OKorNOT
ItemGroup::change_vstatus(VSTATUS new_vstatus, 
			  unsigned long uid_asking)
{
  int i;
  for (i = 0; i < _no_in_group; i++)
    {
      _p_item[i]->Item::change_vstatus(new_vstatus, uid_asking);
    }
  return store_me();
}
// *****************************************************
OKorNOT
ItemGroup::check(Ballot* p_ballot)
{
  short check_sum;
  short store_sum;				
  check_sum = calc_sum(p_ballot);
  store_sum = report_sum(p_ballot);
  if (check_sum != store_sum)
    {	
      *(short*)((char*)p_ballot +_byte) = (short)check_sum;
      if (store_sum != TWO_NOT_READS || check_sum != 0)
	{
	  clerklog << "Discrepancy in item group sum for group "
		   << _id << ".  First item is " << _p_item[0]->dropping_id()
		   << ":" << _p_item[0]->item_id() << ".  Calc sum is "
		   << check_sum << "; stored sum is " << store_sum
		   << ".  Storing Calc sum.";
	  return NOT_OK;
	}
    }				
  return OK;
}
// ************************************************************
void
ItemGroup::check_kind(void)
{
  int i;
  short max = 0;
  short min = 0;
  char the_kind = '0';
  for (i = 0; i < _no_in_group; i++)
    {
      if (_p_item[i]->_max_vote > max)
	{
	  max = _p_item[i]->_max_vote;
	}
      if (_p_item[i]->_min_vote < min)
	{
	  min = _p_item[i]->_min_vote;
	}
    }
  if (max == 1 && min == 0)
    the_kind = 'Y';
  else if (max <= 9)
    the_kind = '9';
  else
    the_kind = '0';
  for (i = 0; i < _no_in_group; i++)
    {
      _p_item[i]->_kind = the_kind;
    }
}
// ************************************************************
void
ItemGroup::detach(GroupedItem* p_old_item)
{
  int i = -1;
  while (_p_item[++i] != p_old_item)
    {
      if (i >= _no_in_group)
	{
	  clerklog << "Can't find item " << p_old_item->item_id()
		   << " for detaching from Item Group";
	  return;
	}
    }
  while (++i <= _no_in_group)
    {
      _p_item[i-1] = _p_item[i];
    }
  _no_in_group--;
}
//  ***********************************************
//     makes a place in the ballot for the voter's sum_limit
void
ItemGroup::find_spot(ItemList* p_tl)
{
  _byte = p_tl->inc_next_byte();
  p_tl->inc_next_byte();   // 2 bytes for the sum of the group
}
//  *********************************************
//     calculates how many more items are after this
//     one in the group.  Used by GroupedItem::put_info
short
ItemGroup::how_many_more(GroupedItem* gi)
{
  short more = _no_in_group -1;
  int i = -1;
  while (_p_item[++i] != gi)
    {
      if (i >= _no_in_group)
	return -1;
      more--;
    }
  return more;
}
// ************************************************************
YESorNO
ItemGroup::isempty(void)
{
  if (_no_in_group == 0)
    {
      return YES;
    }
  return NO;
}
//  ***************************************************
short
ItemGroup::mark_delete_group(unsigned long uid)
{
  int i = -1;
  short start = 0, end = 0;
  short deletes = 0;
  while (start < 1  && ++i < _no_in_group)
    {
      if (_p_item[i] == NULL)
	continue;
      start = _p_item[i]->_dropping_id;
    }
  i = _no_in_group;
  while (end < 1 && --i >= 0)
    { 		
      if (_p_item[i] == NULL)
	continue;
      end = _p_item[i]->_dropping_id;
    }
  if (end >= start)
    {
      _p_conf->items().dropping_order.delete_from(start, end);
    }
  for (i = 0; i < _no_in_group; i++)
    {
      if (_p_item[i] != NULL)
	{
	  _p_item[i]->mark_delete(uid);
	  deletes++;
	}
    }
  return deletes;
}
// ********************************************************
//    Marks the vote for each item in the group as 'vote'.
//    vote can be UNREAD_MIN, READ, or NOT_READ
//    The behavior is the same for READ and NOT_READ.
void
ItemGroup::mark_group(Ballot* p_ballot, short vote, RTYPE *cc)
{
  int i;
  short old_vote, new_vote; 
  short *p_sum;
  p_sum = (short*)((char*)p_ballot + _byte);
  *p_sum = 0;
  for (i = 0; i < _no_in_group; i++)
    {
      old_vote = _p_item[i]->report_vote(p_ballot);
      switch (vote)
	{
	case UNREAD_MIN:
	  *p_sum += (new_vote = (_p_item[i]->_min_vote < 0 ? 0 
				 : _p_item[i]->_min_vote));
	  switch (old_vote)
	    {
	    case READ:
	      old_vote = 
		_p_item[i]->TalliedItem::mark_vote(p_ballot, new_vote, cc);
	      break;
	    case NOT_READ:
	      _p_item[i]->TalliedItem::add_vote(new_vote);
	      *(unsigned char*)((char*)p_ballot 
				+ _p_item[i]->_byte)
		= PACK_VOTE(UNREAD_MIN);
	      break;
	    }
	  break;
	case READ:
	case NOT_READ:
	  switch (old_vote)
	    {
	    case UNREAD_MIN:
	      _p_item[i]->TalliedItem::change_vote((_p_item[i]->_min_vote < 0?
						    0 : _p_item[i]->_min_vote),
						   NOT_READ);
	      *(unsigned char *)((char*)p_ballot + _p_item[i]->_byte) 
		= PACK_VOTE(NOT_READ);
	      break;
	    default:
	      _p_item[i]->TalliedItem::mark_vote(p_ballot, vote, cc);
	      break;
	    }
	}
    }
  *cc = MORE_STATS;
}
// ********************************************************
short
ItemGroup::mark_vote(Ballot* p_ballot, short vote, 
		     RTYPE * cc, GroupedItem* p_item, short old_vote)
{
  short new_sum = 0;
  short *p_sum;
  p_sum = (short*)((char*)p_ballot + _byte);
  if (*p_sum == TWO_NOT_READS)
    *p_sum = 0;
  old_vote = p_item->fix(old_vote);
  switch (vote)
    {
    case READ:
    case NOT_READ:
      mark_group(p_ballot, vote, cc);
      return old_vote;
      if (*p_sum != 0)
	clerklog << "Whoops on READ, NOT_READ!";
      break;
      //  incoming vote is never == UNREAD_MIN
    default:
      // first switch prepares
      switch (old_vote)
	{
	case NOT_READ:  //NOT_READ never happens except in mail
	case READ:  // first vote in group
	  // here we force the min vote on the whole group
	  if ((new_sum = min_sum() 
	       - (p_item->_min_vote < 0 ? 0 : p_item->_min_vote) 
	       + vote) > _sum_limit)
	    {
	      *cc = VIOLATES_SUM;
	      return old_vote;
	    }
	  mark_group(p_ballot, UNREAD_MIN, cc); 
	  break;
	default:
	  if ((new_sum = *p_sum + vote - old_vote)
	      > _sum_limit)
	    {
	      *cc = VIOLATES_SUM;
	      return old_vote;
	    }
	  break;
	}
      if (vote != (p_item->_min_vote < 0 ? 0 : p_item->_min_vote)
	  || (old_vote != READ && old_vote != NOT_READ))
	p_item->TalliedItem::mark_vote(p_ballot, vote, cc);
    }
  *p_sum = new_sum;
  return old_vote;
}
// ************************************************
short
ItemGroup::min_sum(void)
{
  int i;
  if (_min_sum > -1)
    return _min_sum;
  _min_sum = 0;
  for (i = 0; i < _no_in_group; i++)
    {
      _min_sum += (_p_item[i]->_min_vote < 0 ? 0 : _p_item[i]->_min_vote);
    }
  return _min_sum;
}
// ************************************************************
short
ItemGroup::report_sum(Ballot *p_ballot)
{
  short sum;
  if (p_ballot == NULL)
    return 0;
  sum =  *(short *)((char*)p_ballot + _byte);
  return (sum == TWO_NOT_READS? 0 : sum);
}
// ************************************************************
long
ItemGroup::send_stats(ITEM_STAT *here, 
		      Ballot* p_ballot, YESorNO start)
{
  long len = 0;
  ITEM_STAT *pt = here;
  static int i = 0;
  if (start == YES)
    i=0;
  for (; i < _no_in_group; i++)
    {
      if ((long)(len + sizeof(ITEM_STAT)) > msgmax )  // used to be msgmni
	break;
      len += _p_item[i]->report_stat(pt, LIST, p_ballot);
      pt++;
    }
  return len;
}
// ************************************************************
OKorNOT
ItemGroup::store(ostream& strm)
{
  long_byte = (long)_byte;
  strm.write((char*)(&long_byte), (char*)(&_sum_limit) - (char*)(&long_byte)
	     + sizeof(_sum_limit));
  return OK;
}
// ************************************************************
OKorNOT 
ItemGroup::store_me(void) 
{
  int i;
  ostream& strm = (ostream&)_p_conf->open_info();
  if (!strm)
    return NOT_OK;
  strm.ostream::seekp(_p_item[0]->_offset);
  for (i=0; i < _no_in_group; i++)
    {
      if (_p_item[i]->_offset == _p_item[0]->_offset && i!=0)  // only happens
	_p_item[i]->_offset = strm.tellp();        // when created
      strm.ostream::seekp(_p_item[i]->_offset);
      _p_item[i]->GroupedItem::store(strm);
    }
  _p_conf->close_info();
  return OK;
}

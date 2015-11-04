/* $Id: gitem.cc,v 1.3 2003/01/15 18:54:07 marilyndavis Exp $ */ 
// gitem.cpp - member functions for the GroupedItem class
/*********************************************************
 **********************************************************/
#include <iostream.h>
#include <fstream.h>
#include <strstream.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "evotedef.h"
#include "itemgrou.h"
#include "item.h"
#include "itemlist.h"
#include "ballotbo.h"
#include "conf.h"
GLOBAL_INCS
#include "conflist.h"
extern ConfList conferences;
#ifdef EDEBUG
void vmem(char* msg, YESorNO free = NO, int size = 0);
#endif
short GroupedItem::more_to_come = 0;
short
GroupedItem::report_fixed_vote(Ballot *p_ballot)
{
  short vote;
  vote = TalliedItem::report_fixed_vote(p_ballot);
  vote = fix(vote);
  return vote;
}
short
GroupedItem::fix(short vote)
{ 
  return ((vote == UNREAD_MIN) ? 
	  (_min_vote < 0 ? 0 : _min_vote)
	  : vote);
}
//  *************************************************
//   GROUPEDITEM member functions 
GroupedItem::GroupedItem(Conf* p_conf, ITEM_INFO* p_app_item,
			 streampos offset):
  TalliedItem(p_conf,p_app_item, offset)
{
  if (p_app_item->eVote.more_to_come ==   // first in group
      p_app_item->eVote.no_in_group -1)
    {
      _p_group = new ItemGroup(p_conf,
			       p_app_item->eVote.sum_limit, 
			       p_app_item->eVote.no_in_group);
#ifdef EDEBUG
      if (edebug & MEMS)
	{
	  char msg[100];
	  sprintf(msg,"Made new ItemGroup, size = %d",
		  sizeof(ItemGroup));
	  vmem(msg, NO, sizeof(ItemGroup));
	}
#endif  
    }
  else
    {
      _p_group = p_conf->items().groups().last_group();
    }
  _p_group->attach(this);
  _type = 'G';
  if (p_app_item->eVote.more_to_come == 0)  // last in group
    {
      if (_p_group->_in_so_far != _p_group->_no_in_group)
	_status = (STATUS)TROUBLE;
      _p_group->check_kind();
    }
}
GroupedItem::GroupedItem(Conf* p_conf, istream& strm)
  :TalliedItem(p_conf, strm)
{
  unsigned short id;
  static long last_id = 0;
  static short items_left = 0;
  static ItemGroup *p_last_group = NULL;
  char ch;
  strm >> id ;		
  if ((STATUS)_status == DUPLICATE)
    {
      while (1)
	{
	  ch = strm.get();
	  if (ch == 'F' || ch == 'X' || ch == 'M')
	    return;
	}
    }
  while (1)
    {
      switch (ch = strm.get())
	{
	case 'F':  // first item in group
	  if (last_id != 0)
	    {
	      clerklog << "Did not read in all the items for item group "
		       << last_id << " in conf " << p_conf->name() << ".  " 
		       << items_left << " lost.  Group deleted.";	
	      _p_conf->items().groups().mark_delete(p_last_group, 0);									
	    }
	  _p_group = new ItemGroup(p_conf, strm, id);
#ifdef EDEBUG
	  if (edebug & MEMS)
	    {
	      char msg[100];
	      sprintf(msg,"Made new ItemGroup, size = %d",
		      sizeof(ItemGroup));
	      vmem(msg, NO, sizeof(ItemGroup));
	    }
#endif  
	  /*	  if (_p_group->_no_in_group == 1)
		  {
		  clerklog << "Only one Item in Group " << id 
		  << " in conf " << p_conf->name();
		  _status = (STATUS)TROUBLE;
		  return;
		  } */
	  items_left = more_to_come = _p_group->_no_in_group - 1;
	  last_id = id;
	  p_last_group = _p_group;
	  if (_p_group->_no_in_group > 1)
	    break;
	case 'X':  // last one in the group
	  _p_group = p_last_group;
	  if (--items_left > 0)
	    {
	      clerklog << "Cannot find " << items_left 
		       << " items for group " << id << " in conf "
		       << p_conf->name();
	      _status = (STATUS)TROUBLE;
	      return;
	    }
	  if (id != last_id)
	    {
	      clerklog << "Getting item for group " << id 
		       << " instead of " << last_id << " in conf "
		       << p_conf->name();
	      _status = (STATUS)TROUBLE;
	      return;
	    }
	  more_to_come = 0;
	  last_id = 0;
	  break;
	case 'M':  // more to come
	  _p_group = p_last_group;
	  if (--items_left < 0 || id != last_id )
	    {
	      clerklog << "Trouble reading grouped items: group "
		       << last_id << " expects " << items_left 
		       << ". Now reading " << id << "in conf"
		       << p_conf->name();
	      _status = (STATUS)TROUBLE;
	      return;
	    }
	  more_to_come = items_left;
	  break;
	case ' ':
	  continue;
	  break;
	default:
	  clerklog << "GroupedItem::GroupedItem: unexpected " << ch
		   << "\nComing down now.";
	  conferences.store_all();
	  exit(1);
	  continue;
	  break;
	}
      _p_group->attach(this);
      if (ch == 'X')  // last in group
	_p_group->check_kind();
      if ((STATUS)_status == TROUBLE)
	{
	  p_last_group = NULL;
	  last_id = 0;
	  items_left = 0;
	}
      return;
    }	
}
// *****************************************
//   change the vote status
OKorNOT
GroupedItem::change_vstatus(VSTATUS new_vstatus, 
			    unsigned long uid_asking)	
{
  if (_author != uid_asking)
    return NOT_OK;
  if (_vstatus == new_vstatus 
      || (new_vstatus == CLOSED && _vstatus == CLOSED))
    return UNDECIDED;
  return _p_group->change_vstatus(new_vstatus, uid_asking);
}
//  ****************************************
//  3 modes like Item::check
OKorNOT
GroupedItem::check(Ballot* p_ballot)
{
  OKorNOT all_ok;
  all_ok = TalliedItem::check(p_ballot);
  if (this == _p_group->first() 
      && p_ballot != NULL
      && p_ballot->action() != DROP
      && _p_group->check(p_ballot) != OK)
    return NOT_OK;
  return all_ok;
}
// ***************************************************************
//  virtual OKorNOT GroupedItem::how_voted(char *here, Conf *p_conf, unsigned long uid)
//  Finds uid's vote and reports it to 'here'
OKorNOT
GroupedItem::how_voted(char* here, Conf *p_conf, unsigned long uid, 
		       unsigned long uid_asking)
{
  Ballot* ballot;
  long dummy_block;
  short vote_sum;
  short the_vote;
  if ((PRIV_TYPE)_priv_type == PRIVATE && uid != uid_asking)
    {
      sprintf(here,"Item #%d is PRIVATE.  You can not see another's vote.  Q!",
	      _dropping_id);
      return UNDECIDED;
    }
  if ((ballot = p_conf->community().get_ballot(uid)) == NULL
      && (ballot = p_conf->ballot_box().find_ballot(uid, dummy_block)) 
      == NULL)
    return NOT_OK;
  the_vote = report_fixed_vote(ballot);
  if (the_vote != LATE && the_vote != NOT_VALID && the_vote != WAS_ARCHIVE)
    {
      vote_sum = _p_group->report_sum(ballot);
    }
  sprintf(here, FEN_HOW_VOTED, EEN_HOW_VOTED);
  return OK;
}
// ************************************************************
//   void mark_bytes(streampos bytes[])
void
GroupedItem::mark_bytes(streampos bytes[])
{
  TalliedItem::mark_bytes(bytes);
  bytes[p_group()->_byte]++;
  bytes[p_group()->_byte+1]++;
}
// ***********************************************
//  short GroupedItem::mark_read(Ballot* p_ballot)
// ************************************************   
short
GroupedItem::mark_read(Ballot* p_ballot)
{ 
  short old_vote;
  switch (old_vote=report_vote(p_ballot))
    {
    case NOT_READ:
      *(unsigned char *)((char*)p_ballot + _byte) = PACK_VOTE(READ);
      _readers++;
      break;
    case UNREAD_MIN:
      *(unsigned char*)((char*)p_ballot + _byte) 
	= PACK_VOTE(_min_vote < 0 ? 0 : _min_vote);
      _readers++;
      break;
    case READ:
    default:
      break;
    }
  return old_vote;
}
//  ****************************************************
//   short GroupedItem::mark_vote(Ballot* p_ballot, short vote, RTYPE *cc)
short
GroupedItem::mark_vote(Ballot* p_ballot, short vote, RTYPE *cc)
{		
  short old_vote;
  old_vote = report_vote(p_ballot);
  if (_vstatus == CLOSED)
    {
      *cc = NO_MODS;
      return fix(old_vote);
    }
  if (vote != READ && vote != NOT_READ 
      && (vote < _min_vote || vote > _max_vote))
    {
      *cc = NOT_GOOD;
      return fix(old_vote);
    }
  if (old_vote == vote)
    {
      *cc = NO_CHANGE;
      return old_vote;  // can't be UNREAD_MIN
    }
  if ((old_vote == NOT_READ || old_vote == UNREAD_MIN)  // for mail only
      && vote != NOT_READ)
    mark_read(p_ballot);
  return _p_group->mark_vote(p_ballot, vote, cc, this, old_vote);
}
//  ******************************************************
//   virtual void put_info(ITEM_INFO *here)
void
GroupedItem::put_info(ITEM_INFO *here)
{
  TalliedItem::put_info(here);
  if (_kind == 'Y')
    here->eVote.type = GROUPEDY;
  else
    if (_kind == '9')
      here->eVote.type = GROUPEDn;
    else
      here->eVote.type = GROUPEDN;
  here->eVote.more_to_come = _p_group->how_many_more(this);
  here->eVote.sum_limit = _p_group->_sum_limit;
  here->eVote.no_in_group = _p_group->_no_in_group;
}
//  ************************************************
//  long report_stat(ITEM_STAT* here, STAT_TYPE type = NORMAL, 
//                    Ballot *p_ballot = NULL)
//     returns the length of the string it wrote.
long
GroupedItem::report_stat(ITEM_STAT* here, STAT_TYPE type, 
			 Ballot *p_ballot, short old_vote)
{
  short vote = UNKNOWN;
  short my_sum = -1;
  char vote_str[8];
  char ave_str[8];
  char sum_str[6];
  unsigned len;
  strcpy(vote_str,"  - ");
  strcpy(ave_str,"  *?*");
  strcpy(sum_str,"  *?*");
  init_stat(here);
  here->my_sum = 0;
  if (this == _p_group->first())
    {
      if (p_ballot != NULL)
	{
	  my_sum = _p_group->report_sum(p_ballot);
	  here->my_sum = my_sum;
	}
    }
  report_vote_str(here->vote_str, p_ballot);
  if (_vstatus != UNSEEN)
    {
      switch (_kind)
	{
	case 'Y':
	  sprintf(sum_str, " %4lu", (unsigned long)_sum);
	  break;
	case '0':
	  sprintf(ave_str, " %4.1f", _ave);
	  break;
	case '9':
	  sprintf(ave_str, " %4.2f", _ave);
	  break;
	}
    }
  if (p_ballot != NULL)
    {
      switch (vote = report_fixed_vote(p_ballot))
	{
	case LATE:
	  strcpy(vote_str, " -o-");
	  break;
	case WAS_ARCHIVE:
	case NOT_VALID:  // read_only or ballot waiting to drop
	  strcpy(vote_str, " XXX");
	  break;
	case NOT_READ:
	  break;
	case READ:
	  strcpy(vote_str, " ACC");
	  break;
	default:
	  if (_kind == 'Y')
	    {
	      if (vote == 1)
		strcpy(vote_str," yes");
	      else
		strcpy(vote_str,"  no");
	    }
	  else
	    sprintf(vote_str," %3d", vote);
	  break;
	}
    }
  switch (type)
    {
    case LIST:
      if (_kind == 'Y')
	len = sprintf(here->text, FEN_GSTATY, EEN_GSTATY);
      else
	len = sprintf(here->text, FEN_GSTAT, EEN_GSTAT);
      break;
    case NORMAL:
      if (_kind == 'Y')
	len = sprintf(here->text, FEN_GSTATY, EEN_GSTATY);
      else
	len = sprintf(here->text, FEN_GSTAT, EEN_GSTAT);
      break;
    case WITH_VOTE:
      my_sum = _p_group->report_sum(p_ballot);
      if (_kind == 'Y')
	len = sprintf(here->text, FEN_GSTATYV, EEN_GSTATYV);
      else
	len = sprintf(here->text, FEN_GSTATV, EEN_GSTATV);
      break;
    }
  if (len > sizeof(here->text))
    len += (sizeof(ITEM_STAT) - sizeof(here->text));
  else 
    len = sizeof(ITEM_STAT);
  return (long)len;
}
long GroupedItem::send_stats(ITEM_STAT* here, 
			     Ballot* p_ballot, YESorNO start)
{return p_group()->send_stats(here, p_ballot, start);}
//  ************************************************************
//   OKorNOT GroupedItem store(ostream& strm)
OKorNOT
GroupedItem::store(ostream& strm)
{		
  TalliedItem::store(strm);
  strm << _p_group->_id << ' ';		
  if (this == _p_group->first())
    {
      strm << 'F';
      _p_group->store(strm);
    }
  else if (this == _p_group->last())
    {
      strm << 'X';
    }
  else
    {
      strm << 'M';
    }
  return OK;
}
//   *****************************************
//   OKorNOT Grouped Item::store_me()
OKorNOT
GroupedItem::store_me(void)
{
  YESorNO newitem = NO;
  ostream& strm = (ostream&)_p_conf->open_info();
  if (!strm)
    return NOT_OK;
  strm.ostream::seekp(_offset);
  if (_p_group->first() == this)
    {
      if (_p_group->_no_in_group == 0)  // This happens when the
	{                               // item is brand new.
	  newitem = YES;
	  _p_group->_no_in_group = more_to_come + 1;
	}
    }
  store(strm);
  if (newitem == YES)
    _p_group->_no_in_group = 0;
  _p_conf->close_info();
  return OK;
}

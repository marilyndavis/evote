/* $Id: item.cc,v 1.3 2003/01/15 18:54:07 marilyndavis Exp $ */ 
// item.cc - member functions for the Item class, base of Item hierarchy
/*********************************************************
 **********************************************************/
#include <iostream.h>
#include <fstream.h>
#include <strstream.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "evotedef.h"
#include "item.h"
#include "itemlist.h"
#include "ballotbo.h"
#include "conf.h"
GLOBAL_INCS
extern int msgmax;
//  ***********************************************
// Member functions for Item class
//   ******************************************
Item::Item(Conf* p_conf, ITEM_INFO* p_app_item, streampos offset):_type('P'), 
  
  _dropping_id(p_app_item->dropping_id), _item_id(&p_app_item->static_id),
  _bit(0), _readers(0), _uid_delete(0),  _vstatus(p_app_item->eVote.vstatus), 
#ifdef TIME
  _open(p_app_item->eVote.open),
  _close(p_app_item->eVote.close),
#endif
  _status((unsigned char)ACTIVE),
  _byte(0),  _offset(offset),
  _p_conf(p_conf),  _next(NULL)
{
  _author = p_app_item->eVote.author;
#ifdef TITLE
  strcpy(_title, p_app_item->eVote.title);
#endif
}
Item::Item(Conf* p_conf, istream & strm)
  :_item_id(), _p_conf(p_conf), _next(NULL)
{
  char ch;
  _offset = strm.tellg();
  strm.get(_type);
  strm.get(ch);  // ' '
  strm.read((char*)(&_dropping_id), 
	    ((char*)(&_status) - (char*)(&_dropping_id)
	     + sizeof(_status)));
  _byte = (streampos)long_byte;
  strm.get(ch); // ' '
  if ((first_duplicate = _p_conf->items().wheres(_item_id)) != NULL)
    {
      _status = (unsigned char)DUPLICATE;
      _readers = first_duplicate->_readers;
    }
}
// ************************************************************
void
Item::mark_bytes(streampos bytes[])
{
  bytes[_byte]++;
}
// ********************************************
//  returns UNDECIDED if the old status is the same as the new
//          NOT_OK if it couldn't save the new status on disk
OKorNOT
Item::change_vstatus(VSTATUS new_vstatus, 
		     unsigned long uid_asking)
{
  if ((VSTATUS)_vstatus == new_vstatus)
    return UNDECIDED;
  if (new_vstatus == CLOSED && _vstatus == CLOSED)
    return UNDECIDED;
  //  _vstatus had the no_of_participants at closing time
  //  in it -- if there are at least 3.
  switch (new_vstatus)
    {
    case UNSEEN:
      if (_vstatus == CLOSED)
	{
	  return PROBLEM;
	}
      _vstatus = new_vstatus;
      break;
    case OPEN:
      return PROBLEM;
    case CLOSED:
    default:
      _vstatus = CLOSED;
      _participants_when_closed = _p_conf->no_of_participants();
#ifdef TIME
      time(&_close);
#endif
      break;
    }
  return store_me();
}
//  **************************************
//    This function has 3 modes:Start checking mode; iterate through
//    checking mode; and finish check mode.  The current mode is
//    decided by the checking flag and the p_ballot.
//    In mode 3, it writes to clerklog if there are discrepancies;
//    it fixes the statistics to match the data; and it return a
//    NOT_OK.  Other modes only return OK;
OKorNOT
Item::check(Ballot *p_ballot)
{
  short vote;
  if (_status != (unsigned char)ACTIVE 
      && _status != (unsigned char)CHECKING)
    return OK;
  // 2nd mode, interating through ballots
  if (p_ballot != NULL && (STATUS)_status == CHECKING
      && !(p_ballot->action() & DROP))
    {
      switch (vote = report_fixed_vote(p_ballot))
	{
	case NOT_VALID:
	  break;
	case READ:
	  ch_readers++;
	  break;
	case NOT_READ:
	  break;
	default:
	  clerklog << "Impossible case in check.";
	  break;
	}
      return OK;
    }
  if (p_ballot == NULL && (STATUS)_status == ACTIVE)  // starting mode
    {
      _status = (unsigned char)CHECKING;
      ch_readers = 0;					
      return OK;
    }
  if (p_ballot == NULL && (STATUS)_status == CHECKING) // ending mode
    {
      _status = (unsigned char)ACTIVE;
      if (_readers != ch_readers)
	{
	  clerklog << "Item " << _dropping_id << ":" << _item_id
		   << " corrected from " << _readers << " to "
		   << ch_readers << " readers.";
	  _readers = ch_readers;
	  return NOT_OK;
	}
      return OK;
    }
  clerklog << "Impossible to get here.";
  return NOT_OK;
}
//    ***********
//    find_spot , for a plain, not voted on item, assigns the bit in the
//                voter's record to indicate whether or not the voter has
//                read this item.
void
Item::find_spot(ItemList* p_tl)
{
  _byte = p_tl->fill_byte();
  _bit = p_tl->inc_next_bit();
  if (_bit == 8)
    {
      _bit = 0;
      p_tl->set_next_bit(1);
      _byte = p_tl->inc_next_byte();
      p_tl->set_fill_byte(_byte);
    }
}
// ***************************************************************
//   for the plain item case, it tells whether or not the uid has
//   accessed the item.  Put's the answer at char* here.
OKorNOT
Item::how_voted(char* here, Conf *p_conf, 
		unsigned long uid, unsigned long uid_asking)
{
  Ballot *ballot;
  short vote_sum = 0;  /* in FEN_HOW_VOTED */
  long dummy_block;
  short the_vote;
  if ((ballot = p_conf->community().get_ballot(uid)) == NULL
      && (ballot = p_conf->ballot_box().find_ballot(uid, dummy_block)) 
      == NULL)
    {
      return NOT_OK;
    }
  the_vote = report_fixed_vote(ballot);
  sprintf(here, FEN_HOW_VOTED, EEN_HOW_VOTED);
  return OK;
}
//  ******************************************************
void
Item::init_stat(ITEM_STAT *here)
{
  here->dropping_id = _dropping_id;
  here->my_sum = 0L;
  here->voters = 0L;
  here->pos_voters = 0L;
  here->neg_voters = 0L;
  here->pos = 0.;
  here->neg = 0.;
  here->sum_squares = 0.;
}
//  ******************************************************
//   marks the item for deletion later.
void
Item::mark_delete(unsigned long uid)
{
  _uid_delete = uid;
  _status = (unsigned char)DELETE;
  _dropping_id = 0;
}
//  ******************************************************
//     returns the old entry, READ or NOT_READ
short
Item::mark_read(Ballot* p_ballot)
{ 
  short cc;
  if ((cc=report_vote(p_ballot)) != NOT_READ)
    return cc;
  _readers++;
  *(unsigned char*)((char*)p_ballot + _byte) |= (01 << _bit);
  return NOT_READ;
}
// ***********************************************
short
Item::mark_vote(Ballot* p_ballot, short vote, RTYPE *cc)
{	
  short old_vote = report_vote(p_ballot);
  if (vote != NOT_READ && vote != WAS_ARCHIVE)
    {
      *cc = FAILURE;
      clerklog << "Tried to vote on a plain item!";	
      return old_vote;
    }
  if (old_vote == READ)
    {
      _readers--;
      *(unsigned char *)((char*)p_ballot + _byte) &= ~(01 << _bit);
    }
  return old_vote;
}
//  ******************************************************
void
Item::put_info(ITEM_INFO *here)
{
  here->static_id.local_id = _item_id._local_id;
  here->static_id.network_id = _item_id._network_id;
  here->dropping_id  = _dropping_id;
  here->eVote.no_in_group = 1;
  here->eVote.more_to_come = 0;
  here->eVote.sum_limit = 0;
  here->eVote.priv_type = PUBLIC;
  here->eVote.author = _author;
#ifdef TITLE
  strcpy(here->eVote.title, _title);
#endif
  here->eVote.open = _open;
  here->eVote.close = _close;
  here->eVote.type = PLAIN;
  here->eVote.vstatus = _vstatus;
  here->eVote.participants_when_closed = _participants_when_closed;
}
//  ************************************************
short
Item::report_fixed_vote(Ballot *p_ballot)
{
  ACTION action;
  if (((action = p_ballot->action()) & DROPPING
       && _close == (time_t)0) 
      || action & DROP)
    {
      return NOT_VALID;
    }
  if (action & READ_ONLY && p_ballot->mod_date() == 0l)
    return NOT_VALID;
  return report_vote(p_ballot);
}
//  ************************************************
//    Ballot *p_ballot = NULL, short old_entry = NOT_READ)
//     returns the length it wrote.
long
Item::report_stat(ITEM_STAT *here, STAT_TYPE type, 
		  Ballot *p_ballot, short old_entry)
{
  init_stat(here);
  report_vote_str(here->vote_str, p_ballot);
  switch (type)
    {
    case LIST:
      (void)sprintf(here->text, FEN_STAT, EEN_STAT);
      break;
    default:
      (void)sprintf(here->text, FEN_STAT, EEN_STAT);
      break;
    }
  return (long)sizeof(ITEM_STAT);
}
OKorNOT
Item::store(ostream & strm)
{
  strm << _type << ' ';
  long_byte = (long)_byte;
  strm.write((char*)(&_dropping_id), 
	     ((char*)(&_status) - (char*)(&_dropping_id)
	      + sizeof(_status)));
  strm << ' ';
  return OK;
}
//  *************************************************************
//      This is called to store just this item.  It is called when
//      the item is newly created in ItemList::create() and when
//      the vstatus changes.
OKorNOT
Item::store_me(void)
{
  ostream& strm = (ostream&)_p_conf->open_info();
  if (!strm)
    return NOT_OK;
  strm.ostream::seekp(_offset);
  store(strm);
  _p_conf->close_info();
  return OK;
}
// ************************************************************
//     Places a list of uids that answer the questions 'here'.
//     The only valid question for this plain Item is ACC or !ACC
long
Item::who_voted(char *here, Conf* p_conf, char *question, 
		YESorNO *p_continue)
{
  short target;
  long len = 0L;
  short this_vote;
  char vote_str[5];
  unsigned long uid;
  static Ballot *ballot;
  BallotBox& bbox= p_conf->ballot_box();
  if (strncmp(question, "ACC", 3) == 0
      || strncmp(question, "=ACC", 4) == 0
      || strncmp(question, "==ACC", 5) == 0)
    {
      target = READ;
    }
  else if (strncmp(question, "!ACC", 4) == 0
	   || strncmp(question, "=!ACC", 5) == 0
	   || strncmp(question, "<ACC", 4) == 0
	   || strncmp(question, "==!ACC", 6) == 0)
    {
      target = NOT_READ;
    }
  else
    {
      sprintf(here,"\nThere is no voting on Item #%d.  You can only ask:  \
\n'W ACC' to see who has accessed this item or \n'W !ACC' to see who has not \
accessed this item. Q!", _dropping_id);
      return -1L;
    }
  if (*p_continue == NO)
    ballot = bbox.iterator(START);
  else
    *p_continue = NO;
  if (ballot == NULL)
    {
      sprintf(here, "\nNo one.\n  Q!");
      return -1L;
    }
  do
    {
      if ((this_vote = report_fixed_vote(ballot)) == NOT_VALID
	  || this_vote == LATE || this_vote == WAS_ARCHIVE)
	continue;
      // these are the non-hits:
      if (this_vote != target)
	{
	  continue;
	}
      switch (this_vote)
	{
	case READ:
	  strcpy(vote_str,"ACC");
	  break;
	case NOT_READ:
	  strcpy(vote_str,"!ACC");
	  break;						
	default:
	  break;
	}
      uid = *(unsigned long*)ballot;
      if (len + 20L + (long)strlen(vote_str) > msgmax)
	{
	  *p_continue = YES;
	  break;
	}
      (void)sprintf(here+len, FEN_WHO_VOTED, EEN_WHO_VOTED);
      len += (long)strlen(here+len);
    }
  while ((ballot = bbox.iterator(NEXT)) != NULL);
  return len + 1L;    /* 1 for the final \0 */
}

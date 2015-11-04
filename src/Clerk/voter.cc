/* $Id: voter.cc,v 1.3 2003/01/15 18:54:07 marilyndavis Exp $ */ 
// voter.cc -- Implements things on the voter's behalf
/*********************************************************
 **********************************************************/
#ifdef linux
#define _POSIX_SOURCE
#endif
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <iostream.h>
#include <fstream.h>
#include <strstream.h>
#include "evotedef.h"
#include "ballotbo.h"
#include "qlist.h"
#include "item.h"
#include "itemlist.h"
#include "conf.h"
#include "voter.h"
#include "voterlis.h"
#include "ballot.h"
extern QList qlist;
GLOBAL_INCS
#ifdef EDEBUG
void vmem(char* msg, YESorNO free = NO, int size = 0);
#endif
//************************************************************
Voter::~Voter(void)
{
  if (_p_ballot != NULL)  /* NULL if it was a LeaveAdmin */
    {
      store_ballot();
      delete[] _p_ballot;
#ifdef EDEBUG
      if (edebug & MEMS)
	{
	  char msg[100];
	  sprintf(msg,"Deleted ballot for %lu", _uid);
	  vmem(msg, NO, -_p_conf->ballot_box().ballot_size());
	}
#endif  
    }
}
// ************************************************************
OKorNOT
Voter::change_action(ACTION new_action)
{
  OKorNOT cc;
  ACTION old_action = (ACTION)(_p_ballot->action() & ~LOCAL);
  _changed = YES;
  if ((cc = _p_ballot->change_action(new_action)) != OK)
    return cc;
  switch (new_action & ~LOCAL)
    {
    case EVERYTHING:
    case VACATION:
    case VOTE_ONLY:
    case SIGNER:
      switch (old_action)
	{
	case EVERYTHING:
	case VOTE_ONLY:
	case VACATION:
	case SIGNER:
	  break;
	case DROPPING:
	case DROP:
	case READ_ONLY:
	  _p_conf->ballot_box().change_no_of_participants(1L);
#ifdef EDEBUG
	  if (edebug & VOTERS)
	    {
	      dlog << "Voter::change_action adds to no_of_participants -->"
		   << _p_conf->ballot_box().no_of_participants();
	      (*dlogger).flush();
	    }
#endif
	  break;
	default:
	  // impossible
	  break;
	}
      break;
    case DROPPING:
    case DROP:
    case READ_ONLY:
      switch (old_action)
	{
	case EVERYTHING:
	case VOTE_ONLY:
	case VACATION:
	case SIGNER:
	  _p_conf->ballot_box().change_no_of_participants(-1L);
#ifdef EDEBUG
	  if (edebug & VOTERS)
	    {
	      dlog << "Voter::change_action substracts from no_of_participants -->"
		   << _p_conf->ballot_box().no_of_participants();
	      (*dlogger).flush();
	    }
#endif
	  break;
	case DROPPING:
	case DROP:
	case READ_ONLY:
	  break;
	default:
	  /* impossible */
	  break;
	}
    }
  return OK;
}
//************************************************************
// time_t Voter::pull_time -- finds the time stamp and
//        offset for the TimeStampItem
time_t
Voter::pull_time(Item * p_item, streampos * offset)
{
  if (p_item->type() != 'S')
    {
      *offset = (streampos)0;
      return (time_t)-1;
    }
  return ((TimeStampItem*)p_item)->pull_time(_p_ballot, offset);
}
// ************************************************************
// push_time -- puts in a new timestamp and offset and
//              returns the old ones.
time_t
Voter::push_time(Item *p_item, time_t new_time, 
		 streampos new_offset, streampos * old_offset)
{
  if (p_item->type() != 'S')
    {
      *old_offset = (streampos)0;
      return (time_t)-1;
    }
  return ((TimeStampItem*)p_item)->push_time(_p_ballot, new_time,
					     new_offset, old_offset);
}
// **************************************************
//     returns the old value READ or NOT_READ or the vote,
//     places a new stat in qlist.output if it changes the stat.
//************************************************************
long
Voter::read(Item *p_item, short *old_entry)
{
  long len;
  _changed = YES;
  *old_entry = p_item->mark_read(_p_ballot);
  len = p_item->report_stat((ITEM_STAT*)qlist.output, NORMAL, _p_ballot, *old_entry);
  return len;
}
//  *********************************************************
//    triggers store if _changed == YES
OKorNOT
Voter::store_ballot(void)    
{
  OKorNOT cc = OK;
  if (_changed != NO)
    if ((cc = _p_conf->store_ballot(_p_ballot, _block, _offset)) == OK)
      _changed = NO;
  return cc;
}
//  ******************************************************
//    reports the new stat to qlist.output, and the old_vote if the
//    vote has changed or is new.
//    Notice that the call has the RTYPE* first.  Putting it last
//    doesn't work somehow.  Then *cc gets changed to 0!
//   Returns the length of the ITEM_STAT.
long
Voter::vote(RTYPE *cc, Item* p_item, short vote,
	    short * old_vote)
{
  int len;
  if (_p_ballot->action() & VACATION)
    {
      *cc = NOT_ALLOWED;
      return 0L;
    }
  _changed = YES;
  *old_vote = p_item->mark_vote(_p_ballot, vote, cc);
  if (*cc == MORE_STATS)
    return 0L;
  len = p_item->report_stat((ITEM_STAT*)qlist.output, 
			    WITH_VOTE, _p_ballot, *old_vote);
  return len;
}
//************************************************************
fstream& 
operator<< (fstream &strm, Voter& voter)
{
  (ostream&)strm << voter.uid();
  return strm;
}

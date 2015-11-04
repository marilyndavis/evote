/* $Id: voterlis.cc,v 1.4 2003/01/15 18:54:07 marilyndavis Exp $ */ 
// voterlis.cc -- maintains the list of voters for a conference
/*********************************************************
 *    Copyright (c) 1994...2015 Deliberate.com Patented.
 *    by Marilyn Davis
 **********************************************************/
#include <stdlib.h>
#include <iostream.h>
#include <fstream.h>
#include <strstream.h>
extern "C" {
#include <string.h>
}
#ifdef linux
#include <sys/msg.h>
/*#include "/usr/src/linux/include/linux/errno.h"*/
#endif
#include <errno.h>
#include <signal.h>
#include "evotedef.h"
#include "voter.h"
#include "voterlis.h"
#include "conf.h"
#include "qlist.h"
#include "ballot.h"
#include "wholist.h"
extern WhoList wholist;
GLOBAL_INCS
extern QList qlist;
char * get_rtype(RTYPE rtype);
#ifdef EDEBUG
#include"debug.h"
#endif
// **************************************************
VoterList::~VoterList(void)
{
  Voter *x = _first;
  Voter *y;
  while (x != NULL)
    {
      y = x->_next;
      delete x;
#ifdef EDEBUG
      if (edebug & MEMS)
	{
	  vmem("Deleted a voter", NO, -sizeof(Voter));
	}
#endif
      x = y;
    }
}
// **************************************************
void
VoterList::attach_after(Voter *the_voter, Voter *after_voter)
{
  if (after_voter == NULL)    // first in list
    {
      if (_first != NULL)
	the_voter->_next = _first;
      _first = the_voter;
    }
  else
    {
      the_voter->_next = after_voter->_next;
      after_voter->_next = the_voter;
    }
#ifdef EDEBUG
  if (edebug & VOTERS)
    {
      Voter *x = _first;
      dlog << "Attached " << the_voter->uid() << " to " << _p_conf->name();
      while (x != NULL)
	{
	  dlog << "   " << x->uid();
	  x = x->_next;
	}
      (*dlogger).flush();
    }
#endif
}
// ************************************************
//     This gets called by ItemList::attach when a new plain
//     item needs a new byte.  On line voters need to fix up
//     their ballots.
void
VoterList::blank_voters(unsigned short byte, int no)
{
  Voter *voter = _first;
  int i;
  while (voter != NULL)
    {
      for (i = 0; i < no; i++)
	*((unsigned char*)voter->_p_ballot+byte+i) = (unsigned char)0;
      voter = voter->_next;
    }
}
// **************************************************
void
VoterList::detach(Voter *voter)
{
  Voter *x;
  Voter *comes_after;
#ifdef EDEBUG
  if (edebug & VOTERS)
    {
      static int t = 0;
      t++;
      Voter *x = _first;
      dlog << "Detaching " << voter->uid() << " from " << _p_conf->name();
      while (x != NULL)
	{
	  dlog << "   " << x->uid() << " at offset " << x->_offset;
	  x = x->_next;
	}
      (*dlogger).flush();
    }
#endif
  if ((x = wheres(voter->_uid, comes_after)) != voter)
    {
#ifdef EDEBUG
      if (edebug & VOTERS)
	{
	  dlog << "\nCan't detach " << voter->_uid 
	       << ":" << voter->pid() << " from " << _p_conf->name();
	  (*dlogger).flush();
	}
#endif
      return;
    }
  if ((comes_after) != NULL)
    (comes_after)->_next= voter->_next;
  else
    {
      _first = _first->_next;
    }
}
// **************************************************
Ballot*
VoterList::get_ballot(unsigned long uid)
{
  Voter* x;
  Voter *comes_after;
  if ((x = wheres(uid, comes_after)) == NULL)
    return NULL;
  return x->_p_ballot;
}
// **************************************************
Voter*
VoterList::find(unsigned long uid_in, 
		int pid_in, int qid_in, YESorNO force,
		int* p_trouble)
{
  Voter *x;
  Voter *comes_after;
  Ballot* p_ballot;
  long block;
  streampos offset;
  YESorNO y = NO;
  YESorNO &no = y;
  char name[300];
  if ((x = wheres(uid_in, comes_after)) != NULL)
    {
      if (x->_pid == pid_in)
	{
#ifdef EDEBUG
	  if (edebug & VOTERS)
	    {
	      dlog << "\nFound voter for pid " << pid_in
		   << ". Uid = " << x->_uid << ". Ballot at "
		   << x->_p_ballot;
	      (*dlogger).flush();
	      if (edebug & FLOWER)
		{
		  sprintf(debugger, 
			  "Voterlist found Voter for pid %d and uid %lu",
			  pid_in, x->_uid);
		  doutput(debugger);
		}
	    }
#endif
	  // If the voter drops from the conf and then returns
	  // the ballot may still exit as a DROPPING.  In this
	  // case, the Entering::Entering fails and Joining::Joining
	  // is called.  The Joining::Joining picks up the same
	  // Voter object but the qid needs to change.
	  x->_qid = qid_in;
	  return x;
	}
      // Here we have found a voter object for the uid with an old
      // pid.  Must have crashed off line.  Save his ballot, delete
      // him, and make a new voter object with the right pid
      if (kill(x->_pid, 0) == -1)
	// should fail because pid is dead
	// or because there's no permission
	//  0  means don't really do it, just check 
	{		
	  wholist.whois(name, x->_uid);
	  switch (errno)
	    {
	    case ESRCH:  // dead pid
#ifdef EDEBUG
	      if (edebug & VOTERS)
		{
		  dlog << "\nFound voter " << name << " but pid " << x->_pid
		       << "is dead. New pid = " <<  pid_in << " Uid = " 
		       << x->_uid << ". Ballot at "
		       << x->_p_ballot;
		  (*dlogger).flush();
		  if (edebug & FLOWER)
		    {
		      sprintf(debugger,
			      "Found voter %s but pid %d is dead. New pid %d"
			      " uid = %lu", name, x->_pid, pid_in, uid_in);
		      doutput(debugger);
		    }
		}
#endif
	      clerklog << "User " << x->_uid << ": " << name 
		       << " tried to enter "
		       << _p_conf->name() 
		       << " twice at once. Old pid is dead." 
		       << "\nDropping first queue: "
		       << x->_qid << " to dead pid: " << x->_pid 
		       << "\n  Keeping new queue: "
		       << qid_in << " to new pid: " << pid_in;
	      qlist.drop_q(x->_qid, no, YES);
	      forget(x);   // detach and delete  - delete stores ballot
#ifdef EDEBUG
	      if (edebug & MEMS)
		{
		  vmem("Deleted a voter for being on twice.", NO, -sizeof(Voter));
		}
#endif
	      break;
	    case EPERM: // pid still alive
#ifdef EDEBUG
	      if (edebug & VOTERS && edebug & FLOWER)
		{
		  sprintf(debugger,"Voterlist detects two live pids for %s.  older is %d. current is %d.  Voterlist returning NULL.", name, x->_pid, pid_in);
		  doutput(debugger);
		}
#endif
	      clerklog << "User " << x->_uid << " tried to enter "
		       << _p_conf->name() 
		       << " twice at once. Old pid is alive. "
		       << "\nDropping second queue: "
		       << qid_in << " to new pid: " << pid_in
		       << "\n  Keeping old queue: "
		       << x->_qid << " to old pid: " << x->_pid ;
	      *p_trouble = ++(x->_times_on_twice);
	      return NULL;
	      break;
	    default:
	      clerklog << "QList::drop_oldqs errno = " << errno;
	      break;
	    }
	}
      else // didn't fail - must be superuser and pid is still alive
	{
	  //  pid is alive still
	  *p_trouble = ++(x->_times_on_twice);
	  return NULL;
	}
    }
  if ((p_ballot = _p_conf->fetch_ballot(uid_in, block, offset, force)) 
      == NULL)
    return NULL;
#ifdef EDEBUG
  if (edebug & VOTERS)
    {
      dlog << "\nMaking voter for pid " << pid_in
	   << ". Uid = " << uid_in << ". Ballot at "
	   << p_ballot;
      (*dlogger).flush();
      if (edebug & FLOWER)
	{
	  sprintf(debugger,"VoterList making voter for pid %d and uid %lu",
		  pid_in, uid_in);
	  doutput(debugger);
	}
    }
#endif
  x = new Voter(_p_conf, uid_in, pid_in, qid_in, p_ballot, block, offset);
#ifdef EDEBUG
  if (edebug & MEMS)
    {
      char msg[100];
      sprintf(msg,"Made new voter for uid = %lu", uid_in);
      vmem(msg, NO, sizeof(Voter));
    }
#endif
  attach_after(x, comes_after);
  return x;
}
// **************************************************************
//  detaches and deletes the Voter
void
VoterList::forget(Voter* p_voter)
{
  detach(p_voter);
  delete p_voter;
#ifdef EDEBUG
  if (edebug == MEMS)
    {
      vmem("VoterList deleted voter", NO, -sizeof(Voter));
    }
#endif
}
//  *****************************************************
//  refetch_all()   - called after some big change in
//                    the data file.
OKorNOT
VoterList::refetch_all(void)
{
  Voter *p_voter = _first;
  OKorNOT all_ok = OK;
  while (p_voter != NULL)
    {
      delete[] (p_voter->_p_ballot);  // is this right?
#ifdef EDEBUG
      if (edebug & MEMS)
	{
	  vmem("Deleted a ballot in refetch_all", NO, 
	       -_p_conf->ballot_box().ballot_size());
	}
#endif
      if ((p_voter->_p_ballot 
	   = _p_conf->fetch_ballot(p_voter->_uid, 
				   p_voter->_block, p_voter->_offset, NO)) 
	  == NULL)
	{
	  all_ok = NOT_OK;
	}
      p_voter = p_voter->_next;
    }
  if (all_ok != OK)
    {
      clerklog << "Trouble refetching voters.";
      (*logger).flush();
    }
  return all_ok;
}
// *****************************************************************
//     Expects that the body of message is already in the
//     out_msg_queue->mtext.  Sends that message to each online voter.
//     Only used for demo.
// ******************************************************/
void
VoterList::send_all(RTYPE message_type, int len)
{
  Voter* x = _first;	
#ifdef EDEBUG
  if (edebug & MESSAGES && _first != NULL)
    {
      dlog << "Sending all voters in " << _p_conf->name()
	   << " a message: " << get_rtype(message_type);
    }
#endif
  while (x != NULL)
    {
      qlist.send((long)message_type, x->_qid, len);
      x = x->_next;
    }
}
// ****************************************************
void
VoterList::send_all(RTYPE message_type, char* msg, int len)
{
  qlist.put_msg(msg);
  send_all(message_type, len);
}
// **********************************
//   When a new ballot gets added, other on-line voters
//   in the same (always the conf's overflow block) block,
//   get shifted on disk.  This function does it to those
//   voters in memory.
OKorNOT
VoterList::slide_in(unsigned long uid_in, long shift_block, 
		    short rec_len)
{
  Voter* p_voter = _first;
  while (p_voter != NULL)
    {
      if (p_voter->_block == shift_block 
	  && p_voter->_uid > uid_in)
	{
	  p_voter->_offset += rec_len;
	}
      p_voter = p_voter->_next;
    }	
#ifdef EDEBUG
  if (edebug & VOTERS)
    {
      Voter *x = _first;
      dlog << "Slid in " << uid_in;
      while (x != NULL)
	{
	  dlog << "   " << x->uid() << " has offset " << x->_offset;
	  x = x->_next;
	}
      (*dlogger).flush();
    }
#endif
  return OK;
}
// ************************************************
OKorNOT
VoterList::store_all(void)
{
  Voter *p_voter = _first ;
  OKorNOT all_ok = OK;
  while (p_voter != NULL)
    {
      if (p_voter->store_ballot() != OK)
	all_ok = NOT_OK;
      p_voter = p_voter->_next;
    }
  return all_ok;
}
//  **********************************
//   returns a pointer to the Voter if it is on-line already
//   Otherwise, it returns a NULL
Voter *
VoterList::wheres(unsigned long uid_in, Voter *&belongs_after)
{
  Voter * checker = _first;
  belongs_after = NULL;
  while (checker != NULL
	 && checker->_uid < uid_in)
    {
      belongs_after = checker;
      checker = checker->_next;
    }
  if (checker == NULL || uid_in == checker->_uid)
    return checker;
  return NULL;
}
// **************************************************
ostream& 
operator << (ostream& strm, VoterList& list)
{
  Voter *x = list.first();
  char c = '\t';
  int i = 0;
  strm << '\n' << "qid:uid\n";
  while (x != NULL)
    {
      strm << x->pid() << x->_uid << c;
      x = x->_next;
      c = (++i%4 ? '\t' : '\n');
    }
  strm << '\n';
  return strm;
}

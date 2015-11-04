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

/* $Id: voter.h,v 1.3 2003/01/15 18:54:07 marilyndavis Exp $ */ 
// voter.h defines the Voter
/*********************************************************
 **********************************************************/
#ifndef VOTRDEF
#define VOTRDEF
#include<fstream.h>
class Ballot;
class Conf;
class Item;

class Voter
{
 public:
  Voter(Conf* p_conf, unsigned long uid, int pid, int qid, Ballot* pballot, 
	long block, streampos offset);
  ~Voter(void);
  OKorNOT change_action(ACTION action);
  time_t last_access(void);
  Voter* next(void);
  Ballot* p_ballot(void);
  Conf* p_conf(void);
  int pid(void);
  time_t pull_time(Item * p_item, streampos * offset);
  time_t push_time(Item * p_item, time_t new_time, streampos 
		   new_offset, streampos * old_offset);
  long read(Item* p_item, short * old_entry); 
  OKorNOT store_ballot(void); 
  unsigned long uid(void);
  long vote(RTYPE *cc, Item* p_item, short vote, 
	    short * old_vote); 
 private:
  Conf* _p_conf;
  unsigned long _uid;
  int _pid;
  int _qid;
  YESorNO _changed;
  Voter *_next;
  Ballot *_p_ballot;
  long _block;
  streampos _offset;
  int _times_on_twice;
  friend class VoterList;
  void set_next(Voter *p_voter);
  friend ostream& operator << (ostream&, VoterList&);			
  friend fstream& operator << (fstream&, Voter&);
};
#include "conf.h"
#include "ballot.h"
inline Ballot* Voter::p_ballot(void){return _p_ballot;}
inline Conf* Voter::p_conf(void){return _p_conf;} 
inline int Voter::pid(void){return _pid;} 
inline unsigned long Voter::uid(void){return _uid;}
inline Voter* Voter::next(void){return _next;}
inline void Voter::set_next(Voter *p_voter){_next = p_voter;}
inline
Voter::Voter(Conf* p_conf, unsigned long uid, int pid, int qid, 
	     Ballot* pballot, long block, streampos offset)
  :_p_conf(p_conf), _uid(uid), _pid(pid), _qid(qid), _changed(MAYBE),
  _next(NULL), _p_ballot(pballot), _block(block), _offset(offset),
  _times_on_twice(0)
{}
inline time_t Voter::last_access(void)
{  // last time the voter was in this conf.  BallotBox::store writes it.
  return _p_ballot->mod_date();
}
#endif

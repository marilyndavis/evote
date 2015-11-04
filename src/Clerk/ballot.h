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

/* $Id: ballot.h,v 1.3 2003/01/15 18:54:07 marilyndavis Exp $ */ 
/************************************************************
 *  ballot.h Header for functions that deal with the ballot's
 *           header and initialize ballots.  Does not deal
 *           with votes.
 *********************************************************
 **********************************************************/
#ifndef BALLOTDEF
#define BALLOTDEF
class Header
{
 private:
  unsigned long _uid;  // must be first!
  time_t _mod_date;
  short _action;
  short _filler;
  friend class BallotBox;
  friend class Ballot;
 public:
  Header(unsigned long uid);
  void start(unsigned long uid);
};
class Ballot
{
 private:
  Header _header;  // must be first!
  unsigned char _vote[RECLEN - sizeof(Header)];
  friend class Hash;
  friend class BallotBox;
 public:
  Ballot(unsigned long uid = NO_UID);
  ACTION action(void){return (ACTION)(_header._action);}
  OKorNOT change_action(ACTION new_action);
  YESorNO is_mail(void){return (this->action() & LOCAL ? NO : YES);}
  time_t mod_date(void){return _header._mod_date;}
  void new_date(void){time(&(_header._mod_date));}
  void set_date(time_t new_date){_header._mod_date = new_date;}
  void start(short ballot_size= RECLEN, unsigned long uid = NO_UID);
  unsigned long uid(void){return *(unsigned long*)this;}
};
#endif 

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

/* $Id: titem.cc,v 1.3 2003/01/15 18:54:07 marilyndavis Exp $ */ 
// titem.cpp - member functions for the TalliedItem class
/*********************************************************
 **********************************************************/
#include <iostream.h>
#include <fstream.h>
#include <strstream.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "evotedef.h"
#include "item.h"
#include "itemlist.h"
#include "ballotbo.h"
#include "conf.h"
GLOBAL_INCS
extern int msgmax;
#ifdef EDEBUG
extern int edebug;
#endif
// ************************************************************
void
TalliedItem::mark_bytes(streampos bytes[])
{
  bytes[_byte]++;
}
// ************************************************************
short
TalliedItem::report_fixed_vote(Ballot* p_ballot)
{
  ACTION action;
  if ((action = p_ballot->action()) & DROPPING
      || action & DROP)
    {
      if (this->vstatus() != CLOSED)
	return NOT_VALID;
      if (p_ballot->mod_date() >= _close)
	return report_vote(p_ballot);
      return NOT_VALID;
    }
  if (action & READ_ONLY && p_ballot->mod_date() == 0l)
    return NOT_VALID;
  return report_vote(p_ballot);
}
// ************************************************************
short
TalliedItem::report_vote(Ballot* p_ballot)
{
  return UNPACK_VOTE(*(unsigned char*)((char*)p_ballot + _byte));
}
//**************************************************************
//  TALLIEDITEM  member functions
TalliedItem::TalliedItem(Conf* p_conf, ITEM_INFO* p_app_item, 
			 streampos offset) 
  :Item(p_conf, p_app_item, offset), _ave(0.), 
  _max_vote(p_app_item->eVote.max),
  _min_vote(p_app_item->eVote.min),
  _priv_type((unsigned short)p_app_item->eVote.priv_type),
  _sum(0.),  _pos(0.), _neg(0.), _sum_squares(0.),
  _pos_voters(0L), _neg_voters(0L),
  _voters(0)
{
  if (_max_vote > MAX_POS)
    {
      clerklog << "Programmer error.  Votes must be less than " <<
	MAX_POS << ".  " << _max_vote << " is not allowed.";
      exit(0);
    }
  if (_min_vote < MIN_NEG)
    {
      clerklog << "Programmer error.  Votes must be greater than " <<
	MIN_NEG << ".  " << _min_vote << " is not allowed.";
      exit(1);
    }
  set_kind();
  _type = 'T';
}
//  ********************************
TalliedItem::TalliedItem(Conf* p_conf, istream &strm):Item(p_conf, strm)
{
  char ch;
  strm.read((char*)(&_ave), (char*)(&_voters) - (char*)(&_ave) +
	    sizeof(_voters));
  strm.get(ch); // ' '
  if ((STATUS)_status == DUPLICATE)
    {
      _voters = ((TalliedItem*)first_duplicate)->_voters;
      _sum = ((TalliedItem*)first_duplicate)->_sum;
      _pos = ((TalliedItem*)first_duplicate)->_pos;
      _neg = ((TalliedItem*)first_duplicate)->_neg;
      _pos_voters = ((TalliedItem*)first_duplicate)->_pos_voters;
      _neg_voters = ((TalliedItem*)first_duplicate)->_neg_voters;
      _sum_squares = ((TalliedItem*)first_duplicate)->_sum_squares;
    }
  if (_voters > 0L)
    _ave = _sum/_voters;
  set_kind();
}	
//  ********************************
//  add_vote  adds in a new vote for a voter who has not voted on this
//            item before
//            note that the range of the vote is not checked here but
//            is checked in the client application.
void
TalliedItem::add_vote(short vote)
{
  _voters++;
  _sum += vote;
  _sum_squares += vote * vote;
  _ave = _sum/_voters;
  if (vote > 0)
    {
      _pos_voters++;
      _pos += vote;
    }
  else if (vote < 0)
    {
      _neg -= vote;
      _neg_voters++;
    }
}
//  ***********************************
//   change_vote - also contains code for deleting a vote, which
//      the voter indicates by sending in a READ.  
void
TalliedItem::change_vote(short oldv, short newv)
{
  // old_vote is always a real vote if this is called
  if (oldv > 0)
    {
      _pos -= oldv;
      _pos_voters--;
    }
  else if (oldv < 0)
    {
      _neg += oldv;
      _neg_voters--; ;
    }
  _sum_squares -= oldv * oldv;
  _sum -= oldv;
  switch (newv)
    {
    case NOT_READ:
    case READ:
      _voters--;
      break;
    default:
      if (newv > 0)
	{
	  _pos_voters++;
	  _pos += newv;
	}
      else if (newv < 0)
	{
	  _neg_voters++; ;
	  _neg -= newv;
	}
      _sum_squares += newv * newv;
      _sum += newv;
      break;
    }
  if (_voters == 0L)
    _ave = 0.;	
  else
    _ave = _sum/_voters;
}
//  ****************************************
//  3 modes like Item::check
OKorNOT
TalliedItem::check(Ballot* p_ballot)
{
  OKorNOT all_ok = OK;
  short vote;
  unsigned char miner;
  if ((STATUS)_status != ACTIVE && (STATUS)_status != CHECKING)
    return OK;
  if (p_ballot == NULL && (STATUS)_status == ACTIVE)
    {
      _status = (unsigned char)CHECKING;
      ch_readers = 0L;
      ch_sum = 0.;
      ch_neg = 0.;
      ch_pos = 0.;
      ch_pos_voters = 0L;
      ch_neg_voters = 0L;
      ch_sum_squares = 0.;
      ch_voters = 0L;
      return OK;
    }
  if (p_ballot == NULL && (STATUS)_status == CHECKING)
    {
      _status = (unsigned char)ACTIVE;
      if (ch_readers != _readers)
	{
	  clerklog << "Item " << _dropping_id << ":" << _item_id
		   << " " << _title << " corrected from " << _readers << " to "
		   << ch_readers << " readers.";
	  all_ok = NOT_OK;
	  _readers = ch_readers;
	}
      if (ch_sum != _sum)
	{
	  clerklog << "Item " << _dropping_id << ":" << _item_id
		   << " " << _title << " corrected from " << _sum << " to "
		   << ch_sum << " sum.";
	  _sum = ch_sum;
	  all_ok = NOT_OK;
	}
      if (ch_voters != _voters)
	{
	  clerklog << "Item " << _dropping_id << ":" << _item_id
		   << " " << _title << " corrected from " << _voters << " to "
		   << ch_voters << " voters.";
	  _voters = ch_voters;
	  all_ok = NOT_OK;
	}
      if (ch_pos_voters != _pos_voters)
	{
	  clerklog << "Item " << _dropping_id << ":" << _item_id
		   << " " << _title << " corrected from " << _pos_voters << " to "
		   << ch_pos_voters << " positive voters.";
	  _pos_voters = ch_pos_voters;
	  all_ok = NOT_OK;
	}
      if (ch_neg_voters != _neg_voters)
	{
	  clerklog << "Item " << _dropping_id << ":" << _item_id
		   << " " << _title << " corrected from " << _neg_voters << " to "
		   << ch_neg_voters << " negative voters.";
	  _neg_voters = ch_neg_voters;
	  all_ok = NOT_OK;
	}
      if (ch_sum_squares != _sum_squares)
	{
	  clerklog << "Item " << _dropping_id << ":" << _item_id
		   << " " << _title << " corrected from " << _sum_squares << " to "
		   << ch_sum_squares << " sum of squares.";
	  _sum_squares = ch_sum_squares;
	  all_ok = NOT_OK;
	}
      if (ch_pos != _pos)
	{
	  clerklog << "Item " << _dropping_id << ":" << _item_id
		   << " " << _title << " corrected from " << _pos << " to "
		   << ch_pos << " sum of positives.";
	  _pos = ch_pos;
	  all_ok = NOT_OK;
	}
      if (ch_neg != _neg)
	{
	  clerklog << "Item " << _dropping_id << ":" << _item_id
		   << " " << _title << " corrected from " << _neg << " to "
		   << ch_neg << " sum of negatives.";
	  _neg = ch_neg;
	  all_ok = NOT_OK;
	}
      if (all_ok == NOT_OK)
	{
	  if (_voters > 0L)
	    _ave = _sum/_voters;
	  else
	    _ave = 0.;
	}
      return all_ok;
    }
  if (p_ballot != NULL && (STATUS)_status == CHECKING)
    {
      if (p_ballot->action() & DROP)
	return OK;
      vote = TalliedItem::report_fixed_vote(p_ballot);
      switch (vote)
	{
	case WAS_ARCHIVE:
	case LATE:
	case NOT_READ:
	case NOT_VALID:
	  break;
	case READ:
	  ch_readers++;
	  break;
	case UNREAD_MIN:
	  miner = (_min_vote < 0 ? 0 : _min_vote);
	  ch_voters++;
	  ch_sum += miner;
	  ch_sum_squares += miner * miner;
	  if (_min_vote > 0)
	    {
	      ch_pos += _min_vote;
	      ch_pos_voters++; ;
	    }
	  break;
	default:
	  ch_readers++;
	  ch_voters++;
	  ch_sum += vote;
	  ch_sum_squares += vote * vote;
	  if (vote > 0)
	    {
	      ch_pos_voters++;
	      ch_pos += vote;
	    }
	  else if (vote < 0)
	    {
	      ch_neg_voters++; ;;
	      ch_neg -= vote;
	    }
	  break;
	}
#ifdef EDEBUG
      if (edebug & ITEMS)
	{
	  static unsigned long last_guy;
	  static int v[10];
	  static int id[10];
	  static int vs[10];
	  static int rs[10];
	  static int vi = -1;
	  if (last_guy != 0 && last_guy != *(unsigned long*)p_ballot)
	    {
	      dlog << last_guy << "> " 
		   << id[0] << ":" << v[0] << " vs:" << vs[0] 
		   << " rs:" << rs[0] << " "
		// << id[1] << ":" << v[1] << " vs:" 
		// << vs[1] << " rs:" << rs[1] << " "
		// << id[2] << ":" << v[2] << " vs:" 
		// << vs[2] << " rs:" << rs[2] << " "
		// << id[3] << ":" << v[3] << " vs:" 
		// << vs[3] << " rs:" << rs[3] << " "
		// << id[4] << ":" << v[4] << " vs:" 
		// << vs[4] << " rs:" << rs[4] << " "
		   << id[5] << ":" << v[5] << " vs:" 
		   << vs[5] << " rs:" << rs[5] << " ";
	      //   << id[6] << ":" << v[6] << " vs:" 
	      //<< vs[6] << " rs:" << rs[6];
	      (*dlogger).flush();
	      vi = -1;
	    }
	  if (vi < 8)
	    {
	      v[++vi] = vote;
	      id[vi] = _dropping_id;
	      vs[vi] = ch_voters;
	      rs[vi] = ch_readers;
	    }
	  last_guy = *(unsigned*)p_ballot;
	}
#endif
      return OK;
    }
  clerklog << "Can't get here.";
  return NOT_OK;
}
// *****************************************
void
TalliedItem::find_spot(ItemList* p_tl)
{	
  _bit = 99;  // not used
  _byte = p_tl->inc_next_byte();
}
//  *********************************************
OKorNOT
TalliedItem::how_voted(char* here, Conf *p_conf, 
		       unsigned long uid, unsigned long uid_asking)
{
  if ((PRIV_TYPE)_priv_type == PRIVATE && uid != uid_asking)
    {
      sprintf(here,"Item #%d is PRIVATE.  You can not see another's vote.  Q!",
	      _dropping_id);
      return UNDECIDED;
    }
  return Item::how_voted(here, p_conf, uid, uid_asking);
}
//  ******************************************************
void
TalliedItem::init_stat(ITEM_STAT *here)
{
  Item::init_stat(here);
  here->voters = _voters;
  if (_vstatus == UNSEEN)
    return;
  here->sum_squares = _sum_squares;
  here->pos_voters = _pos_voters;
  here->neg_voters = _neg_voters;
  here->pos = _pos;
  here->neg = _neg;
}
// ***********************************************
short
TalliedItem::mark_read(Ballot* p_ballot)
{ 
  short old_vote;
  if ((old_vote=report_vote(p_ballot)) != NOT_READ)
    return old_vote;
  _readers++;
  *(unsigned char*)((char*)p_ballot + _byte) = PACK_VOTE(READ);
  return old_vote;
}
// ***********************************************
//     marks the new vote in the ballot.
//     Returns the old vote.
short
TalliedItem::mark_vote(Ballot* p_ballot, short vote, RTYPE *cc)
{
  short old_vote;
  old_vote = report_fixed_vote(p_ballot);
  if (p_ballot->action() & VACATION)
    {
      *cc = NOT_ALLOWED;
      return old_vote;
    }
  if (_vstatus == CLOSED)
    {
      *cc = NO_MODS;
      return old_vote;
    }
  if ((vote < _min_vote || vote > _max_vote) 
      && vote != READ && vote != NOT_READ && vote != WAS_ARCHIVE)
    {
      *cc = NOT_GOOD;
      return old_vote;
    }
  if (old_vote == vote)
    {
      *cc = NO_CHANGE;
      return old_vote;
    }
  switch (old_vote)
    {
    case LATE:  // can't happen.  Only if poll is closed.
    case WAS_ARCHIVE:  // only if DROPPING and was READ_ONLY
    case NOT_VALID:  // can't happen.  Only if guy is DROPPING
      clerklog << "old_vote is WAS_ARCHIVE or LATE or NOT_VALID????  Coming down.";
      exit(1);
      break;
    case NOT_READ:
      switch (vote)
	{
	case NOT_READ:
	  return NOT_READ;
	  break;
	case READ:
	  _readers++;
	case WAS_ARCHIVE:
	  break;
	default:
	  _readers++;
	  add_vote(vote);
	  break;
	}
      break;
    case READ:
      switch (vote)
	{	
	case NOT_READ:
	  _readers--;
	  break;
	case READ:
	  return READ;
	  break;
	default:
	  add_vote(vote);
	  break;
	}
      break;
    default:  // real vote before
      switch (vote)
	{
	case NOT_READ:
	  _readers--;		
	  break;
	case READ:
	  break;
	default:
	  break;
	}
      change_vote(old_vote, vote);
      break;
    }
  *(unsigned char*)((char*)p_ballot + _byte) = PACK_VOTE(vote);
  return old_vote;
}					
//  ******************************************************
void
TalliedItem::put_info(ITEM_INFO *here)
{
  Item::put_info(here);
  if (_kind == 'Y')
    here->eVote.type = TALLIEDY;
  else
    here->eVote.type = TALLIEDN;
  here->eVote.max = _max_vote;
  here->eVote.min = _min_vote;
  here->eVote.priv_type = (PRIV_TYPE)_priv_type;
}
//  ************************************************
//  returns the length of the string it wrote
long
TalliedItem::report_stat(ITEM_STAT *here, STAT_TYPE type, 
			 Ballot *p_ballot, short old_vote)
{
  short vote = UNKNOWN;
  char ave_str[8];
  char sum_str[8];
  unsigned len;
  strcpy(ave_str, "  *?*");
  strcpy(sum_str, "  *?*");
  init_stat(here);
  report_vote_str(here->vote_str, p_ballot);
  if (_vstatus != UNSEEN)
    {
      switch (_kind)
	{
	case 'Y':
	  sprintf(sum_str, " %4d", (int)_sum);
	  break;
	case '0':
	  sprintf(ave_str, " %4.1f", _ave);
	  break;
	case '9':
	  sprintf(ave_str, " %4.2f", _ave);
	  break;
	}
    }
  switch (type)
    {
    case LIST:
      if (_kind == 'Y')
	len = sprintf(here->text, FEN_TSTATY, EEN_TSTATY);
      else
	len = sprintf(here->text, FEN_TSTAT, EEN_TSTAT);
      break;
    case NORMAL:
      if (_kind == 'Y')
	len = sprintf(here->text, FEN_TSTATY, EEN_TSTATY);
      else
	len = sprintf(here->text, FEN_TSTAT, EEN_TSTAT);
      break;
    case WITH_VOTE:
      vote = report_vote(p_ballot);
      if (_kind == 'Y')
	len = sprintf(here->text, FEN_TSTATYV, EEN_TSTATYV);
      else
	len = sprintf(here->text, FEN_TSTATV, EEN_TSTATV);
      break;
    }
  if (len > sizeof(here->text))
    len += (sizeof(ITEM_STAT) - sizeof(here->text));
  else 
    len = sizeof(ITEM_STAT);
  return (long)len;
}
//  *********************************************
void
TalliedItem::report_vote_str(char* here, Ballot *p_ballot)
{
  short vote;
  YESorNO fixed = NO;
  if (p_ballot == NULL)
    {
      strcpy(here, "!ACC");
      return;
    }
  switch (vote = report_vote(p_ballot))
    {
    case LATE:
      strcpy(here, "-o-");
    case READ:
      strcpy(here,"ACC");
      break;
    case NOT_READ:
      strcpy(here,"!ACC");
      break;					
    case UNREAD_MIN:
      fixed = YES;
      vote = ((GroupedItem*)this)->fix(vote);
    default:
      if (_kind == 'Y')
	{
	  if (vote == 0)
	    strcpy(here,"NO");
	  else
	    strcpy(here,"YES");
	}
      else
	sprintf(here,"%d", vote);
    }
  if (fixed == YES)
    {
      int i = 0;
      while (here[++i] != '\0')
	;
      strcpy(&here[i+1],"*");
    }
}
// ************************************
void
TalliedItem::set_kind(void)
{
  if (_max_vote == 1 && _min_vote == 0)
    _kind = 'Y';
  else if (_max_vote <= 9)
    _kind = '9';
  else
    _kind = '0';
}
// ************************
//   store  writes itself out to the indicated ostream
OKorNOT
TalliedItem::store(ostream& strm)
{
  Item::store(strm);
  strm.write((char*)(&_ave), (char*)(&_voters) - (char*)(&_ave) +
	     sizeof(_voters));
  strm << ' ';
  return OK;
}
//   *****************************************
OKorNOT
TalliedItem::store_me(void)
{
  ostream& strm = (ostream&)_p_conf->open_info();
  if (!strm)
    return NOT_OK;
  strm.ostream::seekp(_offset);
  store(strm);
  _p_conf->close_info();
  return OK;
}
//  **********************************************************************
//  Places a list of uids that answer the questions 'here'.
//  Processes queries like who_voted =9 or <3 or >= 6
//  Places a list of uid's and votes at *here and returns the
//   number of chars written to here.  Or if the list
//  isn't appropriate, places a string and returns -1.
long
TalliedItem::who_voted(char *here, Conf* p_conf, char *question,
		       YESorNO *p_continue)
{
  typedef enum {NEQL, LESS, EQL, MORE, IF, ALL} HOW;
  HOW how = EQL;
  int number = NO_LIMIT;
  short target;
  int i = -1;
  long len = 0L;
  char no_one[85];
  short this_vote;
  char vote_str[15];
  unsigned long uid;
  YESorNO suppress_signers = NO;
  static Ballot *ballot;
  BallotBox& bbox= p_conf->ballot_box();
  no_one[0] = '\0';
  if ((PRIV_TYPE)_priv_type == PRIVATE)
    {
      sprintf(here,"Item #%d is PRIVATE.  You can not ask %s. Q!",
	      _dropping_id, question);
      return -1;
    }
  while (*question == 'W') 
    question++;
  // READ == 122, NOT_READ == 121
  if (strncmp(question, "ACC", 3) == 0 
      || strncmp(question, "=ACC", 4) == 0
      || strncmp(question, "==ACC", 5) == 0)
    {
      strcpy(no_one,"only accessed");
      target = READ;
      how = EQL;
    }
  else  
    if (strncmp(question, ">!ACC", 5) == 0  // hits if READ or VOTED
	|| strncmp(question, ">=ACC", 5) == 0
	|| strncmp(question, ">=!VOTE", 7) == 0)
      {
	strcpy(no_one,"accessed or voted on");
	target = NOT_READ;
	how = NEQL;
      }
    else if (strncmp(question, "!ACC", 4) == 0
	     || strncmp(question, "<ACC", 4) == 0
	     || strncmp(question, "=!ACC", 5) == 0
	     || strncmp(question, "==!ACC", 6) == 0
	     || strncmp(question, "<!VOTE", 6) == 0
	     || strncmp(question, "<=!ACC", 6) == 0)
      {
	strcpy(no_one,"not accessed");
	target = NOT_READ;
	how = EQL;
      }
    else if (strncmp(question, "VOTE", 4) == 0
	     || strncmp(question, ">ACC", 4) == 0
	     || strncmp(question, "=VOTE", 5) == 0
	     || strncmp(question, "==VOTE", 6) == 0
	     || strncmp(question, ">=VOTE", 6) == 0
	     || strncmp(question, ">!VOTE", 6) == 0)
      {
	strcpy(no_one, "voted on");
	how = LESS;
	target = _max_vote + 1;
	question[0] = '<';
      }
    else if (strncmp(question, "!VOTE", 5) == 0
	     || strncmp(question, "=!VOTE", 6) == 0
	     || strncmp(question,  "==!VOTE", 7) == 0
	     || strncmp(question, "<VOTE", 5) == 0 
	     || strncmp(question, "<=!VOTE", 7) == 0
	     || strncmp(question, "<=ACC", 5) == 0)
      {
	strcpy(no_one, "not voted on");
	how = LESS;
	target = _min_vote;
      }
    else if (strncmp(question, ">=!ACC", 6) == 0
	     || strncmp(question, "<=VOTE", 6) == 0)
      {
	how = ALL;
      }
    else if ((PRIV_TYPE)_priv_type == IF_VOTED)
      {
	sprintf(here,"\nItem #%d is an if-voted item.  "
		"You can ask who has voted by\n"
		"entering 'W Vote' at the eVote prompt "
		"but you cannot ask 'W %s'. Q!",
		_dropping_id, question);
	return -1;
      }
    else   // priv_type must equal PUBLIC, must be a numeric question 
      {
	// find the number
	while (question[++i] != '\0' && (question[i] < '0' 
					 || question[i] > '9' ))
	  ;
	if (((number = atoi(&question[i])) == 0 &&
	     (question[i] != '0' || question[i+1] != '\0'))
	    || number > NO_LIMIT || number < -NO_LIMIT)
	  {
	    sprintf(here,"Can't interpret the question: W %s. Q!", question);
	    return -1;
	  }
	target = (short)number;
	// adjust the number if <= or >=		
	strcpy(no_one,"voted ");
	switch (question[i=0])
	  {
	  case '<':
	    how = LESS;
	    strcat(no_one," <");
	    if (question[++i] == '=')
	      {
		strcat(no_one,"=");
		target += 1;
	      }
	    break;
	  case '>':
	    how = MORE;
	    strcat(no_one," >");
	    if (question[++i] == '=')
	      {
		strcat(no_one,"=");
		target -= 1;
	      }
	    break;
	  case '=':
	    strcat(no_one," =");
	    how = EQL;
	    break;
	  default:
	    if (question[i] <= '9' && question[i] >= '0')
	      {
		strcat(no_one," =");
		how = EQL;
	      }
	    else
	      {
		sprintf(here,"\nCan't interpret the question: W %s Q!", 
			question);
		return -1;
	      }
	    break;
	  }
	if (_kind == 'Y')
	  {
	    if (number == 1)
	      strcat(no_one," YES on");
	    else if (number == 0)
	      strcat(no_one," NO on");
	  }
	else
	  {
	    char numstr[10];
	    sprintf(numstr," %d on", number);
	    strcat(no_one, numstr);
	  }
      }
  if (*p_continue == NO)
    ballot = bbox.iterator(START);
  else
    *p_continue = NO;
  if (strncmp(p_conf->name(),"petition", 8) == 0)
    {
      if (strcmp(this->_title, "No of reservations") != 0)
	suppress_signers = YES;
    }
  do
    {
      if (ballot->action() & READ_ONLY)
	continue;
      if (suppress_signers && ballot->action() == SIGNER)
	continue;
      this_vote = report_fixed_vote(ballot);
      if (this_vote == NOT_VALID || this_vote == LATE 
	  || this_vote == WAS_ARCHIVE)
	continue;
      // these are the non-hits:
      if ((how != ALL) &&
	  ((how == MORE && this_vote <= target)
	   || (how == NEQL && this_vote == target)
	   || (how == LESS && this_vote >= target
	       && target != _min_vote)
	   || (how == LESS && target == _min_vote  // == !VOTE
	       && this_vote <= _max_vote)
	   || (how == EQL && this_vote != target)
	   || (how == IF && this_vote > _max_vote)))
	{
	  continue;
	}
      switch (this_vote)
	{
	case READ:
	  if ((target < UNKNOWN || target > NOT_REAL) && how == MORE)
	    continue;
	  strcpy(vote_str,"ACC");
	  break;
	case NOT_READ:
	  if ((target < UNKNOWN || target > NOT_REAL) && how == MORE)
	    continue;
	  strcpy(vote_str,"!ACC");
	  break;						
	default:
	  if (how == IF || (PRIV_TYPE)_priv_type == IF_VOTED)
	    strcpy(vote_str, "VOTED");
	  else
	    report_vote_str(vote_str, ballot);
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
  if (len == 0L)
    {
      sprintf(here,"\nNo one has %s item #%d.\n Q!", no_one, _dropping_id);
      return -1;
    }
  else
    {
      (void)sprintf(here + len, "%c",'\0');
      len += (long)strlen(here + len) + 1L; /* 1 for the \0 */
    }
  return len;
}

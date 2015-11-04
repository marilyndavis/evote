/* $Id: ballot.cc,v 1.3 2003/01/15 18:54:07 marilyndavis Exp $ */ 
/************************************************************
 *  ballot.cc Implements functions that deal with the ballot's
 *           header and initializing ballots.  Does not deal
 *           with votes.
 *********************************************************
 *    Copyright (c) 1994...2015 Deliberate.com Patented.
 *    by Marilyn Davis
 **********************************************************/
#include "evotedef.h"
extern "C" {
#include <time.h>
}
#include "ballot.h"	
//**************************************************
Header::Header(unsigned long uid):_uid(uid)
{
  time(&_mod_date);
  _action = (short)EVERYTHING;
}
//**************************************************
void
Header::start(unsigned long uid)
{
  _uid = uid;
  _mod_date = 0l;
  _action = (short)EVERYTHING;
  _filler = 0;
}
//**************************************************
Ballot::Ballot(unsigned long uid):_header(uid)
{
  short i;
  short len;
  len = RECLEN - sizeof (Header);
  for (i = 0; i < len; i++)
    {
      _vote[i] = NOT_READ;
    }
}
//**************************************************
OKorNOT 
Ballot::change_action(ACTION new_action)
{
  ACTION old_action = (ACTION)_header._action;
  if (new_action == old_action)
    return UNDECIDED;
  if (new_action == LOCAL)
    new_action = (ACTION)((short)old_action | (short)LOCAL);
  if (old_action & LOCAL)
    new_action = (ACTION)((short)new_action | (short)LOCAL);
  _header._action = (short)new_action;
  return OK;
}
// ***********************************************
//    initialized the already constructed ballot.
void 
Ballot::start(short ballot_size, unsigned long uid)
{
  unsigned short i;
  this->_header.start(uid);
  for (i = 0; i < (ballot_size - sizeof(Header)); i++)
    this->_vote[i] = (char)NOT_READ;
}

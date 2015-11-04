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

/* $Id: sitem.cc,v 1.3 2003/01/15 18:54:07 marilyndavis Exp $ */ 
// sitem.cpp - member functions for the TimeStampItem class which
//             are used for petitions.
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
// ************************************************************
inline TimeStampItem::TimeStampItem(Conf* p_conf, ITEM_INFO* p_app_item, 
				    streampos offset)
  :TalliedItem(p_conf, p_app_item, offset),
   content_bytes(1+sizeof(time_t)+ sizeof(streampos))
{
  _type = 'S'; 
  filler = 32 -(sizeof(Header) + content_bytes);
}
// ************************************************************
inline TimeStampItem::TimeStampItem(Conf* p_conf, istream & strm)
  :TalliedItem(p_conf, strm), content_bytes(1 + sizeof(time_t)
					    +sizeof(streampos))
{
  filler = 32 - (sizeof(Header) + content_bytes);
}
// ************************************************************
void TimeStampItem::blank(Ballot* p_ballot)
{
  char * start_at = (char*)p_ballot + _byte;
  int i;
  TalliedItem::blank(p_ballot);
  *(time_t*)(start_at + 1) = (time_t)0;
  *(streampos*)(start_at + 1 + sizeof(time_t))  = (streampos)0;
  for (i = 0, start_at += content_bytes; i < filler ; i++, start_at++)
    {
      *start_at = '\0';
    }
}
// ************************************************************
void
TimeStampItem::mark_bytes(streampos bytes[])
{
  int i;
  int filler = 7;  /* to make a 32 byte item + header */
  TalliedItem::mark_bytes(bytes);
  for (i = 1; i <= content_bytes + filler; i++)
    bytes[_byte + i]++;
}
// ************************************************************
void
TimeStampItem::find_spot(ItemList* p_tl)
{
  short dummy;
  TalliedItem::find_spot(p_tl);  
  dummy = p_tl->inc_next_byte(sizeof(time_t) + sizeof(streampos) + filler);
}
// ************************************************************
time_t
TimeStampItem::pull_time(Ballot *p_ballot, streampos * offset)
{
  short shift;
  shift = _byte + 1 + sizeof(time_t);
  *offset = *(streampos*)((char*)p_ballot + shift);
  return *(time_t*)((char*)p_ballot + _byte + 1);
}
// ************************************************************
time_t
TimeStampItem::push_time(Ballot *p_ballot, time_t new_time, 
			 streampos new_offset, streampos * old_offset)
{
  streampos old_off;
  time_t old_time;
  old_time = this->pull_time(p_ballot, &old_off);
  *(streampos*)((char*)p_ballot + _byte + 1 + sizeof(time_t)) = new_offset;
  *(time_t*)((char*)p_ballot + _byte + 1) = new_time;
  *old_offset = old_off;
  return old_time;
}
//  ******************************************************
void
TimeStampItem::put_info(ITEM_INFO *here)
{
  TalliedItem::put_info(here);
  here->eVote.type = TIMESTAMP;
}
// ************************************************************
// Returns the timestamp when uid voted or 0 if there's a problem.
time_t
TimeStampItem::send_stamp(Conf* p_conf, unsigned long uid,
			  unsigned long uid_asking)
{
  Ballot *ballot;
  long dummy_block;
  streampos offset;
  if ((ballot = p_conf->community().get_ballot(uid)) == NULL
      && (ballot = p_conf->ballot_box().find_ballot(uid, dummy_block)) 
      == NULL)
    {
      return 0L;
    }
  return pull_time(ballot, &offset);
}
//  **********************************************************************
//     Places a list of uids and time stamps at here.
long
TimeStampItem::who_signed(char *here, Conf* p_conf,
			  YESorNO *p_continue)
{
  long len = 0L;
  short this_vote;
  unsigned long uid;
  time_t time_stamp;
  static Ballot *ballot;
  BallotBox& bbox= p_conf->ballot_box();
  streampos offset;
  if (strncmp(p_conf->name(),"petition", 8) != 0)
    {
      sprintf(here,"%s is not a petition list.", p_conf->name());
      return -1;
    }
  if (*p_continue == NO)
    ballot = bbox.iterator(START);
  else
    *p_continue = NO;
  do
    {
      if (ballot->action() & READ_ONLY)
	continue;
      this_vote = report_fixed_vote(ballot);
      if (this_vote != 1)
	{
	  continue;
	}
      time_stamp = pull_time(ballot, &offset);
      uid = *(unsigned long*)ballot;
      if ((long)(len + sizeof(unsigned long) + 3 + sizeof(time_t))
	  > msgmax)
	{
	  *p_continue = YES;
	  break;
	}
      (void)sprintf(here+len, FEN_WHO_SIGNED, EEN_WHO_SIGNED);
      len += (long)strlen(here+len);
    }
  while ((ballot = bbox.iterator(NEXT)) != NULL);
  if (len == 0L)
    {
      sprintf(here,"\nNo one has signed item #%d.\n Q!", _dropping_id);
      return -1;
    }
  else
    {
      (void)sprintf(here + len, "%c",'\0');
      len += (long)strlen(here + len) + 1L; /* 1 for the \0 */
    }
  return len;
}

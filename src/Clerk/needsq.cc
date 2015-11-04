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

/* $Id: needsq.cc,v 1.4 2003/01/15 18:54:07 marilyndavis Exp $ */ 
// needsq.cc -- Implementation of Instructions that need an 
//              outgoing queue and don't have one.
/*********************************************************
 **********************************************************/
#include"evotedef.h"
#include"instruct.h"
#include"qlist.h"
#include"conf.h"
#include"voter.h"
#include"wholist.h"
extern WhoList wholist;
extern QList qlist;
extern time_t now;
#include <fstream.h>
#include <stdlib.h>
extern ofstream *logger;
extern ostream *dlogger;
extern int edebug;
extern time_t now;
char * uid_string(unsigned long uid);  /* debug thing */
// ***********************************************************
EnterAdmin::EnterAdmin(char* input, int pid, ITYPE itype):
  NeedsQ(input, pid, itype)
{
  unsigned long no_of_participants;
  int _memid;
  char * get_name(unsigned long uid);
  unsigned long for_debugger;
  for_debugger = _uid;
  YESorNO x = NO;
  if (_status == NOT_OK)
    {
      qlist.send(DEL_NO_CONF, _qid, 1);
      qlist.drop_q(_p_outq, x, NO);
#ifdef EDEBUG
      if (edebug & QUEUES)
	dlog << "Asked " << uid_string(_uid) << " to drop " << _qid 
	     << " : pid = " << pid << ", for " << get_itype(_itype);
#endif
      return;
    }
  _memid = _p_conf->memid();
  no_of_participants = _p_conf->no_of_participants();
  sprintf(qlist.output, FEN_ENTER_ADMIN, EEN_ENTER_ADMIN);
  qlist.send(MQ_OK, _qid);
  _p_conf->protect();
  _p_voter = new Voter(_p_conf, 0, pid, _qid, NULL, 0L, 0L);
  _p_outq->attach_voter(_p_voter);
}
// ***********************************************************
Entering::Entering(char* input, int pid, ITYPE itype)
  :NeedsQ(input, pid, itype)
{
  int drop_days;
  unsigned long no_of_participants;
  int _memid;
  char * get_name(unsigned long uid);
  unsigned long for_debugger;
  for_debugger = _uid;
  int trouble = 0;
  YESorNO x = NO;
  YESorNO& no = x;
  int action;
  if (_status == NOT_OK)
    {
      qlist.send(DEL_NO_CONF, _qid, 1);
      qlist.drop_q(_p_outq, x, NO);
#ifdef EDEBUG
      if (edebug & QUEUES)
	dlog << "Asked " << uid_string(_uid) << " to drop " << _qid 
	     << " : pid = " << pid << ", for " << get_itype(_itype);
#endif
      return;
    }
  _memid = _p_conf->memid();
  no_of_participants = _p_conf->no_of_participants();
  sscanf(input, FNE_ENTERING, NEE_ENTERING);
  _p_voter = _p_conf->community().find(_voter, _pid, _qid, NO, &trouble);
  if (_p_voter != NULL)
    {
      action = (int)_p_voter->p_ballot()->action();
      /* This is temporary and just needed for old data the
	 first time it is read by eVote 2.5 -- Fall, 2000 */
      if ((strncmp(_p_conf->name(), "petition", 8) != 0)
	  && action == SIGNER)
	_p_voter->p_ballot()->change_action(VACATION);
      if (action & DROPPING)
	{
	  /*	  if (action & LOCAL)
		  {  
		  _p_conf->community().forget(_p_voter);
		  _p_voter = NULL;
		  }
		  else // is mail   */
	  _p_voter->change_action(EVERYTHING); 
	}
    }
  if (_p_voter == NULL)
    {
      if (trouble > 0)  // means on twice and pid not dead
	{
	  sprintf(qlist.output, FEN_ON_TWICE, EEN_ON_TWICE);
	  qlist.send(DEL_ON_TWICE, _qid);
	  qlist.drop_q(_p_outq, no, NO);
	  return;
	}
      drop_days = _p_conf->drop_day();
      sprintf(qlist.output, FEN_NEW_VOTER, EEN_NEW_VOTER);
#ifdef EDEBUG
      if (edebug & QUEUES)
	dlog << "Asked " << uid_string(_uid) << " to drop " << _qid 
	     << " : pid = " << pid << ", for " << get_itype(_itype);
#endif
      qlist.send(DEL_NEW_VOTER, _qid);
      qlist.drop_q(_p_outq, no, NO);
      return;
    }
  sprintf(qlist.output, FEN_ENTERING, EEN_ENTERING);
  qlist.send(MQ_OK, _qid);
#ifdef EDEBUG
  if (edebug & VOTERS)
    dlog << get_name(_uid) << ":" << _uid << " entering " 
	 << _conf_name << ".";
#endif
  _p_outq->attach_voter(_p_voter);
}
// ***********************************************************
Joining::Joining(char* input, int pid, ITYPE itype)
:NeedsQ(input, pid, itype)
{
  int _memid;
  unsigned long no_of_participants;
  int trouble = 0;
  YESorNO local;
  /*  char * px;
      px = (char*)new char[1000];
      delete[] (char*)px; */
  if (_status == NOT_OK)
    {
      qlist.send(NO_CONF, _qid, 1);
      return;
    }
  sscanf(input, FNE_JOINING, NEE_JOINING);
  if ((_p_voter = _p_conf->community().find(_voter, _pid, _qid, 
					    YES, &trouble))
      == NULL)
    {
      clerklog << "Joining received a null voter from find on "
	       << _voter << " in "
	       << _p_conf->name();
      if (trouble > 0)  // means on twice and pid not dead
	{
	  sprintf(qlist.output, FEN_ON_TWICE, EEN_ON_TWICE);
	  qlist.send(DEL_ON_TWICE, _qid);
	  YESorNO x = NO;
	  YESorNO& no = x;
	  qlist.drop_q(_p_outq, no, NO);
	  return;
	}
      exit(1);
    }
  _memid = _p_conf->memid();
  no_of_participants = _p_conf->no_of_participants();
  sprintf(qlist.output, FEN_NEW_MID, EEN_NEW_MID);
  if (local == YES)
    _p_voter->change_action(LOCAL);
  else
    wholist.joins(_voter);
  qlist.send(MQ_OK, _qid);
  _p_outq->attach_voter(_p_voter);				
}

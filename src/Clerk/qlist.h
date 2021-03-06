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

/* $Id: qlist.h,v 1.3 2003/01/15 18:54:07 marilyndavis Exp $ */ 
// qlist.h -- Defines QList class for maintaining the active outgoing
//            message queues
/*********************************************************
 **********************************************************/
#ifndef QLISTH
#define QLISTH
#include"evotedef.h"
#include"outq.h"
class Voter;

class QList
{
 public:
  OKorNOT drop_q(int qid, YESorNO & user_dropped, YESorNO force);
  OKorNOT drop_q(OutQ *&, YESorNO & user_dropped, YESorNO force);
 
QList(void):_first(NULL), _active_qs(0L){}
  void put_msg(char* msg);  /* voterlist uses this for send_all */
  void start(int msgmax);
  OKorNOT drop_oldqs();
  int drop_readies();
  OutQ * find(int qid);
  OutQ * match(int pid);
  OutQ* newq(int pid, unsigned long uid, Voter* p_voter= NULL);
  OKorNOT send(long priority);
  OKorNOT send(long priority, int qid, int len = 0);
  char *output;
#ifdef EDEBUG
  OKorNOT check();
#endif
 private:
  OutQ* _first;
  int _active_qs;		
  struct msgbuf* _sysmsg;
  OKorNOT attach(OutQ*);  // ()Inst:NeedsQ
  OKorNOT detach(int qid);
  OKorNOT drop_qs();
  OKorNOT send_msg(long priority, int qid, int len);
  friend class OutQ;
  friend class InQ;
};		
inline OKorNOT QList::drop_q(int qid, YESorNO & user_dropped, YESorNO force)
{OutQ* p_q = find(qid); return p_q ? drop_q(p_q, user_dropped, force): OK;}
inline void QList::put_msg(char* msg){strcpy(_sysmsg->mtext, msg);}
inline void QList::start(int msgmax){_sysmsg = (struct msgbuf *)new char[(sizeof(long) + msgmax)]; output = _sysmsg->mtext;}
#endif

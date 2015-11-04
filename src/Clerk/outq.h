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

/* $Id: outq.h,v 1.3 2003/01/15 18:54:07 marilyndavis Exp $ */ 
// outq.h -- defines the OutQ class which keeps the message queue
//           for the user.
/*********************************************************
 **********************************************************/
#ifndef OUTQH
#define OUTQH
#include"evotedef.h"
#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include "../Clerklib/eVote_defaults.h"
#include "voter.h"
class OutQ
{
 public:
  OutQ(int pid, unsigned long uid, Voter* voter); 
  int qid(void){return _qid;}
  key_t pid(void){return _pid;}
  Voter *p_voter(void){return _p_voter;}
  unsigned long uid(void){return _uid;}
  void attach_voter(Voter *_p_voter);
  char *conf_name(void);
 private:
  long _last_use;
  key_t _pid;
  unsigned long _uid;
  Voter* _p_voter;
  OutQ* _next;
  YESorNO ready_to_drop;
  int _qid;
  char _conf_name[CONFLEN + 1];
  OKorNOT drop(YESorNO & user_dropped, YESorNO force = NO);
  friend class QList;
};
inline void OutQ::attach_voter(Voter *p_voter){_p_voter = p_voter; strcpy(_conf_name, p_voter->p_conf()->name());};
inline char * OutQ::conf_name(void){return _conf_name;};
#endif

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

/* $Id: inq.h,v 1.3 2003/01/15 18:54:07 marilyndavis Exp $ */ 
// inq.h -- header for the InQ class which manages the input
//          message queue
/*********************************************************
 **********************************************************/
#ifndef INQUEUEHPP
#define INQUEUEHPP
#include "../Clerklib/eVote_defaults.h"
#include<sys/types.h>
#include<unistd.h>
class InQ
{
 public:
  ~InQ(void);
  char *buffer(void){return _sysmsg->mtext;} 
  OKorNOT deliver (YESorNO wait, char * key_str);
  OKorNOT send_myself(char* msg, long priority);
  OKorNOT start(char *key_str);
 private:
  key_t clerk_key;
  struct msgbuf* _sysmsg;
  int _msgqid;
  friend class QList;
};
#endif

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

/* $Id: msgbuf.h,v 1.3 2003/01/15 18:54:07 marilyndavis Exp $ */ 
// msgbuf.h -- defines the msgbuf for the message queues
/*********************************************************
 **********************************************************/
#ifndef _msgbuf_h
#define _msgbuf_h
struct msgbuf
{
  long mtype;
  char mtext[MSGMAX];
};
#endif

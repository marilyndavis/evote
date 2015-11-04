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

/* $Id: ipc_msg.h,v 1.4 2003/01/15 18:54:08 marilyndavis Exp $ */ 
/**********************************************************
 *	ipc_msg.h
 *   All functions using the message queues need this.
 *   The definitions are in ipc_msg.c
 *   DON'T ALTER EVEN ONE  char in this file!!!!!!
 **********************************************************/
#ifndef ipc_msg_h
#define ipc_msg_h
#include "eVote_defaults.h"
OKorNOT send_inst(int len, YESorNO grab_return);
int get_msg(YESorNO wait); 
/*  These are the IPC buffers */
#define MSG_LEN 2048  /* this length gets replaced at runtime
			 with msglen + sizeof(long) 
			 defined independantly in ipc_msg.c */
/*  These pointers point to buffers that are sizeof(long) + msgmax
    long */
extern struct msgbuf *in_msg_buffer;
extern struct msgbuf *out_msg_buffer;
extern int extra_space;
#endif

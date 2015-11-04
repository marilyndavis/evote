/* $Id: ipc_msg.h,v 1.4 2003/01/15 18:54:08 marilyndavis Exp $ */ 
/**********************************************************
 *	ipc_msg.h
 *   All functions using the message queues need this.
 *   The definitions are in ipc_msg.c
 *   DON'T ALTER EVEN ONE  char in this file!!!!!!
 **********************************************************/ 
/*********************************************************
 *    Copyright (c) 1994...2015 Deliberate.com	Patented.
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the eVote(R)/Clerk License as
 *  published by Deliberate.Com.
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  eVote(R)/Clerk License for more details.
 *  You should have received a copy of the eVote(R)/Clerk License
 *  along with this program in Chapter VII of ../../doc/eVote.doc. 
 *  If not, write to Deliberate.Com 2555 W. Middlefield Rd., #150
 *  Mountain View, CA 94043 USA or office@deliberate.com.
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

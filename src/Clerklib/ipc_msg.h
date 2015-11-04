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

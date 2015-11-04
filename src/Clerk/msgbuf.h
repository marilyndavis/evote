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

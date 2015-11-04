/* $Id: inq.h,v 1.3 2003/01/15 18:54:07 marilyndavis Exp $ */ 
// inq.h -- header for the InQ class which manages the input
//          message queue
/*********************************************************
 *    Copyright (c) 1994...2015 Deliberate.com Patented.
 *    by Marilyn Davis
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

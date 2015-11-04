/* $Id: maybeq.cc,v 1.3 2003/01/15 18:54:07 marilyndavis Exp $ */ 
// maybeq.cc  -- instructions for users that may or may not already
//               have a return queue.  Just WhoNum and WhoIs
/*********************************************************
 **********************************************************/
#include"evotedef.h"
#include"instruct.h"
#include"qlist.h"
extern QList qlist;
#include"wholist.h"
extern WhoList wholist;
extern int edebug;
extern ostream *dlogger;
char * uid_string(unsigned long uid); // util.cc
// *************************************************************
//  Looks for someone in the WhoList: address --> id
WhoNum::WhoNum(char* input, int pid, ITYPE itype):
MaybeQ(input, pid, itype)
{
  char name[WHO_LEN];  /* WHO_LEN is in msgdef.h */
  YESorNO add;
  unsigned long id;
  sscanf(input, FNE_WHO_NUM, NEE_WHO_NUM);
  id = wholist.whonum(name, add);
  sprintf(qlist.output, FEN_WHO_NUM, EEN_WHO_NUM);
  if (_p_voter == NULL)   // Then it's like a RespondOnce
    qlist.send(DEL_GOOD, _qid);
  else
    qlist.send(GOOD, _qid);  // Then it's like a HasQ
}
// *************************************************************
//  Returns the name string associated with the
//  incoming id.
WhoIs::WhoIs(char* input, int pid, ITYPE itype):
MaybeQ(input, pid, itype)
{
#ifdef EDEBUG
  if (edebug & QUEUES)
    dlog << "Asked " << uid_string(_uid) << " to drop " << _qid 
      << " : pid = " << pid << ", for " << get_itype(_itype);
#endif
  sscanf(input, FNE_WHO_IS, NEE_WHO_IS);
  if (wholist.whois(qlist.output, _voter) == OK)
    {
      if (_p_voter == NULL)
	qlist.send(DEL_GOOD, _qid, strlen(qlist.output) + 1);
      else
	qlist.send(GOOD, _qid, strlen(qlist.output) + 1);
    }
  else
    {
      if (_p_voter == NULL)
	qlist.send(DEL_NO_VOTER, _qid, 1);
      else
	qlist.send(NO_VOTER, _qid, 1);
    }
}

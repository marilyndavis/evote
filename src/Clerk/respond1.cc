/* $Id: respond1.cc,v 1.3 2003/01/15 18:54:07 marilyndavis Exp $ */ 
// respond1.cc -- Implements instructions that only get one reponse
//                and then the queue goes away.
/*********************************************************
 *    Copyright (c) 1994...2015 Deliberate.com Patented.
 *    by Marilyn Davis
 **********************************************************/
#include"evotedef.h"
#include"instruct.h"
#include"qlist.h"
#include"conflist.h"
#include"conf.h"
#include"wholist.h"
extern WhoList wholist;
extern QList qlist;
extern ConfList conferences;
#include <fstream.h>
extern ofstream *logger;
extern ostream *dlogger;
extern int edebug;
char * uid_string(unsigned long uid);
extern time_t now;
//  *********************************************
//    takes the conf on-line but so that it will be picked up again
//    the next time someone enters it.
AdjournConf::AdjournConf(char *input, int pid, ITYPE itype) 
  :RespondOnce(input, pid, itype)
{
  if (_status != OK)
    return;	
  //		sscanf(input, FNE_ADJOURN_CONF, NEE_ADJOURN_CONF);  - read by about conf
#ifdef EDEBUG
  if (edebug & QUEUES)
    dlog << "Asked " << uid_string(_uid) << " to drop " << _qid 
	 << " : pid = " << pid << ", for itype = get_itype(_itype)" <<
      " for voter " << _voter;
#endif
  if (conferences.adjourn(_p_conf, _uid) == OK)
    {
      qlist.send(DEL_GOOD, _qid);
    }
  else
    qlist.send(DEL_NOT_GOOD, _qid);
}
// ***********************************************************
CheckConf::CheckConf(char *input, int pid, ITYPE itype) 
  :RespondOnce(input, pid, itype)
{
  OKorNOT all_ok = OK;
  if (_status != OK)
    return;
  clerklog << "Checking all statistics on " << _p_conf->name() 
	   << " at " << uid_string(_uid) << "'s request.";
  if (_p_conf->check_order() != OK)
    all_ok = NOT_OK;
  if (_p_conf->check_drops() > 0)
    all_ok = NOT_OK;
  if (_p_conf->check_stats() != OK)
    all_ok = NOT_OK;
#ifdef EDEBUG
  if (edebug & QUEUES)
    dlog << "Asked " << uid_string(_uid) << " to drop " << _qid 
	 << " : pid = " << pid << ", for " << get_itype(_itype);
#endif
  if (all_ok == OK)
    qlist.send(DEL_GOOD, _qid); 
  else
    qlist.send(DEL_NOT_GOOD, _qid);
}
// ***********************************************************
CreateConf::CreateConf(char *input, int pid, ITYPE itype) 
  :RespondOnce(input, pid, itype)
{
  short drop_day;
  if (_status != OK)
    {
#ifdef EDEBUG
      if (edebug & QUEUES)
	dlog << "Asked " << uid_string(_uid) << " to drop " << _qid 
	     << " : pid = " << pid << ", for " << get_itype(_itype);
#endif
      qlist.send(DEL_REDUNDANT, _qid, 1);
      return;
    }
  sscanf(input, FNE_CREATE_CONF, NEE_CREATE_CONF);
#ifdef EDEBUG
  if (edebug & QUEUES)
    dlog << "Asked " << uid_string(_uid) << " to drop " << _qid 
    	 << " : pid = " << pid << ", for " << get_itype(_itype);
#endif
  if (conferences.create(_conf_name, drop_day) == NULL)
    {
      qlist.send(DEL_NOT_GOOD, _qid, 1);
      return;
    }
  qlist.send(DEL_DONE, _qid, 1);
  clerklog << _conf_name << " created by uid " << uid_string(_uid);
}
// ***********************************************************
DropConf::DropConf(char* input, int pid, ITYPE itype) 
  :RespondOnce(input, pid, itype)
{
  if (_status == NOT_OK)
    return;
#ifdef EDEBUG
  if (edebug & QUEUES)
    dlog << "Asked " << uid_string(_uid) << " to drop " << _qid 
	 << " : pid = " << pid << ", for " << get_itype(_itype);
#endif
  if (conferences.drop(_p_conf, _uid) == OK)
    qlist.send(DEL_GOOD, _qid);
  else
    qlist.send(DEL_NOT_GOOD, _qid);
}
// ***********************************************************
DropVoter::DropVoter(char *input, int pid, ITYPE itype) 
  :RespondOnce(input, pid, itype)
{
  unsigned long uid_to_drop; 
  YESorNO only_if_non_voter;
  int cc;
  YESorNO all = NO;
  ConfIter lister;
  if (_status != OK)
    return;
  sscanf(input, FNE_DROP_VOTER, NEE_DROP_VOTER);
  if (_p_conf == NULL)
    {
      all = YES;
    }
  while (1)
    {
      if (all)   // protecting from an adjourn_some
	{
	  if ((_p_conf = lister()) == NULL)
	    break;
	  _p_conf->protect();
	}
      /* only_if_non_voter == YES is for a petition list.  When a signer removes
	 her signature, we want to drop her from the data if she hasn't
	 signed anything else */
      switch ((cc = _p_conf->drop_voter(uid_to_drop, _uid, only_if_non_voter)))
	{
	case PROBLEM:
	  // Was locked.  Now it's not.
	case OK:
	case CANT:
	  // succeeded
	  if (all == NO)
	    {
	      qlist.send(DEL_GOOD, _qid);
#ifdef EDEBUG
	      if (edebug & QUEUES)
		dlog << "Asked " << uid_string(_uid) << " to drop " << _qid 
		     << " : pid = " << pid << ", for " << get_itype(_itype);
#endif
	      return;
	    }
	  break;
	case NOT_OK:
	  // No voter to drop
	  if (all == NO)
	    {
#ifdef EDEBUG
	      if (edebug & QUEUES)
		dlog << "Asked " << uid_string(_uid) << " to drop " << _qid 
		     << " : pid = " << pid << ", for " << get_itype(_itype);
#endif
	      qlist.send(DEL_NO_VOTER, _qid);
	      return;
	    }
	  break;
	case STOP:
	  // failed, still online
#ifdef EDEBUG
	  if (edebug & QUEUES)
	    dlog << "Asked " << uid_string(_uid) << " to drop " << _qid 
		 << " : pid = " << pid << ", for " << get_itype(_itype);
#endif
	  if (all)
	    {
	      _p_conf->expose();
	    }
	  qlist.send(DEL_ON_LINE, _qid);
	  return;
	case UNDECIDED:
	  // failed, drop_day won't allow it
#ifdef EDEBUG
	  if (edebug & QUEUES)
	    dlog << "Asked " << uid_string(_uid) << " to drop " << _qid 
		 << " : pid = " << pid << ", for " << get_itype(_itype);
#endif
	  if (all)
	    {
	      conferences.expose(_p_conf);
	    }
	  qlist.send(DEL_NOT_GOOD, _qid);
	  return;
	default:
	  clerklog << "Impossible return " << cc << " in DropVoter.";
	  break;
	}
      if (all)
	{
	  _p_conf->expose();
	}
    }
  qlist.send(DEL_GOOD, _qid);
}
// ***********************************************************
Exist::Exist(char *input, int pid, ITYPE itype) 
  :RespondOnce(input, pid, itype)
{
  if (_status == NOT_OK)
    return;   // already sent DEL_NO_CONF in RespondOnce
  else
    {
#ifdef EDEBUG
      if (edebug & QUEUES)
	dlog << "Asked " << uid_string(_uid) << " to drop " << _qid 
	     << " : pid = " << pid << ", for " << get_itype(_itype);
#endif
      qlist.send(DEL_GOOD, _qid);
    }
}
//  *********************************************************
Hello::Hello(char *input, int pid, ITYPE itype) 
  :RespondOnce(input, pid, itype)
{
  qlist.send(DEL_GOOD, _qid, 1);
}
// *************************************************************
//  Forces new ideas of the items in the conf.
SyncConf::SyncConf(char* input, int pid, ITYPE itype, int len) 
  :RespondOnce(input, pid, itype)
{
  int sends;
  if (_status != OK)
    return;
#ifdef EDEBUG
  if (edebug & QUEUES)
    dlog << "Asked " << uid_string(_uid) << " to drop " << _qid 
	 << " : pid = " << pid << ", for " << get_itype(_itype);
#endif
  sscanf(input, FNE_SYNC_CONF, NEE_SYNC_CONF);
  _p_conf->protect();
  switch (_p_conf->sync_conf(input, len, _uid, sends))
    {
    case OK:
      qlist.send(DEL_GOOD, _qid, 1);
      _p_conf->expose();
      break;
    case NOT_OK:
      _p_conf->expose();
      conferences.burn(_p_conf);
      qlist.send(DEL_NOT_GOOD, _qid, 1);
      break;
    default:
      break;
    }
}
// *************************************************************
// Drops an unsigned int id from the WhoList.
WhoDrop::WhoDrop(char* input, int pid, ITYPE itype) 
  :RespondOnce(input, pid, itype)
{
  unsigned long id;
  YESorNO force;
#ifdef EDEBUG
  if (edebug & QUEUES)
    dlog << "Asked " << uid_string(_uid) << " to drop " << _qid 
	 << " : pid = " << pid << ", for " << get_itype(_itype);
#endif
  sscanf(input, FNE_WHO_DROP, NEE_WHO_DROP);
  switch (wholist.drop(id, force))
    {
    case OK:  // actually dropped the voter
      qlist.send(DEL_DONE, _qid, 1);
      break;
    case PROBLEM: // Discrepancy in count
      qlist.send(DEL_NOT_GOOD, _qid, 1);
      break;
    case UNDECIDED:  // decremented count
      qlist.send(DEL_GOOD, _qid, 1);
      break;
    case NO_VOTER: // couldn't find the uid
      qlist.send(DEL_NO_VOTER, _qid, 1);
      break;
    default:
      break;
    }
}
// *************************************************************
// Returns a list of uids that are in the conf.
// It could take several messages to send back the whole list.
WhosIn::WhosIn(char* input, int pid, ITYPE itype) 
  :RespondOnce(input, pid, itype)
{
  int len;
  YESorNO cont = NO;
  if (_status != OK)
    return;  // already sent DEL_NO_CONF
  do
    {
      len = (int)_p_conf->whos_in(qlist.output, _p_conf, &cont);
      if (len == -1)
	{
	  qlist.send(DEL_STRING_OUT, _qid, 0);
	}
      else     
	if (cont == YES)
	  {
	    qlist.send(UID_LIST_MORE, _qid, len);
	  }
	else
	  {
	    qlist.send(DEL_UID_LIST, _qid, len);
	  }
    }
  while (cont == YES);
}
// *************************************************************
// Drops an unsigned int id from the WhoList.
WhoSync::WhoSync(char* input, int pid, ITYPE itype) 
  :RespondOnce(input, pid, itype)
{
#ifdef EDEBUG
  if (edebug & QUEUES)
    dlog << "Asked " << uid_string(_uid) << " to drop " << _qid 
	 << " : pid = " << pid << ", for " << get_itype(_itype);
#endif
  sscanf(input, FNE_WHO_SYNC, NEE_WHO_SYNC);
  switch (wholist.sync())
    {
    case UNDECIDED:
      qlist.send(DEL_ON_LINE, _qid, 1);
      break;
    case PROBLEM:
      qlist.send(DEL_NOT_GOOD, _qid, 1);
      break;
      /*    case NOT_OK:
	    qlist.send(DEL_DONE, _qid, 1);
	    break; */
    case OK:
      qlist.send(DEL_GOOD, _qid, 1);
      break;
    default:
      clerklog << "Impossible case in whosync.";
      break;
    }
}

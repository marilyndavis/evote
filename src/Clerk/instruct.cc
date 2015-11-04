/* $Id: instruct.cc,v 1.3 2003/01/15 18:54:07 marilyndavis Exp $ */ 
// instruct.cc has functions for the abstract classes in the
// instruction hierarchy: Instruction, NeedsQ, HasQ, MaybeQ, 
// and RespondOnce
/*********************************************************
 *    Copyright (c) 1994...2015 Deliberate.com Patented.
 *    by Marilyn Davis
 **********************************************************/
#include<stdio.h>
#include "evotedef.h"
#include "instruct.h"
#include "qlist.h"
#include "conflist.h"
#include "conf.h"
#include "voter.h"
extern ConfList conferences;
extern QList qlist;
#ifdef EDEBUG
#include"debug.h"
#endif
#include <fstream.h>
extern time_t now;
extern ofstream * logger;
extern ostream * dlogger;
/*************************************************************
 *    Base class for all instructions.
 *************************************************************/
Instruction::Instruction(char * input, int pid, ITYPE itype):
_pid(pid), _status(OK), _itype(itype), _p_voter(NULL),
_fetched(NO)
{
  time(&_now);
  strcpy(qlist.output, "  Q!");  /* default message to send */
#ifdef EDEBUG
  if (edebug & FLOWER && edebug & QUEUES)
    {
      sprintf(debugger,
	      "echo %u:Clerk received a %s from pid %d.", 
	      999, get_itype(_itype), pid);
      doutput(debugger);
    }
#endif		  
  switch (itype)
    {
      /* NoQ's */
    case GROW_CONF:
    case REORDER_CONF:
      /* Respond Once's - inherits from NeedsQ */
    case ADJOURN_CONF:
    case CHECK_CONF:
    case CREATE_CONF:
    case DROP_CONF:
    case DROP_VOTER:
    case EXIST:
    case SYNC_CONF:
    case WHOS_IN:
      /* NeedsQ's */
    case ENTERING:
    case JOINING:
    case ENTER_ADMIN:
      /* All above are about conferences */
      sscanf(input, FNE_ABOUT_CONF, NEE_ABOUT_CONF);
      if (strlen(_conf_name) > CONFLEN)
	_conf_name[CONFLEN] = '\0';
#ifdef EDEBUG
  if (edebug & MESSAGES)
    {
      sprintf(debugger,
	      "echo %lu: Conf = %s found for that %s on %d",
	      _uid, _conf_name, get_itype(_itype), pid);
      doutput(debugger);
    }
#endif
      if (strcmp(_conf_name, "ALL") == 0 && _itype == DROP_VOTER)
	{  // This is for DropVoter
	  _p_conf = NULL;
	  return;
	}
      if (strlen(_conf_name) > MAX_FILE_NAME_LEN)
	_conf_name[MAX_FILE_NAME_LEN] = '\0';
      _p_conf = conferences.find(_conf_name);
      if (_p_conf == NULL)
	{
	  _p_conf = conferences.fetch(_conf_name);
	  if (_p_conf != NULL)
	    _fetched = YES;
	}
      if (_p_conf == NULL && _itype == CREATE_CONF)
	return;
      else
	if (_itype == CREATE_CONF)
	  {
	    _status = NOT_OK;
	    return;
	  }
      if (_p_conf == NULL)
	{
	  _status = NOT_OK;
	}
      break;
    default:
      break;
    }
#ifdef EDEBUG
  if (edebug & QUEUES && edebug & MESSAGES)
    {
      qlist.check();
    }
#endif
}
Instruction::~Instruction(void)
{
}
// ***********************************************************
//  NeedsQ -- all instructions that need a new out queue
//            pass through here.
// ***********************************************************
NeedsQ::NeedsQ(char * input, int pid, ITYPE itype):
 
Instruction(input, pid, itype)
{
#ifdef EDEBUG
  if (edebug & FLOWER && edebug & QUEUES)
    {
      sprintf(debugger,
	      "echo %lu:NeedsQ called for that %s on %d",
	      _uid, get_itype(_itype), pid);
      doutput(debugger);
    }
#endif		  
  sscanf(input, FNE_NEEDSQ, NEE_NEEDSQ);
  _p_outq = qlist.newq(pid, _uid);
  _qid = _p_outq->qid();
#ifdef EDEBUG
  if (edebug & FLOWER && edebug & QUEUES)
    {
      sprintf(debugger,
	      "echo %lu:NeedsQ got qid %d for that %s on %d",
	      _uid, _qid, get_itype(_itype), pid);
      doutput(debugger);
    }
#endif		  
}
// ***********************************************************
//  HasQ -- instructions that should already have an out-
//          queue to the user pass through here and look
//          for their queue.
// ***********************************************************
HasQ::HasQ(char * input, int pid, ITYPE itype)
  :Instruction(input, pid, itype)
{	
  _voter = 0;
#ifdef EDEBUG
  if (edebug & FLOWER && edebug & QUEUES)
    {
      sprintf(debugger,
	      "echo ????:HasQ called for that %s on %d",
	      get_itype(_itype), pid);
      doutput(debugger);
    }
#endif		  
  if ((_p_outq = qlist.match(pid)) == NULL)
    {
      YESorNO x = NO;
      YESorNO &no = x;
      clerklog << "Can't find queue for a HasQ on pid = " << pid
	<< " while trying to " << get_itype(itype);
#ifdef EDEBUG
      if (edebug & FLOWER && edebug & QUEUES)
	{
	  sprintf(debugger,
		  "echo %lu:HasQ couldn't find queue for that %s on %d",
		  _uid, get_itype(_itype), pid);
	  doutput(debugger);
	}
      if (edebug & QUEUES)
	{
	  dlog << "Can't find queue for a HasQ on pid = " << pid
	    << ", for itype = " << get_itype(_itype);
	}
#endif
      _p_outq = qlist.newq(pid, _uid);
      _qid = _p_outq->qid();
      qlist.send(PROGRAMMER_ERROR, _qid);
      qlist.drop_q(_p_outq, no, NO);
      _status = NOT_OK;
    }
  else
    {
      _p_voter = _p_outq->p_voter();
      if (_p_voter == NULL)
	{
#ifdef EDEBUG
	  if (edebug & FLOWER && edebug & QUEUES)
	    {
	      sprintf(debugger,
		      "echo %lu:No voter attached to queue %d for that %s on %d",
		      _uid,  _p_outq->qid(), get_itype(_itype), pid);
	      doutput(debugger);
	    }
#endif
	  clerklog << "No voter attached to queue on pid = " << pid
		   << " during " << get_itype(itype) 
		   << "\n   Perhaps voter hasn't joined?";
	  _status = NOT_OK;
	}
      else
	{
	  _voter = _p_voter->uid();
	  _p_conf = _p_voter->p_conf();
	  _qid = _p_outq->qid();
	  _uid = _p_outq->uid();
	}
#ifdef EDEBUG
  if (edebug & FLOWER && edebug & QUEUES)
    {
      sprintf(debugger,
	      "echo %lu:HasQ found qid %d for that %s on %d and voter %lu",
	      _uid, _qid, get_itype(_itype), pid, _voter);
      doutput(debugger);
    }
#endif		  
    }
}
// ***********************************************************
//   MaybeQ is only used by WhoIs and WhoNum
//   If these calls are made by a user who is already
//   online and attached to a conf, then there is
//   a queue already and we want to use it.
//   If not, we want this to be just like a 
//   RespondOnce
// ***********************************************************
MaybeQ::MaybeQ(char *input, int pid, ITYPE itype):
Instruction(input, pid, itype)
{
#ifdef EDEBUG
  if (edebug & FLOWER && edebug & QUEUES)
    {
      sprintf(debugger,
	      "echo %lu:MaybeQ called for that %s on %d",
	      _uid, get_itype(_itype), pid);
      doutput(debugger);
    }
#endif		  
  if ((_p_outq = qlist.match(pid)) == NULL
    || (_p_voter = _p_outq->p_voter()) == NULL)
    {
      sscanf(input, FNE_NEEDSQ, NEE_NEEDSQ);
      _p_outq = qlist.newq(pid, _uid);
      _qid = _p_outq->qid();
      tmpq = YES;
    }
  else
    {
      _p_voter = _p_outq->p_voter();
      _voter = _p_voter->uid();
      _qid = _p_outq->qid();
      _uid = _p_outq->uid();
      tmpq = NO;
    }
}
MaybeQ::~MaybeQ(void)
{		
  YESorNO x = NO;
  YESorNO &no = x;
  if (tmpq)
    qlist.drop_q(_p_outq, no, NO);
}
/*************************************************************
 *  RespondOnce is for calls that only need one response
 *     so the out queue is discarded after the message is
 *     sent.
 *************************************************************/
RespondOnce::RespondOnce(char *input, int pid, ITYPE itype):
NeedsQ(input, pid, itype)
{
  char * uid_string(unsigned long uid);  /* in util.cc */
#ifdef EDEBUG
  if (edebug & FLOWER && edebug & QUEUES)
    {
      sprintf(debugger,
	      "echo %lu:RespondOnce called for that %s on %d",
	      _uid, get_itype(_itype), pid);
      doutput(debugger);
    }
#endif		  
 if (_status != OK && _itype != CREATE_CONF)
    {
      qlist.send(DEL_NO_CONF, _qid, 1);
#if EDEBUG
      if (edebug & QUEUES)
      	dlog << "Asked " << uid_string(_uid) << " to drop " << _qid 
      	     << " : pid = " << pid << ". for itype = " << get_itype(_itype);
#endif
    }
}
RespondOnce::~RespondOnce(void)
{
  YESorNO x = NO;
  YESorNO &no = x;
#ifdef EDEBUG
  if (edebug & FLOWER && edebug & QUEUES)
    {
      sprintf(debugger,
	      "echo %lu:Deleting RespondOnce and qid %d for that %s on %d",
	      (unsigned long)999, _p_outq->qid(), get_itype(_itype), _pid);
      doutput(debugger);
    }
#endif		  
  qlist.drop_q(_p_outq, no, NO);
}

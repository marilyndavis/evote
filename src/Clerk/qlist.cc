/* $Id: qlist.cc,v 1.4 2003/01/15 18:54:07 marilyndavis Exp $ */ 
// qlist.cc Maintains the list of all outgoing queues in the system
//          Sends messages to the queues.
/*********************************************************
 **********************************************************/
#include<stdio.h>
#include<sys/types.h>
#include<signal.h>
#include"evotedef.h"
#include"inq.h"
#include"qlist.h"
#include"voter.h"
#include"outq.h"
extern InQ inq;
extern long now;
extern int msgmax;
extern int msgtql;
#include <fstream.h>
#include <errno.h>
#include <stdlib.h>
extern ofstream *logger;
extern int msgmni;
char * get_rtype(RTYPE);
char * get_itype(ITYPE);
#ifdef EDEBUG
#include"debug.h"
unsigned long this_uid;
unsigned this_pid;
unsigned this_qid;
#endif
// ********************************
//  attach   attaches the entry to the q list and returns the index where it
//        is stored.
OKorNOT
QList::attach(OutQ* new_q)
{
  char msg[80];
  int count = 0;
  new_q->_next = _first;
  _first = new_q;
  ++_active_qs;
  while (_active_qs > msgmni - MSG_TRIGGER && ++count < 3)
    {
      drop_readies();
      if (count == 2)
	{
	  clerklog << "The number of available message queues is only "
		   << msgmni - MSG_TRIGGER << ".";
	  sprintf(msg, FNE_DROP_OLDQS, DROP_OLDQS, (unsigned long)0);
	  inq.send_myself(msg, PR_DROP_OLDQS);
	}
    }
  /*  if (_active_qs > msgmni - MSG_TRIGGER)
    {
      clerklog << "System problem. Can't make more message queues.  MSGMNI = "
	       << msgmni << " needs to be higher.";
      exit(1);
      } */
#ifdef EDEBUG
  if (edebug & FLOWER && edebug & QUEUES)
    {
      sprintf(debugger,
	      "echo %lu:Clerk attached qid %d, pid %d active_qs = %d",
	      new_q->uid(), new_q->_qid, 
	      new_q->_pid, _active_qs);
      doutput(debugger);
    }
  if (edebug & QUEUES || edebug & LIMITS)
    {
      dlog << "QList::attach qid = " << new_q->_qid 
	   << " pid = " << new_q->_pid 
	   << " for user " << new_q->uid() 
	   << ", p_voter at " << new_q->_p_voter;
      if (new_q->_p_voter != NULL)
	dlog << " for voter " 
	     << new_q->_p_voter->uid();
      dlog << "    Active queues, not counting in-queue= " << _active_qs 
	   << ".  Limit, MSGMNI = " << msgmni; 
    }
#endif
  return OK;
}		
//************************************************************
#ifdef EDEBUG
OKorNOT
QList::check(void)
{
  struct msqid_ds buf;
  int msgs_waiting = 0;
  OutQ* qx = _first;
  unsigned long voter_uid = 0;
  if (edebug & LIMITS)
    {
      while (qx != NULL)
	{
	  if (msgctl(qx->_qid, IPC_STAT, &buf) == -1)
	    {
	      voter_uid = 0;
	      if (qx->_p_voter != NULL)
		voter_uid = qx->_p_voter->uid();
	      dlog << "Queue is gone: qid = " << qx->_qid
		   << " voter at " << qx->_p_voter;
	      dlog << "   pid = " << qx->_pid << ". uid = " 
		   << qx->uid() << ". voter = "
		   << voter_uid;
	    }
	  else
	    msgs_waiting += buf.msg_qnum;
	  qx = qx->_next;
	}
      dlog << msgs_waiting << " messages waiting in out queues.";
    }
  qx = _first;
  if (edebug & FLOWER && edebug & QUEUES)
    {
      sprintf(debugger,
	      "echo Clerk checking queues ");
      doutput(debugger);
      while (qx != NULL)
	{
	  unsigned long voter_uid = 0;
	  if (qx->_p_voter != NULL)
	    voter_uid = qx->_p_voter->uid();
	  sprintf(debugger,"echo ... queue %d for pid %d user %lu and voter %lu ready = %s",
		  qx->_qid, qx->_pid, qx->uid(), voter_uid,
		  qx->ready_to_drop ? "YES" : "NO");
	  doutput(debugger);
	  qx = qx->_next;
	}
    }
  return OK;
}
#endif
//************************************************************
OKorNOT
QList::detach(int qid)
{
  OutQ* qx = _first;
  OutQ* before = NULL;
  while (qx != NULL && qx->_qid != qid)
    {
      before = qx;
      qx = qx->_next;
    }
  if (qx == NULL || qx->_qid != qid)
    {
      return NOT_OK;
    }
  if (before != NULL)
    before->_next = qx->_next;
  else // qx == _first
    _first = qx->_next;
  _active_qs--;
#ifdef EDEBUG
  if (edebug & FLOWER && edebug & QUEUES)
    {
      sprintf(debugger,
	      "echo %lu:Clerk QList::detach and deleted qid %d, pid %d active_qs = %d",
	      qx->uid(), qx->_qid, 
	      qx->_pid, _active_qs);
      doutput(debugger);
    }
  if (edebug & QUEUES && edebug & LIMITS)
    {
      dlog << "Detached and deleted qid " << qx->_qid << " for user " 
	   << qx->uid() << " on pid " << qx->_pid;
      if (qx->_p_voter != NULL)
	dlog << "  Voter is " << qx->_p_voter->uid() ;
      dlog << "   Active queues = " << _active_qs;
    }
#endif
  delete qx;
#ifdef EDEBUG
  if (edebug & MEMS)
    {
      vmem("Deleting a queue", NO, -sizeof(OutQ));
    }
#endif
  return OK;
}
//  *******************************************************
//    drops the queues whose pid has died.
OKorNOT
QList::drop_oldqs(void)
{
  OutQ * this_q = _first;
  OutQ * next_q = NULL;
  int dropped = 0;
  YESorNO x = YES;
  YESorNO & dummy = x;
  struct msqid_ds buf;
#ifdef EDEBUG
  if (edebug & QUEUES && edebug & FLOWER)
    {
      sprintf(debugger, "drop_oldqs called.");
      doutput(debugger);
      check();
    }
#endif
  dropped = drop_readies();
  this_q = _first;
  next_q = NULL;
  while (this_q != NULL)
    {
      next_q = this_q->_next;
      if (kill((int)this_q->_pid, 0) == -1)
	// should fail because pid is dead
	// or because there's no permission
	//  0  means don't really do it, just check 
	{		
	  switch (errno)
	    {
	    case ESRCH:  // dead pid
#ifdef EDEBUG
	      if (edebug & QUEUES)
		{
		  dlog << "In drop_oldqs, dead pid " << (int)this_q->_pid
		       << ". Dropping queue.";
		  (*dlogger).flush();
		}
	      if (edebug & FLOWER && edebug & QUEUES)
		{
		  sprintf(debugger,
			  "echo %lu:Clerk found dead pid %d for qid %d last use is %ld now is %ld",
			  this_q->uid(), this_q->_pid,
			  this_q->_qid, this_q->_last_use, now);
		  doutput(debugger);
		}
#endif		  
	      if (this_q->ready_to_drop)
		{
		  drop_q(this_q, dummy, YES);
		  dropped++;
		}
	      else
		{
		  if (this_q->_last_use < now - 300)
		    {
		      // The pid is dead and we haven't seen a
		      // leaving message for 5 minutes so let's
		      // figure they died without leaving
		      this_q->ready_to_drop = YES;
		      if (this_q->_p_voter)
			{
			  this_q->_p_voter->p_conf()->community().forget(this_q->_p_voter);
			}
		    }
		}
	      break;
	    case EPERM: // pid still alive
#ifdef EDEBUG
	      if (edebug & FLOWER && edebug & QUEUES)
		{
		  sprintf(debugger,
			  "echo %lu:Clerk found still alive pid %d for qid %d",
			  this_q->uid(), this_q->_pid,
			  this_q->_qid);
		  doutput(debugger);
		}
	      if (edebug & QUEUES)
		{
		  unsigned long voter_uid = 0;
		  if (this_q->_p_voter != NULL)
		    {
		      voter_uid = this_q->_p_voter->uid();
		    }		      
		  dlog << "In clean, live pid for voter "
		       << voter_uid << ". qid = "
		       << this_q->_qid << ". user = " << voter_uid
		       << ". pid = "
		       << this_q->_pid << ". ";
		  if (this_q->_p_voter != NULL)
		    dlog << "\n   Ballot at "
			 << this_q->_p_voter->p_ballot();
		  (*dlogger).flush();
		}
#endif
	      break;
	    default:
	      clerklog << "QList::drop_oldqs errno = " << errno;
	      break;
	    }
	}
      else // didn't fail - must be superuser and pid is alive
	{
	  int cc, er;
	  if ((cc = msgctl(this_q->qid(), IPC_STAT, &buf)) != 0)
	    {
	      if ((er = errno) == EINVAL)
		{
		  drop_q(this_q, dummy, YES);
		  dropped++;
		}
	    }
	  //	      if (this_q->_p_voter != NULL)
	  //		this_q->_p_voter->store_ballot();
	}
      this_q = next_q;
    }
  if (dropped > 0)
    return OK;
  else
    {
      return NOT_OK;
    }
}
//  *******************************************************
//    drops the queues whose didn't drop first time.
//    Called from attach and called with drop_oldqs
int
QList::drop_readies(void)
{
  OutQ * this_q = _first;
  OutQ * next_q = NULL;
  int dropped = 0;
  YESorNO x = YES;
  YESorNO & dummy = x;
#ifdef EDEBUG
  if (edebug & QUEUES && edebug & FLOWER)
    {
      doutput("drop_readies called");
    }
#endif
  while (this_q != NULL)
    {
      next_q = this_q->_next;
      if (this_q->ready_to_drop)
	{
#ifdef EDEBUG
	  if (edebug & FLOWER && edebug & QUEUES)
	    {
	      this_qid = this_q->_qid;
	      this_pid = this_q->_pid;
	      if (this_q->_p_voter)
		{
		  this_uid = this_q->_p_voter->uid();
		}
	    }
#endif
	  if (drop_q(this_q, dummy, NO) == OK)
	    {
	      dropped++;
#ifdef EDEBUG
	      if (edebug & FLOWER && edebug & QUEUES)
		{
		  sprintf(debugger,
			  "echo %lu: Ready to drop and dropped for pid %d for qid %d",
			  this_uid, this_pid,  this_qid);
		  doutput(debugger);
		}
#endif
	    }
	}
      this_q = next_q;
    }
  return dropped;
}
//  ************************************************************
OKorNOT
QList::drop_q(OutQ *& p_q, YESorNO & user_dropped, YESorNO force)
{
  int pid, qid;
  unsigned long uid;
  if (!p_q)
    {
      clerklog << "Tried to drop a null queue";
      return NOT_OK;
    }
  pid = p_q->_pid;
  qid = p_q->_qid;
  uid = p_q->uid();
#ifdef EDEBUG
  if (edebug & FLOWER && edebug & QUEUES)
    {
      sprintf(debugger,
	      "echo %lu:Clerk QList::drop_q dropping qid %d with a force = %s",
	      uid, qid, force? "YES" : "NO");
      doutput(debugger);
    }
#endif		  
  switch (p_q->drop(user_dropped, force))
    {
    case OK:
#ifdef EDEBUG
      if ((edebug & MEMS))
	vmem("Deleting Queue", NO, -sizeof(OutQ));
      if (edebug & FLOWER && edebug & QUEUES)
	{
	  sprintf(debugger,
		  "echo %lu:Clerk Dropped qid %d is OK", uid, qid);
	  doutput(debugger);
	}
#endif		  
      detach(qid);  // deletes the OutQ
      p_q = NULL;
      break;
    case UNDECIDED:  // try later
#ifdef EDEBUG
      if ((edebug & MEMS))
	vmem("Deleting Queue", NO, -sizeof(OutQ));
      if (edebug & FLOWER && edebug & QUEUES)
	{
	  sprintf(debugger,
		  "echo %lu:Clerk did not drop qid %d.  Still busy.", uid, qid);
	  doutput(debugger);
	}
#endif		  
      break;
    default:
      clerklog << "Unable to drop queue: " << p_q->_qid << " errno = "
	       << errno;
      break;
    }
  return OK;
}
// ***********************************************************
//    Called when the Clerk is coming down only.
OKorNOT
QList::drop_qs(void)
{
  OutQ *qx = _first;
  OKorNOT status = OK;
  YESorNO x = YES;
  YESorNO &yes = x;
  while (qx != NULL)
    {
      if (drop_q(qx, yes, YES) != OK)
	{
	  status = NOT_OK;
	}
      qx = _first;
    }
  return status;
}
// ***********************************************************
OutQ*
QList::find(int qid)
{
  OutQ *qx = _first;
  while (qx != NULL && qx->_qid != qid )
    {
      qx = qx->_next;
    }
  if (qx == NULL)
    {
#ifdef EDEBUG
      if (edebug & VOTERS)
	dlog << "\nCouldn't find queue for qid = " << qid;
#endif
      return NULL;
    }
#ifdef EDEBUG
  if (edebug & VOTERS)
    {
      dlog << "\nFound queue for qid = " << qid << ".\n";
      if (qx->_p_voter != NULL)
	dlog << "  voter at "
	     << qx->_p_voter << ".  Ballot at "
	     << (qx->_p_voter)->p_ballot();
      (*dlogger).flush();
    }
#endif
  return qx;
}
// ***********************************************************
OutQ*
QList::match(int pid)
{
  OutQ *qx = _first;
  while (qx != NULL && (qx->_pid != pid 
	  || qx->ready_to_drop == YES))
    {
      qx = qx->_next;
    }
  if (qx != NULL)
    {
      time(&(qx->_last_use));
    }
  if (qx == NULL)
    {
#ifdef EDEBUG
      if (edebug & VOTERS)
	dlog << "\nCouldn't find queue for pid = " << pid;
#endif
      return NULL;
    }
#ifdef EDEBUG
  if (edebug & VOTERS)
    {
      dlog << "\nFound queue for pid = " << pid <<
	" qid = " << qx->_qid << ".\n";
      if (qx->_p_voter != NULL)
	dlog << "  voter at "
	     << qx->_p_voter << ".  Ballot at "
	     << (qx->_p_voter)->p_ballot();
      (*dlogger).flush();
    }
#endif
  return qx;
}
// ***********************************************************
OutQ*
QList::newq(int pid, unsigned long uid, Voter* p_voter)
{
  OutQ* newq = new OutQ(pid, uid, p_voter);
#ifdef EDEBUG
  if (edebug & FLOWER && edebug & QUEUES)
    {
      sprintf(debugger,
	      "echo %lu:Clerk Made a new qid %d for pid %d and voter %lu",
	      uid, newq->_qid, pid, (p_voter? p_voter->uid(): 0));
      doutput(debugger);
    }
  if (edebug & MEMS)
    vmem("Making new Queue", NO, sizeof(OutQ));
#endif
  attach(newq);
  return newq;
}
//  ***************************************************************
//   Sends a message on the queue
OKorNOT
QList::send(long priority, int qid, int len)
{
  // This is called by Instructions to send messages back to the
  // voters.  _strm already has the message in it, if there's
  // a text message.  Text messages end in "Q!".
  if (len == 0)
    {
      len = -1;
      do
	{
	  while (_sysmsg->mtext[++len] && _sysmsg->mtext[len] != 'Q')
	    {
	      if (len >= msgmax)
		{
		  clerklog << "Tried to send a message without an end";
		  return NOT_OK;
		}
	    }
	}
      while (_sysmsg->mtext[len + 1] != '!');
      _sysmsg->mtext[len++] = '\0';
    }
  return send_msg(priority, qid, len);
}
// *********************************************************
//      really sends the message in the _sysmsg->mtext
//      to the mentioned queue.  
//      len is the length of the message to be sent
OKorNOT
QList::send_msg(long priority, int qid, int len)
{
  // This private function really sends the message
  int cc;
  int tries = 0;
  _sysmsg->mtype = priority;
  YESorNO try_again = NO;
  if (len > msgmax)
    {
      clerklog << "Message too long: "<< len << " bytes, only " 
	       << msgmax	<< "allowed." << "\n   mtype = " 
	       << _sysmsg->mtype << ".  Start of message = \n"
	       << _sysmsg->mtext;
      return NOT_OK;
    }
#ifdef EDEBUG
  if (edebug & FLOWER)
    {
      OutQ* p_q;
      if (qid == inq._msgqid)
	{
	  int type;
	  sscanf(_sysmsg->mtext, " %d", &type);
	  sprintf(debugger,
		  "echo ____:Clerk: snd to myself qid %d, itype = %s :",
		  qid, get_itype((ITYPE)type));
	}	  
      else
	{
	  p_q = find(qid);
	  if (p_q == NULL)
	    {
	      sprintf(debugger,
		      "echo ____:Clerk: snd to qid %d, pid _____ rtype = %s :",
		      qid, get_rtype((RTYPE)_sysmsg->mtype));
	    }
	  else
	    {			
	      sprintf(debugger,
		      "echo %lu:Clerk: snd to qid %d, pid %d rtype = %s for %lu:",
		      p_q->uid(), qid, 
		      p_q->_pid, get_rtype((RTYPE)_sysmsg->mtype),
		      p_q->p_voter() ? p_q->p_voter()->uid() : 
		      (unsigned long)0);
	    }
	}
      if (_sysmsg->mtype != NEW_STAT && _sysmsg->mtype != NEW_STATS
	 && _sysmsg->mtype != MORE_STATS && _sysmsg->mtype != VIOLATES_SUM)
	strcat(debugger, _sysmsg->mtext);
      if (_sysmsg->mtype == VIOLATES_SUM)
	{
	  unsigned int len;
	  len = (unsigned int)strlen(debugger);
	  debugger[len-5] = '\0';
	}
      doutput(debugger);
    }
#endif
  cc = -1;
  tries = 0;
  while ((cc = msgsnd(qid, _sysmsg, len, IPC_NOWAIT)) == -1)
    {
      switch (errno)
	{
	case EACCES:
	  clerklog << "\nNo permission to send.";
	  break;
	case EINVAL:
	  if ((RTYPE)_sysmsg->mtype > RESEND)
	    {
	      clerklog << "\nmsg qid not valid or mtype < 1 or size too big."
		       << "\nlen = " << len << " qid = " << qid;
	    }
	  break;
	case EAGAIN:
	  if (tries == 0)
	    {
	      clerklog << "Message queues are full!";
	      drop_oldqs();
	      tries++;
	      try_again = YES;
	    }
	  else
	    {
	      clerklog << "Too many message bytes in the system. "
		       << "Clerk is too far behind.  Coming down.  ";
	      clerklog << "Try running eVote_Clerk as root so you can increase the priority.";
	      exit(0);
	    }
	  break;
	case EFAULT:
	  clerklog << "Points to illegal address.";
	  break;
	case 43:
	  clerklog << "Identifier removed.";
	  break;
	default:
	  clerklog << "msgsnd: Unknown error: " << errno;
	  if (++tries > 4)
	    {
	      clerklog << "Giving up "
		       << "Coming down.";
	      exit(0);
	    }
	  try_again = YES;
	  break;
	}
      if ((RTYPE)_sysmsg->mtype > RESEND)
	{
	  clerklog << "Message not sent to qid " << qid << ": "
		   << len << " bytes. " << msgmax << " bytes allowed "
		   << msgtql << " total bytes allowed. " << "\n   mtype = " 
		   << (qid == inq._msgqid ? get_itype((ITYPE)_sysmsg->mtype) :
		       get_rtype((RTYPE)_sysmsg->mtype)) << ".  Message = \n"
		   << (char*)_sysmsg->mtext;
	}
#ifdef EDEBUG
      if (edebug & FLOWER)
	{
	  OutQ* p_q;
	  p_q = find(qid);
	  if (p_q == NULL)
	    {
	      if (qid == inq._msgqid)
		{
		  sprintf(debugger,
			  "echo ____:Clerk: snd to self %d, pid _____ itype = %s :",
			  qid, get_itype((ITYPE)_sysmsg->mtype));
		}
	      else
		{
		  sprintf(debugger,
			  "echo ____:Clerk: snd to qid %d, pid _____ rtype = %s :",
			  qid, get_rtype((RTYPE)_sysmsg->mtype));
		}
	    }
	  else
	    {			
	      sprintf(debugger,
		      "echo %lu:Clerk: snd to qid %d, pid %d rtype = %s for %lu:",
		      p_q->uid(), qid, 
		      p_q->_pid, get_rtype((RTYPE)_sysmsg->mtype),
		      p_q->p_voter() ? p_q->p_voter()->uid(): 
		      (unsigned long)0);
	    }
	  if (_sysmsg->mtype != NEW_STAT && _sysmsg->mtype != NEW_STATS
	     && _sysmsg->mtype != MORE_STATS && _sysmsg->mtype != VIOLATES_SUM)
	    strcat(debugger, _sysmsg->mtext);
	  if (_sysmsg->mtype == VIOLATES_SUM)
	    {
	      unsigned int len;
	      len = (unsigned int)strlen(debugger);
	      debugger[len-5] = '\0';
	    }
	  strcat(debugger,
		  "THIS FAILED!!!!!");
	  doutput(debugger);
	}
#endif
      if (tries == 1)
	continue;
      if ((RTYPE)_sysmsg->mtype > RESEND)
	{
	  clerklog << "Can't send a message to queue " << qid;
	  (*logger).flush();
	}
      return NOT_OK;
    }
  return OK;
}

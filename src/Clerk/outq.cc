/* $Id: outq.cc,v 1.4 2003/01/15 18:54:07 marilyndavis Exp $ */ 
// Implements an outgoing message queue to the user interface
/*********************************************************
 **********************************************************/
#include"outq.h"
#include<sys/msg.h>
#include<errno.h>
#include<stdlib.h>
#include"qlist.h"
extern QList qlist;
extern long now;
#include <fstream.h>
extern ofstream *logger;
#ifdef EDEBUG
#include"debug.h"
#endif
extern int msgmni;
// **********************************************************
//   this constructor is called when a new return queue is needed
//   by Instruction::NeedsQ 
OutQ::OutQ(int pid, unsigned long uid, Voter* p_voter)
  :_pid(pid), _uid(uid), _p_voter(p_voter), _next(NULL), ready_to_drop(NO)
{
  struct msqid_ds ctlbuf;
#ifdef EDEBUG
  if (edebug & QUEUES)
    dlog << "Making queue for " << uid << ", pid = " << pid << ".";
  if (edebug & FLOWER && edebug & QUEUES)
    {
      sprintf(debugger,
	      "echo %u:Clerk OutQ making new queue for %lu from pid %d.", 
	      999, uid, pid);
      doutput(debugger);
    }
#endif
  while ((_qid = msgget((key_t)pid, IPC_CREAT|0600)) == -1)
    {
      switch (errno)
	{
	case ENOSPC:
	  if (qlist.drop_oldqs() != OK)
	    {
	      clerklog << "The system tunable parameter, MSGMNI = "
		       << msgmni << " needs to be higher.";
	      exit(0);
	    }				
	  break;
	case EACCES:
	  //	  clerklog << "Waiting for access to queue, pid = " << pid;
	  sleep(1);
	  break;
	default:
	  clerklog << "Unable to open queue to voter, pid = " << pid
		   << " errno = " << errno;
	  exit(0);
	  break;
	}
    }
  // Now we fix the uid so that the voter can delete this queue.
  if (_uid != 0)
    {
      if (msgctl(_qid, IPC_STAT, &ctlbuf) == -1)
	{
	  clerklog << "Clerk:msgctl: Can't inspect queue to voter: " <<
	    errno;
	  return;
	}
      if (ctlbuf.msg_perm.cuid != uid)
	{
	  ctlbuf.msg_perm.uid = uid;	
	  if (msgctl(_qid, IPC_SET, &ctlbuf) == -1)
	    {
	      clerklog << "\nClerk:msgctl: Can't fix uid on queue to voter: "
		       << errno;
	      return;
	    }
	}
    }
#ifdef EDEBUG
  if (edebug & QUEUES)
    dlog << "Made queue for " << uid << ", pid = " << pid << " qid = " << _qid;
  if (edebug & FLOWER && edebug & QUEUES)
    {
      sprintf(debugger,
	      "echo %u:Clerk OutQ %d made for %lu from pid %d.", 
	      999, _qid, uid, pid);
      doutput(debugger);
    }
#endif
  if (p_voter != NULL)  
    {
      attach_voter(p_voter);
    }
  time(&_last_use);
}
// ******************************************
//  void OutQ::drop()  drops the queue from the system
OKorNOT
OutQ::drop(YESorNO & user_dropped, YESorNO force)
{
  struct msqid_ds buf;
  int times = 0;
  do
    {
      if (msgctl(_qid, IPC_STAT, &buf) == -1)
	{
#ifdef linux  // 43 is EIDRM for Linux
	  if (errno == EIDRM || errno == EINVAL)
#else
	    if (errno == EINVAL)
#endif
	      {
		user_dropped = YES;
#ifdef EDEBUG
		if (edebug & FLOWER && edebug & QUEUES)
		  {
		    sprintf(debugger,
			    "echo %u:Clerk OutQ::drop found already gone on qid = %d",
			    999, _qid);
		    doutput(debugger);
		  }
#endif		  
		return OK;  // dropped by the other side
	      }
	  clerklog << "OutQ::drop(): msgctl failed, errno = " << errno;
	  break;
	}
      if (buf.msg_qnum > 0)
	sleep(1);
    }
  while (force == YES && buf.msg_qnum > 0 && ++times < 2);
  if (force == NO && buf.msg_qnum > 0)
    {
#ifdef EDEBUG
      if (edebug & FLOWER && edebug & QUEUES)
	{
	  sprintf(debugger,
		  "echo %u:Clerk OutQ::drop undecided about qid = %d. Keeping.",
		  999, _qid);
	  doutput(debugger);
	}
#endif		
      ready_to_drop = YES;
      return UNDECIDED;
    }
  if (msgctl(_qid, IPC_RMID, &buf) == -1)
    {
      switch (errno)
	{ 
#ifdef linux
	case EIDRM:  // 43 is EIDRM for Linux
#endif
	case EINVAL:
	  // dropped between last call and this by other side
#ifdef EDEBUG
  if (edebug & FLOWER && edebug & QUEUES)
    {
      sprintf(debugger,
	      "echo %u:Clerk OutQ::drop found qid = %d gone now.",
	      999, _qid);
      doutput(debugger);
    }
#endif		  
	  user_dropped = YES;
	  return OK;
	  break;
	case EPERM:
	  clerklog << "OutQ::drop: not owner on " << _qid
		   << "  buf.msg_perm.cuid = " << buf.msg_perm.cuid
		   << "; uid = " << buf.msg_perm.uid;
	  break;
	default:
	  clerklog << "OutQ::drop: unknown error on " << _qid;
	  break;
	}
      return NOT_OK;
    }
#ifdef EDEBUG
  if (edebug & FLOWER && edebug & QUEUES)
    {
      sprintf(debugger,
	      "echo %u:Clerk OutQ::dropped qid = %d.",
	      999, _qid);
      doutput(debugger);
    }
#endif		  
  return OK;
}

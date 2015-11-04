/* $Id: inq.cc,v 1.4 2003/01/15 18:54:07 marilyndavis Exp $ */ 
// inq.cc -- Controlls the incoming message queue.
// instantiates the instructions
/*********************************************************
 *    Copyright (c) 1994...2015 Deliberate.com Patented.
 *    by Marilyn Davis
 **********************************************************/
#include <stdlib.h>
#include <stdio.h>
#include <fstream.h>
#include <errno.h>
#include <stdio.h>
#include "evotedef.h"
extern long now;
extern ofstream *logger;
extern int msgmni;
extern int msgmax;
extern int msgtql;
void doutput(char *);
#include "inq.h"
#include "instruct.h"
#include "qlist.h"
extern QList qlist;
#ifdef EDEBUG
#include"debug.h"
#include"voter.h"
#endif
//  *********************************************************
//     starts the message queue 
OKorNOT
InQ::start(char * key_str)
{
  struct msqid_ds buf;
  YESorNO try_again = NO;
  YESorNO trouble = NO;
  clerk_key = (key_t)63220L;
  if (key_str[0] != '\0')
    clerk_key = (key_t)atol(key_str);
  time(&now);
  _sysmsg = (struct msgbuf *)new char[(sizeof(long) + msgmax)];
  do
    {
      try_again = NO;
      if ((_msgqid = msgget(clerk_key,
			     IPC_CREAT | (long)00666 | IPC_EXCL)) == -1)
	// Others need to write the queue and also to read it
	// to do a IPC_STAT on it to see who owns it so that
	// they can set their input queue so that only The Clerk can
	// write to it, also to be sure there aren't too many
	// messages waiting on the queue.
	{
	  switch (errno)
	    {
	    case EACCES:
	      cerr << "\nNo access.";
	      break;
	    case ENOENT:
	      cerr <<  "\nNo entity.";
	      break;
	    case ENOSPC:
	      cerr << "\nNo space.";
	      break;
	    case EEXIST:
	      cerr << "\nIn Q Already exists. Restarting it.";
	      if ((_msgqid = msgget(clerk_key, 0 )) == -1)
		{
		  cerr << "\nStill can't open.";
		}
	      else
		{
		  cerr << "\nOld Q id is " << _msgqid << ".  Trying again.";
		  msgctl(_msgqid, IPC_RMID, &buf);
		  try_again = YES;
		  trouble = YES;
		}
	      break;
	    default:
	      cerr << "\nmsgget: Unknown error:" << errno;
	      break;
	    }
	}
    } while (try_again == YES);
  if (_msgqid == -1)
    {							
      cerr <<  "\nTrouble starting queue to eVote.";
      cerr <<  "\nWrite bugs@deliberate.com \n";
      return NOT_OK;
    }
  if (trouble == YES)
    {
      cerr << "\nOK.  Queue is ready.";
    }
  return OK;
}
// *********************************************
InQ::~InQ(void)
{ 
  struct msqid_ds buf;
  if (msgctl(_msgqid, IPC_RMID, &buf) == -1)
    {
      switch (errno)
	{ 
	case EINVAL:
	  //	  cerr << " ~InQ: no queue ";
	  break;
	case EACCES:
	  cerr << "	~InQ: no permission on ";
	  break;
	default:
	  cerr << "	~InQ: unknown error on ";
	  break;
	}
    }
}
unsigned long who_am_i(void){return (unsigned long)0;}
// ***************************************
//  OKorNOT deliver (YESorNO wait, char * key_str)
//       key_str is the string representation of the
//               key to the receiving queue.
OKorNOT
InQ::deliver(YESorNO wait, char *key_str)
{
  static short commands_since_priority_change = 0;
  int len;
  ITYPE itype;
  struct msqid_ds ctlbuf;
  long priority = - 1000l;
#ifdef EDEBUG
  int memsize;
#endif  
  int pid;
  Instruction* instruction;
//       priority = 0  receives first message of any type
//                = n  receives first message of type n
//                = -n receives first lowest type <= n.
  if ((len = msgrcv(_msgqid, _sysmsg, msgmax + sizeof(long),
		    priority, (wait ? 0 : IPC_NOWAIT))) == -1)
    {
      switch (errno)
	{
	case 4:  /* stopped in debugger or sent a kill */
	case ENOMSG:  /* happens when IPC_NOWAIT and no msg */
	  // no message found in msgrcv above
	  return UNDECIDED;
	  break;
	case EIDRM:  /* queue removed while waiting for msg */	
	default:
	  clerklog << "errno = " << errno << " in InQ::deliver. Interpreting as a quit.";
	  (void)sprintf(_sysmsg->mtext, FNE_QUIT, NNE_QUIT);
	  len = (int)strlen(_sysmsg->mtext);
	  _sysmsg->mtype = QUIT;
	  sprintf(&(_sysmsg->mtext[len])," %6d", 0);
	  break;
	}
    }
  //		_sysmsg->mtext[len] = '\0';  // to clean up cout's
  // use the caller's pid for the return key			
  pid = atoi(&_sysmsg->mtext[len-7]);
  msgctl(_msgqid, IPC_STAT, &ctlbuf); 
  if (ctlbuf.msg_qnum > (unsigned)msgtql - 5)
    {
      clerklog << ctlbuf.msg_qnum 
	       << " messages on queue.  The limit is "
	       << msgtql;
      (*logger).flush();
    }
#ifdef EDEBUG
  if (edebug & LIMITS)
    {
      dlog << ctlbuf.msg_qnum 
	   << " messages on in queue.  Limit is " << msgtql;
    } 
  if (edebug & FLOWER)
    {
      OutQ* p_q;
      char * get_itype(ITYPE);
      ITYPE itype;
      p_q = qlist.match(pid);  
      sprintf(debugger,"%ld messages on in queue. limit is %d %d active qs limit is %d.",
	      (long)ctlbuf.msg_qnum,
	      msgtql, qlist._active_qs, msgmni);
	      /*      sprintf(debugger, "echo XXXXXXXXXXXXXXXXX "); */
      doutput(debugger); 
      sscanf(_sysmsg->mtext,"%3d", &(int)itype);
      if (p_q != NULL)
	sprintf(debugger,
		"echo %lu:Clerk: rcv has out qid %d, pid %d itype = %s len = %d: %s ",
		(p_q->p_voter() == NULL ? (unsigned long)0 : p_q->p_voter()->uid()),
		p_q->qid(), pid, 
		get_itype(itype), len, _sysmsg->mtext);
      else
	sprintf(debugger,
		"echo ____:Clerk: rcv has out qid _____, pid %d itype = %s len = %d: %s ",
		pid, get_itype(itype), len, _sysmsg->mtext);
      doutput(debugger);
    }	
#endif			
  sscanf(_sysmsg->mtext,"%3d", &(int)itype);
  len -= 6;
  commands_since_priority_change++;
  switch (itype)
    {
    case ADJOURN_CONF:
#ifdef EDEBUG
      if (edebug & MEMS)
	{
	  vmem("Made an Instruction --AdjournConf", NO, memsize = sizeof(AdjournConf));
	}
      if (edebug & MESSAGES)
	dlog << "\nAdjournConf : " << _sysmsg->mtext << " from pid " << pid;
#endif					
      instruction = new AdjournConf(_sysmsg->mtext, pid, itype);
      delete (AdjournConf*)instruction;
      break;
    case CHANGE_ACTION:
#ifdef EDEBUG
      if (edebug & MEMS)
	{
	  vmem("Made an Instruction --ChangeAction", NO, memsize = sizeof(ChangeAction));
	}
      if (edebug & MESSAGES)
	dlog << "\nChangeAction : " << _sysmsg->mtext << " from pid " << pid;
#endif					
      instruction = new ChangeAction(_sysmsg->mtext, pid, itype);
      delete (ChangeAction*)instruction;
      break;
    case CHANGE_VSTATUS:
#ifdef EDEBUG
      if (edebug & MEMS)
	{
	  vmem("Made an Instruction --  ChangeVStatus", NO, memsize = sizeof(ChangeVStatus));
	}
      if (edebug & MESSAGES)
	dlog << "\nChangeVStatus : " << _sysmsg->mtext << " from pid " << pid;
#endif					
      instruction = new ChangeVStatus(_sysmsg->mtext, pid, itype);
      delete (ChangeVStatus*)instruction;
      break;
    case CHECK_CONF:
#ifdef EDEBUG
      if (edebug & MEMS)
	{
	  vmem("Made an Instruction --  CheckConf", NO, memsize = sizeof(CheckConf));
	}
      if (edebug & MESSAGES)
	dlog << "\nCheck_Conf : " << _sysmsg->mtext << " from pid " << pid;
#endif
      instruction = new CheckConf(_sysmsg->mtext, pid, itype);
      delete (CheckConf*)instruction;
      break;
    case CREATE_CONF:
#ifdef EDEBUG
      if (edebug & MEMS)
	{
	  vmem("Made an Instruction -- CreateConf", NO, memsize = sizeof(CreateConf));
	}
      if (edebug & MESSAGES)
	dlog << "\nCreateConf : " << _sysmsg->mtext << " from pid " << pid;
#endif
      instruction = new CreateConf(_sysmsg->mtext, pid, itype);
      delete (CreateConf*)instruction;
      break;
    case CREATE_ITEMS:
#ifdef EDEBUG
      if (edebug & MEMS)
	{
	  vmem("Made an Instruction -- CreateItems", NO, memsize = sizeof(CreateItems));
	}
      if (edebug & MESSAGES)
	dlog << "\nCreate_Items : " << _sysmsg->mtext << " from pid " << pid;
#endif
      instruction = new CreateItems(_sysmsg->mtext, pid, itype, len);
      delete (CreateItems*)instruction;
      break;
    case DO_DEBUG:
#ifdef EDEBUG
      if (edebug & MEMS)
	{
	  vmem("Made an Instruction -- DoDebug", NO, memsize = sizeof(DoDebug));
	}
      if (edebug & MESSAGES)
	dlog << "\nDo_Debug : " << _sysmsg->mtext << " from pid " << pid;
#endif
      instruction = new DoDebug(_sysmsg->mtext, pid, itype);
      delete (DoDebug*)instruction;
      break;
    case DOWN_PRIORITY:
      commands_since_priority_change = 0;
#ifdef EDEBUG
      if (edebug & MEMS)
	{
	  vmem("Made an Instruction -- DownPriority", NO, memsize = sizeof(DownPriority));
	}
      if (edebug & MESSAGES)
	dlog << "\nDownPriority : " << _sysmsg->mtext << " from pid " << pid;
#endif
      instruction = new DownPriority(_sysmsg->mtext, pid, itype);
      delete (DownPriority*)instruction;
      break;
    case DROP_CONF:
#ifdef EDEBUG
      if (edebug & MEMS)
	{
	  vmem("Made an Instruction -- DropConf", NO, memsize = sizeof(DropConf));
	}
      if (edebug & MESSAGES)
	dlog << "\nDrop_Conf : " << _sysmsg->mtext << " from pid " << pid;
#endif
      instruction = new DropConf(_sysmsg->mtext, pid, itype);
      delete (DropConf*)instruction;
      break;
    case DROP_OLDQS:
#ifdef EDEBUG
      if (edebug & MEMS)
	{
	  vmem("Made an Instruction -- DropOldQs", NO, memsize = sizeof(DropOldQs));
	}
      if (edebug & MESSAGES)
	dlog << "\nDrop_OldQs : " << _sysmsg->mtext << " from pid " << pid;
#endif
      instruction = new DropOldQs(_sysmsg->mtext, pid, itype);
      delete (DropOldQs*)instruction;
      break;
    case DROP_ITEMS:
#ifdef EDEBUG
      if (edebug & MEMS)
	{
	  vmem("Made an Instruction -- DropItems", NO, memsize = sizeof(DropItems));
	}
      if (edebug & MESSAGES)
	dlog << "\nDrop_Item : " << _sysmsg->mtext << " from pid " << pid;
#endif
      instruction = new DropItems(_sysmsg->mtext, pid, itype, len);
      delete (DropItems*)instruction;
      break;
    case DROP_VOTER:
#ifdef EDEBUG
      if (edebug & MEMS)
	{
	  vmem("Made an Instruction -- DropVoter", NO, memsize = sizeof(DropVoter));
	}
      if (edebug & MESSAGES)
	dlog << "\nDrop_Voter : " << _sysmsg->mtext << " from pid " << pid;
#endif
      instruction = new DropVoter(_sysmsg->mtext, pid, itype);
      delete (DropVoter*)instruction;
      break;
    case ENTER_ADMIN:
#ifdef EDEBUG
      if (edebug & MEMS)
	{
	  vmem("Made an Instruction -- EnterAdmin", NO, memsize = sizeof(EnterAdmin));
	}
      if (edebug & MESSAGES)
	dlog << "\nEnterAdmin : " << _sysmsg->mtext << " from pid " << pid;
#endif
      instruction = new EnterAdmin(_sysmsg->mtext, pid, itype);
      delete (EnterAdmin*)instruction;
      break;
    case ENTERING:
#ifdef EDEBUG
      if (edebug & MEMS)
	{
	  vmem("Made an Instruction -- Entering", NO, memsize = sizeof(Entering));
	}
      if (edebug & MESSAGES)
	dlog << "\nEntering : " << _sysmsg->mtext << " from pid " << pid;
#endif
      instruction = new Entering(_sysmsg->mtext, pid, itype);
      delete (Entering*)instruction;
      break;
    case EXIST:
#ifdef EDEBUG
      if (edebug & MEMS)
	{
	  vmem("Made an Instruction -- Exist", NO, memsize = sizeof(Exist));
	}
      if (edebug & MESSAGES)
	dlog << "\nExist : " << _sysmsg->mtext << " from pid " << pid;
#endif
      instruction = new Exist(_sysmsg->mtext, pid, itype);
      delete (Exist*)instruction;
      break;
    case FLUSH:
#ifdef EDEBUG
      if (edebug & MEMS)
	{
	  vmem("Made an Instruction -- Flush", NO, memsize = sizeof(Flush));
	}
      if (edebug & MESSAGES)
	dlog << "\nFlush : " << _sysmsg->mtext << " from pid " << pid;
#endif
      instruction = new Flush(_sysmsg->mtext, pid, itype);
      delete (Flush*)instruction;
      break;
    case GROW_CONF:
#ifdef EDEBUG
      if (edebug & MEMS)
	{
	  vmem("Made an Instruction -- GrowConf", NO, memsize = sizeof(GrowConf));
	}
      if (edebug & MESSAGES)
	dlog << "\nGrowConf : " << _sysmsg->mtext << " from pid " << pid;
#endif
      instruction = new GrowConf(_sysmsg->mtext, pid, itype);
      delete (GrowConf*)instruction;
      break;
    case HELLO:
#ifdef EDEBUG
      if (edebug & MEMS)
	{
	  vmem("Made an Instruction -- Hello", NO, memsize = sizeof(Hello));
	}
      /*      if (edebug & MESSAGES)
	{
	  dlog << "\nHello : " << _sysmsg->mtext << " from pid " << pid;
	  _qlist.check();
	  } */
#endif
      instruction = new Hello(_sysmsg->mtext, pid, itype);
      delete (Hello*)instruction;
      break;
    case HOW_VOTED:
#ifdef EDEBUG
      if (edebug & MEMS)
	{
	  vmem("Made an Instruction -- HowVoted", NO, memsize = sizeof(HowVoted));
	}
      if (edebug & MESSAGES)
	dlog << "\nHow_Voted : " << _sysmsg->mtext << " from pid " << pid;
#endif
      instruction = new HowVoted(_sysmsg->mtext, pid, itype);
      delete (HowVoted*)instruction;
      break;
    case I_READ:
#ifdef EDEBUG
      if (edebug & MEMS)
	{
	  vmem("Made an Instruction -- IRead", NO, memsize = sizeof(IRead));
	}
      if (edebug & MESSAGES)
	dlog << "\nI_Read : " << _sysmsg->mtext << " from pid " << pid;
#endif
      instruction = new IRead(_sysmsg->mtext, pid, itype);
      delete (IRead*)instruction;
      break;
    case SEND_STAMP:
#ifdef EDEBUG
      if (edebug & MEMS)
	{
	  vmem("Made an Instruction -- SendStamp", NO, memsize = sizeof(SendStamp));
	}
      if (edebug & MESSAGES)
	dlog << "\nSend_Vote : " << _sysmsg->mtext << " from pid " << pid;
#endif
      instruction = new SendStamp(_sysmsg->mtext, pid, itype);
      delete (SendStamp*)instruction;
      break;
    case SEND_VOTE:
#ifdef EDEBUG
      if (edebug & MEMS)
	{
	  vmem("Made an Instruction -- IVote", NO, memsize = sizeof(IVote));
	}
      if (edebug & MESSAGES)
	dlog << "\nSend_Vote : " << _sysmsg->mtext << " from pid " << pid;
#endif
      instruction = new IVote(_sysmsg->mtext, pid, itype);
      delete (IVote*)instruction;
      break;
    case JOINING:
#ifdef EDEBUG
      if (edebug & MEMS)
	{
	  vmem("Made an Instruction -- Joining", NO, memsize = sizeof(Joining));
	}
      if (edebug & MESSAGES)
	dlog << "\nJoining : " << _sysmsg->mtext << " from pid " << pid;
#endif
      instruction = new Joining(_sysmsg->mtext, pid, itype);
      delete (Joining*)instruction;
      break;
    case LEAVE_ADMIN:
#ifdef EDEBUG
      if (edebug & MEMS)
	{
	  vmem("Made an Instruction -- LeaveAdmin", NO, memsize = sizeof(LeaveAdmin));
	}
      if (edebug & MESSAGES)
	dlog << "\nLeaveAdmin : " << _sysmsg->mtext << " from pid " << pid;
#endif
      instruction = new LeaveAdmin(_sysmsg->mtext, pid, itype);
      delete (LeaveAdmin*)instruction;
      break;
    case LEAVING:
#ifdef EDEBUG
      if (edebug & MEMS)
	{
	  vmem("Made an Instruction -- Leaving", NO, memsize = sizeof(Leaving));
	}
      if (edebug & MESSAGES)
	dlog << "\nLeaving : " << _sysmsg->mtext << " from pid " << pid;
#endif
      instruction = new Leaving(_sysmsg->mtext, pid, itype);
      delete (Leaving*)instruction;
      break;
    case MOVE:
#ifdef EDEBUG
      if (edebug & MEMS)
	{
	  vmem("Made an Instruction -- Move", NO, memsize = sizeof(Move));
	}
      if (edebug & MESSAGES)
	dlog << "\nMove : " << _sysmsg->mtext << " from pid " << pid;
#endif
      instruction = new Move(_sysmsg->mtext, pid, itype);
      delete (Move*)instruction;
      break;
    case MID_DROPPED:
#ifdef EDEBUG
      if (edebug & MEMS)
	{
	  vmem("Made an Instruction -- MidDropped", NO, memsize = sizeof(MidDropped));
	}
      if (edebug & MESSAGES)
	dlog << "\nMid Dropped : " << _sysmsg->mtext << " from pid " << pid;
#endif
      instruction = new MidDropped(_sysmsg->mtext, pid, itype);
      delete (MidDropped*)instruction;
      break;
    case NEW_EXE:
#ifdef EDEBUG
      if (edebug & MEMS)
	{
	  vmem("Made an Instruction -- NewExe", NO, memsize = sizeof(NewExe));
	}
      if (edebug & MESSAGES)
	dlog << "\nNewExe : " << _sysmsg->mtext << " from pid " << pid;
#endif
      instruction = new NewExe(_sysmsg->mtext, pid, itype);
      delete (NewExe*)instruction;
      break;
    case NEW_LOG:
#ifdef EDEBUG
      if (edebug & MEMS)
	{
	  vmem("Made an Instruction -- NewLog", NO, memsize = sizeof(NewLog));
	}
      if (edebug & MESSAGES)
	dlog << "\nNew Log : " << _sysmsg->mtext << " from pid " << pid;
#endif
      instruction = new NewLog(_sysmsg->mtext, pid, itype);
      delete (NewLog*)instruction;
      break;
    case PULL_TIME:
#ifdef EDEBUG
      if (edebug & MEMS)
	{
	  vmem("Made an Instruction -- PullTime", NO, memsize = sizeof(PullTime));
	}
      if (edebug & MESSAGES)
	dlog << "\nNew Log : " << _sysmsg->mtext << " from pid " << pid;
#endif
      instruction = new PullTime(_sysmsg->mtext, pid, itype);
      delete (PullTime*)instruction;
      break;
    case PUSH_TIME:
#ifdef EDEBUG
      if (edebug & MEMS)
	{
	  vmem("Made an Instruction -- PushTime", NO, memsize = sizeof(PushTime));
	}
      if (edebug & MESSAGES)
	dlog << "\nNew Log : " << _sysmsg->mtext << " from pid " << pid;
#endif
      instruction = new PushTime(_sysmsg->mtext, pid, itype);
      delete (PushTime*)instruction;
      break;
    case QUIT:
#ifdef EDEBUG
      if (edebug & MEMS)
	{
	  vmem("Made an Instruction -- Quit", NO, memsize = sizeof(Quit));
	}
      if (edebug & MESSAGES)
	dlog << "\nQuit : " << _sysmsg->mtext << " from pid " << pid;
#endif
      instruction = new Quit(_sysmsg->mtext, pid, itype);
      delete (Quit*)instruction;
      break;
    case REORDER_CONF:
#ifdef EDEBUG
      if (edebug & MEMS)
	{
	  vmem("Made an Instruction -- ReorderConf", NO, memsize = sizeof(ReorderConf));
	}
      if (edebug & MESSAGES)
	dlog << "\nReorder : " << _sysmsg->mtext << " from pid " << pid;
#endif
      instruction = new ReorderConf(_sysmsg->mtext, pid, itype);
      delete (ReorderConf*)instruction;
      break;
    case SEND_STATS:
#ifdef EDEBUG
      if (edebug & MEMS)
	{
	  vmem("Made an Instruction -- SendStats", NO, memsize = sizeof(SendStats));
	}
      if (edebug & MESSAGES)
	dlog << "\nSend_Stats : " << _sysmsg->mtext << " from pid " << pid;
#endif
      instruction = new SendStats(_sysmsg->mtext, pid, itype);
      delete (SendStats*)instruction;
      break;
    case SYNC_CONF:
#ifdef EDEBUG
      if (edebug & MEMS)
	{
	  vmem("Made an Instruction -- SyncConf", NO, memsize = sizeof(SyncConf));
	}
      if (edebug & MESSAGES)
	dlog << "\nSync_Conf : " << _sysmsg->mtext << " from pid " << pid;
#endif
      instruction = new SyncConf(_sysmsg->mtext, pid, itype, len);
      delete (SyncConf*)instruction;
      break;
    case UID_EXIST:
#ifdef EDEBUG
      if (edebug & MEMS)
	{
	  vmem("Made an Instruction -- UidExist", NO, memsize = sizeof(UidExist));
	}
      if (edebug & MESSAGES)
	dlog << "\nUidExist : " << _sysmsg->mtext << " from pid " << pid;
#endif
      instruction = new UidExist(_sysmsg->mtext, pid, itype);
      delete (UidExist*)instruction;
      break;
    case UP_PRIORITY:
#ifdef EDEBUG
      if (edebug & MESSAGES)
	dlog << "\nUpPriority : " << _sysmsg->mtext << " from pid " << pid;
#endif
      if (commands_since_priority_change > 5)
	{
	  instruction = new UpPriority(_sysmsg->mtext, pid, itype);
	  delete (UpPriority*)instruction;
#ifdef EDEBUG
	  if (edebug & MEMS)
	    {
	      vmem("Made an Instruction -- UpPriority", NO, memsize = sizeof(UpPriority));
	    }
#endif
	  commands_since_priority_change = 0;
	}
      else
	{
#ifdef EDEBUG
	  if (edebug & FLOWER)
	    {
	      strcpy(debugger,"echo UP_PRIORITY instruction ignored.");
	      doutput(debugger);
	    }
	  if (edebug & MESSAGES)
	    dlog << "\nUpPriority instruction ignored.";
#endif
	}
      break;
    case WHO_DROP:
#ifdef EDEBUG
      if (edebug & MEMS)
	{
	  vmem("Made an Instruction -- WhoDrop", NO, memsize = sizeof(WhoDrop));
	}
      if (edebug & MESSAGES)
	dlog << "\nWho_Drop : " << _sysmsg->mtext << " from pid " << pid;
#endif
      instruction = new WhoDrop(_sysmsg->mtext, pid, itype);
      delete (WhoDrop*)instruction;
      break;
    case WHO_IS:
#ifdef EDEBUG
      if (edebug & MEMS)
	{
	  vmem("Made an Instruction -- WhoIs", NO, memsize = sizeof(WhoIs));
	}
      if (edebug & MESSAGES)
	dlog << "\nWho_Is : " << _sysmsg->mtext << " from pid " << pid;
#endif
      instruction = new WhoIs(_sysmsg->mtext, pid, itype);
      delete (WhoIs*)instruction;
      break;
    case WHO_NUM:
#ifdef EDEBUG
      if (edebug & MEMS)
	{
	  vmem("Made an Instruction -- WhoNum", NO, memsize = sizeof(WhoNum));
	}
      if (edebug & MESSAGES)
	{
	  dlog << "\nWho_Num : " << _sysmsg->mtext << " from pid " << pid;
	  //	  _qlist.check();
	}
#endif
      instruction = new WhoNum(_sysmsg->mtext, pid, itype);
      delete (WhoNum*)instruction;
      break;
    case WHO_SIGNED:
#ifdef EDEBUG
      if (edebug & MEMS)
	{
	  vmem("Made an Instruction -- WhoSigned", NO, memsize = sizeof(WhoSigned));
	}
      if (edebug & MESSAGES)
	{
	  dlog << "\nWho_Signed : ";
	  dlog << _sysmsg->mtext;
	  dlog << " from pid " << pid;
	}
#endif
      instruction = new WhoSigned(_sysmsg->mtext, pid, itype);
      delete (WhoSigned*)instruction;
      break;    case WHO_SYNC:
#ifdef EDEBUG
      if (edebug & MEMS)
	{
	  vmem("Made an Instruction -- WhoSync", NO, memsize = sizeof(WhoSync));
	}
      if (edebug & MESSAGES)
	dlog << "\nWho_Sync : " << _sysmsg->mtext << " from pid " << pid;
#endif
      instruction = new WhoSync(_sysmsg->mtext, pid, itype);
      delete (WhoSync*)instruction;
      break;
    case WHO_VOTED:
#ifdef EDEBUG
      if (edebug & MEMS)
	{
	  vmem("Made an Instruction -- WhoVoted", NO, memsize = sizeof(WhoVoted));
	}
      if (edebug & MESSAGES)
	{
	  dlog << "\nWho_Voted : ";
	  dlog << _sysmsg->mtext;
	  dlog << " from pid " << pid;
	}
#endif
      instruction = new WhoVoted(_sysmsg->mtext, pid, itype);
      delete (WhoVoted*)instruction;
      break;
    case WHOS_IN:
#ifdef EDEBUG
      if (edebug & MEMS)
	{
	  vmem("Made an Instruction -- WhosIn", NO, memsize = sizeof(WhosIn));
	}
      if (edebug & MESSAGES)
	dlog << "\nWhosIn : " << _sysmsg->mtext << " from pid " << pid;
#endif
      instruction = new WhosIn(_sysmsg->mtext, pid, itype);
      delete (WhosIn*)instruction;
      break;
    default:
      clerklog << "Unrecognized message:" << _sysmsg->mtext << "from pid " 
	       << pid;
    }
#ifdef EDEBUG
  if (edebug & LIMITS)
    {
      //      if (_qlist.check() != OK)
      //	dlog << "Instruction " << itype 
      //	     << ": after new, before delete failed queue check.";
      //      (*dlogger).flush();
    }
#endif
if (itype == QUIT || itype == NEW_EXE)
  return STOP;
return OK;
}			
//  **************************************************
OKorNOT
InQ::send_myself(char * msg, long priority)
{
  int len;
  (void)sprintf(qlist._sysmsg->mtext,"%s", msg);
  len = (int)strlen(qlist._sysmsg->mtext);
  sprintf(&(qlist._sysmsg->mtext[len]), " %5d ", getpid());
  return qlist.send_msg(priority, _msgqid, len + 8);
}

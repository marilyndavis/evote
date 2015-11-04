/* $Id: ipc_msg.c,v 1.10 2003/10/20 17:07:34 marilyndavis Exp $ */ 
/**********************************************************
 *	  ../eVote/src/Clerklib/ipc_msg.c  -
 *    IPC message calls to the and from The Clerk
 *********************************************************
 *    Copyright (c) 1994...2015 Deliberate.com
 *		Patented.
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
#include <stdio.h>
#include "Clerkdef.h"
#include "Clerk.h"
#include "msgdef.h"
#include "ipc_msg.h"
#include <errno.h>
#include <time.h>
#include <pwd.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <signal.h>
#define MSG_LEN 2048  /* this length gets replaced at runtime
			 with msglen + sizeof(long) */
extern char version[];
void drop_info(void);  /* in ipc_shm.c */
char * pw_name(int uid); /* to share with ipc_shm.c */
void report_mems(int this_Clerk_pid, int other_Clerk_pid); /* ipc_shm.c */
void stop_mems(int this_Clerk_pid, int other_Clerk_pid); /*ipc_shm.c */
extern int* p_shmids;
static int* p_msqids = NULL;
int get_id(int ** p_ids);
static int receive_qid = -1;
static key_t clerk_key = (key_t)63221L; /* this key should be higher than
					   any process id can be in 
					   the system */
extern int time_out;
void call_ipcs(void);
#ifdef EDEBUG
/************************************************************
 *  These are debugging tools.
 *  FLOW is #defined in msgdef.h and is a file for both sides
 *  of IPC to put in messages so that you can study the flow
 *  of messages.  FLOWER determines what is put into the file
 *  and the various values of FLOWER are #defined in 
 *  ../Clerk/evotedef.h.  output puts the string to the FLOW
 *  file
 *  itype is instruction-type; rtype is return-type -- messages
 *  to and from The Clerk.
 *  get_itype and get_rtype return strings representing the message
 *  debugger is some space for concocting debug messages */
void output(char *);
ITYPE itype;
RTYPE rtype;	  
char * get_itype(ITYPE);
char *get_rtype(RTYPE rtype);
char debugger[2*MSG_LEN];
int edebug = 0; /* 8 for flow output */
YESorNO trace = NO;
#define FLOWER 8  
#define STRESS 1
/*  
 *  in_value is called from eVote_demo if there is  -% 8 in the args */
void 
in_value(int set_at)
{
  edebug = set_at;
}
#else
void 
in_value(int set_at)
{
}
#endif
struct msgbuf *in_msg_buffer;
struct msgbuf *out_msg_buffer;
int extra_space = 8;
static unsigned long evote_uid = 0;
static int last_len_sent;
#define TRY (5)            /*  patience factor - message
			       sends are retried this many times */
/* here */
void report_qs(int *this_Clerk_pid, int *other_Clerk_pid);
void stop_qs(int *this_Clerk_pid, int *other_Clerk_pid);
/* in ../eVoteui/defaults.c */
extern int msgmax;
extern int msgtql;
/********************************************************
 *  looks for DEL_XXX return types.  If we have one,
 *  it deletes the queue and changes the return
 *  type to be XXX.
 *********************************************************/
OKorNOT 
check_del_return(void)
{
  struct msqid_ds mbuf;
  int trouble = 0;
  /*  sleep(1); to slow down and let the Clerk get ahead when testing */
  switch (in_msg_buffer->mtype)
    {
    case DEL_ON_TWICE:
      sscanf(in_msg_buffer->mtext, FEN_ON_TWICE, ENN_ON_TWICE);
      if (trouble == 0)  /* trouble == 0 if this process should die */
	{              /* if trouble > 0, it should wait  */
	  drop_info();
	}
    case DEL_DONE:   /*  These are sent when the queue is */
    case DEL_GOOD:   /*  going away after this message and */
    case DEL_NEW_VOTER: 
    case DEL_NO_CONF:   /* the receive_qid needs to find a new   */
    case DEL_NO_VOTER:  /* queue for the next instr, if   */
    case DEL_NOT_GOOD:  /* there is one.   Then, mtype is */
    case DEL_ON_LINE:   /* incremented to indicate the    */
    case DEL_REDUNDANT: /* rest of the message.          */
    case DEL_STRING_OUT:
    case DEL_UID_LIST:
#ifdef EDEBUG
      if (edebug & FLOWER)
	{
	  sprintf(debugger,
		  "echo %lu:Appli: deleting qid %d, pid %d rtype = %s:",
		  who_am_i(), receive_qid, getpid(), 
		  get_rtype((RTYPE)in_msg_buffer->mtype));
	  output(debugger);
	}
#endif
      if (msgctl(receive_qid, IPC_RMID, &mbuf) == -1)
	{
	  switch (errno)
	    {
	    case EINVAL:  /* queue removed by The Clerk */
	      break;
	    default:
#ifdef EDEBUG
	      if (edebug & FLOWER)
		{
		  int er = errno;
		  sprintf(debugger,
			  "echo %lu:Appli: FAILED deleting qid %d, pid %d rtype = %s: errno = %d",
			  who_am_i(), receive_qid, getpid(), 
			  get_rtype((RTYPE)in_msg_buffer->mtype), er);
		  output(debugger);
		}
#endif
	      return NOT_OK;
	    }
	}
      receive_qid = -1;
      in_msg_buffer->mtype++;
      break;
    default:
      break;
    }
  if (in_msg_buffer->mtype == ON_TWICE && trouble == 0)
    exit(0);
  return OK;
}
/*****************************************************************
 *  Looks for a message in the return queue and collects it
 *  in the message buffer. Returns the number of bytes
 *  collected, or -1 on error.  
 *  it waits for a message if wait == YES.  If NO, it returns
 *  immediately.
 *  The return queue, whose queue id is receive_qid, is set up 
 *  by the Clerk.
 *******************************************************************/	
int 
get_msg(YESorNO wait)
{
  int bytes;
  int times = 0;
  int resends = 0;
  YESorNO erdone = NO;
  int drop_stats(char *input);
  OKorNOT get_myq(void);
  void new_memory(char* input);
  void new_vstatus(char* input);  /* in eVotesta.c */
  OKorNOT redo_stats(void);
  OKorNOT send_inst(int len, YESorNO grab_msg);
  /*  sleep(1); to slow down for testing the Clerk */
  times = 0;
#ifdef EDEBUG  
  if (edebug & FLOWER)
    {
      sprintf(debugger,
	      "echo %lu:Appli: get_msg called on qid %d, pid %d. %s",
	      who_am_i(), receive_qid, getpid(), wait ? "wait": "don't wait");
      output(debugger);
    }
#endif 
  if (get_myq() == NOT_OK)
    {
#ifdef EDEBUG
      if (edebug & FLOWER)
	{
	  sprintf(debugger,
		  "echo %lu:Appli: rcv can't get queue on qid %d, pid %d. Returning -1 at start.",
		  who_am_i(), receive_qid, getpid());
	  output(debugger);
	}
#endif 
      return -1;
    }  
  do  /* go around until we find the message we were looking for */
    {	
#ifdef EDEBUG
      if (edebug & FLOWER)
	{
	  sscanf(out_msg_buffer->mtext,"%3d", (int*)&itype);
	  sprintf(debugger,
		  "echo %lu:Appli: rcv loop called on qid %d, pid %d. Out message, maybe old, is %s:%s",
		  who_am_i(), receive_qid, getpid(), get_itype(itype),
		  out_msg_buffer->mtext);
	  output(debugger);
	}
#endif 
      while ((bytes = msgrcv(receive_qid, in_msg_buffer, 
			     msgmax, 0, (wait? 0 :IPC_NOWAIT)))
	     == -1 && ++times < TRY)
	{	
	  erdone = NO;
#ifdef EDEBUG
	  if (edebug & FLOWER)
	    {
	      sprintf(debugger,
		      "echo %lu:Appli: rcv loop msgrcv got -1 qid %d, pid %d. ",
		      who_am_i(), receive_qid, getpid());
	      output(debugger);
	    }
#endif 
	  switch (errno)
	    {
	    case EINTR:  /* happens when alarm sounds while waiting */
#ifdef EDEBUG
	      if (edebug & FLOWER)
		{
		  sscanf(out_msg_buffer->mtext,"%3d", (int*)&itype);
		  sprintf(debugger,
			  "echo %lu:Appli: rcv loop on qid %d, pid %d. itype = %s: ALARM! returning -1",
			  who_am_i(), receive_qid, getpid(), get_itype(itype));
		  output(debugger);
		}
#endif 
	      fprintf(stderr,"\nTimed out after %d seconds of waiting for a response from The Clerk. \n", time_out);
	      return -1;
	      break;
	    case ENOMSG:  /* happens when IPC_NOWAIT and no msg */
#ifdef EDEBUG
	      if (edebug & FLOWER)
		{
		  sprintf(debugger,
			  "echo %lu:Appli: rcv found nothing on qid %d, pid %d. returning 0",
			  who_am_i(), receive_qid, getpid());
		  output(debugger);
		}
#endif 
	      return 0;
	      break;
	    case EINVAL: /* queue removed between this call and msgget */
	      fprintf(stderr,"\nQueue for %d, # %d removed between this call and msgget",
		      getpid(), receive_qid);
	      erdone = YES;
	    case EIDRM:  /* queue removed while waiting for msg */	
	      if (erdone == NO)
		fprintf(stderr,"\nQueue for %d, # %d removed while waiting for msg",
			getpid(), receive_qid);
	      fprintf(stderr,"\nRestarting queue in get_msg");
#ifdef EDEBUG
	      if (edebug & FLOWER)
		{
		  sprintf(debugger,
			  "echo %lu:Appli: rcv found no queue on qid %d, pid %d. : restarting queue.",
			  who_am_i(), receive_qid, getpid());
		  output(debugger);
		}
#endif 
	      if (get_myq() == NOT_OK)
		{
#ifdef EDEBUG
		  if (edebug & FLOWER)
		    {
		      sprintf(debugger,
			      "echo %lu:Appli: rcv can't get queue on qid %d, pid %d. Returning -1",
			      who_am_i(), receive_qid, getpid());
		      output(debugger);
		    }
#endif 
		  return -1;			
		}
	      break;
	    case EACCES:
	      fprintf(stderr,"\nPermission denied.");
#ifdef EDEBUG
	      if (edebug & FLOWER)
		{
		  sprintf(debugger,
			  "echo %lu:Appli: rcv failed on qid %d, pid %d. Permission denied",
			  who_am_i(), receive_qid, getpid());
		  output(debugger);
		}
#endif 
	      break;
	    default:
	      fprintf(stderr,"\nerrno = %d", errno);
#ifdef EDEBUG
	      if (edebug & FLOWER)
		{
		  sprintf(debugger,
			  "echo %lu:Appli: rcv failed on qid %d, pid %d. Errno = %d",
			  who_am_i(), receive_qid, getpid(), errno);
		  output(debugger);
		}
#endif 
	      break;
	    }
	}
      if (bytes == -1)
	{
	  fprintf(stderr,"\neVote:get_msg: Can't receive message.\n");
#ifdef EDEBUG
	  if (edebug & FLOWER)
	    {
	      sprintf(debugger,
		      "echo %lu:Appli: rcv giving up on qid %d, pid %d. returning -1",
		      who_am_i(), receive_qid, getpid());
	      output(debugger);
	    }
#endif 
	  return -1;
	}
      /*				in_msg_buffer->mtext[bytes]='\0'; */
#ifdef EDEBUG
      if (edebug & FLOWER)
	{
	  static RTYPE last_rtype = (RTYPE)0;
	  sprintf(debugger,
		  "echo %lu:Appli: rcv %d bytes on qid %d, pid %d rtype = %s:",
		  who_am_i(), bytes, receive_qid, getpid(), 
		  get_rtype((RTYPE)in_msg_buffer->mtype));
	  if (in_msg_buffer->mtype != NEW_STAT
	     && in_msg_buffer->mtype != NEW_STATS
	     && in_msg_buffer->mtype != VIOLATES_SUM
	     && in_msg_buffer->mtype != MORE_STATS)
	    strcat(debugger, in_msg_buffer->mtext);
	  if (last_rtype == NEW_MID && in_msg_buffer->mtype == NEW_MID)
	    {
	      strcat(debugger," two NEW_MID's in a row!");
	    }
	  output(debugger);
	  last_rtype = (RTYPE)in_msg_buffer->mtype;
	}
#endif 
      /*  Here we check for a problem and unsolicited messages.  
	  If we find an unsolicited message,
	  we do what it asks and then go around for the message
	  we were really looking for. */
      switch (in_msg_buffer->mtype)
	{
	case PROGRAMMER_ERROR:
	  fprintf(stderr, "\nUnexpected call into Clerk.\nProbably progammer error on a user that has not entered.\n");
	  return -1;
	  break;
	case REDO_STATS:
	  if (redo_stats() != OK)
	    fprintf(stderr,"\nNo system resources for new stats!");
	  break;
	case DROP_STATS:
	  drop_stats(in_msg_buffer->mtext);
	  break;
	case NEW_MID:  /* this is tricky because new_memory() */
	  {            /* overwrites the out_msg_buffer and   */
	    /* this could be called with something */
	    /* important in there.                 */
	    static char save_out_msg[MSG_LEN + 1];
	    int i, j;
#ifdef EDEBUG
	    if (edebug & FLOWER)
	      {
		sprintf(debugger,
			"echo %lu:Appli: on qid %d, pid %d NEW_MID case",
			who_am_i(), receive_qid, getpid());
		output(debugger);
	      }
#endif 
	    for (i=0; i < MSG_LEN 
		  && (out_msg_buffer->mtext[i-2] != 'Q'
		      || out_msg_buffer->mtext[i-1] != '!'); 
		i++)
	      {
		save_out_msg[i] = out_msg_buffer->mtext[i];
	      }
#ifdef EDEBUG
	    if (edebug & FLOWER)
	      {
		sprintf(debugger,
			"echo %lu:Appli: on qid %d, pid %d NEW_MID saved: %.*s",
			who_am_i(), receive_qid, getpid(), i,
			save_out_msg);
		output(debugger);
	      }
#endif 
	    new_memory(in_msg_buffer->mtext);
	    for (j=0; j < i; j++)
	      {
		out_msg_buffer->mtext[j] = save_out_msg[j];
	      }
#ifdef EDEBUG
	    if (edebug & FLOWER)
	      {
		sprintf(debugger,
			"echo %lu:Appli: on qid %d, pid %d NEW_MID restored: %.*s",
			who_am_i(), receive_qid, getpid(), i,
			out_msg_buffer->mtext);
		output(debugger);
	      }
#endif 
	  }
	  break;
	case NEW_VSTATUS:
	  new_vstatus(in_msg_buffer->mtext);
	  break;
	case RESEND:
	  if (resends > 4)
	    {
#ifdef EDEBUG
	      if (edebug & FLOWER)
		{
		  sprintf(debugger,
			  "echo %lu:Appli: rcv too many resends on qid %d, pid %d. Returning -1.",
			  who_am_i(), receive_qid, getpid());
		  output(debugger);
		}
#endif 
	      return -1;
	    }
#ifdef EDEBUG
	  if (edebug & FLOWER)
	    {
	      sprintf(debugger,
		      "echo %lu:Appli: rcv sleeping and resending qid %d, pid %d. ",
		      who_am_i(), receive_qid, getpid());
	      output(debugger);
	    }
#endif 
	  sleep(resends++);
	  send_inst(last_len_sent, NO);
	  break;
	default:
	  break;
	}
    }
  while (in_msg_buffer->mtype == REDO_STATS 
	|| in_msg_buffer->mtype == RESEND
	|| in_msg_buffer->mtype == NEW_MID
	|| in_msg_buffer->mtype == NEW_VSTATUS
	|| in_msg_buffer->mtype == DROP_STATS);
  if (check_del_return() != OK)
    {
#ifdef EDEBUG
      if (edebug & FLOWER)
	{
	  sprintf(debugger,
		  "echo %lu:Appli: rcv giving up after failed check_del_return qid %d, pid %d. ",
		  who_am_i(), receive_qid, getpid());
	  output(debugger);
	}
#endif 
      return -1;
    }
#ifdef EDEBUG
  if (edebug & FLOWER)
    {
      sprintf(debugger,
	      "echo %lu:Appli: rcv returning %d bytes on qid %d, pid %d. ",
	      who_am_i(), bytes, receive_qid, getpid());
      output(debugger);
    }
#endif 
  return bytes;
}
/*************************************************************/
OKorNOT 
get_myq(void)
{
  int cc;
  int times = 0;
  struct msqid_ds ctlbuf;
  if (receive_qid <= 0 )
    {
      while ((receive_qid = msgget((key_t)getpid(), IPC_CREAT|0600)) == -1
	    && errno == ENOSPC && ++times < TRY)
	{  /* too many queues on system - wait for eVote to clear*/
	  sleep(1); 
	}
#ifdef EDEBUG
      if (edebug & FLOWER)
	{
	  sprintf(debugger,
		  "echo %lu:Appli: Made qid %d, pid %d: ",
		  who_am_i(), receive_qid, getpid());
	  output(debugger);
	}
#endif
      if (receive_qid == -1)
	{
	  fprintf(stderr,"\neVote:getmsg: Can't open queue to The Clerk.");
	  fprintf(stderr,"\n              Please call support.\n");
	  if (times == TRY)
	    fprintf(stderr,"\nToo many queues on the system.\nTry later.\n");
	  return NOT_OK;
	}
      if ((cc = msgctl(receive_qid, IPC_STAT, &ctlbuf)) == -1)
	{
	  fprintf(stderr,"\neVote:msgctl: Can't inspect my queue from The Clerk: %d.",
		  errno);
	  return NOT_OK;
	}
      if (ctlbuf.msg_perm.cuid == evote_uid)
	return OK;
      ctlbuf.msg_perm.uid = evote_uid;	
      if ((cc = msgctl(receive_qid, IPC_SET, &ctlbuf)) == -1)
	{
	  fprintf(stderr,"\neVote:msgctl: Can't fix uid on queue to The Clerk:%d.",
		  errno);
	  return NOT_OK;
	}
#ifdef EDEBUG
      if (edebug & FLOWER)
	{
	  sprintf(debugger,
		  "echo %lu:Appli: Changed owner on qid %d, to %d: ",
		  who_am_i(), receive_qid, ctlbuf.msg_perm.uid);
	  output(debugger);
	}
#endif
    }
  return OK;
}
/************************************************************
 *     Reads the eVote_defaults file for the desired ipcs
 *     call and executes it.
 *************************************************************/
void 
call_ipcs(void)
{
  char * str;
  int i;
  char *arg[10];
  int no_args = 1;
  str = find_default("IPCS", NO, NO);
  arg[0] = str;
  no_args = 1;
  for (i = 0; str[i]; i++)
    {
      if (str[i] == ' ' || str[i] == '\t')
	{
	  str[i] = '\0';
	  for (i++; str[i]; i++)
	    {
	      if (str[i] != ' ' && str[i] != '\t')
		{
		  arg[no_args++] = &str[i];
		  break;
		}
	    }
	  i--;
	}
    }
  arg[no_args] = NULL;
  execvp("ipcs", arg);
  fprintf(stderr,"\nexecv failed on %d:\n ipcs ", errno);
  for (i = 0; i < no_args; i++)
    fprintf(stderr,"%s ", arg[i]);
  fflush(stderr);
}
/*******************************************************
 *     Returns a pointer to the message in the 
 *     input message buffer.
 *     Called from ../demo/eVoteui/queries.c
 ********************************************************/
char * 
message(void)
{
  return in_msg_buffer->mtext;
}
/************************************************************/
char *
pw_name(int uid)
{
  struct passwd * pw;
  pw = getpwuid(uid);
  return pw->pw_name;
}
/*****************************************************************
 *      sends the message buffer to the eVote dbs
 *      if len == 0, it calculates the len by expected there to be 
 *      a "Q!" at the end.
 *      if get_return == YES, it waits for a return message
 *      and answers NOT_OK if the message is not = DONE
 *      If the message comes back as a RESEND, it resends it.
 *      This can only happen when grouped items get put into the
 *      system, and only when 2 users are adding grouped items
 *      simultaneously.  The one that gets the first one there
 *      first, forces the others to resend so they will be all
 *      bunched together later.
 ****************************************************************/	
OKorNOT 
send_inst(int len, YESorNO get_return)
{
  static int send_qid = -1;
  int cc;
  YESorNO clerk_slow = NO;
  char mail_msg[400];
  int slow_times = 0;
  int tries = 0;
  struct msqid_ds mbuf;

#ifdef EDEBUG
  if (edebug & FLOWER)
    {
      sscanf(out_msg_buffer->mtext, "%3d", (int*)&itype);
      sprintf(debugger,
	      "echo %lu:Appli: send_inst called on qid %d, from pid %d itype = %s",
	      who_am_i(), send_qid, getpid(), get_itype(itype));
      output(debugger);
    }
#endif					
  if (receive_qid > 0)
    get_msg(NO);  /* look for unsolicited messages first */
  if (send_qid < 0)
    {
      if ((send_qid = msgget(clerk_key, 0)) < 0) 
	{
	  sprintf(mail_msg,
		  "mail %s -s \"The Clerk is down!!\" << EOF"
		  "\neVote:  No message queue to Clerk!"
		  "\n\n\tProbably the Clerk needs starting.\n"
		  "\n        Please contact support.\nEOF",
		  find_default("EVOTE_MAIL_TO", NO,NO));
	  system(mail_msg);
	  return PROBLEM;
	}
    }
  do
    {
      if (len == 0)
	{
	  len = -1;
	  do
	    {
	      while (out_msg_buffer->mtext[++len] != 'Q')
		{
		  if (len >= msgmax )
		    {
		      int i;
		      fprintf(stderr,"\neVote:send_inst: No end to message!\n");
		      fprintf(stderr,"\n  message goes:\n");
		      for (i = 0; i < msgmax; i++)
			fprintf(stderr,"%c", out_msg_buffer->mtext[i]);
		      return NOT_OK;
		    }
		}
	    }
	  while (out_msg_buffer->mtext[len+1] != '!');
	  len += 2;
	}
      if (len >= msgmax)
	{
	  fprintf(stderr,"\nMessage too long: %d", len);
	  return NOT_OK;
	}
      sprintf(out_msg_buffer->mtext+len, " %5d ", getpid());
      last_len_sent = len;
      tries = 0;
      do
	{
	  if ((cc = msgctl(send_qid, IPC_STAT, &mbuf)) == -1)
	    {
	      perror("\neVote:msgctl: Can't inspect queue to eVote.\n");
	      return NOT_OK;
	    }
	  if (mbuf.msg_qnum > msgtql - 10)
	    {
	      clerk_slow = YES;
	      sleep(1);
	    }
	}
      while (mbuf.msg_qnum > msgtql - 10 && ++tries < TRY );
      if (mbuf.msg_qnum > msgtql - 10)
	{
	  fprintf(stderr,"\neVote: Too many requests on queue:%lu\n",
		  who_am_i());
	  return NOT_OK;
	}
      evote_uid =  mbuf.msg_perm.cuid;
#ifdef EDEBUG
      if (edebug & FLOWER)
	{
	  sscanf(out_msg_buffer->mtext,"%3d", (int*)&itype);
	  sprintf(debugger,
		  "echo %lu:Appli: about to snd on qid %d, from pid %d itype = %s len = %d: %s",
		  who_am_i(), send_qid, getpid(), get_itype(itype), 
		  len + extra_space, out_msg_buffer->mtext);
	  output(debugger);
	}
#endif					
      tries = 0;
      do
	{
	  if ((cc = msgsnd(send_qid, out_msg_buffer, len + extra_space, 0 /*IPC_NOWAIT*/)) 
	      == -1)
	    {
	      switch (errno) 
		{
		case EACCES:
		  fprintf(stderr,"\nNo permission to send.");
		  break;
		case EINVAL:
		  fprintf(stderr,"\nmsg qid not valid or mytpe < 1 or size too big.");
		  break;
		case EAGAIN:
		  sprintf(mail_msg,"mail %s -s "
			  "\"Clerk message queue is full!\""
			  "<< EOF\nBad news.\nEOF",
			  find_default("EVOTE_MAIL_TO", NO,NO));
		  /* system(mail_msg); */
		  fprintf(stderr,"\neVote:Message queue is full.");
		  fprintf(stderr,"\n      Please notify support.\n");
		  len = msgctl(send_qid, IPC_STAT, &mbuf);
		  fprintf(stderr,"\n%ld messages currently on queue.", 
			  (long)mbuf.msg_qnum);
		  fprintf(stderr,"\n%ld bytes currently on queue.", 
			  (long)mbuf.msg_cbytes);
		  fprintf(stderr,"\n%ld bytes allowed on queue.", 
			  (long)mbuf.msg_qbytes);
		  fprintf(stderr,"\nLast receive at %s", ctime(&mbuf.msg_rtime));
		  fprintf(stderr,"\nLast send at %s", ctime(&mbuf.msg_stime));
		  break;
		case EFAULT:
		  fprintf(stderr,"\nPoints to illegal address.");
		  break;
		default:
		  fprintf(stderr,"\nUnknown error: %d.", errno);
		  break;
		}
	      return NOT_OK;
	    }
#ifdef EDEBUG
	  if (edebug & FLOWER)
	    {
	      sscanf(out_msg_buffer->mtext,"%3d", (int*)&itype);
	      sprintf(debugger,
		      "echo %lu:Appli: snd on qid %d, pid %d itype = %s: %s",
		      who_am_i(), send_qid, getpid(), get_itype(itype), 
		      out_msg_buffer->mtext);
	      output(debugger);
	    }
#endif					
	  if (clerk_slow == YES)
	    {
	      out_msg_buffer->mtype = PR_UP_PRIORITY;
	      sprintf(out_msg_buffer->mtext, FNE_UP_PRIORITY, NNE_UP_PRIORITY);
	    }
	}
      while (clerk_slow == YES && ++slow_times < 2);
      if (get_return == NO)
	{
	  if ((ITYPE)atoi(out_msg_buffer->mtext) == LEAVING ||
	      (ITYPE)atoi(out_msg_buffer->mtext) == LEAVE_ADMIN)
	    {
	      in_msg_buffer->mtype = DEL_DONE;  /* flag to drop queue */
	      check_del_return();
	    }
	  else
	    return OK;
	}
      else if ((cc = get_msg(YES)) == -1)
	return NOT_OK;
    }
  while (in_msg_buffer->mtype == RESEND && ++tries < TRY);
  return OK;
}
/*******************************************************
 *    Assumes the input message buffer has a list of
 *    uids and votes.  It iterates through the list
 *    returning the vote report and placing the uid
 *    in p_uid.
 *    When it reaches the end of the list, it sets up
 *    for the next call and it returns NULL.
 *    Called by process_who in ../demo/eVoteui/queries.c
 ********************************************************/
char * 
uid_report(unsigned long *p_uid)
{
  int j;
  static char *pt;
  static char *buf;
  int len;
  static char vote_str[] = "starting  ";
  if (strcmp(vote_str,"starting  ") == 0)
    {
      len = strlen(in_msg_buffer->mtext) +1;
      if ((buf = malloc(len)) == NULL)
	{
	  fprintf(stderr,"\neVote can't find the necessary resources to allocate.");
	  *p_uid = 0L;
	  return NULL;
	}
      strcpy(buf, in_msg_buffer->mtext);
      while (in_msg_buffer->mtype == UID_LIST_MORE)
	{
	  get_msg(YES);
	  if ((buf = realloc(buf, len += strlen(in_msg_buffer->mtext)))
	     == NULL)
	    {
	      fprintf(stderr,"\neVote can't find the necessary resources to allocate.");
	      *p_uid = 0;
	      return NULL;
	    }
	  strcat(buf, in_msg_buffer->mtext);
	}
      pt = buf;
    }
  if (*pt != '\0')
    {
      sscanf(pt, FEN_WHO_VOTED, p_uid, vote_str); 
      while (*pt == ' ')
	pt++;
      for (j=0; j < 2; j++)
	{
	  while (*pt != ' ')
	    pt++;
	  while (*pt == ' ')
	    pt++;
	}
      return vote_str;
    }
  else /* no more entries in the list */
    {
      free(buf);
      strcpy(vote_str,"starting  ");
      *p_uid = 1;
      return NULL;
    }
}
/*****************************************************************
 *    These next functions are for eVote.c.  They
 *    are called from ../demo/eVoteui/command.c
 *****************************************************************/
#define MAX_LEN 100
struct msqid_ds stat_buf;
int in_qid;
/**************************************************************
 *     if verbose == YES, it fprints a report.
 *     returns OK if the Clerk is up and running.
 *		returns NOT_OK if there is no Clerk.
 *		returns UNDECIDED if it is not sure.
 *****************************************************************/
OKorNOT 
report_Clerk(YESorNO verbose)
{
  int pid = 0;
  YESorNO used_yet = YES;
  if ((in_qid = msgget(clerk_key, 0)) < 0)
    {
      if (verbose == YES)
	printf("\neVote_Clerk is not running. In-queue is not available.");
      return NOT_OK;		
    }
  if (msgctl(in_qid, IPC_STAT, &stat_buf) == -1)
    {
      if (verbose == YES)
	{
	  printf("\nFound The Clerk's in-queue %d but failed msgctl call.",
		 in_qid);
	  printf("\nRun as root or clerk to check The Clerk's process.\n");
	}
      return UNDECIDED;
    }
  if (verbose == YES)
    printf("\nThe Clerk's in_queue is alive. In-queue id is  %d", in_qid);
  if ((pid = stat_buf.msg_lrpid) == 0)
    {
      used_yet = NO;
      if (say_hello() != OK)
	{		  
	  if (verbose == YES)
	    printf("\nThere is a zombie in-queue.  Run 'eVote stop_ipc' first.\n");
	  return NOT_OK;
	}				
      if (msgctl(in_qid, IPC_STAT, &stat_buf) == -1)
	{
	  if (verbose == YES)
	    printf("\nFound The Clerk's in-queue %d but failed msgctl call.",
		   in_qid);
	  return NOT_OK;
	}
      if ((pid = stat_buf.msg_lrpid) == 0)
	{
	  if (verbose == YES)
	    {
	      printf("\nSomething is wrong with The Clerk.\n");
	    }
	  return NOT_OK;
	}
    }
  if (kill(pid, 0) == -1)
    {
      switch (errno)
	{
	case ESRCH:  /* dead!  */
	  if (verbose == YES)
	    printf("\neVote_Clerk is not running now.");
	  return NOT_OK;
	  break;
	case EPERM:  /* alive! */
	  break;
	}
    }
  /* else alive */
  if (verbose == YES)
    {
      printf("\neVote_Clerk is up and running; pid = %d.", pid);
      if (used_yet == NO)
	{
	  printf("\nHowever, it has not been used yet.\n");
	}
      else
	{
	  printf("\nLast receive at %s", ctime(&stat_buf.msg_rtime));
	  printf("Last send at %s", ctime(&stat_buf.msg_stime));
	  printf("Currently %ld messages on the in-queue.\n", 
		 (long)stat_buf.msg_qnum);
	}
    }
  return OK;
}
/*****************************************************************
 *     Prints a report to stdout about message queues and
 *     shared memory segments.
 *****************************************************************/
void 
report_ipc(void)
{
  int this_Clerk_pid = 0;
  int other_Clerk_pid = 0;
  report_qs(&this_Clerk_pid, &other_Clerk_pid);
  report_mems(this_Clerk_pid, other_Clerk_pid);
}
/******************************************************
 *   Finds all the active queues and gives a report on them.
 *********************************************************/
void 
report_qs(int *this_Clerk_pid, int *other_Clerk_pid)
{
  int cc;
  int count = 0;
  int msqid;

  /*  printf("\n\nChecking out-queues...");*/
  while ((msqid = get_id(&p_msqids)) != -1)
    {
      if ((cc = msgctl(msqid, IPC_STAT, &stat_buf)) == -1)	
	{
	  if (errno == EPERM)
	    {
	      printf("\nYou don't have permission, run as root or clerk.\n");
	      exit(0);
	    }
	  continue;
	}
      if (msqid == in_qid)
	{
	  continue;
	  printf("\n\nFound The Clerk's input queue.\n");
	  *this_Clerk_pid = stat_buf.msg_lrpid;
	  count--;
	}
      else
	if (stat_buf.msg_perm.key > 32000)
	  {
	    printf("\n\nFound an input message queue for another Clerk!\n");
	    *other_Clerk_pid = stat_buf.msg_lrpid;
	    count--;
	  }
      printf("\n\nChecking queue %d, key %d",
	     msqid, stat_buf.msg_perm.key);
      printf("\n  created by user %d = %s, group %d, perm = %o:",
	     stat_buf.msg_perm.cuid, pw_name(stat_buf.msg_perm.cuid),
	     stat_buf.msg_perm.cgid, stat_buf.msg_perm.mode);
      printf("\n  Last send: by process %d on %s", stat_buf.msg_lspid,
	     ctime(&stat_buf.msg_stime)); 
      printf("  Last recv: by process %d on %s", stat_buf.msg_lrpid,
	     ctime(&stat_buf.msg_rtime));
      printf("  Last used on: %s  %ld messages left on queue.",
	     ctime(&stat_buf.msg_ctime), (long)stat_buf.msg_qnum);
      count++;
    }
  /*  printf("\n\nFound %d out-queue%s\n", count, count == 1? "." : "s.");*/
}
/*********************************************************
 *   Allocates space for message buffers.  Checks that
 *   The Clerk hasn't expired.
 *********************************************************/
void 
start_up(YESorNO verbose)
{
  void get_version(void);
  char *str;
  str = find_default("EVOTE_MSG_KEY", NO, verbose);
  if (str[0] != '\0')
    clerk_key = (key_t)atol(str);
  msgmax = atoi(find_default("MSGMAX", NO, verbose));
  msgtql = atoi(find_default("MSGTQL", NO, verbose));
  /*  time_out = atoi(find_default("TIME_OUT", NO, verbose)); */
#ifdef EXP  
  if (NUMBER == 0)  /* demo expires */
    {
      time_t now;
      struct tm * p_time;
      time(&now);
      p_time = localtime(&now);
      if (p_time->tm_year >= EXP + 1)
	{
	  fprintf(stderr,"\nThis Clerk %s Demo expired over a year ago and won't run any more.\nPlease contact Deliberate.com:  office@deliberate.com\n\n", 
		  VERSION);
	  exit(0);
	}
      else if (p_time->tm_year >= EXP)
	{
	  fprintf(stderr,"\nThis Clerk %s Demo has expired.\nPlease contact Deliberate.Com:  office@deliberate.com\n", VERSION);
	}
    }
#endif
  if (in_msg_buffer == NULL)
    in_msg_buffer = 
      malloc(sizeof(long) + msgmax);
  if (out_msg_buffer == NULL)
    out_msg_buffer = 
      malloc(sizeof(long) + msgmax);
  get_version();
}
/*********************************************************
 *   Stuffs the version string.
 *********************************************************/
void 
get_version(void)
{
  if (NUMBER == 0)
    {
      sprintf(version,"Clerk %s Demo - No Ser.No.", VERSION);
    }
  else
    {
      sprintf(version,"Clerk %s Ser.No. %d", VERSION, NUMBER);
    }
  return;
}
/*********************************************************
 *   Stops message queues unless they belong to another 
 *   Clerk.  Then it stops shared memory segments unless
 *   they belong to another Clerk.
 *********************************************************/
void 
stop_ipc(void)
{
  int this_Clerk_pid = 0;
  int other_Clerk_pid = 0;
  stop_qs(&this_Clerk_pid, &other_Clerk_pid);
  stop_mems(this_Clerk_pid, other_Clerk_pid);
}
/*********************************************************
 *   Slow function to stop all the queues in the system.
 *   Let me know if you know a faster way.
 *   If it discovers another  Clerk, it doesn't kill it's
 *   queues and it returns the pid of the other Clerk by
 *   reference.
 *********************************************************/
void 
stop_qs(int* this_Clerk_pid, int* other_Clerk_pid)
{
  int i, cc;
  int count = 0;
  ITYPE itype;
  int msqid;
  int get_msqid(void);
  struct 
  {
    long mtype;
    char mtext[MAX_LEN];
  } msg_buf;
  *other_Clerk_pid = -1;
  while ((msqid = get_id(&p_msqids)) != -1)
    {
      if ((cc = msgctl(msqid, IPC_STAT, &stat_buf)) == -1)	
	{
	  continue;
	}
      if (*other_Clerk_pid == -1 
	  && stat_buf.msg_perm.key != clerk_key
	  && stat_buf.msg_perm.key > 33000
	  && stat_buf.msg_lrpid != 0)  /* ever used? */
	{
	  *other_Clerk_pid = stat_buf.msg_lrpid;
	  continue;
	}
      if (stat_buf.msg_perm.key == clerk_key)
	{
	  *this_Clerk_pid = stat_buf.msg_lrpid;
	}
     /* See if the last receive pid is alive */
      if (stat_buf.msg_lrpid > 0 &&
	 kill(stat_buf.msg_lrpid, 0) == -1)
	/* should fail because pid is dead
	   or because there's no permission
	   0  means don't really do it, just check */
	{		
	  switch (errno)
	    {
	    case ESRCH:  /* dead pid -- fall through */
	      break;
	    case EPERM: /* pid still alive -- only kill it
			   if it doesn't belong to other Clerk */
	      if (*other_Clerk_pid != stat_buf.msg_lrpid)
		{
		  break;
		}
	      continue;
	      break;
	    default:
	      printf("\nOdd return from kill call in stop_qs(): %d.\n",
		     errno);
	      break;
	    }
	}
      else /* pid still alive */
	{
	  if (*this_Clerk_pid == stat_buf.msg_lrpid
	     || *this_Clerk_pid == stat_buf.msg_lspid)
	    {
	      /* do nothing here, fall through */
	    }
	  else
	    {
	      continue;
	    }
	}
      printf("\nRemoving queue %d, key %d",
	     msqid, stat_buf.msg_perm.key);
      if (msqid == in_qid)
	printf(".  This is the eVote_Clerk's in-queue.");
      printf("\n  created by user %d = %s, group %d, perm = %o:",
	     stat_buf.msg_perm.cuid, pw_name(stat_buf.msg_perm.cuid),
	     stat_buf.msg_perm.cgid, stat_buf.msg_perm.mode);
      printf("\n  Last send: by process %d on %s", stat_buf.msg_lspid,
	     ctime(&stat_buf.msg_stime)); 
      printf("  Last recv: by process %d on %s", stat_buf.msg_lrpid,
	     ctime(&stat_buf.msg_rtime));
      printf("  Last used on: %s  %ld messages left on queue:",
	     ctime(&stat_buf.msg_ctime), (long)stat_buf.msg_qnum);
      if (stat_buf.msg_qnum > 0)
	printf("\nRemoving messages:");
      for (i = 0; i < stat_buf.msg_qnum; i++)
	{ 
	  cc = msgrcv(msqid, (struct msgbuf*)&msg_buf, 
		      MAX_LEN, 0, MSG_NOERROR ||
		      IPC_NOWAIT);  
	  if (msqid == in_qid)
	    {
	      sscanf(msg_buf.mtext, "%3d", (int*)&itype);
	      printf("\n    #%d: ITYPE = %s, %d bytes, %s",
		     i+1, get_itype((ITYPE)itype), cc, msg_buf.mtext);
	    }
	  else
	    {
	      printf("\n    #%d: RTYPE = %s, %d bytes, %s",
		     i+1, get_rtype((RTYPE)msg_buf.mtype), cc, msg_buf.mtext);
	    }
	}
      cc = msgctl(msqid, IPC_RMID, NULL);
      if (cc == -1)
	{
	  printf("\nError removing queue %d.", msqid);
	  switch (errno)
	    {
	    case EINVAL:
	      printf("\nThe msqid %d is not a valid q identifier.", msqid);
	      break;
	    case EPERM:
	      printf("\nYou don't have permission, run as super user.");
	      exit(0);
	      break;
	    case EFAULT:
	      printf("\nNeeds better than a NULL as last argument.");
	      break;
	    }
	}
      else
	count++;
    }
  if (count == 1)
    printf("\n\nRemoved 1 queue.\n\n");
  else if (count > 0)
    printf("\n\nRemoved %d queues.\n\n", count);
}
/**************************************************/
int 
get_id(int **p_ids)
{
  static int offset = 0;
  OKorNOT collect_ipc(void);
  if (*p_ids == NULL)
    {
      if (collect_ipc() != OK)
	{
	  fprintf(stderr, "\n\nNo facilities to inspect ipc facilities.");
	  fprintf(stderr, "\nTry adding\n"
		  "\tIPCS = ipcs -c"
		  "\nto your EVOTE_BIN/eVote.cf file.\n"
		  "\nThe ipcs call listed there should produce \"msgqid\""
		  "\non the left margin of the report like this:.\n"
		  "\n------ Message Queues --------"
		  "\nmsqid     owner     perms     used-bytes  messages    "
		  "\n0         clerk     666       0           0           \n");
	  exit(0);
	}
    }
  if (*(*p_ids + offset) == -1)
    {
      offset = 0;
      return -1;
    }
  return *(*p_ids + offset++);
}
/**************************************************/
OKorNOT 
collect_ipc(void)
{
  char line[100];
  int pfd[2];
  OKorNOT collect_ids(int ** pp_ids);
  int done = 0;
  if (pipe(pfd) == -1)
    {
      fprintf(stderr, "\nCan't establish pipe.\n");
      return NOT_OK;
    }
  switch (fork())
    {
    case -1:
      fprintf(stderr,"\nCan't fork a new process.\n");
      return NOT_OK;
    case 0:  /* child */
      if (close(1) == -1)  /* close stdout */
	{
	  fprintf(stderr,"\nCan't close stdout of ipcs process.\n");
	  return NOT_OK;
	}
      if (dup(pfd[1]) != 1) /* attach stdout to write of pipe */
	{
	  fprintf(stderr,"\nCan't attach ipcs output to pipe.\n");
	  return NOT_OK;
	}
      close(pfd[0]);  /* close extra descriptors */
      close(pfd[1]);
      call_ipcs();
      return NOT_OK;
    }
  /* parent */
  if (close(pfd[1]) == -1)
    {
      fprintf(stderr,"\nCan't close writing end for ipcs.\n");
      return NOT_OK;
    }
  if (close(0) == -1)  /* close stdin of parent */
    {
      fprintf(stderr,"\nCan't close stdin for ipcs.\n");
      return NOT_OK;
    }
  if (dup(pfd[0]) != 0)  /* attach reading end to stdin */
    {
      fprintf(stderr,"\nCan't attach pipe to stdin.\n");
      return NOT_OK;
    }
  while (fgets(line, 100, stdin) != NULL)
    {
#ifdef linux
      if (strncmp(line,"msqid", 5) == 0)
	{
#endif
#ifdef __FreeBSD__
	  if (strncmp(line,"Message Queues:", 15) == 0)
	    {
	      fgets(line, 100, stdin);
#endif
	  if (collect_ids(&p_msqids) != OK)
	    return NOT_OK;
	  done++;
	  continue;
	}
#ifdef linux
      if (strncmp(line,"shmid", 5) == 0)
	{
#endif
#ifdef __FreeBSD__
	if (strncmp(line,"Shared Memory:", 14) == 0)
	{
	  fgets(line, 100, stdin);	  
#endif
	  if (collect_ids(&p_shmids) != OK)
	    return NOT_OK;
	  done++;
	  continue;
	}
    }
  if (done != 2)
    {
      fprintf(stderr,"\n$EVOTE_HOME_DIR/eVote/src/Clerklib/ipc_msg.c: "
	      "\n     Returning badly from get_ids\n");
      return NOT_OK;
    }
  return OK;
}
/**************************************************/
OKorNOT 
collect_ids(int ** pp_ids)
{
  int i = -1;
  int len = 1;
  int ch;
  char buf[12];
#ifdef __FreeBSD__
  YESorNO bsd_done = NO;
#endif
  if ((*pp_ids = malloc(sizeof(int) * len)) == NULL)
    return NOT_OK;
  **pp_ids = -1;
  while ((ch = getchar()) != EOF)
    {
      if (ch == ' ' || ch == '\t')
	{
	  buf[++i] = '\0';
	  i = -1;
	  if ((*pp_ids = realloc(*pp_ids, sizeof(int) * ++len))
	     == NULL)
	    return NOT_OK;
	  *(*pp_ids + len - 2) = atoi(buf);
	  *(*pp_ids + len - 1) = -1;
	  while (((ch = getchar()) != EOF)   /* dump line */
		&& ch != '\n' && ch != '\r')
	    ;
#ifdef __FreeBSD__
	  bsd_done = NO;
#endif	 
	  continue;
	}
#ifdef __FreeBSD__
      if ((ch == 'q' || ch == 'm') && bsd_done == NO)
	{
	  getchar();  /* collect the space */
	  bsd_done = YES;
	  continue;
	}
#endif
      if (ch < '0' || ch > '9')
	return OK;
      buf[++i] = ch;
    }
  return NOT_OK;
}
/**************************************************/
#ifdef EDEBUG
void 
output(char * command)
{
  int i = -1;
  static char starter[80];
  if (!starter[0])
    {
      sprintf(starter,"touch %s", FLOW);
      system(starter);
      sprintf(starter,"chmod o+w %s", FLOW);
      system(starter);
    }
  while (command[++i] != '\0')
    {
      switch (command[i])
	{
	case ':':
	  /*	  colon = i; */
	case '>':
	case '<':
	case '(':
	case ')':
	case '"':
	case '\'':
	case ';':
	  command[i] = '-';
	  /*	  command[colon + 6] = '\0';
		  command[i+1] = '\0'; */
	  break;
	default:
	  break;
	}
    }
  strcat(command, " >> ");
  strcat(command, FLOW);
  system(command);
}
#endif
#include"mtype.c"  /* This is get_itype() and get_rtype().  These are
		      shared with the Clerk */

/* $Id: maint.c,v 1.4 2003/01/15 18:54:08 marilyndavis Exp $ */ 
/**********************************************************
 *	 ../eVote/src/Clerklib/maint.c  -
 *   These functions maintain The Clerk, starting, stopping,
 *   changing the priority, checking, flushing the log, turning
 *   on a debug trace, ...
 *   Makes calls to The Clerk.  
 *********************************************************
 **********************************************************
 *    These functions write to stdout with printf's.
 **********************************************************/
#include "msgdef.h"
#include "Clerkdef.h"
#include "Clerk.h"
#include "ipc_msg.h"
#include <signal.h>
#include <unistd.h>
#include<stdio.h>
extern int time_out;
/***************************************************************
 *    Asks The Clerk to become a lower priority process.
 ***************************************************************/
OKorNOT 
down_priority(void)
{
  out_msg_buffer->mtype = PR_DOWN_PRIORITY;
  sprintf(out_msg_buffer->mtext, FNE_DOWN_PRIORITY, NNE_DOWN_PRIORITY);
  return send_inst(0, NO);
}
/***************************************************************
 *    Prepares the data for a new eVote_Clerk executable.
 *    It reorders all the conferences so that there are no
 *    gaps in the data and it brings down The Clerk.
 ***************************************************************/
OKorNOT 
new_exe(void)
{
  out_msg_buffer->mtype = PR_NEW_EXE;
  sprintf(out_msg_buffer->mtext, FNE_NEW_EXE, NNE_NEW_EXE);
  return send_inst(0, NO);
}
/***************************************************************
 *     Prods eVote_Clerk to flush the log file, make a backup
 *     called eVote_log.back.  If there is already an 
 *     eVote_log.back, it appends the old data in eVote_log.back
 *     to eVote_log.old so that eVote_log.back only contains the
 *     lastest log messages.
 *     Clerk.log starts empty again.
 ***************************************************************/
OKorNOT 
new_log(void)
{
  out_msg_buffer->mtype = PR_NEW_LOG;
  sprintf(out_msg_buffer->mtext, FNE_NEW_LOG, NNE_NEW_LOG);
  return send_inst(0, NO);
}
/***************************************************************
 * turns on debugging in The Clerk
 ************************************************************/
OKorNOT 
send_debug(int level)
{
  out_msg_buffer->mtype = PR_DO_DEBUG;
  sprintf(out_msg_buffer->mtext, FNE_DO_DEBUG, NNE_DO_DEBUG);
  return send_inst(0, NO);
}
/***************************************************************
 *  flushes The Clerk log.
 ***************************************************************/
OKorNOT 
send_flush(void)
{
  out_msg_buffer->mtype = PR_FLUSH;
  sprintf(out_msg_buffer->mtext, FNE_FLUSH, NNE_FLUSH);
  return send_inst(0, NO);
}
/***************************************************************
 *    Sends a HELLO message to The Clerk.  If all is well,
 *    it also receives a GOOD message back.
 ***************************************************************/
OKorNOT 
say_hello(void)
{
  OKorNOT cc = NOT_OK;
  unsigned old_alarm;
  int hello_alarm;
  void catch_signal(int);
  void (*old_fn)(int);
  hello_alarm = time_out ? time_out : 5;
  out_msg_buffer->mtype = PR_HELLO;
  sprintf(out_msg_buffer->mtext, FNE_HELLO, NNE_HELLO);
  in_msg_buffer->mtype = NOT_GOOD;
  old_fn = signal(SIGALRM, catch_signal);
  old_alarm = alarm(hello_alarm);
  /*    for (i = 0; i < 4000000000; i++); */
  cc = send_inst(0, YES); 
  signal(SIGALRM, old_fn);
  alarm(old_alarm);
  if (cc != OK)
    {
      if (cc != PROBLEM)
	fprintf(stderr,"\nHello to Clerk timed out after %d seconds.\n", time_out);
      return NOT_OK;
    }
  if (in_msg_buffer->mtype != GOOD)
    return NOT_OK;
  return OK;
}
void 
catch_signal(int signo)
{
  switch (signo)
    {
    case SIGALRM:
      break;
    default:
      fprintf(stderr,"\ncatch_signal: Signal %d caught! Can't happen.\n", signo);
      break;
    }
}
/**********************************************************
 * Brings down The Clerk
 **************************************************************/
OKorNOT 
send_quit(void)
{
  out_msg_buffer->mtype = PR_QUIT;
  sprintf(out_msg_buffer->mtext, FNE_QUIT, NNE_QUIT);
  return send_inst(0, NO);
}
/***************************************************************
 *    Asks The Clerk to become a higher priority process.
 ***************************************************************/
OKorNOT 
up_priority(void)
{
  out_msg_buffer->mtype = PR_UP_PRIORITY;
  sprintf(out_msg_buffer->mtext, FNE_UP_PRIORITY, NNE_UP_PRIORITY);
  return send_inst(0, NO);
}

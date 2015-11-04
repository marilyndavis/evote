/* $Id: noq.cc,v 1.3 2003/01/15 18:54:07 marilyndavis Exp $ */ 
// noq.cc - Implements instructions that don't need a reply
/*********************************************************
 **********************************************************/
#include"evotedef.h"
#include"instruct.h"
#include"qlist.h"
#include"conflist.h"
#include"conf.h"
extern QList qlist;
extern ConfList conferences;
extern time_t now;
#include <fstream.h>
extern ofstream *logger;
extern ostream *dlogger;
extern int edebug;
extern time_t now;
char * uid_string(unsigned long uid);
extern char * crash_command;
//  *********************************************************
DoDebug::DoDebug(char *input, int pid, ITYPE itype)
  :Instruction(input, pid, itype)
{
  int level;
  sscanf(input, FNE_DO_DEBUG, NEE_DO_DEBUG);
  void start_debug(YESorNO app);  
#ifdef EDEBUG			
  clerklog << "DoDebug message received from user " << uid_string(_uid)
	   << " on pid " << pid << ". Debug level = %d" << level;
  edebug = level;
  start_debug(NO);
#else
  clerklog << "DoDebug message received from user " << uid_string(_uid)
	   << " on pid " << pid << ";  but debugging is not available.";
#endif
  (*logger).flush();
}
// *********************************************************
DownPriority::DownPriority(char *input, int pid, ITYPE itype) 
  :Instruction(input, pid, itype)
{
  int new_nice;
  sscanf(input, FNE_DOWN_PRIORITY, NEE_DOWN_PRIORITY);
  new_nice = reset_nice(-1);
  clerklog << "DownPriority message received from user " 
	   << uid_string(_uid)	<< " on pid " << pid << ".  Priority now "
	   << new_nice;
  (*logger).flush();
}
// ***********************************************************
DropOldQs::DropOldQs (char *input, int pid, ITYPE itype) 
  :Instruction(input, pid, itype)
{
  sscanf(input, FNE_DROP_OLDQS, NEE_DROP_OLDQS);
  if (_uid != 0)
    clerklog << "Drop_oldqs called by uid " << uid_string(_uid);
  qlist.drop_oldqs();
}	
//  *********************************************************
Flush::Flush(char *input, int pid, ITYPE itype) 
  :Instruction(input, pid, itype)
{
  sscanf(input, FNE_FLUSH, NEE_FLUSH);
  //  while (1)  // for testing the crash_command
  //    {
  //      *input++ = 'x';
  //    }
  //
  clerklog << "Flush message received from user " << uid_string(_uid)
	   << " on pid " << pid;
  (*logger).flush();
#ifdef EDEBUG
  if (edebug > 0)
    (*dlogger).flush();
#endif
}
// ***************************************************************
//     Increases the ballot size of the conf for more items.
GrowConf::GrowConf(char *input, int pid, ITYPE itype):Instruction(input, pid, itype)
{
  if (_status != OK)
    return;
  _p_conf->grow_ballot();
}
//  *********************************************************
NewExe::NewExe(char *input, int pid, ITYPE itype) 
  :Instruction(input, pid, itype)
{
  sscanf(input, FNE_NEW_EXE, NEE_NEW_EXE);
  clerklog << "NewExe message received from user " << uid_string(_uid)
	   << " on pid " << pid;
  conferences.new_exe();
}
//  *********************************************************
NewLog::NewLog(char *input, int pid, ITYPE itype)
  :Instruction(input, pid, itype)
{
  void new_log(unsigned long uid);  /* in main */
  sscanf(input, FNE_NEW_LOG, NEE_NEW_LOG);
  clerklog << "NewLog message received from user " << uid_string(_uid)
	   << " on pid " << pid;
  new_log(_uid);
}
//  *********************************************************
Quit::Quit(char *input, int pid, ITYPE itype)
  :Instruction(input, pid, itype)
{
  sscanf(input, FNE_QUIT, NEE_QUIT);
  crash_command = "";  /* suppress the crash command */
  clerklog << "Quit message received from user " << uid_string(_uid)
	   << " on pid " << pid;
  // the action for quitting is handled in main.  InQ::deliver
  // passes a STOP back to main from this instruction.
}
//  *****************************************************
ReorderConf::ReorderConf(char *input, int pid, ITYPE itype)
  :Instruction(input, pid, itype)
{
  if (_status != OK)
    return;
  _p_conf->reorder();
}
// *********************************************************
UpPriority::UpPriority(char *input, int pid, ITYPE itype)
  :Instruction(input, pid, itype)
{
  int new_nice;
  sscanf(input, FNE_UP_PRIORITY, NEE_UP_PRIORITY);
  new_nice = reset_nice(1);
  clerklog << "UpPriority message received from user " << uid_string(_uid)
	   << " on pid " << pid << ".  Priority now " << new_nice;
  (*logger).flush();
}

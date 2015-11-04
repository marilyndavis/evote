/* $Id: main.cc,v 1.5 2003/01/24 22:15:19 marilyndavis Exp $ */ 
/************************************************************
 *   main.cc  -- main function and other startup and maintenance
 *               functions.
 ************************************************************
 *    Copyright (c) 1994...2015 Deliberate.com Patented.
 *    by Marilyn Davis
 **********************************************************/
extern "C" {
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <signal.h>
}
#include <iostream.h>
#include <fstream.h>
#include <strstream.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/shm.h>
#include <unistd.h>
#include <sys/stat.h>
#include "evotedef.h"
#include "conflist.h"
#include "qlist.h"
#include "inq.h"
#include "wholist.h"
#include "memlist.h"
extern MemList memlist;
#include<errno.h>
static char log_file_name[PATH_LEN + MAX_FILE_NAME_LEN + 5];
GLOBAL_DECS
YESorNO stop = NO;  // for signal catching
void catch_signal(int signo);
char *crash_command = "";
char *key_str = "";
char *whereami = "";
void giveup(void);
static int starting_nice;
//  ********************************************************************
//   main(int argc, char* argv[])
//    called by
//    eVote_Clerk -l log_file_path -d evote_data_dir &
//          -x n  to debug level n.
//    See evotedef.h for possible debug levels.
//    or you can just type
//    eVote
//    and accept the #defined (in evotedef.h) place for files
int new_stuff = 20;
YESorNO sync_who = NO;  /* available to WhoList::sync() and
			   BallotBox::try_dropping_voters() */
int reset_nice(int how);
int 
main(int argc, char *argv[])
{
  YESorNO wait = NO;
  short times_qempty;
  YESorNO just_adjourned = NO;
  int nice_minus_20;
  void set_signals();
  void start_up(int, char**);
  set_signals();    	
  atexit(giveup);
  umask(UMASK_VALUE);
  start_up(argc, argv);
  //  Tune this algorithm so that eVote spends the right amount
  //  of time in grooming, inq.deliver, qlist.drop_oldqs and waiting.
  while (1)
    {
      if (stop == YES)
	exit(0);
      switch (inq.deliver(wait, key_str))
	{
	case STOP:
	  exit(0);
	  break;
	case OK:				// processed an instruction
	  new_stuff++;
	  times_qempty = 0;
	  just_adjourned = NO;
	  wait = NO;
	  break;
	case UNDECIDED:  // didn't find an instruction
	  if (new_stuff >= 10 && conferences.groom() == NO)
	    {
	      new_stuff = 0;
	    }
	  else if (new_stuff < 10)
	    {
	      switch (++times_qempty)
		{
		case 1:
		  qlist.drop_oldqs();
		  break;
		case 2:
		  memlist.drop_old_segs();
		  break;
		case 3:  // lower priority if things are quiet
		  if ((nice_minus_20 = nice(0)) == -1  && errno == EPERM)
		    break;
		  if (nice_minus_20 + 20 >= starting_nice)
		    break;
		  reset_nice(1);
		  break;
		default:
		  if (sync_who)
		    {
		      if (wholist.sync() != UNDECIDED)
			sync_who = NO;
		    }
		  else if (just_adjourned == NO)
		    {
		      conferences.adjourn_some(NULL);
		      just_adjourned = YES;
		    }
		  else
		    {
		      wait = YES;
		    }
		  break;
		} 
	    }
	  break;
	default:
	  break;
	}
    }	
}
// ***************************************************************
//    void start_up   sorts out the command-line args
void 
start_up(int argc, char *argv[])
{
  int i=0;
  void arg_err();
  void fix_slash(char* name);
  char evote_home_dir[PATH_LEN + MAX_FILE_NAME_LEN + 5]; 
#ifdef EDEBUG
  void start_debug(YESorNO append);
#endif
  strcpy(evote_home_dir, EVOTE_HOME_DIR);
  msgmni = MSGMNI;
  msgmax = MSGMAX;
  msgtql = MSGTQL;
  shmmni = SHMMNI;
  starting_nice = 20;
  while (--argc > 0)
    {
      if (*argv[++i] != '-')
	arg_err();
      --argc;
      switch (*(argv[i] + 1))
	{
	case 'C':   // CRASH_COMMAND
	case 'c':   
	  crash_command =  argv[++i];
	  if (strcmp(crash_command, "none") == 0)
	    crash_command = "";
	  break;
	case 'H':   // EVOTE_HOME_DIR
	case 'h':   
	  strcpy(evote_home_dir, argv[++i]);
	  break;
	case 'K':   // EVOTE_MSG_KEY
	case 'k':   
	  key_str = argv[++i];
	  break;
	case 'w':
	case 'W':
	  whereami = argv[++i];
	  break;
	case 'x':    // debug level
	case 'X':
#ifdef EDEBUG
	  if (argv[i+1] == NULL)
	    arg_err();
	  edebug = atoi(argv[++i]);
#else
	  cout << "\nThe Clerk has not been compiled with debugging possible.";
#endif
	  break;
	case 'n':
	case 'N':
	  if (argv[i+1] == NULL)
	    arg_err();
	  msgmni = atoi(argv[++i]);
	  break;
	case 'm':
	case 'M':
	  if (argv[i+1] == NULL)
	    arg_err();
	  msgmax = atoi(argv[++i]);
	  break;
	case 'p':
	case 'P':
	  if (argv[i+1] == NULL)
	    arg_err();
	  starting_nice = atoi(argv[++i]);
	  break;
	case 'i':
	case 'I':
	  if (argv[i+1] == NULL)
	    arg_err();
	  shmmni = atoi(argv[++i]);
	  break;
	case 't':
	case 'T':
	  if (argv[i+1] == NULL)
	    arg_err();
	  msgtql = atoi(argv[++i]);
	  break;
	default:
	  arg_err();
	}
    }
  if (starting_nice != 20)
    {
      if (nice(starting_nice - 20) == -1 && errno == EPERM)
	clerklog << "Unable to set starting nice to " 
		 << starting_nice << ".  Run as root for this feature.";
    }
  fix_slash(evote_home_dir);
  strcpy(output_dir, evote_home_dir);
  strcat(output_dir, "eVote/data/");
#ifdef EDEBUG
  start_debug(NO);
#endif
  strcpy(log_file_name, output_dir);
  strcat(log_file_name, "Clerk.log");
  logger = new ofstream(log_file_name, ios::app);
  if (!logger)
    {
      cout << "\nCan't open " << log_file_name;
      exit(1);
    }
  clerklog << "Started opening log.";
  /*  cout << "\nStarting eVote_Clerk, Copyright 1994-2003 by Deliberate.Com.\n";
  cout << "\n                                                           Patented.\n";
  cout.flush(); */
  conferences.start();
  wholist.start();
  inq.start(key_str);
  qlist.start(msgmax);
}
// ***************************************************************
void 
new_log(unsigned long uid)
{
  char command[100];
  int cc = 0;
  char back_file[PATH_LEN + MAX_FILE_NAME_LEN + 5];
  char old_file[PATH_LEN + MAX_FILE_NAME_LEN + 5];
  fstream checker;
  delete logger;
  strcpy(back_file, log_file_name);
  back_file[strlen(log_file_name)-9] = '\0';
  strcpy(old_file, back_file);
  strcat(back_file,"Clerk_log.back");
  strcat(old_file,"Clerk_log.old");
  checker.open(back_file, ios::in|ios::nocreate);
  if (!checker)
    {
      ;  // do nothing
    }
  else
    {
      checker.close();
      sprintf(command,"cat %s >> %s", back_file, old_file);
      if ((cc = system(command)) == 0)
	{
	  sprintf(command,"rm %s", back_file);
	  cc = system(command);
	}
    }
  sprintf(command,"mv %s %s", log_file_name, back_file);
  if (cc == 0)
    cc = system(command);
  logger = new ofstream(log_file_name, ios::app);
  if (!logger)
    {
      cout << "\nCan't open " << log_file_name;
      exit(1);
    }
  if (cc == -1)
    {
      char msg[80];
      clerklog << "Unable to start new log.  Perhaps too many processes are running.";
      sprintf(msg, FNE_QUIT, QUIT, (unsigned long)0);
      inq.send_myself(msg, 1l);
    }
  else
    clerklog << "  New log started by user id: " << uid << ".";
}
// ************************************************************
// Resets the nice value for the process using the increment
// given in how.
int 
reset_nice(int how)
{
  int nice_minus_20 = nice(how);
  if (nice_minus_20 == -1 && errno == EPERM)
    {
      clerklog << "Trouble setting priority.  The Clerk must run under root to lower the nice value and raise priority.";
      return 9999;
    }
  return nice_minus_20 + 20;
}
// *******************************************************
//      reports error, gives usage and quits.
void 
arg_err(void)
{
  cerr << "\neVote_Clerk: argument error:";
  cerr << "\n   usage:eVote -l log_file_dir -d data_dir &\n";
  exit(1);
}
// *******************************************************
//     Caught signals come here.
void 
catch_signal(int signo)
{
  static int count = 0;
  char *signame;
  YESorNO finish = YES;
  if (++count > 1)
    exit(1);
  signal(signo, catch_signal);
  switch (signo)
    {
    case SIGILL:
      signame = "SIGILL";
      break;
    case SIGTRAP:
      signame = "SIGTRAP";
      break;
    case SIGABRT:
      signame = "SIGABRT";
      break;
#ifdef __FreeBSD__
    case SIGXCPU:
      signame = "SIGXCPU";
      break;
    case SIGXFSZ:
      signame = "SIGXFSZ";
      break;
    case SIGEMT:
      signame = "SIGEMT";
      break;
    case SIGTSTP:
      signame = "SIGTSTP";
      break;
    case SIGTTIN:
      signame = "SIGTTIN";
      break;
    case SIGTTOU:
      signame = "SIGTTOU";
      break;
#endif
    case SIGFPE:
      signame = "SIGFPE";
      break;
    case SIGBUS:
      signame = "SIGBUS";
      finish = NO;
      break;
#ifdef __FreeBSD__
    case SIGKILL:
      signame = "SIGKILL";
      break;
    case SIGUSR1:
      signame = "SIGUSR1";
      break;
    case SIGUSR2:
      signame = "SIGUSR2";
      break;
#endif
    case SIGSEGV:
      signame = "SIGSEGV";
      finish = NO;
      break;
#ifdef __FreeBSD__
    case SIGSYS:
      signame = "SIGSYS";
      break;
    case SIGPIPE:
      signame = "SIGPIPE";
      break;
#endif
    case SIGTERM:
      signame = "SIGTERM";
      break;
#ifdef linux
    case SIGPOLL:
      signame = "SIGPOLL";
      break;
#endif
    case SIGHUP:
      signame = "SIGHUP";
      break;
    case SIGINT:
      signame = "SIGINT";
      break;
    case SIGQUIT:
      signame = "SIGQUIT";
      break;
#ifdef linux
    case SIGPWR:
      signame = "SIGPWR";
      break;
#endif
    }
  clerklog << "Signal #" << signo << ":" << signame 
    << " received. Coming down.";
  cout << "\n eVote_Clerk::Signal #" << signo << ":" << signame 
    << " received. Coming down now.\n";
  if (finish == NO)
    {
      exit(0);
    }
  stop = YES;
}
//  ************************************************************
//  called atexit to give up.
void 
giveup(void)
{
  conferences.store_all();
  memlist.drop_old_segs();
  if (crash_command != NULL && crash_command[0] != '\0')
    system(crash_command);
}
//  ******************************************************
//  Sets every signal in my book to go to catch_signal() 
void 
set_signals(void)
{
  if (signal(SIGINT, SIG_IGN) != SIG_IGN)
    signal(SIGINT, catch_signal);
#ifdef linux
  if (signal(SIGPWR, SIG_IGN) != SIG_IGN)
    signal(SIGPWR, catch_signal);
#endif
  if (signal(SIGQUIT, SIG_IGN) != SIG_IGN)
    signal(SIGQUIT, catch_signal);
  if (signal(SIGHUP, SIG_IGN) != SIG_IGN)
    signal(SIGHUP, catch_signal);
#ifdef linux
  if (signal(SIGPOLL, SIG_IGN) != SIG_IGN)
    signal(SIGPOLL, catch_signal);
#endif
  if (signal(SIGTERM, SIG_IGN) != SIG_IGN)
    signal(SIGTERM, catch_signal);
#ifdef __FreeBSD__
  if (signal(SIGSYS, SIG_IGN) != SIG_IGN)
    signal(SIGSYS, catch_signal);
  if (signal(SIGPIPE, SIG_IGN) != SIG_IGN)
    signal(SIGPIPE, catch_signal);
#endif
  if (signal(SIGSEGV, SIG_IGN) != SIG_IGN)
    signal(SIGSEGV, catch_signal);
  if (signal(SIGBUS, SIG_IGN) != SIG_IGN)
    signal(SIGBUS, catch_signal);
  if (signal(SIGFPE, SIG_IGN) != SIG_IGN)
    signal(SIGFPE, catch_signal);
#ifdef __FreeBSD__
  if (signal(SIGTSTP, SIG_IGN) != SIG_IGN)
    signal(SIGTSTP, catch_signal);
  if (signal(SIGTTIN, SIG_IGN) != SIG_IGN)
    signal(SIGTTIN, catch_signal);
  if (signal(SIGTTOU, SIG_IGN) != SIG_IGN)
    signal(SIGTTOU, catch_signal);
  if (signal(SIGEMT, SIG_IGN) != SIG_IGN)
    signal(SIGEMT, catch_signal);
  if (signal(SIGKILL, SIG_IGN) != SIG_IGN)
    signal(SIGKILL, catch_signal);
  if (signal(SIGUSR1, SIG_IGN) != SIG_IGN)
    signal(SIGUSR1, catch_signal);
  if (signal(SIGUSR2, SIG_IGN) != SIG_IGN)
    signal(SIGUSR2, catch_signal);
#endif
  if (signal(SIGABRT, SIG_IGN) != SIG_IGN)
    signal(SIGABRT, catch_signal);
  if (signal(SIGTRAP, SIG_IGN) != SIG_IGN)
    signal(SIGTRAP, catch_signal);
  if (signal(SIGILL, SIG_IGN) != SIG_IGN)
    signal(SIGILL, catch_signal);
}

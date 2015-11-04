/* $Id: eVote.c,v 1.3 2003/01/15 18:54:08 marilyndavis Exp $ */ 
/****************************************************************
*  eVote.c    
*     This program controls the starting and stopping of eVote.
*     It recognizes:
*         eVote adjourn conf_name - adjourns the conf.  It is
*                          restarted when someone enters again.
*         eVote check [conf_name]   -  recalculates the stats
*                          on the conf, if given.  If not,
*                          it checks that The Clerk is running.
*         eVote drop uid conf_name -  drops the user from the conf.
*         eVote dropc conf_name - drops the conference.
*         eVote flush   -  flushes the Clerk.log file
*         eVote ipc     -  reports statistics about The Clerk's
*			   message queues and shared memory.
*         eVote new_exe -  prepares the data for a new eVote_Clerk
*                          executable and quits.
*         eVote new_log -  flushes, backs-up and restarts Clerk.log
*         eVote priority [up/down] - changes the 'nice' of The Clerk.
*         eVote start   -  starts eVote_Clerk 
*         eVote stop    -  stops eVote_Clerk
*         eVote stop_ipc - stops message queues and shared memory.
*         eVote -v      -  gives a startup trace.
*         eVote    with no arguments also starts The Clerk.  
*	                - if it isn't already running and if the 
*                         user has permission.  If it is running,
*	                  it starts the eVote Demo.
*         eVote [-h eVote_home_dir]
*               If given, this argument will override
*               value in eVote_defaults.  If nothing
*               is in eVote_defaults, the default is
*		/usr/local/eVote/.
*   NOTE:  If you change eVoteui/command.c and change this 
*          documentation be sure to also change arg_err() so
*          that it prints out the right information for users.
*********************************************************

int
main(int argc, char *argv[])
{
  int command(int argc, char *argv[]);
  return command(argc, argv);
}

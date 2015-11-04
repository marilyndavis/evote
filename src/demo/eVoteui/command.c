/* $Id: command.c,v 1.5 2003/01/23 19:54:27 marilyndavis Exp $ */ 
/*********************************************************
 **********************************************************/
#include"../eVote.h"
#include"../../Clerklib/Clerk.h"
#include<sys/types.h>
#include<sys/stat.h>
#include<unistd.h>
#include<pwd.h>
YESorNO debugging = NO;
OKorNOT send_debug(int level);
extern int msgmax;
extern int msgtql;
extern char version[];
/**********************************************************
 *	  ../eVote/src/demo/eVoteui/command.c
 *    This file contains functions for eVote.c, eVote's
 *    command center.  Notice that it is not in eVoteuilib.a.
 *********************************************************
 *   void command(int argc, char *argv[])
 *      main function for eVote.c
 **********************************************************/
YESorNO start_quiet = NO;  /* for silencing the democracy 
			      lecture in the demo. */
YESorNO start_trace = NO;
int time_out = 0;
int
command(int argc, char *argv[])
{
  char path[PATHLEN + 1];
  int i=0;
  int argcc = argc;
  void arg_err();
  char command[200];
  void try_string(int argc, char *argv[], int iarg);
  char extras[40]="";
  char * home_dir = NULL;
  char * key_str = NULL;
  char * msgmni_str = NULL;
  char * msgmax_str = NULL;
  char * msgtql_str = NULL;
  char * shmmni_str = NULL;
  char * starting_nice = NULL;
  char * crash_command = NULL;
  char * whereami = NULL;
  while (++i < argc)
    {
      /* first look for a string */
      if (*argv[i] != '-')
	{
	  if (strcmp(argv[i],"start") == 0)
	    continue;		
	  try_string(argcc, argv, i);
	}
      /* Now look for command line args */
      switch (*(argv[i] + 1))
	{
	case '?':
	  arg_err();	
	  break;
	case '%':
	  debugging = YES;
	  break;
	case 'c':
	case 'C':
	  if (strlen(argv[++i]) >= PATHLEN)
	    {
	      fprintf(stderr,"\nYour crash_command can only be %d characters.\n",
		      PATHLEN);
	      exit(1);
	    }
	  crash_command = argv[i];
	  break;
	case 'k':
        case 'K':
	  key_str = argv[++i];
	  break;
	case 'n':
	case 'N':
	  msgmni_str =argv[++i];
	  break;
	case 'm':
	case 'M':
	  msgmax_str =argv[++i];
	  break;
	case 'p':
	case 'P':
	  starting_nice =argv[++i];
	  break;
	case 's':
	case 'S':
	  start_quiet = YES;
	  break;
	case 't':
	case 'T':
	  msgtql_str =argv[++i];
	  break;
	case 'i':  /* 's' is for silent mode on demo */
	case 'I':
	  shmmni_str =argv[++i];
	  break;
	case 'h':  /* EVOTE_HOME_DIR */
	case 'H':
	  home_dir =argv[++i];
	  break;
	case 'v':
	case 'V':
	  start_trace = YES;
	  break;
	case 'w':
	case 'W':
	  whereami = argv[++i];
	  break;
	case 'x':
	case 'X':
	  /* pass on to eVote_Clerk */
	  strcat(extras," -x ");
	  strcat(extras, argv[++i]);
	  if (atoi(argv[i]) <= 0)
	    arg_err();
	  break;
	default:
	  arg_err();
	}
    }
  /* if we got this far may be asking to start the Clerk*/
  start_up(start_trace);
  if (report_Clerk(NO) != NOT_OK)/* probably running */
    {   
      if (argcc == 1 || (debugging == YES && argcc == 2)
	 || (argcc == 2 && start_quiet == YES))			
				/* no args, start Demo */
	{
	  struct stat sbuf;
	  strcpy(path, eVote_cf_path(start_trace));
	  path[strlen(path)-3] = '\0';
	  strcat(path,"_demo");
	  if (stat(path, &sbuf) == -1)
	    {
	      printf("\neVote is already running!");
	      printf("\nThe eVote Demo is not installed at this site.\n");
	      exit(0);
	    }
	  if (start_quiet == NO)
	    execl(path, "eVote_demo", (char*)0);
	  else
	    execl(path, "eVote_demo", "-s", (char*)0);
	  printf("\nFailed to execute eVote_demo.\n");
	  exit(0);
	}
      printf("\neVote is already running!");
      printf("\nYou must 'eVote quit' first to restart.\n");
      exit(0);
    }
  else 
    if (start_quiet == YES || have_permission(start_trace) == NO)
      {
	printf("\nThe Clerk is not running.  You can not use eVote.");
	printf("\nPlease notify your system administrator.\n");
	exit(0);
      }
  stop_ipc(); /* not running - dump all the old queues */
  /* now we want to start the Clerk for sure */
  if (starting_nice == NULL)
    starting_nice = find_default("STARTING_NICE", YES, start_trace);
  if (home_dir == NULL)
    home_dir = find_default("EVOTE_HOME_DIR", YES, start_trace);
  if (key_str == NULL)
    key_str = find_default("EVOTE_MSG_KEY", YES, start_trace);
  if (msgmni_str == NULL)
    msgmni_str  = find_default("MSGMNI", YES, start_trace);
  if (msgmax_str == NULL)
    msgmax_str = find_default("MSGMAX", YES, start_trace);
  if (msgtql_str == NULL)
    msgtql_str=  find_default("MSGTQL", YES, start_trace);
  if (shmmni_str == NULL)
    shmmni_str=  find_default("SHMMNI", YES, start_trace);
  if (whereami == NULL)
    whereami =  find_default("WHEREAMI", YES, start_trace);
  if (crash_command == NULL)
    {
      crash_command=  find_default("CRASH_COMMAND", YES, start_trace);
      if (crash_command[0] == '\0')
	crash_command = "none";
    }
  if (starting_nice == NULL || home_dir == NULL 
     || key_str == NULL || msgmni_str == NULL
     || msgmax_str == NULL ||	msgtql_str == NULL
     || shmmni_str == NULL ||	crash_command == NULL)
    {
      fprintf(stderr,"\nUnable to malloc space for default values to start eVote_Clerk.\n");
      exit(1);
    }
  strcpy(path, eVote_cf_path(start_trace));
  path[strlen(path)-3] = '\0';
  sprintf(command,
	  "nohup %s_Clerk -h %s -k %s -n %s -m %s -t %s"
	  " -i %s -p %s -w %s -c %s %s & ", 
	  path, home_dir, key_str, msgmni_str, msgmax_str,
	  msgtql_str, shmmni_str, starting_nice, whereami,
	  crash_command, extras);
  if (start_trace)
    printf("Starting the Clerk:\n%s", command);
  fflush(stdout);
  system(command);
  if (debugging == NO)
    {
      int times = 0;
      while (report_Clerk(NO) == NOT_OK && ++times < 5)
	sleep(2);
      if (times < 5)    /* probably running */
	show_opening_screen();
      else  /* probably not */
	printf("\nFailed to start The Clerk.\n");
    }
  exit(0);
}
/**************************************************************
 *   Prints out usage information and quits.
 *******************************************************v******/
void
arg_err(void)
{
  printf("\neVote: unrecognized input.");
  printf("\n       Please enter:");
  printf("\n");
  printf("\n         eVote adjourn conf  - adjourns the conference.");
  printf("\n         eVote check [conf]  -  recalculates conf if present.");
  printf("\n                          If not, it checks the Clerk.");
  printf("\n         eVote drop uid conf - drops the user from the conf.");
  printf("\n         eVote dropc conf - drops the conference.");
  printf("\n         eVote flush   -  flushes the Clerk.log file");
  printf(
	 "\n         eVote ipc   -  reports statistics about the Clerk and ");
  printf(
	 "\n                          its queues and shared memory segments.");
  printf("\n         eVote new_exe - prepares the data for a new eVote_Clerk.");
  printf("\n         eVote new_log -  flushes, backs-up and restarts Clerk.log");
  printf("\n         eVote priority [up/down] prods the Clerk to go faster/slower.");
  printf("\n         eVote start   -  starts eVote_Clerk"); 
  printf("\n         eVote stop    -  stops eVote_Clerk");
  printf("\n         eVote stop_ipc - stops the message queues and shared memory segments.");
  printf("\n");
  printf("\n         eVote    with no arguments also starts the Clerk  ");
  printf("\n                  - if it isn't already running.  If it is run-");
  printf("\n                  ning, it starts the eVote Demo.");
  printf("\n");
  printf("\n         eVote -s starts the eVote Demo in silent mode.");
  printf("\n");
  printf("\n         eVote [-h eVote_home_dir]");
  printf("\n                If given, this argument will override the");
  printf("\n                value in the \"eVote_defaults\" file.  If there");
  printf("\n                is nothing in eVote_defaults, the value is ");
  printf("\n                /usr/local/eVote/\n");
  printf("\n         eVote [-k message_key]"
	 "\n                If given, this argument will override the");
  printf("\n                value in the \"eVote_defaults\" file.  If there");
  printf("\n                is nothing in eVote_defaults, the value is ");
  printf("\n                63221 by default.");
  printf("\n");
  exit(1);
}
/*************************************************************
 *     returns YES if the user has permission to start
 *     eVote_Clerk, NO if not.
 *************************************************************/
YESorNO
have_permission(YESorNO verbose)
{
  int cc;
  struct stat buf;
  char Clerk_exe[PATHLEN + 1];
  struct passwd *pw;
  strcpy(Clerk_exe, eVote_cf_path(start_trace));
  Clerk_exe[strlen(Clerk_exe)-3] = '\0';
  strcat(Clerk_exe,"_Clerk");
  if (verbose)
    printf("Path to Clerk: %s\n", Clerk_exe);
  if ((cc = stat(Clerk_exe, &buf)) == -1)
    {
      printf("\nCould not find path for eVote_Clerk: %s\n", 
	     Clerk_exe);
      return NO;
    }
  if (verbose)
    printf("\nThe Clerk's permissions are %o", buf.st_mode & 0777);
  if (buf.st_mode & 00000001)  /* everyone has perm! */
    {
      if (verbose)
	printf("\nEveryone has permission to start the Clerk!");
      return YES;
    }
  if (buf.st_mode & 00000010 && getgid() == buf.st_gid)
    {
      if (verbose)
	printf("\nYou are in the right group to start the Clerk.");
      return YES;    /* in right group */
    }
  if (buf.st_mode & 00000100 && getuid() == buf.st_uid)
    {
      if (verbose)
	printf("\nYou, %d, are the owner of the Clerk.", buf.st_uid);
      return YES;   /* owner */
    }
  printf("\neVote_Clerk's permissions are %o", buf.st_mode & 0777);
  pw = getpwuid(buf.st_uid);
  printf("\nIt is owned by %s.", pw->pw_name);
  return NO;
}
/************************************************************
 *      Tries to identify strings that the user typed on the
 *      command line. 
 *************************************************************/
void
try_string(int argc, char *argv[], int iarg)
{
  OKorNOT cc = OK;
  YESorNO have_permission();
  void report_ipc();
  OKorNOT adjourn_conf(char *conf_name);
  OKorNOT check_conf(char* conf_name);
  OKorNOT drop_conf(char * conf_name);
  OKorNOT drop_voter(char * conf_name, 
		     unsigned long uid_to_drop);
  char *get_conf_list(YESorNO finished);
  OKorNOT report_Clerk(YESorNO verbose);
  OKorNOT send_flush();
  OKorNOT send_quit();
  OKorNOT new_log();
  OKorNOT Clerk;
  start_up(start_trace);
  if (strcmp(argv[iarg],"version") == 0)
    {
      printf("\neVote Clerk Version %s.\n", version);
      return;
    }
  /* first come the requests that require verbose report_Clerk */
  if (strcmp(argv[iarg],"ipc") == 0 )
    {
      (void)report_Clerk(YES);
      report_ipc();
      exit(0);
    }
  else if (strcmp(argv[iarg],"check") == 0)
    {
      if (argc - 1 == iarg)
	{
	  (void)report_Clerk(YES);
	  printf("\n");
	  exit(0);
	}
      if (strcmp(argv[iarg+1], "all") == 0 
	 || strcmp(argv[iarg+1], "ALL") == 0)
	{
	  char *name;
	  while ((name = get_conf_list(NO)) != NULL)
	    check_conf(name);
	  get_conf_list(YES);
	}
      else
	check_conf(argv[iarg+1]);
      send_flush();
      exit(0);
    }
  Clerk = report_Clerk(start_trace);
  if (strcmp(argv[iarg],"quit") == 0 || strcmp(argv[iarg],"stop") == 0 
      || strcmp(argv[iarg],"new_exe") == 0 
      || strcmp(argv[iarg],"stop_ipc") == 0
      || strcmp(argv[iarg],"debug") == 0
      || strcmp(argv[iarg],"drop") == 0
      || strcmp(argv[iarg],"dropc") == 0
      || strcmp(argv[iarg],"priority") == 0
      || strcmp(argv[iarg],"adjourn") == 0)
    {
      if (have_permission(start_trace) == NO)
	{
	  printf("\nYou do not have permission to %s.\n", argv[iarg]);
	  exit(0);
	}
      if (strcmp(argv[iarg],"stop_ipc") == 0)
	{
	  switch (Clerk)
	    {
	    case OK:
	      printf("\nThe Clerk is alive. Use 'eVote stop'.\n");
	      exit(0);
	      break; 
	    case UNDECIDED:
	    case NOT_OK:
	      stop_ipc();
	      exit(0);
	      break;
	    default:
	      /* impossible */
	      break;
	    }
	}
      if (Clerk == OK || Clerk == UNDECIDED)
	{
	  if (strcmp(argv[iarg],"new_exe") == 0)
	    {
	      cc = new_exe();
	      exit(0);
	    }
	  if (strcmp(argv[iarg], "adjourn") == 0)
	    {
	      if (adjourn_conf(argv[iarg + 1]) == OK)
		printf("\n%s adjourned.\n", argv[iarg + 1]);
	      else
		printf("\nFailed to adjourn %s.\n", argv[iarg + 1]);
	      exit(0);
	    }
	  if (strcmp(argv[iarg], "drop") == 0)
	    {
	      unsigned long uid;
	      if ((uid = (unsigned long) atoi(argv[iarg+1])) == 0)
		{
		  printf("\nCan't understand dropping user %s from %s.\n",
			 argv[iarg+1], argv[iarg+2]);
		  exit(0);
		}
	      if (drop_voter(argv[iarg+2], uid) != OK)
		printf("\nFailed to drop %s from %s.\n",
		       argv[iarg+1], argv[iarg+2]);
	      else
		printf("\nDone.\n");
	      exit(0);
	    }
	  if (strcmp(argv[iarg], "dropc") == 0)
	    {
	      if (drop_conf(argv[iarg + 1]) == OK)
		printf("\n%s dropped.\n", argv[iarg + 1]);
	      else
		printf("\nFailed to drop %s.\n", argv[iarg + 1]);
	      exit(0);
	    }
	  if (strcmp(argv[iarg], "priority") == 0)
	    {
	      if (argc == 2 && (strncmp(argv[iarg+1],"down", 4) == 0))
		down_priority();
	      else
		up_priority();
	      exit(0);
	    }
	  if (strcmp(argv[iarg], "debug") == 0)
	    {
	      if (send_debug(atoi(argv[iarg +1])) == OK)
		printf("\nNow debugging at level %s.\n",
		       argv[iarg + 1]);
	      exit(0);
	    }
	  if (strcmp(argv[iarg], "quit") == 0
	     || strcmp(argv[iarg], "stop") == 0)
	    {
	      int count = 0;
	      do
		{
		  cc = send_quit();
		  if (debugging == NO)
		    sleep(1);
		  Clerk = report_Clerk(NO);   
		}
	      while (Clerk == OK && ++count < 3);  
	      if (count >= 3)
		{
		  stop_ipc();
		}
	    }
	}
      else
	{
	  printf("\neVote_Clerk is not running.\n");
	  /*						stop_ipc();  not running - dump all the old queues */
	}
      exit(0);
    }
  if ((Clerk = report_Clerk(NO)) == NOT_OK)
    {
      (void)report_Clerk(YES);
    }
  else if (Clerk == UNDECIDED)
    {
      printf("\neVote_Clerk process is unused.");
    }
  if (strcmp(argv[iarg],"flush") == 0 )
    {
      if (Clerk != OK)
	printf("\nThere is nothing to flush.\n");
      else
	send_flush();
    }
  else if (strcmp(argv[iarg],"new_log") == 0)
    {
      if (Clerk == NOT_OK)
	printf("\nCan't make a new Clerk.log file.\n");
      else
	new_log();
    }
  else
    arg_err();
  exit(0);
}

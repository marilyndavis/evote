/* $Id: debug.cc,v 1.3 2003/01/15 18:54:07 marilyndavis Exp $ */ 
/*
    debug.cc Some debugging tools are in here.
    If you are having a memory trouble, also check the documentation
    at the top of atd_new.cc
*/
/*********************************************************
 *    Copyright (c) 1994...2015 Deliberate.com Patented.
 *    by Marilyn Davis
 **********************************************************/
#ifdef EDEBUG
#include <fstream.h>
#include<stdio.h>
#include<stdlib.h>
#include"evotedef.h"
#include"qlist.h"
extern QList qlist;
extern char output_dir[];
extern int edebug;
extern ostream *dlogger;
extern char *dfile;
extern int msgmax;
// An effort to check for memory leaks.
void
vmem(char *comment, YESorNO report_free = NO, int size = 0)
{
  char debugger[200];
  int i = -1;
  static long tot_size = 0;
  static long queue_size = 0;
  static long instruct_size = 0;
  
  tot_size += size;
  if (strcmp(comment, "Making new Queue") == 0
     || strcmp(comment, "Deleting Queue") == 0)
    queue_size += size;
  if (strncmp(comment, "Made an Instruction", 19) == 0
     || strcmp(comment,"Deleting Instruction") == 0)
    instruct_size += size;
  for (i = 0; comment[i]; i++)
    {
      if (comment[i] == '\n')
	{
	  comment[i] = '\0';
	  break;
	}
    }
  sprintf(debugger,"echo %s >> %s%s", comment, output_dir, MEM_FILE);
  system(debugger);
  if (size != 0)
    {
      sprintf(debugger, "echo %ld = total, size = %d >> %s%s", 
	      tot_size, size, output_dir, MEM_FILE);
      system(debugger);
    }
  if (report_free == YES)
    {
      sprintf(debugger, "cat /proc/meminfo >> %s%s", output_dir, MEM_FILE);
      //  sprintf(debugger, "free >> %s%s", output_dir, MEM_FILE);
      system(debugger);
      sprintf(debugger, "echo total = %ld, queue_size = %ld, instruct_size = %ld >> %s%s", tot_size, queue_size, instruct_size, output_dir, MEM_FILE);
      system(debugger);
    }
}
// open debug output file
void
start_debug(YESorNO app)
{
  char debugger[200];
  static YESorNO flower_started = NO;
  static YESorNO debug_started = NO;
  if (edebug <= 0)
    return;
  if ((edebug & FLOWER) && flower_started == NO)
    {
      flower_started = YES;
      sprintf(debugger,"rm -f %s", FLOW);
      system(debugger);
      sprintf(debugger,"echo STARTING > %s", FLOW);
      system(debugger);
      sprintf(debugger,"chmod uog+w %s", FLOW);
      system(debugger);
    }
  if (edebug && debug_started == NO)
    {
      debug_started = YES;
      sprintf(debugger, "rm -f %s%s", output_dir, MEM_FILE);
      system(debugger);
      sprintf(debugger,"echo STARTING > %s%s", output_dir, MEM_FILE);
      system(debugger);
      sprintf(debugger,"chmod +w %s%s", output_dir, MEM_FILE);
      system(debugger);
    }
  if (edebug > 0)
    {
      if (!dlogger)
	;
      else
	return;
      char debug_name[PATH_LEN + MAX_FILE_NAME_LEN + 5];
      strcpy(debug_name, output_dir);
      strcat(debug_name, dfile);
      dlogger = new ofstream(debug_name, (app ? ios::app : ios::out));
      if (!dlogger)
	{
	  cout << "\nCan't open " << debug_name;
	  exit(1);
	}
    }
}
// Puts output to the FLOW file
void
doutput(char * command)
{
  int i = 0;  /* 25 */
  int colon;
  static char * msg;
  static unsigned long count;
  count++;  /* for debugger */
  
  if (msg == NULL)
    {
      msg = new char[msgmax + 300];
    }      
  /*  qlist.check(); */
  /*  if (i < strlen(command))
      { */
  while (command[++i] != '\0')
    {
      switch (command[i])
	{
	case ':':
	  colon = i;
	  break;
	case '\n':
	case '\'':
	case '`':
	case '"':
	case '>':
	case '<':
	case '(':
	case ')':
	  command[i] = '-';
	  /*	      command[colon + 1] = '\0';
		      command[i+1] = '\0'; */
	  break;
	default:
	  break;
	}
    }
  if (strncmp(command, "echo", 4) != 0)
    sprintf(msg, "echo %lu: %s >> %s", count, command, FLOW);
  else
    sprintf(msg, "echo %lu: %s >> %s", count, &command[5], FLOW);
  system(msg);
}
#endif

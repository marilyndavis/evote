/* $Id: fence.c,v 1.5 2003/10/17 23:07:50 marilyndavis Exp $ */ 
/*********************************************************
 *   fence.c   -   Can be called by wrapper from the
 *                  sendmail aliases file:
 *    fence: "|/usr/local/wrapper fence more command line"
 *      fence filters out unwanted mail, sends unsubscribe
 *      messages to majordomos sending mail from a list if
 *      that list isn't on the list of goodlists and then
 *      it executes the rest of the command line.
 ************************************************************
 *   To compile, alter the makefile and type:
 *   make
 *   Put the resulting executable in with your majordomo
 *   scripts and you're ready.
 *************************************************************
 *    Copyright (c) 1994...2015 Deliberate.com Patented.
 *    by Marilyn Davis
 **********************************************************/
#include "filter.h"
int
main(int argc, char *argv[], char *env[])
{
  void go_on(int argc, char ** argv, char ** env);
  program = "fence";
  look_for_trouble(argc, argv, env);
  go_on(argc, argv, env);
  exit(0);
}
/**********************************************************
 *    This forks and execs a process:
 *        It execs argv[1] and copies the other arguments
 *        to argv[1]' command line.
 *        The parent's stdout is attached to the child's
 *        stdin.
 ***********************************************************/
void
go_on(int argc, char ** argv, char ** env)
{
  int i;
  char *path;
  char next_arg[100];

  if (fork_is_parent())  
    {
      dump_message(stdout, NO, NO, NO);
      return;
    }
  /* child process */
  if ((path = malloc(strlen(WRAPPER_PATH) + strlen(argv[1]) + 2))
     == NULL)
    {
      alert("Can't malloc space for path.", NO);
    }
  strcpy(path, WRAPPER_PATH);
  if (path[strlen(WRAPPER_PATH) -1] != '/')
    strcat(path,"/");
  strcat(path, argv[1]);
  execve(path, argv+1, env);  
  sprintf(error_msg,"Exec Failed, ERROR = %d Call was \"%s\".  Args were:", errno, path);
  for (i = 2; argv[i] != NULL; i++)
    {
      sprintf(next_arg, " %s", argv[i]);
      strcat(error_msg, next_arg);
    }
  alert("Exec failed!", YES);
}

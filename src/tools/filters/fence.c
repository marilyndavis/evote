/* * eVote - Software for online consensus development.
 * Copyright (C) 2015 Marilyn Davis <marilyn@deliberate.com>
 *
 * This file is part of eVote.
 *
 * eVote is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as
 * published by the Free Software Foundation, either version 3 of the
 * License, or (at your option) any later version.
 *
 * eVote is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with eVote.  If not, see <http://www.gnu.org/licenses/>.
 */

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

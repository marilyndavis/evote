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

/* $Id: ui.c,v 1.3 2003/01/15 18:54:08 marilyndavis Exp $ */ 
/*********************************************************
 *    ui.c 
 *    This file contains startup functions for the Demo
 *    that are not used by the eVote command executable.
 *********************************************************
 **********************************************************/
#include"../eVote.h"
#include"../../Clerklib/Clerk.h"
/*********************************************************
 *   Deciphers the command line arguments for Demo
 ***********************************************************/
void
get_args(int argc, char* argv[], char *this_conf, short* drop_days)
{
  int i = 0;
  void in_value(int x);
  *this_conf = '\0';
  while (--argc > 0)
    {
      /* first look for a string */
      if (*argv[++i] != '-')
	{
	  if (argv[i][0] >= '0' && argv[i][0] <= '9')
	    *drop_days = atoi(argv[i]);
	  else
	    strcpy(this_conf, argv[i]);
	}
      else
	{
	  /* Now look for command line args */
	  switch (*(argv[i] + 1))
	    {
	    case 's':
	      toggle_blurbs();
	      break;
	    case '%':
	      if (*(argv[i] + 2) <= '9' && *(argv[i] + 2) >= '0')
		in_value(atoi(argv[i] + 2));
	      break;
	    default:
	      printf("\neVote_demo: argument error.");
	      printf("\nusage: eVote_demo [conf_name]\n");
	      exit(0);
	      break;
	    }
	}
    }
}
/*******************************************************
 *     Sends and receives a message to The Clerk.  If there
 *     is trouble, it prints an error message and quits.
 ********************************************************/
void
hello_Clerk(void)
{
  if (say_hello() == OK)
    return;
  printf("\neVote is unable to communicate with The Clerk.");
  printf("\nPlease notify your system administrator and try later.\n");
  exit(0);
}

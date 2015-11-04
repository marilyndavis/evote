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

/* $Id: angel.c,v 1.4 2003/01/15 18:54:13 marilyndavis Exp $ */ 
/*********************************************************
 *   angel.c   -   Can be called by wrapper from the
 *                  sendmail aliases file:
 *    angel: "|/usr/local/wrapper angel"
 *    People can .forward a copy of their mail to this by 
 *    putting:
 *    \login, angel@your_side.com
 *    It will unsubscribe them from majordomo email lists 
 *    from which they receive messages unless the email list 
 *    is in the "goodlists" file.
 *********************************************************
 *   owner-angel: you@yoursite.com
 *   angel: "|/usr/local/majordomo/wrapper angel"
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
  program = "angel";
  look_for_trouble(argc, argv, env);
  exit(0);
}

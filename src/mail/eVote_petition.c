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

/* $Id: eVote_petition.c,v 1.4 2003/01/15 18:54:08 marilyndavis Exp $ */ 
/**********************************************************
 *   eVote_petition.c   -   called by wrapper
 *   Controls the handling of an incoming petition signature.  
 *********************************************************
 **********************************************************/
#include<stdio.h>
#include "mailui/mailui.h"
#include<errno.h>
#ifdef TRACE
FILE * fptrace;
#endif
int
main(int argc, char *argv[], char *env[])
{
  int cc1, cc2;
#ifdef TRACE
  fptrace = fopen("./trace.out", "w");
#endif
  time(&now);
  cc2 = parse_cf();
  set_signals(react_to_signal);
  petition = YES;
  cc1 = parse_headers(argc, argv, env);
  if (cc1 | cc2)
    bounce_error(cc1 | cc2);
  translate(the_language, DO_PETITION, NULL, 
	    MAYBE, MAYBE, MAYBE, NULL, NULL, MAYBE);
  exit(0);
}

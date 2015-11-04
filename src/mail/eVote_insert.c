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

/* $Id: eVote_insert.c,v 1.5 2003/01/15 18:54:08 marilyndavis Exp $ */ 
/**********************************************************
 *   eVote_insert.c   -   called by wrapper
 *      Controls the handling of an incoming message.
 *      Checks to see if they are for eVote, if so
 *      processes it.  Otherwise, passes the 
 *      message to majordomo's resend.
 *********************************************************
 **********************************************************/
#include<stdio.h>
#include "mailui/mailui.h"
#include<errno.h>
#include<time.h>

int
main(int argc, char *argv[], char *env[])
{
  int cc1, cc2;
  int cc;

  time(&now);
  cc2 = parse_cf(); /* for time_out */
  /*  set_signals(react_to_signal);*/
  cc1 = parse_headers(argc, argv, env);

  if (cc1 | cc2)
    bounce_error(cc1 | cc2);
  if (argc < 3)  /* a subscription or cancellation request 
		   via the approval address */
    {
      subs(argc, argv);
    }
  if (confirm_this.status == CHECK )
    {
      if (check_confirm() == FOR_UNSUB)
	subs(0, NULL);
    }
  if ((cc = fix_first_token())
      && (same(token, "eVote") || same(token, "eVote:")
	  || same(token, "e-vote")))
    {
      do_eVote(cc);
    }
  /* pass it on to resend */
  subject = original_subject;
  send(LIST);
  dump_message(stdout, NO, NO, NO);
  return 0;
}

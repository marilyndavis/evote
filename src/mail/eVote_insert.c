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

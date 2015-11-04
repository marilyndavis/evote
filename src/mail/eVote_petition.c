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

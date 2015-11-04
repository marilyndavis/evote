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
 *    Copyright (c) 1994...2015 Deliberate.com Patented.
 *    by Marilyn Davis
 **********************************************************/
#include "filter.h"
int
main(int argc, char *argv[], char *env[])
{
  program = "angel";
  look_for_trouble(argc, argv, env);
  exit(0);
}

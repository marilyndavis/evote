/* $Id: puppet.c,v 1.4 2003/01/15 18:54:13 marilyndavis Exp $ */ 
/*********************************************************
 *   puppet.c   -   called by wrapper
 *********************************************************
 *   puppet  -p puppet_address -h hidden_address
 *   Mail coming into this address should have the real destination
 *   address as the first line of the message.  It must come from
 *   the hidden address and then the message will go to the real
 *   destination and look like it comes from the puppet address.
 *********************************************************
 *   To compile, alter the makefile and type:
 *   make
 *   Put the resulting executable in with your majordomo
 *   scripts and you're ready.
 *********************************************************
 *    Copyright (c) 1994...2015 Deliberate.com Patented.
 *    by Marilyn Davis
 **********************************************************/
#include"filter.h"
char * hidden_address = NULL;
char * puppet_address = NULL;
int
main(int argc, char *argv[], char *env[])
{
  char * find_hidden_addresses(int argc, char * argv[]);
  void send_out(void);
  program = "puppet";
  read_message(argc, argv, env);
  find_hidden_addresses(argc, argv);
  if (same(hidden_address, from))
    {
      send_out();
    }
  mail_it(puppet_address, to_address, subject);
  printf("\nHi %s, \n\n"
	 "You have received the following message through %s\n"
	 "From: %s.\n\n"
	 "How can it be that %s knows your puppet address?",
	 puppet_address, to_address, from, from);
  dump_message(stdout, NO, NO, YES);
  return 0;
}
/************************************************************/
void
find_hidden_addresses(int argc, char * argv[])
{
  int i;
  for (i = 1; i < argc; i++)
    {
      if (same(argv[i], "-h"))
	{
	  hidden_address = argv[++i];
	}
      if (same(argv[i], "-p"))
	{
	  puppet_address = argv[++i];
	}
    }
  if (hidden_address == NULL || puppet_address == NULL)
    {
      fprintf(stderr, 
	      "puppet needs \"-h hidden_address \" in the alias file.");
      exit(1);
    }
  return;
}
/************************************************************
 *    When mail comes from the protected address, it is expected
 *    that the first token in the message will be the intended
 *    address.  This discovers that address, and sends the
 *    message out to the intended recipient.
 ************************************************************/
void
send_out(void)
{
  int i, ats = 0;
  get_first_token();
  for (i = 0; first_token[i]; i++)
    {
      if (first_token[i] == '@')
	ats++;
    }
  if (ats != 1)
    {
      mail_it(hidden_address, to_address, subject );
      printf("%s doesn't look like an address."
	     "\nBecause you, %s, are the owner"
	     "\nof this puppet address, you should make the first"
	     "\nword of your messages to this address be the email"
	     "\naddress of the intended recipient.  Please try again.\n",
	     first_token, hidden_address);
      printf("\n  ================    START OF MESSAGE RECEIVED ===================\n\n");
      dump_message(stdout, NO, NO, NO);
      printf("\n\n  =====================    END OF MESSAGE   =======================\n");
      exit(0);
    }
  mail_it(first_token, puppet_address, subject);
  dump_message(stdout, YES, YES, NO);
  exit(0);
}

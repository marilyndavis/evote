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

/* $Id: shelter.c,v 1.4 2003/01/15 18:54:13 marilyndavis Exp $ */ 
/*********************************************************
 *   shelter.c   -   called by wrapper
 *    Secures an email address.
 *********************************************************
 *
 *   shelter  -h hidden_address 
 *     The alias hides the hidden_address.  The owner of hidden_address
 *     can use the alias for incoming and outgoing mail and no one will
 *     know the hidden_address exists.
 *********************************************************
 *   To compile, alter the makefile and type:
 *   make
 *   Put the resulting executable in with your majordomo
 *   scripts and you're ready.
 *********************************************************
 **********************************************************/
#include"filter.h"
char * fake_address = NULL;
char * hidden_address = NULL;
int
main(int argc, char *argv[], char *env[])
{
  char * find_hidden_address(int argc, char * argv[]);
  void send_out(void);
  program = "shelter";
  read_message(argc, argv, env);
  hidden_address = find_hidden_address(argc, argv);
  fake_address = to_address;
  if (same(hidden_address, from))
    {
      send_out();
    }
  mail_it(hidden_address, fake_address, subject);
  printf("\nHi %s, \n\n"
	 "You have received the following message from %s.\n\n"
	 "To reply, use your Reply-To key and put:\n\n%s\n\n"
	 "as the first line of your message.\n\n",
	 hidden_address, from, from);
  dump_message(stdout, NO, NO, YES);
  return 0;
}
/***********************************************************/
char *
find_hidden_address(int argc, char * argv[])
{
  int i;
  for (i = 1; i < argc; i++)
    {
      if (same(argv[i], "-h"))
	{
	  hidden_address = argv[i+1];
	  return hidden_address;
	}
    }
  fprintf(stderr, 
	  "shelter needs \"-h hidden_address \" in the alias file.");
  exit(1);
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
  int i, j, no, message_begins, ats = 0;
  char **list;
  char ** address_list(int *, int *);

  get_first_token();
  for (i = 0; first_token[i]; i++)
    {
      if (first_token[i] == '@')
	ats++;
    }
  if (ats != 1)
    {
      mail_it(hidden_address, alert_address, subject );
      printf("%s doesn't look like an address."
	     "\nBecause you, %s, are the owner"
	     "\nof this protected address, you should make the first"
	     "\nword of your messages to this address be the email"
	     "\naddress of the intended recipient.  Please try again.\n",
	     first_token, hidden_address);
      printf("\n  ================    START OF MESSAGE RECEIVED ===================\n\n");
      dump_message(stdout, NO, NO, NO);
      printf("\n\n  =====================    END OF MESSAGE   =======================\n");
      exit(0);
    }
  list =  address_list(&no, &message_begins);
  for ( i = 0; i < no; i++)
    {
      mail_it(list[i], fake_address, subject);
      for (j = message_begins; buffer[j]; j++)
	{
	  putchar(buffer[j]);
	  if (samen(&buffer[j], "\n..end..\n", 9))
	    {
	      break;
	    }
	}
      fflush(stdout);
    }
  exit(0);
}

/************************************************************
 * Parses email address from the start of the message, until
 * two \n \n are in a row. Then the message begins.
 ************************************************************/
#define BUNCH 10
char **
address_list(int * no, int * message_begins)
{
  char ** list;
  int i, no_addresses;
  void give_up(char* message);
  int size, ncount, start;

  list = malloc((size = BUNCH ) * sizeof (char*));
  if (list == NULL)
    {
      give_up("Can't find memory for processing.");
    }

  for (no_addresses = 0, i = msg_start - 1; 
       buffer[i]; no_addresses++,i--)
    {
      ncount = 0;
      while (buffer[++i] == ' ' || buffer[i] == '\n' 
	     || buffer[i] == '\t')
	{
	  if((ncount =  (buffer[i] == '\n'? ncount + 1 : 0)) == 2)
	    {
	      *no = no_addresses;
	      *message_begins = i + 1;
	      return list;
	    }
	}
      start = i;
      while (buffer[i] != '\t' && buffer[i] != '\n'
	     && buffer[i] != ' ')
	i++;
      list[no_addresses] = malloc(i-start + 1);
      if (list[no_addresses] == NULL)
	{
	  give_up("Can't find memory for processing");
	}
      strncpy(list[no_addresses], &buffer[start], i-start);
      list[no_addresses][i-start] = '\0';
      if (no_addresses == size-1)
	{
	  list = realloc(list, (size += BUNCH)* sizeof(char*));
	  if (list == NULL)
	    {
	      give_up("Can't find memory for processing");
	    }
	}
    }
  give_up("Can't find the end of the address list.");
  return NULL;
}

void 
give_up(char* message)
{
  mail_it(hidden_address, alert_address, subject );
  printf("%s\n",message);
  printf("\n\n  ================    START OF MESSAGE RECEIVED ===================\n\n");
  dump_message(stdout, NO, NO, NO);
  printf("\n\n  =====================    END OF MESSAGE   =======================\n");
  exit(0);
}

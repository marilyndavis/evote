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

/* $Id: in_message.c,v 1.4 2003/01/15 18:54:09 marilyndavis Exp $ */ 
/**********************************************************
 *  in_message.c  -- Functions to deal with the incoming
 *                   message.
 *********************************************************
 **********************************************************/
#include<stdio.h>
#include "../mailui/mailui.h"
#include<time.h>
/* public */
char **argv_copy;
int argc_copy;
char **env_copy;
char * petition_address;
int end_mark = -1;
int sig_start = 0;
/* local */
static OKorNOT fix_subject(int argc, char *argv[]);
/************************************************************
 *   Fixes up the subject
 *************************************************************/
OKorNOT
fix_subject(int argc, char *argv[])
{
  unqp_subject();
  if ((subject = convert_hex(subject)) == NULL)
    return NOT_OK;  /* not enough space */
  if (strip_subject() != OK)
    return NOT_OK; /* something wrong with list.config */
  return OK;
}
/*************************************************************
 *      Searches the command line arguments for the list
 *      name and then parses the header for the subject,
 *      and the message sender.
 *      After this, buffer contains the header and one
 *      line of the message body.
 *      buffer[msg_start] is the beginning of the real
 *      message -- just past the headers.
 *      returns PASS = 0 if everything is OK, some recipients
 *      for the error if not.
 ************************************************************/
int
parse_headers(int argc, char* argv[], char *env[])
{
  int return_code = PASS;
  YESorNO sub;
  argc_copy = argc;
  argv_copy = argv;
  env_copy = env;
#ifdef TRACE
  on_exit(exit_trace, NULL);
  fprintf(fptrace,"\nReading headers \n\n");
#endif
  if ((bytes = buff_up()) == -1)
    {
      sprintf(error_msg, "\n%s's host, %s is out of resources right now."
	      "\nPlease try your eVote command again later.\n",
	      (list == NULL ? "unknown" : list),
	      (whereami == NULL ? "unknown" : whereami));
      return  (SENDER | OWNER | ADMIN);
    }
  /*  close(0); */
  parse_message(NO);
  if ((return_code = get_list(argc, argv, &sub)) != PASS)
    return return_code;
#ifdef TRACE
  fprintf(fptrace, "\nMessage to %s"
	  "\nfrom %s", list, from);
#endif
  if (subject)
    {
      fix_subject(argc, argv); 
      if (confirm_this.status == CHECK) /* key from subject */
	{
	  strcpy(confirm_this.key, strip_key());
	}
      if ((original_subject = make_string(subject)) == NULL)
	return (SENDER | OWNER | ADMIN); /* not enough memory */
  /* fix_subject sets status to CHECK if it finds a Confirm: 
     STARTING if not */
      if (petition == YES ) /* came from eVote_petition */
	{          
	  /* check for translation of different subject */
	  /* switch to the right petition list */
	  /* determine need for a confirmation */
	  if (sort_out_petition(argc, argv) != OK) 
	    return (SENDER | OWNER | ADMIN);
	}
    }
  else
    {
      subject = "Your Message";
    }
  if (((return_path == NULL  || return_path[0] == '\0')
      && (reply_to == NULL || reply_to[0] == '\0')
      && (from == NULL || from[0] == '\0'))
     || error_msg[0] != '\0')
    {
      fprintf(stderr, "\nTrouble parsing headers.\n");
      if (error_msg[0] != '\0')
	fprintf(stderr, "%s", error_msg);
      if (!from || from[0] == '\0')
	{
	  fprintf(stderr,"\nCan't find 'From:' header.\n");
	  if (error_msg[0] == '\0')
	    sprintf(error_msg, "\nCan't read 'From:' header.\n");
	}
      return (SENDER | OWNER | ADMIN);
    }
  return PASS;
}
/************************************************************
 *     Set's the set_start variable at the current spot,
 *     plus correction.
 *************************************************************/
void
set_sig_start(int cc)
{
  read_to_end();
  sig_start = tokens_read_to;
  if (cc == EOF)
    {
      sig_start = end_mark;
      return;
    }
  while (buffer[--sig_start] != ' ' 
	&& buffer[sig_start] != '\t'
	&& buffer[sig_start] != '\n')
    {
      if (msg_start >= sig_start)
	{
	  sig_start = msg_start;
	  break;
	}
    }
}

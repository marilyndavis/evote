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

/* $Id: out_message.c,v 1.5 2003/10/20 17:07:34 marilyndavis Exp $ */ 
/*********************************************************
 *  out_message.c  Functions that prepare the outgoing
 *             message.
 *********************************************************
 **********************************************************/
#include<stdio.h>
#include "../mailui/mailui.h"
YESorNO header_done = NO;    /* available for alteration for 
				multiple sends */
static void cc(int whom);
/****************************************************
 *      dumps the message and quits.
 ****************************************************/
void
big_finish(int exit_code)
{
  printf("\n  ================    START OF MESSAGE RECEIVED ===================\n\n");
  dump_message(stdout, NO, NO, NO);
  printf("\n\n  =====================    END OF MESSAGE   =======================\n");
  finish(exit_code);
}
/****************************************************
 * called when an error is detected before the process has split.
 *****************************************************/
void
bounce_error(int whom)
{
  YESorNO did;
  /* This call to send splits the process and comes
     back the parent with the child's stdin sucking
     from this stdout */
  if (from == NULL)  /* then this comes from a call
			      to eVote_mail */
    {
      fprintf(stderr, error_msg);
      exit(1);
    }
  /* generate a message header on stdout */
  gen_header(whom, "Error:", YES);
  printf("\nIn response to your message which starts:\n");
  if (msg_start > 0 && tokens_read_to > msg_start)
    {
      print_tokens(NO);
    }
  else
    {
      print_first_line();
    }
  printf("\n----\n");
  printf("%s", error_msg);
  did = NO;
  if (list != NULL && list[0] != '\0')
    {
      if (whom & OWNER && whom != OWNER) 
	{
	  did = YES;
	  printf("\nThis message is also being sent to owner-%s", list);
	}
      if (whom & APPROVAL && whom != APPROVAL)
	{
	  did = YES;
	  printf("\nThis message is also being sent to %s-approval", list);
	}	      
      if (did && whereami != NULL && whereami[0] != '\0')
	printf("@%s\n", whereami);
    }
  if (whom & ADMIN)
    {
      printf("\nThis message is also being sent to %s", eVote_mail_to);
      if (whereami != NULL && whereami[0] != '\0')
	printf("@%s\n", whereami);
    }
#ifdef EVOTE_ERRORS
  if (whom & DEVELOPER)
    printf("\nThis message is also being sent to %s.\n", 
	   EVOTE_ERRORS);
#endif
  if (from == NULL)
    exit(1);
  big_finish(0);
}
/********************************************************
 *     Generates the Cc: line in the header.
 ***********************************************************/
void cc(int whom)
{
  if (whom & (OWNER | ADMIN | DEVELOPER | APPROVAL))
    {
      printf("Cc: ");
      if ((whom & OWNER) && list != NULL && list[0] != '\0')
	{
	  printf("owner-%s", list);
	}
      else if ((whom & APPROVAL) && list != NULL && list[0] != '\0')
	{
	  printf("%s-approval", list);
	}
      if ((whom & ADMIN) && from != NULL && from[0] != '\0' 
	 && !((whom & APPROVAL) && (list == NULL || list[0] == '\0')))
	{
	  if ((whom & OWNER) && (list != NULL && list[0] != '\0'))
	    printf(", \n\t");
	  printf(eVote_mail_to);
	}
#ifdef EVOTE_ERRORS
      if (whom & DEVELOPER)
	{
	  if (whom & (OWNER | ADMIN ))
	    printf(", \n\t");
	  printf("%s", EVOTE_ERRORS);
	}
#endif		 
      printf("\n");
    }
  return;
}
/********************************************************
 *        Closes ipc to Clerk, prints out version and exits.
 ***********************************************************/
void
finish(int exit_code)
{
  if (version[0] == '\0')
    get_version();
  if (time_str[0] != '\0')
    i_am_leaving();
  time_str[0] = '\0';
  printf("\n%s\n\n", version);
  /*	time(&end);
	fprintf(stderr, "%ld:%s\n", end - now, from); */
  /*#ifdef PETITION  
  if ((fp = fopen("/tmp/petition.through", "a")) != NULL)
    {
      dump_message(fp, NO, NO, NO);
      fclose(fp);
    }
    #endif */
  if (exit_code != -2)
    exit(exit_code);
  /* if exit_code == -2, we want to flush and close stdout
     but continue with a fork and exec to send a report.html
     via ftp.  */
}
/********************************************************
 *     whom is the addressee list:  SENDER ADMIN ...
 *     show_date is the "Last participation" date.
 ***********************************************************/
void
gen_header(int whom, const char *subject_prefix, YESorNO show_date)
{
  int i;
  int message_id_start = -1;
  /*	time_t now; */
  char at_where[MAX_ADDRESS + 1];
  char from_str[MAX_ADDRESS + 1];
  char *new_subject;
  if (header_done == YES)
    return;
  header_done = YES;
  send(whom);
  if (subject == NULL)
    {
      subject = original_subject = "Your Mail";
    }
  if (subject_prefix[0] == '\0' ||
     (new_subject = malloc(strlen(subject) + strlen(original_subject)
				  + strlen(subject_prefix) + 2))
     == NULL)
    {
      new_subject = subject;
    }
  else
    {
      sprintf(new_subject,"%s %s", subject_prefix, subject);
    }
  at_where[0] = '\0';
  if (whereami != NULL && whereami[0] != '\0')
    {
      sprintf(at_where, "@%s", whereami);
    }
  if (list != NULL && list[0] != '\0')
    {
      if (whom & MAJOR)
	sprintf(from_str,"owner-%s%s", list, at_where);
      else
	if (petition && petition_address != NULL 
	   && !same_address(petition_address, list))
	  {
	    sprintf(from_str,"%s%s", petition_address, at_where);
	  }
	else
	  sprintf(from_str,"%s-eVote%s", list, at_where);
    }
  else
    {
      sprintf(from_str,"eVote-owner%s", at_where);
    }
  printf("From: %s\n", from_str); 
  if (whom & MAJOR)  /* generated subscription requests */
    {
      printf("Subject: %s\n", new_subject);
      printf("Sender: %s\n", from_str);
      printf("Reply-To: %s\n", from_str);
      printf("To: %s\n", eVote_mail_to);
    }
  else if (whom & LIST)  /* generated messages to list */
    {
      printf("Subject: %s\n", new_subject);
      printf("Sender: %s-eVote%s\n", list, at_where);
      printf("Reply-To: %s%s\n", list, at_where);
      printf("To: %s%s\n", list, at_where);
    }
  else  /* to SENDER */
    {
      if (whom & SENDER)
	{
	  printf("To: %s\n", from);
	  cc(whom & ~SENDER);
	}
      else if (whom & (APPROVAL | OWNER)) 
	{
	  if (list != NULL && list[0] != '\0')
	    {
	      printf("To: owner-%s\n", list);
	      cc(whom & ~APPROVAL & ~OWNER);
	    }
	  else 
	    {
	      printf("To: %s\n", eVote_mail_to);
	    }
	}
      else
	{
	  printf("To: %s\n", eVote_mail_to);
	}
      if (new_subject != NULL && original_subject[0] != '\0')
	{
	  if (samen(original_subject, subject_prefix, strlen(subject_prefix)))
	    {
	      new_subject = original_subject;
	    }
	  else
	    {
	      sprintf(new_subject,"%s %s", subject_prefix, original_subject);
	    }
	}
      if (new_subject != NULL && new_subject[0] != '\0')
	printf("Subject: %s\n", new_subject);
      else 
	printf("Subject: %s\n", subject);
      if (petition && petition_address 
	 && !same_address(petition_address, list))
	{
	  printf("Sender: %s%s\n", petition_address, at_where);
	  printf("Reply-To: %s%s\n", petition_address, at_where);
	}
      else
	if (list)
	  {
	    printf("Sender: %s-eVote@%s\n", list, whereami);
	    printf("Reply-To: %s%s\n", list, at_where);
	  }
	else
	  {
	    printf("Sender: %s\n", from_str);
	    printf("Reply-To: %s\n", from_str);
	  }      
      if (date_start > 0)
	{
	  printf("In-Reply-To: Your message on ");
	  i = date_start;
	  while (buffer[i] && buffer[i] != '\n' && buffer[i] != '\r' 
		&& buffer[i]	!= '\0')
	    putchar(buffer[i++]);
	  if (message_id_start > 0)
	    {
	      printf("\n\t\t");
	      i = message_id_start;
	      while (buffer[i] != '\n' && buffer[i] != '\r' 
		    && buffer[i]	!= '\0')
		{
		  putchar(buffer[i++]);
		}			
	    }
	  printf("\n");
	}
    }
  printf("MIME-Version: 1.0\n");
  printf("Content-Type: text/plain; charset=ISO-8859-1\n");
  printf("Content-Transfer-Encoding: 8bit\n");
  {
    struct tm *tm;
    int i;
    i = strlen(version);
    while (version[--i])
      {
	if (version[i] == ' ')
	  break;
      }
    srand((unsigned int)time(NULL));
    tm = localtime (&now);
    printf("Message-ID: <%d%02d%02d%02d%02d%02d.eVote.%s.%05d@%s>\n",
	   tm->tm_year+1900, tm->tm_mon+1, tm->tm_mday, tm->tm_hour,
	   tm->tm_min, tm->tm_sec, 
	   (version[i] == ' '? &version[++i] : "0"),
	   (int)((65534.*rand()/(RAND_MAX+1.0)) + .5),
	   whereami); 
  }
  if (whom & MAJOR)
    printf("\n");
  else
    printf("\n--\n\n");
  if (show_date && (whom & SENDER) && time_str[0] != '\0')
    printf("%s's last eVote participation: \n\t%s", from, time_str);
  return;
}

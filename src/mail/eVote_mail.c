/* $Id: eVote_mail.c,v 1.7 2003/10/20 17:07:34 marilyndavis Exp $ */ 
/****************************************************************
 *  eVote_mail.c   
 *     This program has several functions:
 *     1.  Sychronize eVote's notion of a majordomo-run list to
 *     majordomo's notion.
 *         eVote_mail sync list_name
 *         If the list_name is a petition list, it also checks
 *         that the signature files match the Clerk's notion.
 *     2.  Drop an email address from all lists and from eVote's
 *         data.  
 *         eVote_mail drop <address>
 *     3.  Marking a list member as a read_only.
 *         eVote_mail no_vote list_name email_address
 *     4.  Expiring old email addresses from the "bounces" list:
 *         eVote_mail bounce 30
 *         will expire addresses at least 30 days old.
 *     5.  Expiring old confirm messages waiting on all lists for
 *         any reason, administrative or signature requests:
 *         eVote_mail confirm 5
 *         will expire all messages at least 5 days old.
 *         eVote_mail confirm 2 petitiona
 *         will expire signature messages only from the petitiona
 *         list only that are at least 2 days old.
 *     NOTES:
 *     Once eVote_insert is inserted into the alias line for the 
 *     list name, the first subscribe or unsubscribe to the list 
 *     will start a list inside the Clerk and synchronize the Clerk's
 *     concept of the members of the list to that of majordomo.
 *     All the members will be voting members unless the -r
 *     option is in the alias for the approval address, in which case
 *     all new members will be read-only members.
 sample-approval: "|/usr/local/majordomo/wrapper eVote_insert -r",owner-sample
 *     Whenever someone subscribes or unsubscribes through email,
 *     the Clerk gets the message and adjusts her list.  However,
 *     if you fiddle with majordomo's list in an editor, you'll
 *     need to call this program to tell eVote about the new member(s):
 *         eVote_mail sync list_name
 *         or
 *         eVote_mail sync  -r list_name 
 *     if you want new list members to have read_only access 
 *     until you verify their voter registration.
 *     To change a new member to a non-voter/read_only:
 *         eVote_mail no_vote list_name email_address
 *     If the member has already voted, you need to drop him:
 *     and resubscribe him and immediately make him a non-voter.
 *     Non-voters can be changed to voters by the list owner
 *     by sending this message to the list:
 *     eVote approve <password> vote <email_address>
 *     where password is the majordomo administrative password
 *     for the list.
 *     eVote approve <password> vacation <email_address>
 *     will put the email address on vacation.
 *     eVote approve <password> back <email_address>
 *     will restart the subscription on majordomo's list.
 *     eVote approve <password> bounce <email_address>
 *     will put th email address on vacation and attach it to the
 *     "bounces" list.
 eVote approve <password> move <old_email_address> <new_email_address>
 *     will move the vote record from the old email address to 
 *     the new email address.
 *     To synchronize the who.list file, the system's concept of
 *     the email addresses that are useful, turn off your modem
 *     to be sure no one is online and say:
 *     eVote_mail sync_who
 *****************************************************************
 *    Copyright (c) 1994...2015 Deliberate.com Patented.
 *    by Marilyn Davis
 **********************************************************/
#include<stdio.h>
#include "mailui/mailui.h"
static void arg_err(void);
void do_sync(YESorNO read_only);
static void parse_command(int argc, char **argv, YESorNO *sync,
			  YESorNO *read_only, YESorNO *no_vote, 
			  YESorNO *sync_who, YESorNO *do_drop,
			  YESorNO * do_bounces, YESorNO *do_confirms,
			  long * expire);
int
main(int argc, char *argv[])
{
  /* local */
  YESorNO sync = NO;
  YESorNO read_only = NO;
  YESorNO sync_who = NO;
  YESorNO no_vote = NO;
  YESorNO do_drop = NO;
  YESorNO do_confirms = NO;
  YESorNO do_bounces = NO;
  long expire = 0;
  ACTION dummy;
  time(&now);
  parse_command(argc, argv, &sync, &read_only, &no_vote, &sync_who, 
		&do_drop, &do_bounces, &do_confirms, &expire);
  make_lclist();
  if (parse_cf())   /* returns zero if OK */
    {
      if (error_msg[0] != '\0')
	fprintf(stderr,"%s", error_msg);
      fprintf(stderr,"\nRequest failed.\n");
      exit(0);
    }
  if (initialize() != OK)
    {
      fprintf(stderr,"\nTrouble communicating with eVote_Clerk.\n");
      exit(0);
    }
  if (do_bounces)
    {
      printf("\nDeleted %d addresses.\n", expire_bounces(expire));
      exit(0);
    }
  if (do_confirms)
    {
      printf("\nDeleted %d messages.\n", expire_confirms(expire));
      exit(0);
    }
  if (sync_who)
    {
      fprintf(stderr,"%s", who_sync());
      exit(0);
    }
  if (sync)
    {
      do_sync(read_only);
    }
  if (do_drop)
    {
      mail_voter = who_num(from, NO);
      if (drop_mail_voter("ALL", NO) == OK)
	{
	  move_reader(from, NULL);
	}
      exit(0);
    }
  if (no_vote)
    {
      switch (process_action(READ_ONLY, from, &dummy))
				/* adjust_major_list is not needed for a change
				   to READ_ONLY */
	{
	case OK:
	  printf("%s", error_msg);  /* has a message there anyway */
	  printf("\nSuccessful.\n");
	  exit(0);
	case PROBLEM:
	  fprintf(stderr,"%s", error_msg);
	  fprintf(stderr, "\n\nSend a subscription request to "
		  "majordomo@%s for %s,"
		  "\nor, if %s is on majordomo's list, give the command:\n\n"
		  "eVote_mail sync %s\n\nand "
		  "try your \"eVote_mail no_vote %s %s\" command again.\n", 
		  whereami, from, from, list, list, from);
	  exit(1);
	case UNDECIDED:
	case NOT_OK:
	  fprintf(stderr,"%s", error_msg);
	  exit(1);
	default:
	  /* impossible */
	  break;
	}
    }
  return 0;
}
/*********************************************************/
void
arg_err(void)
{
  fprintf(stderr,"\nusage: eVote_mail sync list_name\n"
	  "\n       to ask eVote to synchronize or start the existing"
	  "\n       majordomo-run email list, list_name.\n"
	  "\n       eVote_mail -r sync list_name\n"
	  "\n       to make any new members on the list 'read_only'\n"
	  "\n   or: eVote_mail drop email_address\n"
	  "\n       to drop email_address from all data.\n"
	  "\n   or: eVote_mail no_vote list_name email_address\n"
	  "\n       to disallow votes from email_address on the list_name list.\n"
	  "\n   or: eVote_mail bounce 30\n"
	  "\n       to drop the addresses from the bounces list that are 30 days old.\n"
	  "\n   or: eVote_mail confirm 5\n"
	  "\n       to drop all messages awaiting confirmation for 5 days.\n"
	  "\n   or: eVote_mail confirm 2 petitiona\n"
	  "\n       to drop signature messages awaiting confirmation on the petitiona list for 2 days.\n"
	  "\n   or: eVote_mail sync_who\n"
	  "\n       to syncronize the who.list file.\n");
  exit(0);
}
/*********************************************************/
void
parse_command(int argc, char **argv, YESorNO *sync,
		   YESorNO *read_only, YESorNO *no_vote,
		   YESorNO * sync_who, YESorNO *do_drop, 
		   YESorNO * do_bounces, YESorNO *do_confirms,
		   long * expire)
{
  int i;
  if (argc < 2)
    {
      arg_err();
    }
  if (same(argv[1], "sync_who"))
    {
      *sync_who = YES;
      return;
    }
  if (argc < 3)
    {
      arg_err();
    }
  if (samen(argv[1], "bounce", 6))
    {
      if (argc != 3)
	arg_err();
      if ((*expire = atoi(argv[2])) == 0)
	arg_err();
      *do_bounces = YES;
      return;
    }
  if (samen(argv[1], "confirm", 7))
    {
      if (argc < 3)
	arg_err();
      *do_confirms = YES;
      if (sscanf(argv[2], "%ld", expire) != 1)
	arg_err();
      list = "ALL";
      if (argc == 3)
	return;
      list = argv[3];
      return;
    }
  if (same(argv[1], "no_vote"))
    {
      if (argc != 4)
	arg_err();
      list = argv[2];
      from = argv[3];
      *no_vote = YES;
      return;
    }
  if (same(argv[1], "drop"))
    {
      if (argc != 3)
	arg_err();
      from = argv[2];
      *do_drop = YES;
      return;
    }
  if (same(argv[1], "sync"))
    {
      *sync = YES;
      if (argc < 3)
	arg_err();
      for (i = 2; i < argc; i++)
	{
	  if (same(argv[i], "-r"))
	    *read_only = YES;
	  else
	    list = argv[i];
	}
      if (list == NULL)
	arg_err();
      return;
    }
  if (same(argv[1], "-r"))
    {
      *read_only = YES;
      if (argc < 4)
	arg_err();
      for (i = 2; i < argc; i++)
	{
	  if (same(argv[i], "sync"))
	    *sync = YES;
	  else
	    list = argv[i];
	}
      if (list == NULL || *sync == NO)
	{
	  arg_err();
	}
      return;
    }
  arg_err();
}
/*********************************************************/
void
do_sync(YESorNO read_only)
{
  OKorNOT sync_list(char*, YESorNO);
  YESorNO do_all = NO;
  OKorNOT cc = OK;
  if (same(lclist, "all"))
    do_all = YES;
  while (do_all == NO
	|| (lclist = get_next_list(NO)) != NULL)
    {		
      int i;
      for (i = 0; lclist[i]; i++)
	{
	  if (lclist[i] >= 'A' && lclist[i] <= 'Z')
	    lclist[i] += 'a' - 'A';
	}
      list = lclist;
      cc = OK;
      if (do_all && does_conf_exist(list) != YES)
	{
	  printf("%s is not eVoted.\n", list);
	  continue;
	}
      if ((cc = sync_list(lclist, read_only))
	 || error_msg[0] != '\0')
	{
	  printf("%s", error_msg);
	  error_msg[0] = '\0';
	} 
      if (samen(lclist, "petition", 8) && sync_petitions() != OK)
	{
	  cc = NOT_OK;
	}
      if (error_msg[0] != '\0')
	printf("%s", error_msg);
      switch (cc)
	{
	case NOT_OK:
	  printf("\nRequest to sync %s found adjustments.\n", list );
	  break;
	case UNDECIDED:
	  printf("\nRequest to sync %s encountered system or programmer problems.\n",
		 list);
	  break;
	case OK:
	  printf("\n%s is synchronized and ready to go.\n", list);
	  break;
	default:
	  /* impossible */
	  break;
	}
      if (!do_all)
	break;
    }
}	

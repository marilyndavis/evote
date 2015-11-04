/* $Id: listm.c,v 1.6 2003/01/28 04:11:27 marilyndavis Exp $ */ 
/**********************************************************
 *   listm.c  Functions that relate to the mailing list.
 *********************************************************
 **********************************************************/
#include<stdio.h>
#include "mailui.h"
typedef struct
{
  unsigned long uid;
  ACTION action;
} ACTION_LIST;
char *list = NULL;
char *lclist = NULL;  /* lower-case version of list */
/*  approval_string = the keyword to check the list password */
char * approval_string = "approve";
static void bounce_them(char *target);
static int list_members(ACTION_LIST* eVote_list);
static void move_it(void);       
static ACTION_LIST * whos_in(char *list_name);
/**********************************************************
 *   Adds or subtracts the target from the listserver
 *   list depending on the change in action.
 *   If it must add, it expects the whole line to
 *   be stored at line.  If it must subtract, it
 *   reports the subtracted line into line.
 **********************************************************/
void
adjust_reader_list(char * target, char * line,
		   ACTION old_action, ACTION new_action)
{
  char * dropped;
  switch (new_action)
    {
    case EVERYTHING:
      switch (old_action)
	{
	case VACATION:
	  /*	case VACATION + LOCK: */
	case VOTE_ONLY:
	  /*	case VOTE_ONLY + LOCK: */
	case SIGNER:
	  add_reader(target, line, lclist);
	  break;
	case READ_ONLY:
	  break;
	default:
	  /* impossible */
	  break;
	}
      break;
    case VOTE_ONLY:
    case VACATION:
    case SIGNER:
      switch (old_action)
	{
	case EVERYTHING:
	  if ((dropped = drop_reader(target, lclist)) != NULL)
	    strcpy(line, dropped);
	  break;
	case READ_ONLY:  /* impossible */
	  break;
	default:
	  /* impossible */
	  break;
	}
      break;
    case READ_ONLY:
      switch (old_action)
	{
	case EVERYTHING:
	  break;
	case VOTE_ONLY:  /* impossible */
	case SIGNER:
	case VACATION:
	  break;
	default:
	  /* impossible */
	  break;
	}
      break;
    default:
      /* impossible */
      break;
    }
}
/**********************************************************/
void
approve(void)
{
  int cc;
  ACTION action = UNSET;
  ACTION old_action;
  char * do_what;
  char * target;
  char off_bounces[MAX_LINE + 1] = "";
  char *pt;
  void process_unsign(void);  /* in petition.c */
  char new_line[MAX_LINE + 1];
  if ((cc = get_token()) == -1)
    {
      sprintf(error_msg,"\neVote expects a password after \"%s\".\n", 
	      approval_string);
      bounce_error(SENDER);
    }
  if(is_password(token) == NO)
    {
      sprintf(error_msg,"\nThe password you gave is wrong.\n");
      bounce_error(SENDER | OWNER);
    }
  block_token();
  do_what = save_token(&cc);
  /* first are the commands that pertain to a poll */
  if (same(do_what,"drop") || same(do_what,"close") || same(do_what, "unsign"))
    {
      if (subject == NULL || subject[0] == '\0')
	{
	  sprintf(error_msg, "\nYour eVote command requires a subject.\n");
	  send_help("help", SENDER, "Error:");
	}
      else
	{
	  copy_poll(YES); /* If dropping_id != 0, we have a polled
			     subject */
	}
      if (dropping_id == 0)
	{
	  if (mail_voter != 0)
	    {
	      sprintf(error_msg,"\nThere is no poll attached to the subject, \"%s\".\n", 
		      subject);
	    }
	  if (same(do_what,"unsign"))
	    {
	      bounce_error(SENDER);
	    }
	  send_help("help", SENDER, "Error:");
	}
      confirm_this.status = VERIFIED;
      if (same(do_what,"drop"))
	{
	  cc = get_token();
	  if (same(token, "quiet") || same(token, "silent"))
	    process_drop(YES, YES);
	  else
	    process_drop(YES, NO);
	}
      if (same(do_what,"close"))
	process_close(YES);
    }
  /* now the commands that don't require a poll */
  else if (same(do_what,"vote") || same(do_what,"back"))
    {
      action = EVERYTHING;
    }
  else if (same(do_what,"no_vote") || same(do_what, "novote")
	   || same(do_what,"no-vote"))
    {
      action = READ_ONLY;
    }
  else if (same(do_what,"vacation") || same(do_what, "bounce"))
    {
      action = VACATION;
    }
  else if (same(do_what,"vote_only"))
    {
      action = VOTE_ONLY;
    }
  else if (same(do_what,"move"))
    {
      move_it();
    }
  else
    {
      sprintf(error_msg,"\neVote can't interpret your command.\n");
      bounce_error(SENDER);
    }
  if ((target = save_token(&cc)) == NULL
      || same(token,"end"))
    {
      sprintf(error_msg,"\nYou need to specify an email address for your %s command.\n", do_what);
      bounce_error(SENDER);
    }
  if (same(do_what,"unsign"))
    {
      process_unsign();
    }
  i_am_leaving();
  if (same(do_what,"back"))
    {
      char tmp[MAX_LINE + 1];
      sprintf(new_line,"<%s> %c", target, (cc == EOF? '\n' : cc));
      if (cc != '\n' && cc != EOF)
	{
	  do
	    {
	      cc = get_token();
	      sprintf(tmp, "%s%c", token, (cc == EOF ? '\n' : cc));
	      strcat(new_line, tmp);
	    }
	  while (cc != '\n' && cc != EOF);
	}
      if ((pt = drop_reader(target, "bounces")) != NULL)
	{
	  sprintf(off_bounces,"\n%s has been removed from the bounces list.\n",
		  target);
	}
    }
  if (same(do_what,"bounce"))
    {
      bounce_them(target);
    }
  if ((cc = process_action(action, target, &old_action)) == NOT_OK)
    {
      strcat(error_msg, off_bounces);
      bounce_error(SENDER | OWNER);
    }
  /* puts a message in error_msg even when it's successful */
  adjust_reader_list(target, new_line, old_action, action);
  strcat(error_msg, off_bounces);
  /*	from = target; */
  if (cc == UNDECIDED)
    {
      bounce_error(SENDER | OWNER);
    }
  gen_header(SENDER | OWNER, "Re:", YES);
  printf("%s", error_msg);
  big_finish(0);
}
/*************************************************************
 *  Bounces a list of email addresses starting with target.
 ************************************************************/
void
bounce_them(char *target)
{
  ACTION old_action;
  char new_line[MAX_LINE + 1];
  int cc;
  gen_header(SENDER | OWNER, "Re:", YES);
  do
    {
      new_line[0] = '\0';
      if (same(target, "end"))
	break;
      if (process_action(VACATION, target, &old_action) != OK)
	{
	  printf("%s", error_msg);
	  error_msg[0] = '\0';
	  old_action = EVERYTHING;
	}
      /* puts a message in error_msg even when it's successful */
      adjust_reader_list(target, new_line, old_action, VACATION);
      if (new_line[0] == '\0')
	continue;
      if (!is_listed(target, "bounces"))
	add_reader(target, new_line, "bounces");
      printf("\n%s was moved to the bounces list.\n", target);
    }
  while ((target = save_token(&cc)) != NULL);
  big_finish(0);
}
/*************************************************************
 *   prints a list of the members and their current 
 *   action and returns the number of members.
 *************************************************************/
int
list_members(ACTION_LIST* eVote_list)
{
  int topper;
  for (topper = 0; eVote_list[topper].uid != 0; topper++)
    {
      switch (eVote_list[topper].action)
	{
	case EVERYTHING:
	case VOTE_ONLY:
	  /*	case (VOTE_ONLY + READ_ONLY): 
		case LOCK:
		case (VOTE_ONLY +READ_ONLY) | LOCK:*/
	  /*	case VOTE_ONLY | LOCK: */
	  printf("\n                  ");
	  break;
	case SIGNER:
	  printf("\nSignature only -> ");
	  break;
	case VACATION:
	  printf("\nOn vacation    -> ");
	  break;
	case READ_ONLY:
	  /*	case READ_ONLY | LOCK: */
	  printf("\nRead only      -> ");
	  break;
	default:
	  /* impossible */
	  break;
	}
      printf("%s", who_is(eVote_list[topper].uid));
    }
  printf("\n");
  return topper;
}
/**********************************************************
 *  Makes a lower case version of the list name and
 *  stores it in lclist.
 **********************************************************/
void
make_lclist(void)
{
  int i;
  if (list == NULL)
    return;
  lclist = malloc(strlen(list)+1);
  for (i=0; list[i]; i++)
    {
      lclist[i] = (list[i] <= 'Z' && list[i] >= 'A' ?
		   list[i] - 'A' + 'a' : list[i]);
    }
  lclist[i] = '\0';
}
/*************************************************************
 * provides a list of the members of the email list
 *************************************************************/
void
members(void)
{
  ACTION_LIST *eVote_list;
  char t_str[40];
  if (petition != NO)
    {
      sprintf(error_msg,"\nThe \"eVote members\" command is not available on this petition list.\n");
      bounce_error(SENDER);
    }
  strcpy(t_str, time_str);  /* i_am_leaving wipes it out */
  i_am_leaving();   /* whos_in is a respond_once -- don't
		       be online */
  if ((eVote_list = whos_in(lclist)) == NULL)
    /* resource problem */
    bounce_error(SENDER  | ADMIN);
  strcpy(time_str, t_str);   /* put it back for message */
  gen_header(SENDER, "Re:", YES);
  time_str[0] = '\0';  /* wipe it out again to avoid a double leaving */
  printf("\neVote's list for %s:\n", lclist);
  list_members(eVote_list);
  big_finish(0);
}
/**********************************************************
 *  interprest the move command
 **********************************************************/
void
move_it(void)
{
  char *was;
  char *is;
  int cc;
  int found;
  char * from2;
  if ((was = save_token(&cc)) == NULL ||
      (is = save_token(&cc)) == NULL)
    {
      sprintf(error_msg,"\nThe \"move\" command is:\n\n\teVote %s < password > old_address new_address\n\n",
	      approval_string);
      bounce_error(SENDER);
    }
  move_voter(was, is);
  found = move_reader(was, is);
  from2 = from;
  from = is;
  header_done = NO;
  gen_header(SENDER | OWNER, "Re:", NO);
  if (found == 0)
    printf("\n\nAs directed by %s, %s has been moved to \n%s.\n",
	   from2, was, is);
  else
    printf("\n\nAs directed by %s, %s has been moved to \n%s on %d list%s.\n",
	   from2, was, is, found, (found == 1 ? "" : "s"));
  /*  time_str[0] = '\0'; */
  big_finish(0);
}
/**********************************************************
 *  Starts a new list and subscribes everyone
 *  in the Majordomo list to The Clerk's list.
 **********************************************************/
OKorNOT
start_list(char *list)
{
  char *name;
  char *whole_line;
  char err_buf[MAX_ERROR + 1] = "";
  OKorNOT cc = OK;

  if ((cc = check_reader_files()) != OK)
    {
      return cc;
    }
  switch (send_eVote_conf(lclist, 0))
    {
    case FAILURE:
      sprintf(error_msg,"\neVote returned FAILURE when starting new list.");
      return NOT_OK;
      break;
    case NOT_GOOD:
      sprintf(error_msg,"\neVote: Can't open a new file to create %s.\n",
	      list);
      return NOT_OK;
      break;
    case DONE:
      break;
    case REDUNDANT:
      sprintf(error_msg,"\neVote: A %s conference already exists.\n", list);
      break;
    default:
      /* impossible */
      break;
    }
  while ((name = get_next_name(lclist, NO, &whole_line)) != NULL)
    {
      if ((mail_voter = who_num(name, YES)) == 0)
	{
	  sprintf(error_msg, "\nThe Clerk rejected the address: %s\n",
		  name);
	  if (strlen(err_buf) + strlen(error_msg) <= MAX_ERROR)
	    {
	      strcat(err_buf, error_msg);
	    }
	  cc = NOT_OK;
	}
      else
	{
	  ACTION action = EVERYTHING;
	  if (same(whole_line, "vacation"))
	    action = VACATION;
	  if (add_mail_voter(lclist, action) != OK)
	    {
	      if (strlen(err_buf) + strlen(error_msg) <= MAX_ERROR)
		{
		  strcat(err_buf, error_msg);
		}
	      cc = NOT_OK;
	    }
	}
    }
  get_next_name(lclist, YES, &whole_line);
  strcpy(error_msg, err_buf);
  return cc;
}
/*********************************************************
 *   Makes eVote's notion of who is on the list
 *   match majordomo's, or mailman's.
 *   Returns NOT_OK if there was some discrepancy -- but fixed.
 *        UNDECIDED if there was a system or programming problem.
 *               OK if the sync was unnecessary.
 *   For mailman, the whole_line either contains "vacation", meaning
 *     nomail is YES or "everything" meaning nomail is NO so the
 *     user not on vacation.
 *********************************************************/
OKorNOT
sync_list(char *list_name, YESorNO read_only)
{
  ACTION_LIST * eVote_list;
  char *name;
  int topper, i;
  ACTION new_action = EVERYTHING;
  char *whole_line;
  OKorNOT cc = OK;
  if (read_only == YES)
    new_action = READ_ONLY;
  i = strlen(list_name);
  list = malloc(i+1);
  if ((list = malloc(strlen(list_name) + 1)) == NULL)
    {
      printf("\nUnable to allocate space for list name.\n");
      return UNDECIDED;
    }
  strcpy(list, list_name);
  make_lclist();
  if (does_conf_exist(lclist) == NO)
    {
      printf("\nStarting a new list for eVote: %s\n", list_name);
      return start_list(lclist);
    }
  if ((eVote_list = whos_in(lclist)) == NULL)
    /* resource problem */
    return UNDECIDED;
  printf("\neVote's list for %s starts with:\n", lclist);
  topper = list_members(eVote_list);
  if ((cc = check_reader_files()) != OK)
    return cc;
  while ((name = get_next_name(lclist, NO, &whole_line)) != NULL)
    {
      ACTION action = EVERYTHING;
      if (same(whole_line, "vacation"))
	action = VACATION;
      if ((mail_voter = who_num(name, YES)) == 0)
	{
	  printf("The Clerk rejected the address: %s\n",
		 name);
	  continue;
	}
      for (i = 0; i < topper; i++)
	{
	  if (mail_voter == eVote_list[i].uid)
	    {
	      if ((eVote_list[i].action & SIGNER)
		  || (eVote_list[i].action & VOTE_ONLY)
		  || (eVote_list[i].action & VACATION 
		      && action != VACATION))
		{
		  process_action(action, name,
				 &(eVote_list[i].action));
		}
	      break;
	    }
	}
      if (mail_voter == eVote_list[i].uid)
	{
	  eVote_list[i].uid = 0;
	  continue;
	}
      if (add_mail_voter(lclist, new_action) == OK)
	printf("\nAdded %s as %s.\n", name, print_action(new_action));
      else
	{
	  printf("%s", error_msg);
	  error_msg[0] = '\0';
	  cc = NOT_OK;
	}
    }
  get_next_name(lclist, YES, &whole_line);
  for (i = 0; i < topper; i++)
    {
      if (eVote_list[i].uid != 0)
	{
	  char temp[200];
	  name = temp;
	  mail_voter = eVote_list[i].uid;
	  strcpy(name, who_is(mail_voter));
	  /*	  if ((eVote_list[i].action & (VOTE_ONLY | LOCK))
		  || (eVote_list[i].action & (VACATION | LOCK)))*/
	  if ((eVote_list[i].action & VOTE_ONLY)
	      || (eVote_list[i].action & VACATION )
	      || (eVote_list[i].action & SIGNER ))
	    continue;
	  if (drop_mail_voter(lclist, NO) == OK)
	    {
	      printf("\nDropped %s who was %s", name, 
		     print_action(eVote_list[i].action));
	      cc = NOT_OK;
	    }
	}
    }
  if ((eVote_list = whos_in(lclist)) == NULL)
    /* resource problem */
    return UNDECIDED;
  printf("\neVote ends with:\n");
  list_members(eVote_list);
  return cc;
}
/*************************************************************
 *   This call starts a sync happening when nothing else
 *   is happening.
 *   It checks each and every ballot in the system
 *   to be sure that the list of email addresses
 *   is current.  Errors are logged to the Clerk.log.
 *************************************************************/
char *
who_sync(void)
{
  switch (send_who_sync())
    {
    case ON_LINE:
      return "Sync started.  Check the Clerk.log in a few minutes.\n";
      break;
    case NOT_GOOD:
      return "Discrepancies found in the sync.  See the Clerk.log.\n";
      break;
    case DONE:
      return "Dropped some addresses.  See the Clerk.log.\n";
      break;
    case GOOD:
      return "Who list was correct.\n";
      break;
    default:
      /* impossible */
      break;
    }
  return NULL;
}
/*****************************************************************
 * Returns an array of uid's in the conf.  End is marked with a 0.
 *****************************************************************/
#define WBLOCK 3
ACTION_LIST *
whos_in(char *list_name)
{
  static ACTION_LIST *ins;
  ACTION action;
  char * ret_str;
  int space;
  int i = 0;
  unsigned long uid;

  if ((ins = malloc((space = WBLOCK) * sizeof(ACTION_LIST)))
      == NULL)
    {
      fprintf(stderr, "\nNo space to determine who's in %s.\n", 
	      list_name);
      return NULL;
    }
  switch (send_whos_in(list_name))
    {
    case NO_CONF:
      ins[0].uid = 0;
      ins[1].uid = 0;
      break;
    case STRING_OUT:   /* No one */
      ins[0].uid = 0;
      ins[1].uid = 1;
      break;
    case UID_LIST:
    case UID_LIST_MORE:
      while ((ret_str = uid_report(&uid)) != NULL)
	{
	  action = (ACTION)atoi(ret_str);
	  if ((i+2) > space)
	    if ((ins = realloc(ins, (space += WBLOCK)*sizeof(ACTION_LIST)))
		== NULL)
	      {
		fprintf(stderr, "\nNo space to determine who's in %s.\n", 
			list_name);
		return NULL;
	      }
	  ins[i].action = action;
	  ins[i++].uid = uid;
	}
      ins[i].uid = 0;
      break;
    default:
      /* impossible */
      break;
    }
  return ins;
}

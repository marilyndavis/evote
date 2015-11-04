/* $Id: poll_list.c,v 1.3 2003/01/15 18:54:09 marilyndavis Exp $ */ 
 /**********************************************************
 *  poll_list.c  Functions that deal with the whole list
 *               of polls.
 *********************************************************
 **********************************************************/
#include<stdio.h>
#include"mailui.h"
short dropping_id = 0;
PRIV_TYPE ptype;
ITEM_INFO *p_item_copy;
static void alpha_items(YESorNO really_sort, int **alpha_sort);
/***********************************************************
 *   Places the indexes into the item_info array in sorted
 *   order, ascii sorted on their titles.
 ***********************************************************/
void
alpha_items(YESorNO really_sort, int **alpha_sort)
{
  int i, j;
  int *sort_list;
  if ((sort_list = malloc(*p_no_items * sizeof(int))) == NULL)
    {
      sprintf(error_msg, "\nNo resources to sort list.\n");
      fprintf(stderr, "\nNo resources to sort list.\n");
      bounce_error(SENDER | ADMIN);
    }
  if (really_sort == NO)
    {
      for (i = 1; i <= *p_no_items; i++)
	sort_list[i-1] = i;
      *alpha_sort = sort_list;
      return;
    }
  /*  The sorted list starts at index 0 and the
      item_list starts at index 1.
      That makes this insertion sort extra confusing. */
  sort_list[0] = 1;
  for (i = 2; i <= *p_no_items; i++)
    {
      for (j = i - 2; j >= 0; j--)
	{
	  if (strCmp(item_info[sort_list[j]].eVote.title, 
		     item_info[i].eVote.title)
	      > 0)
	    sort_list[j+1] = sort_list[j];
	  else
	    break;
	}
      sort_list[j+1] = i;
    }
  *alpha_sort = sort_list;
  return;
}
/**********************************************************
 *  Sends the list of polled subjects to the sender.  If the 
 *  next token is "open" or "closed", it only lists the open 
 *  or closed subjects.
 **********************************************************/
void
list_polls(YESorNO for_error)
{
  int i, j;
  int cc;
  int count = 0;
  int dummy;
  YESorNO list_open = NO;
  YESorNO list_closed = NO;
  YESorNO list_all = YES;
  YESorNO started;
  YESorNO alpha = NO;
  int one;
  VSTATUS vstat;
  int *alpha_sort;
  if (!for_error)
    {
      gen_header(SENDER, "Re:", YES);
    }
  else
    {
      gen_header(SENDER, "ERROR:", NO);
      printf("\nYour \"eVote %s\" command cannot be processed because there is no"
	     "\npoll attached to the subject, "
	     "\"%s\".\n"
	     "\nPlease check the spelling on the subject line of your message.\n", 
	     token, original_subject);
      printf("\n\nTo learn more about eVote, please send an \"eVote help\" command.\n");
      alpha = YES;
    }
  do
    {
      if (for_error)
	break;
      cc = get_token();
      if (token[0] == '\0')
	break;
      if (same(token,"closed"))
	{
	  list_closed = YES;
	  list_all = NO;
	  continue;
	}
      if (same(token, "open"))
	{
	  list_open = YES;
	  list_all = NO;
	  continue;
	}
      if (is_option(&cc, "sort", "alpha", &dummy) == YES)
	alpha = YES;
    }
  while (cc != EOF && cc != '\n');
  if (alpha == YES)
    alpha_items(YES, &alpha_sort);
  else
    alpha_items(NO, &alpha_sort);
  if (list_all == YES)
    {
      list_open = YES;
      list_closed = YES;
    }
  if (!for_error)
    {
      printf("\nIn response to your command:\n");
      print_tokens(NO);
    }
  for (i = 1; i <= *p_no_items; i++)
    {
      if (!(item_info[i].eVote.type >= GROUPED
	    && item_info[i].eVote.type <= GROUPEDN))
	{
	  vstat = item_info[i].eVote.vstatus;
	  if (item_info[i].eVote.type == PLAIN)
	    vstat = item_info[i+1].eVote.vstatus;
	  if (list_closed == YES)
	    {
	      if (vstat == CLOSED)
		{
		  one = i;
		  count++;
		}
	    }
	  if (list_open == YES)
	    {
	      if (vstat != CLOSED)
		{
		  one = i;
		  count++;
		}
	    }
	}
    }
  printf("\nThe %s list has %ld participants.  It has %d subject%s\nwith ",
	 list, no_of_participants, count,
	 (count == 1 ? " " : "s"));
  if (count == 1)
    {
      printf("%s poll attached.\n",
	     (list_all == YES ? "a" : 
	      (list_closed == YES ? "a closed " : "an open ")));
    }
  else
    {
      printf("%spolls attached. \n",
	     (list_all == YES  ? "":
	      (list_closed == YES ? "closed " : "open ")));
    }
  if (count == 0)
    finish(0);
  if (count == 1)
    {
      printf("\nThe one subject with a%spoll attached is:\n",
	     (item_info[one].eVote.vstatus == CLOSED ?
	      " closed " : "n open "));
      if (item_info[one].eVote.vstatus == CLOSED)
	printf("\n Number of Voters                   Subject  Poll Type     Final Result Summary\n");
      else
	printf("\n Number of Voters                   Subject  Poll Type    Result Summary So Far\n");
      print_list_line(one);
    }
  else
    {
      printf("\nThese are listed below.");
    }
  printf("\n\nTo learn the specifics of a particular poll:");
  printf("\n\n     1.   Send email to %s@%s.", list, whereami);
  printf("\n\n     2.   Make your subject line match the subject listed here.");
  printf("\n\n     3.   Have your message say:");
  printf("\n\n             eVote info");
  if (count == 1)
    printf("\n\n");
  started = NO;
  if (count > 1 && list_open == YES)
    {
      for (j = 0, i = alpha_sort[j]; j < *p_no_items; i = alpha_sort[++j] )
	{
	  if (item_info[i].eVote.type >= GROUPED
	      && item_info[i].eVote.type <= GROUPEDN)
	    continue;
	  vstat = item_info[i].eVote.vstatus;
	  if (item_info[i].eVote.type == PLAIN)
	    vstat = item_info[i+1].eVote.vstatus;
	  switch (vstat)
	    {
	    case OPEN:
	    case UNSEEN:
	      if (started == NO)
		{
		  started = YES;
		  printf("\n");
		  highlight("OPEN POLLS");
		  printf("\n Number of Voters                   Subject  Poll Type    Result Summary So Far\n");
		}
	      print_list_line(i);
	      break;
	    case CLOSED:
	    default:
	      break;
	    }
	}
      if (started == NO)
	printf("\n\nThere are no open polls for the %s list.\n", list);
    }
  started = NO;
  if (count > 1 && list_closed == YES)
    {
      for (j = 0, i = alpha_sort[j]; j < *p_no_items; i = alpha_sort[++j] )
	{
	  if (item_info[i].eVote.type >= GROUPED
	      && item_info[i].eVote.type <= GROUPEDN)
	    continue;
	  vstat = item_info[i].eVote.vstatus;
	  if (item_info[i].eVote.type == PLAIN)
	    vstat = item_info[i+1].eVote.vstatus;
	  switch (vstat)
	    {
	    case OPEN:
	    case UNSEEN:
	      break;
	    case CLOSED:
	    default:
	      if (started == NO)
		{
		  printf("\n");
		  highlight("CLOSED POLLS");
		  printf("\n Number of Voters                   Subject  Poll Type     Final Result Summary\n");
		  started = YES;
		}
	      print_list_line(i);
	      break;
	    }
	}
      if (started == NO)
	printf("\n\nThere are no closed polls for the %s list.\n", list);
    }
  printf("\n");
  highlight("KEY:");
  printf("\n\tFor Yes/No polls:");
  printf("\n\n\t\t[12/10] =  [number of yes votes/number of no votes]");
  printf("\n\n\t\t1.2     =  Ratio of yes/no votes");
  printf("\n\n\tFor numeric polls:");
  printf("\n\n\t\t[12/10] =  [number of people who voted a positive number");
  printf("\n\t\t          / number of people who voted a negative number]");
  printf("\n\n\t\t2.3:1   =  {sum of positive votes/sum of negative votes}\n\n");
  big_finish(0);
}

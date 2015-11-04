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

/* $Id: menu.c,v 1.4 2003/01/15 18:54:08 marilyndavis Exp $ */ 
/**********************************************************
 *	../eVote/src/demo/menu.c  -
 *    User interface for the eVote menu.
 **********************************************************
 **********************************************************/
#include <stdio.h>
#include <pwd.h>
#include "../eVote.h"
extern YESorNO show_blurbs; /* in blurbs.c */
int time_out = 0;
/********************************************************
 *   This draws the eVote menu.
 *********************************************************/
void
eVote_menu(ITEM_INFO *p_item, YESorNO* voted_yet)
{
  char *priv_str;
  char *stat_str;
  YESorNO isauthor = NO;
  if (who_am_i() == p_item->eVote.author)
    isauthor = YES;
  if (p_item->eVote.type == PLAIN)
    {
      printf("\n\nItem #%d - %s - is PLAIN:\n", p_item->dropping_id, 
	     print_title(p_item->dropping_id));
      printf("\nThere is no vote on Item #%d.  However, you may enter:",
	     p_item->dropping_id);
      printf("\n   W - to receive (W)ho accessed statistics.");
      printf("\n   H - to receive (H)ow voted statistics.  The");
      printf("\n       result will be 'ACC' or '!ACC'.");
      printf("\n");
      return;
    }
  switch (p_item->eVote.priv_type)
    {
    case PUBLIC:
      priv_str = "a PUBLIC";
      break;
    case PRIVATE:
      priv_str = "a PRIVATE";
      break;
    case IF_VOTED:
      priv_str = "an IF_VOTED";
      break;
    }
  switch (p_item->eVote.vstatus)
    {
    case OPEN:
      stat_str = "OPEN for voting.";
      break;
    case CLOSED:
      stat_str = "CLOSED to voting.";
      break;
    case UNSEEN:
      stat_str = "OPEN for voting but the results are UNSEEN until the item CLOSES.";
      break;
    default:
      /*impossible */
      break;
    }
  printf("\n\nItem #%d %c %s %c is:\n", p_item->dropping_id, 34, 
	 print_title(p_item->dropping_id), 34);
  printf("\n  %s item, ", priv_str);
  printf("%s",  stat_str); 
  if (p_item->eVote.vstatus == CLOSED)
    {
      if (p_item->eVote.type == TALLIEDY
	 || p_item->eVote.type == GROUPEDY)
	{
	  printf("\n  Acceptable votes were YES and NO.");
	}
      else
	{
	  printf("\n  Votes from %d to %d were accepted.",
		 p_item->eVote.min, p_item->eVote.max);
	}
    }
  if (p_item->eVote.type >= GROUPED)
    {
      printf("\n  It is item %d in a group of %d.  The group has a sum-limit of %d.",
	     p_item->eVote.no_in_group - p_item->eVote.more_to_come,
	     p_item->eVote.no_in_group, p_item->eVote.sum_limit);
    }
  printf("\n\nPlease enter:\n");
  if (p_item->eVote.vstatus != CLOSED)
    {
      if (p_item->eVote.type == TALLIEDY 
	  || p_item->eVote.type == GROUPEDY)
	{
	  printf("\n     y - to vote YES.");
	  printf("\n     n - to vote NO.");
	}
      else
	{
	  printf("\n     # - where '#' is a number from %d to %d:-> your vote.",
		 p_item->eVote.min, p_item->eVote.max);
	}
      if (*voted_yet == YES 
	 || (*voted_yet == MAYBE 
	     && ((*voted_yet = have_i_voted(p_item))
		 == YES)))
	printf("\n     R - to (R)emove your vote on this item.");
    }
  printf("\n\n     S - to (S)ee how you voted on this item.");
  if (p_item->eVote.priv_type != PRIVATE)
    {
      printf("\n     W - to receive (W)ho voted statistics.");
      printf("\n     H - to receive (H)ow voted statistics.");
    }
  if (isauthor == YES)
    {
      printf("\n");
      if (p_item->eVote.vstatus != OPEN)
	printf("\n     O - to change the vote status to OPEN.");
      if (p_item->eVote.vstatus != UNSEEN)
	printf("\n     U - to change the vote status to UNSEEN.");
      if (p_item->eVote.vstatus != CLOSED)
	printf("\n     L - to change the vote status to CLOSED.");
    }
  printf("\n\n     X - to e(X)plain something.");
  printf("\n     T - to access the (T)eacher.");
  if (show_blurbs == YES)
    printf("\n     S - to (S)ilence the democracy lecture.");
  printf("\n     Q - to (Q)uit eVote and return to the Conf? prompt.");
  printf("\n     C - to see the contents screen and return to Conf?");
  printf("\n");
}
/********************************************************
 *  This is called when the user indicates that she
 *  wants to access eVote, maybe by pressing the 'v' key.
 *  p_item is the last item accessed.  input is the 
 *  unprocessed input from the user.
 *  If come_back == YES, it tries to go straight back to
 *  the conference prompt after following the instructions.
 *  If it returns non-NULL, it expects the Conf? prompt to
 *  process the returned string.
 *********************************************************/
char *
process_eVote(ITEM_INFO* p_item,  
		     char* input, YESorNO come_back)
{
  short confusion = 0;
  char eprompt[10];
  YESorNO voted_yet = MAYBE;
  YESorNO get_input = NO;
  VSTATUS new_vstatus;
  char vote_str[8];
  sprintf(eprompt,"\neVote #%d? ", p_item->dropping_id);
  if (*input == 0)
    get_input = YES;
  else
    {
      get_input = NO;
      if (*input == 'v' || *input == 'V')
	{
	  come_back = YES;
	  input++;
	}
    }
  while (1)
    {
      new_vstatus = NOT_KNOWN;
      if (get_input == YES)
	{
	  input = GetArg(eprompt);
	  come_back = NO;
	}
      get_input = YES;
      while (*input == ' ')
	input++;
      switch (*input)
	{
	case 'c':
	case 'C':
	  sprintf(input,"c%d", p_item->dropping_id);
	  return input;
	  break;
	case '?': /* give menu */
	  eVote_menu(p_item, &voted_yet);
	  come_back = NO;
	  break;
	case 'H': /* how did someone else vote? */
	case 'h':
	  if (strcmp(input, "help") == 0
	     || strcmp(input,"HELP") == 0
	     || strcmp(input,"Help") == 0)
	    {
	      printf("\n\nThis eVote demo has three help features:");
	      printf("\n\n   'x'  -  to eXplain a word: 'x VTR' explains VTR.");
	      printf("\n   '?'  -  tells you your current choices.");
	      printf("\n   't'  -  accesses the teacher.\n");
	      break;
	    }
	  switch (process_how(p_item, ++input))
	    {
	    case UNDECIDED:
	      come_back = NO;
	      break;
	    case OK:
	      break;
	    case NOT_OK:
	      return NULL;
	      break;
	    default:
	      /*impossible */
	      break;
	    }
	  break;
	case 'S':
	case 's':  /* see his own vote */
	  (void)how_voted(p_item, NULL, 0); 
	  break;
	case 'q':
	case 'Q':
	  return NULL;
	case 'r':
	case 'R':
	  if (voted_yet == YES 
	     || (voted_yet == MAYBE 
		 && ((voted_yet = have_i_voted(p_item))
		     == YES)))
	    {
	      sprintf(vote_str,"%d", READ);
	      (void)process_vote(p_item, vote_str);
	      voted_yet = NO;
	    }
	  else
	    {
	      printf("\nYou have not voted yet on item #%d.\n",
		     p_item->dropping_id);
	    }
	  break;
	case 't':
	case 'T':
	  strcpy(input,"x teacher");
	  break;
	case 'x':
	case 'X':
	  explain(input);
	  break;
	case 'v':
	case 'V':
	  printf("\n\nYou are already at the eVote #%d? prompt.", p_item->dropping_id);
	  printf("\n\nUse `v' at the Conf? prompt to access eVote.\n\n");
	  break;
	case 'w':
	case 'W':
	  switch (process_who(p_item, ++input))
	    {
	    case UNDECIDED:
	      come_back = NO;
	      break;
	    case NOT_OK:
	      return NULL;
	      break;
	    case OK:
	      break;
	    default:
	      /* impossible */
	      break;
	    }
	  break;
	case 'U':
	case 'u':
	case 'O':
	case 'o':
	case 'L':
	case 'l':
	  change_vstatus(p_item, *input, NO);
	  break;
	default:
	  if (p_item->eVote.type == TALLIEDY
	      || p_item->eVote.type == PLAIN
	      || p_item->eVote.type == GROUPEDY)
	    {
	      if (*input == 'y' || *input == 'Y')
		strcpy(input, "1");
	      else if (*input == 'n' || *input == 'N')
		strcpy(input, "0");
	    }
	  if (*input < '0' || *input > '9')
	    {
	      come_back = NO;
	      if (input[0] != '\0')
		printf("\n%s is not understood.\n", input);
	      if (++confusion == 1)
		{
		  get_input = YES;
		  break;
		}
	      if (confusion > 2)
		{
		  eVote_menu(p_item, &voted_yet);
		  confusion = 0;
		}
	      else /* confusion == 2 */
		printf("\nEnter 'q' to quit eVote and return to the Conf? prompt. ");
	      break;
	    }
	  if (p_item->eVote.type == PLAIN)
	    {
	      printf("\n\nItem #%d : \"%s\" is a PLAIN item.",
		     p_item->dropping_id, 
		     print_title(p_item->dropping_id));
	      printf("\nThere is no vote on item #%d.\n", 
		     p_item->dropping_id);
	      printf("\nEnter '?' to see your choices.\n");
	      break;
	    }
	  (void)process_vote(p_item, input);
	  voted_yet = YES;
	  break;
	} /* end of switch */
      if (come_back == YES)
	return NULL;
    }  /* end of while forever */
}

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

/* $Id: input.c,v 1.3 2003/01/15 18:54:08 marilyndavis Exp $ */ 
/**********************************************************
 *	../eVote/src/eVote/eVoteui/input.c
 **********************************************************
 **********************************************************/
#include "../eVote.h"
/* List of questions that eVote asks.  The first field in each
   structure is the question number, #defined in eVoteui.h.  If you 
   add a question, you must also add a number for it in eVoteui.h 
   and increment the number of questions (NO_QUESTIONS) also #defined
   there.  */
char *GetArg(char *);
static struct questions_def qdata[NO_QUESTIONS] =
{	
  {NO_Q,"",""},
  {CHANGEIT, "\nEnter line number to change, or <RETURN> if OK: ",
   "\nPress <ENTER> if the vote parameters displayed above are OK.  If not, \nenter the question/answer number you want to change.\n\n"},
  {DROP_DAYS, "\nDrop Days? ",
   "\nHow many days must a voter not participate before she/he can remove \nher/himself from this conference? \n  0 allows lurking. \n  30 is a reasonable number.\n  9999 means voters can never be removed. \nHow many days?\n\n"},
  {DROPS_OK, "\nOkay?(y/n) ",
   "\nDo you want to drop the items? Enter 'y' for YES or 'n' for NO.\n\n"},
  {GROUPIT, "Grouped? ",
   "\nIf your instruction is something like, 'Vote YES on 3 of the next 5 items.'\nor 'Distribute 100 votes over then next 20 items.', enter 'y' for YES.\nOtherwise, enter 'n' for NO and this item will not be linked to other items.\n\n"},
  {HOW_WHO, "Name please? ",
   "\nWhose vote would you like to see?  Please enter the login name: \n\n"},
  {JOIN, "","\nDo you really want to join this conference? \n\n"},
  {NO_TOGETHER,"Number of items in the group? ",
   "\nYou indicated that this item will be grouped with other items.\nEnter a number to indicate the total number of items that will \nbe grouped together.\n\n"},
  {OKAY, "Ok? ","\nPress 'y' for YES if this is OKay. Otherwise press 'n' for NO. \n\n"},
  {MAX,"Maximum vote? ",
   "\nPlease enter the maximum vote for your item.  Votes greater than \nthe number you enter will not be allowed.  Or, if this is a \nyes/no question, you can enter 'y' and YES will be the maximum vote.\n\n"},
  {MIN,"Minimum vote? ",
   "\nPlease enter a number, 0 or larger.  Votes less than the number \nyou enter will not be allowed.  Or, if this is a yes/no question, \nyou can enter 'n' and NO will be the minimum vote.\n\n"},
  {PRIV_IF,"See IF others voted? ",
   "\nEnter 'y' for YES if user are allowed to see IF other participants \nhave voted yet.  Enter 'n' for NO if this information is secret.\n\n"},
  {PRIV_HOW,"See HOW others voted? ",
   "\nEnter 'y' for YES if votes are to be PUBLIC.\nEnter 'n' for NO if votes are secret.\n\n"},
  {REALLY_QUIT, "Really quit? ",
   "\nAnswer 'n' for NO if you want to continue adding your item(s). \nAnswer 'y' for YES if you really want to quit and NOT add your item(s). \n\n"},
  {SAME_LIMITS,"Same max/min? ", 
   "\nWill all the items in the group have the same minimum and maximum vote?\n\n"},
  {SEE_IT,"See tally? ",
   "\nDo you want the vote tally to be visible on the contents screen \nwhile the vote is in progress?\n\n"},
  {SUM_LIMIT,"Sum-limit? ",
   "\nHow many votes does each user have to distribute over the group?\n\n"},
  {TITLER, "Title?                                      |<--length limit\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b",
   "\nYour new item needs a title for the contents screen.\nPlease enter a title of 36 characters or less. \n\n"},
  {VOTE, "Vote? ","\nPlease enter a number to indicate your vote.\n\n"},
  {VOTED_ON, "Tallied? ",
   "\nEnter 'y' for YES if you want voting on this item; 'n' if not.\n\n"},
  {WHO_WHAT, "Question? ",
   "\nEnter a who-voted question. \n\n"},
  {YN_VOTE, "Yes/No vote? ",
   "\nEnter 'y' for YES if this vote question will be answered YES or NO.\n\n"}
};
struct questions_def * ask = qdata;
/**************************************************************
 *    LOW - LEVEL communication with online user
 ***********************************************************
 *       question is a QUESTION enum as defined in eVote.h.
 *       min, max are the limits on the acceptible number.
 *       yesno == YES if we are happy with a 'y' or 'n' for 
 *       '1' and '0'.
 *   returns a number, within the limits,
 *    or EQUIT to quit.
 *       ENOREPLY if no reply was given
 *       EQUESTION if terribly confused
 ************************************************************/
short
eVote_asknum(QUESTION question, short min, short max, 
		   YESorNO yesno)
{
  short num = -1;
  int mistakes = 0;
  char * pch;
  while (num < min || num > max)
    {
      if (++mistakes > MISTAKES)
	return EQUESTION;
      if (*(pch = GetArg(ask[question].qprompt)) == '\0')
	return ENOREPLY;
      if (yesno == YES)
	{
	  switch (*pch)
	    {
	    case '1':
	      if (strcmp(pch,"1") != 0)
		break;
	    case 'y':
	    case 'Y':
	      strcpy(pch,"1");
	      break;
	    case '0':
	      if (strcmp(pch,"0") != 0)
		break;
	    case 'n':
	    case 'N':
	      strcpy(pch,"0");
	      break;
	    default:
	      break;
	    }
	}
      switch (num = check_atoi(pch))
	{
	default:
	  if (num <= max && num >= min)
	    return num;
	case EQUESTION:
	  if (mistakes >= MISTAKES)
	    break;
	  if (mistakes % 2 != 0 && question != NO_Q)
	    {
	      printf("%s", ask[question].qhelp);
	      break;
	    }
	  else
	    printf("\nPlease enter a number from %d to %d. \n",
		   min, max);
	  if (yesno == YES)
	    {
	      if (min == 0 && max == 1)
		printf("Or enter 'y' for YES or 'n' for NO.\n");
	      else
		if (min == 0 && max == 98) /* collecting min */
		  printf("Or enter 'n' for NO.\n");
		else
		  if (min == 1) /* collecting max */
		    printf("Or enter 'y' for YES.\n");
	    }
	  printf("\n");
	  break;
	case EQUIT:
	  return EQUIT;
	}
    }
  return num;
}
/*********************************************************
 *   returns y or n or q, responds to an input ? with more
 *   prompting but returns a '?' if the user seems hopelessly
 *   lost.
 **********************************************************/
char
eVote_askyn(QUESTION question)
{
  int mistakes = 0;
  char * msg = ask[question].qprompt;
  while (++mistakes < MISTAKES)
    {
      switch (*GetArg(msg))
	{
	case 'n':
	case 'N':
	  return 'n';
	  break;
	case 'y':
	case 'Y':
	  return 'y';
	  break;
	case 'q':
	case 'Q':
	  return 'q';
	  break;
	default:
	  if (mistakes + 1 >= MISTAKES || question == NO_Q)
	    break;
	  printf("%s", ask[question].qhelp);
	  continue;
	  break;
	}
    }
  return '?';
}

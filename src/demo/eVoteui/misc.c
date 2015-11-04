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

/* $Id: misc.c,v 1.4 2003/01/23 19:54:27 marilyndavis Exp $ */ 
/**********************************************************
 *	../eVote/src/demo/eVoteui/misc.c
 **********************************************************
 **********************************************************/
#include <dirent.h>
#include <pwd.h>
#include "../eVote.h"
/**********************************************************
 *     Does atoi conversion plus checks that a 0 return really 
 *     means zero before it returns.
 *     or returns:
 *	        EQUIT     if the user pressed a 'q'
 *         EQUESTION if the user pressed anything else -
 *                      including a negative number!
 *         ENOREPLY  if the string is empty
 **************************************************************/
short
check_atoi(char *str)
{
  short answer;
  char *pch = str;
  if ((answer = atoi(str)) > 0)
    return answer;		
  if (answer < 0)
    return EQUESTION;
  if (*pch == '\0')
    return ENOREPLY;
  pch--;
  while (*(++pch) != '\0')
    {
      switch (*pch)
	{
	case ' ':
	case '0':
	  break;
	case 'q':
	case 'Q':
	  return EQUIT;
	  break;
	default:
	  return EQUESTION;
	  break;
	}
    }
  return 0;
}
/*************************************************************
 *       This function changes the case of characters in
 *       change, if it makes it match keep - up to max
 *       characters.
 *       If max == 0, it doesn't check length at all.
 *       returns  OK if they match after fixing (or before).
 *       returns NOT_OK if they don't match.
 *       Note: this function does not put the 'change' string
 *       back the way it was - even if it doesn't match.
 *       Note also:  this function checks characters until it
 *       discovers a '\0' in either input string.  If it's OK
 *       up to there, it considers it to be a match.
 *************************************************************/
OKorNOT
fix_case(char *change, char* keep, short max)
{
  short j;
  for (j = 0; change[j] != '\0' && keep[j] != '\0'&& (max ? j < max: 1) ; j++)
    {
      if (change[j] == keep[j])
	continue;
      else if (change[j] < keep[j])
	{
	  if (change[j] <= 'Z' 
	      && change[j] + 'z' - 'Z' == keep[j])
	    {
	      change[j] += ('z' - 'Z');
	      continue;
	    }
	  return NOT_OK;
	}
      else if (change[j] > keep[j])
	{
	  if (change[j] >= 'a' 
	     && change[j] + 'A' - 'a' == keep[j])
	    {
	      change[j] += ('A' - 'a');
	      continue;
	    }
	  return NOT_OK;
	}
    }
  if (strncmp(change, keep, max) == 0)
    return OK;
  return NOT_OK;
}
/****************************************************
 *     This returns a pointer to the login name of the
 *     user with the uid.  
 *     If DEMO is #defined, and if the uid == 19999
 *     and if there is no user with uid == 19999, "demo"
 *     is returned.
 ****************************************************/
char
*get_name(unsigned long uid)
{
  static char the_name[NAME_LEN+1];
  struct passwd* pwd;
  if ((pwd = getpwuid((unsigned short)uid)) == NULL)
    {
#ifdef DEMO
      if (uid == 19999)
	strcpy(the_name,"demo");
      else
#endif
	sprintf(the_name,"was %lu", uid);
    }
  else
    {
      strcpy(the_name, pwd->pw_name);
    }
  return the_name;
}

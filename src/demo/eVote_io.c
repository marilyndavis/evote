/* $Id: eVote_io.c,v 1.3 2003/01/15 18:54:08 marilyndavis Exp $ */ 
/**********************************************************
 *  eVote_io.c   - input/output routines.
 **********************************************************
 *    Copyright (c) 1994...2015 Deliberate.com Patented.
 *    by Marilyn Davis
 **********************************************************/
#include <stdio.h>
#include"eVote.h"
/************************************************************
 *  This is a puny emulation of Notesfile's GetArg()
 *  function.  The Notesfile function processes
 *  key-aheads.  This is for the demo and testing only.
 *  You want to rewrite this so that it reads your
 *  input stream instead of a raw read with a gets().
 *********************************************************/
char
*GetArg(char *prompt)
{
  static char input[500];
  printf("%s", prompt);
  fgets(input, 500, stdin);
  input[strlen(input) - 1] = '\0';
  return input;
}
/******************************************************
 *   This function translates the static local_id into
 *   the current dropping_id for the item.
 ********************************************************/
short
make_dropping(unsigned long local_id)
{
  short i = 0;
  while (++i <= *p_no_items)
    {
      if (item_info[i].static_id.local_id == local_id)
	return i;
    }
  return 0;
}
/**********************************************************
 *    This returns the identification number that you 
 *    want printed for the user to see.  In the demo, 
 *    it just returns dropping_id).  In your application
 *    you may want to print the number stored
 *    as local_id.  Whatever, fix it here.
 **********************************************************/
short
printing_id(short dropping_id)
{
  return dropping_id;
  /* - * - * - * - * - * - * - * - * - * - * - * - * - * - * 
     Your application may prefer:
     return (short) item_info[dropping_id].static_id.local_id;
     * - * - * - * - * - * - * - * - * - * - * - * - * - * - */
}
/**********************************************************
 *    This returns a pointer to the item's title.
 **********************************************************/
char
*print_title(short dropping_id)
{
  return item_info[dropping_id].eVote.title;
  /* - * - * - * - * - * - * - * - * - * - * - * - * - * - * 
     You may want to pull in the title from your own
     database. 
   * - * - * - * - * - * - * - * - * - * - * - * - * - * - */
  return "No title available";
}

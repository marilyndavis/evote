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

/* $Id: iteml.c,v 1.4 2003/01/15 18:54:08 marilyndavis Exp $ */ 
/**********************************************************
 *
 *     ../eVote/src/Clerklib/iteml.c -- makes and drops items
 *                         and changes the status.
 *
 ***********************************************************
 **********************************************************/
#include <unistd.h>
#include <errno.h>
#include <stdio.h>
#include "Clerkdef.h"
#include "Clerk.h"
#include "msgdef.h"
#include "ipc_msg.h"
#include "string.h"
extern int msgmax;
#ifdef EDEBUG
void output(char *);
extern char debugger[];
extern int edebug;
extern YESorNO trace;
#define FLOWER 8
#endif
/************************************************************
 *  RTYPE send_vstatus(ITEM_INFO *p_item, VSTATUS new_vstatus)
 *      changes the vote status. 
 ************************************************************/
RTYPE 
send_vstatus(ITEM_INFO *p_item, VSTATUS new_vstatus)
{
  char* fix_about_item(ITYPE itype, STATIC_ID* static_id);
  
  char *output;
  
  out_msg_buffer->mtype = PR_CHANGE_VSTATUS;
  output = fix_about_item(CHANGE_VSTATUS, &(p_item->static_id)); 
  sprintf(output, FNE_CHANGE_VSTATUS, NNE_CHANGE_VSTATUS);
  if (send_inst(0,YES)!=OK)
    return FAILURE;
  else 
    return (RTYPE)in_msg_buffer->mtype;		
}
/************************************************************
 *     int drop_items(ITEM_INFO* drop_in, int no_to_drop)
 *     
 *     Drops the items from the conf.  
 *	    Expects drop_in to be an array of consecutive 
 *	    ITEM_INFO's to drop.  It is best to have this array be
 *     a copy of the item_info array rather than to point into 
 *     it because that array could change out from under us.
 *     Returns the number of items dropped.
 *************************************************************/
int 
drop_items(ITEM_INFO* drop_in, int want_to_drop)
{
  int bytes_in;
  int atatime;
  STATIC_ID * drop_out;
  int len;
  int no_dropped = 0;
  int this_bunch;
  
  int drop_stats(char *input);
  
  (void)sprintf(out_msg_buffer->mtext, FNE_DROP_ITEMS, NNE_DROP_ITEMS);
  len = (int)strlen(out_msg_buffer->mtext);
  
  atatime = (msgmax - len - extra_space)/(sizeof (STATIC_ID));
  
  out_msg_buffer->mtype = PR_DROP_ITEMS;
  
  do
    {	
      (void)sprintf(out_msg_buffer->mtext, FNE_DROP_ITEMS, NNE_DROP_ITEMS);
      len = (int)strlen(out_msg_buffer->mtext);
      drop_out = (STATIC_ID*)&out_msg_buffer->mtext[len];
      this_bunch = 0;
      do
	{	/* Only send if it's not grouped or the first 
		   in the group.  The Clerk will drop the rest
		   of the group. */
	  if (drop_in->eVote.more_to_come 
	     == drop_in->eVote.no_in_group - 1)
	    {
	      *drop_out = drop_in->static_id;
	      drop_out++;
	      ++this_bunch;
	    }
	  drop_in++;
	  --want_to_drop;
	}
      while (this_bunch < atatime  && want_to_drop > 0);
      if (send_inst( (char*)drop_out - out_msg_buffer->mtext,NO)
	  != OK)  
	return no_dropped;
      if ((bytes_in = get_msg(YES)) == -1)
	return no_dropped;
      switch(in_msg_buffer->mtype)
	{
	case GOOD:
	  break;
	default:
	  return no_dropped;
	}
      no_dropped += drop_stats(NULL);
    }
  while (want_to_drop >0);				
  return no_dropped;
}
/**************************************************************
 *  OKorNOT eVote_new_items(ITEM_INFO *p_item, int no_to_make)
 *	This is called to make new items.  p_item is a array of
 *	of new items to make, no_to_make long.
 ************************************************************/
OKorNOT 
eVote_new_items(ITEM_INFO *p_item, int no_to_make)
{
  OKorNOT fix_stats(YESorNO grab_msg);
  int atatime;
  int len;
  ITEM_INFO* put_here;
  int this_bunch;
#ifdef EDEBUG
  if (edebug & FLOWER)
    {
      sprintf(debugger,
	      "echo %lu:Appli: pid %d eVote_new_items making %d",
	      who_am_i(), getpid(), no_to_make);
      output(debugger);
    }
#endif
  (void)sprintf(out_msg_buffer->mtext,
		FNE_CREATE_ITEMS,NNE_CREATE_ITEMS);
  len = (int)strlen(out_msg_buffer->mtext);
  atatime = (msgmax - len - extra_space)/sizeof(ITEM_INFO);
  out_msg_buffer->mtype = PR_CREATE_ITEMS;
  do
    {	
      put_here = (ITEM_INFO*)(out_msg_buffer->mtext+len);
      this_bunch = 0;
      do
	{
	  *put_here = *p_item;
	  put_here++;
	  p_item++;
	  --no_to_make;
	}
      while (++this_bunch < atatime && no_to_make > 0);
#ifdef EDEBUG
      if (edebug & FLOWER)
	{
	  sprintf(debugger,
		  "echo %lu:Appli: pid %d eVote_new_items sending this bunch = %d",
		  who_am_i(), getpid(), this_bunch);
	  output(debugger);
	}
#endif
      if (send_inst(sizeof(ITEM_INFO)*this_bunch + len,NO)
	  != OK)							
	return NOT_OK;
#ifdef EDEBUG
      if (edebug & FLOWER)
	{
	  sprintf(debugger,
		  "echo %lu:Appli: pid %d eVote_new_items sending this bunch = %d returned ok.",
		  who_am_i(), getpid(), this_bunch);
	  output(debugger);
	}
#endif
      if (fix_stats(YES) != OK)
	{
	  switch(in_msg_buffer->mtype == NO_ITEM)
	    {
	    case NO_ITEM:
	      fprintf(stderr,"\nNo system resources for new item.\n");
	      break;
	    case REDUNDANT:
	      fprintf(stderr,"\nTitle is redundant: %s.\n", 
		      p_item->eVote.title);
	      break;
	    }
	  return NOT_OK;
	}
#ifdef EDEBUG
      if (edebug & FLOWER)
	{
	  sprintf(debugger,
		  "echo %lu:Appli: pid %d eVote_new_items fixstats ok on this bunch = %d still need %d",
		  who_am_i(), getpid(), this_bunch, no_to_make);
	  output(debugger);
	}
#endif
      /* repack the headings in case we sent an
	 MID_DROPPED since the last send in this loop */
      out_msg_buffer->mtype = PR_CREATE_ITEMS; 
      (void)sprintf(out_msg_buffer->mtext,
		    FNE_CREATE_ITEMS,NNE_CREATE_ITEMS);
      len = (int)strlen(out_msg_buffer->mtext);
    }
  while (no_to_make > 0);				
#ifdef EDEBUG
  if (edebug & FLOWER)
    {
      sprintf(debugger,
	      "echo %lu:Appli: pid %d eVote_new_items leaving happy",
	      who_am_i(), getpid());
      output(debugger);
    }
#endif
  return OK;
}
/*************************************************************
 *    char* fix_about_item(ITYPE itype, STATIC_ID* static_id)
 *
 *       Called by change_vstatus(), how_voted(), i_just_read()
 *       send_vote(), send_stats(), & who_voted().
 *
 *	In the Clerk, these will all be 'AboutItem' 
 *	Instructions so the beginning of the message for 
 *	these are all formatted	the same way.
 *
 *	Returns a pointer to where in the out_msg_buffer the
 *	rest of the message should start.
 *
 ************************************************************/
char* 
fix_about_item(ITYPE itype, STATIC_ID* static_id)
{
  int bytes;
  
  (void)sprintf(out_msg_buffer->mtext, FNE_ABOUT_ITEM, 
		NNE_ABOUT_ITEM);
  bytes = (int)strlen(out_msg_buffer->mtext);
  *(STATIC_ID*)(out_msg_buffer->mtext + bytes) = *static_id;
  return out_msg_buffer->mtext + bytes + sizeof(STATIC_ID);
}

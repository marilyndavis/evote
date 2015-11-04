/* $Id: confl.c,v 1.4 2003/01/15 18:54:08 marilyndavis Exp $ */ 
/**********************************************************
 *	 ../eVote/src/Clerklib/confl.c  -
 *   This is code for keeping track of a conference.
 *   Makes calls to the Clerk.  
 *********************************************************
 *    Copyright (c) 1994...2015 Deliberate.com	Patented.
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the eVote(R)/Clerk License as
 *  published by Deliberate.Com.
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  eVote(R)/Clerk License for more details.
 *  You should have received a copy of the eVote(R)/Clerk License
 *  along with this program in Chapter VII of ../../doc/eVote.doc. 
 *  If not, write to Deliberate.Com 2555 W. Middlefield Rd., #150
 *  Mountain View, CA 94043 USA or office@deliberate.com.
 **********************************************************/
#include <stdio.h>
#include <string.h>
#include "Clerkdef.h"
#include "Clerk.h"
#include "msgdef.h"
#include "ipc_msg.h"
char current_conf[CONFLEN + 1];
void check_length(char *name);
/* The conf the user is currently in - available for export */
extern int msgmax;
/****************************************************************
 *   Here, we take the conf out of memory and onto disk for
 *   continuing another time.  It will be reconvened the first 
 *   time someone enters it with a call to i_am_entering();
 *   Also forces a backup of the data to be made in 
 *   ../eVote/data/backup/X??conf_name.ext 
 ************************************************************/
OKorNOT 
adjourn_conf(char *conf_name)
{
  check_length(conf_name);
  out_msg_buffer->mtype = PR_ADJOURN_CONF;
  sprintf(out_msg_buffer->mtext, FNE_ADJOURN_CONF, NNE_ADJOURN_CONF);
  return send_inst(0, YES);
}
/**************************************************/
void 
check_length(char *name)
{
  if (strlen(name) > CONFLEN -1)
    name[CONFLEN -1] = '\0';
}
/********************************************************/
YESorNO 
does_conf_exist(char* conf_name)
{
  check_length(conf_name);
  out_msg_buffer->mtype = PR_EXIST;
  sprintf(out_msg_buffer->mtext, FNE_EXIST, NNE_EXIST);
  if (send_inst(0, YES) != OK)
    return MAYBE;
  switch (in_msg_buffer->mtype)
    {
    case NO_CONF:
      return NO;
      break;
    case GOOD:
      return YES;
      break;
    default:
      fprintf(stderr,"\nUnrecognized char %ld returned by does_conf_exist.\n",
	      in_msg_buffer->mtype);
      return MAYBE;
    }
  return MAYBE;
}
/**********************************************************
 *   Drops the conf from the database, making backup files 
 ************************************************************/
OKorNOT 
drop_conf(char * conf_name)
{
  check_length(conf_name);
  out_msg_buffer->mtype = PR_DROP_CONF;
  sprintf(out_msg_buffer->mtext, FNE_DROP_CONF, NNE_DROP_CONF);
  if (send_inst(0, YES) != OK)
    return NOT_OK;
  switch (in_msg_buffer->mtype)
    {
    case GOOD:
      return OK;
    case NOT_GOOD:
      return NOT_OK;
    case NO_CONF:
      fprintf(stderr,"\nNo %s conf to drop.\n", conf_name);
      return NOT_OK;
    default:
      fprintf(stderr,"\nUnrecognized char %ld returned by drop_conf.\n",
	      in_msg_buffer->mtype);
      return NOT_OK;
    }
  return OK;
}
/*****************************************************
 *   The named conf is checked.  The ballots are resummed and the
 *   item totals are checked.  Discrepancies are fixed, if possible,
 *   and reported in eVotelog. 
 ************************************************************/
RTYPE 
send_check_conf(char * conf_name)
{
  check_length(conf_name);
  out_msg_buffer->mtype = PR_CHECK_CONF;
  sprintf(out_msg_buffer->mtext, FNE_CHECK_CONF, NNE_CHECK_CONF);
  if (send_inst(0, YES) != OK)
    return FAILURE;
  return (RTYPE)in_msg_buffer->mtype;
}
/***************************************************************
 *       This function starts a new conference. 
 ************************************************************/
RTYPE 
send_eVote_conf(char * conf_name, short drop_days)
{
  out_msg_buffer->mtype = PR_CREATE_CONF;
  check_length(conf_name);
  sprintf(out_msg_buffer->mtext, FNE_CREATE_CONF, NNE_CREATE_CONF);
  if (send_inst(0, YES) != OK)
    {
      return FAILURE;
    }
  return (RTYPE)in_msg_buffer->mtype;
}
/*****************************************************
 *      Makes a call for the uids in conf_name.
 ************************************************************/
RTYPE 
send_whos_in(char * conf_name)
{
  out_msg_buffer->mtype = WHOS_IN;
  sprintf(out_msg_buffer->mtext, FNE_WHOS_IN, NNE_WHOS_IN);
  if (send_inst(0, YES) != OK)
    return FAILURE;
  switch (in_msg_buffer->mtype)
    {
    case STRING_OUT:
    case NO_CONF:
    case UID_LIST:
    case UID_LIST_MORE:
      break;
    default:
      fprintf(stderr, 
	      "\nsend_whos_in on %s: Unexpected return type = %ld.\n"
	      "%s\n",
	      conf_name, in_msg_buffer->mtype, 
	      get_rtype((RTYPE)in_msg_buffer->mtype));
      break;
    }
  return (RTYPE)in_msg_buffer->mtype;
}
/*************************************************************
 *     This should only be called when everything is quiet --
 *     So this call starts the sync happening in cracks, when
 *     nothing else is happening.
 *     This checks all the ballots in the system against the
 *     who file and makes sure that each is credited with
 *     the right number of subscriptions and it drops the
 *     identities of people who aren't here any more.
 *************************************************************/
RTYPE 
send_who_sync(void)
{
  out_msg_buffer->mtype = WHO_SYNC;
  sprintf(out_msg_buffer->mtext, FNE_WHO_SYNC, NNE_WHO_SYNC);
  if (send_inst(0, YES) != OK)
    return FAILURE;
  return (RTYPE)(in_msg_buffer->mtype);
}
#ifdef FUTURE
/**************************************************************
 *   This function will force eVote to drop its notions of what
 *   items are in the conf and use the id_list to remap the
 *   conf.  If id_list contains unrecognized items, they will
 *   become PLAIN items.  If they are recognized, the info and
 *   stats will follow them.  It is expected that id_list[0] is
 *   garbage and id_list[1] has item#1, etc.
 *************************************************************/
OKorNOT 
sync_conf(char* conf_name, ITEM_ID* id_list,
		  short no_items)
{
  int no_sends = 1;
  int atatime, len;
  check_length(conf_name);
  out_msg_buffer->mtype = PR_SYNC_CONF;
  (void)sprintf(out_msg_buffer->mtext, FNE_SYNC_CONF, NNE_SYNC_CONF);
  len = (int)strlen(out_msg_buffer->mtext);
  atatime = (msgmax - len - extra_space)/sizeof(ITEM_ID);
  no_sends = no_items/atatime;
  if (no_items%atatime > 0)
    no_sends++;
  id_list++;	  /* skip that first one */
  do
    {
      if (no_items < atatime)
	atatime = no_items;
				/* redo to get no_sends right */
      (void)sprintf(out_msg_buffer->mtext, FNE_SYNC_CONF, NNE_SYNC_CONF);
      len = (int)out_msg_buffer->mtext;
      memcpy(out_msg_buffer->mtext+len, (char*)id_list,
	     atatime*sizeof(ITEM_ID));
      id_list += atatime;
      no_items -= atatime;
      no_sends--;
      if (send_inst(len + atatime*sizeof(ITEM_ID), YES) != OK)
	return NOT_OK;
      switch (in_msg_buffer->mtype)
	{
	case GOOD:
	  continue;
	  break;
	default:
	  printf("\nSync_info:Unexpected return %d.\n", in_msg_buffer->mtype);
	  return NOT_OK;
	}
    }
  while (no_items > 0);
  return OK;					
}
#endif

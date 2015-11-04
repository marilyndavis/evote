/* $Id: queriesl.c,v 1.3 2003/01/15 18:54:08 marilyndavis Exp $ */ 
/**********************************************************
 *	 ../eVote/src/Clerklib/queriesl.c  -
 *   Functions to query the data about who voted and how they voted.
 *********************************************************
 **********************************************************/
#include </usr/include/pwd.h>
#include <stdio.h>
#include "Clerkdef.h"
#include "msgdef.h"
#include "ipc_msg.h"
/*****************************************************************
 *  RTYPE send_how  - asks the Clerk how how_uid voted.
 *  If successful, places the answer in ret_vote.  If it's a
 *  grouped question, it also places the sum for the group in
 *  ret_sum.
 *****************************************************************/	
RTYPE 
send_how(ITEM_INFO* p_item, unsigned long how_uid, 
	 short *ret_vote, short *ret_sum)
{
  RTYPE cc;
  char* fix_about_item(ITYPE itype, STATIC_ID* static_id);
  short the_vote;
  char *rest;
  short vote_sum;
  rest = fix_about_item(HOW_VOTED, &(p_item->static_id));
  out_msg_buffer->mtype = PR_HOW_VOTED;
  sprintf(rest, FNE_HOW_VOTED, NNE_HOW_VOTED);
  if (send_inst(0, YES) == NOT_OK)
    return FAILURE;
  switch (cc = (RTYPE)in_msg_buffer->mtype)
    {
    case NO_ITEM:
      return FAILURE;
    case NO_VOTER:
    case STRING_OUT:
      return cc;
      break;
    default:
      fprintf(stderr,"\nsend_how: Unexpected return :%d\n", cc);
      return FAILURE;
    case GOOD:
      sscanf(in_msg_buffer->mtext, FEN_HOW_VOTED, ENN_HOW_VOTED);
      *ret_vote = the_vote;
      if (p_item->eVote.type >= GROUPED)
	{
	  *ret_sum = vote_sum;
	}
    }
  return cc;
}
/*****************************************************************
 *  RTYPE send_timestamp  - asks the Clerk when how_uid voted.
 *****************************************************************/	
time_t 
send_timestamp(ITEM_INFO* p_item, unsigned long how_uid)
{
  RTYPE cc;
  char* fix_about_item(ITYPE itype, STATIC_ID* static_id);
  char *rest;
  time_t when;
  if (p_item->eVote.type != TIMESTAMP)
    return 0L;
  rest = fix_about_item(SEND_STAMP, &(p_item->static_id));
  out_msg_buffer->mtype = PR_SEND_STAMP;
  sprintf(rest, FNE_SEND_STAMP, NNE_SEND_STAMP);
  if (send_inst(0, YES) == NOT_OK)
    return 0;
  switch (cc = (RTYPE)in_msg_buffer->mtype)
    {
    case NO_ITEM:
      return 0;
    case NO_VOTER:
    case STRING_OUT:
      return 0;
      break;
    default:
      fprintf(stderr,"\nsend_stamp: Unexpected return :%d\n", cc);
      return 0;
    case GOOD:
      sscanf(in_msg_buffer->mtext, FEN_SEND_STAMP, ENN_SEND_STAMP);
      return when;
    }
}
/*******************************************************
 *  Asks who has signed the petition
 ********************************************************/
RTYPE 
who_signed(ITEM_INFO* p_item)
{
  char *fix_about_item(ITYPE itype, STATIC_ID* static_id);
  char *rest;
  rest = fix_about_item(WHO_SIGNED, &(p_item->static_id));
  out_msg_buffer->mtype = PR_WHO_SIGNED;
  sprintf(rest, FNE_WHO_SIGNED, NNE_WHO_SIGNED);
  if (send_inst(0, YES) != OK)
    return FAILURE;
  switch (in_msg_buffer->mtype) 
    {
    case STRING_OUT:
    case UID_LIST:
    case UID_LIST_MORE:
      break;
    case NO_ITEM:
      return FAILURE;
      break;
    default:
      fprintf(stderr,"\nwho_signed: unexpected return type = %ld\n", 
	      in_msg_buffer->mtype);
      return FAILURE;
      break;
    }
  return (RTYPE)in_msg_buffer->mtype;
}
/*******************************************************
 *  Asks a question like who-voted <3 on the item. 
 *  inin is the "<3" string.  
 ********************************************************/
RTYPE 
who_voted(ITEM_INFO* p_item, char *inin)
{
  char *fix_about_item(ITYPE itype, STATIC_ID* static_id);
  char inout[QUESTLEN];
  char *rest;
  int i = -1, j = -1;
  /* collapse the white space from the question */
  do
    {
      if (inin[++i] == ' ' || inin[i] == '\t')
	continue;
      if (++j >= QUESTLEN)
	{
	  return TOO_LONG; 
	}
      if (inin[i] >= 'a' && inin[i] <= 'z')
	inout[j] = inin[i] + ('A'-'a');
      else
	inout[j] = inin[i];
    }
  while (inin[i] != '\0');
  rest = fix_about_item(WHO_VOTED, &(p_item->static_id));
  out_msg_buffer->mtype = PR_WHO_VOTED;
  sprintf(rest, FNE_WHO_VOTED, NNE_WHO_VOTED);
  if (send_inst(0, YES) != OK)
    return FAILURE;
  switch (in_msg_buffer->mtype) 
    {
    case STRING_OUT:
    case UID_LIST:
    case UID_LIST_MORE:
      break;
    case NO_ITEM:
      return FAILURE;
      break;
    default:
      fprintf(stderr,"\nwho_voted: unexpected return type = %ld\n", 
	      in_msg_buffer->mtype);
      return FAILURE;
      break;
    }
  return (RTYPE)in_msg_buffer->mtype;
}

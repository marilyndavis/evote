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

/* $Id: mtype.c,v 1.3 2003/01/15 18:54:08 marilyndavis Exp $ */ 
/**********************************************************
 *  ../eVote/src/Clerklib/mtype.c  -
 *  two functions that return strings that
 *  describe the message type.
 *  This file is #included in both the application
 *  side and in the Clerk.
 *********************************************************
 *********************************************************
 *  This one module is #included into the Clerk: util.cc  
 *********************************************************/
#include <stdio.h>
/*****************************************************************
 *  Returns a description string for an incoming
 *  message (to the Clerk).
 *****************************************************************/
char * 
get_itype(ITYPE itype)
{
  static char msg[200];
  static int count = 0;
  switch (itype)
    {
    case ADJOURN_CONF:
      return "ADJOURN_CONF";
    case CHANGE_ACTION:
      return "CHANGE_ACTION";
    case CHANGE_VSTATUS:
      return "CHANGE_VSTATUS";
    case CHECK_CONF:
      return "CHECK_CONF";
    case CREATE_CONF:
      return "CREATE_CONF";
    case CREATE_ITEMS:
      return "CREATE_ITEMS";
    case DO_DEBUG:
      return "DO_DEBUG";
    case DOWN_PRIORITY:
      return "DOWN_PRIORITY";
    case DROP_CONF:
      return "DROP_CONF";
    case DROP_OLDQS:
      return "DROP_OLDQS";
    case DROP_ITEMS:
      return "DROP_ITEMS";
    case DROP_VOTER:
      return "DROP_VOTER";
    case ENTERING:
      return "ENTERING";
    case EXIST:
      return "EXIST";
    case FLUSH:
      return "FLUSH";
    case GROW_CONF:
      return "GROW_CONF";
    case HELLO:
      return "HELLO";
    case HOW_VOTED:
      return "HOW_VOTED";
    case I_READ:
      return "I_READ";
    case JOINING:
      return "JOINING";
    case LEAVING:
      return "LEAVING";
    case MID_DROPPED:
      return "MID_DROPPED";
    case MOVE:
      return "MOVE";
    case NEW_EXE:
      return "NEW_EXE";
    case NEW_LOG:
      return "NEW_LOG";
    case PULL_TIME:
      return "PULL_TIME";
    case PUSH_TIME:
      return "PUSH_TIME";
    case QUIT:
      return "QUIT";
    case REORDER_CONF:
      return "REORDER_CONF";
    case SEND_STAMP:
      return "SEND_STAMP";
    case SEND_STATS:
      return "SEND_STATS";
    case SEND_VOTE:
      return "SEND_VOTE";
    case SYNC_CONF:
      return "SYNC_CONF";
    case UID_EXIST:
      return "UID_EXIST";
    case UP_PRIORITY:
      return "UP_PRIORITY";
    case WHO_DROP:
      return "WHO_DROP";
    case WHO_NUM:
      return "WHO_NUM";
    case WHO_IS:
      return "WHO_IS";
    case WHO_SYNC:
      return "WHO_SYNC";
    case WHO_VOTED:
      return "WHO_VOTED";
    case WHOS_IN:
      return "WHOS_IN";
    default:
      sprintf(msg,"Unknown ITYPE = %d ", itype);
      if (++count < 3)
	{
	  sprintf(msg,"%s comes after %s ",
		  get_itype((ITYPE)(itype -1)), get_itype((ITYPE)itype));
	  sprintf(msg,"%s and before %s",
		  get_itype((ITYPE)(itype +1)), get_itype((ITYPE)itype));
	}
      return msg;
    }
  return NULL;
}
/*****************************************************************
 *   returns a descriptive string for the return message
 *   type (from the Clerk).
 *****************************************************************/
char * 
get_rtype(RTYPE rtype)
{
  static char msg[200];
  static int count;
  switch (rtype)
    {
    case FAILURE:     /* Can't use 0 in message queue */
      return "FAILURE";
    case	DROP_STATS:  /* Unsolicited returns come first */
      return "DROP_STATS";
    case NEW_MID:
      return "NEW_MID";
    case NEW_VSTATUS:
      return "NEW_VSTATUS";
    case REDO_STATS:
      return "REDO_STATS";
    case RESEND:
      return "RESEND";
    case DEL_DONE:  /*  Notice that each DEL_* is followed by *. */
      return "DEL_DONE";
    case DONE:        /*  It's important to preserve this order.   */
      return "DONE";    
      /*  DEL_*  means * and the message queue     */
    case DEL_GOOD: 
      return "DEL_GOOD";  /*  was deleted so don't try to use it.      */
    case GOOD:     
      return "GOOD";        /*  DEL_*  means * and the RTYPE             */
    case MORE_STATS:  /*  MORE_ Used when group changes on a vote  */
      return "MORE_STATS"; 
    case MQ_OK:      
      return "MQ_OK";    /*  This is interpreted by send_inst(YES),   */
    case MQ_OK_VAC:
      return "MQ_OK_VAC";
    case NEW_STAT:  
      return "NEW_STAT";
    case NEW_STATS: 
      return "NEW_STATS";
    case DEL_NEW_VOTER:
      return "DEL_NEW_VOTER";
    case NEW_VOTER:
      return "NEW_VOTER";
    case NO_CHANGE:
      return "NO_CHANGE";
    case DEL_NO_CONF:
      return "DEL_NO_CONF";
    case NO_CONF:
      return "NO_CONF";
    case NO_MODS:
      return "NO_MODS";
    case NO_ITEM:
      return "NO_ITEM";
    case DEL_NO_VOTER:
      return "DEL_NO_VOTER";
    case NO_VOTER:
      return "NO_VOTER";
    case NOT_ALLOWED:
      return "NOT_ALLOWED";
    case DEL_NOT_GOOD:
      return "DEL_NOT_GOOD";
    case NOT_GOOD:
      return "NOT_GOOD";
    case DEL_ON_LINE:
      return "DEL_ON_LINE";
    case ON_LINE:
      return "ON_LINE";
    case DEL_ON_TWICE:
      return "DEL_ON_TWICE";
    case ON_TWICE:
      return "ON_TWICE";
    case DEL_REDUNDANT:
      return "DEL_REDUNDANT";
    case REDUNDANT:
      return "REDUNDANT";
    case PROGRAMMER_ERROR:
      return "PROGRAMMER_ERROR";
    case DEL_STRING_OUT:
      return "DEL_STRING_OUT";
    case STRING_OUT:
      return "STRING_OUT";
    case TOO_LONG:
      return "TOO_LONG";
    case UID_LIST:
      return "UID_LIST";
    case UID_LIST_MORE:
      return "UID_LIST_MORE";
    case DEL_UID_LIST:
      return "DEL_UID_LIST";
    case VIOLATES_SUM:
      return "VIOLATES_SUM";
    default:
      sprintf(msg,"Unknown RTYPE = %d ", rtype);
      if (++count < 3)
	{
	  sprintf(msg,"%s comes after %s ",
		  get_rtype((RTYPE)(rtype -1)), get_rtype((RTYPE)rtype));
	  sprintf(msg,"%s and before %s",
		  get_rtype((RTYPE)(rtype +1)), get_rtype((RTYPE)rtype));
	}
      return msg;
    }
  return NULL;
}

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

/* $Id: Clerkdef.h,v 1.4 2003/01/15 18:54:08 marilyndavis Exp $ */ 
#ifndef Clerkdef_h
#define Clerkdef_h
/**********************************************************
 *   ../eVote/src/Clerk/Clerkdef.h  - one of three files 
 *               shared by the application and by
 *               eVote_Clerk.  The others are ./msgdef.h
 *               ../eVote_defaults.h
 *********************************************************
 **********************************************************
 *  eVote's shared values and structures.
 **********************************************************/
#include<sys/types.h>
/********************************************************
 *	STATIC_ID  
 ********************************************************/
struct static_id_def
{
  unsigned long network_id;
  unsigned long local_id;
};
typedef struct static_id_def STATIC_ID;
/********************************************************
 *   The following enum's shouldn't be changed.  However,
 *   you might want to study them.
 ********************************************************/
/********************************************************
 *  VOTE_TYPE  enum - voted on? numeric vote? grouped?
 ********************************************************/
typedef enum 
{
  ZIP, 
  PLAIN, 			/* no vote */
  TALLIED, 		/* generic tallied*/
  TALLIEDY, 	/* tallied yes/no */
  TALLIEDN,		/* tallied numeric*/
  GROUPED,		/* generic grouped*/
  GROUPEDY,		/* grouped yes/no */
  GROUPEDn,   /* grouped numeric, max < 10 */
  GROUPEDN,		/* grouped numeric, max >= 10 */
  TIMESTAMP   /* petition == YES/NO with timestamp */
} VOTE_TYPE;
/********************************************************
 *   VSTATUS enum - VoteSTATUS - still voting? voting 
 *	  		 closed? stats unseen?
 ********************************************************/
typedef enum 
{ 
  NOT_KNOWN, 	/* for initialization */
  UNSEEN,     /* unseen stats */
  OPEN, 			/* voting allowed*/
  CLOSED  		/* no more voting*/
} VSTATUS;
/********************************************************
 *   PRIV_TYPE enum - can we see others' votes?
 ********************************************************/
typedef enum 
{ 
  PRIVATE,		/* secret votes */
  PUBLIC,			/* public vote */
  IF_VOTED		/* who has voted */
} PRIV_TYPE;
/********************************************************
 *
 *	eVOTE - data structure.  Stores vote info for an item.
 *
 ********************************************************/
#define TITLEN 36          /* length of the title in demo */
struct eVote_def
{
  VOTE_TYPE type;
  time_t open;
  time_t close;
  VSTATUS vstatus;  
  unsigned long participants_when_closed;
  PRIV_TYPE priv_type;
  short max; 	/* maximum vote, <= 99 */
  short min;	/* minimum vote, >= 0 */
  short no_in_group;	
  short more_to_come;	/*	how many more items 
				are in this group? */
  short sum_limit; 		/*	maximum that votes in 
					this group may add to. */
  unsigned long author;              
  char title[TITLEN + 1];   /* title - for demo  */	
};
typedef struct eVote_def eVOTE;
/********************************************************
 * ITEM_INFO data structure.  Stores everything.
 ********************************************************/
struct item_info_def     /* maintained by ipc_shm.c */
{
  short dropping_id;
  STATIC_ID static_id;
  eVOTE eVote;
};
typedef struct item_info_def ITEM_INFO;
/********************************************************
 *	ITEM_STAT data structure - stores current stats
 *	for an item.  Don't access these data directly
 *	but let char* get_stats(short dropping_id) do it
 *	for you.
 ********************************************************/
#define STXTLEN (26)	/*	length of the statistics string 
				for an item not including the 
				'\0  -- in msgdef.h 
				FEN_..._STAT! */
struct item_stat_def	/*	Maintained by stats.c */
{
  short dropping_id;
  
  unsigned long voters;
  unsigned long pos_voters;
  unsigned long neg_voters;
  float pos;
  float neg;
  float sum_squares;
  short my_sum;   /* my sum for first of group */
  char vote_str[5];
  char text[STXTLEN + 1];
};	
typedef struct item_stat_def ITEM_STAT;
/**********************************************************
 *   Note that sizeof(ITEM_STAT) < sizeof(ITEM_INFO).
 *   It is important.  If it needs to change so that
 *   ITEM_INFO is bigger, ItemList::create() needs
 *   fixing.
 ***********************************************************/
/**********************************************************
 *		Favorite enums.
 **********************************************************/   
typedef enum {ZILCH, MAJORDOMO, MAILMAN} LISTSERVE;
typedef enum {NOT_OK, OK, UNDECIDED, PROBLEM, STOP,CANT} OKorNOT;
typedef enum {NO, YES, MAYBE, PUNT, DROPIT} YESorNO;
/**********************************************************
 *  Other doodads.
 **********************************************************/
#define QUESTLEN (8)  /* max length of a question "<=!ACC" */ 
#define NOQ         (-1)
#define PATHLEN (128)
/*********************************************************
 *   RTYPE enum - return types for messages from the Clerk.
 *********************************************************/
typedef enum
{
  FAILURE,     /* Can't use 0 in message queue */
  DROP_STATS,  /* Unsolicited returns come first */
  NEW_MID,
  NEW_VSTATUS,
  REDO_STATS,
  RESEND,
  DEL_DONE,  /*  Notice that each DEL_* is followed by *. */
  DONE,        /*  It's important to preserve this order.   */
  /*  DEL_*  means * and the message queue     */
  DEL_GOOD,  /*  was deleted so don't try to use it.      */
  GOOD,        /*  DEL_*  means * and the RTYPE             */
  /*  is incremented for further study later.  */
  MORE_STATS,  /*  MORE_ Used when group changes on a vote  */
  MQ_OK,       /*  This is interpreted by send_inst(YES),   */
  MQ_OK_VAC,   /* 12 */
  NEW_STAT,  
  NEW_STATS, 
  DEL_NEW_VOTER, 
  NEW_VOTER,
  NO_CHANGE,
  DEL_NO_CONF,
  NO_CONF,
  NO_MODS,
  NO_ITEM,
  DEL_NO_VOTER,
  NO_VOTER,
  NOT_ALLOWED,
  DEL_NOT_GOOD,
  NOT_GOOD,
  DEL_ON_LINE,
  ON_LINE,
  DEL_ON_TWICE,
  ON_TWICE,
  DEL_REDUNDANT,
  REDUNDANT,
  PROGRAMMER_ERROR,
  DEL_STRING_OUT,
  STRING_OUT,
  TOO_LONG,
  UID_LIST_MORE,
  DEL_UID_LIST,
  UID_LIST,
  VIOLATES_SUM
} RTYPE;     /* return message types */
typedef enum {EVERYTHING = 0, 
              SIGNER = 1, 
              READ_ONLY = 2, /* No voting allowed */
	      VOTE_ONLY = 4,
	      VACATION = 8,  /* Keeps ballot but no action allowed */
              UNSET = 16,
	      /*	      LOCK = 128,  */
	      DROPPING = 256, DROP = 512, LOCAL = 1024, 
	      /* Those are for the mechanics */
} ACTION;
/* votes besides real votes */
#define UNKNOWN             (120)
#define NOT_READ	(121)
#define READ		(122)
#define NO_MOD_ALLOWED		(123)
#define WAS_ARCHIVE         (124)
#define OUTSIDE_LIMITS			(125)
#define UNREAD_MIN          (126)
#define LATE                (127)
#define NOT_VALID           (128)
#define NOT_REAL            (136)
#define MAX_POS             (UNKNOWN-1)
#define MIN_NEG             (NOT_REAL+1-256)
#define NO_LIMIT            (32767)
#define CONFLEN	80		 /* longest possible conference
				    name - not including the path */
#define MY_HOST_ID    (1l) /* Host id for this machine */

#endif


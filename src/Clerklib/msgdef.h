/* $Id: msgdef.h,v 1.3 2003/01/15 18:54:08 marilyndavis Exp $ */ 
/**********************************************************
 *   msgdef.h  - one of three files shared by the application 
 *               and by eVote_Clerk.  The others are
 *               Clerkdef.h and ../eVote_defaults.h
 *********************************************************
 *    Copyright (c) 1994...2015 Deliberate.com
 *		Patented.
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
/*********************************************************
 *   IT IS PROBABLY UNNECESSARY TO CHANGE ANYTHING!!
 ********************************************************/
/********************************************************
 *   The following enum's shouldn't be changed.  However,
 *   you might want to study them.
 ********************************************************/
#ifndef _msgdef_h
#define _msgdef_h
#define FLOW "../port/flow"  
/* file for both 
   sides to write to show the message flow 
   between eVote and caller - needs EDEBUG  
   but the file deletes at startup even if there's no EDEBUG */
/*********************************************************
 *		ITYPE - Incoming message TYPE.
 *   Message types that eVote_Clerk understands.
 **********************************************************/
typedef enum
{
  ADJOURN_CONF,
  CHANGE_ACTION,
  CHANGE_VSTATUS,
  CHECK_CONF,
  CREATE_CONF,	
  CREATE_ITEMS,
  DO_DEBUG,
  DOWN_PRIORITY,
  DROP_CONF,
  DROP_OLDQS,
  DROP_ITEMS,
  DROP_VOTER,
  ENTER_ADMIN,
  ENTERING,
  EXIST,
  FLUSH,
  GROW_CONF,
  HELLO,
  HOW_VOTED,
  I_READ,
  JOINING,
  LEAVE_ADMIN,
  LEAVING,
  MID_DROPPED,  		
  MOVE,
  NEW_EXE,
  NEW_LOG,
  PULL_TIME,
  PUSH_TIME,
  QUIT,
  REORDER_CONF,
  SEND_STAMP,
  SEND_STATS,
  SEND_VOTE,
  SYNC_CONF,
  UID_EXIST,
  UP_PRIORITY,
  WHO_DROP,
  WHO_NUM,
  WHO_IS,
  WHO_SIGNED,
  WHO_SYNC,
  WHO_VOTED,
  WHOS_IN
} ITYPE;      /* message types into the eVote */
/* These are the message queue priorities 
   assigned to each instruction */
#define PR_ADJOURN_CONF   10l
#define PR_CHANGE_ACTION  10l
#define PR_CHANGE_VSTATUS 10l
#define PR_CHECK_CONF     10l
#define PR_CREATE_CONF    10l
#define PR_CREATE_ITEMS   10l
#define PR_DO_DEBUG        1l
#define PR_DOWN_PRIORITY  10l 
#define PR_DROP_CONF      10l
#define PR_DROP_OLDQS     1l
#define PR_DROP_ITEMS     10l
#define PR_DROP_VOTER     10l
#define PR_EMERGENCY1     1l
#define PR_EMERGENCY2     5l
#define PR_ENTER_ADMIN    40l
#define PR_ENTERING       10l
#define PR_EXIST          10l
#define PR_FLUSH          1l
#define PR_GROW_CONF      5l
#define PR_HELLO          1l
#define PR_HOW_VOTED      10l
#define PR_I_READ         10l
#define PR_JOINING        10l
#define PR_LEAVE_ADMIN    10l
#define PR_LEAVING        10l
#define PR_MID_DROPPED    10l
#define PR_MOVE           10l
#define PR_NEW_EXE        10l
#define PR_NEW_LOG        1l
#define PR_NEW_MID        10l
#define PR_PULL_TIME      10l
#define PR_PUSH_TIME      10l
#define PR_QUIT           2l
#define PR_REORDER_CONF   5l
#define PR_SEND_STAMP     10l
#define PR_SEND_STATS     10l
#define PR_SEND_VOTE      10l
#define PR_SYNC_CONF      1l
#define PR_UID_EXIST      10l
#define PR_UP_PRIORITY    1l
#define PR_WHO_DROP       10l
#define PR_WHO_NUM        10l
#define PR_WHO_IS         10l
#define PR_WHO_SIGNED     40l
#define PR_WHO_SYNC       40l
#define PR_WHO_VOTED      10l
#define PR_WHOS_IN        10l
/**********************************************************
 *  These are the format strings for each message 
 *  FNE_	are the format strings for messages from notes to 
 *				the eVote_Clerk.
 *  NNE_	are the arguments for filling the format strings 
 *				in the application code.
 *  NEE_	are the args for receiving in the eVote_Clerk.
 *  FEN_	are the format strings for return messages.
 *  EEN_	are args for sending return messages.
 *  ENN_	are the args for receiving return messages in 
 *				the application.
 *  The E's stand for eVote, the N's stand for Notesfile, 
 *  the application eVote was initially designed for.
 *********************************************************/
#define FNE_ABOUT_ITEM   "%3d "
#define NNE_ABOUT_ITEM		itype
#define NEE_ABOUT_ITEM   &(int)_itype
#define FNE_ABOUT_CONF	  "%3d %lu %s "
#define NEE_ABOUT_CONF  	&(int)_itype,&_uid,_conf_name
#define FNE_ADJOURN_CONF	"%3d %lu %s Q!"
#define NNE_ADJOURN_CONF	ADJOURN_CONF,who_am_i(),conf_name
#define NEE_ADJOURN_CONF	&(int)_itype,&_uid,_conf_name
#define FNE_CHANGE_ACTION "%3d %d Q!"
#define NNE_CHANGE_ACTION CHANGE_ACTION, new_action
#define NEE_CHANGE_ACTION &(int)_itype, &(int)new_action
#define FNE_CHANGE_VSTATUS " %d %lu %lu Q!"
#define NNE_CHANGE_VSTATUS new_vstatus, who_am_i(), voter()
#define NEE_CHANGE_VSTATUS (int*)&new_vstatus, &uid_asking, &_voter
#define FNE_CHECK_CONF 		"%3d %lu %s Q!"
#define NNE_CHECK_CONF		CHECK_CONF,who_am_i(),conf_name
#define NEE_CHECK_CONF		&(int)_itype, &_uid, _conf_name
#define FNE_CREATE_CONF	"%3d %lu %s %hd Q!"
#define NNE_CREATE_CONF	CREATE_CONF,who_am_i(),conf_name,drop_days
#define NEE_CREATE_CONF	&(int)_itype,&_uid,_conf_name,&drop_day
#define FNE_CREATE_ITEMS "%3d > "
#define NNE_CREATE_ITEMS CREATE_ITEMS
#define NEE_CREATE_ITEMS &(int)_itype
#define FNE_DOWN_PRIORITY	 "%3d %lu Q!"
#define NNE_DOWN_PRIORITY  DOWN_PRIORITY,who_am_i()
#define NEE_DOWN_PRIORITY	 &(int)_itype,&_uid
#define FNE_DO_DEBUG "%3d %lu %d Q!"
#define NNE_DO_DEBUG DO_DEBUG,who_am_i(),level
#define NEE_DO_DEBUG &(int)_itype, &_uid, &level
#define FNE_DROP_CONF		"%3d %lu %s Q!"
#define NNE_DROP_CONF		DROP_CONF,who_am_i(),conf_name
#define NEE_DROP_CONF		&(int)_itype,&_uid,_conf_name
#define FNE_DROP_OLDQS		"%3d %lu Q!"
#define NNE_DROP_OLDQS		DROP_OLDQS,who_am_i()
#define NEE_DROP_OLDQS		&(int)_itype,&_uid
#define FNE_DROP_ITEMS	"%3d > "
#define NNE_DROP_ITEMS	DROP_ITEMS
#define NEE_DROP_ITEMS  &(int)_itype
#define FEN_DROP_STATS	"%3hd %3hd Q!"
#define EEN_DROP_STATS	low, high
#define ENN_DROP_STATS	&from,&to
#define FNE_DROP_VOTER	"%3d %lu %s %lu %d Q!"
#define NNE_DROP_VOTER	DROP_VOTER,who_am_i(),conf_name,uid_to_drop,only_if_non_voter
#define NEE_DROP_VOTER	&(int)_itype,&_uid,_conf_name,&uid_to_drop,(int*)&only_if_non_voter
#define FNE_ENTER_ADMIN	"%3d %lu %s %lu Q!"
#define NNE_ENTER_ADMIN	ENTER_ADMIN,who_am_i(),conf_name,voter()
#define NEE_ENTER_ADMIN	&(int)_itype,&_uid,_conf_name,&_voter
#define FEN_ENTER_ADMIN  "%d %lu Q!"
#define EEN_ENTER_ADMIN  _memid, no_of_participants
#define ENN_ENTER_ADMIN  &new_mid, &no_of_participants
#define FNE_ENTERING	"%3d %lu %s %lu Q!"
#define NNE_ENTERING	ENTERING,who_am_i(),conf_name,voter()
#define NEE_ENTERING	&(int)_itype,&_uid,_conf_name,&_voter
#define FEN_ENTERING  "%d %lu %ld %d Q!"
#define EEN_ENTERING  _memid, no_of_participants, _p_voter->last_access(), action
#define ENN_ENTERING  &new_mid, &no_of_participants, &last_access, &current_action
#define FNE_EXIST "%3d %lu %s Q!"
#define NNE_EXIST EXIST,who_am_i(),conf_name
#define NEE_EXIST &(int)_itype, &uid, _conf_name
#define FNE_FLUSH			 "%3d %lu Q!"
#define NNE_FLUSH				FLUSH,who_am_i()
#define NEE_FLUSH				&(int)_itype,&_uid
#define FNE_GROW_CONF "%3d %lu %s Q!"
#define NNE_GROW_CONF	GROW_CONF,who_am_i(),conf_name
#define NEE_GROW_CONF	&(int)_itype,&_uid,_conf_name
#define FNE_HELLO           "%3d %lu Q!"
#define NNE_HELLO           HELLO,who_am_i()
#define NEE_HELLO           &(int)_itype,&_uid
#define FNE_HOW_VOTED				" %lu Q!"
#define NNE_HOW_VOTED				how_uid
#define NEE_HOW_VOTED				&how_uid
#define FEN_HOW_VOTED 			"%hd %hd Q!"
#define EEN_HOW_VOTED				the_vote, vote_sum
#define ENN_HOW_VOTED				&the_vote, &vote_sum
#define FNE_I_READ					" Q!"
#define NNE_I_READ					
#define FNE_SEND_STAMP				" %lu Q!"
#define NNE_SEND_STAMP				how_uid
#define NEE_SEND_STAMP				&how_uid
#define FEN_SEND_STAMP 			"%ld Q!"
#define EEN_SEND_STAMP				when
#define ENN_SEND_STAMP				&when
#define FNE_SEND_VOTE					" %hd Q!"
#define NNE_SEND_VOTE					vote
#define NEE_SEND_VOTE					&vote
#define FNE_JOINING	"%3d %lu %s %lu %d Q!"
#define NNE_JOINING	JOINING,who_am_i(),conf_name, voter(), local
#define NEE_JOINING	&(int)_itype,&_uid,_conf_name,&_voter,(int*)&local
#define FNE_LEAVE_ADMIN   "%3d Q!"
#define NNE_LEAVE_ADMIN  LEAVE_ADMIN
#define NEE_LEAVE_ADMIN	 &(int)_itype
#define FNE_LEAVING   "%3d Q!"
#define NNE_LEAVING		LEAVING
#define NEE_LEAVING		&(int)_itype
#define FNE_MID_DROPPED "%3d %d "
#define NNE_MID_DROPPED MID_DROPPED, memid
#define NEE_MID_DROPPED &(int)_itype, &mid
#define FNE_MOVE		"%3d %lu %s %s Q!"
#define NNE_MOVE		MOVE,who_am_i(), was, is
#define NEE_MOVE		&(int)_itype,&_uid, was, is
/* All instructions that need a return q start with this format */
#define FNE_NEEDSQ     "%3d %lu "
#define NEE_NEEDSQ     &(int)_itype,&_uid
#define FNE_NEW_EXE			 "%3d %lu Q!"
#define NNE_NEW_EXE				NEW_EXE,who_am_i()
#define NEE_NEW_EXE				&(int)_itype,&_uid
#define FNE_NEW_LOG			 "%3d %lu Q!"
#define NNE_NEW_LOG				NEW_LOG,who_am_i()
#define NEE_NEW_LOG				&(int)_itype,&_uid
#define FEN_NEW_MID		"%d %lu Q!"
#define EEN_NEW_MID		_memid, no_of_participants
#define ENN_NEW_MID		&new_mid, &no_of_participants
#define FNE_PULL_TIME				" Q!"
#define NNE_PULL_TIME				
#define NEE_PULL_TIME				
#define FEN_PULL_TIME 			" %ld Q!"
#define EEN_PULL_TIME				time_stamp
#define ENN_PULL_TIME				&time_stamp
#define FNE_PUSH_TIME				" %ld Q!"
#define NNE_PUSH_TIME				new_time
#define NEE_PUSH_TIME				&new_time
#define FEN_PUSH_TIME 			"%ld Q!"
#define EEN_PUSH_TIME				old_time
#define ENN_PUSH_TIME				&old_time
#define FEN_NEW_VOTER		"%hd Q!"
#define EEN_NEW_VOTER		drop_days
#define ENN_NEW_VOTER		&drop_days
#define FEN_ON_TWICE    "%d Q!"
#define EEN_ON_TWICE    trouble
#define ENN_ON_TWICE    &trouble
/*  Plain items, not tallied */
#define FEN_STAT  "%3lu  -    - "
#define EEN_STAT  _readers
#define FENN_STAT  "%*s",STXLEN
#define ENN_STAT  stat[i].text
#define FEN_NEW_VSTATUS "%d Q!"
#define EEN_NEW_VSTATUS lid
#define ENN_NEW_VSTATUS &lid
/* Tallied Yes/No items - max vote ==1, min vote == 0 */
#define FEN_TSTATY  "%3lu %3lu%5s"
#define EEN_TSTATY  _readers,_voters,sum_str
/*  with vote */
#define FEN_TSTATYV "%3lu %3lu%5s %3hd %3hd Q!"
#define EEN_TSTATYV _readers,_voters,sum_str,vote,old_vote
#define FNN_TSTATYV "%lu %lu %s %hd %hd Q!"
#define ENN_TSTATYV &readers,&voters,sum_str,&return_vote,&old_vote
/* Tallied numeric items */
#define FEN_TSTAT  "%3lu %3lu%5s"
#define EEN_TSTAT  _readers,_voters,ave_str
/* with vote */
#define FEN_TSTATV "%3lu %3lu%5s %3hd %3hd Q!"
#define EEN_TSTATV _readers,_voters,ave_str,vote,old_vote
#define FNN_TSTATV " %lu %lu %s %hd %hd Q!"
#define ENN_TSTATV &readers,&voters,ave_str,&return_vote,&old_vote
/* Grouped yes/no  */
#define FEN_GSTATY  "%3lu%4s%5s"
#define EEN_GSTATY  _readers,vote_str,sum_str
/*  with vote */
#define FEN_GSTATYV "%3lu%4s%5s %3lu %3hd %3hd Q!"
#define EEN_GSTATYV _readers,vote_str,sum_str,_voters,my_sum,old_vote
#define FNN_GSTATYV "%lu %s %s %lu %hd %hd Q!" 
#define ENN_GSTATYV &readers,vote_str,sum_str,&voters,&my_sum,&old_vote
/* Grouped numeric */
#define FEN_GSTAT  "%3lu%4s%5s"
#define EEN_GSTAT  _readers,vote_str,ave_str
/* with vote */
#define FEN_GSTATV "%3lu%4s%5s , %3lu %3hd %3hd Q!"
#define EEN_GSTATV _readers,vote_str,ave_str,_voters,my_sum,old_vote
#define FNN_GSTATV "%lu %s %s , %lu %hd %hd Q!" 
#define ENN_GSTATV &readers,vote_str,ave_str,&voters,&my_sum,&old_vote
#define FNE_QUIT	"%3d %lu Q!"
#define NNE_QUIT	QUIT,who_am_i()
#define NEE_QUIT	&(int)_itype,&_uid
#define FNE_REORDER_CONF	"%3d %lu %s Q!"
#define NNE_REORDER_CONF	REORDER_CONF,who_am_i(),conf_name
#define NEE_REORDER_CONF	&(int)_itype,&_uid,_conf_name
#define FNE_SEND_STATS 	" %d Q!"
#define NNE_SEND_STATS	n
#define NEE_SEND_STATS	&no_to_send
#define FNE_SYNC_CONF " %3d %lu %s %3d > "
#define NNE_SYNC_CONF SYNC_CONF, who_am_i(), conf_name, no_sends
#define NEE_SYNC_CONF &(int)_itype,&_uid,_conf_name, &sends
#define FNE_UID_EXIST			 "%3d %lu Q!"
#define NNE_UID_EXIST				UID_EXIST, uid_to_check
#define NEE_UID_EXIST				&(int)_itype,&uid_to_check
#define FNE_UP_PRIORITY	 "%3d %lu Q!"
#define NNE_UP_PRIORITY  UP_PRIORITY,who_am_i()
#define NEE_UP_PRIORITY	 &(int)_itype,&_uid
#define FNE_WHO_DROP		"%3d %lu %lu %d Q!"
#define NNE_WHO_DROP		WHO_DROP, who_am_i(), id, force
#define NEE_WHO_DROP		&(int)_itype, &_uid, &id, (int*)&force
#define FNE_WHO_IS		"%3d %lu %lu Q!"
#define NNE_WHO_IS		WHO_IS,who_am_i(), who_id
#define NEE_WHO_IS		&(int)_itype,&_uid, &_voter
#define FEN_WHO_IS		"%s Q!"
#define EEN_WHO_IS		name
#define FNE_WHO_NUM		"%3d %lu %s %d Q!"
#define NNE_WHO_NUM		WHO_NUM,who_am_i(), name, add
#define NEE_WHO_NUM		&(int)_itype,&_uid, name, (int*)&add
#define FEN_WHO_NUM		"%lu Q!"
#define EEN_WHO_NUM		id
#define ENN_WHO_NUM		&id
#define FNE_WHO_SIGNED			"%3d %hd Q!"
#define NNE_WHO_SIGNED			WHO_SIGNED,p_item->dropping_id
#define NEE_WHO_SIGNED			&(int)_itype,&sdummy
#define FEN_WHO_SIGNED			"%lu %lu "
#define EEN_WHO_SIGNED			uid, time_stamp
#define FNE_WHO_SYNC " %3d %lu Q!"
#define NNE_WHO_SYNC WHO_SYNC, who_am_i()
#define NEE_WHO_SYNC &(int)_itype,&_uid
#define FNE_WHO_VOTED			"%3d %hd %s Q!"
#define NNE_WHO_VOTED			WHO_VOTED,p_item->dropping_id,inout
#define NEE_WHO_VOTED			&(int)_itype,&sdummy,question
#define FEN_WHO_VOTED			"%lu %s "
#define EEN_WHO_VOTED			uid, vote_str
#define ENN_WHO_VOTED			&uid, vote_str
#define FNE_WHOS_IN		"%3d %lu %s Q!"
#define NNE_WHOS_IN		WHOS_IN, who_am_i(), conf_name
#define NEE_WHOS_IN		&(int)_itype,&_uid, _conf_name
#define FEN_WHOS_IN		FEN_WHO_VOTED
#define EEN_WHOS_IN		EEN_WHO_VOTED
/* other things */
#define WHO_LEN 200+1  /* length of longest email address sent
			  into the WhoList manager  */
#endif

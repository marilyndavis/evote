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

/* $Id: Clerk.h,v 1.4 2003/01/23 19:54:27 marilyndavis Exp $ */ 
#ifndef Clerk_h
#define Clerk_h
/**********************************************************
 *   ../eVote/src/Clerk/Clerk.h  -
 *          Declarations for the interface to the Clerk.
 ***********************************************************
 **********************************************************/
#include"Clerkdef.h"
#include"msgdef.h"

/* Calls in confl.c */
OKorNOT adjourn_conf(char *conf_name);
YESorNO does_conf_exist(char* conf_name);
OKorNOT drop_conf(char * conf_name);
RTYPE send_check_conf(char * conf_name);
RTYPE send_eVote_conf(char * conf_name, short drop_days);
RTYPE send_whos_in(char * conf_name);
RTYPE send_who_sync(void);

/* in ipc_msg.c */
char *get_itype(ITYPE); /* the parameter should really be an ITYPE */
char *get_rtype(RTYPE); /* the parameter should really be an RTYPE */
void get_version(void);
OKorNOT report_Clerk(YESorNO verbose);
void report_ipc(void);
void start_up(YESorNO verbose);
void stop_ipc(void);
char *uid_report(unsigned long* p_uid);

/* in ipc_shm.c */
extern short * p_no_items;
extern ITEM_INFO* item_info;

/* Calls in iteml.c */

OKorNOT close_sent(ITEM_INFO * p_item);
int drop_items(ITEM_INFO* drop_in, int want_to_drop);
OKorNOT eVote_new_items(ITEM_INFO *p_item, int no_to_make);
OKorNOT get_key_pair(ITEM_INFO * p_item, 
		     char * , char * );
YESorNO is_close_done(ITEM_INFO * p_item);
YESorNO is_warning_done(ITEM_INFO * p_item);
RTYPE send_vstatus(ITEM_INFO *p_item, VSTATUS new_vstatus);
OKorNOT store_key_pair(ITEM_INFO * p_item, 
		       char *, char * );
OKorNOT warning_sent(ITEM_INFO * p_item, 
                      time_t time_to_close);

/* calls in maint.c */

OKorNOT down_priority(void);
OKorNOT new_exe(void);
OKorNOT new_log(void);
OKorNOT say_hello(void);
OKorNOT send_flush(void);
OKorNOT send_quit(void);
OKorNOT up_priority(void);

/* calls in queriesl.c */

RTYPE send_how(ITEM_INFO* p_item, unsigned long how_uid, 
	       short *ret_vote, short *ret_sum);
RTYPE who_signed(ITEM_INFO* p_item);
RTYPE who_voted(ITEM_INFO* p_item, char *inin);
time_t send_timestamp(ITEM_INFO *p_item, unsigned long how_uid);

/* start.c */
extern char version[];
char * eVote_cf_path(YESorNO verbose);
char *find_default(char * looking_for, YESorNO make_space,
		   YESorNO verbose);
void show_opening_screen(void);

/* calls in stats.c */
extern ITEM_STAT* item_stat;  /* list of item_stats */

short get_my_sum(short dropping_id);
char* get_stats(short dropping_id);
unsigned long get_voters(short dropping_id);
float get_ratio(short dropping_id, float* p_pos, float* p_neg);
float get_vratio(short dropping_id, unsigned long* n_pos, 
		 unsigned long * n_neg);

/* Calls in voterl.c */

extern ACTION current_action;
extern unsigned long mail_voter;
extern unsigned long no_of_participants;
extern char time_str[];

OKorNOT change_action(ACTION new_action);
YESorNO does_uid_exist(unsigned long uid_to_check);
char get_watch(ITEM_INFO * p_item);
YESorNO have_i_read(short dropping_id);
YESorNO have_i_voted(ITEM_INFO* p_item);
void i_am_leaving(void);
OKorNOT i_just_read(ITEM_INFO* p_item);
void leave_admin(void);
time_t pull_time(ITEM_INFO* p_item);
time_t push_time(ITEM_INFO* p_item, time_t new_time);

RTYPE send_move(char *was, char *is);
RTYPE send_vote(ITEM_INFO* p_item, short vote, 
		short *ret_old_vote);
RTYPE send_drop_voter(char * conf_name, unsigned long uid_to_drop,
		      YESorNO only_if_non_voter);
OKorNOT send_enter_admin(char *conf_name);
char * send_entering(char *conf_name, short *ret_drop_days, 
		     int time_limit, ACTION *p_current_action);
char * send_joining(int time_limit, YESorNO local);
char set_watch(ITEM_INFO * p_item, char byte);
unsigned long voter(void);
/*  local should be YES if this user has a login id, no if mail */
char * who_is(unsigned long who_id);
unsigned long who_num(char * email_address, YESorNO add);
RTYPE who_drop(unsigned long id, YESorNO force);
unsigned long who_am_i(void);

#endif

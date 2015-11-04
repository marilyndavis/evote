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

/* $Id: eVote.h,v 1.3 2003/01/15 18:54:08 marilyndavis Exp $ */ 
/**********************************************************
 *   eVote.h   header file for application side
 *********************************************************
 **********************************************************/
#include <stdio.h>
#define MAX_ITEMS_IN_GROUP  (100)  /* suit your taste on this */
#define MISTAKES (3)   /* number of times the query functions
                          will confuse the user before they quit */
#define NAME_LEN (11)   /* length of author names to print on
			   contents screen */
#define TRY (2)      /* If the voter is already online, and the
			old pid is still alive, we'll wait for
			the old pid to die for TRY minutes
			before we give up */
/* for set_up_item_group() */
/* - * - * - * - * - * - * - * - * - * - * - * - * - * - * - * 
 *    APP_ITEM definition.   Change this to suit your application.
 *                           This was copied from Notesfile code.
 * - * - * - * - * - * - * - * - * - * - * - * - * - * - * - */
#define TITLEN 36          /* length of the title in demo */
struct your_def
{
  short your_id;
  /*		struct daddr_f where;   Your stuff */
  char title[TITLEN + 1];   
  /*		struct auth_f auth;   Your stuff */
};
typedef struct your_def YOUR_ITEM;
#include "eVoteui/eVoteui.h"
extern struct questions_def *ask;
/**********************************************************
 *  These are the headings as they appear on the screen
 *  that shows the list of items in the conference.
 *  That screen is called the "contents" screen.
 *  You need to alter this to fit into your contents screen.
 **********************************************************/
#define PLAIN_HEAD    "\nITEM  RSPS  ACC             TITLE                                 AUTHOR"
#define TALLIED_YHEAD "\nITEM  RSPS  ACC VTR  YES    TITLE                                 AUTHOR"
#define GROUPED_YHEAD "\nITEM  RSPS  ACC YOU  YES    TITLE                                 AUTHOR"
#define TALLIED_NHEAD "\nITEM  RSPS  ACC VTR  AVE    TITLE                                 AUTHOR"
#define GROUPED_NHEAD "\nITEM  RSPS  ACC YOU  AVE    TITLE                                 AUTHOR"
#define DOTTED_LINE   "\n----   ---  --- --- ----    ------------------------------------  -----------"
/*  defined strings for the demo */
#define BIG_PROMPT "\n(c)ontents, # to read, (v)ote, (a)dd, (d)rop, (l)ist, (g)o, (q)uit, ?, \nConf?                                          or x for e(x)planations\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b"
#define BIG_PROMPT2 "\n(c)ontents, # to read, (v)ote, (a)dd, (d)rop, (l)ist, (g)o, (q)uit, ?, \nConf?                                               or t for (t)eacher\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b"
/* declarations of functions in eVoteui/item.c that need
   YOUR_ITEM  -- Please don't change these */
OKorNOT collect_eVote_details(YOUR_ITEM ** pp_your_item,
			      ITEM_INFO** pp_eVote_item, 
			      YESorNO checking,
			      short lines_printed);
OKorNOT set_up_item_group (YOUR_ITEM *p_your_item,
			   ITEM_INFO *p_eVote_item);

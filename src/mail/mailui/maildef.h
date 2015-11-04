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

/* $Id: maildef.h,v 1.3 2003/01/15 18:54:09 marilyndavis Exp $ */ 
/**********************************************************
 *  ../eVote/src/mail/mailui/maildef.h
 *   #defines and typedefs for eVote's email interface.
 ***********************************************************
 **********************************************************/
#ifndef maildef_h
#define maildef_h
#define EVOTE_ERRORS "eVote_errors@deliberate.com"
#define BLOCK 1024
#define MAX_LINE    300  /* max line length */
#define MAX_ADDRESS 200  /* longest email address allowed */
#define FNAME       500  /* long file name with path */
#define MAX_TOKEN   300  /* maximum size of one word */
#define MAX_ARGS    40   /* maximum number of arguments to mailer */
#define TRY 20  /* number of minutes to try and try to process
		   an email message if the user already has
		   a message in progress */
/* Errors are sent to one or more of the following by |'ing these */
#define PASS 0l       
#define SENDER 1l
#define OWNER 2l
#define ADMIN 4l
#define APPROVAL 8l
#define LIST 16l
#define DEVELOPER 32l
#define MAJOR 64l
#define CLOSING_SPREAD 20  /* if there are <= CLOSING_SPREAD participants
			      when a poll closes, a spreadsheet is
			      sent out. */
#endif

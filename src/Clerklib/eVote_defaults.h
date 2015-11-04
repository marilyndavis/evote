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

/* $Id: eVote_defaults.h,v 1.5 2003/01/15 18:54:08 marilyndavis Exp $ */ 
/**********************************************************
 *   eVote_defaults.h   -  This file is included in The Clerk
 *                         and the user interface
 ********************************************************
 *  Some system tunable parameters are very important
 *  to eVote.  If the limits declared by the system
 *  parameters are exceeded, the results are unpredictable
 *  and often diasterous.
 *
 *  The values for these parameters are taken from this
 *  file if no values are listed in the eVote.config
 *  file and if none are supplied as command-line 
 *  arguments to the eVote executable at start-up time.
 *
 *  The eVote.config file is read by the eVote exe-
 *  cutable and the values are passed along to the
 *  eVote_Clerk.
 *
 *  The first thing in each comment is the command-line
 *  argument flag for setting each of these values on
 *  the fly -- *if* it is of interest to eVote_Clerk.
 *  Some values are for the mail interface programs.
 *
 *  Repeating, priority goes like this:
 *
 *   0.  Command line arguments to eVote_Clerk.
 *
 *  When handled by the eVote executable:
 *
 *   1.  What is in the eVote.config file.
 *   2.  What is listed here.
 ********************************************************/
#ifndef _eVote_defaults_h
#define _eVote_defaults_h
/* There is EVOTE_HOME_DIR/src/tools/ipctest that gives the
   right numbers for these */

#include<sys/types.h>
#include<sys/msg.h>
#include<sys/shm.h>

#ifdef __FreeBSD__
#define MSGMNI   (40)
#define MSGTQL   (40)
#define MSGMAX   (2048)
#define SHMMNI   (96)
struct msgbuf
{
  long mtype;
  char mtext[MSGMAX];
};
#endif
#ifdef linux
#define key __key /* in the new linux compiler */
#define MSGMNI      (128)   /* -n Maximum number of simultaneous
			       message queues. This should equal
			       1 + the greatest number of
			       simultaneous logins. */
#define MSGMAX		(4056)   /* -m Maximum message length */
#define MSGTQL		(16384)    /* -t Maximum number of outstanding
				      messages.  */
#define SHMMNI		(128)    /* -i Maximum number of shared 
				    memory segments.  There is 
				    one shared memory segment
				    for each conference in
				    memory.  */
#endif

/* Other parameters handled in the same way */
#define BLOCK_SIZE (4096)  /* -b vote data is read in this
			      sized blocks */
#define CRASH_COMMAND ""  /* The Clerk tries to execute this if a signal
			     brings it down */
#define EVOTE_HOME_DIR "/usr/local"  /*  -h Data will appear in
					 EVOTE_HOME_DIR/eVote/data */
#define EVOTE_MAIL_TO "eVote-owner"  /* emergency messages 
				 are sent here */
#define EVOTE_MSG_KEY "63220"
#define IPCS "ipcs -c"       /* ipcs command to parse */
#define LISTDIR "/usr/local/majordomo/lists"  /* where the list serve
						 keeps lists */
#define MAILER "/usr/lib/sendmail -f\\$sender -t -io"
#define TIME_OUT 60
#define WHEREAMI "themoon.org"
#define PYTHONPATH "/home/mailman"
#endif

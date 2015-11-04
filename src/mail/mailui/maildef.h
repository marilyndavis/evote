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

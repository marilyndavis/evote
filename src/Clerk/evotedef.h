/* $Id: evotedef.h,v 1.4 2003/01/15 18:54:07 marilyndavis Exp $ */ 
// evotedef.h -- Clerk's #defines and global variables
/*********************************************************
 **********************************************************/
#ifndef EVOTEDEFS
#define EVOTEDEFS
#include <memory.h>
#include <time.h>
#include "../Clerklib/msgdef.h"
#include "../Clerklib/Clerkdef.h"
#define DATA_FORMAT_VERSION 1
char * get_itype(ITYPE itype);
#ifdef ATD_REPLACE_NEW
#include "atd_new.h"
#define new new (__FILE__, __LINE__)
#endif
typedef enum
{
  LIST,
  NORMAL,
  WITH_VOTE
}STAT_TYPE;  
#define	RECLEN	32	/* default length of a vote ballot */
#define	BUFLEN	512	/* length of the vote ballots buffer */
#define INFO_EXT  ".inf"   /* keeps info for the items */
#define DATA_EXT  ".dat"   /* keeps the ballots */
#define BINF_EXT  ".bnf"   /* keeps info for ballot box */
#define EXT_LEN	   (4)
#define PATH_LEN  PATHLEN  /* defined in Clerkdef.h */
#define MAX_FILE_NAME_LEN  CONFLEN + PATHLEN + EXT_LEN
#define MSG_TRIGGER (3) /* When there are only this many msg queue 
			   slots left, it tries to drop old queues */
#define GROW_TRIGGER (2)   /* idle-time grows if item_bytes_left <
			      ballot_size/GROW_TRIGGER, 
			      reorders if voter_spots_left 
			      < ballots_per_block/GROW_TRIGGER  
			      So, when the ballots are half
			      full, they grow -- when this is (2)
			      "petition" confs don't idle-time
			      grow. */
#ifdef EDEBUG
#define dlog *dlogger << '\n' 
#define GLOBAL_DECS long now;\
                    char debugger[MSGMAX + 1000]; \
                    ofstream *dlogger; \
                    int edebug = 0; \
                    char *dfile = "Clerk.debug"; \
                    char output_dir[PATH_LEN + MAX_FILE_NAME_LEN + 5]; \
                    ConfList conferences; \
                    InQ inq; \
                    QList qlist; \
                    WhoList wholist; \
                    ofstream *logger; \
	            int msgmni; \
                    int msgmax; \
                    int msgtql; \
                    int shmmni;
#define GLOBAL_INCS extern long now;\
                    extern char debugger[]; \
                    extern int edebug; \
                    extern char *dfile; \
                    extern ostream *dlogger; \
                    extern char output_dir[]; \
		    extern ofstream *logger;
#else
#define GLOBAL_DECS long now;\
                    char output_dir[PATH_LEN + MAX_FILE_NAME_LEN + 5]; \
                    InQ inq; \
                    QList qlist; \
                    WhoList wholist; \
                    ConfList conferences; \
                    ofstream *logger; \
	            int msgmni; \
                    int msgmax; \
                    int msgtql; \
                    int shmmni;
#define GLOBAL_INCS extern long now;\
                    extern char output_dir[]; \
		    extern ofstream *logger;
#endif
#define clerklog    time(&now);*logger << '\n' << ctime(&now) << ' '  
#define NO_UID      (unsigned long)(0)  // empty ballot indicator
//  returned as votes
#define UMASK_VALUE (0077)  /* 0022 for testing 0077 for production */
#define DATA_PERM (0600)   /* 644 for drops test 0600 for production */
#define INFO_PERM (0600)   /* ditto */
typedef enum {START,NEXT, FINISH} IT_CHOICE;  // iterator arguments
typedef enum {DELETE, ACTIVE, CHECKING, TROUBLE, DUPLICATE, WAIT} STATUS;  
// status of an item
#ifdef EDEBUG
#define  MESSAGES   1      // incoming message reported to Clerk.debug
#define  DUMP       2      // BallotBox and Ballot dumps to Clerk.dump 
#define  QUEUES     4      // Queue creation and deletion to Clerk.debug
#define  FLOWER     8      // Messages receieved and sent to Clerk.flow
// already #defined in msgdef.h
// -%8 puts messages received and sent from
// the application in Clerk.flow too.
#define  VOTERS    16      // Puts adds and delets to voterlist to Clerk.debug
#define  CONFS     32      
#define  DUMP2     64      // Puts other info in Clerk.dump
#define  LIMITS   128
#define  FILES    256
#define  WHODUMP  512
#define  GROOM   1024
#define  MEMS    2048       // Memory info goes to Clerk.mem
#define  ITEMS   4096
#define  ADJOURNS 8192
#define  SMEM    16384
#define  MEM_FILE "Clerk.debug"
#endif
#endif

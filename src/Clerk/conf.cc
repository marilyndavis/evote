/* $Id: conf.cc,v 1.3 2003/01/15 18:54:07 marilyndavis Exp $ */ 
// conf.cc -- defines Conf class which maintains the conference
/*********************************************************
 **********************************************************/
extern "C" {
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>
}
#include <stdio.h>
#include <pwd.h>
#include <iostream.h>
#include <fstream.h>
#include <strstream.h>
#include <sys/types.h>
#include <sys/msg.h>
#include "evotedef.h"
#include "ballotbo.h"
#include "itemlist.h"
#include "voterlis.h"
#include "voter.h"
#include "conf.h"
#include "conflist.h"
#include "qlist.h"
extern QList qlist;
GLOBAL_INCS
extern ConfList conferences;
extern int msgmax;
#ifdef EDEBUG
extern int edebug;
void vmem(char* msg, YESorNO free = NO, int size = 0);
#endif
char * uid_string(unsigned long uid);
// **************************************************
Conf::Conf(char *conf_name, short drop_day, YESorNO really_new, 
	   YESorNO protect)
  :_drop_day(drop_day),  _next(NULL),   _protected(protect),  _status(OK)
{
  if (strlen(conf_name) > CONFLEN)
    conf_name[CONFLEN] = '\0'; 
  strcpy(_name, conf_name);
  if (_protected)
    time(&protected_since);
#ifdef EDEBUG
  if (edebug & ADJOURNS)
    {
      dlog << "Constructing " << conf_name;
    }
  if (edebug & MEMS)
    {
      vmem("Starting conf creation.");
    }
#endif  
  _p_ballot_box = new BallotBox(this, conf_name);
#ifdef EDEBUG
  if (edebug & MEMS)
    {
      vmem("Ballot Box created.", NO, sizeof(BallotBox));
    }
#endif  
  _p_voters = new VoterList(this);
#ifdef EDEBUG
  if (edebug & MEMS)
    {
      vmem("Voter List created.", NO, sizeof(VoterList));
    }
#endif  
  _p_items = new ItemList(this, drop_day, really_new);
#ifdef EDEBUG
  if (edebug & MEMS)
    {
      vmem("Item List created.", NO, sizeof(ItemList));
    }
#endif  
  strcpy(_info_fname, output_dir);
  strcat(_info_fname, conf_name);
  strcat(_info_fname, INFO_EXT);
  if (really_new == YES)
    store_info();
  else
    if (fetch_info() != OK)
      {
	clerklog << "Couldn't read %s from disk. " << _name;
	_status = NOT_OK;
      }
}
// **************************************************
Conf::~Conf(void)
{
#ifdef EDEBUG
  if (edebug & ADJOURNS)
    {
      dlog << "Deleting " << _name;
    }
  if (edebug & MEMS)
    {
      vmem("Starting conf deletion.");
    }
#endif  
  delete _p_ballot_box;
#ifdef EDEBUG
  if (edebug & MEMS)
    {
      vmem("Ballot Box deleted.", NO, -sizeof(BallotBox));
    }
#endif  
  delete _p_voters;
#ifdef EDEBUG
  if (edebug & MEMS)
    {
      vmem("Voter list deleted.", NO, -sizeof(VoterList));
    }
#endif  
  delete _p_items;
#ifdef EDEBUG
  if (edebug & MEMS)
    {
      vmem("Item List deleted.", YES, -sizeof(ItemList));
    }
#endif  
}
//  ********************************************
//    stores the conf as is and backs it up.
OKorNOT
Conf::backup(char *footnote)
{
  OKorNOT backup_file(char *name, char *ext, YESorNO remove);
  
  if (store_all() != OK)
    return NOT_OK;
  //  clerklog << "Backing up " << _name << ":" << footnote;
  if (backup_file(_name, DATA_EXT, NO) == OK
      && backup_file(_name, INFO_EXT, NO) == OK
      && backup_file(_name, BINF_EXT, NO) == OK)
    return OK;
  return NOT_OK;
}
//  **********************************************
//   consider everyone on-line too
OKorNOT
Conf::check_stats(void)
{
  _p_voters->store_all();
  if (_p_items->check_stats() == NOT_OK)
    {
      store_info();
      clerklog << "Conf " << _name << ": checking complete.";
      return NOT_OK;
    }
  clerklog << "Conf " << _name << " is all OK.";
  return OK;
}
// *******************************************************
//     closes the info_file for the conf.
void
Conf::close_info(void)
{
  //  (ios&)_info_file.seekp((streamoff)0, ios::end);
  _info_file.seekp((streamoff)0, ios::end);
  items().set_end_offset(_info_file.tellp());			
#ifdef EDEBUG
  if (edebug & CONFS)
    dlog << "Closing " << name() << " with end_offset = "
	 << items().get_end_offset();
#endif
  _info_file.close();
}
//  **********************************
//     answers the question but does not put
//     the user online if he isn't.
YESorNO
Conf::does_uid_exist(unsigned long uid)
{
  long block;
  
  if (_p_voters->get_ballot(uid) == NULL 
      && _p_ballot_box->find_ballot(uid, block) == NULL)
    return NO;
  return YES;
}
// *******************************************
void
Conf::drop_items(char* input, int len, unsigned long uid)
{
  time_t early_closer;
  early_closer = _p_items->drop_items(input, len, uid);
  if (_p_ballot_box->pending_droppers() > 0)
    _p_ballot_box->try_dropping_voters(early_closer);
}
#ifdef PETITION
// **********************************************************
//     Called on petition lists when an item is dropped.
//     Petition lists start with "petition"
//     Looks for voters who have voted on no other items
//     and marks the DROPPING and sets ready_to_drop.
unsigned long
Conf::drop_pending_non_signers(void)
{
  Ballot *p_ballot;
  unsigned long uid = 0;
  long block;
  unsigned long count = 0;
  p_ballot = ballot_box().iterator(START, &block, 0);
  do
    {
      uid = *(unsigned long*)p_ballot;
      if (!(p_ballot->action() & SIGNER))
	continue;
      if (drop_voter(uid, 0, MAYBE, p_ballot, block) == OK)
	count++;
    }
  while ((p_ballot  = ballot_box().iterator(NEXT, &block)) != NULL);
  ballot_box().set_ready_to_drop(count);
  return count;
}
#endif
// *******************************************
//   drops the voter from the ballot box and subtracts
//   out all the votes.
//  only_if_non_voter == YES  if voter is unsigning petition
//                       NO   for regular lists
//                       MAYBE if voter is being dropped because the
//                             petition is being dropped.
OKorNOT
Conf::drop_voter(unsigned long uid_to_drop, 
		 unsigned long by_uid, YESorNO only_if_non_voter,
		 Ballot * p_ballot, long block)
{
  Voter * p_voter;
  Voter* comes_after;
  time_t now;
  
  if ((p_voter = _p_voters->wheres(uid_to_drop, comes_after)) 
      != NULL)
    {
      clerklog << "Uid " << uid_string(by_uid) << " tried to drop voter uid " 
	       << uid_to_drop << " while " << uid_to_drop << " is still online.";
      (*logger).flush();
      return STOP;
    }
  if (p_ballot == NULL  /* already have ballot?  */
      && (p_ballot = _p_ballot_box->find_ballot(uid_to_drop, block)) 
      == NULL || p_ballot->action() & DROPPING)
    {
      return NOT_OK;
    }
  // p_ballot points into the ballot_box buffer, not a private copy!
  if (only_if_non_voter == NO && ((_drop_day == 9999)
				  || (time(&now) - p_ballot->mod_date()
				      < _drop_day*86400)))
    {
      if (getpwuid(uid_to_drop) != NULL)
	return UNDECIDED;
    }
  if (only_if_non_voter != NO)  /* MAYBE or YES */
    {
      if (items().signed_anything(p_ballot) == YES)
	return CANT;
    }
  p_ballot->new_date();
  p_ballot->change_action(VOTE_ONLY); /* so that we can unvote */
  /*  if (only_if_non_voter == NO) */
  _p_items->drop_votes(p_ballot);
  _p_ballot_box->drop_pending(p_ballot);
  if (only_if_non_voter != MAYBE && _p_items->isalive(p_ballot) == NO)
    return _p_ballot_box->delete_ballot(p_ballot, block);
  else
    _p_ballot_box->write_ballot(block, 
				(streampos)((char*)p_ballot-
					    _p_ballot_box->_buffer), 
				p_ballot);
  return OK;
}		
//       ******************************
//   fetch_info  reads info from info files.
OKorNOT
Conf::fetch_info(void)
{
  float extra1;
  long extra2;
  int data_format_version;
  istream& strm = (istream&)open_info(ios::in);
  
  if (!strm)
    return NOT_OK;
  strm >> _drop_day >> extra1 >> extra2 >> data_format_version;
  if (data_format_version != DATA_FORMAT_VERSION)
    {
      clerklog << _info_fname << " in wrong data format. Should be
" << DATA_FORMAT_VERSION << " but it's " << data_format_version;
      clerklog << "Coming down now.";
      exit(1);
    }
  if (_p_items->fetch(strm) != OK)
    {
      _status = NOT_OK;
      clerklog << "Trouble fetching info in " << _name;
    }
#ifdef EDEBUG
  if (edebug & MEMS)
    {
      vmem("Item List fetched itself.");
    }
#endif  
  close_info();
  if (_p_ballot_box->fetch_info() != OK)
    _status = NOT_OK;
#ifdef EDEBUG
  if (edebug & MEMS)
    {
      vmem("BallotBox fetched itself");
    }
#endif  
  return _status;
}
// *****************************************************
//    fstream& open_info(int mode = ios::out)
//       opens the _info_file
fstream&
Conf::open_info(int mode)
{
  char name_in_file[CONFLEN + 1];
#ifdef EDEBUG
  if (edebug & CONFS)
    {
      dlog << "About to open " << name() 
	   << ".inf with mode = " << mode << ". End_offset = "
	   << _p_items->get_end_offset();
      (*logger).flush();
      (*dlogger).flush();
    }
#endif
  _info_file.open(_info_fname, mode, INFO_PERM);
  while (!_info_file)
    {
      if (conferences.adjourn_some(this) == 0)
	{
	  clerklog << "Can't open " << _info_fname
		   << " in open_info for " << name();
	  clerklog << "Coming down now.";
	  exit(0);
	}
      _info_file.open(_info_fname, mode, INFO_PERM);
    }
  if (mode == ios::in)
    {
      (istream&)_info_file >> name_in_file;
      if (strcmp(name_in_file, _name) != 0)
	{
	  clerklog << "File corrupted : " << _info_fname;
	  clerklog << "Coming down now.";
	  close_info();
	  exit(0);
	}
    }
  return _info_file;
}			
// *******************************************
//  OKorNOT store_all  - stores both the info and the data files
OKorNOT
Conf::store_all(void)
{
  if (store_data() == OK && store_info() == OK)
    return OK;
  return NOT_OK;
}
// **************************************************
//     asks everybody to please store their info in the info_file
OKorNOT
Conf::store_info(void)
{
  float extra1 = -1.;
  long extra2 = -1L;
  int data_format_version = DATA_FORMAT_VERSION;
  OKorNOT status = OK;
  ostream& strm =(ostream&) open_info(ios::out);
  
  if (!_info_file)
    return NOT_OK;
  strm << _name << ' ' << _drop_day << ' '
       << extra1 << ' ' << extra2 << ' ' << data_format_version << ' ';
  if (_p_ballot_box->store_info() == NOT_OK
      ||  _p_items->store(strm) == NOT_OK )
    status = NOT_OK;
  close_info();
  return status;
}
// **************************************************
//   Passes back, into char* qlist.output, a list of user id's for
//   the voters in the *p_conf.  The format is "101 x " and
//   x isn't used by the other side.  This is so that the
//   same routines can be used for this feature as the
//   who_voted feature.
//   If *p_continue is YES -- and this function might set it to
//   be YES, then there are more uids than can fit in the
//   msgmax of the ipc message queue and it sends a few
//   messages.
long
Conf::whos_in(char *here, Conf* p_conf, YESorNO *p_continue)
{
  long len = 0L;
  unsigned long uid;
  char vote_str[6] = "x";
  static Ballot * ballot;   // static in case we have to
  // go around again.
  BallotBox& bbox = p_conf->ballot_box();
  if (*p_continue == NO)
    ballot = bbox.iterator(START);
  else
    *p_continue = NO;
  if (ballot == NULL)
    {
      sprintf(here, "\nNo one.\n Q!");
      return -1L;
    }
  do
    {
      if (ballot->action() & DROPPING)
	continue;
      uid = *(unsigned long*)ballot;
      if (len + 21L > msgmax)
	{
	  *p_continue = YES;
	  break;
	}
      sprintf(vote_str, "%d", (int)ballot->action());
      len += (long)sprintf(here + len,
			   FEN_WHOS_IN, EEN_WHOS_IN);
    }
  while ((ballot = bbox.iterator(NEXT)) != NULL);
  if (len == 0L)
    {
      sprintf(here, "\nNo one.\n Q!");
      return -1L;
    }
  len += sprintf(here + len, "%c", '\0');
  return len;
}

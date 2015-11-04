/* $Id: conf.h,v 1.3 2003/01/15 18:54:07 marilyndavis Exp $ */ 
// conf.h header for the functions that maintain the conference
/*********************************************************
 **********************************************************/
#ifndef CONFDEF
#define CONFDEF
#include <fstream.h>
class ConfList;
class BallotBox;
class ItemList;
class VoterList;
class Voter;
class Item;
class Ballot;

class Conf
{
 public:
  Conf(char *name, short drop_day, YESorNO really_new, YESorNO protect = NO);
  ~Conf(void);
  BallotBox& ballot_box(void);
  short ballots_per_block(void);
  short ballot_size(void);
  OKorNOT backup(char *footnote);
  OKorNOT change_vstatus(short did, VSTATUS new_vstatus,
			 unsigned long uid_asking);
  OKorNOT check_ballots(void);
  int check_drops(void);
  OKorNOT check_order(void);
  OKorNOT check_stats(void);
  void close_info(void);
  VoterList& community(void);
  OKorNOT create_items(char *input, int len, unsigned long uid, 
		       int*no_return);
  YESorNO does_uid_exist(unsigned long uid);
  short drop_day(void);
  void drop_items(char *input, int len, unsigned long uid);
  OKorNOT drop_voter(unsigned long uid_to_drop, 
		     unsigned long by_uid,
		     YESorNO only_if_non_voter,
		     Ballot *p_ballot = NULL,
		     long block = 0);
  void expose(void);
#ifdef PETITION
  unsigned long drop_pending_non_signers(void);
#endif
  Ballot* fetch_ballot(unsigned long uid, long &block, 
		       streampos &offset, YESorNO force);
  OKorNOT fetch_info(void);
  unsigned long get_local_id(void);
  OKorNOT grow_ballot(YESorNO force = NO);
  fstream& open_info(int mode = ios::app);
  void protect(void);
  YESorNO isempty(void);
  YESorNO is_protected(void);
  ItemList& items(void);
  streampos item_bytes_left(void);
  int items_to_drop(void);
  char *name(void);
  int memid(void);
  void mid_dropped(int mid);
  unsigned long no_of_participants(void);
  unsigned long ready_to_drop(void);
  OKorNOT reorder(YESorNO force = NO, YESorNO refetch = YES);
  unsigned long send_stats(Voter* p_voter, Item* starting,
			   int no_to_send);
  OKorNOT store_all(void);
  OKorNOT store_ballot(Ballot* p_ballot, long &block, streampos &offset);
  OKorNOT store_data(void);
  OKorNOT store_info(void);
  OKorNOT sync_conf(char* input, int len, 
		    unsigned long uid, int sends);
  int voter_spots_left(void);
  int voters_to_drop(void);
  long whos_in(char *here, Conf* p_conf, 
	       YESorNO * p_continue);
  friend class ConfList;
 private:
  short _drop_day;
  char _name[CONFLEN + 1];
  Conf *_next;
  YESorNO _protected;
  BallotBox* _p_ballot_box;
  VoterList* _p_voters;
  ItemList* _p_items;    // ItemList contains the GroupList
  OKorNOT _status;
  time_t protected_since;
#ifdef PETITION
  void drop_non_signers(void);
#endif
  fstream _info_file;
  char _info_fname[CONFLEN + PATH_LEN + EXT_LEN + 1];
};
ostream& operator<< (ostream&, Conf& );
#include "ballotbo.h"
#include "itemlist.h"
#include "voterlis.h"
inline BallotBox& Conf::ballot_box(void) {	return *_p_ballot_box;}
inline short Conf::ballot_size(void){return _p_ballot_box->ballot_size();}
inline short Conf::ballots_per_block(void) 
{return _p_ballot_box->ballots_per_block();}
inline OKorNOT Conf::change_vstatus(short did, VSTATUS new_vstatus,
				    unsigned long uid_asking)
{return _p_items->change_vstatus(did, new_vstatus, uid_asking);}
inline OKorNOT Conf::check_ballots(void){return _p_ballot_box->check_ballots();}
inline int Conf::check_drops(void){return _p_ballot_box->try_dropping_voters((time_t)0);}
inline OKorNOT Conf::check_order(void){	return _p_ballot_box->check_order();}
inline VoterList& Conf::community(void){return *_p_voters;}
inline OKorNOT Conf::create_items(char *input, int len, unsigned long uid, 
				  int* no_return)
{return _p_items->create(input, len, uid, no_return);}
inline short Conf::drop_day(void){return _drop_day;}
inline void Conf::expose(void){_protected = NO; return;}
inline Ballot* Conf::fetch_ballot(unsigned long uid, long &block, 
				  streampos &offset, YESorNO force)
{return _p_ballot_box->fetch(uid, block, offset, force);}
inline unsigned long Conf::get_local_id(void){return _p_items->get_local_id();}
inline OKorNOT Conf::grow_ballot(YESorNO force)
{return _p_ballot_box->grow(force);}
inline YESorNO Conf::isempty(void){return _p_items->isempty();}
inline YESorNO Conf::is_protected(void){if (time(NULL)-protected_since > 180){_protected = NO;} return _protected;}
inline streampos 
Conf::item_bytes_left(void)
{return (streampos)_p_ballot_box->ballot_size() - _p_items->next_byte();}
inline ItemList& Conf::items(void){return *_p_items;} 
inline int Conf::items_to_drop(void){return _p_items->items_to_drop();}
inline int Conf::memid(void){return _p_items->memid();}
inline char* Conf::name(void){return _name;}  
inline unsigned long Conf::no_of_participants(void){return _p_ballot_box->no_of_participants();}
inline void Conf::protect(void){time(&protected_since); _protected = YES; return;}
inline unsigned long Conf::ready_to_drop(void)
{return _p_ballot_box->ready_to_drop();}
inline OKorNOT Conf::reorder(YESorNO force, YESorNO refetch)
{return _p_ballot_box->reorder(force, refetch);}
inline unsigned long Conf::send_stats(Voter* p_voter, Item* starting,
				      int no_to_send)
{return _p_items->send_stats(p_voter, starting, no_to_send);}
inline OKorNOT Conf::store_ballot(Ballot* pballot, long& block, 
				  streampos & offset)
{return _p_ballot_box->store(pballot, block, offset);}

inline OKorNOT Conf::store_data(void){return _p_voters->store_all();}
inline OKorNOT Conf::sync_conf(char* input, int len, unsigned long uid, 
			       int sends)
{return _p_items->sync_conf(input, len, uid, sends);}
inline int Conf::voter_spots_left(void){return _p_ballot_box->voter_spots_left();}
inline int Conf::voters_to_drop(void){return _p_ballot_box->voters_to_drop();}
#endif

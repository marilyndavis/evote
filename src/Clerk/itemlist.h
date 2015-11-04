/* $Id: itemlist.h,v 1.3 2003/01/15 18:54:07 marilyndavis Exp $ */ 
// itemlist.h -- defines ItemList and related classes to keep 
//  the list of items for a conference
/*********************************************************
 *    Copyright (c) 1994...2015 Deliberate.com Patented.
 *    by Marilyn Davis
 **********************************************************/
#ifndef ITEMLISTHPP
#define ITEMLISTHPP
#include<sys/types.h>
#include<sys/ipc.h>
class Ballot;
class Conf;
class ItemGroup;
class Item;
class ItemList;
class TalliedItem;
class GroupedItem;
class Voter;
class VoterList;

#include "applist.h"
#include "itemid.h"
// This GroupList class is contained by each Conf's ItemList member
// and is the list of ItemGroups for the conf.
#define GBUNCH 2
class GroupList
{
 public:
  GroupList(Conf *p_conf);
  ~GroupList(void);
  void set_max_id(short max);
  ItemGroup* last_group(void);
  YESorNO last_group_complete(void);
 private:
  ItemGroup* find(short gid);
  short _max_id;			// can be bigger than _next_index
  short _next_index;  // so we need both
  Conf* _p_conf;
  ItemGroup **_p_group;
  short _space;
  void attach(ItemGroup *);
  void detach(ItemGroup *);
  short mark_delete(ItemGroup *, unsigned long uid);
  short new_id(void);
  friend class ItemGroup;
  friend class ItemList;
  friend class GroupedItem;
};
class DroppingOrder
{
 public:
  DroppingOrder(void);
  ~DroppingOrder(void);
  Item*& operator[](short i);
  void attach(Item* p_item);
  void delete_from(short start, short end = 0);
  void delete_from(Item* p_item);
  short next_did(void);
 private:
  Item** _do_item;
  short _next_did;
  short _space;
  friend class ItemList;
};
class ItemList
{
 public:
  ItemList(Conf* cf, short drop_day, YESorNO really_new);
  ~ItemList(void);
  void blank_ballot(Ballot* p_ballot);
  OKorNOT change_vstatus(short did, VSTATUS new_vstatus, 
			 unsigned long uid_asking);
  OKorNOT check_stats(void);
  OKorNOT create(char *input, int len, unsigned long uid, 
		 int* no_return);
  time_t drop_items(char* input, int len, unsigned long uid);
  OKorNOT drop_votes(Ballot* p_ballot);
  OKorNOT fetch(istream & strm);
  streampos get_end_offset(void);
  streampos fill_byte(void);
  unsigned long get_local_id(void);
  GroupList& groups(void);
  short inc_next_bit(void);
  streampos inc_next_byte(int no_bytes = 1);
  YESorNO isalive(Ballot * p_ballot);
  YESorNO isempty(void);
  streampos item_bytes_left(void);
  short items_to_drop(void);
  DroppingOrder dropping_order;
  int memid(void);
  streampos next_byte(void);
  //  unsigned long put_info(short so_far, short atatime);
  unsigned long send_stats(Voter* p_voter, 
			   Item* starting, int no_to_send);
  void set_end_offset(streampos newpos);
  void set_fill_byte(short n);
  void set_next_bit(short n);
#ifdef PETITION
  YESorNO signed_anything(Ballot * p_ballot);
#endif
  OKorNOT store(ostream& strm);
  OKorNOT sync_conf(char* input, short len, 
		    unsigned long uid, short sends);
  Item* wheres(ItemID & tid);
  Item* wheres(short did);
 private:
  Conf* _p_conf;
  AppList _applist;
  Item* _first;
  GroupList _groups;
  short _resends;
  streampos _end_offset;   
  streampos _fill_byte;
  streampos _next_byte;
  short _next_bit;
  short _to_delete;        
  friend class AppList;
  friend class BallotBox;		
  static unsigned long _last_id;
  void attach_after(Item *new_item, Item *& attach_after);
  long check_space(int no_bytes = 1);
  void detach(Item* p_item);
  void delete_item(Item* p_item);
  OKorNOT finish_create(Item* new_item, unsigned long uid);
  OKorNOT kill_old_items(streampos dead_bytes[]);  // BallotBox::
  void request_grow(long priority);
  short start_list(char* input);  // sync, drop_items, create
  Item* wheres(ItemID & tid, Item *&belongs_after);
#ifdef EDEBUG
  void ItemList::show_map(ofstream& strm);
  void ItemList::show_onlines(ofstream& strm);
  void ItemList::show_packing(Ballot* pb, ofstream& strm);
  void ItemList::show_votes(Ballot* pb, ofstream& strm);
#endif
};
#include "ballot.h"
inline ItemGroup*
GroupList::last_group(void){if (_next_index == 0) 
  return NULL;
 return _p_group[_next_index -1];}
inline void GroupList::set_max_id(short max){_max_id = max;}
inline short GroupList::new_id(){return ++_max_id;}
inline short DroppingOrder::next_did(){return _next_did;}
inline streampos ItemList::get_end_offset(){return _end_offset;}
inline streampos ItemList::fill_byte(){return _fill_byte;}
inline GroupList& ItemList::groups(){return _groups;}
inline short ItemList::inc_next_bit(){return _next_bit++;}
inline streampos ItemList::inc_next_byte(int no_bytes)
{streampos starts = _next_byte; _next_byte += no_bytes; return starts;}
inline YESorNO ItemList::isempty(){if (_first == NULL)return YES; 
 return NO;}
inline short ItemList::items_to_drop(){return _to_delete;}
inline int ItemList::memid(){return _applist.get_memid();}
inline streampos ItemList::next_byte(){return _next_byte;}
inline void ItemList::set_end_offset(streampos newpos){_end_offset = newpos;}
inline void ItemList::set_fill_byte(short n){_fill_byte = n;}
inline void ItemList::set_next_bit(short n){_next_bit = n;}
inline Item* ItemList::wheres (ItemID& tid)
{Item* x = NULL;return wheres(tid, x);}
inline Item* ItemList::wheres (short did)
{return dropping_order[did];}
#endif

/* $Id: item.h,v 1.3 2003/01/15 18:54:07 marilyndavis Exp $ */ 
// header for the Item hierarchy
/*********************************************************
 **********************************************************/
#ifndef ITEMHPP
#define ITEMHPP
class ItemID;
class ItemGroup;
class ItemList;
class Conf;
class Ballot;

#include "itemid.h"
// Item is the base class for the item hierarchy.  
class Item
{
 public:
  Item(Conf* p_conf, ITEM_INFO* p_app_item, streampos offset);
  Item(Conf* p_conf, istream & strm);
  virtual ItemGroup* p_group(void);
  char type(void);
  virtual OKorNOT change_vstatus(VSTATUS new_vstatus, 
				 unsigned long uid_asking);
  virtual OKorNOT check(Ballot* p_ballot);
  virtual OKorNOT how_voted(char *here, Conf* p_conf, unsigned long uid, 
			    unsigned long uid_asking);
  ItemID& item_id(void);
  time_t close(void);
  short dropping_id(void);
  void mark_delete(unsigned long uid);
  virtual void init_stat(ITEM_STAT *here);
  virtual short mark_read(Ballot *p_ballot);
  virtual short mark_vote(Ballot *p_ballot, short new_vote, RTYPE *cc);
  Item* next(void);
  unsigned long participants_when_closed(void);
  virtual void put_info(ITEM_INFO * here);
  virtual short report_fixed_vote(Ballot *p_ballot);
  virtual long report_stat(ITEM_STAT *here, 
			   STAT_TYPE type = NORMAL, 
			   Ballot *p_ballot = NULL, 
			   short old_vote = NOT_READ); 
  virtual short report_vote(Ballot *p_ballot); 
  virtual void report_vote_str(char* here, Ballot *p_ballot);
  STATUS status(void);
  VSTATUS vstatus(void);
  virtual long who_voted(char *here, Conf* p_conf, 
			 char *question,	YESorNO * p_continue);
 protected:
  char _type;
  short _dropping_id;// keep this the first persistent variable,
  ItemID _item_id;   // except _type has special persistence
  short _bit;
  long long_byte;
  unsigned long _readers;
  unsigned long _uid_delete;
  VSTATUS _vstatus;
  unsigned long _participants_when_closed;
  unsigned long _author;  
#ifdef TITLE
  char _title[TITLEN + 1];  // for demo
#endif
#ifdef TIME
  time_t _open;
  time_t _close;
#endif
  unsigned char _status;    // keep this the last persistent variable
				// really that's a STATUS enum
  streampos _byte;
  unsigned long ch_readers;  // non persistant space for checking 
  Item* first_duplicate;
  streampos _offset;
  Conf* _p_conf;
  void add_reader(void);
  virtual void blank(Ballot *ballot);
  virtual short ballot_bytes(void);
  virtual void mark_bytes(streampos bytes[]);
  virtual void find_spot(ItemList *tl);
  virtual OKorNOT store(ostream &strm);
 private:
  Item* _next;
  virtual OKorNOT store_me(void);
  friend class ItemList;
  friend class DroppingOrder;
};
// TalliedItem is a item with a vote.
class TalliedItem: public Item
{
 public:
  TalliedItem(Conf* p_conf, ITEM_INFO* p_app_item, streampos offset);
  TalliedItem(Conf* p_conf, istream & strm);
  virtual OKorNOT change_vstatus(VSTATUS new_vstatus, 
				 unsigned long uid_asking);
  virtual OKorNOT check(Ballot* p_ballot);
  virtual OKorNOT how_voted(char *here, Conf* p_conf, unsigned long uid,
			    unsigned long uid_asking);
  virtual void init_stat(ITEM_STAT *here);
  virtual short mark_read(Ballot *p_ballot);
  virtual short mark_vote(Ballot* p_ballot, short vote, RTYPE *cc);
  virtual void put_info(ITEM_INFO * here);
  virtual ItemGroup* p_group(void);
  virtual short report_fixed_vote(Ballot *p_ballot);
  virtual long report_stat(ITEM_STAT *here, 
			   STAT_TYPE type = NORMAL, 
			   Ballot *p_ballot = NULL, short old_vote = NOT_READ);
  virtual short report_vote(Ballot *p_ballot);
  virtual void report_vote_str(char* here, Ballot *p_ballot);
  virtual long who_voted(char *here, Conf* p_conf, char *question,
			 YESorNO * p_continue);
 protected:
  float _ave;             // keep this the first persistent var
  short _max_vote;
  short _min_vote;
  unsigned char _priv_type;   // really PRIV_TYPE enum
  float _sum;
  float _pos;
  float _neg;
  float _sum_squares;
  float _extra1;
  float _extra2;
  unsigned long _pos_voters;
  unsigned long _neg_voters;
  unsigned long _voters;      // keep this the last persistent var
  float ch_sum;  // non persistant space for checking stats
  float ch_sum_squares;
  float ch_pos;
  float ch_neg;
  unsigned long ch_pos_voters;
  unsigned long ch_neg_voters;
  unsigned long ch_voters;  // this too
  char _kind;
  virtual void add_vote(short vote);
  virtual short ballot_bytes(void);
  virtual void blank(Ballot *ballot);
  virtual void mark_bytes(streampos bytes[]);
  virtual void change_vote(short oldv, short newv);
  virtual void find_spot(ItemList* p_tl);
  virtual OKorNOT store(ostream &strm);
  virtual OKorNOT store_me(void);
 private:
  virtual void set_kind(void);
  friend class ItemGroup;
};
// ************************************************************
//  TimeStampItem is a yes/no item with space for a time stamp
//  (time_t) and an offset (streampos) 
class TimeStampItem: public TalliedItem
{
 public:
  TimeStampItem(Conf* p_conf, ITEM_INFO* p_app_item, streampos offset);
  TimeStampItem(Conf* p_conf, istream & strm);
  time_t send_stamp(Conf* p_conf, unsigned long uid, unsigned long uid_asking);
  time_t pull_time(Ballot *p_ballot, streampos * spos);
  time_t push_time(Ballot *p_ballot, time_t new_time, 
		   streampos new_offset, streampos * old_offset);
  long who_signed(char *here, Conf* p_conf, 
		  YESorNO * p_continue);
  void put_info(ITEM_INFO * here);
 private:
  int content_bytes; /* 1 for titem part*/
  /*    + sizeof(time_t) + sizeof(streampos);*/
  int filler;/* = 32 - (sizeof(Header) + content_bytes);*/
  /* this size is also in itemlist::create */
 protected:
  short ballot_bytes(void);
  void blank(Ballot *ballot);
  void mark_bytes(streampos bytes[]);
  void find_spot(ItemList* p_tl);
};
// ****************************************************
// GroupedItem is a item that has its votes attached to some other
// items.  The list of items that are attached to each other is kept
// in a ItemGroup.  The list of ItemGroups for one conference is kept
// in a GroupList which is contained in the ItemList.
class GroupedItem: public TalliedItem
{
 public:
  GroupedItem(Conf* p_conf, ITEM_INFO* p_app_item, streampos offset);
  GroupedItem(Conf* p_conf, istream & strm);
  ItemGroup *p_group(void);
  OKorNOT change_vstatus(VSTATUS new_vstatus, 
			 unsigned long uid_asking);
  OKorNOT check(Ballot* p_ballot);
  short fix(short vote);
  OKorNOT how_voted(char *here, Conf* p_conf, unsigned long uid,
		    unsigned long uid_asking);
  short mark_read(Ballot *p_ballot);
  short mark_vote(Ballot* p_ballot, short vote, RTYPE *cc);
  void put_info(ITEM_INFO * here);
  short report_fixed_vote(Ballot* p_ballot);
  long report_stat(ITEM_STAT *here, STAT_TYPE type = NORMAL, 
		   Ballot *p_ballot = NULL, short old_vote = NOT_READ);
  long send_stats(ITEM_STAT* here, Ballot* p_ballot, YESorNO start);
  friend ItemGroup;
 private: 
  short ballot_bytes(void);
  void mark_bytes(streampos bytes[]);
  static short more_to_come;
  ItemGroup *_p_group;
  OKorNOT store(ostream &strm);
  OKorNOT store_me(void);
  friend class ItemList;
};
inline void Item::add_reader(void) {_readers++;}
inline short Item::ballot_bytes(void) {return (_bit ? 0:1);}
inline void
Item::blank(Ballot *p_ballot){if (_bit == 0)
  *((char*)p_ballot + _byte) = (char)0;}
inline ItemID& Item::item_id(){return _item_id;}
inline time_t Item::close(){return _close;}
inline short Item::dropping_id(){return _dropping_id;}
inline Item* Item::next(){return _next;}
inline unsigned long Item::participants_when_closed()
{return _participants_when_closed;}
inline ItemGroup* Item::p_group(){cout << "never happen";
 return NULL;}
inline short Item::report_vote(Ballot* p_ballot)
{ if ((*((unsigned char*)p_ballot + _byte) & 01 << _bit) == 0) 
  return NOT_READ;
 return READ;}
inline void Item::report_vote_str(char* here, Ballot *p_ballot)
{if (p_ballot == NULL || report_vote(p_ballot) != READ) 
  strcpy(here,"!ACC"); 
 else strcpy(here,"ACC");}
inline STATUS Item::status(){return (STATUS)_status;}
inline char Item::type(){return _type;} 
inline VSTATUS Item::vstatus(){return _vstatus;}
inline short TalliedItem::ballot_bytes(){return 1;}
inline void TalliedItem::blank(Ballot* p_ballot){if (_vstatus == CLOSED)
  *((char*)p_ballot + _byte) = LATE;}
inline OKorNOT TalliedItem::change_vstatus(VSTATUS new_status, 
					   unsigned long uid_asking)
{return Item::change_vstatus(new_status, uid_asking);}	
inline short GroupedItem::ballot_bytes(){ return (more_to_come ? 1 : 3);}
/* ballot_bytes should return 3 if it's the last in the group? */
inline ItemGroup* TalliedItem::p_group(){cout << "never happen";
 return NULL;}
inline ItemGroup* GroupedItem::p_group(){return _p_group;}
inline short TimeStampItem::ballot_bytes(){return content_bytes + filler;}
#endif

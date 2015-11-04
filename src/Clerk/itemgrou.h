/* $Id: itemgrou.h,v 1.3 2003/01/15 18:54:07 marilyndavis Exp $ */ 
// itemgrou.h -- header for the ItemGroup class needed to manage
//               a group of vote items.
/*********************************************************
 *    Copyright (c) 1994...2015 Deliberate.com Patented.
 *    by Marilyn Davis
 **********************************************************/
#ifndef ITEMGROUHPP
#define ITEMGROUHPP
class Conf;
class Ballot;
class GroupedItem;
class ItemList;
// ItemGroup is a class for keeping the list of GroupedItems that are
// attached to each other.  The instantiated ItemGroup is pointed to
// by each of the GroupedItems.
#define IGBUNCH 10
class ItemGroup
{
 public:
  ItemGroup(Conf *p_conf, istream& strm, short id);
  ItemGroup(Conf *p_conf, short limit, short number);
  ~ItemGroup(void);
  GroupedItem *first(void);
  short calc_sum(Ballot *p_ballot);
  OKorNOT change_vstatus(VSTATUS new_vstatus, unsigned long uid_asking);
  OKorNOT check(Ballot *p_ballot);
  YESorNO isempty(void);
  GroupedItem *last(void);
  short mark_delete_group(unsigned long uid);
  short mark_vote(Ballot* p_ballot, short vote, 
		  RTYPE * cc, GroupedItem *p_item, short old_vote);
  short report_sum(Ballot *p_ballot);
  long send_stats(ITEM_STAT *here, Ballot *p_ballot, YESorNO start);
 private:
  friend class ItemList;
  friend class GroupedItem;
  friend class GroupList;		
  streampos _byte; 
  long long_byte; // first persistent field
  short _min_sum;
  short _no_in_group;
  unsigned short _id;
  short _sum_limit;   // last persistent field
  short _in_so_far;
  Conf* _p_conf;
  GroupedItem **_p_item;
  short _space;
  void attach(GroupedItem *p_gt);
  void check_kind(void);
  void detach(GroupedItem *p_gt);
  void find_spot(ItemList *tl);
  short how_many_more(GroupedItem *gi);
  void ItemGroup::mark_group(Ballot* p_ballot, short vote, RTYPE *cc);
  OKorNOT store(ostream &strm);
  OKorNOT store_me(void);
  short min_sum(void);
  GroupedItem* operator[](short i);
};
inline GroupedItem * ItemGroup::first(void){return _p_item[0];}
inline GroupedItem * ItemGroup::last(void){return _p_item[_no_in_group - 1];}
inline GroupedItem * ItemGroup::operator[](short i){return _p_item[i];}
#endif

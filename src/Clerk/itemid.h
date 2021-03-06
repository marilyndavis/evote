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

/* $Id: itemid.h,v 1.3 2003/01/15 18:54:07 marilyndavis Exp $ */ 
// itemid.h header for the ItemID class
/*********************************************************
 **********************************************************/
#ifndef ITEMIDHPP
#define ITEMIDHPP
// ItemId, a little class contained in every item class that identifies
// the item.  This should change depending on the conferencing system.
class ItemID
{
 public:
  ItemID(void);
  
  ItemID(STATIC_ID* aid):_network_id(aid->network_id),
    _local_id(aid->local_id){}
  ItemID(ItemID* tid):_network_id(tid->_network_id),
    _local_id(tid->_local_id){}
  ItemID(unsigned long h, unsigned long t):_network_id(h), _local_id(t){}
  void fetch(istream &strm);
  unsigned long network_id(){return _network_id;}
  unsigned long local_id(){return _local_id;}
  void set(STATIC_ID* aid);
  void set(unsigned long network_id, unsigned long local_id);
  int operator == (ItemID& other);
  int operator >= (ItemID& other);
 private:
  unsigned long _network_id;
  unsigned long _local_id;
  OKorNOT store(ostream & strm);
  friend class Item;
  friend istream& operator >> (istream&, ItemID&);
};
ostream& operator<< (ostream& strm, ItemID& tid);
istream& operator >> (istream& strm, ItemID& tid);
inline ItemID::ItemID(void){_network_id = 0l; _local_id = 0l;}
inline void
ItemID::fetch(istream &strm)
{
  char ch;
  strm >> _network_id >> ch >> _local_id ;
}
inline void
ItemID::set(unsigned long network_id, unsigned long local_id)
{
  _network_id = network_id;
  _local_id = local_id;
}
inline void
ItemID::set(STATIC_ID* aid)
{
  _network_id = aid->network_id;
  _local_id = aid->local_id;
}
inline OKorNOT
ItemID::store(ostream &strm)
{
  strm << _network_id << ':' << _local_id << ' ';
  return OK;
}
inline int ItemID::operator==(ItemID& other)
{
  if (_network_id == other._network_id
      && _local_id == other._local_id)
    return YES;
  return NO;
}
inline int ItemID::operator>=(ItemID& other)
{
  if (_network_id < other._network_id)
    return 0;
  if (_local_id < other._local_id && _network_id == other._network_id)
    return 0;
  return 1;
}	
inline ostream& operator << (ostream& strm, ItemID& tid)
{
  strm << tid.network_id() << ":" << tid.local_id() << ' ';
  return strm;
}
inline istream& operator >> (istream& strm, ItemID& tid)
{
  char ch;
  strm >> tid._network_id >> ch >> tid._local_id;
  return strm;
}
#endif

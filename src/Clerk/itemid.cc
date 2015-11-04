/* $Id: itemid.cc,v 1.3 2003/01/15 18:54:07 marilyndavis Exp $ */ 
// itemid.cc -- manages the itemid.
/*********************************************************
 **********************************************************/
#include <fstream.h>
#include "evotedef.h"
#include "itemid.h"
void
ItemID::fetch(fstream &strm)
{
  char ch;
  strm >> _network_id >> ch >> _local_id ;
}
void
ItemID::set(unsigned long network_id, unsigned long local_id)
{
  _network_id = network_id;
  _local_id = local_id;
}
OKorNOT
ItemID::store(fstream &strm)
{
  strm << _network_id << ':' << _local_id << ' ';
  return OK;
}
int 
ItemID::operator==(ItemID& other)
{
  if (_network_id == other._network_id
      && _local_id == other._local_id)
    return YES;
  return NO;
}
int 
Item_IdID::operator>=(ItemID& other)
{
  if (_network_id < other._network_id)
    return 0;
  if (_local_id < other._local_id && _network_id == other._network_id)
    return 0;
  return 1;
}	
ostream& 
operator << (ostream& strm, ItemID& tid)
{
  strm << tid.network_id() << ":" << tid.local_id() << ' ';
  return strm;
}
istream& 
operator >> (istream& strm, ItemID& tid)
{
  char ch;
  strm >> tid._network_id >> ch >> tid._local_id;
  return strm;
}

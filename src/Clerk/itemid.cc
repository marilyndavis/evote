//
// eVote - Software for online consensus development.
// Copyright (C) 2015 Marilyn Davis <marilyn@deliberate.com>
//
// This file is part of eVote.
//
// eVote is free software: you can redistribute it and/or modify
// it under the terms of the GNU Affero General Public License as
// published by the Free Software Foundation, either version 3 of the
// License, or (at your option) any later version.
//
// eVote is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License
// along with eVote.  If not, see <http://www.gnu.org/licenses/>.
//

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

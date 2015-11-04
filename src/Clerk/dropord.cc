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

/* $Id: dropord.cc,v 1.3 2003/01/15 18:54:07 marilyndavis Exp $ */ 
// dropord.cc - manages the user-friendly ordering of the list
//              of items in the conference
/*********************************************************
 **********************************************************/
#include <fstream.h>
#include "evotedef.h"
#include "itemlist.h"
#include "item.h"
#include <fstream.h>
GLOBAL_INCS
#define DOBUNCH 10
#ifdef EDEBUG
void vmem(char* msg, YESorNO free = NO, int size = 0);
#endif
DroppingOrder::DroppingOrder(void):_next_did(1)
{
  _do_item = new Item * [DOBUNCH+1];
  _do_item[0] = NULL;
  _do_item[1] = NULL;
  _space = DOBUNCH;
#ifdef EDEBUG
      if (edebug & MEMS)
	{
	  vmem("Made _do_item in DroppingOrder", NO,
	       (DOBUNCH +1)* sizeof(Item*));
	}
#endif  
}
DroppingOrder::~DroppingOrder(void)
{
#ifdef EDEBUG
      if (edebug & MEMS)
	{
	  vmem("Deleted _do_item", NO, -(_space+1)*sizeof(Item*));
	}
#endif  
  delete[] _do_item;
}
void
DroppingOrder::attach(Item* p_item)
{
  if (_next_did +1 >= _space )
    {
      Item ** old_list;
      old_list = _do_item;
      _do_item = new Item*[_space + DOBUNCH + 1];
      memcpy((char*)_do_item, (char*)old_list, _next_did * sizeof(Item*));
      delete[] old_list;
      _space += DOBUNCH;
#ifdef EDEBUG
      if (edebug & MEMS)
	{
	  vmem("Remade _do_item in DroppingOrder", NO,
	       DOBUNCH * sizeof(Item*));
	}
#endif  
    }
  if (p_item->_status != DELETE )
    {
      if (p_item->_dropping_id == _next_did)
	{
	  _do_item[_next_did++] = p_item;
	  _do_item[_next_did] = NULL;
	}
      else
	if (p_item->_dropping_id > 0 && p_item->_dropping_id < _next_did)
	  _do_item[p_item->_dropping_id] = p_item;
    }
}
//   deletes from DroppingOrder -- from dropping_id start to end
void
DroppingOrder::delete_from(short start, short end)
{
  short dropping;
  if (start == -1)
    return;
  if (end == 0)
    end = start;
  if (end > _next_did -1)
    end = _next_did -1;
  dropping = (end - start + 1);
  while (++end < _next_did)
    {
      _do_item[start] = _do_item[end];
      _do_item[start]->_dropping_id = start++;
    }
  _do_item[start] = _do_item[end];  // null at _next_did
  _next_did -= dropping;
}
// *******************************************
//   void delete_from(Item* p_item)
void
DroppingOrder::delete_from(Item* p_item)
{
  if (p_item->dropping_id() == -1)
    return;
  delete_from(p_item->dropping_id());
}		
// 
Item*& 
DroppingOrder::operator[](short i)
{  
  static Item* nullitem = NULL;
  if (i < 1 || i >= _next_did)
    return nullitem;
  return _do_item[i];
}

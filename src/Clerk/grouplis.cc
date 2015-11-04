/* $Id: grouplis.cc,v 1.3 2003/01/15 18:54:07 marilyndavis Exp $ */ 
// grouplis.cc Functions for the GroupList class which manages
//     the list of grouped items for the conference.
/*********************************************************
 **********************************************************/
#include <iostream.h>
#include <fstream.h>
#include <strstream.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/msg.h>
#include "evotedef.h"
#include "itemlist.h"
#include "itemgrou.h"
#include "item.h"
#include "conf.h"
GLOBAL_INCS
#ifdef EDEBUG
void vmem(char* msg, YESorNO free = NO, int size = 0);
#endif  
//  ***************************************
//  GROUPLIST member functions
GroupList::GroupList(Conf *p_conf):_max_id(-1), _next_index(0),
				   _p_conf(p_conf)
{
  _space = GBUNCH;
  _p_group = new ItemGroup*[_space];
#ifdef EDEBUG
  if (edebug & MEMS)
    {
      vmem("Group List created its pointers to groups.", NO, 
	   _space*sizeof(ItemGroup*));
    }
#endif  
}
//  ***************************************
GroupList::~GroupList(void)
{
  int i;
  for (i = 0; i < _next_index; i++)
    {
      delete _p_group[i];
#ifdef EDEBUG
      if (edebug & MEMS)
	{
	  vmem("Group List deleted an Itemgroup.", NO,
	       -(sizeof(ItemGroup)));
	}
#endif  
    }
  delete[] _p_group;
#ifdef EDEBUG
  if (edebug & MEMS)
    {
      vmem("Group List deleted its list.", NO, 
	   -(_space*sizeof(ItemGroup*)));
    }
#endif  
}
//  ***************************************
void
GroupList::attach(ItemGroup * p_tg)
{
  if (find(p_tg->_id) != NULL)
    {
      clerklog << "Tried to reattach ItemGroup " << p_tg->_id;
      return;
    }
  if (_next_index >= _space)
    {
      int i;
      ItemGroup** old_list;
      old_list = _p_group;
      _p_group = new ItemGroup*[_space + GBUNCH];
      if (_p_group == NULL)
	{
	  clerklog << "GroupList::attach:No resources for another group.";
	  return;
	}
      for (i=0; i < _next_index; i++)
	_p_group[i] = old_list[i];
      delete[] old_list;
      _space += GBUNCH;
#ifdef EDEBUG
      if (edebug & MEMS)
	{
	  vmem("GroupList grew", NO, +GBUNCH*sizeof(ItemGroup*));
	}
#endif  
    }
  _p_group[_next_index++] = p_tg;
}
//  ***************************************
void
GroupList::detach(ItemGroup *p_tg)
{
  int i = -1;
  while (++i < _next_index && _p_group[i] != p_tg)
    ;
  if (_p_group[i] == p_tg)
    {
      while (++i <= _next_index)
	{
	  _p_group[i-1] = _p_group[i];
	}	
      _next_index--;
    }
  else
    {
      clerklog << "Can't detach Item Group " << p_tg->_id
	       << " from Group List for " << _p_conf->name();
    }
}
//  ***************************************
ItemGroup*
GroupList::find(short gid)
{
  int i;
  for (i = 0 ; i < _next_index ; i++)
    {
      if (gid == _p_group[i]->_id)
	return _p_group[i];
    }
  return NULL;
}
//  ***************************************
YESorNO
GroupList::last_group_complete(void)
{
  ItemGroup* ig;
  if ((ig = last_group()) == NULL)
    return YES;
  if (ig->_no_in_group != ig->_in_so_far
      && (ig->_p_item[0])->status() != DELETE)
    return NO;
  return YES;
}
//  ******************************************************
//   marks all the items in the group as deleted and deletes
//   the group from the list of groups and destroys the 
//   ItemGroup object.
short
GroupList::mark_delete(ItemGroup* group, unsigned long uid)
{
  short no_deletes;
  no_deletes = group->mark_delete_group(uid);
  detach(group);
  delete group;
#ifdef EDEBUG
  if (edebug & MEMS)
    {
      vmem("Deleted ItemGroup", NO,
	   -sizeof(ItemGroup));
    }
#endif  
  return no_deletes;
}

/* $Id: conflist.cc,v 1.3 2003/01/15 18:54:07 marilyndavis Exp $ */ 
/************************************************************
 *  conflist.cc  -- functions for maintaining the single list of
 *                  conferences in the whole system.
 *********************************************************
 *    Copyright (c) 1994...2015 Deliberate.com Patented.
 *    by Marilyn Davis
 **********************************************************/
extern "C" {
#include <string.h>
#include <stdio.h>
#include <time.h>
#include <sys/stat.h>
#include<unistd.h>
#ifdef linux
#include<linux/unistd.h>
#endif
#include <fcntl.h>
}
#include <stdlib.h>
#include <iostream.h>
#include <fstream.h>
#include <strstream.h>
#include "evotedef.h"
#include "conflist.h"
#include "conf.h"
#include "memlist.h"
extern MemList memlist;
GLOBAL_INCS
#ifdef EDEBUG
#define dumplog (*(_p_conf->ballot_box().dumper))
extern int edebug;
void vmem(char* msg, YESorNO free = NO, int size = 0);
#endif
char * uid_string(unsigned long uid);
ConfList::~ConfList(void)
{		
  char command[300];
  struct stat sbuf;
  store_all();	
  if (stat(_fname, &sbuf) != -1)
    {
      sprintf(command, "rm %s", _fname);
      system(command);
    }
}
//  ***********************************
//   The active file has a persistent list of files currently online.
void
ConfList::activate(Conf *the_conf)
{
  _active_file.open(_fname, ios::app, INFO_PERM);
  while (!_active_file)
    {
      if (adjourn_some(the_conf) == 0)
	{
	  clerklog << "Can't add " << the_conf->name() << " to active_confs file.";
	  clerklog << "Too many open files?  Coming down now.";
	  exit(0);
	}
      _active_file.open(_fname, ios::app, INFO_PERM);
    }
  _active_file << the_conf->name() << ' ';
  _active_file.close();
}
//  *********************************
//  adjourn - saves the conference files for resuming later.
OKorNOT
ConfList::adjourn(Conf* p_conf, unsigned long by_uid)
{
  if (p_conf->store_all() == OK) 
    {
      deactivate(p_conf);
      detach(p_conf);
#ifdef EDEBUG
      if (edebug & ADJOURNS)
	{
	  dlog << "Adjourning " << p_conf->name();
	  (*dlogger).flush();
	}
  if (edebug & MEMS)
    {
      char msg[100];
      sprintf(msg,"Deleting %s", p_conf->name());
      vmem(msg, NO, -sizeof(Conf));
    }
#endif
      delete p_conf;
    }
  else
    {
      clerklog << p_conf->name() << " not adjourned.";
      return NOT_OK;
    }
  return OK;
}
int
ConfList::adjourn_some(Conf * except)
{
  int adjourned = 0;
  Conf* p_conf = _first;
  Conf* p_next = _first;
  int still_active = 0;  
  static YESorNO starting = YES;
#ifdef EDEBUG
  void start_debug(YESorNO append);
  if (edebug & ADJOURNS)
    {
      dlog << "adjourn_some except == "
	   << (except ? except->name() : "none");
      /*      delete dlogger;
      dlogger = NULL;
      sprintf(debug_it, "ipcs >> %s%s", output_dir, dfile);
      system(debug_it);
      start_debug(YES); */
    }
#endif
  while (p_conf != NULL)
    {
      p_next = p_conf->_next;
      if (p_conf != except && p_conf->is_protected() == NO
	 && p_conf->community().first() == NULL) // no one online
	{
	  adjourn(p_conf, 0);   //redundant call?  No, necessary when
	  // all is quiet to bring down online confs.
	  adjourned++;
	}
      else
	{
	  still_active++;
#ifdef EDEBUG
	  if (edebug & ADJOURNS)
	    {
	      dlog << "Keeping " << p_conf->name();
	      (*dlogger).flush();
	    }
#endif
	}
      p_conf = p_next;
    }
  if (starting && !still_active)
    {
      start();
      starting = NO;
    }
#ifdef EDEBUG
  if (edebug & ADJOURNS)
    {
      dlog << "Adjourned " << adjourned << ", " << still_active 
	   << " still active.";
      /*      delete dlogger;
      dlogger = NULL;
      sprintf(debug_it, "ipcs >> %s%s", output_dir, dfile);
      system(debug_it);
      start_debug(YES); */
    }
#endif
  return adjourned;
}
//           ****************************
//          attaches the_conf pointer to the list after the after_conf
//          which was provided by the wheres() function below.
void
ConfList::attach_after(Conf *the_conf, Conf *after_conf)
{
  //  It could be that the after_conf was deleted after
  //  it was discovered if adjourn_some was called to free
  //  facilities.
  if (_first == NULL || after_conf == NULL)    //first in conf
    {
      if (_first != NULL)
	the_conf->_next = _first;
      _first = the_conf;
    }
  else
    {
      the_conf->_next = after_conf->_next;
      after_conf->_next = the_conf;
    }
  activate(the_conf);
}
//  *********************************************
//     deletes the conf without saving anything.
//     This is called when SyncConf is failing.
void
ConfList::burn(Conf* the_conf)
{
  detach(the_conf);
#ifdef EDEBUG
  if (edebug & MEMS)
    {
      char msg[100];
      sprintf(msg,"Deleting %s", the_conf->name());
      vmem(msg, NO, -sizeof(Conf));
    }
#endif
  delete the_conf;
}
//  *************************
//  create creates the new conference 
Conf *
ConfList::create(char *conf_name, short drop_day, YESorNO really_new)
{
  Conf *x, *belongs_after;
  
  if (strlen(conf_name) > MAX_FILE_NAME_LEN)
    conf_name[MAX_FILE_NAME_LEN] = '\0';
  x = new Conf(conf_name, drop_day, really_new);
  if (x->_status == NOT_OK)
    {
      burn(x);
      x = NULL;
    }
  else
    {
      wheres(conf_name, belongs_after );
      attach_after(x, belongs_after);
    }
#ifdef EDEBUG
  if (edebug & MEMS)
    {
      char mes[200];
      sprintf(mes, "Created %s", x->name());
      vmem(mes, NO, sizeof(Conf));
    }
#endif
  return x;
}
//  ***************************
//      deactivate
//         Marks the entry in the active_confs file.
void
ConfList::deactivate(Conf * the_conf)
{
  streamoff offset;
  char name[CONFLEN + 1];
  
  _active_file.open(_fname, ios::in|ios::out, INFO_PERM);
  while (!_active_file)
    {
      if (adjourn_some(the_conf) == 0)
	{
	  clerklog << "Can't remove " << the_conf->name() << 
	    " from active_confs file during deactivate.";
	  clerklog << "Too many open files?  Coming down now.";
	  exit(0);
	}
      _active_file.open(_fname, ios::in | ios::out, INFO_PERM);
    }
  while (1)
    {
      offset = _active_file.tellg();
      if (_active_file >> name)
	{
	  if (strcmp(name, the_conf->name()) == 0)
	    {
	      _active_file.seekp(offset? (offset + 1) : offset);
	      _active_file << 'X';
	      _active_file.close();
	      return;
	    }
	}
      else
	break;
    }
  _active_file.close();
  clerklog << "Couldn't mark " << the_conf->name() << " as adjourned in active_confs.";
}
//         **********************
//         detach
void
ConfList::detach(Conf *conf)
{
  Conf *comes_after;
  Conf *x;
  
  if ((x = wheres(conf->name(), comes_after)) != conf)
    {
      clerklog << conf->name() << "can't detach.";
      return;
    }
  if (comes_after != NULL)
    (comes_after)->_next = conf->_next;
  else  // detaching the first one
    _first = conf->_next;
}
//  *****************************************************
//    drops the conf forever but makes backup copies of files
OKorNOT
ConfList::drop(Conf* p_conf, unsigned long by_uid)
{
  char msg[80];
  OKorNOT backup_file(char *, char *, YESorNO);
  OKorNOT remove_file(char *, char *);
  
  time(&now);
  sprintf(msg,"Dropped by %lu: %s", by_uid, ctime(&now));
  if (p_conf->isempty() == YES)
    clerklog << "Dropped empty conf " << p_conf->name();
  if (p_conf->store_all() == OK 
     && backup_file(p_conf->name(), INFO_EXT, NO) == OK  // these utilities
     && backup_file(p_conf->name(), DATA_EXT, NO) == OK // are in
     && backup_file(p_conf->name(), BINF_EXT, NO) == OK
     && remove_file(p_conf->name(), INFO_EXT) == OK      // conf.cpp   
     && remove_file(p_conf->name(), BINF_EXT) == OK
     && remove_file(p_conf->name(), DATA_EXT) == OK)
    {
      deactivate(p_conf);
      detach(p_conf);
#ifdef EDEBUG
  if (edebug & MEMS)
    {
      char msg[100];
      sprintf(msg,"Deleting %s", p_conf->name());
      vmem(msg, NO, -sizeof(Conf));
    }
#endif
      delete p_conf;
    }
  else
    {
      clerklog << p_conf->name() << " not dropped.";
      return NOT_OK;
    }
  return OK;
}
//  YESorNO ConfList::does_uid_exist(unsigned long uid) asks if
//    the uid exists in any conference.  This is called
//    by WhoList::drop to be sure that we're not throwing
//    away a uid that is in use.
YESorNO
ConfList::does_uid_exist(unsigned long uid)
{
  ConfIter lister;
  Conf *x;
  
  while ((x = lister()) != NULL)
    {
      if (x->does_uid_exist(uid) == YES)
	{
	  return YES;
	}
    }
  return NO;
}
//   ************
//  fetch makes a new in-memory conference and reads it in from disk.
//     If it's not on disk, it returns a NULL.
Conf*
ConfList::fetch(char * conf_name)
{
  char info_file[CONFLEN + PATH_LEN + EXT_LEN +1];
  char data_file[CONFLEN + PATH_LEN + EXT_LEN +1];
  char binf_file[CONFLEN + PATH_LEN + EXT_LEN +1];
  
  fstream file;
  
  strcpy(info_file, output_dir);
  strcat(info_file, conf_name);
  strcpy(data_file, info_file);
  strcpy(binf_file, info_file);
  strcat(info_file, INFO_EXT);
  strcat(data_file, DATA_EXT);
  strcat(binf_file, BINF_EXT);
  file.open(info_file, ios::in|ios::nocreate);
  if (!file)
    {
      return NULL;
    }
  file.close();
  file.open(data_file, ios::in|ios::nocreate);	
  if (!file)
    return NULL;
  file.close();
  file.open(binf_file, ios::in|ios::nocreate);	
  if (!file)
    return NULL;
  file.close();
  return create(conf_name, 0, NO);
}
// *****************
//  find looks for the conf in the current in-memory confs.  If it can't
//    find it, it returns a NULL.
Conf * ConfList::
find(char *conf_name)
{
  Conf *comes_after;
  return wheres(conf_name, comes_after);
}
//  *********************************************
//   OKorNOT ConfList::groom()   finds something that needs doing and
//     does it.  If it finds something, it returns YES, if not NO.
YESorNO
ConfList::groom(void)
{
  Conf* now = _first;
  Conf* next = _first;
  Conf* worst_grow = NULL;
  Conf* worst_reorder = NULL;
  int cc, grow_check,  grow_need = 0;
  int items_to_drop, voter_spots_left;
  int ballots_per_block, ballot_size, item_bytes_left;
  unsigned long voters_to_drop, reorder_check, reorder_need = 0;
  
  while (next != NULL)
    {
      now = next;
      next = next->_next;
      voters_to_drop = now->voters_to_drop() + now->ready_to_drop();
      items_to_drop = now->items_to_drop();
      reorder_check = voters_to_drop + items_to_drop;
      voter_spots_left = now->voter_spots_left();
      ballots_per_block = now->ballots_per_block();
      cc = ballots_per_block/GROW_TRIGGER;
      if ((cc -= voter_spots_left) > 0)
	reorder_check += 2 * cc;  // more weight for too little space
      // than for drops
      if (reorder_check > reorder_need)
	{
	  reorder_need = reorder_check;
	  worst_reorder = now;
	}
      ballot_size = now->ballot_size();
      item_bytes_left = (int)(now->item_bytes_left());
      if ((grow_check = ballot_size/GROW_TRIGGER 
	   - item_bytes_left) > grow_need
	  && (strncmp(now->name(), "petition", 8) != 0))
	{
	  grow_need = grow_check;
	  worst_grow = now;
	}
#ifdef EDEBUG
      if (edebug & GROOM)
	{
	  dlog << "\nGroom considering " << now->name();
	  dlog << "\n  voters to drop = " << voters_to_drop
	    << " items to drop = " << items_to_drop;
	  dlog << "\n  voter_spots_left = " << voter_spots_left
	    << " ballots_per_block = " << ballots_per_block;
	  dlog << "\n  ballot size = " << ballot_size
	    << " item bytes left = " << item_bytes_left;
	  dlog << "\n  reorder check = " << reorder_check
	    << " grow check = " << grow_check;
	}
#endif
    }
  if (grow_need > 0)
    {				
#ifdef EDEBUG
      if (edebug & GROOM)
	{
	  dlog << "\nGroom growing " << worst_grow->name();
	}
#endif
      worst_grow->grow_ballot();
      return YES;
    }
  if (reorder_need > 0)
    {
#ifdef EDEBUG
      if (edebug & GROOM)
	{
	  dlog << "\nGroom reordering " << worst_reorder->name();
	}
#endif
      worst_reorder->reorder();
      return YES;
    }
  return NO;
}
//  **************************************************************
//      logs to the clerklog all the conferences containing uid
int
ConfList::log(unsigned long uid, YESorNO really)
{
  Ballot *p_ballot;
  ConfIter lister;
  IT_CHOICE bhow = START;
  Conf * p_conf = NULL;
  int count = 0;
  while ((p_conf = lister()))
    {
      bhow = START;
      while ((p_ballot = p_conf->ballot_box().iterator(bhow))
	     != NULL)
	{
	  bhow = NEXT;
	  if (*(unsigned long*)p_ballot == uid)
	    {
	      if (really)
		clerklog << uid << " is in " << p_conf->name();
	      count++;
	    }
	}
    }
  return count;
}
//  **************************************************************
//     if choice == START, hands back the first p_conf online now.
//     if choice == NEXT,  passes back the next conf.
Conf*
ConfList::on_iterator(IT_CHOICE choice)
{
  static Conf * last_conf = NULL;	
  
  if (choice == START || last_conf == NULL)
    {
      last_conf = _first;
      return _first;
    }
  
  last_conf = last_conf->_next;
  return last_conf;
}
// **************************************************
//  new_exe  reorders all the conferences and then
//  quits.  The quitting part is handled by main
void
ConfList::new_exe(void)
{
  Conf* x = _first;
  
  while (x != NULL)
    {
      x->reorder(YES, YES);
      x = x->_next;
    }
}
//  *******************************
//  start()  run at startup to look for
//       confs that have been left in a partially saved state.
void
ConfList::start(void)
{ 
  Conf* p_conf;
  char fname2[CONFLEN + PATH_LEN + EXT_LEN + 1];
  fstream work_file;
  char conf_name[CONFLEN + 1];
  char command[300];
  int count = 0;
  
  strcpy(_fname, output_dir);
  strcat(_fname, "active_confs");
  work_file.open(_fname, ios::in, INFO_PERM);
  if (!work_file) // good, the Clerk came down gracefully
    {
      _active_file.open(_fname, ios::out, INFO_PERM);
      if (!_active_file)
	{
	  cout << "Clerk: Can't open active_confs file.";
	  exit(0);
	}
      _active_file.close();
      return;
    }
  work_file.close();
  strcpy(fname2, output_dir);
  strcat(fname2, "temp");
  sprintf(command,"mv %s %s", _fname, fname2);
  while (1)
    {
      system(command);
      work_file.open(fname2, ios::in, INFO_PERM);
      if (work_file)
	break;
      if (++count > 5)
	{
	  cout << "Clerk:Can't open work file, temp.";
	  exit(0);
	}	
      sleep(1);
    }
  while (work_file >> conf_name)
    {
      if (conf_name[0] == 'X')
	continue;
      p_conf = fetch(conf_name);
      p_conf->check_ballots();
      p_conf->check_stats();
    }
  work_file.close();
  sprintf(command,"rm %s", fname2);
  system(command);
}
// *******************************
//  store_all stores all the conferences that are in memory onto disk.
//            It then deletes them from memory.
OKorNOT
ConfList::store_all(char *except = NULL)
{
  Conf* now = NULL;
  Conf* next = _first;
  OKorNOT all_ok = OK;
  
  while (next != NULL)
    {
      now = next;
      next = now->_next;
      if (except == NULL || strcmp(except, now->name()) != 0)
	if (now->store_all() != OK)
	   {
	     all_ok = NOT_OK;
	   }
      memlist.mid_dropped(now->memid());
      detach(now);
#ifdef EDEBUG
  if (edebug & MEMS)
    {
      char msg[100];
      sprintf(msg,"Deleting %s", now->name());
      vmem(msg, NO, -sizeof(Conf));
    }
#endif
      delete now;
    }
  return all_ok;
}
//        ****************************
//    wheres finds the conf_name in the in-memory conferences if it can.
//           If it's not there, it returns NULL but returns in belongs_
//           after a pointer to the conf that would come before the 
//           sought-after conf, if the sought-after conf was there.	
Conf *
ConfList::wheres(char *conf_name, Conf *&belongs_after)
{
  Conf * checker = _first;
  int cc = 0;
  
  belongs_after = NULL;
  while (checker != NULL)
    {
      if ((cc = strcmp(checker->name(), conf_name)) >= 0)
	break;
      belongs_after = checker;
      checker = checker->_next;
    }
  if (cc == 0)
    return checker;
  return NULL;
}
//        ***************
//        operator <<   - to ostream - lists the names of all the in-memory
//                        conferences.
ostream& operator << (ostream& strm, ConfList& list)
{
  Conf *x = list.on_iterator(START);
  char c = '\t';
  int i = 0;
  
  strm << '\n';
  while (x != NULL)
    {
      strm << x->name()  << c;
      x = list.on_iterator(NEXT);
      c = (++i%4 ? '\t' : '\n');
    }
  strm << '\n';
  return strm;
}
// ConfIter  -- little class for generation iterators through
//              the conferences via the data files.
extern ConfList conferences; 
ConfIter::ConfIter(void)
{
  char path[CONFLEN + PATH_LEN + EXT_LEN +1];
  strcpy(path, output_dir);
  if (path[strlen(path)] == '/')
    path[strlen(path)] = '\0';
  dirp = opendir(path);
  if (dirp == NULL)
    {
      clerklog << "Can't read directory " << dirp << ".  Coming down.";
      exit(1);
    }
}
Conf * ConfIter::operator()()
{
  char conf_name[CONFLEN + 1];
  int len;
  Conf * answer;
  while ((dp = readdir(dirp)) != NULL)
    {
      if (dp->d_name[0] == '~')
	continue;
      strcpy(conf_name, dp->d_name);
      len = (int)strlen(conf_name);
      if (strcmp(conf_name+len-3,"dat") != 0)
	continue;
      conf_name[len-4] = '\0';
      if ((answer = conferences.find(conf_name)) == NULL)
	{
	  if ((answer = conferences.fetch(conf_name)) == NULL)
	    {
	      clerklog << "Can't fetch " << conf_name 
		<< " in ConfIter.  Coming down.";
	      exit(1);
	    }
	}
      return answer;
    }
  closedir(dirp);
  return NULL;
}

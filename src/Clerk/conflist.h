/* $Id: conflist.h,v 1.3 2003/01/15 18:54:07 marilyndavis Exp $ */ 
// conflist.h -- header file for the ConfList class which has 
// responsibility for the one list of conferences.
/*********************************************************
 *    Copyright (c) 1994...2015 Deliberate.com Patented.
 *    by Marilyn Davis
 **********************************************************/
#ifndef CONFLISTHPP
#define CONFLISTHPP
#include <dirent.h>
/*#include <linux/dirent.h>
#include <linux/unistd.h>
#include <unistd.h> */
#include<iostream.h>
#include<fstream.h>
class Conf;

class ConfIter
{
 private:
  Conf * current_conf;
  struct dirent * dp;
  DIR *dirp;
 public:
  ConfIter(void);
  Conf * operator()();
};
class ConfList
{
 public:
  ConfList(void);
  ~ConfList(void);
  OKorNOT adjourn(Conf *p_conf, unsigned long by_uid);
  int adjourn_some(Conf *except);
  void burn(Conf *p_conf);
  Conf* create(char *conf_name, short drop_day, YESorNO really_new = YES);
  YESorNO does_uid_exist(unsigned long uid);
  OKorNOT drop(Conf* p_conf, unsigned long by_uid);
  void expose(Conf *p_conf);
  Conf *fetch(char *conf_name);	
  Conf *find(char *conf_name);
  Conf *first(void);
  unsigned long get_local_id(void);
  YESorNO groom(void);
  int log(unsigned long uid, YESorNO really = YES);
  void new_exe(void);
  Conf *on_iterator(IT_CHOICE choice);
  void protect(Conf *p_conf);
  void start(void);
  OKorNOT store_all(char * except = NULL); 
  friend ostream& operator << (ostream&, ConfList&);
 private:
  Conf *_first;
  fstream _active_file;
  char _fname[CONFLEN + PATH_LEN + EXT_LEN + 1];
  Conf *_protected_conf;
  void activate(Conf * the_conf);
  void attach_after(Conf *the_conf, Conf *after_this);
  void deactivate(Conf *the_conf);
  void detach(Conf *the_conf);
  Conf *wheres(char *conf_name, Conf *&belongs_after);
};
inline ConfList::ConfList(void):_first(NULL), _protected_conf(NULL){};
inline void ConfList::expose(Conf* p_conf){_protected_conf = NULL;}
inline Conf * ConfList::first(void){return _first;}
inline void ConfList::protect(Conf* p_conf){_protected_conf = p_conf;}
ostream& operator<< (ostream&, ConfList&);
#endif

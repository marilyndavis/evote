/* $Id: wholist.h,v 1.3 2003/01/15 18:54:07 marilyndavis Exp $ */ 
// wholist.h -- defines WhoList class which is responsible for the
//              email address <=> voter id translation
/*********************************************************
 *    Copyright (c) 1994...2015 Deliberate.com Patented.
 *    by Marilyn Davis
 **********************************************************/
#ifndef WHOLISTHPP
#define WHOLISTHPP
#define CHANGE_LIMIT 25  /* backs up after this many changes */
#include<fstream.h>
class WhoList
{
 public:
  WhoList(void);
  ~WhoList(void);
  OKorNOT drop(unsigned long entry, YESorNO force = NO, 
	       int * subs = NULL);
  OKorNOT joins(unsigned long who_id);
  OKorNOT move(char *was, char*is);
  OKorNOT sync(void);
  unsigned long whonum(char* name, YESorNO add,
		       int subs = 0, unsigned long id = 0L);
  OKorNOT whois(char* out_buffer, unsigned long who_id);
  OKorNOT whosync(void);
  void start(void);
 private:
  streampos _alpha_hash[26];
  char _fname[CONFLEN + PATH_LEN + EXT_LEN + 1];
  int _changes;
  unsigned long collapse(int *count, unsigned long * ins);
  int collect_subs(fstream * from_file);
  void grow(unsigned long this_big);
  unsigned long insert_entry(char * name, streampos offset,
			     int subs = 0, unsigned long id = 0L);
  unsigned long new_number(void);
  void open_who_file(void);
  void write_subs(int no_of_subs);
  streampos _last_byte;
  streampos *_numbers;
  unsigned long _n_space;
  fstream _who_file;
#ifdef EDEBUG
  void WhoList::check(void);
#endif
};
#endif

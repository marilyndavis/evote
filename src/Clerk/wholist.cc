/* $Id: wholist.cc,v 1.5 2003/01/15 18:54:07 marilyndavis Exp $ */ 
// wholist.cc Maintains the WhoList, the list of email addresses
// and associated voter id's.
/*********************************************************
 **********************************************************/
#include <stdlib.h>
#include <iostream.h>
#include <fstream.h>
#include <strstream.h>
#include <stdio.h>
#include <errno.h>
//   TOINDEX maps the first character of an email address to
//   26 indices:
//                               TOINDEX      TOSORT
//    'a'/'A' to 'z'/'Z' ---->   0 to 25      0 to 25
//    < 'A'              ---->   -1           -many to -1
//    > 'z'              ---->   25           more than 25
#define TOINDEX(X) ((X) >= 'A' && (X) <= 'Z' ? ((X) - 'A') : \
		    ((X) >= 'a' && (X) <= 'z' ? ((X) - 'a') : \
		     ((X) < 'A' ? -1 : 25)))
#define TOSORT(X) ((X) >= 'A' && (X) <= 'Z' ? ((X) - 'A') : \
		    ((X) >= 'a' && (X) <= 'z' ? ((X) - 'a') : \
		     ((X) < 'A' ? (X) - 'A' : 25 + ((X) - 'Z'))))
#define NUMBLOCK 2
#include "evotedef.h"
GLOBAL_INCS
#include "conflist.h"
#include "conf.h"
#include "ballot.h"
#include "wholist.h"
extern ConfList conferences;
extern WhoList wholist;
#ifdef EDEBUG
#define dumplog (*(_p_conf->ballot_box().dumper))
extern int edebug;
void vmem(char* msg, YESorNO free = NO, int size = 0);
#endif
OKorNOT backup_file(char *name, char *ext, YESorNO remove);
extern YESorNO sync_who;  /* sneak a flag back to main */
extern char * whereami;  /* in main only */
//************************************************************
WhoList::~WhoList(void)
{
  _who_file.close();
}
//************************************************************
// Takes out the email addresses that have been deleted
unsigned long
WhoList::collapse(int *count, unsigned long * ins)
{
  fstream tmp_file;
  char tmp[CONFLEN + PATH_LEN + EXT_LEN + 1];
  char command[2*(CONFLEN + PATH_LEN + EXT_LEN + 1)];
  char ch;
  int subs;
  char name[200];
  unsigned long i;
  unsigned long num;
  streampos subspot;
  unsigned long in = 0;
  unsigned long out = 0;
  int ats;
  _who_file.close();
  backup_file("who", ".list", NO);
  sprintf(tmp,"%s%s", output_dir, "who.tmp");
  sprintf(command, "mv %s %s", _fname, tmp);
  system(command);
  open_who_file();
  do
    {
      tmp_file.open(tmp, ios::in, DATA_PERM);
      if (!tmp_file )
	{
	  clerklog << "Can't open file: " << tmp << ".   Coming down.";
	  exit(0);
	}
    }
  while (!tmp_file);
  tmp_file.seekg(ios::beg);
  while (tmp_file >> ch)
    {
      if (ch == '<')
	{
	  i = 0;
	  ats = 0;
	  continue;
	}
      if (ch == '@' && ++ats > 3)
	break;
      if (ch != '>')
	{
	  name[i++] = ch;
	  continue;
	}
      name[i] = '\0';
      subspot = tmp_file.tellg();
      subs = collect_subs(&tmp_file);
      tmp_file >> num;
      if (subs > 0)
	{
	  in++;
	  if (count[num] != -1)
	    {
	      clerklog << "Trouble in collapse.  Still a mismatch with user " 
		       << num << ".  Exiting.";
	      exit(1);
	    }
	  _who_file << '<' << name << '>';
	  write_subs(subs);
	  _who_file << num;
	  continue;
	}
      // subs == 0
      out++;
      if (count[num] != -2)
	{
	  clerklog << "Trouble in collapse.  Still a mismatch with user " 
		   << num << ".  Exiting.";
	  exit(1);
	}
    }
  _who_file.close();
  sprintf(command,"rm %s", tmp);
  system(command);
  for (i = 0; i < _n_space; i++)
    {
      _numbers[i] = (streampos)(-1);
    }
  start();
  *ins = in;
  return out;
}
//************************************************************
//    force == YES   drop the voter no matter what
//    force == MAYBE  only mark the subs as zero and don't drop yet
//                   This means that a list is being dropped and a           
//                   sync will happen at the end.
OKorNOT
WhoList::drop(unsigned long dropper, YESorNO force, int *subs)
{
  char buf[1024];
  streampos bytes;
  streampos i;
  streampos next_read = (streampos)0;
  streampos next_write = (streampos)0;
  unsigned long number;
  char ch;
  streampos subtract;
  streampos sub_at = (streampos)0;
  int namei = -2;
  int subscribed = 0;
  if (force != MAYBE && ++_changes > CHANGE_LIMIT)
    {
      _changes = 0;
      _who_file.flush();
      backup_file("who",".list", NO);
    }
#ifdef EDEBUG
  if (edebug & WHODUMP)
    {
      dlog << "\n\nAbout to drop " << dropper ;
      check();
    }
#endif		     
  if (dropper >= _n_space || _numbers[dropper] < (streampos)0)
    return NOT_OK;
  next_write = _numbers[dropper];
  _who_file.seekg(next_write);
  if (!(_who_file >> ch) || ch != '<')
    {
      clerklog << "Who file out of sync somehow while dropping "
	       << dropper << ". \n Expect a < at " << next_write
	       << " but got " << (char)ch << ". \n Coming down now.";
      exit(1);
    }
  i = 0;
  while (_who_file >> ch)
    {
      if (namei == -2)
	namei = TOINDEX(ch);
      if (ch != '>')
	{
	  buf[i++] = ch;
	  continue;
	}
      buf[i] = '\0';
      sub_at = _who_file.tellg();
      subscribed = collect_subs(&_who_file) - 1;
      _who_file >> number;
      if (number != dropper)
	{
	  clerklog << "Whofile and _numbers[] messed up dropping "
		   << dropper << " = " << buf << ". Coming down.";
	  exit(1);
	}
      if (subs != NULL)
	*subs = subscribed;
      if (force == MAYBE || (force == NO && subscribed > 0))
	{
	  _who_file.seekp(sub_at);
	  write_subs(subscribed);
	  return UNDECIDED;
	}
      /*  conference.does_uid_exist is a very expensive call! */
      if (force == NO && conferences.does_uid_exist(dropper) == YES)
	{
	  clerklog << "Can't delete " << buf << " = "
		   << dropper << " from who.list.  Still subscribed to some list.";
	  return PROBLEM;
	}
      subtract = _who_file.tellg() - next_write;
      _last_byte -= subtract;
      while (_who_file.read(buf, 1024))
	{
	  next_read = _who_file.tellg();
	  _who_file.seekp(next_write);
	  _who_file.write(buf, 1024);
	  next_write = _who_file.tellp();
	  _who_file.seekg(next_read);
	}
      bytes = _who_file.gcount();
      _who_file.clear();
      _who_file.seekp(next_write);
      _who_file.write(buf, bytes);
      _who_file.seekg(_last_byte + (streampos)1);
      bytes = 0L;
      while (_who_file >> ch)
	{
	  bytes++;
	}
      _who_file.clear();
      _who_file.seekp(_last_byte + (streampos)1);
      if (bytes > 1024)
	bytes = 1024;
      for (i = 0; i < bytes; i++)
	{
	  buf[i] = '@';
	}
      if (!(_who_file.write(buf, bytes)))
	{
	  clerklog << "what";
	}
      for (i = 1; i < _n_space; i++)
	{
	  if (_numbers[i] > _numbers[dropper])
	    _numbers[i] -= subtract;
	}
      _numbers[dropper] = (streampos)-(streampos)1;
      for (i = namei + (streampos)1; i < (streampos)26; i++)
	{
	  _alpha_hash[i] -= subtract;
	}
#ifdef EDEBUG
      if (edebug & WHODUMP)
	{
	  dlog << "\n\nJust dropped " << dropper ;
	  check();
	}
#endif		     
      return OK;
    }
  _who_file.clear();
  _who_file.flush();
#ifdef EDEBUG
  if (edebug & WHODUMP)
    {
      dlog << "\n\nDidn't drop " << dropper ;
      check();
    }
#endif		     
  return NOT_OK;
}
//************************************************************
// Returns the number of email lists that the voter is subscribed to
int
WhoList::collect_subs(fstream * from_file)
{
  int i;
  char sub[4];
  int subscribed;
  sub[3] = '\0';
  for (i=0; i < 3; i++)
    {
      if (!(*from_file >> sub[i]))
	{
	  clerklog << "Can't read subs in whofile.  Coming down.";
	  exit(1);
	}
    }
  if ((subscribed = atoi(sub)) == 0)
    {
      for (i=0; i < 3; i++)
	if (sub[i] != '0')
	  {
	    clerklog << "Whofile messed up around subs.  Coming down.";
	    exit(1);
	  }
    }
  return subscribed;
}
//************************************************************
void
WhoList::grow(unsigned long new_top)
{
  streampos *new_space;
  unsigned long i;
  unsigned long new_size;
  if (_n_space > new_top)
    return;
  new_size = _n_space;
  do
    {
      new_size += NUMBLOCK;
    }
  while (new_size < new_top + 1);
  if ((new_space = new streampos[new_size]) == NULL)
    {
      clerklog << "Unable to grow wholist number list";
      exit(1);
    }
  if (_numbers != NULL)
    {
      memcpy(new_space, _numbers, sizeof(streampos) * _n_space);
      delete[] _numbers;
    }
  for (i = _n_space; i < new_size; i++)
    {
      new_space[i] = (streampos)(-1);
    }
#ifdef EDEBUG
  if (edebug & MEMS)
    {
      char msg[100];
      sprintf(msg,"Grew wholist from %ld to %ld.",
	      (long)sizeof(streampos) * _n_space,
	      (long)sizeof(streampos) * new_size);
      vmem(msg, NO, 		  
	   sizeof(streampos) * new_size - sizeof(streampos) * _n_space);
    }
#endif
  _n_space = new_size;
  _numbers = new_space;
}
//************************************************************
unsigned long
WhoList::insert_entry(char * name, streampos offset,
		      int subs = 0, unsigned long id = 0L)
{
  char buf[2][1024];
  YESorNO end = NO;
  int jflop = +1;
  int kflop = -1;
  streampos i;
  unsigned long n;
  int j = 1;
  int k = 0;
  streampos bytes = (streampos)1024;
  streampos next_read;
  streampos next_write;
  streampos add;
  YESorNO is_valid(char *);
  if (!is_valid(name))
    {
      clerklog << "WhoList rejected invalid email address: "
	       << name;
      return 0L;
    }
  if (id == 0)
    id = new_number();
  if (++_changes > CHANGE_LIMIT)
    {
      _changes = 0;
      _who_file.flush();
      backup_file("who",".list", NO);
    }
#ifdef EDEBUG
  if (edebug & WHODUMP)
    {
      dlog << "\n\nAbout to insert " << name << " at offset " << offset
	   << " with id = " << id;
      check();
    }
#endif		     
  _who_file.clear();
  _who_file.seekg(offset);
  if (!_who_file.read(buf[0], 1024))
    {
      bytes = _who_file.gcount();
      _who_file.clear();
      end = YES;
    }
  else
    {
      next_read = _who_file.tellg();
    }
  _who_file.seekp(offset);
  _who_file << '<' << name << '>';
  write_subs(subs);
  _who_file << id ;
  next_write = _who_file.tellp();
  add = next_write - offset;
  _last_byte += add;
  while (end == NO)
    {
      _who_file.seekg(next_read);
      if (!_who_file.read(buf[j], 1024))
	{
	  bytes = _who_file.gcount();
	  _who_file.clear();
	  end = YES;
	}
      else
	{
	  next_read = _who_file.tellg();
	}
      _who_file.seekp(next_write);
      _who_file.write(buf[k], 1024);
      next_write = _who_file.tellp();
      k += (kflop *= -1 );  /* k started as 1 */
      j += (jflop *= -1 );  /* j started as 0 */
    }
  /* lop off garbage */
  for (i = (streampos)2; i < bytes ; i++)
    {
      if (buf[k][i] == '@' && buf[k][i-1] == '@' && buf[k][i-2] == '@')
	{
	  bytes = i - (streampos)2;
	  break;
	}
    }
  _who_file.seekp(next_write);
  _who_file.write(buf[k], bytes);
  for (n = 1 ; n < _n_space; n++)
    {
      if (_numbers[n] >= offset)
	{
	  _numbers[n] += add;
	}
    }
  _numbers[id] = offset;
  for (n = TOINDEX(name[0]) + 1; n < 26 ; n++)
    {
      _alpha_hash[n] += add;
    }
#ifdef EDEBUG
  if (edebug & WHODUMP)
    {
      dlog << "\n\nJust inserted " << name << " with id = " << id;
      check();
    }
#endif		     
  return id;
}
//************************************************************
YESorNO
is_valid(char *name)
{
  int i;
  int at;
  int afterdot;
  int domain;
  int dots;
  YESorNO compart = NO;
  for (i = 0, at = 0, dots = 0, afterdot = 0; name[i]; i++)
    {
      if (name[i] == '@')
	{
	  at++;
	  domain = i+1;
	}
      if (name[i] == '.')
	{
	  if (afterdot == 3)
	    compart = YES;
	  afterdot = 0;
	  dots++;
	}
      else
	afterdot++;
    }
  /*  if (afterdot == 3)*/
  compart = YES;
  /* checking for a local domain */
  if (whereami[0] != '\0' && at == 1 && dots < 2
      && (strcmp(&name[domain], whereami) == 0
	  || strcmp(&name[domain], "localhost.localdomain") == 0))
    return YES;
  if (at != 1 || afterdot > 3 || compart == NO)
    return NO;
  return YES;
}
//************************************************************
OKorNOT
WhoList::joins(unsigned long who_id)
{
  streampos sub_at;
  char ch;
  int subs;
  unsigned long num;
  if (_numbers)
    _who_file.seekg(_numbers[who_id]);
  if (!_numbers || !(_who_file >> ch) || ch != '<')
    {
      clerklog << "Can't find user " << who_id << " in who.list for joining.";
      return NOT_OK;
    }
  while (_who_file >> ch)
    {
      if (ch != '>')
	continue;
      break;
    }
  sub_at = _who_file.tellg();
  subs = collect_subs(&_who_file);
  _who_file >> num;
  if (num != who_id)
    {
      clerklog << "Can't find user " << who_id << " in who.list for joining.";
      return NOT_OK;
    }
  _who_file.seekp(sub_at);
  write_subs(++subs);
  return OK;
}
WhoList::WhoList(void):_changes(0), _last_byte(streampos(-1)),  
  
  _numbers(NULL), _n_space(0L)
{}
//************************************************************
void 
WhoList::open_who_file(void)
{     
  strcpy(_fname, output_dir);
  strcat(_fname, "who.list");
  do
    {
      _who_file.open(_fname, ios::in|ios::out, DATA_PERM);
      if (!_who_file )
	{
	  clerklog << "Can't open who_file: " << _fname << ".   Coming down.";
	  exit(0);
	}
    }
  while (!_who_file);
  _who_file.seekg(ios::beg);
}
//************************************************************
void
WhoList::start(void)
{
  int lasti = - 1;
  int thisi;
  char ch;
  streampos name_off = (streampos)0;
  unsigned long num;
  for (ch = 0; ch < 26; ch ++)
    _alpha_hash[ch] = (streampos)0;
  open_who_file();
  while (_who_file >> ch)
    {
      if (ch == '<')
	{
	  name_off = _who_file.tellg() - (streampos)1;
	  if (!(_who_file >> ch))
	    {
	      clerklog << "Nothing after < in who file.  Coming down.";
	      exit(0);
	    }
	  if ((thisi = TOINDEX(ch)) > lasti)
	    {
	      int i;
	      for (i= lasti + 1; i < 26 ; i++)
		{
		  _alpha_hash[i] = name_off;
		}
	      lasti = thisi;
	    }
	}
      if (ch != '>')
	continue;
      collect_subs(&_who_file);
      if (!(_who_file >> num))
	{
	  clerklog << "No number after right arrow in who file.  Coming down.";
	  exit(0);
	}
      if (num >= _n_space)
	grow(num);   /* exits if it finds trouble */
      _numbers[num] = name_off;
      _last_byte = _who_file.tellg() - 1;
    }
  _who_file.clear();
  _changes = 0;
}
//************************************************************
// moves the voter from was email address to is
OKorNOT
WhoList::move(char *was, char * is)
{
  unsigned long num;
  int subs;
  if (!is_valid(is))
    return NOT_OK;
  if ((num = whonum(was, NO)) == 0)
    {
      if ((num = whonum(is, NO)) != 0)
	return UNDECIDED;
      return NOT_OK;
    }
  if (whonum(is, NO) != 0)
    return PROBLEM;
  if (drop(num, YES, &subs) != OK)
    return STOP;                   // impossible
  if (whonum(is, YES, subs, num) != num)
    {
      return STOP;                  // impossible
    }
  return OK;
}
//************************************************************
// gets the next unused id
unsigned long
WhoList::new_number(void)
{
  unsigned long i;
  for (i = 1; i < _n_space ; i++)
    {
      if (_numbers[i] == -1)
	return i;
    }
  grow(i);
  return i;
}
//************************************************************
// Syncronizes the wholist in four steps --
//  1.  Drops people who have been on vacation for a long time
//  2.  Counts through all the conferences for how many 
//      the id is subscribed to
//  3.  Makes the subs count be correct.
//  4.  Drops addresses that aren't subscribed and complains about
//      subscribed ids who aren't on the list
// main() calls this 4 times for each request to sync the wholist
// so that it can also watch for users who send regular requests
OKorNOT
WhoList::sync(void)
{
  Ballot *p_ballot;
  IT_CHOICE bhow = START;
  Conf * p_conf = NULL;
  static int * count;
  unsigned long i;
  unsigned long num, ins, outs = 0;
  streampos subspot;
  char ch;
  int subs;
  char name[200];
  OKorNOT cc = OK;
  static int state = 0;
  IT_CHOICE choice = START;
  while ((p_conf = conferences.on_iterator(choice)) != NULL)
    { 
      choice = NEXT;
      if (p_conf->community().first() != NULL)
	{
	  // some conf is active -- not a good time for this
	  sync_who = YES; // set a flag to do it when things settle
	  return UNDECIDED;
	}
    }
  switch (++state)
    {
    case 1:
      clerklog << "Starting who.list sync";
      count = new int[_n_space];
      for (i = 0; i < _n_space ; i++)
	count[i] = 0;
      _who_file.flush();
      backup_file("who",".list", NO);
      _changes = 0;
      /* first we find and drop people on vacation for a year */
      clerklog << "Vacation droppers.";
      {  // to scope the ConfIter
	ConfIter list;
	while ((p_conf = list()) != NULL)
	  {
	    /*	    if (strncmp(p_conf->name(), "petition", 8) == 0)
		    continue; */
	    bhow = START;
	    p_conf->protect();  // So that adjourn_some
	    // doesn't take this one off line
	    while ((p_ballot = p_conf->ballot_box().iterator(bhow))
		   != NULL)
	      {
		bhow = NEXT;
		if ((p_ballot->action() & VACATION )
		    && time(&now) - p_ballot->mod_date()
		    > 365 *86400)
		  {
		    time_t x;
		    char check[200];
		    x = p_ballot->mod_date();
		    strcpy(check, ctime(&x));
		    whois(name, *(unsigned long*)p_ballot);
		    clerklog << "Dropping " << *(unsigned long *)p_ballot 
			     << " = " << name << " from " << p_conf->name()
			     << ", on vacation since " << check;
		    count[*(unsigned long*)p_ballot] = -1;
		    outs++;
		  }
	      }
	    for (i = 0; i < _n_space; i++)
	      {
		if (count[i] == -1)
		  {
		    count[i] = 0;
		    p_conf->drop_voter(i, (unsigned long)0, NO);
		  }
	      }
	    p_conf->expose();
	  }
      }
      sync_who = YES;
      clerklog << outs << " dropped.";
      return UNDECIDED;
    case 2:
      clerklog << "Counting through the data.";
      /* Now we count up what's left */
      if (_changes)
	{
	  _who_file.flush();
	  backup_file("who",".list", NO);
	  _changes = 0;
	}	  
      {
	ConfIter lister;
	while ((p_conf = lister()))
	  {
	    bhow = START;
	    while ((p_ballot = p_conf->ballot_box().iterator(bhow))
		   != NULL)
	      {
		if (p_ballot->action() != DROP)
		  {
		    unsigned long index;
		    if ((index = *(unsigned long*)p_ballot) > _n_space -1)
		      {
			clerklog << p_conf->name() << " has a strange ballot on uid "
				 << index ;
		      }
		    else
		      {
			++count[*(unsigned long*)p_ballot];
		      }
		  }
		bhow = NEXT;
	      }
	  }
      }
      sync_who = YES;
      return UNDECIDED;
    case 3:
      _who_file.seekg(ios::beg);
      while (_who_file >> ch)
	{
	  if (ch == '<')
	    {
	      i = 0;
	      continue;
	    }
	  if (ch != '>')
	    {
	      name[i++] = ch;
	      continue;
	    }
	  name[i] = '\0';
	  subspot = _who_file.tellg();
	  subs = collect_subs(&_who_file);
	  _who_file >> num;
	  if (subs != count[num])
	    {
	      outs++;
	      // double check, could have changed since state == 1
	      if (_changes)
		count[num] = conferences.log(num, NO);
	      if (subs != count[num])
		{
		  clerklog << name << ", who_id = " << num << " had " 
			   << subs << " counts but is synced to " << count[num] << ".";
		  _who_file.seekp(subspot);
		  write_subs(count[num]);
		  conferences.log(num); 
		}
	    }
	  if (count[num] == 0)
	    count[num] = -2;
	  else
	    count[num] = -1;
	}
      clerklog << outs << " out of sync.";
      sync_who = YES;
      return UNDECIDED;
      break;
    case 4:
      _who_file.clear();
      for (num = 1; num < _n_space ; num++)
	{
	  if (count[num] > 0)
	    {
	      clerklog << num << " is subscribed but is not in who.list!";
	      conferences.log(num);
	      cc = PROBLEM;
	    }
	  if (count[num] == -2)
	    {
	      whois(name, num);
	      clerklog << num << " = " << name << " is being dropped from who.list.";
	    }
	}
      outs = collapse(count, &ins);
      clerklog << "Dropped " << outs << " from who list and " << 
	ins << " remain.";
      delete[] count;
      state = 0;
      sync_who = NO;
      return cc;
    }
  return OK;  /* never happens */
}
//**************************************************
OKorNOT
WhoList::whois(char result[], unsigned long who_id)
{
  int i;
  char ch;
  unsigned long num;
  result[0] = '\0';
  if (who_id >= _n_space)
    return NOT_OK;
  if (_numbers[who_id] < 0)
    return NOT_OK;
  _who_file.seekg(_numbers[who_id]);
  if (!(_who_file >> ch) || ch != '<')
    {
      clerklog << "Whofile messed up looking for " << who_id << 
	".  Coming down.";
      exit(1);
    }
  i=-1;
  while (_who_file >> ch)
    {
      if (ch == '>')
	break;
      result[++i] = ch;
    }
  result[++i] = '\0';
  collect_subs(&_who_file);
  if ((!(_who_file >> num)) || num != who_id)
    {
      clerklog << "Whofile should have " << who_id << " but has "
	       << num << ". Coming down.";
      exit(1);
    }
  return OK;
}
//**************************************************
unsigned long
WhoList::whonum(char *name, YESorNO add, int subs = 0,
		unsigned long id = 0)
{
  streampos start;
  char found[200];
  int len;
  int i, namei;
  unsigned long answer;
  YESorNO eof = NO;
  len = strlen(name);
  found[0] = '\0';
  namei = TOINDEX(name[0]);
  start = (namei > (streampos)-1 ? _alpha_hash[namei] : (streampos)0);
  _who_file.seekg(start);
  while (eof == NO && _who_file >> found[0])
    {
      if (found[0] != '<')   /* looking for '<' */
	{
	  if (found[0] == '@')
	    {
	      if (_who_file.tellg() > _last_byte)
		{
		  eof = YES;
		  break;
		}
	    }
	  continue;
	}
      start = _who_file.tellg() - (streampos)1;
      for (i = 0; i < len; i++)
	{
	  if (!(_who_file >> found[i]))
	    {
	      eof = YES;
	      break;
	    }
	  if (found[i] == '>') /* stored name is shorter than name */
	    {
	      break;
	    }
	  if (TOSORT(found[i]) > TOSORT(name[i]))
	    {
	      if (add == YES)
		{
		  answer = insert_entry(name, start, subs, id);
		  return answer;
		}
	      else
		return 0;
	    }
	  if (TOSORT(found[i]) < TOSORT(name[i])) /* not a hit, look at next */
	    break;
	}
      if (i == len) /* hit */
	{
	  if (!(_who_file >> found[0]) || found[0] != '>')
	    // stored name is longer than name
	    {
	      answer = insert_entry(name, start);
	      return answer;
	    }
	  collect_subs(&_who_file);
	  _who_file >> answer;
	  return answer;
	}
    } 
  /* Came to EOF */
  _who_file.clear();
  if (add == NO)
    return 0;
  answer = insert_entry(name, _last_byte + 1);
  return answer;
}
//************************************************************
void
WhoList::write_subs(int subs)
{
  char substr[4];
  int i;
  sprintf(substr,"%03d", subs);
  for (i=0; i < 3; i++)
    {
      _who_file << substr[i];
    }
}
//************************************************************
#ifdef EDEBUG
void
WhoList::check(void)
{
  char ch;
  char name[200];
  int i;
  int a=0;
  int left_arrows = 0;
  streampos offset;
  unsigned long num;
  int sub;
  name[0] = '\0';
  if (!(edebug & WHODUMP))
    return;
  dlog << "Reading who file:";
  _who_file.seekg(ios::beg);
  i = 0;
  while (_who_file >> ch)
    {
      switch (ch)
	{
	case '<':
	  if (left_arrows == 0)
	    offset = _who_file.tellg() - (streampos)1;
	  if (++left_arrows > 1)
	    name[i++] = ch;
	  break;
	case '>':
	  left_arrows = 0;
	  sub = collect_subs(&_who_file);
	  _who_file >> num;
	  name[i] = '\0';
	  dlog << name << '\t' << num << '\t' << sub << '\t' << offset;
	  if (_numbers[num] != offset)
	    {
	      dlog << "\n\n_numbers[" << num << "] is " << _numbers[num]
		   << " but the offset is really " << offset;
	    }
	  if (_alpha_hash[TOINDEX(name[0])] > offset ||
	      _alpha_hash[TOINDEX(name[0]) + 1] < offset)
	    {
	      dlog << "\n\n_alpha_hash[TOINDEX(name[0])] is "
		   << _alpha_hash[TOINDEX(name[0])] 
		   << "\n next offset in hash is "
		   << _alpha_hash[TOINDEX(name[0] + 1)] 
		   << " but offset for " << name
		   << "is " << offset << "!";
	    }
	  i = 0;
	  name[i] = '\0';
	  break;
	default:
	  if (name[i] == '@' && name[i-1] == '@' && name[i-2] == '@')
	    {
	      a++;
	      break;
	    }
	  name[i++] = ch;
	  break;
	}
    }
  dlog << "\n " << a << " @'s \n";
  _who_file.clear();
  dlog << "alpha hash is: ";
  for (i = 0; i < 26 ; i++)
    {
      dlog << (char)(i + 'a') << " == " << _alpha_hash[i] ;
    }
  (*dlogger).flush();
}
#endif

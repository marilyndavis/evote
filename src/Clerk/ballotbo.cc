/* $Id: ballotbo.cc,v 1.4 2003/01/15 18:54:07 marilyndavis Exp $ */ 
/************************************************************
 *    ballotbo.cc  -- Functions that manage the ballots
************************************************************/
/*********************************************************
 *    Copyright (c) 1994...2015 Deliberate.com Patented.
 *    by Marilyn Davis
 **********************************************************/
#include "evotedef.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <iostream.h>
#include <fstream.h>
#include <strstream.h>
#include <sys/types.h>
#include <sys/msg.h>

#include "inq.h"
#include "ballot.h"
#include "conf.h"
#include "itemlist.h"
#include "voterlis.h"
#include "ballotbo.h"
#include "conflist.h"
#include "wholist.h"
GLOBAL_INCS
extern InQ inq;
extern ConfList conferences;
extern WhoList wholist;
#ifdef EDEBUG
extern int edebug;
ofstream * dumper;
#define dumplog *dumper
#define DEXT ".dump"
void vmem(char* msg, YESorNO free = NO, int size = 0);
#endif
//  ***************************************
//   BALLOTBOX member functions
// ***********************************************
// constructors
// *******************************************
BallotBox::BallotBox(Conf * p_conf, char *conf_name):
  _buffer(NULL), _no_of_participants(0L), _hash(this),
  _p_conf(p_conf), _pending_droppers(0),  _voters_to_drop(0)
#ifdef EDEBUG
    
, dump_up(NO)
#endif
{
  int i;
  YESorNO new_conf;
  _block_in_buffer = -1L;		
  
  strcpy(_name, output_dir);
  strcat(_name, conf_name);
  strcpy(_infname, _name);
  strcat(_name, DATA_EXT);
  strcat(_infname, BINF_EXT);
  i = DATA_PERM;
  do
    {
      _datafile.open(_name, ios::in|ios::out|ios::nocreate, DATA_PERM);
      if (!_datafile)
	{ 
	  /* there isn't an old one, try a new one */
	  _datafile.open(_name, ios::in|ios::out|ios::noreplace, 
			 DATA_PERM);            
	  if (!_datafile)  // Too many open files
	    {
	      if (conferences.adjourn_some(_p_conf) == 0)
		{
		  clerklog << "Can't open" << _name ;
		  clerklog << "Too many open files?  Coming down now.";
		  exit(0);
		}
	    }
	  else
	    {
	      new_conf = YES;
	    }
	}
      else
	{
	  new_conf = NO;
	}
    }
  while (!_datafile);
  
  if (new_conf == YES)
    {
      Ballot * p_empty;
      
      //      clerklog << "Making new data file for " << _name;
      /* Here we also initialize the variable read in 
	 // from info file because we assume there isn't
	 // one and we'll never call fetch_info which 
	 // usually does the job.*/
      _chunks = 1;
      _block_size = STARTING_BLOCK_SIZE;
      _rec_len = RECLEN;
      if (strncmp(conf_name, "petition", 8) == 0)
	{
/*	  _rec_len = sizeof(Header);
	  while (_rec_len % 8)
	    _rec_len += 2; */
	  _rec_len = 32;
	}
      _ballot_size = _rec_len * _chunks;
      _ballots_per_block = _block_size/_ballot_size;
      _no_in_overflow = 0;
      /* _rec_len must not be > RECLEN */
      p_empty = new Ballot(NO_UID);
      //  If it's not a new conf, _buffer is made in fetch_info()   
      _buffer = new char[_block_size];
#ifdef EDEBUG
      if (edebug & MEMS)
	{
	  char msg[100];
	  sprintf(msg,"Made _buffer, %d bytes.", _block_size);
	  vmem(msg, NO, _block_size);
	}
#endif  
      // make the overflow buffer
      _datafile.seekp((streampos)0);
      for (i = 0; i < _ballots_per_block; i++)
	{
	  _datafile.write((char*)(p_empty), _rec_len);
	}
      _datafile.flush();
      delete p_empty;
    }
#ifdef EDEBUG
  if (edebug & DUMP  || edebug & DUMP2)
    {
      start_dump();
      dumplog << "\n Opened data file. \n";
      /*		dump_file();  can't do this here, info has not been read in */
    }
#endif
}
BallotBox::~BallotBox(void)
{
#ifdef EDEBUG
  if (edebug & (DUMP | DUMP2) && dump_up == YES)
    {
      dumplog << "\nClosing data file: \n";
      dump_file();
      end_dump();
    }
#endif
  _datafile.close();
  if (_buffer != NULL)
    {
      delete[] _buffer;
#ifdef EDEBUG
      if (edebug & MEMS)
	{
	  vmem("Deleted _buffer.", NO, -_block_size);
	}
#endif
    }
}
//  ***********************************
//  add_ballot  - adds a vote ballot. Makes offset be the offset into 
//              the overflow buffer
OKorNOT 
BallotBox::add_ballot(Ballot *p_new_ballot, streampos& offset)
{
  char *here, *above;
  long priority = 1000l;
  unsigned long uid;
  
  uid = p_new_ballot->uid();		
  /*   Be sure there is room for the new Ballot.  If we are running out
       //  of space, send a message to reorder.  Make the priority of the
       //  message reflect the urgency of the situation. */
  switch (_ballots_per_block - _no_in_overflow)
    {
    case 0:
      if (reorder() != OK)
	{
	  clerklog << "Can't reorder " << _p_conf->name() 
	    << ". Can't take more voters.";
	  exit(0);
	}
      break;
    case 1:
      priority = PR_EMERGENCY1;
      break;
    case 3:
      priority = PR_EMERGENCY2;
      break;
    case 5:
      priority = PR_REORDER_CONF;
      break;
    default:
      break;
    }
  if (priority != 1000l)
    {	 
      char buf[50];
      sprintf(buf, FNE_REORDER_CONF, REORDER_CONF, (unsigned long)0, 
	      _p_conf->name());
      inq.send_myself(buf, priority);
#ifdef EDEBUG
      if (edebug & DUMP2)
	{
	  if (dump_up == NO)
	    start_dump();
	  
	  dumplog << "\nSending a priority " << priority 
	    << " reorder request: "
	      << (_ballots_per_block - _no_in_overflow) 
		<< " voter spots left.";
	}
#endif
    }
  read_in(_hash._blocks_in_hash);
  /* start at the beginning of the last ballot in the overflow  */
  here = _buffer + (_no_in_overflow - 1) * _ballot_size;
  while (here >= _buffer && *(unsigned long*)here > uid)
    {  /* while the uid coming in is still less than here's, 
	  move here up one.  */
      above = here + _ballot_size;
      memcpy(above, here, _ballot_size);
      here -= _ballot_size;
    }
  if (here >= _buffer && *(unsigned long*)here == uid 
      && !(((Ballot*)here)->action() & DROP))
    {
      clerklog << "Tried to add uid " << uid << " which is already there.";
      memcpy(p_new_ballot, (char *)here, _ballot_size);
      offset = (streampos)((char*)here - _buffer);
      return NOT_OK;
    }
  here += _ballot_size;
  memcpy(here, (char*)p_new_ballot, _ballot_size);	
  write_out(_hash._blocks_in_hash);
  /* move over any affected voters that are on line  */
  if (_p_conf->community().slide_in(uid, 
				    _hash._blocks_in_hash,
				    _ballot_size) != OK)
    {
      clerklog << "Impossible";
      return NOT_OK;
    }
  offset = (streampos)(here - _buffer);
  _no_in_overflow++;
#ifdef EDEBUG
  if (edebug & DUMP2)
    {
      if (dump_up == NO)
	start_dump();
      dumplog << "\nAdded " << uid << " _no_in_overflow is " 
	      << _no_in_overflow;
    }
#endif
  return OK;
}
// **************************************************
//  void BallotBox::blank_ballots(byte)
//     This is called by ItemList when a new plain item starts
//     on a new byte.  All the ballots in the system must be blanked
//     in that byte.  ItemList also calls VoterList::blank_voters(byte)
//     to take care of the on-line voters.
//
void 
BallotBox::blank_ballots(streampos byte, int num)
{  
  unsigned char blank = 0;
  int i;
  long block;
  short ballot;
  
  if (num == 0)
    return;
  for (block = 0; block <= _hash._blocks_in_hash; block++)
    {
      for (ballot = 0; ballot < _ballots_per_block; ballot++)
	{
	  for (i = 0; i < num; i++)
	    {
	      _datafile.seekg((streampos)
			      (block*_block_size + ballot*_ballot_size
			       + byte + i));
	      _datafile.put(blank);
	    }
	}
    }
  _block_in_buffer = -1;
}
//  **********************************************
//  OKorNOT check_ballots()
//    This is called at start up time if the conf
//    was online when the server came down -  if 
//    it came down without closing properly.
OKorNOT 
BallotBox::check_ballots(YESorNO growing)
{
  long i;
  short overflow = 0;
  long real_blocks_in_hash;
  short deletes = 0;
  short pending = 0;
  Ballot *pt;
  Ballot * blank_ballot;
  streampos end;
  
  _datafile.seekp((streampos)0, ios::end);
  end = _datafile.tellp();
  if (end % _block_size != 0 || growing == YES)
    {
      blank_ballot = (Ballot*)new char[_ballot_size];
#ifdef EDEBUG
      if (edebug & MEMS)
	{
	  char msg[100];
	  sprintf(msg,"Made blank ballot %d bytes.", 
		  _ballot_size);
	  vmem(msg, NO, _ballot_size);
	}
#endif  
      blank_ballot->start(_ballot_size);
      _p_conf->items().blank_ballot(blank_ballot);
      do
	{
	  _datafile.write((char*)blank_ballot, _ballot_size);	
	  end += _ballot_size;
	}
      while (end % _block_size != 0);
      
      delete[] (char*)blank_ballot;
#ifdef EDEBUG
      if (edebug & MEMS)
	{
	  vmem("Deleted blank ballot.", NO, -_ballot_size);
	}
#endif  
    }
  real_blocks_in_hash = end/_block_size -1;
  if (_hash._blocks_in_hash != real_blocks_in_hash)
    {
#ifdef EDEBUG
      if (edebug & MEMS)
	{
	  vmem("Remade hash table", NO, 
	       (real_blocks_in_hash - _hash._blocks_in_hash)
	       * sizeof(unsigned long));
	}
#endif  
      if (_hash._blocks_in_hash > 0)
	delete[] _hash._table;
      _hash._blocks_in_hash = real_blocks_in_hash;
      _hash._table = new unsigned long[_hash._blocks_in_hash];
      
    }
  for (i = 0; i < _hash._blocks_in_hash; i++)
    {
      read_in(i);
      pt = (Ballot*)_buffer;
      while ((char*)pt < _buffer + _block_size)
	{
	  if (pt->uid() == NO_UID)
	    break;
	  if (pt->action() & DROPPING)
	    pending++;
	  if (pt->action() & DROP)
	    deletes++;
	  pt = (Ballot*)((char *)(pt) + _ballot_size);
	}
      pt = (Ballot*)((char *)(pt) -_ballot_size);
      _hash._table[i] = pt->uid();
    }
  read_in(_hash._blocks_in_hash); // overflow buffer
  pt = (Ballot*)_buffer;
  while ((char*)pt < _buffer + _block_size)
    {
      if (pt->uid() == NO_UID)
	break;
      overflow++;
      if (pt->action() & DROP)
	{
#ifdef EDEBUG
      if (edebug & DUMP2)
	{
	  if (dump_up == NO)
	    start_dump();
	  
	  dumplog << "Voters_to_drop found in check_ballots "
	    << *(unsigned long*)pt;
	}
#endif
	  deletes++;
	}
      if (pt->action() & DROPPING)
	pending++;
      pt = (Ballot*)((char *)(pt) + _ballot_size);
    }
  _voters_to_drop = deletes;
#ifdef EDEBUG
      if (edebug & DUMP2)
	{
	  if (dump_up == NO)
	    start_dump();
	  
	  dumplog << "\nIn BallotBox::check_ballots, _voters_to_drop set to " 
	      << _voters_to_drop;
	}
#endif
  _no_in_overflow = overflow;
  _pending_droppers = pending;
  check_no_of_participants();
  return OK;
}
void 
BallotBox::check_no_of_participants(void)
{
  IT_CHOICE how = START;
  Ballot * p_ballot;
  unsigned long count = 0L;
  unsigned long pending_droppers_check = 0;
  unsigned long voters_to_drop_check = 0;
  ACTION a;
  #ifdef EDEBUG
  if (edebug & VOTERS)
    {
      dlog << "Voter::check_no_of_participants called with number = " 
	<< _no_of_participants;
      (*dlogger).flush();
    }
#endif
  while ((p_ballot = iterator(how, NULL, 0, YES)) 
	!= NULL)
    {
      how = NEXT;
      if (((a = p_ballot->action()) & READ_ONLY )
	 && p_ballot->_header._mod_date == 0l)
	continue;
      if (a & DROPPING)
	{
	  pending_droppers_check++;
	  continue;
	}
      if (a & DROP)
	{
	  voters_to_drop_check++;
	  continue;
	}
      count++;
#ifdef EDEBUG
  if (edebug & VOTERS)
    {
      dlog << "Voter::check_no_of_participants p_ballot uid = " <<
	*(unsigned long*)p_ballot << " action is " << a  << " \ncount = "
	  << count << "block_in is " << _block_in_buffer;
      (*dlogger).flush();
    }
#endif
    }
  if (count != _no_of_participants)
    {
      clerklog << "In " << _p_conf->name() << " number of participants "
	<< "\n  was changed from " << _no_of_participants << " to " 
	  << count << ".";
      _no_of_participants = count;
    }
  if (pending_droppers_check != _pending_droppers)
    {
      clerklog << "In " << _p_conf->name() << " pending droppers "
	<< "\n  was changed from " << _pending_droppers << " to " 
	  << pending_droppers_check << ".";
      _pending_droppers = pending_droppers_check;
    }
  if (voters_to_drop_check != _voters_to_drop)
    {
      clerklog << "In " << _p_conf->name() << " voters to drop "
	<< "\n  was changed from " << _voters_to_drop << " to " 
	  << voters_to_drop_check << ".";
      _voters_to_drop = voters_to_drop_check;
    }
}
OKorNOT 
BallotBox::check_order(void)
{
  YESorNO start_overflow = NO;
  OKorNOT all_ok = OK;
  Ballot* p_ballot;
  IT_CHOICE how = START;
  Ballot *last = NULL;
  
  while ((p_ballot = iterator(how)) != NULL)
    {
      how = NEXT;
      if (last != NULL
	  && *(unsigned long*)last >= *(unsigned long*)p_ballot)
	{
	  if (start_overflow == NO 
	      && _block_in_buffer == _hash._blocks_in_hash)
	    {
	      start_overflow = YES;
	    }
	  else
	    {
	      all_ok = NOT_OK;
	      clerklog << _p_conf->name() << " ballots out of order in block "
		<< _block_in_buffer << ". " << *(unsigned long*)last << " is before "
		  << *(unsigned long*)p_ballot;
	    }
	  last = p_ballot;
	}
    }
  check_no_of_participants();
  return all_ok;
}
// ********************************************
//  OKorNOT BallotBox::delete_ballot
//  Marks for
//  deletion, here and in who.list.
//
OKorNOT 
BallotBox::delete_ballot(Ballot* p_ballot, long block, 
				 YESorNO looping)
{
  unsigned long uid;  
  OKorNOT ret = OK;

  p_ballot->change_action(DROP);
  write_ballot(block, (streampos)((char*)p_ballot-_buffer), 
	       p_ballot);

  _voters_to_drop++;
  _pending_droppers--;

  uid = *(unsigned long *)p_ballot;
  if (p_ballot->is_mail() == YES)
    {
      char name[200];

      if (wholist.whois(name, *(unsigned long*)p_ballot) != OK)
	{
	  strcpy(name, "Not in who list");
	}
      clerklog << "Deleting ballot for "
	       << *(unsigned long*)p_ballot
	       << ": "
	       << name
	       << " in "
	       << _p_conf->name()
	       << ".";
	/* looping == YES if delete_ballot is called from a loop
	   in try_dropping_voters
	   and the who.list should wait for a signal to
	   collapse itself  --> force == MAYBE in wholist.drop call */
      switch (wholist.drop(*(unsigned long*)p_ballot, looping ? MAYBE : NO))
	{
	case OK:  // actually dropped the voter
	  break;
	case PROBLEM: // Discrepancy in count
	  break;
	case UNDECIDED:  // decremented count only
	  break;
	case NO_VOTER: // couldn't find the uid
	  break;
	default:
	  //impossible
	  break;
	}
    }
#ifdef EDEBUG
      if (edebug & DUMP2)
	{
	  if (dump_up == NO)
	    start_dump();
	  
	  dumplog << "\n\nIn BallotBox::delete_ballot on "
	    << uid
	      << " _voters_to_drop incremented to " 
		<< _voters_to_drop << "\n";
	}
#endif
  if (block != _hash._blocks_in_hash && 
     _hash._table[block] == uid)
    {
      char *pt = (char *)p_ballot;
      while (((Ballot*)(pt -= _ballot_size))->action() & DROP)
	{
	  if (pt <= _buffer)  // all this block is dropped!
	    {
	      if (block == _hash._blocks_in_hash -1) // last in hash, it's ok
		{
		  _hash._table[block] = (unsigned long)4294967295U;
		  break;
		}
	      else
		{
		  char buf[50];
		  ret = STOP;
		  sprintf(buf, FNE_REORDER_CONF, REORDER_CONF, 
			  (unsigned long)0, _p_conf->name());
		  /* if we're looping, we're coming from a reorder already
		     so a low priority is fine to give users a chance for
		     service */
		  inq.send_myself(buf, (looping ? PR_REORDER_CONF 
				     : PR_EMERGENCY1));
#ifdef EDEBUG
		  if (edebug & DUMP2)
		    {
		      if (dump_up == NO)
			start_dump();
		      
		      dumplog << "\nSending a first priority reorder request: block = "
			<< block << " all voters dropped.";
		      (*dumper).flush();
		    }
		  if (edebug & MESSAGES)
		    {
		      dlog << "\nSending a first priority reorder request: block = "
			<< block << " all voters dropped.";
		      (*dlogger).flush();
		    }
#endif		
		  break;
		}
	    }
	  if (_hash._table[block] !=  (unsigned long)4294967295U)
	    _hash._table[block] = *(unsigned long*)pt;	       
	  store_info();
	}
    }			
  return ret;
}
// ********************************************
//  double_ballot_size  - called by grow 
//                     be sure to reorder before calling this
//                     and to back up the files using renameX
OKorNOT 
BallotBox::double_ballot_size(void)
{
  streampos bytes = 0;
  char temp_name[PATH_LEN + 5];
  int i, j, k;
  char command[CONFLEN + 2 * PATH_LEN + EXT_LEN + 10];
  short new_ballots_per_block = _ballots_per_block/2;
  short new_ballot_size = _chunks * _rec_len * 2;
  long new_blocks_in_hash = _hash._blocks_in_hash * 2;
  Ballot * blank_ballot;
  char * pt;
  long new_hash_index = -1;  // index into the hash table
  
#ifdef EDEBUG
  if (edebug & MESSAGES)
    {
      time(&now);
      dlog << "\n" << _p_conf->name() << ": double_ballot_size called at "
	<< ctime(&now);
    }
#endif
  // First we calculate more carefully the new_blocks_in_hash
  read_in(_hash._blocks_in_hash -1);  // This will be the incomplete block
  pt = (_buffer + _block_size - _ballot_size);  // last ballot
  i = 0;
  while (*(unsigned long*)pt == NO_UID)
    {
      i++;
      pt -= _ballot_size;
    }
  // now i has the number of ballots left in the partial block
  if (i >= _ballots_per_block/2)
    new_blocks_in_hash--;
  strcpy(temp_name, output_dir);
  strcat(temp_name, "temp");
  ofstream new_file(temp_name, ios::out);
  while (!new_file)
    {
      if (conferences.adjourn_some(_p_conf) == 0)
	{
	  clerklog << "Can't open " << temp_name 
	    << "for resizing ballots for "<< _name;
	  clerklog << "Too many open files?  Coming down now.";
	  exit(0);
	}
      new_file.open(temp_name, ios::out);
    }
  if (_hash._blocks_in_hash > 0L)
    delete[] _hash._table;
  
  _hash._table = new unsigned long[new_blocks_in_hash];
#ifdef EDEBUG
  if (edebug & MEMS)
    {
      vmem("Remade hash table", NO, 
	   (new_blocks_in_hash - _hash._blocks_in_hash)
	   * sizeof(unsigned long));
    }
#endif  
  blank_ballot = (Ballot*)new char[new_ballot_size];
#ifdef EDEBUG
  if (edebug & MEMS)
    {
      vmem("Made blank ballot", NO, new_ballot_size);
    }
#endif  
  blank_ballot->start(new_ballot_size);
  _p_conf->items().blank_ballot(blank_ballot);
  for (i = 0; i < _hash._blocks_in_hash; i++)
    {
      read_in(i);
      for (j = 0; j < _ballots_per_block; j++)
	{
	  if (((Ballot*)(_buffer + j*_ballot_size))->uid() == NO_UID)
	    {  // Expected if we're on the last block
	      if (i == _hash._blocks_in_hash - 1)
		{
		  if (new_hash_index == -1 
		      || _hash._table[new_hash_index] 
		      != ((Ballot *)(_buffer 
				     + (j-1)*_ballot_size))->uid())
		    {
		      _hash._table[++new_hash_index] = ((Ballot *)(_buffer
								   +(j-1)*_ballot_size))->uid();
		    }
		  while (j % new_ballots_per_block != 0)
		    {
		      new_file.write((char*)blank_ballot, new_ballot_size);
		      j++;
		    }
		  break;
		}
	      else
		{
		  clerklog << "Double_ballot_size on " 
		    << _p_conf->name() << " : blank ballot in old block " 
		      << i << ", ballot "	<< j;
		}
	    }  // end if a NO_UID
	  new_file.write((char*)&_buffer[j*_ballot_size], _ballot_size);
	  for (k = _ballot_size; k < new_ballot_size; k++)
	    // add the new space
	    {
	      new_file.put(NOT_READ);
	    }
	  // fix up the hash table
	  if ((bytes += new_ballot_size) % _block_size == 0)
	    {
	      _hash._table[++new_hash_index] =
		((Ballot*)(_buffer + j*_ballot_size))->uid();
	    }
	}
    }
  // Make an empty overflow buffer.  Reorder is called before
  // this function.  Therefore, the overflow is empty.
  for (i = 0; i < new_ballots_per_block; i++)
    {
      new_file.write((char*)blank_ballot, new_ballot_size);	
    }
#ifdef EDEBUG
  if (edebug & MEMS)
    {
      vmem("Deleted blank ballot", NO, -new_ballot_size);
    }
#endif  
  delete[] (char*)blank_ballot;
  _chunks *= 2;
  _ballot_size = _chunks * _rec_len;
  _ballots_per_block = new_ballots_per_block;
  _hash._blocks_in_hash = new_blocks_in_hash;
  _block_in_buffer = -1L;
  _datafile.close();
  new_file.close();
  strcpy(command, "mv ");
  strcat(command, temp_name);
  strcat(command, " ");
  strcat(command, _name);
  if (system(command) == -1)
    {
      char msg[80];
      clerklog << "Unable to copy files in BallotBox::double_ballot_size().  \n   Perhaps there are too many processes running.";
      sprintf(msg, FNE_QUIT, QUIT, (unsigned long)0);
      inq.send_myself(msg, 1l);
      return NOT_OK;
    }
  _datafile.open(_name, ios::in|ios::out);
#ifdef EDEBUG
  if (edebug & DUMP2)
    {
      if (dump_up == NO)
	start_dump();
      dumplog << "\nAfter doubling size: ";
      dump_file();
    }
#endif
  store_info();
  return OK;
}
void 
BallotBox::drop_pending(Ballot *p_ballot)
{
  _pending_droppers++;
  p_ballot->change_action(DROPPING);
  _no_of_participants--;
#ifdef EDEBUG
  if (edebug & VOTERS)
    {
      dlog << "BallotBox::drop_pending, _no_of_participants -- --->"
	<< _no_of_participants;
      (*dlogger).flush();
    }
#endif
}
//  ****************************************************
//  fetch  fetches the voters vote ballot from disk or makes a
//         new one.  It allocates the space for it and hands back
//         a pointer to it.
//         It returns the block number and the offset where the
//         ballot is found in the appropriate reference variables.
//         Note that the pballot returned by find_ballot points into 
//         _buffer so the ballot gets copied from there to the
//         new space.
Ballot* 
BallotBox::fetch(unsigned long uid, long &block, 
		 streampos &offset, YESorNO force)
{
  Ballot *pballot;
  Ballot *preturn;
  
  // Make space here because the BallotBox is the only guy who
  // knows how big the ballot is.
  //  but for speed, call later - after we're sure we need it
  //  preturn = (Ballot *) new char[_ballot_size];
  
  // Note that if _ballot_size is > RECLEN, the alloted space
  // is really bigger than one Ballot.  However, the front of
  // the space is cast to a ballot.
  if ((pballot = find_ballot(uid, block)) != NULL)
    {
      if ((pballot->action() & DROPPING) && force == YES)
	{
	  _pending_droppers--;
	  _no_of_participants++;
	  pballot->start(_ballot_size, uid);
	}
      else
	if (pballot->action() & DROPPING)  /* force == NO */
	  pballot = NULL;
    }
  if (pballot != NULL)
    {
      offset = streampos((char*)pballot - _buffer);
      preturn = (Ballot *) new char[_ballot_size];
      memcpy(preturn, pballot, _ballot_size);
    }
  if (pballot == NULL)
    {  // need a new ballot for this new voter
      if (force == NO)
	{
	  return NULL;
	}
      offset = -1L;
      preturn = (Ballot *) new char[_ballot_size];
      preturn->start(_ballot_size, uid);
      // Also note that if the ballot size is big, this runs 
      // off the Ballot._vote array declared in the Ballot
      // class but runs onto the rest of the char array.
      _p_conf->items().blank_ballot(preturn);
      // Puts zeros in the bytes for plain items.
      add_ballot(preturn, offset);
      block = _hash._blocks_in_hash;
      _no_of_participants++;
#ifdef EDEBUG
  if (edebug & VOTERS)
    {
      dlog << "BallotBox::fetch, _no_of_participants++ --->"
	<< _no_of_participants;
      (*dlogger).flush();
    }
#endif
    }
#ifdef EDEBUG
  if (edebug & MEMS)
    {
      char msg[100];
      sprintf(msg,"Made a new ballot for uid = %lu, size = %d",
	      uid, _ballot_size);
      vmem(msg, NO, _ballot_size);
    }
#endif  
  return preturn;
}
// ********************************************
//   fetch_info  - reads the ballotbox variables in from the bnf
OKorNOT 
BallotBox::fetch_info(YESorNO growing)
{	
  fstream strm;
  long real_blocks_in_hash;
  streampos end;
  OKorNOT cc = OK;
  long extra_long[3];
  double extra_double[3];
  char extra_chars[6];
  
  strm.open(_infname, ios::in);
  while (!strm)
    {
      if (conferences.adjourn_some(_p_conf) == 0)
	{
	  clerklog << "Can't open info file for ballotbox in conf " 
	    << _name;
	  clerklog << "Too many open files?  Coming down now.";
	  exit(0);
	}
      strm.open(_infname, ios::in);
    }
  cc = _hash.fetch_info((istream&)strm);
  (istream&)strm >> _block_size >> _chunks >> _rec_len
    >> _no_in_overflow >> _voters_to_drop
     >> _pending_droppers >> _no_of_participants >> _ready_to_drop
       >> extra_long[0] >> extra_long[1] >> extra_double[0]
	 >> extra_double[1] >> extra_double[2] >> extra_chars ;
  strm.close();
  _buffer = new char[_block_size];
#ifdef EDEBUG
  if (edebug & MEMS)
    {
      char msg[100];
      sprintf(msg,"Made a new buffer, %d bytes", _block_size);
      vmem(msg, NO, _block_size);
    }
#endif  
  _ballot_size = _chunks * _rec_len;
  if (_ballot_size == 0)
    {
      clerklog << "Data for " << _p_conf->name() 
	<< " is corrupt.  Coming down now. ";
      conferences.store_all(_p_conf->name());
      exit(0);
    }
  _ballots_per_block = _block_size/_ballot_size;
  _datafile.seekp((streampos)0, ios::end);
  end = _datafile.tellp();
  real_blocks_in_hash = end/_block_size -1L;
  if (end % _block_size != 0 || cc == NOT_OK
     ||_hash._blocks_in_hash != real_blocks_in_hash)
    check_ballots(growing);
#ifdef EDEBUG
  if (edebug & DUMP)
    {
      if (dump_up == NO)
	start_dump();
      dumplog << "\nAfter fetch_info:\n";
      dump_file();
    }
#endif
  return OK;
}
// *************************************************
// find_ballot  returns a pointer to the voter's ballot
// 
Ballot* 
BallotBox::find_ballot(unsigned long uid, long& block)
{
  Ballot *p_ballot;
  
  read_in(block = _hash.which_block(uid));      // get the right _buffer
  if ((p_ballot = where_in_block(uid)) == NULL    // if it's not in the _buffer 
       && block < _hash._blocks_in_hash)      // and we don't already have
    {		                        // the overflow buffer
      read_in(block = _hash._blocks_in_hash);  // get the overflow _buffer
      p_ballot = where_in_block(uid);
    }
  return p_ballot;
}
OKorNOT 
BallotBox::grow(YESorNO force)
{
#ifdef EDEBUG
  if (edebug & DUMP2)
    {
      if (dump_up == NO)
	start_dump();
      dumplog << "\nGrow called";
    }
  if (edebug & MESSAGES)
    {
      dlog << "\nGrow called";
    }
#endif
  if (_ballot_size/GROW_TRIGGER < _p_conf->item_bytes_left() 
      && force == NO)
    return OK;
  if (reorder(YES, NO) == OK)
    {
      if (_ballots_per_block <= 8)
	{
#ifdef EDEBUG
	  if (edebug & DUMP2)
	    {
	      if (dump_up == NO)
		start_dump();
	      dumplog << "Doubling block size.  Was = " << _block_size << ".  Becoming = "
		<< 2*_block_size ;
	    }
#endif
	  _block_size *= 2;
	  delete[] _buffer;
#ifdef EDEBUG
	  if (edebug & MEMS)
	    {
	      vmem("Deleted _buffer.", NO, -_block_size);
	    }
#endif  
	  store_info();
	  fetch_info(YES);  // YES = growing
	}
      if (double_ballot_size() == OK)
	{
	  if (_p_conf->community().refetch_all() == OK)
	    {
	      _hash.check();
	      store_info();
	      return OK;
	    }
	}
    }
  return NOT_OK;		
}
//  ***************************************
//  iterator(IT_CHOICE choice, long * p_block = NULL, unsigned long uid)
//          If choice is START, hands back the ballot for uid or the
//          first ballot after uid.  If choice is NEXT, gets the next
//          ballot.
Ballot *
BallotBox::iterator(IT_CHOICE choice, long * p_block,
			    unsigned long uid, YESorNO all)
{
  static long block_in;
  static Ballot* p_ballot;
  
  if (choice == START)
    {
      block_in = _hash.which_block(uid);
      read_in(block_in);
      p_ballot = where_in_block(uid, YES);
      // That YES makes it pass back the next uid if that uid
      // doesn't exit.
    }
  if (choice == NEXT)
    {
      if (_block_in_buffer != block_in)
	read_in(block_in);
      do
	{
	  p_ballot = (Ballot*)((char *)(p_ballot) + _ballot_size);

	  if ((int)p_ballot >= (int)_buffer + (int)_block_size
	      || *(unsigned long*)p_ballot == NO_UID)
	    {
	      if (++block_in > _hash._blocks_in_hash)
		return NULL;
	      read_in(block_in);
	      p_ballot = (Ballot*)_buffer;
	    }
	  if (all == YES && p_ballot->action() & DROP
	     && *(unsigned long*)p_ballot != NO_UID)
	    break;
	}
      while (*(unsigned long*)p_ballot == NO_UID 
	     || p_ballot->action() & DROP);
    }
  if (p_ballot != NULL && !(p_ballot->action() & DROPPING))
    {
      Ballot* on_line_ballot 
	= _p_conf->community().get_ballot(p_ballot->uid());
      if (on_line_ballot != NULL)
	{
	  if (p_block != NULL)
	    *p_block = -1;
	  return on_line_ballot;
	}
    }
  if (p_block != NULL)
    *p_block = block_in;
  return p_ballot;
}
void 
BallotBox::read_in(long block)
{
  if (block == _block_in_buffer)
    return;
  
  _datafile.seekg((streampos)block*_block_size);
  _datafile.read(_buffer, _block_size);
#ifdef EDEBUG
  if (edebug & DUMP)
    {
      if (dump_up == NO)
	start_dump();
      dumplog << "\nJust after read: \n";
      _p_conf->items().show_packing((Ballot*)_buffer, dumplog);
    }
#endif
  _block_in_buffer = block;
  
}
//  ***************************************
//  reorder  - reorders the datafile so that the overflow buffer is
//             empty, the deleted voters are out of here, and the
//             deleted items are taken out of the ballots. 
//             It backs up the old datafile to Xconf.data
//
OKorNOT 
BallotBox::reorder(YESorNO force, YESorNO refetch)
{
  streampos *dead_bytes;
  char news[_block_size];
  Ballot* p_new;
  Ballot* p_old;
  char temp_name[PATH_LEN + 5];
  char command[CONFLEN + 2 * PATH_LEN + EXT_LEN + 10];
  long i;
  int count;
  short j;
  streampos bytes_in = 0L;
  short deletes = 0;
  unsigned long last_valid_uid = 0L;
  YESorNO last_uid_changed = NO;
  long space;
  long new_hash_index = -1L;
  Ballot* blank_ballot;
  char footnote[80];
  static int counter = 0;

  ++counter;
#ifdef EDEBUG
  if (edebug & DUMP2)
    {
      if (dump_up == NO)
	start_dump();
      time(&now);
      dumplog << "\nReorder called: " << ctime(&now) << "\n\tdropping "
	<< _voters_to_drop << " voters.";
    }		
  if (edebug & MESSAGES)
    {
      time(&now);
      dlog << "\nReorder called: " << ctime(&now);
      dlog << _hash._blocks_in_hash << " blocks in hash. "
	<< _no_in_overflow << " in overflow buffer.";
      dlog << "  ballot size = " << _ballot_size << ".  chunks = "
	<< _chunks << ". \nballots per block = " << _ballots_per_block
	  << ".  voters to drop = " << _voters_to_drop 
	    << ". pending droppers = " << _pending_droppers;
      dlog << "\n item_bytes_left = " << _p_conf->item_bytes_left()
	<< ".  items_to_drop = " << (i=_p_conf->items_to_drop());
    }
#endif			
  try_dropping_voters(0);
  if (_voters_to_drop > 0)
    {
      clerklog << "Dropping " << _voters_to_drop 
	<< " ballots in " << _p_conf->name();
    }
  if (force == NO 
     && (_ballots_per_block/GROW_TRIGGER 
	 - _p_conf->voter_spots_left() <= 0)
     && _p_conf->voters_to_drop() == 0 
     && _p_conf->items_to_drop() == 0)
    return OK;
#ifdef EDEBUG
  if (edebug & DUMP2)
    {
      if (dump_up == NO)
	start_dump();
      time(&now);
      dumplog << "\nStart of reorder: " << ctime(&now);
      dump_file();
    }
#endif			
  //		void write_reorder_ballot(Ballot*& pballot, 
  //                          streampos& bytes_in,
  //				 long & new_hash_index,
  //                 streampos* dead_bytes, ofstream &strm);
  // write_reorder_ballot is only called from this function.  It is
  // defined below.
  // Note in particular that it increments the ballot pointer!
  // backup does a store_all
  sprintf(footnote, "Reordering.");	
  _p_conf->backup(footnote);
  strcpy(temp_name, output_dir);
  strcat(temp_name, "temp");
  ofstream new_file(temp_name, ios::out, DATA_PERM);
  while (!new_file)
    {
      if (conferences.adjourn_some(_p_conf) == 0)
	{
	  clerklog << "Can't open " << temp_name 
	    << "for reordering " << _name;
	  clerklog << "Too many open files?  Coming down now.";
	  exit(0);
	}
      new_file.open(temp_name, ios::out, DATA_PERM);
    }
  // Do we need a bigger hash?
  if (_hash._blocks_in_hash > 0L)		
    {
      read_in(_hash._blocks_in_hash -1L);  // last block in hash
      p_old = (Ballot*)_buffer;
      count = 0;
      while ((char*)p_old < (_buffer + _block_size) && p_old->uid() != NO_UID)
	{
	  count++;
	  p_old = (Ballot*)((char *)(p_old) + _ballot_size);
	}
      // now count == the number of ballots in the last hashed block
      space = _ballots_per_block - count - _no_in_overflow + _voters_to_drop;
    }
  else
    space =  -_no_in_overflow
      + _voters_to_drop;  // first reorder
#ifdef EDEBUG
  if (edebug & DUMP2)
    {
      if (dump_up == NO)
	start_dump();
      dumplog << "\nSpace = " << space;
    }
#endif
  if (space < 0)
    {
      if (_hash._blocks_in_hash > 0L)
	delete[] _hash._table;
      _hash._table = new unsigned long[_hash._blocks_in_hash + 1L];
#ifdef EDEBUG
      if (edebug & MEMS)
	{
	  char msg[100];
	  sprintf(msg,"Made a new hash table %ld bytes.", 
		  (long)sizeof(unsigned long) * _hash._blocks_in_hash);
	  vmem(msg, NO, sizeof(unsigned long));
	}
#endif  
    }
  dead_bytes = new streampos[_ballot_size];
#ifdef EDEBUG
  if (edebug & MEMS)
    {
      vmem("Made dead_bytes array", NO, sizeof(streampos)*_ballot_size);
    }
#endif  
  for (i = 0; i < _ballot_size; i++)
    {
      dead_bytes[i]=(streampos)0;
    }
  // the call to kill_old_items finds all the deleted items
  // and actually deletes them.  It recalculates the ballot
  // to item mapping and reports the ballot bytes that need
  // skipping in dead_bytes.
  if (_p_conf->items().kill_old_items(dead_bytes) == NOT_OK)
    {
      delete[] dead_bytes;
      delete[] _hash._table;
      _hash._table = new unsigned long[_hash._blocks_in_hash];
      _hash.check();  
#ifdef EDEBUG
      if (edebug & MEMS)
	{
	  vmem("Remade hash table same size and deleted dead_bytes", NO,  
	       -sizeof(streampos)*_ballot_size);
	}
#endif  
      return NOT_OK;
    }
  // read in overflow block
  read_in(_hash._blocks_in_hash);  
  memcpy(news, _buffer, _block_size);   // make a copy
  p_new = (Ballot *)news;
  // Now we read in each block and write out the collapsed
  // ballot for non-DELETED voters, splicing in ballots
  // from the overflow buffer so that they're in order
  for (i = 0L; i < _hash._blocks_in_hash; i++)
    {
      read_in(i);
      for (j = 0; j < _ballots_per_block; j ++)
	{
	  p_old = (Ballot*)(_buffer + j*_ballot_size);
	  
	  if (p_old->action() & DROP)
	    {
	      deletes++;
	      _voters_to_drop--;
#ifdef EDEBUG
      if (edebug & DUMP2)
	{
	  if (dump_up == NO)
	    start_dump();
	  
	  dumplog << "\nIn reorder, _voters_to_drop decremented to "
	    << _voters_to_drop << "\n\t p_old deleted on "
	      << *(unsigned long*)p_old ;
	}
#endif
	      continue;
	    }
	  if (p_old->uid() == NO_UID)
	    break;
	  while ((char*)p_new < news + _block_size
		&& p_new->uid() != NO_UID 
		&& p_new->uid() < p_old->uid())
	    // pnew or pold is incremented in write_reorder_ballot
	    // it also figures the hash and keeps track of 
	    // new_hash_index
	    {
	      if (((Ballot*)p_new)->action() & DROP)
		{
		  deletes++;
		  _voters_to_drop--;
#ifdef EDEBUG
		  if (edebug & DUMP2)
		    {
		      if (dump_up == NO)
			start_dump();
		      
		      dumplog << "In reorder, _voters_to_drop decremented to "
			      << _voters_to_drop << "\n\t p_new deleted on "
			      << *(unsigned long*)p_new;
		    }
#endif
		  p_new = (Ballot*)((char *)(p_new) + _ballot_size);
		}
	      else
		{ // must collectt last_valid_uid before
		  // call to write_reorder_ballot!!
		  
		  last_valid_uid = p_new->uid();
		  last_uid_changed = YES;
		  write_reorder_ballot(p_new, bytes_in, new_hash_index, 
				       dead_bytes, new_file);
		}
	    }
	  last_valid_uid = p_old->uid();
	  last_uid_changed = YES;
	  write_reorder_ballot(p_old, bytes_in, new_hash_index, dead_bytes, new_file);
	}
    }
  // finish writing out the overflow buffer
  while (p_new->uid() != NO_UID && (char*)p_new < news + _block_size )
    {
      if (((Ballot*)p_new)->action() & DROP)
	{
	  deletes++;
	  _voters_to_drop--;
#ifdef EDEBUG
	  if (edebug & DUMP2)
	    {
	      if (dump_up == NO)
		start_dump();
	      
	      dumplog << "In reorder, _voters_to_drop decremented to "
		      << _voters_to_drop << "\n\tp_new deleted from overflow.  "
		      << *(unsigned long*)p_new ;
	    }
#endif
	  p_new = (Ballot*)((char *)(p_new) + _ballot_size);
	}
      else
	{
	  last_valid_uid = p_new->uid();
	  last_uid_changed = YES;
	  write_reorder_ballot(p_new, bytes_in, new_hash_index, 
			       dead_bytes, new_file);
	}
    }
  // It could be that the last of the users came at a block boundary.
  // If not, put her in the hash and fill up the block with nothing.
  blank_ballot = (Ballot*)new char[_ballot_size];
#ifdef EDEBUG
  if (edebug & MEMS)
    {
      char msg[100];
      sprintf(msg,"Made a blank ballot, size = %d",
	      _ballot_size);
      vmem(msg, NO, _ballot_size);
    }
#endif  
  blank_ballot->start(_ballot_size);
  _p_conf->items().blank_ballot(blank_ballot);
  if (last_uid_changed == YES  // otherwise there's no one left
     && (new_hash_index == -1L   
	 || _hash._table[new_hash_index] != last_valid_uid))
    {
      _hash._table[++new_hash_index] = last_valid_uid;
      while ((bytes_in % _block_size) != 0)
	{
	  new_file.write((char*)blank_ballot, _ballot_size);
	  bytes_in += _ballot_size;
	}
    }	
  // fill in the overflow buffer
  for (count = 0; count < _ballots_per_block; count++)
    {
      new_file.write((char*)blank_ballot, _ballot_size);
    }
  delete[] (char*)blank_ballot;
#ifdef EDEBUG
  if (edebug & MEMS)
    {
      vmem("Deleted blank ballot.", NO, -_ballot_size);
    }
#endif  
  if (_hash._blocks_in_hash > new_hash_index + 1L )
    {  // shrunk a block or more -- shrink _hash._table
      unsigned long *holder = NULL;
      
      if (new_hash_index + 1L > 0L) // otherwise no one's left
	{
	  holder = new unsigned long[new_hash_index + 1L];
	  for (i = 0L; i < new_hash_index + 1L ; i++)
	    {
	      holder[i] = _hash._table[i];
	    }
	}
      delete[] _hash._table;
      _hash._table = holder;
#ifdef EDEBUG
      if (edebug & MEMS)
	{
	  char msg[100];
	  sprintf(msg,"Made a new hash table for %ld entries, size = %ld",
		  new_hash_index + 1, 
		  (long)sizeof(unsigned long) * (new_hash_index + 1));
          vmem(msg, NO, 
               sizeof(unsigned long)*((new_hash_index + 1)
				       - _hash._blocks_in_hash));
	}
#endif  
    }
  if (_voters_to_drop != 0)
    {
      clerklog << "Error in reorder in "
	       << _p_conf->name();
    }
  delete[] dead_bytes;
#ifdef EDEBUG
  if (edebug & MEMS)
    {
      vmem("Deleted dead_bytes array", NO,
	   -sizeof(streampos)*_ballot_size);
    }
#endif  
  _datafile.close();
  new_file.close();
  strcpy(command,"mv ");
  strcat(command, temp_name);
  strcat(command, " ");
  strcat(command, _name);
  if (system(command) == -1)
    {
      clerklog << "Unable to copy files in BallotBox::reorder().  \n   Perhaps there are too many processes running.";
      return NOT_OK;
    }
  _datafile.open(_name, ios::in|ios::out|ios::nocreate, DATA_PERM);
  while (!_datafile)
    {
      if (conferences.adjourn_some(_p_conf) == 0)
	{
	  clerklog << "Reorder: " << _p_conf->name() 
	    << " can't move files.";
	  clerklog << "Too many open files?  Coming down now.";
	  exit(0);
	}
      _datafile.open(_name, ios::in|ios::out|ios::nocreate, DATA_PERM);
    }
  _no_in_overflow = 0;
  _block_in_buffer = -1L;
  _hash._blocks_in_hash = new_hash_index + 1L;
  _voters_to_drop = 0;
#ifdef EDEBUG
  if (edebug & DUMP2)
    {
      if (dump_up == NO)
	start_dump();
      dumplog << "\nVoters_to_drop reset to 0.";
      dumplog << "\nEnd of reorder:";
      dump_file();
    }
#endif
  if (refetch == YES)
    {
      return _p_conf->community().refetch_all();
    }
  return OK;
}
//  *************************************
//  store  writes the voter's ballot to disk.
OKorNOT 
BallotBox::store(Ballot *pballot, long &block, streampos &offset)
{
#ifdef EDEBUG
  static int times = 0;

  if (edebug & DUMP)
    {
      if (dump_up == NO)
	start_dump();
      if (++times > 200000)
	exit(20);
      dumplog << "\nJust before write to file: \n";
      _p_conf->items().show_packing(pballot, dumplog);
    }
#endif
  if (pballot == NULL)
    return OK;
  if (!(pballot->_header._action & READ_ONLY))
    time(&(pballot->_header._mod_date));
  _datafile.seekp((streampos)(block*_block_size + offset));
  _datafile.write((char *)pballot, _ballot_size);
  if (block == _block_in_buffer)
    {
      memcpy(_buffer + offset, (char*)pballot, _ballot_size);
    }
  return OK;
}
//  **********************************************
//  store_info  writes the variables onto the stream
OKorNOT 
BallotBox::store_info(void)
{
  fstream strm;
  long extra_long[3] = {0L, 0L, 0L};
  double extra_double[3] = {0., 0., 0.};
  char extra_chars[6]="extra";
  static int counter = 0;

  counter++;
  strm.open(_infname, ios::out);
  while (!strm)
    {
      if (conferences.adjourn_some(_p_conf) == 0)
	{
	  clerklog << "Can't open info file for ballotbox in conf " 
	    << _name;
	  clerklog << "Too many open files?  Coming down now.";
	  exit(0);
	}
      strm.open(_infname, ios::out);
    }
  if (_hash.store_info((ostream&)strm) != OK)
    {
      strm.close();
      return NOT_OK;
    }
  (ostream&)strm << _block_size << ' ' << _chunks << ' ' 
    << _rec_len << ' '
    << _no_in_overflow << ' ' 
      << _voters_to_drop << ' '<< _pending_droppers
      << ' ' << _no_of_participants << ' '
	<< _ready_to_drop << ' ' 
	  << extra_long[0] << ' ' <<  extra_long[1] << ' '
	    << extra_double[0] << ' ' << extra_double[1] << ' '
	      << extra_double[2] << ' ' << extra_chars ;
  strm.close();
  return OK;
}
// **********************************
//   int BallotBox::try_dropping_voters(time_t early_item_closing)
//       This looks for DROPPING voters and tries to make
//       them DROPs.  
//       This is called when items are deleted and early_item_closing
//       is the closing time of the dropped item that closed
//       the earliest.
//       If early_item_closing is 0, this is called from reorder()
//       and the rest of the reorder will happen so the reorder
//       message will be low priority to give users a chance to get
//       into the Clerk.
//       
extern YESorNO sync_who;
int 
BallotBox::try_dropping_voters(time_t early_item_closing)
{
  IT_CHOICE how = START;
  Ballot * p_ballot;
  int worth_trying = (_pending_droppers > _ready_to_drop ? _pending_droppers : _ready_to_drop);
  int dropped = 0;
  YESorNO stop = NO;
  
  if (worth_trying == 0)
    return 0;
  while ((p_ballot = iterator(how)) != NULL)
    {
      how = NEXT;
      
      if (!(p_ballot->action() & DROPPING))
	continue;
      if (early_item_closing == (time_t)0
	 || p_ballot->mod_date() >= early_item_closing)
	{
	  if (_p_conf->items().isalive(p_ballot) == NO)
	    {
	      if (stop)
		{  /* count up the rest that are ready 
		      but can't happen now */
		  _ready_to_drop++;
		  continue;
		}
	      if (early_item_closing == (time_t)0) 
		/* happens when eVote check conf is called 
		   or on the first groom for a petition list
		   where a petition has been dropped */
		{
		  clerklog << "  Dropping " << p_ballot->uid()
		    << ".";
		}

	      /* if early_item_closing == 0 we are called from 
		 reorder so the reorder will happen when this
		 function returns */
	      switch (delete_ballot(p_ballot, _block_in_buffer, 
				   early_item_closing == 0 ? YES: NO))
		{
		case STOP:  /* max dropped, needs a reorder */
		  stop = YES;
		  _ready_to_drop = 0;
		case OK:
		  _ready_to_drop 
		    = (_ready_to_drop == 0 ? 0 : _ready_to_drop - 1);
		  dropped++;
		  break;
		default:
		  break;
		}
#ifdef EDEBUG
	      if (edebug & DUMP2)
		{
		  if (dump_up == NO)
		    start_dump();
	  
		  dumplog << "In try_dropping_voter, dropping->drop on "
		    << *(unsigned long*)p_ballot 
		      << ". _voters_to_drop incremented to "
			<< _voters_to_drop;
		}
#endif
	    }
	}
      if (--worth_trying <= 0)
	break;
    }
  if (dropped)
    sync_who = YES;
  else
    _ready_to_drop = 0;
  return dropped;
}
// **********************************
//  where_in_block  returns a pointer to the asked for voter ballots if it is
//                 in the _buffer that is in memory.
//  The before_flag, if set to YES, indicates that the caller wants to
//  know where it belongs in the block, even if it's not there.
//  before_flag defaults to NO
Ballot * 
BallotBox::where_in_block(unsigned long uid, YESorNO pass_something)
{
  unsigned long bot_uid, mid_uid, top_uid;  // for edebug, take out one day
  char *pbot = _buffer;
  char *ptop = _buffer + (_ballots_per_block - 1) * _ballot_size;
  char *pt;
  
  // correct ptop if it's the overflow block
  if (_block_in_buffer == _hash._blocks_in_hash)
    {		
      ptop = _buffer + (_no_in_overflow -1)*_ballot_size;
      if (_no_in_overflow == 0)
	return NULL;
      while (((Ballot*)ptop)->action() & DROP)
	{
	  if ((ptop -= _ballot_size) < _buffer)
	    return NULL;
	}
    }
  // and if it's the last hashed block
  else if (_block_in_buffer == _hash._blocks_in_hash -1L)
    {
      while (*(unsigned long*)ptop == NO_UID)
	ptop -= _ballot_size;
      while (((Ballot*)ptop)->action() & DROP)
	{
	  if ((ptop -= _ballot_size) < _buffer)
	    return NULL;
	}
    }
  // This is to catch the other blocks
  // They are guaranteed to have some valid voter in them by the
  // action in drop_voter().
  while (((Ballot*)ptop)->action() & DROP)
    {
      ptop -= _ballot_size;
    }
  while (((Ballot*)pbot)->action() & DROP)
    {
      pbot += _ballot_size;
    }
  if ((top_uid= ((Ballot *)ptop)->uid()) == uid)
    return (Ballot *)ptop;
  if ((bot_uid = ((Ballot *)pbot)->uid()) == uid)
    return (Ballot *)pbot;
  do
    {
      // set pt to the start of a ballot in the middle
      pt = pbot + (long)((ptop-pbot)/(2* _ballot_size)) * _ballot_size;
      if ((mid_uid = ((Ballot *)pt)->uid()) < uid)
	{
	  pbot = pt;
	  bot_uid = ((Ballot *)pbot)->uid();
	}
      else if (mid_uid > uid) 
	{
	  ptop = pt;
	  top_uid = ((Ballot *)ptop)->uid();
	}
      else  // found the ballot
	{
	  break;
	}
    }
  while (ptop - pbot > _ballot_size); // while they are apart by 
  // more than one ballot 
  if (mid_uid != uid)
    pt = NULL;
  else if (((Ballot *)pt)->action() & DROP)
    {  // check for another entry in the overflow
      if (_block_in_buffer == _hash._blocks_in_hash)
	{
	  while ((pt + _ballot_size) < _buffer + _block_size
		&& ((Ballot*)(pt += _ballot_size))->uid() == uid)
	    {
	      if (!(((Ballot*)pt)->action() & DROP))  // found it
		{
		  return (Ballot*)pt;
		}
	    }
	  ptop = pt;  // next one after failure
	  pt = NULL;  //flag that it's not found
	}
      else  // not the overflow buffer
	{  // perhaps pt is more than one from ptop, fix it
	  ptop = pt + _ballot_size;  // next one
	  pt = NULL;  // flag
	}
    }
  if (pt == NULL && pass_something == YES)
    {
      if (uid < *(unsigned long*)pbot)  // uid == 0 ? - for iterator start
	pt = pbot;
      else
	{
	  pt = ptop;
	  while (((Ballot*)pt)->action() & DROP)
	    {
	      pt += _ballot_size;
	      if ((pt > _buffer + _block_size)
		  || *(unsigned long*)pt == NO_UID)
		return NULL;
	    }
	}
    }
  return (Ballot*)pt;
}
//  *****************************************
//  void write_ballot
//  writes one ballot at the specified block and offset
void 
BallotBox::write_ballot(long block, streampos offset, Ballot * p_ballot)
{
  _datafile.seekp((streampos)(block*_block_size + offset ));
  _datafile.write((char*)p_ballot, _ballot_size);
}
// ********************************
//  write_out  writes the buffer in memory out to the file into the
//             specified block
void 
BallotBox::write_out(long block)
{
  if (block == -1L)
    block = _block_in_buffer;
  _datafile.seekp((streampos)block*_block_size);
  _datafile.write(_buffer, _block_size);
}
// *****************************
// write_reorder_ballot  -  this is only called from reorder() above and so
//               is kept with it.	                         
//       Uses the dead_byte array to collapse the vote ballot to 
//       get rid of deleted items.  The collapsed ballot is
//       written to the ofstream.
//       ItemList::kill_old_items made the dead_byte array.
//       kill_old_items and this function are called by reorder();
//			 bytes_in keeps track of the total bytes in the file
void 
BallotBox::write_reorder_ballot(Ballot *& p_ballot, streampos & bytes_in, 
				long& new_hash_index,	
				streampos * dead_bytes, ofstream & new_file)
{
  long db_index = 0L;
  streampos bytes_here = (streampos)0;
  char *p_ch = (char*)p_ballot;
  int k;
  
  if (p_ballot->action() & DROP)
    {
      p_ch += 16;
      p_ballot = (Ballot *)p_ch;
      return;
    }
  new_file.write(p_ch, sizeof(Header));
  bytes_here = sizeof(Header);
  bytes_in += sizeof(Header);
  p_ch += sizeof(Header);
  for (k = sizeof(Header); k < _ballot_size; k++, p_ch++)
    {
      if (dead_bytes[db_index] == (streampos)k)
	{
	  db_index++;
	  continue;
	}
      new_file << *p_ch;
      bytes_in++;
      bytes_here++;
    }
  while (bytes_here < _ballot_size)
    {
      new_file << (unsigned char)NOT_READ;
      bytes_in++;
      bytes_here++;
    }
  if ((bytes_in % _block_size) == 0)
    {
      _hash._table[++new_hash_index] = p_ballot->uid();
    }
  p_ch = (char*)p_ballot + _ballot_size;
  p_ballot = (Ballot *)p_ch;
} 
#ifdef EDEBUG
YESorNO 
BallotBox::dump_ballot(Ballot *p_ballot, YESorNO even_empty)
{
  unsigned int i;
  char line[81];
  char name[200];
  char *lp;
  OKorNOT cc;
  int count = 0;
  if (dump_up == NO)
    start_dump();
  if ((cc = wholist.whois(name, *(unsigned long*)p_ballot)) != OK
     && !(p_ballot->action() & DROP) && !even_empty)
    return NO;
  dumplog << "\n\n" << (cc == OK ? name : "no who" ) << " ";
  dumplog << *(unsigned long*)p_ballot << ' ' << ctime(&(p_ballot->_header._mod_date)) << p_ballot->action() ;
  dumplog << " -> ";
  if (p_ballot->action() & EVERYTHING)
    dumplog << "  EVERYTHING";
  /*  if (p_ballot->action() & LOCK)
      dumplog << "  LOCK"; */
  if (p_ballot->action() & LOCAL)
    dumplog << "  LOCAL";
  if (p_ballot->action() & DROP)
    dumplog << "  DROP";
  if (p_ballot->action() & DROPPING)
    dumplog << "  DROPPING";
  if (p_ballot->action() & VACATION)
    dumplog << "  VACATION";
  if (p_ballot->action() & VOTE_ONLY)
    dumplog << "  VOTE_ONLY";
  if (p_ballot->action() & READ_ONLY)
    dumplog << "  READ_ONLY";
  if (p_ballot->action() & SIGNER)
    dumplog << "  SIGNER";
  if (p_ballot->action() & UNSET)
    dumplog << "  UNSET";
  lp = line;		
  for (i = 0, count = 0; i < (_ballot_size - sizeof(Header)); i++)
    {
      if ((count += 4)  >=  80 )
	{
	  dumplog << line << '\n';
	  lp = line;
	  count = 4;
	}
      (void)sprintf(lp,"%4d", p_ballot->_vote[i]);
      lp += 4;
    }
  dumplog << line ;
  (*dumper).flush();
  return YES;
}
void 
BallotBox::dump_buffer(void)
{
  int i;
  Ballot *pb;
  int skips = 0;
  YESorNO show_even_blanks = NO;
  
  if (dump_up == NO)
    start_dump();
  for (i = 0; i < _ballots_per_block; i++)
    {
      pb = (Ballot *)(_buffer + i*_ballot_size);
      if (!dump_ballot(pb, show_even_blanks))
	{
	  skips++;
	}
      else if (skips) /* ballot after blank ballots! */
	{/* so we start over and show blanks */
	  i = -1;
	  show_even_blanks = YES;
	  dumplog << "There's a good ballot after "
		  << skips << " blanks, starting over \n\n";
	  skips = 0;
	}
    }
  dumplog << "\nSkipped " << skips << " blank ballots";
  (*dumper).flush();
}
// ***************************************
//  void BallotBox::dump_file()
//   dumps the whole file
void 
BallotBox::dump_file(void)
{
  long i;
  time(&now);
  
  if (dump_up == NO)
    start_dump();
  dumplog << "\n" << _p_conf->name() << " file dump at " << ctime(&now);
  dumplog << _hash._blocks_in_hash << " blocks in hash. "
    << _no_in_overflow << " in overflow buffer.";
  dumplog << "\n  block size = " << _block_size 
    << "  ballot size = " << _ballot_size << ".  chunks = "
    << _chunks << ". \n  ballots per block = " << _ballots_per_block
      << ".  voters to drop = " << _voters_to_drop;
  dumplog << "\n item_bytes_left = " << _p_conf->item_bytes_left()
    << ".  items_to_drop = " << _p_conf->items_to_drop();
  for (i = 0; i <= _hash._blocks_in_hash; i++)
    {	
      if (i == _hash._blocks_in_hash && _no_in_overflow == 0)
	{
	  dumplog << "\n\nOverflow block is empty.\n";
	  break;
	}
      if (i < _hash._blocks_in_hash)
	dumplog << "\n\nBlock " << i << ": _hash._table[" << i << "] = "
	  << _hash._table[i];
      else
	dumplog << "\n\nBlock " << i << ": ";
      read_in(i);
      dump_buffer();
    }
  dumplog << "\n";
  (*dumper).flush();
}
//  *************************
//  void end_dump()
//     closes the dump file
void 
BallotBox::end_dump(void)
{
  (*dumper).flush();
  (dumplog).close();
  delete dumper;
}
// ****************************
//  void start_dump()
//    opens the dump file
void 
BallotBox::start_dump(void)
{	
  char dumpname[CONFLEN + PATH_LEN + EXT_LEN + 1];
  int l;
  
  if (dump_up == YES)
    return;
  dump_up = YES;
  strcpy(dumpname, _name);
  l = strlen(dumpname) - EXT_LEN;
  strcpy(&dumpname[l], DEXT);
  dumper = new ofstream(dumpname, ios::app);
  while (!dumper)
    {
      if (conferences.adjourn_some(_p_conf) == 0)
	{
	  clerklog << "\nCan't open " << dumpname;
	  clerklog << "Too many open files?  Coming down now.";
	  exit(0);
	}
      dumper->open(dumpname, ios::out);
    }
  time(&now);
  dumplog << "\n" << _p_conf->name() << "  " << ctime(&now);
  (*dumper).flush();
}
#endif

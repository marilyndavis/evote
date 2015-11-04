/* $Id: hash.cc,v 1.4 2003/01/15 18:54:07 marilyndavis Exp $ */ 
// hash.cc Keeps the Hash table for the BallotBox
/*********************************************************
 **********************************************************/
#include "evotedef.h"
extern "C" {
#include <stdlib.h>
#include <string.h>
}
#include <iostream.h>
#include <fstream.h>
#include <strstream.h>
#include <sys/types.h>
#include <sys/msg.h>
#include "ballot.h"
#include "conf.h"
#include "ballotbo.h"
GLOBAL_INCS
#ifdef EDEBUG
void vmem(char* msg, YESorNO free = NO, int size = 0);
#include<stdio.h>
#endif  
// **************************************************
//  hash 's table, keeps a list of the last uid in all but the last block
//                 of data on disk for this conf.  One uid per block.
//                 The last block of data is the overflow buffer for new
//                 voters.  When it gets full, all the blocks get reordered
//                 and the hash gets recalculated.  
//                 Note that the hashed blocks are numbered 0,1, ...,
//                 _blocks_in_hash - 1.  The overflow buffer is numbered
//                 _blocks_in_hash.
// **********************************************
Hash::~Hash(void)
{
  if (_table != NULL)
    {
      delete[] _table;
      _table = NULL;
#ifdef EDEBUG
      if (edebug & MEMS)
	{
	  vmem("Deleted hash._table", NO, 
	       -sizeof(unsigned long)*_blocks_in_hash);
	}
#endif 
    }
}
// **********************************
//  check  checks the _datafile that is kept by hash's ballotbox to be
//         sure that each hash table value is the uid of the last
//         voter in each block.
void
Hash::check(void)
{
  long i, place;
  short new_overflow=0;
  unsigned long check_uid;
  // place points to the front of the last ballot in a block
  for (i = 0; i < _blocks_in_hash; i++)
    {
      _p_bbox->read_in(i);
      place = _p_bbox->_ballot_size * (_p_bbox->_ballots_per_block - 1);
      while (((Ballot*)(_p_bbox->_buffer + place))->action() == DROP)
	place -= _p_bbox->_ballot_size;
      check_uid = *(unsigned long*)(_p_bbox->_buffer + place);
      // check the match			
      if (_table[i] != check_uid) 
	{  // bad match
	  if (i == _blocks_in_hash - 1)  // last hashed block 
	    // - may not be full
	    {
	      // back up through empty entries
	      while (((Ballot *)(_p_bbox->_buffer + (place = place 
						     - _p_bbox->_ballot_size)))->uid()
		     == NO_UID)
		;  // back up through the empty entries
	      if (((Ballot*)(_p_bbox->_buffer + place))->uid() 
		  == _table[i])
		break;
	    }
	  clerklog << "Hash check error on " << _p_bbox->_p_conf->name()
		   << " table[" << i << "] was " << _table[i]
		   << ", changed to " 
		   << check_uid;
	  _table[i] = check_uid;
	}
    }
  _p_bbox->read_in(_blocks_in_hash);
  place = 0;
  while (((Ballot *)(_p_bbox->_buffer + place))->uid() != NO_UID)
    {
      place += _p_bbox->_ballot_size;
      new_overflow++;
    }
  if (new_overflow != _p_bbox->_no_in_overflow)
    {
      clerklog << "Number in overflow was " << _p_bbox->_no_in_overflow
	       << ", fixed to " << new_overflow << ".";
    }
  _p_bbox->_no_in_overflow = new_overflow;
}
//  ************************************
//  HASH class is contained in the BallotBox class and keeps a list
//       of the uid's at the end of each block on disk.
//  ***********************************
//   fetch_info reads the hash values in from the stream.  This is
//              not done at construction because the stream isn't
//              open then.  Hash and BallotBox are constructed by
//              the Conf constructor which then opens the info stream.
//              Then the fetch_info's get called.
OKorNOT
Hash::fetch_info(istream& strm)
{
  long i = -1;
  OKorNOT cc;
  strm >>  _blocks_in_hash;
  if (_blocks_in_hash > 0)
    {
      _table = new unsigned long[_blocks_in_hash];
#ifdef EDEBUG
      if (edebug & MEMS)
	{
	  char msg[100];
	  sprintf(msg,"Made hash table for %ld entries, %ld bytes.", 
		  _blocks_in_hash,
		  (long)sizeof(unsigned long)*_blocks_in_hash);
	  vmem(msg, NO, sizeof(unsigned long)*_blocks_in_hash);
	}
#endif  
      while (++i < _blocks_in_hash )
	{
	  strm >> _table[i];
	  if (_table[i] == 0)
	    {
	      cc = NOT_OK;
	      break;
	    }
	}
    }
  strm >> i;   // collect the final 0
  return cc;
}
// ********************************
//  store_info writes the hash data to the strm
OKorNOT
Hash::store_info(ostream& strm)
{
  long i = -1;
  strm << _blocks_in_hash << ' ';
  while (++i < _blocks_in_hash)
    {
      strm << _table[i] << ' ';
    }
  strm << "0  "; 
  return OK;
}
//  ********************************
//   which_block returns the block on disk that the uid's vote ballot is
//               stored in.
long
Hash::which_block(unsigned long uid)
{
  long top = _blocks_in_hash-1; // top block is last in hash
  long bot = 0;         
  long i;
  if (top == -1 || uid > _table[top])  // newish conf, nothing hashed
    return _blocks_in_hash;       // or biggest uid so far, return
  // overflow buffer.
  if (uid <= _table[bot])
    return 0;
  while (top - bot > 1)
    {
      if (_table[i = (top - bot)/2 + bot] >= uid)
	top = i;
      else
	bot = i;
    }
  return top;
}

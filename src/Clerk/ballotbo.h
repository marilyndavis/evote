/* * eVote - Software for online consensus development.
 * Copyright (C) 2015 Marilyn Davis <marilyn@deliberate.com>
 *
 * This file is part of eVote.
 *
 * eVote is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as
 * published by the Free Software Foundation, either version 3 of the
 * License, or (at your option) any later version.
 *
 * eVote is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with eVote.  If not, see <http://www.gnu.org/licenses/>.
 */

/* $Id: ballotbo.h,v 1.3 2003/01/15 18:54:07 marilyndavis Exp $ */ 
// header for ballotbox functions 
/*********************************************************
 **********************************************************/
#ifndef BALLOTBOXDEF
#define BALLOTBOXDEF
class BallotBox;
class Ballot;
class Conf;
#define PACK_VOTE(X) ((X)<0 ? (unsigned char)(256 + (X)) : (X))
#define UNPACK_VOTE(X) ((X) > NOT_REAL ? (short)((X)-256) : (short)(X))
#define TWO_NOT_READS    31097
#define STARTING_BLOCK_SIZE 1024
class Hash
{
 public:
  Hash(BallotBox *p_bb):_p_bbox(p_bb), _blocks_in_hash(0), _table(NULL){}
  // start with nothing hashed, only overflow buffer is active 
  // at first.
  ~Hash(void);
  
 private:
  BallotBox *_p_bbox;
  long _blocks_in_hash;  
  unsigned long *_table;  // keeps a list of uid's.
  
  void check(void);
  void init(void);
  OKorNOT fetch_info(istream& strm);
  OKorNOT store_info(ostream& strm);
  long which_block(unsigned long uid);
  friend class BallotBox;
};
class BallotBox
{
 public:
  BallotBox(Conf* conf, char* conf_name);
  ~BallotBox(void);
  short ballots_per_block(void);
  short ballot_size(void);
  void blank_ballots(streampos byte, int num);
  void change_no_of_participants(int plus_or_minus_one);
  OKorNOT check_ballots(YESorNO growing = NO);
  OKorNOT check_order(void);
  void drop_pending(Ballot *p_ballot);
  Ballot* fetch(unsigned long uid, long& block, 
		streampos& offset, YESorNO force);
  OKorNOT fetch_info(YESorNO growing = NO);
  Ballot *find_ballot(unsigned long uid, long& block);
  OKorNOT grow(YESorNO force = NO);
  Ballot *iterator(IT_CHOICE choice, long * block = NULL,
		   unsigned long uid_in = 0, YESorNO all = NO);
  unsigned long no_of_participants(void);
  unsigned long ready_to_drop(void);
  void set_ready_to_drop(unsigned long no);
  OKorNOT reorder(YESorNO force = NO, YESorNO refetch = YES);	
  OKorNOT store(Ballot *ballot, long &block, streampos &offset);
  OKorNOT store_info(void);
  int try_dropping_voters(time_t early_item_closing);
  short voter_spots_left(void);
  unsigned long voters_to_drop(void);
  unsigned long pending_droppers(void);
  void write_out(long which_block = -1);
  
 private:
  friend class Hash;
  short _ballots_per_block;  
  short _ballot_size;   // number of bytes in a ballot
  long  _block_in_buffer;  // block currently in the _buffer below
  short _block_size;
  char *_buffer;  // data from disk stored here
  short _chunks;  // number of RECLENs in a ballot
  unsigned long _no_of_participants;
  fstream _datafile;
  Hash _hash;
  char _infname[CONFLEN + PATH_LEN + EXT_LEN + 1];
  char _name[CONFLEN + PATH_LEN + EXT_LEN + 1];
  short _no_in_overflow; // number of voters not in order
  Conf *_p_conf;
  short _rec_len;
  unsigned long _pending_droppers;
  unsigned long _ready_to_drop;
  unsigned long _voters_to_drop;
  
  OKorNOT add_ballot(Ballot *ballot, streampos &offset);
  void check_no_of_participants(void);
  OKorNOT delete_ballot(Ballot *p_ballot, long block, 
			YESorNO looping = NO);
  OKorNOT double_ballot_size(void);
  void read_in(long which_block);
  Ballot *where_in_block(unsigned long uid, YESorNO pass_something = NO);
  void write_ballot(long block, streampos offset, Ballot *p_ballot);
  void write_reorder_ballot(Ballot *& p_ballot, streampos & bytes_in, 
			    long& new_index, streampos * dead_bytes, 
			    ofstream & new_file);
  friend class Conf;
  friend class ItemList;
  friend class TalliedItem;
  
#ifdef EDEBUG
  ofstream *dumper;
  YESorNO dump_up;
  void dump_buffer(void);
  void dump_file(void);		
  YESorNO dump_ballot(Ballot* p_ballot, YESorNO even_if_blank);
  void start_dump(void);
  void end_dump(void);
#endif
};
inline short BallotBox::ballots_per_block(void){return _ballots_per_block;}
inline short BallotBox::ballot_size(void){return _ballot_size;} 
inline short BallotBox::voter_spots_left(void)
{return _ballots_per_block - _no_in_overflow;}
inline unsigned long BallotBox::voters_to_drop(void){return _voters_to_drop;}
inline unsigned long BallotBox::pending_droppers(void){return _pending_droppers;}
inline unsigned long BallotBox::ready_to_drop(void){return _ready_to_drop;}
inline void BallotBox::set_ready_to_drop(unsigned long no)
{_ready_to_drop = no;}
inline unsigned long BallotBox::no_of_participants(void)
{return _no_of_participants;}
inline void BallotBox::change_no_of_participants(int i){_no_of_participants += i;}
#endif

/* $Id: voterlis.h,v 1.3 2003/01/15 18:54:07 marilyndavis Exp $ */ 
// voterlis.h  defines the VoterList class which maintains the
//             list of voters for the conf.
/*********************************************************
 **********************************************************/
#ifndef VOTRLISTHPP
#define VOTRLISTHPP
class Voter;
class Ballot;
class Conf;
class BallotBox;
class VoterList
{
 public:
  VoterList(Conf* p_conf);
  ~VoterList(void);
  void blank_voters(unsigned short byte, int num);
  OKorNOT drop_votes(Ballot* p_ballot);
  Voter *find(unsigned long uid_in, int pid, int qid, YESorNO force,
	      int * p_trouble);
  void forget(Voter *);
  Ballot* get_ballot(unsigned long uid);
  Voter *first(void);
  void send_all(RTYPE message_type, int len);
  void send_all(RTYPE message_type, char* msg, int len); 
 private:
  Conf * _p_conf;
  Voter *_first;
  friend class Voter;
  void attach_after(Voter *this_voter, Voter *after_this);
  void detach(Voter *);  
  Voter *wheres(unsigned long uid_in, Voter *&belongs_after);
  friend class Conf;			
  OKorNOT refetch_all(void); 
  OKorNOT store_all(void);   
  friend class BallotBox;
  OKorNOT slide_in(unsigned long uid_in, long shift_block, 
		   short rec_len);
};
inline
VoterList::VoterList(Conf* p_conf):_p_conf(p_conf), _first(NULL){}
inline Voter * VoterList::first(){return _first;}
#endif

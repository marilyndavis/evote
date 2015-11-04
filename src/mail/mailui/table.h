/* $Id: table.h,v 1.4 2003/01/15 18:54:09 marilyndavis Exp $ */ 
/************************************************************
 *    table.h -- #included into table.cc only  -- for the
 *               translation table for petitions.
 *********************************************************
 *    Copyright (c) 1994...2015 Deliberate.com Patented.
 *    by Marilyn Davis
 **********************************************************/
#include<stdio.h>
#include<string.h>
#include<stdlib.h>
extern "C" {
#include"mailui.h"
}
#define TRANSLATION_DATA "/polls/translation.data"
#define LISTER "LIST: "
#define LISLEN 6
#define SUBJECTER "  SUBJECT: "
#define SUBLEN 11
#define TRANSER "    -"
#define TRALEN 5
YESorNO is_flag(char * str, LANGUAGE * lang);
static FILE * open_trans_file(char * mode);

template<class T> class Ulist;
template<class T> class Iter;
template<class T> class Liter;

template<class T> struct Link
{
  private:
  T thing;
  Link * next;
  friend class Iter<T>;
  friend class Liter<T>;
  friend class Ulist<T>;
  public:
  Link(const T& a):thing(a),next(0){}
  //	 T * p_thing(void){return &thing;}
  //	 Link *p_next(void){return next;}
};

template<class T> class Ulist
{
 private:
  Link<T> *top;
  Link<T> *find(T* a);    // finds copy of the Thing
 public:
  T * get(T* a); // delete incoming if duplicate, returns from list
  T * put(T* a); // forces into the list
  void drop(T* a);
  Ulist(){top = 0;}
  YESorNO is_empty(void){return top == NULL ? YES : NO;}
  friend class Iter<T>;
  friend class Liter<T>;
};

template<class T>class Liter
{
 private:
  Link<T> * this_link;
  Ulist<T> * this_list;
 public:
  Liter(Ulist<T>& s):this_link(s.top),this_list(&s){};
  inline Link<T>* operator()();
};

template<class T> 
Link<T> * Ulist<T>::find(T* a)
{
  Liter<T> it(*this);
  Link<T> * pp;
  while(pp = it())
    {
      if( pp->thing == *a)
	{
	  return pp;
	}
    }
  return NULL;
}

template<class T>
void Ulist<T>::drop(T* a)
{
  Liter<T> it(*this);
  Link<T> * pp;
  Link<T> * prev = NULL;
  
  while(pp = it())
    {
      if( pp->thing == *a)
	{
	  if(prev != NULL)
	    prev->next = pp->next;
	  else
	    top = pp->next;
	  delete pp;
	  return;
	}
      prev = pp;
    }
  return;
}
template<class T>
T* Ulist<T>::get(T* a)
{
  Link<T>* found;
  
  if((found = find(a)) != NULL)
    {
      return &(found->thing);
    }
  found = new Link<T>(*a);
  found->next = top;
  top = found;
  return &(found->thing);
}
template<class T>
T* Ulist<T>::put(T* a)
{
  Link<T>* found;
  
  found = new Link<T>(*a);
  found->next = top;
  top = found;
  return &(found->thing);
}

template<class T>class Iter
{
 private:
  Link<T> * this_link;
  Ulist<T> * this_list;
 public:
  Iter(Ulist<T>& s):this_link(s.top),this_list(&s){}
  void reset(void){this_link = this_list->top;}
  inline T* operator()();
};

template<class T>
T * Iter<T>::operator()()
{
  Link<T> * answer;
  
  answer = this_link;
  if(this_link)
    {
      this_link=this_link->next;
      return &(answer->thing);
    }
  return NULL;
}

template<class T>
Link<T> * Liter<T>::operator()()
{
  Link<T> * answer;
  
  answer = this_link;
  if(this_link)
    {
      this_link=this_link->next;
      return answer;
    }
  return NULL;
}

class String
{
 private:
  char * string;
 public:
  char * get_string(){return string;}
  String(char *str){string = new char[strlen(str) + 1]; strcpy(string, str);}
  friend YESorNO operator==(String s1, String s2){return same(s1.string,s2.string);}
  friend YESorNO operator==(String s1, char* s2){return same(s1.string,s2);}
  friend YESorNO operator==(char * s1, String s2){return same(s1,s2.string);}
};

class Subject;

class Translation
{
 private:
  char flag[4];
  String the_string;
  friend class Subject;
 public:
  char *language_flag(){return flag;}
  char *string(){return the_string.get_string();}
  Translation(char * line):the_string(&line[4]){strncpy(flag, line, 3);flag[3]= '\0';}
  Translation(char * str, char * in_flag):the_string(str){strcpy(flag,in_flag);}
  friend YESorNO operator==(Translation& t1, Translation& t2)
    {return (t1.the_string==t2.the_string 
       && t1.flag[1] == t2.flag[1] && t1.flag[2] == t2.flag[2] ? YES: NO);}
};

class Subject
{
 private:
  String the_string;
  YESorNO pub;
  YESorNO confirm;
  char flag[4];
 public:
  char * get_flag() {return flag;}
  YESorNO is_public() {return pub;}
  YESorNO needs_confirm(){return confirm;}
  char *string(){return the_string.get_string();}
  void make_private(){pub = NO;}
  void make_confirm(){confirm = YES;}
  Ulist<Translation> translation_list;
  Subject(char *str, YESorNO new_pub=YES, YESorNO confirm_in=NO,
	  char * in_flag="-en"):the_string(str),pub(new_pub),
    confirm(confirm_in),translation_list(){strcpy(flag, in_flag);}
  Translation *get(Translation * tran){return translation_list.get(tran);}
  Translation *put(Translation *tran){return translation_list.put(tran);}
  void drop(Translation * trans){translation_list.drop(trans);}
  char * pull(TONGUE lang);
  friend YESorNO operator==(Subject& s1, Subject& s2)
    {return s1.the_string==s2.the_string;}
  friend YESorNO operator==(Subject& s1, char * s2)
    {return s1.the_string==s2;}
  friend YESorNO operator==(char * s2, Subject& s1)
    {return s2==s1.the_string;}
};

char * Subject::pull(TONGUE language)
{
  static Translation * the_translation;

  if(the_translation != NULL 
     /*     && strcmp(the_translation->flag, this->flag) ==0) */
     && strcmp(the_translation->flag, make_flag(language.name)) ==0)
    return the_translation->string();
  Iter<Translation> titer(this->translation_list);
  while((the_translation = titer()))
    {
      if(strcmp(the_translation->flag, make_flag(language.name)) == 0)
	return the_translation->string();
    }
  return NULL;
}

class List
{
 private:
  String the_string;
 public:
  char *string(){return the_string.get_string();}
  Ulist<Subject> subject_list;
  List(char *str):the_string(str),subject_list(){}
  friend YESorNO operator==(List& l1, List& l2)
    {return l1.the_string==l2.the_string;}
  Subject *get(Subject * sub){return subject_list.get(sub);}
  void drop(Subject * sub){subject_list.drop(sub);}
};



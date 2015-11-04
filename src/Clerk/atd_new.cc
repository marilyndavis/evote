//
// eVote - Software for online consensus development.
// Copyright (C) 2015 Marilyn Davis <marilyn@deliberate.com>
//
// This file is part of eVote.
//
// eVote is free software: you can redistribute it and/or modify
// it under the terms of the GNU Affero General Public License as
// published by the Free Software Foundation, either version 3 of the
// License, or (at your option) any later version.
//
// eVote is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License
// along with eVote.  If not, see <http://www.gnu.org/licenses/>.
//

/* $Id: atd_new.cc,v 1.3 2003/01/15 18:54:07 marilyndavis Exp $ */ 
/*   Clerk notes: 
 *   Use this source to usurp the system's functions for new and
 *   delete to help to track down memory problems.
 *   To use this EDEBUG must be #defined so that debug.cc is also
 *   compiled into the source.
 *   And add atd_new.o to the list of objects in the makefile
 *   Output should land in Clerk.debug along with the Clerk's
 *   debug output regarding memory if you run 
 *     eVote_Clerk -x 2048      (2048 == MEMS)
 *
 */

#include <new>
#include <cstdlib>
#include <errno.h>
#include <stdio.h>
#define NO_ACE
#ifndef NO_ACE
#include <ace/Object_Manager.h>
#include <ace/Synch.h>
#endif
#define DEBUG
#ifdef DEBUG // want debugging
#undef NDEBUG
#else
#define NDEBUG // don't want debugging
#endif
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
void vmem(char* msg, YESorNO free = NO , int size = 0);
#endif
char logit[2000];

unsigned long keep_id;

////////////////////////////////////////////////////////////////////////
//
// Forward declare stuff in the anonymous namespace
//
////////////////////////////////////////////////////////////////////////
namespace
{
  
  //
  // The function to be called when new fails
  //
  new_handler my_new_handler;
  
  void *
  do_new(
	 std::size_t size,
	 char const * file,
	 int line_num,
	 bool as_array) throw(std::bad_alloc);
  
  void
  do_delete(
	    void * ptr,
	    char const * file_name,
	    int line_number,
	    bool free_as_array) throw();
  
} // anonymous namespace

////////////////////////////////////////////////////////////////////////
//
// Per 18.4.1.1 of the ISO/IEC 14882:1998(E) C++ specifications
//
// 1. Allocates "size" bytes of storage, suitably aligned to
//    represent any object of that size.
// 2. Replaceable (which is what this function does)
// 3. Required behavior:
//    Returns a non-null pointer to suitably aligned storage,
//    or else throw a bad_alloc exception.
// 4. Default behavior:
//    - Executes a loop: Within the loop, the function first attempts
//      to allocate the requested storage.  Whether the attempt
//      involves a call the the Standard C library function malloc
//      is unspecified.
//    - Returns a pointer to the allocated storage if the attempt
//      is successful.  Otherwise, if the last argument to
//      set_new_handler() was a null pointer, throw bad_alloc.
//    - Otherwise, the function calls the current new_handler.  If the
//      called function returns, the loop repeats.
//    - The loop terminates when an attempt to allocate the requested
//      storage is successful or when a called new_handler
//      function does not return.
//
////////////////////////////////////////////////////////////////////////
void * operator new(std::size_t size) throw(std::bad_alloc)
{
  return do_new(size, 0, __LINE__, false);
}

void * operator new(
		    std::size_t size,
		    char const * file_name,
		    int line_number) throw(std::bad_alloc)
{
  return do_new(size, file_name, line_number, false);
}

////////////////////////////////////////////////////////////////////////
//
// Per 18.4.1.1 of the ISO/IEC 14882:1998(E) C++ specifications
//
// 5. Same as above, except that it is called by a placement version
//    of a new-expression when a C++ program prefers a null pointer
//    result as an error indication, instead of a bad_alloc
//    exception.
// 6. Replaceable (which this is)
// 7. Required behavior:
//    Return a non-null pointer to suitably aligned storage, or else
//    return a null pointer.  This nothrow version of operator new
//    returns a pointer obtained as if acquired from the ordinary
//    version.  This requirement is binding on replacement versions
//    of this function.
// 8. Default behavior:
//    - Executes a loop: Within the loop, the function first attempts
//      to allocate the requested storage.  Whether the attempt
//      involves a call the the Standard C library function malloc
//      is unspecified.
//    - Returns a pointer to the allocated storage if the attempt
//      is successful.  Otherwise, if the last argument to
//      set_new_handler() was a null pointer, return a null pointer.
//    - Otherwise, the function calls the current new_handler.  If the
//      called function returns, the loop repeats.
//    - The loop terminates when an attempt to allocate the requested
//      storage is successful or when a called new_handler
//      function does not return.  If the called new_handler function
//      terminates by throwing a bad_alloc exception, the function
//      returns a null pointer.
//
////////////////////////////////////////////////////////////////////////
void * operator new(std::size_t size, std::nothrow_t const &) throw()
{
  try
    {
      return do_new(size, 0, __LINE__, false);
    }
  catch (std::bad_alloc const &)
    {
      return 0;
    }
}

void * operator new(
		    std::size_t size,
		    char const * file_name,
		    int line_number,
		    std::nothrow_t const &) throw()
{
  try
    {
      return do_new(size, file_name, line_number, false);
    }
  catch (std::bad_alloc const &)
    {
      return 0;
    }
}

////////////////////////////////////////////////////////////////////////
//
// Per 18.4.1.1 of the ISO/IEC 14882:1998(E) C++ specifications
//
// 10. The deallocation function
// 11. Replaceable
// 12. Required behavior:
//     Accept a value of ptr that is null or that was returned by an
//     earlier call to the default operator new(std::size_t) or
//     operator new(std::size_t, std::nothrow_t const &).
// 13. Default behavior:
//     - For a null value of ptr, do nothing
//     - Any other value of ptr shall be a value returned earlier
//       by a call to the default operator new, which was not
//       invalidated by an intervening call to operator delete(void*).
//       For such a non-null value of ptr, reclaims storage allocated by
//       the earlier call to the default operator new.
// 14. It is unspecified under what conditions part or all of such
//     reclaimed storage is allocated by a subsequent call to
//     operator new or any of calloc, malloc, or realloc, declared
//     in <cstdlib>.
//
////////////////////////////////////////////////////////////////////////
void operator delete(void * ptr) throw()
{
  do_delete(ptr, 0, __LINE__, false);
}

void operator delete(
		     void * ptr,
		     char const * file_name,
		     int line_number) throw()
{
  do_delete(ptr, file_name, line_number, false);
}

////////////////////////////////////////////////////////////////////////
//
// Per 18.4.1.1 of the ISO/IEC 14882:1998(E) C++ specifications
//
// Exactly as operator delete(void *) throw()
//
////////////////////////////////////////////////////////////////////////
void operator delete(void * ptr, std::nothrow_t const &) throw()
{
  do_delete(ptr, 0, __LINE__, false);
}

void operator delete(
		     void * ptr,
		     char const * file_name,
		     int line_number,
		     std::nothrow_t const &) throw()
{
  do_delete(ptr, file_name, line_number, false);
}

////////////////////////////////////////////////////////////////////////
//
// Per 18.4.1.2 of the ISO/IEC 14882:1998(E) C++ specifications
//
//  1. Allocate size bytes of storage suitably aligned to
//     represent any array object of that size or smaller.
//  2. Replaceable
//  3. Required behavior:
//     Same as for operator new(std::size_t).
//  4. Default behavior:
//     Returns operator new(size)
//
////////////////////////////////////////////////////////////////////////
void *
operator new[](std::size_t size) throw(std::bad_alloc)
{
  return do_new(size, 0, __LINE__, true);
}

void *
operator new[](
	       std::size_t size,
	       char const * file_name,
	       int line_number) throw(std::bad_alloc)
{
  return do_new(size, file_name, line_number, true);
}

////////////////////////////////////////////////////////////////////////
//
// Per 18.4.1.2 of the ISO/IEC 14882:1998(E) C++ specifications
//
//  5. Same as operator new[](std::size_t) throw(std::bad_alloc)
//     except that it is called by a placement version of a new
//     expression when a C++ program prefers a null pointer result
//     as an error indication instead of a bad_alloc exception/
//  6. Replaceable
//  7. Required behavior:
//     Same as for operator new(std::size_t, std::nothrow_t const &).
//     This nothrow version or operator new[] returns a pointer
//     obtained as if acquired from the ordinary version.
//  8. Default behavior:
//     Returns operator new(size_t, nothrow);
//
////////////////////////////////////////////////////////////////////////
void * operator new[](std::size_t size, std::nothrow_t const &) throw()
{
  try
    {
      return do_new(size, 0, __LINE__, true);
    }
  catch (std::bad_alloc const &)
    {
      return 0;
    }
}

void *
operator new[](
	       std::size_t size,
	       char const * file_name,
	       int line_num,
	       std::nothrow_t const &) throw()
{
  try
    {
      return do_new(size, file_name, line_num, true);
    }
  catch (std::bad_alloc const &)
    {
      return 0;
    }
}

////////////////////////////////////////////////////////////////////////
//
// Per 18.4.1.2 of the ISO/IEC 14882:1998(E) C++ specifications
//
//  9. Deallocation of ptr to render the pointer invalid
// 10. Replaceable
// 11. Required behavior
//     Accept a value of ptr that is null or that was returned by
//     an earlier call to operator new[](std::size_t) or
//     operator new[](std::size_t, std::nothrow_t const &).
// 12. Default behavior
//     - For a null pointer, do nothing.
//     - Any other value of ptr shall be a value returned earlier by a
//       call to the default operator new[](std::size_t).  For such a
//       non-null value of ptr, reclaims storage allocated by the
//       earlier call to the default operator new[].
// 13. It is unspecified under what conditions part or all of such
//     reclaimed storage is allocated by a subsequent call to
//     operator new or any of calloc, malloc, or realloc, declared
//     in <cstdlib>.
//
////////////////////////////////////////////////////////////////////////
void operator delete[](void * ptr) throw()
{
  do_delete(ptr, 0, __LINE__, true);
}

void operator delete[](
		       void * ptr,
		       char const * file_name,
		       int line_number) throw()
{
  do_delete(ptr, file_name, line_number, true);
}

////////////////////////////////////////////////////////////////////////
//
// Per 18.4.1.2 of the ISO/IEC 14882:1998(E) C++ specifications
//
// Exactly as operator delete[](void * ptr) throw()
//
////////////////////////////////////////////////////////////////////////
void operator delete[](void * ptr, std::nothrow_t const &) throw()
{
  do_delete(ptr, 0, __LINE__, true);
}

void operator delete[](
		       void * ptr,
		       char const * file_name,
		       int line_number,
		       std::nothrow_t const &) throw()
{
  do_delete(ptr, file_name, line_number, true);
}

////////////////////////////////////////////////////////////////////////
//
// Per 18.4.1.3 of the ISO/IEC 14882:1998(E) C++ specifications
//
// The placement versions are reserved, and may not be redefined.
//
////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////
//
// Per 18.4.2.3 of the ISO/IEC 14882:1998(E) C++ specifications
//
//  1. Establishes the function designed by new_p as the current
//     new_handler.
//  2. Returns 0 on the first call, the previous new_handler on
//     subsequent calls.
//
// NOTE: the spec does not say whether or not this function may be
//       replaced.
//
////////////////////////////////////////////////////////////////////////
new_handler set_new_handler(new_handler new_p) throw()
{
# ifdef DEBUG
  sprintf(logit, "changing new_handler from %p to %p\n",
          my_new_handler, new_p);
  vmem(logit);
  
# endif
  new_handler previous_handler = my_new_handler;
  my_new_handler = new_p;
  return previous_handler;
}

////////////////////////////////////////////////////////////////////////
//
// The guts of the allocation and deallocation are handled in the
// anonymous namespace
//
////////////////////////////////////////////////////////////////////////

namespace
{
  
  ////////////////////////////////////////////////////////////////////////
  //
  // class Node represents one allocation block.  The last data member
  // is an array of one word.  This array will be used as the allocation
  // unit returned by the memory allocator.
  //
  ////////////////////////////////////////////////////////////////////////
  class Node
  {
  public:
    Node()
      : next_(this)
      , prev_(this)
      , orig_alloc_size_((std::size_t)-1)
      , file_name_("unknown")
      , line_number_(-1)
      , allocated_as_array_(false)
      , id_(++next_id_number_)
      {
      }
    Node(
	 Node * prev_node,
	 size_t alloc_size,
	 char const * file,
	 int line,
	 bool allocated_as_array)
      : next_(prev_node->next_)
      , prev_(prev_node)
      , orig_alloc_size_(alloc_size)
      , file_name_(file)
      , line_number_(line)
      , allocated_as_array_(allocated_as_array)
      , id_(++next_id_number_)
      {
	prev_node->next_->prev_ = this;
	prev_node->next_ = this;
      }
    Node * next() { return next_; }
    Node * prev() { return prev_; }
    void set_next(Node * p) { next_ = p; }
    void set_prev(Node * p) { prev_ = p; }
    void * data_ptr() { return data_; }
    std::size_t orig_alloc_size() { return orig_alloc_size_; }
    char const * file_name() { return file_name_; }
    int line_number() { return line_number_; }
    bool is_allocated_as_array() const { return allocated_as_array_; }
    unsigned long id() { return id_; }
    void unlink()
      {
	prev_->next_ = next_;
	next_->prev_ = prev_;
      }
  private:
    Node(Node const &);
    Node & operator=(Node const &);
    
    Node * next_;
    Node * prev_;
    std::size_t orig_alloc_size_;
    char const * file_name_;
    int line_number_;
    unsigned int allocated_as_array_ : 1;
    unsigned long id_;
    int data_[1];
    static unsigned long next_id_number_;
  };
  
  unsigned long Node::next_id_number_;
  
  ////////////////////////////////////////////////////////////////////////
  //
  // Memory_Manager allocates and frees memory blocks.  It also keeps
  // track of memory to try to catch basic memory errors.
  //
  ////////////////////////////////////////////////////////////////////////
  class Memory_Manager
  {
  public:
    virtual void * allocate(
			    std::size_t size,
			    char const * file_name,
			    int line_num,
			    bool allocated_as_array);
    virtual void free(
		      void * ptr,
		      char const * file_name,
		      int line_number,
		      bool free_as_array);
    static Memory_Manager * instance();
    virtual ~Memory_Manager();
    Node * head() { return &head_; }
#ifndef NO_ACE
    ACE_Thread_Mutex & mutex() { return mutex_; }
#endif
    virtual void check_for_leaks();
  protected:
    Memory_Manager();
    Node head_;
#ifndef NO_ACE
    ACE_Thread_Mutex mutex_;
#endif
  };
  
  ////////////////////////////////////////////////////////////////////////
  //
  // This class is used to free memory after the Memory_Manager
  // instance has been destructed.  Consider some static object that
  // allocates memory in its constructor, and deallocates memory in its
  // destructor.  Now, if this object is constructed before this module,
  // then it's destructor will execute after it.  Well, the memory
  // manager will have already been deleted, causing all kinds of
  // problems.  So, we will use this to simply allocat and free memory
  // without accounting for it.  The exception is frees of memory that
  // have already been reported as leaks.
  //
  ////////////////////////////////////////////////////////////////////////
  class Cleanup_Memory_Manager : public Memory_Manager
  {
  public:
    virtual void * allocate(std::size_t, char const *, int, bool);
    virtual void free(void *, char const *, int, bool);
    void set_list(Node * head_ptr);
  };
  
  Memory_Manager::
  Memory_Manager()
{
}
  
  Memory_Manager::
  ~Memory_Manager()
{
}

//
// Check for chunks of memory that have been allocated, but not yet
// freed.
//
void Memory_Manager::
check_for_leaks()
{
  // DO NOT DELETE THE LIST OF NODES!!!
# ifdef ALLOC_ERRORS
  size_t total_bytes_leaked = 0;
  size_t total_blocks_leaked = 0;
  Node * node;
  for (node = head_.next(); node != &head_; node = node->next())
    {
      if (node->file_name())
	{
	  sprintf(logit, "%lu: leak of %d bytes (%p) at %s: %d\n",
		  node->id(), node->orig_alloc_size(), node->data_ptr(),
		  node->file_name(), node->line_number());
	  vmem(logit);
	}
      else
	{
	  sprintf(logit, "%lu: leak of %d bytes (%p)\n",
		  node->id(), node->orig_alloc_size(), node->data_ptr());
	  vmem(logit);
	  
	  
	}
      total_bytes_leaked += node->orig_alloc_size();
      ++total_blocks_leaked;
    }
  if (total_blocks_leaked > 0)
    {
      sprintf(logit, "Leaked %d total bytes in %d blocks\n",
	      total_bytes_leaked, total_blocks_leaked);
      vmem(logit);
    }
# endif
}

//
// Allocate memory chunks by creating nodes with enough size to
// hold the node and the requested amount of memory.
//
void * Memory_Manager::
allocate(
	 std::size_t size,
	 char const * file_name,
	 int line_num,
	 bool allocated_as_array)
{
#ifndef NO_ACE
  ACE_Guard<ACE_Thread_Mutex> guard(mutex_);
#endif
  std::size_t node_size = size;
  node_size += sizeof(Node);
  node_size -= sizeof(int);
  void * ptr = malloc(node_size);
  if (!ptr)
    {
      return 0;
    }
  // Use placement new to put the node into the malloc'd memory
  Node * node = new(ptr) Node(head_.prev(),
                              size,
                              file_name,
                              line_num,
                              allocated_as_array);
  keep_id = node->id();
  return node->data_ptr();
}

//
// Free the memory, reporting errors when appropriate
//
void Memory_Manager::
free(
     void * ptr,
     char const * file_name,
     int line_number,
     bool free_as_array)
{
#ifndef NO_ACE
  ACE_Guard<ACE_Thread_Mutex> guard(mutex_);
#endif
  Node * node;
  for (node = head_.next(); node != &head_; node = node->next())
    {
      if (node->data_ptr() == ptr)
	{
#     ifdef ALLOC_ERRORS
	  if (node->is_allocated_as_array() != free_as_array)
	    {
	      if (node->is_allocated_as_array())
		{
		  if (node->file_name())
		    {
		      if (file_name)
			{
			  sprintf(logit, "ERROR: %lu (%p) array allocation at "
				  "%s: %d "
				  "freed as non-array at %s: %d\n",
				  node->id(),
				  ptr, node->file_name(), node->line_number(),
				  file_name, line_number);
			  vmem(logit);
			  
			}
		      else
			{
			  sprintf(logit, "ERROR: %lu (%p) array allocation at "
				  "%s: %d "
				  "freed as non-array\n", node->id(),
				  ptr, node->file_name(), node->line_number());
			  vmem(logit);
			  
			}
		    }
		  else if (file_name)
		    {
		      sprintf(logit, "ERROR: %lu (%p) array allocation "
			      "freed as non-array at %s: %d\n", node->id(),
			      ptr, file_name, line_number);
		      vmem(logit);
		      
		      
		    }
		  else
		    {
		      sprintf(logit, "ERROR: %lu (%p) array allocation ",node->id(),
			      "freed as non-array\n", ptr);
		      vmem(logit);
		      
		      
		    }
		}
	      else
		{
		  if (node->file_name())
		    {
		      if (file_name)
			{
			  sprintf(logit, "ERROR: %lu (%p) non-array allocation at "
				  "%s: %d "
				  "freed as array at %s: %d\n",node->id(),
				  ptr, node->file_name(), node->line_number(),
				  file_name, line_number);
			  vmem(logit);
			  
			  
			}
		      else
			{
			  sprintf(logit, "ERROR: %lu (%p) non-array allocation at "
				  "%s: %d "
				  "freed as array\n", node->id(),
				  ptr, node->file_name(), node->line_number());
			  vmem(logit);
			  
			  
			}
		    }
		  else if (file_name)
		    {
		      sprintf(logit, "ERROR: %lu (%p) non-array allocation "
			      "freed as array at %s: %d\n",node->id(),
			      ptr, file_name, line_number);
		      vmem(logit);
		      
		      
		    }
		  else
		    {
		      sprintf(logit, "ERROR: %lu (%p) non-array allocation "
			      "freed as array\n", node->id(), ptr);
		      vmem(logit);
		      
		      
		    }
		}
	    }
#     endif
	  // Found what we want to delete!!!
	  node->unlink();
	  ::free(node);
	  return;
	}
    }
# ifdef ALLOC_ERRORS
  if (file_name)
    {
      sprintf(logit, "ERROR: %lu stray pointer (%p) deleted at %s: %d\n",
	      node->id(),
	      ptr, file_name, line_number);
      vmem(logit);
      
      
    }
  else
    {
      sprintf(logit, "ERROR: %lu stray pointer (%p) deleted\n", node->id(),
	      ptr);
      vmem(logit);
      
      
    }
# endif
}


Memory_Manager * mem_mgr;
Cleanup_Memory_Manager * cleanup_mem_mgr;

Memory_Manager * Memory_Manager::
instance()
{
  if (mem_mgr == 0)
    {
#ifndef NO_ACE
      ACE_MT(ACE_GUARD_RETURN(ACE_Recursive_Thread_Mutex,
			      lock,
			      *ACE_Static_Object_Lock::instance(),
			      0));
#endif
      if (mem_mgr == 0)
	{
	  void * buffer = malloc(sizeof(Memory_Manager));
	  if (!buffer)
	    {
	      sprintf(logit, "Can't allocate space for memory manager!\n");
	      vmem(logit);
	      
	      
	      exit(ENOMEM);
	    }
	  mem_mgr = new (buffer) Memory_Manager();
	  
	  buffer = malloc(sizeof(Cleanup_Memory_Manager));
	  if (!buffer)
	    {
	      sprintf(logit, "Can't allocate space for memory manager!\n");
	      vmem(logit);
	      
	      
	      exit(ENOMEM);
	    }
	  cleanup_mem_mgr =
	    new (buffer) Cleanup_Memory_Manager();
	}
    }
  return mem_mgr;
}

void Cleanup_Memory_Manager::
set_list(Node * head_ptr)
{
  Node * prev_node = head_.prev();
  Node * node;
  while ((node = head_ptr->next()) != head_ptr)
    {
      node->unlink();
      node->set_next(prev_node->next());
      node->set_prev(prev_node);
      prev_node->next()->set_prev(node);
      prev_node->set_next(node);
    }
}

////////////////////////////////////////////////////////////////////////
//
// Simple allocation.
////////////////////////////////////////////////////////////////////////
void * Cleanup_Memory_Manager::
allocate(std::size_t size, char const *, int, bool)
{
  // We are simply calling malloc.  No need for a lock.
  return ::malloc(size);
}

////////////////////////////////////////////////////////////////////////
//
// Free memory.  If it was previously reported as a leak, try to
// make ammends.
//
////////////////////////////////////////////////////////////////////////
void Cleanup_Memory_Manager::
free(void * ptr, char const *, int, bool)
{
#ifndef NO_ACE
  ACE_Guard<ACE_Thread_Mutex> guard(mutex_);
#endif
  Node * node;
  for (node = head_.next(); node != &head_; node = node->next())
    {
      if (node->data_ptr() == ptr)
	{
#     ifdef ALLOC_ERRORS
	  if (node->file_name())
	    {
	      sprintf(logit, "%lu: %d bytes (%p), allocated at "
		      "%s: %d, freed after leak detection\n",
		      node->id(), node->orig_alloc_size(), ptr,
		      node->file_name(), node->line_number());
	      vmem(logit);
	      
	      
	    }
	  else
	    {
	      sprintf(logit, "%lu: %d bytes (%p), "
		      "freed after leak detection\n",
		      node->id(), node->orig_alloc_size(), ptr);
	      vmem(logit);
	      
	      
	    }
#     endif
	  
	  // Found what we want to delete!!!
	  node->unlink();
	  ::free(node);
#     ifdef ALLOC_ERRORS
	  if (head_.next() == &head_)
	    {
	      sprintf(logit, "All leaks have been cleaned\n");
	      vmem(logit);
	      
	      
	    }
#     endif
	  return;
	}
    }
  
  // Wasn't found, so free it flat out...
  ::free(ptr);
}

////////////////////////////////////////////////////////////////////////
//
// This is a helper class to check for memory leaks.  It also
// transfers allocation responsibilities to a separate class
// that tries to resolve freeing of memory that has already been
// reported as a leak.  This is a hack around a deficiency in the
// design, but it is sufficient for our needs.
//
////////////////////////////////////////////////////////////////////////
class Memory_Manager_Deleter
{
public:
  ~Memory_Manager_Deleter();
};

Memory_Manager_Deleter::
~Memory_Manager_Deleter()
{
  if (mem_mgr && mem_mgr != cleanup_mem_mgr)
    {
      mem_mgr->check_for_leaks();
      Memory_Manager * old_mem_mgr = mem_mgr;
      cleanup_mem_mgr->set_list(mem_mgr->head());
      mem_mgr = cleanup_mem_mgr;
      old_mem_mgr->~Memory_Manager();
      ::free(old_mem_mgr);
    }
}
static Memory_Manager_Deleter memory_manager_deleter;

////////////////////////////////////////////////////////////////////////
//
// Use the Memory_Manager class to implement the semantics of new
//
////////////////////////////////////////////////////////////////////////
void *
do_new(
       std::size_t size,
       char const * file,
       int line_num,
       bool as_array) throw(std::bad_alloc)
{
  void * ptr;
  
  if (!mem_mgr)
    {
      mem_mgr = Memory_Manager::instance();
    }
  
  // Guarantee allocation for size of 0
  if (size == 0)
    {
#   ifdef ALLOC_ERRORS
      if (file)
	{
	  sprintf(logit, "WARNING: allocation of 0 bytes at %s: %d\n",
		  file, line_num);
	  vmem(logit);
	  
	}
      else
	{
	  sprintf(logit, "WARNING: allocation of 0 bytes\n");
	  vmem(logit);
	  
	  
	}
#   endif
      ++size;
    }
  
  // Execute a loop, attempting to allocate the requested storage.
  while ((ptr = mem_mgr->allocate(size, file, line_num, as_array)) == 0)
    {
      // If the last argument to set_new_handler() was a null
      // pointer, throw bad_alloc.
      if (!my_new_handler)
	{
#     ifdef DEBUG
	  sprintf(logit, "allocation of %d bytes failed: "
		  "throwing bad_alloc\n",
		  size);
	  vmem(logit);
	  
	  
#     endif
	  throw std::bad_alloc();
	}
      
      // Call the current new_handler, and continue the loop if
      // it returns.
#   ifdef DEBUG
      sprintf(logit, "allocation of %d bytes failed: "
	      "calling new_handler\n",
	      size);
      vmem(logit);
      
      
#   endif
      my_new_handler();
    }
  
# ifdef DEBUG
  sprintf(logit, "%lu allocated %d bytes at %p\n", keep_id,
	  size, ptr);
  vmem(logit);
  
  
# endif
  // If the allocation was successful, return the pointer.
  return ptr;
}

////////////////////////////////////////////////////////////////////////
//
// Use the Memory_Manager class to implement the semantics of delete
// delete
//
////////////////////////////////////////////////////////////////////////
void
do_delete(
	  void * ptr,
	  char const * file_name,
	  int line_number,
	  bool free_as_array) throw()
{
  // Do nothing for a null pointer
  if (ptr)
    {
#   ifdef DEBUG
      sprintf(logit, "deleting %p\n", ptr);
      vmem(logit);
      
      
#   endif
#   ifdef ALLOC_ERRORS
      if (!mem_mgr)
	{
	  sprintf(logit, "ERROR: calling free before a call to new\n");
	  vmem(logit);
	  
	  
	  return;
	}
#   endif
      if (mem_mgr)
	{
	  mem_mgr->free(ptr, file_name, line_number, free_as_array);
	}
    }
}

} // anonymous namespace


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

/* $Id: atd_new.h,v 1.3 2003/01/15 18:54:07 marilyndavis Exp $ */ 

#ifndef ATD__new__h_
#  define ATD__new__h_

#  include <new>

#  if defined (ATD_REPLACE_NEW)

void * operator new(
    std::size_t size,
    char const * file,
    int line_num) throw(std::bad_alloc);

void * operator new(
    std::size_t size,
    char const * file,
    int line_num,
    std::nothrow_t const &) throw();

void * operator new[](
    std::size_t size,
    char const * file,
    int line_num) throw(std::bad_alloc);

void * operator new[](
    std::size_t size,
    char const * file,
    int line_num,
    std::nothrow_t const &) throw();

void operator
delete( 
    void * ptr,  
    char const * file_name, 
    int line_number) throw();

void operator
delete( 
    void * ptr,  
    char const * file_name, 
    int line_number,
    std::nothrow_t const &) throw();

void operator
delete[](
    void * ptr,
    char const * file_name,
    int line_number) throw();

void operator
delete[](
    void * ptr,
    char const * file_name,
    int line_number,
    std::nothrow_t const &) throw();

#    define NEW(ctor) new (__FILE__, __LINE__) ctor
#    define NEW_NOTHROW(ctor) new (__FILE__, __LINE__, nothrow) ctor
#    define DELETE(ptr) operator delete (ptr, __FILE__, __LINE__)
#    define DELETE_NOTHROW DELETE(ptr)
#    define NEW_ARR(class, num_elts) \
        new (__FILE__, __LINE__) class [ num_elts ]
#    define NEW_ARR_NOTHROW(class, num_elts) \
        new (__FILE__, __LINE__, nothrow) class [ num_elts ]
#    define DELETE_ARR(ptr) operator delete [] (ptr, __FILE__, __LINE__)
#    define DELETE_ARR_NOTHROW(ptr) DELETE_ARR(ptr)

#  else

#    define NEW(ctor) new ctor
#    define NEW_NOTHROW(ctor) new (nothrow) ctor
#    define DELETE(ptr) delete ptr
#    define DELETE_NOTHROW DELETE(ptr)
#    define NEW_ARR(class, num_elts) \
        new class [ num_elts ]
#    define NEW_ARR_NOTHROW(class, num_elts) \
        new (nothrow) class [ num_elts ]
#    define DELETE_ARR(ptr) delete [] ptr
#    define DELETE_ARR_NOTHROW(ptr) DELETE_ARR(ptr)

#  endif // defined (ATD_REPLACE_NEW)

#endif // ATD__new__h_

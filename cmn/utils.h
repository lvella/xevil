/* 
 * XEvil(TM) Copyright (C) 1994,2000 Steve Hardt and Michael Judge
 * http://www.xevil.com
 * satan@xevil.com
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or (at
 * your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program, the file "gpl.txt"; if not, write to the Free
 * Software Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
 * 02111-1307, USA, or visit http://www.gnu.org.
 */

// "utils.h" Misc. utilities. to be included by ALL files.


#ifndef UTILS_H
#define UTILS_H

#if X11
	#ifndef NO_PRAGMAS
	#pragma interface
	#endif
#endif


#if WIN32
#define Boolean BOOL
#define False FALSE
#define True TRUE

// Uncomment this to enable modeless dialogs for XEvil.
//#define MODELESS_DIALOGS
#endif



// math.h
#ifdef MATH_CLASS_HACK 
// Lame-ass AIX math.h has a function called class()
#define class fakeclass
#endif 
#if X11
#ifndef MATH_H_IS_CC
extern "C" {
#endif
#include <math.h>
#ifndef MATH_H_IS_CC
}
#endif
#endif 
#ifdef MATH_CLASS_HACK
#undef class
#endif 
#if WIN32
	#include <math.h>
#endif


// For some odd-ball linux machines, needed before <stdio.h>
#ifdef IO_COOKIE_HACK  
#define _IO_cookie_io_functions_t int
#endif

extern "C" {
#include <stdio.h>
  //#include <stdlib.h>
#include <time.h> // For seed to srandom and clock().

#if X11
#include <sys/time.h> // for clock_t and gettimeofday().
#include <X11/X.h>
#include <X11/Xlib.h>
#endif

#if WIN32
#include <sys/utime.h>
#endif
}
#include <assert.h>

#include <sys/types.h>  // for u_char, u_int, etc.
#include <string.h>
#include <limits.h>



// Defines
#ifndef MSEC_PER_CLOCK
#define MSEC_PER_CLOCK (1.0e3 / CLOCKS_PER_SEC) 
#endif

#ifndef max
#define max(a,b)               (a<b ? b : a)
#endif
#ifndef min
#define min(a,b)               (a>b ? b : a)
#endif

#if X11
typedef char Boolean;
#endif

typedef int ColorNum;

#if X11
typedef struct timeval CMN_TIME;
#endif
#if WIN32
// milliseconds
typedef int CMN_TIME;
#endif 


// So we can do "new charP[n]".
typedef char *charP;
typedef const char* constCharP;


// Network stuff.
#if X11
#define CMN_SOCKET int
#define CMN_PORT u_short
#endif
#if WIN32
#define CMN_SOCKET SOCKET
#define CMN_PORT USHORT
#endif

#if X11
#include <netinet/in.h>
#endif
typedef struct sockaddr CMN_SOCKADDR;
typedef struct sockaddr_in CMN_SOCKADDR_IN;


#ifdef RANDOM_NEEDS_PROTOTYPES
extern "C" {
long random();
void srandom(int);
}
#endif 


class InStream;
typedef InStream *InStreamP;
class OutStream;
typedef OutStream *OutStreamP;



typedef int GameStyleType;
// Possible values of GameStyleType.
enum {SCENARIOS,LEVELS_ONLY,KILL,DUEL,EXTENDED,TRAINING,LEVELS};



class Timer {
 public:
  Timer() {remaining = maxx = 0;}
  // Starts out ready.
  Timer(int t) {assert (t >= 0); maxx = t; remaining = 0;} 
  Boolean ready() {return remaining == 0;}
  int get_remaining() {return remaining;}
  void set() {remaining = maxx;}
  void set(int time) {remaining = time;}
  void set_max(int m) {maxx = m; remaining = 0;}
  void reset() {remaining = 0;}
  void clock() {if (remaining) remaining--;}

 private:
  int remaining;
  int maxx;
};


// Possible values of PulseValue x:
// (x>=0) means pulse x times per turn.  (x==0 means don't pulse.)
// (x<=-2) means pulse every -x turns.
// (x==-1) invalid value
typedef int PulseValue;

class Pulser {
public:
  Pulser() {value = 0;}
  Pulser(PulseValue x) {value = x;}

  void set_pulse_value(PulseValue x) {value = x;}

  int operator () ();
  /* EFFECTS: Get the number of pulses this turn. */

  void clock() {timer.clock();}
  /* REQUIRES: Must be called exactly once each turn. */


private:
  Timer timer;  // Only used if value <= -2.
  PulseValue value;
};



class Utils {
 public:
  static void seed_random();
  /* REQUIRES: Must be called at startup. */
  /* EFFECTS: Seed the random number generator based on the current time. */

  static Boolean coin_flip();
  /* EFFECTS: Randomly returns True or False; */

  static int choose(int x);
  /* EFFECTS: Randomly return a number from 0 to x-1. */
  
  static int weighted_choose(int n,int* weights);
  /* REQUIRES: weights is an array of length n.  Each weight is >= 0. */
  /* EFFECTS: Randomly choose a number from 0 to n-1.  Probabitly of returning
     m is weights[m]/sum(weights). */

  static void insertion_sort(int arry[],int numElements);

  static void random_list(int arry[],int numElements);
  /* EFFECTS: Fills arry the first numElements of arry with the numbers in 
     {0..(numElements-1)} in a random order. */

  static int minimum(int v1,int v2) {return v1 <= v2 ? v1 : v2;}

  static int maximum(int v1,int v2) {return v1 >= v2 ? v1 : v2;}

  static int minimum(int arry[],int size);
  /* EFFECTS: Return the minimum value in the array arry of size size. */

  static int minimum(int arry[],Boolean oks[],int size);
  /* EFFECTS: Return the minimum value in arry which is ok or -1 if none. */

  static Boolean inList(int key,const int list[],int size);
  /* EFFECTS: Returns whether key is in list list of size size. */

  static void freeif(char *&str);
  /* MODIFIES: str */
  /* EFFECTS: Free memory of str if non-NULL.  Set str to NULL. */

  static char *strdup(const char *str);
  /* EFFECTS: Copy str, allocating new memory.  If str is NULL, return 
     NULL. */

  static int strlen(const char* cs)
    {assert(cs); return ::strlen(cs);}
  /* REQUIRES: cs is non-NULL */
  
  static int strcmp(const char* str1,const char* str2)
    {return ::strcmp(str1,str2);}
  /* EFFECTS: Wrapper for C library strcmp().  Return 0 if strings are the
     same. */

  static char* strchr(const char* cs,int c)
    {return ::strchr(cs,c);}

  static char* strrchr(const char* cs,int c)
    {return ::strrchr(cs,c);}

  static char* strstr(const char* cs,const char* ct)
    {return ::strstr(cs,ct);}

  static void strcpy(char* s,const char* ct)
    {::strcpy(s,ct);}

  static void strncpy(char* s,const char* ct,int n)
    {::strncpy(s,ct,(size_t)n);}

  static void strcat(char* s,const char* ct)
    {::strcat(s,ct);}

  static int atoi(const char*);

  static const char* getenv(const char*);

  static int mod(int n,int m) 
    {return (n >= 0) ? (n % m) : (-(-n % m) + m) % m;} 
  /* EFFECTS: Return (n % m), allowing n to be negative. */

  static int div(int n,int m) {if (n >= 0)
                                 return (n / m);
                               else if (-n % m == 0) 
                                 return (n / m);
                               else 
                                 return (n / m) - 1;}
  /* EFFECTS: Return (n / m), adjust for negative n. */

  static int ceil(float x) {return (int)(x + 0.5f);}
  /* EFFECTS: Round float up to nearest int. */

  static int floor(float x) {return (int)x;}
  /* EFFECTS: Round float down to nearest int. */

  static int ceil_div(int n,int m);
  /* EFFECTS: Integer division, rounding up. */

  static Boolean string_equals_ignore_case(char *str1,char *str2);
  /* EFFECTS: Does str1 equal str2 ignoring case. */

  static void string_read(InStreamP,char *buffer,int bufLen);
  /* EFFECTS: Read character string from InStream, put results in buffer.
     buffer will be null-terminated, even if the string read in is truncated.
     String will be completely read in from the InStream no matter what. */

  static char* string_read(InStreamP in);
  /* EFFECTS: Read char string from in, return newly allocated string with 
     entire message.  Use delete to free string when done.  Return 
     empty string if error, never returns NULL. */

  static int get_string_write_length(const char *msg);
  /* EFFECTS: The amount of bytes necessary to write msg to an output 
     stream. */

  static void string_write(OutStreamP,const char *msg);
  /* EFFECTS: Write msg to the output stream. */

  static const char* arg_value_check(int& n,int argc,char** argv,
                                     const char* name);
  /* MODIFIES: n */
  /* EFFECTS: Helper for parsing command line arguments that are of the 
     form "-name value".  Check element n of argv against the supplied 
     name.  If match found, return value, else NULL.  If a match is found, 
     n will be incremented to skip over the value in the command line 
     list. */

  static Boolean arg_name_check(int n,int argc,char** argv,
                                const char* name);
  /* EFFECTS: Helper for parsing Boolean command line switches, of the
     form "-name".  Check element n of argv against the supplied name. */

// Make X11 versions of these when we need them.
#if WIN32
  static Boolean is_dir(const char* fName);
  /* EFFECTS: Return True if fName exists on the filesystem and is a 
     directory.  Will work whether fName ends in '\' or not. */

  static Boolean mkdir(const char* fName);
  /* EFFECTS: Create a new directory.  Return whether successful.  Will only 
     create the last component of fName as a new directory. */
  /* NOTE: We could make a version that will make all the new directories 
     necessary for fName. */
#endif

  static const char* game_style_to_string(GameStyleType);
  /* EFFECTS: Convert GameStyleType to string, return "Unknown" if unknown
     or invalid GameStyleType. */
  /* NOTE: Is here because the GameStyleType enumeration is here. */

  static char* get_OS_info();
  /* EFFECTS: Return a platform-dependent string describing the current 
     operating system.  Caller must free returned value with delete. */


private:
  static void string_read_body(InStreamP,u_short len,char *buffer,int bufLen);
};



/* A generic untyped growable list. */
typedef void *voidP;
class PtrList {
public:
  PtrList();
  /* EFFECTS: Create PtrList of zero initial allocation. */
  /* NOTE: Guaranteed not to allocate/free any memory if 
     no elements are ever added. */

  PtrList(int);
  /* EFFECTS: Create PtrList of given initial allocation. */
  /* NOTE: Guaranteed not to allocate/free any memory if this constructor
     is called with zero, and no elements are ever added. */

  PtrList(void *);
  PtrList(void *,void *);
  PtrList(void *,void *,void *);
  PtrList(void *,void *,void *,void *);
  PtrList(void *,void *,void *,void *,void *);
  /* EFFECTS: Create a PtrList with length and allocation equal to the number of 
     arguments.  Initialize with the given elements. */
  
  ~PtrList();
  /* NOTE: Does not delete the elements pointed to by the list. */

  PtrList(const PtrList &) {assert(0);}  
  void operator = (const PtrList &) {assert(0);}

  
  //////// PTRLIST IO NOT TESTED /////////
  PtrList(InStreamP);
  /* EFFECTS: Create a list from a stream. */
  
  static int get_write_length(int len);
  /* EFFECTS: write_length of any list of the given length. */

  int get_write_length() const {return get_write_length(len);}
  /* EFFECTS: write_length of this list */

  void write(OutStreamP) const;
  /* EFFECTS: Write to stream. */  
  

  void add(void *);
  /* EFFECTS: Append new value to the end of list. */

  void *get(int) const;
  /* EFFECTS: I'll give you one guess. */

  Boolean contains(void *el) const;
  /* EFFECTS: Is el in the list, using == for comparison. */

  int index(void *el) const;
  /* EFFECTS: Return the index of the first occurance of el in the list or 
     -1 if not found.  Use == for comparison. */
  /* NOTE: Simple linear search. */
  
  void add_unique(void *el) {if (!contains(el)) add(el);}
  /* EFFECTS: Add el to the list if it is not there already.  
     Use == for comparison. */

  void set(int i,void *val);
  /* EFFECTS: Set indexed element to new value. */
  /* REQUIRES: i < length() */ 

  int length() const {return len;}
  /* EFFECTS: Number of elements added to list. */

  Boolean empty() const {return length() == 0;}

  void del(int i);
  /* EFFECTS: Delete element i, copy the last element of the list to 
     element i. */
  /* NOTE: Be careful when iterating and deleting elements as del() moves 
     elements around. */

  void clear() {len = 0;}
  /* EFFECTS: Empty the list. */

  void append(const PtrList &other);
  /* EFFECTS: Add all elements of other to the list.  Does not check for
     duplicates. */

  void fill(int n,void *val = NULL);
  /* EFFECTS: Append val to the list n times.  Useful for initializing a List. */

  void set_and_fill(int i,void *val,void *fill = NULL);
  /* EFFECTS: Set element i to val.  Grow the list as necessary, filling in the 
     newly created empty element with "fill". */


private:
  void commonConstructor(int);
  void increaseSize();

  int len;
  int allc;
  void **data;
};



// Iterator over an IDictionary.
class IDictIterator {
public:
  virtual ~IDictIterator();

  virtual void* next(void*& key) = 0;
  /* MODIFIES: key */
  /* EFFECTS: Return the next value in the iteration or NULL if done.  
     Sets key to be the key corresponding to value. */
};



// A key-value lookup system.
class IDictionary {
public:
  virtual ~IDictionary();
  
  virtual void* get(void* key) = 0;
  /* EFFECTS: Return the value assoicated with key, or NULL if none. */

  virtual void *getAtIndex(int index) = 0;
  /* REQUIRES: 0 <= index < size() */
  /* EFFECTS: Return the value stored at the given index. */

  virtual int length() = 0;
  /* EFFECTS: Return the number of associations in the dictionary. */

  virtual IDictIterator* iterate() = 0;
  /* REQUIRES: Don't add new assocations or delete the IDictionary before you
     are done with the IDictIterator.  You may change the value of an existing 
     <key,value> association, however. */
  /* EFFECTS: Return an iterator for the values of the Dictionary. */

  virtual void* put(void* key,void* value) = 0;
  /* REQUIRES: Neither key nor value are NULL */
  /* EFFECTS: Put (key,value) association into the dictionary.  Return the previous
     value stored at key if any, else return NULL. */
};



IDictionary* HashTable_factory(int startAlloc = -1,
                               int (*hashFntn)(void *key,int length) = 0);
/* EFFECTS: Create a HashTable implementation of IDictionary. */



// Print out debug info.
class DebugInfo {
public:
  static void initialize();
  /* EFFECTS: Call once at startup. */

  static Boolean on() {return _on;}  
  /* EFFECTS: Is printing debug info turned on. */

  static void turn_on() {_on = True;}
  /* EFFECTS: Enable printing out debug info. */

  static void print(const char *);
  /* EFFECTS: Print the debug message to the appropriate place if debugging is on. */


private:
  static Boolean _on;  
};

#endif  //UTILS_H

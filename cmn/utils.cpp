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

// "utils.C"

#if X11
#ifndef NO_PRAGMAS
#pragma implementation "utils.h"
#endif
#endif


// Include Files
#include "stdafx.h"
#include <cstdint>
#include <iostream>
#include <fstream>
#include <limits.h>
#include <string.h>
#include <ctype.h>
#if WIN32
#include <strstrea.h>
#endif
#if X11
#include <strstream>
#endif

#ifdef NO_STDLIB
// We will get warnings from doing this, but fuck it.  At least it compiles.
//int atoi(const char*);
//char* getenv(const char*);
#else
#include <stdlib.h>
#endif

#ifdef WIN32
// For file manipulation routines.
#include <direct.h>
#include <sys/types.h>
#include <sys/stat.h>
// For GetVersionEx
#include <winbase.h>
#endif

#include "utils.h"
#include "streams.h"

using namespace std;

#define PTR_LIST_DEFAULT_ALLC 5



int Pulser::operator () () {
  // Multiple pulses every turn.
  if (value >= 0) {
    return value;
  }

  // Pulse once every few turn.
  assert(value <= -2);
  if (timer.ready()) {
    timer.set(-value);
    return 1;
  }
  return 0;
}



void Utils::seed_random() {
  int seed = (int)time(NULL);

#ifdef USE_RANDOM
  srandom(seed);
#else
  srand((unsigned int)seed);
#endif
}



Boolean Utils::coin_flip() {
#ifdef USE_RANDOM
  return (Boolean)(random() % 2);
#else
  return (Boolean)(rand() % 2);
#endif
}



int Utils::choose(int x) {
  assert (x > 0);
#ifdef USE_RANDOM
  return (int)(random() % x);
#else
  return rand() % x;
#endif
}



int Utils::weighted_choose(int n,int* weights) {
  // Get sum of all weights.
  int sum = 0;
  int m;
  for (m = 0; m < n; m++) {
    assert(weights[m] >= 0);
    sum += weights[m];
  }

  // Choose number in spectrum of summed weights.
  int which = choose(sum);
  // Find index of weight corresponding to chosen number.
  sum = 0;
  for (m = 0; m < n; m++) {
    sum += weights[m];
    if (which < sum) {
      return m;
    }
  }

  // We counted wrong.
  assert(0);
  return 0;
}



void Utils::insertion_sort(int arry[],int numElements) {
  for (int j = 0; j  < numElements - 1; j++) {
    // Set arry[j] to be the minimum from arry[j]..arry[numElements-1].
    for (int i = j + 1; i < numElements; i++) {
      if (arry[i] < arry[j]) {
        int tmp = arry[i];
        arry[i] = arry[j];
        arry[j] = tmp;
      }
    }
  }
}



void Utils::random_list(int arry[],int num) {
  int n;
  for (n = 0; n < num; n++) {
    arry[n] = n;
  }

  for (n = num - 1; n > 0; n--) {
    int index = choose(n);
    int tmp = arry[index];
    arry[index] = arry[n];
    arry[n] = tmp;
  }
}



int Utils::minimum(int arry[],int size) {
  Boolean anySet = False;
  int ret = 0;

  for (int n = 0; n < size; n++) {
    if (!anySet || (arry[n] < ret)) {
      ret = arry[n];
      anySet = True;
    }
  }

  assert(anySet);
  return ret;
}




int Utils::minimum(int arry[],Boolean oks[],int size) {
  Boolean anySet = False;
  int ret = -1;

  for (int n = 0; n < size; n++) {
    if (oks[n] && (!anySet || (arry[n] < ret))) {
      ret = arry[n];
      anySet = True;
    }
  }

  return ret;
}



Boolean Utils::inList(int key,const int list[],int size) {
  for (int i = 0; i < size; i++) {
    if (list[i] == key) {
      return True;
    }
  }
  return False;
}



void Utils::freeif(char *&str) {
  if (str) {
    delete [] str;  
  }
  str = NULL;
}



char *Utils::strdup(const char *str) {
  char *ret = NULL;
  if (str) {
    ret = new char[strlen(str)+1];
    assert(ret);
    strcpy(ret,str);
  }
  return ret;
}



int Utils::ceil_div(int n,int m) {
  assert(n >= 0 && m > 0);
  if (n % m == 0) {
    return n / m;
  }
  else {
    return (n / m) + 1;
  }
}



Boolean Utils::string_equals_ignore_case(char *str1,char *str2) {
  if (str1 == NULL && str2 == NULL) {
    return True;
  }

  if (str1 == NULL || str2 == NULL) {
    return False;
  }

  // Compare lower case versions.
  int n;
  for (n = 0; str1[n] && str2[n]; n++) {
    if (tolower(str1[n]) != tolower(str2[n])) {
      return False;
    }
  }

  // Both ended at the same time.
  if (!str1[n] && !str2[n]) {
    return True;
  }

  // Different lengths, one is a prefix of the other.
  return False;
}



void Utils::string_read(InStreamP in,char *buffer,int bufLen) {
  // get the length.
  u_short l = in->read_short();

  string_read_body(in,l,buffer,bufLen);
}



char* Utils::string_read(InStreamP in) {
  // get the length.
  u_short l = in->read_short();

  // Will return empty string if error.
  if (!in->alive()) {
    l = 0;
  }

  // Allocate string long enough to hold data plus NULL.
  char* buffer = new char[l + 1];
  assert(buffer);

  string_read_body(in,l,buffer,l + 1);

  return buffer;
}



void Utils::string_read_body(InStreamP in,u_short l,char *buffer,int bufLen) {
  if (!in->alive()) {
    // Don't read anything, return empty string.
    buffer[0] = '\0';
    return;
  }

  if (l + 1 < bufLen) {
    // Entire string fits in buffer.
    in->read(buffer,l);
    buffer[l] = '\0';
  }
  else {
    // Will have to truncate some of the string.
    char *b = new char[l];
    in->read(b,l);
    memcpy(buffer,b,bufLen - 1);
    buffer[bufLen - 1] = '\0';
    delete [] b;
  }
}


int Utils::get_string_write_length(const char *msg) {
  return 
    sizeof(short) +      // length of string.
    strlen(msg);         // string itself.
}



void Utils::string_write(OutStreamP out,const char *msg) {
  assert(msg);
  int len = strlen(msg);
  assert(len <= USHRT_MAX);
  out->write_short((u_short)len);
  out->write((void*)msg,len);
}



const char* Utils::arg_value_check(int& n,int argc,char** argv,
                                   const char* name) {
  assert(n < argc);
  if (!strcmp(name,argv[n]) && (n + 1 < argc)) {
    n++;
    return argv[n];
  }
  return NULL;
}



Boolean Utils::arg_name_check(int n,int argc,char** argv,
                              const char* name) {
  assert(n < argc);
  if (!strcmp(name,argv[n])) {
    return True;
  }
  return False;
}



#if WIN32
Boolean Utils::is_dir(const char* fName) {
  if (!fName || !fName[0]) {
    return False;
  }
  
  // Make local copy that is guaranteed NOT to end in '\'.
  char* ffName = Utils::strdup(fName);
  int len = strlen(ffName);
  if (ffName[len - 1] == '\\') {
    ffName[len - 1] = '\0';
  }

  Boolean ret;
  struct _stat buffer;
  int val = _stat(ffName,&buffer);
  if ((val == 0) && (buffer.st_mode & _S_IFDIR)) {
    ret = True;
  }
  else {
    ret = False;
  }
  delete ffName;
  return ret;
}



Boolean Utils::mkdir(const char* fName) {
  int val = _mkdir(fName);
  return (val == 0);
}
#endif



const char* Utils::game_style_to_string(GameStyleType gsType) {
  switch (gsType) {
  case LEVELS:
    return "Levels";
  case SCENARIOS:
    return "Scenarios";
  case LEVELS_ONLY:
    return "Levels Only";
  case KILL:
    return "Kill,Kill,Kill";
  case DUEL:
    return "Duel";
  case EXTENDED:
    return "Extended Duel";
  case TRAINING:
    return "Training";
  default:
    return "Unknown";
  }
}



char* Utils::get_OS_info() {
  ostrstream ret;


#ifdef WIN32
  OSVERSIONINFO osInfo;
  osInfo.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
  Boolean set = False;

  if (GetVersionEx(&osInfo)) {
    switch (osInfo.dwPlatformId) {
      // Windows NT
      case VER_PLATFORM_WIN32_NT:
        set = True;
        ret << "WinNT " << osInfo.dwMajorVersion << '.' 
            << osInfo.dwMinorVersion;
        
        break;
      // Windows 95 or 98
      case VER_PLATFORM_WIN32_WINDOWS:
        set = True;
        // Not very certain of the logic I'm using to discern between
        // Win 95 and 98.
        // Useless fucking Microsoft APIs.
        if (osInfo.dwMajorVersion == 4 &&
            osInfo.dwMinorVersion >= 10) {
          ret << "Windows 98";
        }
        else if (osInfo.dwMajorVersion == 4 &&
                 osInfo.dwMinorVersion == 0) {
          ret << "Windows 95";
        }
        else {
          ret << "Windows 95/98";
        }
        break;    
    }
    // Only copy in the extra info (like service pack number)
    // if there is something there.
    if (set) {
      if (strlen(osInfo.szCSDVersion) && 
          strcmp(osInfo.szCSDVersion," ")) {
        ret << '(' << osInfo.szCSDVersion << ')';
      }
      ret << ends;
    }
  }

  if (!set) {
    ret << "Unknown Win32" << ends;
  }
#endif


#ifdef UNAME_USR_BIN
#define UNAME_PATH "/usr/bin/uname"
#else
#define UNAME_PATH "/bin/uname"
#endif

#ifdef X11
  FILE* fp = popen(UNAME_PATH " -a","r");
  Boolean set = False;
  if (fp) {
    const int BUF_LEN = 120;
    char buffer[120];
    if (fgets(buffer,BUF_LEN,fp) != 0) {
      int strLen = strlen(buffer);
      if (strLen > 0) {
        // Kill trailing newline
        if (buffer[strLen - 1] == '\n') {
          buffer[strLen - 1] = '\0';
        }
        ret << buffer << ends;
        set = True;
      }
    }
    pclose(fp);
  }

  if (!set) {
    ret << "Unknown UNIX" << ends;
  }
#endif


  return ret.str();
}



PtrList::PtrList() {
  // By default, don't allocate any memory untils the first ::add().
  commonConstructor(0);
}



PtrList::PtrList(int startLen) {
  commonConstructor(startLen);
}



PtrList::PtrList(void *el0) {
  commonConstructor(1);
  add(el0);
}



PtrList::PtrList(void *el0,void *el1) {
  commonConstructor(2);
  add(el0);
  add(el1);
}



PtrList::PtrList(void *el0,void *el1,void *el2) {
  commonConstructor(3);
  add(el0);
  add(el1);
  add(el2);
}



PtrList::PtrList(void *el0,void *el1,void *el2,void *el3) {
  commonConstructor(4);
  add(el0);
  add(el1);
  add(el2);
  add(el3);
}



PtrList::PtrList(void *el0,void *el1,void *el2,void *el3,void *el4) {
  commonConstructor(5);
  add(el0);
  add(el1);
  add(el2);
  add(el3);
  add(el4);
}



PtrList::~PtrList() {
  if (data) {
    delete [] data;
  }
}


/*
// Code broken on 64 bits, doesn't compile
PtrList::PtrList(InStreamP in) {
  if (in->alive()) {
    commonConstructor(0);
    return;
  }

  // We will set both len and allc to the size read in from stream.
  int desiredLen = in->read_int();
  commonConstructor(desiredLen);
  for (int n = 0; n < desiredLen; n++) {
    add((void *)in->read_int());
  }  
}
*/


int PtrList::get_write_length(int len) {
  return sizeof(int) +          // length of list
         len * sizeof(void *);  // all elements
}


/*
// Code broken on 64 bits, doesn't compile
void PtrList::write(OutStreamP out) const{
  // Not tested.
  out->write_int(len);
  for (int n = 0; n < len; n++) {
    out->write_int((int)data[n]);
  }
}
*/


void PtrList::add(void *val) {
  assert(len <= allc);
  if (len == allc) {  
    // increases allc, but not len
    increaseSize();
  }

  data[len] = val;
  len++;
}



void *PtrList::get(int i) const {
  assert(i < len && i >= 0);
  return data[i];
}



Boolean PtrList::contains(void *el) const {
  return index(el) != -1;
}



int PtrList::index(void *el) const {
  for (int n = 0; n < len; n++) {
    if (data[n] == el) {
      return n;
    }
  }
  return -1;
}



void PtrList::set(int i,void *val) {
  assert(i < len && i >= 0);
  data[i] = val;
}



void PtrList::set_and_fill(int i,void *val,void *fill) {
  while (i >= len) {
    append(fill);
  }
  set(i,val);
}



void PtrList::del(int i) {
  assert(i < len && i >= 0);
  data[i] = data[len - 1];
  data[len - 1] = NULL; // really unnecessary.
  len--;
}



void PtrList::append(const PtrList &other) {
  for (int n = 0; n < other.length(); n++) {
    add(other.get(n));
  }
}



void PtrList::fill(int num,void *val) {
  for (int i = 0; i < num; i++) {
    append(val);
  }
}



void PtrList::commonConstructor(int startAllc) {
  len = 0;
  allc = startAllc;
  if (startAllc > 0) {
    data = new voidP[allc];
  }
  else {
    data = NULL;
  }
}



void PtrList::increaseSize() {
  assert(len == allc);  
  // Double list size, unless it was zero.  If zero, use default size.
  int newAllc = (allc > 0) ? (2 * allc) : PTR_LIST_DEFAULT_ALLC;
  void **newData = new voidP[newAllc];
  assert(newData);

  if (data) {
    assert(allc > 0);
    memcpy(newData,data,allc * sizeof(void *));
    delete [] data;
  }

  // len is unchanged.
  data = newData;
  allc = newAllc;
}



IDictionary::~IDictionary() {
}



class Bucket {
friend class HashTable;
friend class HashIterator;

private:
  Bucket(void *k,void *v,Bucket*n)
  {key = k; value = v; next = n;}
  
  void* key;
  void* value;
  Bucket* next;
};



class HashTable: public IDictionary {
  friend class HashIterator;

public:
  HashTable(int startAlloc,int (*hashFntn)(void* key,int length));
  /* EFFECTS: Create hash table with initial number of allocated elements, or use
     default number if not specified.  hashFntn specifies the hash function, or 
     use a default implementation. */

  virtual ~HashTable();

  virtual void* get(void* key);
  virtual void* getAtIndex(int index);
  virtual int length();
  virtual IDictIterator* iterate();
  virtual void* put(void* key,void* value);


private:
  Bucket* _get(int &index,void* key);
  /* MODIFIES: index */
  /* EFFECTS: Internal helper function.  Return the Bucket containing key
     or NULL if not found.  Set index to the bucket list for key whether
     key is found or not. */
  
  // Don't use a power of 2 or of 10.  May mess up this crappy hash function.
  enum {DEFAULT_BUCKETS_NUM = 97};

  static int defaultHash(void*,int);
  /* EFFECTS: The default hash function. */

  Bucket** buckets;
  int bucketsNum;
  // The actual hash function being used.
  int (*hashFunction)(void *key,int length);
};



class HashIterator: public IDictIterator {
public:
  HashIterator(HashTable*);
  
  virtual ~HashIterator();

  virtual void* next(void*& key);


private:
  HashTable* hashTable;
  Bucket *bucket;
  int bucketRow;
};



IDictionary* HashTable_factory(int startAlloc,int (*hashFntn)(void *key,int length)) {
  IDictionary *ret = new HashTable(startAlloc,hashFntn);
  assert(ret);
  return ret;
}



HashTable::HashTable(int startAlloc,int (*hashFntn)(void* key,int length)) {
  // Decide number of buckets.
  if (startAlloc == -1) {
    bucketsNum = DEFAULT_BUCKETS_NUM;
  }
  else {
    bucketsNum = startAlloc;
  }

  // Decide which hash function to use.
  if (hashFntn) {
    hashFunction = hashFntn;
  }
  else {
    hashFunction = defaultHash;
  }

  // Create empty list of buckets.
  buckets = new Bucket* [bucketsNum];
  assert(buckets);
  for (int n = 0; n < bucketsNum; n++) {
    buckets[n] = NULL;
  }
}



HashTable::~HashTable() {
  // Delete all the buckets.
  for (int n = 0; n < bucketsNum; n++) {
    Bucket* b = buckets[n];
    while (b) {
      Bucket* bNext = b->next;
      delete b;
      b = bNext;
    } 
  }

  // Delete the list itself.
  delete [] buckets;
}



void* HashTable::put(void* key,void* value) {
  // value may not be NULL, or we have to change what the return value means for
  // HashTable::get
  assert(value);

  int index;
  Bucket* b = _get(index,key);

  // Value already exists, replace it.
  if (b) {
    void* ret = b->value;
    b->value = value;
    return ret;
  }
  // Insert new bucket at beginning of given bucket list.
  else {
    Bucket* newB = new Bucket(key,value,buckets[index]);
    buckets[index] = newB;
    return NULL;
  }
}


  
void* HashTable::get(void* key) {
  int dummy;
  Bucket* b = _get(dummy,key);
  if (b) {
    return b->value;
  }
  return NULL;
}



void* HashTable::getAtIndex(int index) {
  int count = 0;
  for (int n = 0; n < bucketsNum; n++) {
    Bucket* b = buckets[n];
    while (b) {
      if (count == index) {
        return b->value;
      }
      count++;
      b = b->next;
    }
  }
  return NULL;
}



// Inefficient implementation.  We could easily cache the length with
// each put operation.
int HashTable::length() {
  int ret = 0;
  for (int n = 0; n < bucketsNum; n++) {
    Bucket* b = buckets[n];
    while (b) {
      ret++;
      b = b->next;
    }
  }
  return ret;
}



IDictIterator* HashTable::iterate() {
  IDictIterator* ret = new HashIterator(this);
  assert(ret);
  return ret;
}


  
Bucket* HashTable::_get(int &index,void* key) {  
  index = hashFunction(key,bucketsNum);
  assert(index >= 0 && index < bucketsNum);
  Bucket* b = buckets[index];
  while (b) {
    if (b->key == key) {
      return b;
    }
    b = b->next;
  }
  return NULL;
}



// Pretty crappy hash function, I know.
// Careful if bucketsNum is a power of 2.
int HashTable::defaultHash(void* key,int bucketsNum) {
  return ((intptr_t) key) % bucketsNum;
}



IDictIterator::~IDictIterator() {
}



HashIterator::HashIterator(HashTable* hTable) {
  // So we will start at row 0.
  bucketRow = -1;
  bucket = NULL;
  hashTable = hTable;
}



HashIterator::~HashIterator() {
}



void* HashIterator::next(void*& key) {
  while (1) {  
    // If at end of a row of buckets.
    if (bucket == NULL) {
      // Past end of the last row of Buckets.
      if (bucketRow >= hashTable->bucketsNum - 1) {
        return NULL;
      }

      // Start at first bucket of next row of Buckets.
      bucketRow++;
      bucket = hashTable->buckets[bucketRow];
    }
    
    if (bucket) {
      // The bucket we will return this time.
      Bucket* ret = bucket;
      
      // Get ready for next time.
      bucket = bucket->next;

      key = ret->key;
      return ret->value;
    }
  }
}



void DebugInfo::initialize() {
#if WIN32
  if (_on) {
    fstream outStream;
    outStream.open("c:\\out.txt",ios::out);
    outStream.close();
  }
#endif
}



void DebugInfo::print(const char *str) {
  if (!_on) {
    return;
  }

#if WIN32
  fstream outStream;
  outStream.open("c:\\out.txt",ios::app); 
  outStream << str << endl;
  outStream.close();
  TRACE(str);
  TRACE("\n");
#endif

#if X11
  cout << str << endl;
#endif
}



// This gives the default setting for whether to print debug info or not.
Boolean DebugInfo::_on = 
#if X11
False;
#endif
#if WIN32
#if _DEBUG
True;
#else
False;
#endif
#endif // WIN32


//
//  phash.h
//  stockfish
//
//  Created by Jeremy Bernstein on 22.05.13.
//  Copyright (c) 2013 stockfishchess. All rights reserved.
//

#if !defined(PHASH_H_INCLUDED)
#define PHASH_H_INCLUDED

#include "position.h"
#include "ucioption.h"

/*

#include <iostream>
#include "misc.h"
#include "thread.h"

class PHashEntry {

public:
  PHashEntry() : reference(NULL), next(NULL) { }
  ~PHashEntry() {}
  
  void set_reference(void *ref) { reference = ref; }
  void set_next(PHashEntry *n) { next = n; }
  PHashEntry *get_next() const { return next; }
  
private:
  void *reference;
  PHashEntry *next;
};

class PHashList {
  
public:
  PHashList() : head(NULL) {}
  ~PHashList() { clear(); }
  
  void save(void *reference) {
    
    static Mutex m;
    
    m.lock();

    PHashEntry *entry = new PHashEntry;
    entry->set_reference(reference);
    entry->set_next(head);
    head = entry;

    m.unlock();
  }
  
  void iterate() {
    struct timeval tv1, tv2;
    unsigned count = 0;
    PHashEntry *x = head;
    
    gettimeofday(&tv1, NULL);
    
    while (x) {
      
      x = x->get_next();
      count++;
    }
    gettimeofday(&tv2, NULL);
    sync_cout << "\nPHashList::iterate() " << count << " entries in " << ((tv2.tv_sec * 1000.) + (tv2.tv_usec / 1000.) - (tv1.tv_sec * 1000.) + (tv1.tv_usec / 1000.)) << " milliseconds.\n" << sync_endl;
  }

  void clear() {
    struct timeval tv1, tv2;
    unsigned count = 0;
    PHashEntry *x = head;
    
    gettimeofday(&tv1, NULL);
    
    while (x) {
      
      PHashEntry *nx = x->get_next();
      delete x;
      x = nx;
      count++;
    }
    gettimeofday(&tv2, NULL);
    sync_cout << "\nPHashList::clear() " << count << " entries in " << ((tv2.tv_sec * 1000.) + (tv2.tv_usec / 1000.) - (tv1.tv_sec * 1000.) + (tv1.tv_usec / 1000.)) << " milliseconds.\n" << sync_endl;
  }

private:
  PHashEntry *head;
};

extern PHashList PHL; // Our global list of interesting positions
*/

typedef enum { PHASH_READ, PHASH_WRITE } PHASH_MODE;

void init_phash();
void quit_phash();
void store_phash(const Key key, Value v, Bound t, Depth d, Move m, Value statV, Value kingD);
int probe_phash(const Key key, Depth *d);
void starttransaction_phash(PHASH_MODE mode);
void endtransaction_phash();
void to_tt_phash();
void wantsclear_phash();
void wantsprune_phash();

#endif /* defined(ISAM_H_INCLUDED) */

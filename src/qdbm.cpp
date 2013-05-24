/*
 Stockfish, a UCI chess playing engine derived from Glaurung 2.1
 Copyright (C) 2004-2008 Tord Romstad (Glaurung author)
 Copyright (C) 2008-2010 Marco Costalba, Joona Kiiski, Tord Romstad
 
 Stockfish is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.
 
 Stockfish is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.
 
 You should have received a copy of the GNU General Public License
 along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

////
//// Includes
////
#if PA_GTB
#include <cassert>
#include <string>

#include "phash.h"
#include "qdbm/depot.h"
#include "tt.h"

//PHashList PHL; // Our global list of interesting positions

////
//// Local definitions
////

namespace
{
  /// Variables
  DEPOT *PersHashFile = NULL;
  bool PersHashWantsClear = false;
}

typedef struct _phash_data
{
  Value v;
  Bound t;
  Depth d;
  Move m;
  Value statV;
  Value kingD;
} t_phash_data;

////
//// Functions
////

int count_phash();
DEPOT *open_phash(PHASH_MODE mode);
void close_phash(DEPOT *depot);
void clear_phash();
void prune_phash(int numrecs);
void optimize_phash();
int getsize_phash();

//#define PHASH_DEBUG

// init_phash() initializes or reinitializes persistent hash if necessary.

void init_phash()
{
  starttransaction_phash(PHASH_READ);
#ifdef PHASH_DEBUG
  count_phash();
#endif
  endtransaction_phash();
/*
  bool usePersHash = Options["Use Persistent Hash"];
  std::string persHashFilePath = Options["Persistent Hash File"];
  //int persHashSize = Options["PersistentHashSize"];
  const char *cfilename = persHashFilePath.c_str();
  
  if (!usePersHash) {
    close_phash();
    return;
  }
  
  PersHashFile = dpopen(cfilename, DP_OWRITER | DP_OCREAT, 0);
  if (PersHashFile) {
    Key k = 0;
    const char *key = (const char *)&k;
    const char *data = "testdata";
    char data2[128];
    int data2size;
    
    int rv = dpput(PersHashFile, key, sizeof(Key), data, 9, DP_DOVER);
    printf("dpput %d\n", rv);

    data2size = dpgetwb(PersHashFile, key, sizeof(Key), 0, 128, data2);
    if (data2size) {
      printf("dpget: got %0llx size %d\n", *((Key *)key), data2size);
    } else {
      printf("dpget: failure\n");
    }

    count_phash();
    clear_phash();
    count_phash();
  }
*/
}

void quit_phash()
{
  starttransaction_phash(PHASH_WRITE);
  optimize_phash();
  endtransaction_phash();
}

void wantsclear_phash()
{
  PersHashWantsClear = true;
}

DEPOT *open_phash(PHASH_MODE mode)
{
  bool usePersHash = Options["Use Persistent Hash"];
  std::string filename = Options["Persistent Hash File"];
  DEPOT *hash = NULL;
  
  if (usePersHash) {
    hash = dpopen(filename.c_str(), (mode == PHASH_WRITE) ? DP_OWRITER | DP_OCREAT : DP_OREADER, 0);
    if (mode == PHASH_WRITE) {
      dpsetalign(hash, sizeof(t_phash_data)); // optimizes overwrite operations
    }
  }
  return hash;
}

void close_phash(DEPOT *depot)
{
  dpclose(depot);
}

void store_phash(const Key key, Value v, Bound t, Depth d, Move m, Value statV, Value kingD)
{
  Depth oldDepth = DEPTH_ZERO;

  if (PersHashFile) {
    probe_phash(key, &oldDepth);
    if (d >= oldDepth) {
      t_phash_data data;
      int rv = 0;
      
      data.v = v;
      data.t = t;
      data.d = d;
      data.m = m;
      data.statV = statV;
      data.kingD = kingD;
      
      rv = dpput(PersHashFile, (const char *)&key, (int)sizeof(Key), (const char *)&data, (int)sizeof(t_phash_data), DP_DOVER);
#ifdef PHASH_DEBUG
      if (rv) {
        printf("dpput: put %llx\n", key);
      } else {
        printf("dpput: %s\n", dperrmsg(dpecode));
      }
#endif
    }
  }
}

void starttransaction_phash(PHASH_MODE mode)
{
  if (PersHashFile) return;
  
  if (PersHashWantsClear) {
    PersHashFile = open_phash(PHASH_WRITE);
    clear_phash();
    endtransaction_phash();
    PersHashWantsClear = false;
  } /* else {
    int PersHashSize = Options["Persistent Hash Size"] * 1024 * 1024;
    int RealHashSize;
    PersHashFile = open_phash(PHASH_WRITE);
    RealHashSize = getsize_phash();
    if (RealHashSize > PersHashSize) {
      prune_phash((RealHashSize - PersHashSize) / sizeof(t_phash_data));
    }
    endtransaction_phash();
  } */
  PersHashFile = open_phash(mode);
}

void endtransaction_phash()
{
  if (PersHashFile) {
    close_phash(PersHashFile);
    PersHashFile = NULL;
  }
}

int getsize_phash()
{
  if (PersHashFile) {
    //return dpfsiz(PersHashFile);
    return count_phash() * sizeof(t_phash_data);
  }
  return 0;
}

int probe_phash(const Key key, Depth *d)
{
  int rv = 0;
  
  if (PersHashFile) {
    t_phash_data data;
    int datasize = 0;
    
    datasize = dpgetwb(PersHashFile, (const char *)&key, (int)sizeof(Key), 0, (int)sizeof(t_phash_data), (char *)&data);
    if (datasize == sizeof(t_phash_data)) {
      *d = data.d;
      rv = 1;
    }
  }
  return rv;
}

void to_tt_phash()
{
  if (PersHashFile) {
#ifdef PHASH_DEBUG
    int count = 0;
#endif
    
    if (dpiterinit(PersHashFile)) {
      Key *key;
      
      while ((key = (Key *)dpiternext(PersHashFile, NULL))) {
        t_phash_data data;
        int datasize = 0;
        
        datasize = dpgetwb(PersHashFile, (const char *)key, (int)sizeof(Key), 0, (int)sizeof(t_phash_data), (char *)&data);
        if (datasize == sizeof(t_phash_data)) {
          TT.store(*((Key *)key), data.v, data.t, data.d, data.m, data.statV, data.kingD, false);
#ifdef PHASH_DEBUG
          printf("dpgetwb: pull %llx\n", *((Key *)key));
          count++;
#endif
        }
        free(key);
      }
    }
#ifdef PHASH_DEBUG
    printf("restored %d records\n", count);
#endif
  }
}

void optimize_phash()
{
  if (PersHashFile) {
    dpoptimize(PersHashFile, 0);
  }
}

void prune_phash(int numrecs) // totally arbitrary what gets chopped
{
  if (PersHashFile) {
    if (dpiterinit(PersHashFile)) {
      char *key;
      
      while ((key = dpiternext(PersHashFile, NULL)) && numrecs--) {
        dpout(PersHashFile, (const char *)key, sizeof(Key));
        free(key);
      }
      optimize_phash();
    }
  }
}

void clear_phash()
{
  if (PersHashFile) {
#ifdef PHASH_DEBUG
    int count = 0;
#endif
    int rv;
    
    if (dpiterinit(PersHashFile)) {
      char *key;
      
      while ((key = dpiternext(PersHashFile, NULL))) {
        rv = dpout(PersHashFile, (const char *)key, sizeof(Key));
#ifdef PHASH_DEBUG
        if (rv) {
          count++;
          printf("dpout: deleted %0llx\n", *((Key *)key));
        }
#endif
        free(key);
      }
      optimize_phash();
    }
#ifdef PHASH_DEBUG
    printf("purged %d records\n", count);
#endif
  }
}

int count_phash()
{
  int count = 0;

  if (PersHashFile) {
    if (dpiterinit(PersHashFile)) {
      char *key;
      
      while ((key = dpiternext(PersHashFile, NULL))) count++;
    }
#ifdef PHASH_DEBUG
    printf("counted %d records\n", count);
#endif
  }
  return count;
}

// close_egtb() closes/frees persistent hash if necessary
/*void close_phash()
{
  if (PersHashFile) {
    dpclose(PersHashFile);
    PersHashFile = NULL;
  }
}
*/
#endif

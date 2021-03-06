/*
  Stockfish, a UCI chess playing engine derived from Glaurung 2.1
  Copyright (C) 2004-2008 Tord Romstad (Glaurung author)
  Copyright (C) 2008-2014 Marco Costalba, Joona Kiiski, Tord Romstad

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

#include <iostream>

#include "bitboard.h"
#include "evaluate.h"
#include "position.h"
#include "search.h"
#include "thread.h"
#include "tt.h"
#include "ucioption.h"
#ifdef PA_GTB
#include "bitcount.h"
#include "phash.h"
#ifdef USE_EGTB
#include "egtb.h"
#endif
#endif

int main(int argc, char* argv[]) {

  setvbuf(stdout,NULL,_IONBF,0);

#ifdef PA_GTB
  initPopCnt();
#endif

  std::cout << engine_info() << std::endl;

  UCI::init(Options);
#ifdef PA_GTB
  PHInst.init_phash();
#endif
  Bitboards::init();
  Position::init();
  Bitbases::init_kpk();
  Search::init();
  Pawns::init();
  Eval::init();
  Threads.init();
  TT.resize(Options["Hash"]);
  PH.resize(4);
#if defined(PA_GTB) && defined(USE_EGTB)
  init_egtb(); // Init here, not at the top of each move. Setting uci options will check this and update if necessary.
#endif

  UCI::loop(argc, argv);

#ifdef PA_GTB
  PHInst.quit_phash();
#ifdef USE_EGTB
  close_egtb();
#endif
#endif

  Threads.exit();
}

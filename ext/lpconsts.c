/*  Copyright (C) 2007, 2010 Rocky Bernstein <rockyb@rubyforge.org>

  This program is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
#include <ruby.h>
#include <lpsolve/lp_lib.h>

extern VALUE rb_cLPSolve;

/** \file lpconsts.c
 *
 *  \brief Provides Ruby bindings for LPSolve constants.

    The constants below are all prefixed by \a LPSolve::, e.g. 
    \a LPSolve::EQ.
 */

/**
   Routine called by Init_lpsolve() create \a LPSolve:: constants, e.g.
   \a LPSolve::EQ.
 */
void
init_lpsolve_constants()
{

  /* Equality/Inequality values */

  rb_define_const(rb_cLPSolve,"EQ", INT2FIX(EQ));
  rb_define_const(rb_cLPSolve,"FR", INT2FIX(FR));
  rb_define_const(rb_cLPSolve,"GE", INT2FIX(GE));
  rb_define_const(rb_cLPSolve,"LE", INT2FIX(LE));
  rb_define_const(rb_cLPSolve,"OF", INT2FIX(OF));

  /* Verbosity constants */
  rb_define_const(rb_cLPSolve, "NEUTRAL",  INT2FIX(NEUTRAL));
  rb_define_const(rb_cLPSolve, "CRITICAL", INT2FIX(CRITICAL));
  rb_define_const(rb_cLPSolve, "SEVERE",   INT2FIX(SEVERE));
  rb_define_const(rb_cLPSolve, "IMPORTANT",INT2FIX(IMPORTANT));
  rb_define_const(rb_cLPSolve, "NORMAL",   INT2FIX(NORMAL));
  rb_define_const(rb_cLPSolve, "DETAILED", INT2FIX(DETAILED));
  rb_define_const(rb_cLPSolve, "FULL",     INT2FIX(FULL));

  /* Branch and Bound constants */

  rb_define_const(rb_cLPSolve, "NODE_FIRSTSELECT", 
		  INT2FIX(NODE_FIRSTSELECT));
  rb_define_const(rb_cLPSolve, "NODE_GAPSELECT", 
		  INT2FIX(NODE_GAPSELECT));
  rb_define_const(rb_cLPSolve, "NODE_RANGESELECT", 
		  INT2FIX(NODE_RANGESELECT));
  rb_define_const(rb_cLPSolve, "NODE_FRACTIONSELECT", 
		  INT2FIX(NODE_FRACTIONSELECT));
  rb_define_const(rb_cLPSolve, "NODE_PSEUDOCOSTSELECT", 
		  INT2FIX(NODE_PSEUDOCOSTSELECT));
  rb_define_const(rb_cLPSolve, "NODE_PSEUDONONINTSELECT", 
		  INT2FIX(NODE_PSEUDONONINTSELECT));
  rb_define_const(rb_cLPSolve, "NODE_PSEUDOFEASSELECT", 
		  INT2FIX(NODE_PSEUDOFEASSELECT));
  rb_define_const(rb_cLPSolve, "NODE_PSEUDORATIOSELECT", 
		  INT2FIX(NODE_PSEUDORATIOSELECT));
  rb_define_const(rb_cLPSolve, "NODE_USERSELECT", 
		  INT2FIX(NODE_USERSELECT));
  rb_define_const(rb_cLPSolve, "NODE_STRATEGYMASK", 
		  INT2FIX(NODE_STRATEGYMASK));
  rb_define_const(rb_cLPSolve, "NODE_WEIGHTREVERSEMODE", 
		  INT2FIX(NODE_WEIGHTREVERSEMODE));
  rb_define_const(rb_cLPSolve, "NODE_BRANCHREVERSEMODE", 
		  INT2FIX(NODE_BRANCHREVERSEMODE));
  rb_define_const(rb_cLPSolve, "NODE_GREEDYMODE", 
		  INT2FIX(NODE_GREEDYMODE));
  rb_define_const(rb_cLPSolve, "NODE_PSEUDOCOSTMODE", 
		  INT2FIX(NODE_PSEUDOCOSTMODE));
  rb_define_const(rb_cLPSolve, "NODE_DEPTHFIRSTMODE", 
		  INT2FIX(NODE_DEPTHFIRSTMODE));
  rb_define_const(rb_cLPSolve, "NODE_RANDOMIZEMODE", 
		  INT2FIX(NODE_RANDOMIZEMODE));
  rb_define_const(rb_cLPSolve, "NODE_GUBMODE", 
		  INT2FIX(NODE_GUBMODE));
  rb_define_const(rb_cLPSolve, "NODE_DYNAMICMODE", 
		  INT2FIX(NODE_DYNAMICMODE));
  rb_define_const(rb_cLPSolve, "NODE_RESTARTMODE", 
		  INT2FIX(NODE_RESTARTMODE));
  rb_define_const(rb_cLPSolve, "NODE_BREADTHFIRSTMODE", 
		  INT2FIX(NODE_BREADTHFIRSTMODE));
  rb_define_const(rb_cLPSolve, "NODE_AUTOORDER", 
		  INT2FIX(NODE_AUTOORDER));
  rb_define_const(rb_cLPSolve, "NODE_RCOSTFIXING", 
		  INT2FIX(NODE_RCOSTFIXING));
  rb_define_const(rb_cLPSolve, "NODE_STRONGINIT", 
		  INT2FIX(NODE_STRONGINIT));

  /* Presolve constants used in a bitmask */
  rb_define_const(rb_cLPSolve, "PRESOLVE_NONE",        INT2FIX(PRESOLVE_NONE));
  rb_define_const(rb_cLPSolve, "PRESOLVE_ROWS",        INT2FIX(PRESOLVE_ROWS));
  rb_define_const(rb_cLPSolve, "PRESOLVE_COLS",        INT2FIX(PRESOLVE_COLS));
  rb_define_const(rb_cLPSolve, "PRESOLVE_LINDEP",      INT2FIX(4));
  rb_define_const(rb_cLPSolve, "PRESOLVE_SOS",         INT2FIX(PRESOLVE_SOS));
  rb_define_const(rb_cLPSolve, "PRESOLVE_REDUCEMIP",   INT2FIX(64));
  rb_define_const(rb_cLPSolve, "PRESOLVE_KNAPSACK",    INT2FIX(128));
  rb_define_const(rb_cLPSolve, "PRESOLVE_ELIMEQ2",     INT2FIX(256));
  rb_define_const(rb_cLPSolve, "PRESOLVE_IMPLIEDFREE", INT2FIX(512));
  rb_define_const(rb_cLPSolve, "PRESOLVE_REDUCEGCD",   INT2FIX(1024));
  rb_define_const(rb_cLPSolve, "PRESOLVE_PROBEFIX",    INT2FIX(2048));
  rb_define_const(rb_cLPSolve, "PRESOLVE_PROBEREDUCE", INT2FIX(4096));
  rb_define_const(rb_cLPSolve, "PRESOLVE_ROWDOMINATE", INT2FIX(8192));
  rb_define_const(rb_cLPSolve, "PRESOLVE_COLDOMINATE", INT2FIX(16384));
  rb_define_const(rb_cLPSolve, "PRESOLVE_MERGEROWS",   INT2FIX(32768));
  rb_define_const(rb_cLPSolve, "PRESOLVE_IMPLIEDSLK",  INT2FIX(65536));
  rb_define_const(rb_cLPSolve, "PRESOLVE_COLFIXDUAL",  INT2FIX(131072));
  rb_define_const(rb_cLPSolve, "PRESOLVE_BOUNDS",      INT2FIX(262144));
  rb_define_const(rb_cLPSolve, "PRESOLVE_LASTMASKMODE", 
		  INT2FIX(PRESOLVE_DUALS - 1));
  rb_define_const(rb_cLPSolve, "PRESOLVE_DUALS",       INT2FIX(PRESOLVE_DUALS));
  rb_define_const(rb_cLPSolve, "PRESOLVE_SENSDUALS",   INT2FIX(1048576));


  /* Scaling constants used in a bitmask */

  rb_define_const(rb_cLPSolve, "SCALE_NONE",       INT2FIX(SCALE_NONE));
  rb_define_const(rb_cLPSolve, "SCALE_EXTREME",    INT2FIX(SCALE_EXTREME));
  rb_define_const(rb_cLPSolve, "SCALE_RANGE",      INT2FIX(SCALE_RANGE));
  rb_define_const(rb_cLPSolve, "SCALE_MEAN",       INT2FIX(SCALE_MEAN));
  rb_define_const(rb_cLPSolve, "SCALE_GEOMETRIC",  INT2FIX(SCALE_GEOMETRIC));
  rb_define_const(rb_cLPSolve, "SCALE_CURTISREID", INT2FIX(SCALE_CURTISREID));

  /* Above can be Or'd with.. */
  rb_define_const(rb_cLPSolve, "SCALE_QUADRATIC",   INT2FIX(SCALE_QUADRATIC));
  rb_define_const(rb_cLPSolve, "SCALE_LOGARITHMIC", INT2FIX(SCALE_LOGARITHMIC));
  rb_define_const(rb_cLPSolve, "SCALE_USERWEIGHT",  INT2FIX(SCALE_USERWEIGHT));
  rb_define_const(rb_cLPSolve, "SCALE_POWER2",      INT2FIX(SCALE_POWER2));
  rb_define_const(rb_cLPSolve, "SCALE_EQUILIBRATE", INT2FIX(SCALE_EQUILIBRATE));
  rb_define_const(rb_cLPSolve, "SCALE_INTEGERS",    INT2FIX(SCALE_INTEGERS));
  rb_define_const(rb_cLPSolve, "SCALE_DYNUPDATE",   INT2FIX(SCALE_DYNUPDATE));
  rb_define_const(rb_cLPSolve, "SCALE_ROWSONLY",    INT2FIX(SCALE_ROWSONLY));
  rb_define_const(rb_cLPSolve, "SCALE_COLSONLY",    INT2FIX(SCALE_COLSONLY));

  /* Simplex types */
  rb_define_const(rb_cLPSolve, "SIMPLEX_PRIMAL_PRIMAL", 
		  INT2FIX(SIMPLEX_PRIMAL_PRIMAL));
  rb_define_const(rb_cLPSolve, "SIMPLEX_DUAL_PRIMAL", 
		  INT2FIX(SIMPLEX_DUAL_PRIMAL));
  rb_define_const(rb_cLPSolve, "SIMPLEX_PRIMAL_DUAL", 
		  INT2FIX(SIMPLEX_PRIMAL_DUAL));
  rb_define_const(rb_cLPSolve, "SIMPLEX_DUAL_DUAL",   
		  INT2FIX(SIMPLEX_DUAL_DUAL));

  /* Solve return codes. */
  rb_define_const(rb_cLPSolve, "NOMEMORY",    INT2FIX(NOMEMORY));
  rb_define_const(rb_cLPSolve, "OPTIMAL",     INT2FIX(OPTIMAL));
  rb_define_const(rb_cLPSolve, "SUBOPTIMAL",  INT2FIX(SUBOPTIMAL)); 
  rb_define_const(rb_cLPSolve, "INFEASIBLE",  INT2FIX(INFEASIBLE));
  rb_define_const(rb_cLPSolve, "UNBOUNDED",   INT2FIX(UNBOUNDED));
  rb_define_const(rb_cLPSolve, "DEGENERATE",  INT2FIX(DEGENERATE)); 
  rb_define_const(rb_cLPSolve, "NUMFAILURE",  INT2FIX(NUMFAILURE)); 
  rb_define_const(rb_cLPSolve, "USERABORT",   INT2FIX(USERABORT)); 
  rb_define_const(rb_cLPSolve, "TIMEOUT",     INT2FIX(TIMEOUT));
  rb_define_const(rb_cLPSolve, "PRESOLVED",   INT2FIX(PRESOLVED)); 
  rb_define_const(rb_cLPSolve, "PROCFAIL",    INT2FIX(PROCFAIL)); 
  rb_define_const(rb_cLPSolve, "PROCBREAK",   INT2FIX(PROCBREAK)); 
  rb_define_const(rb_cLPSolve, "FEASFOUND",   INT2FIX(FEASFOUND));
  rb_define_const(rb_cLPSolve, "NOFEASFOUND", INT2FIX(NOFEASFOUND));
}


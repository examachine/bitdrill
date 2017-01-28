// Copyright (C) 2003 salvatore orlando <salvatore.orlando@unive.it>
// University of Venice, Ca' Foscari, Dipartimento di Informatica, (VE) Italy
// Istituto di Scienza e Tecnologia dell'Informazione, ISTI - CNR, (PI) Italy
//  
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software 
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
#ifndef __MY_DEFS_H
#define __MY_DEFS_H


#define TEMPORARY_DATASET       "dci.closed.dataset.tmp" 
#define PARTITIONS_FILE         "dci.closed.partitions.info"
#define UNMAP_FILE              "dci.closed.unmap"
#define MAX_ASCII_ITEM_LEN      10
#define BIN_ITEMSETS_FILE_FMT   "dci.closed.itemsets.bin.%04d"
#define PARTIAL_RESULTS         "dci.closed.hash_partial_sort"
#define MAPPED_OUTPUT           "dci.closed.mapped_output"
#define MAPPED_OUTPUT_SINGLES   "dci.closed.mapped_output.singles"

#define PRUNE_FACTOR_PROJ       0.04 
#define PRUNE_FACTOR_NEWLIST    0.8 
#define PRUNE_FACTOR_COMPLEXITY 2 

// mining at home
// #define MINING_AT_HOME_STATS

// statisitcs
//#define MY_STATS

// remove duplicates from transactions
//#define AVOID_TRANS_CHECK

// verbose
#define VERBOSE
#define JOB_DURATION
//#define THREADS_VERBOSE

//#define DEBUG_FOURTH

// use SSE2
#define USE_SSE
//#define USE_MMX

//#define MY_SET_BIT

// partitioning strategies
#define SIMPLEST_POLICY      1   // one thread one job
#define ONE_THREAD_POLICY    2   // one thread per frequent singleton
#define STATIC_DINAMIC       3   // static partitioning dinamic assignment
#define STATIC_STATIC        4   // static partitioning static assignment
#define DINAMIC_DINAMIC      5   // dinamic partitioning dinamic assignment
#define STATIC_STATIC_P      6   // static partitioning dinamic assignment with proj
#define STATIC_DINAMIC_P     7   // static partitioning dinamic assignment with proj
#define DINAMIC_DINAMIC_P    8   // static partitioning dinamic assignment with proj

#ifndef PARTITIONING_POLICY
#define PARTITIONING_POLICY  2
#endif

#if (PARTITIONING_POLICY==DINAMIC_DINAMIC)||(PARTITIONING_POLICY==DINAMIC_DINAMIC_P)
#define WORK_STEALING
#define LEAST_STEALING 2
#endif

#endif

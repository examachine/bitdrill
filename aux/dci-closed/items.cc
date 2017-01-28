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

#include "items.hh"

/** sort items according to their support
 * and maps their id to contiguous and increasing integers.
 * in this function we build a map from original_id to new_id
 * and a reverse "unmap" from new_id to original. 
 * also, we build a map from the new_id to the ascii representation 
 * of the item, so we have it at hand when (if ever) we will have to write
 * frequent itemsets to disk.
 */
#include "print.hh"
#include <algorithm>
using namespace std;

void dci_items::remap_items(void)

{
  m = (*acc).size();  // items
  m1=0;               // frequent items

  // da sistemare ...  
  for (unsigned int i=0; i < (*acc).size(); i++) {
    (*acc)[i].id = i;
    if ((*acc)[i].count >= min_count) {
      m1++;
      if ((*acc)[i].count > max_supp)
	max_supp = (*acc)[i].count;
    }
  }

  // sort in increasing frequency order
  AscendingItemsSort compare_item;
  sort((*acc).begin(), (*acc).end(), compare_item);

  // --------------------------
  // selective sampling
  // "A Sampling­based Framework For Parallel Data Mining"
  // --------------------------
  if (selective_sampling_th==0) return;
  to_be_removed = (*acc).size()*selective_sampling_th/100;
  if (to_be_removed == 0) return;

  if (to_be_removed > m1) 
    to_be_removed = m1;
  int last_ok = (*acc).size()-1 - to_be_removed;

  for (unsigned int i=last_ok+1; i<(*acc).size(); i++) 
    (*acc)[i].count = 0;
  m1 -= to_be_removed;
  max_supp = (*acc)[last_ok].count;
  sort((*acc).begin(), (*acc).end(), compare_item);
}




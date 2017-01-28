#ifndef Merge_Itemsets_Intf
#define Merge_Itemsets_Intf

#include "Pattern.hxx"
#include "Vertical_DB.hxx"

void merge_itemsets(const Pattern_Set& freq,
		    unsigned int k, 
		    const Pattern_Set& freq1, const Pattern_Set& freq2, 
		    const Vertical_DB& vertical_db, int support, 
		    ofstream* out = 0);


void merge_itemsets_old(Pattern_Set& freq_patterns,
		    unsigned int k, Pattern_Set& freq1, Pattern_Set& freq2, 
		    Vertical_DB& vertical_db, 
			int support, ofstream* out);

#endif

//
// C++ Implementation
//
// Description: Merge two mined itemsets
//
// Author: exa
//
// Copyright: See COPYING file that comes with this distribution
//
//

#include "generic/General.hxx"
#include "transaction-db/Transaction_Set.hxx"
#include "fp-tree/Pattern.hxx"
#include "kdci/items.hh"
#include "kdci/frequents.hh"
#include "kdci/candidates.hh"
#include "kdci/vertical.hh"

void read_patterns(const string& pat_name, list<Pattern>& patterns) 
{
  cout << "reading patterns from " << pat_name << endl;

  // read patterns
  ifstream pattern_input(pat_name.c_str());
  while (!pattern_input.eof()) {
    Pattern p;
    if (p.read(pattern_input)) {
      patterns.push_back(p);
    }
  }
  cout << patterns.size() << " patterns read" << endl << endl;
}

void decompose_by_size(list<Pattern> freq, vector< list<Pattern> > freqbysize)
{
  // find maximum pattern length
  int max_len = 0;
  for (list<Pattern>::iterator x=freq.begin();x!=freq.end();x++) {
    max_len = max(max_len, int(x->symbol_set.size()));
  }

  // partition by pattern size
  freqbysize.resize(max_len + 1);
  for (list<Pattern>::iterator x=freq.begin();x!=freq.end();x++) {
    freqbysize[x->symbol_set.size()].push_back(*x);
  }
}

int patternset_freq_items(list<Pattern> freq, vector<int>& txn)
{
  vector<bool> union_txn;
  for (list<Pattern>::iterator x=freq.begin();x!=freq.end();x++) {
    if (union_txn.size() < x->symbol_set[x->symbol_set.size()-1] + 1) {
      union_txn.resize(x->symbol_set[x->symbol_set.size()-1] + 1, false);
    }
    for (vector<int>::iterator i=x->symbol_set.begin(); 
	 i!=x->symbol_set.end(); i++) {
      union_txn[*i] = true;
    }
  }

  int num_freq = 0;
  int i = 0;
  for (vector<bool>::iterator bit=union_txn.begin();
       bit!=union_txn.end(); bit++) {
    if (*bit==true) {
      num_freq++;
      txn.push_back(i);
    }
    i++;
  }

  return num_freq;
}

int main(int argc, char *argv[])
{
  cout << argv[0] <<  " -- merge two disjoint itemsets of a transaction database" << endl;

  if (argc!=5) {
    cout << "usage: " << argv[0] 
	 << " <db> <freq itemset1> <freq itemset2> <support>" << endl;
    cout << "where <freq itemset1> and <freq itemset2> are all frequent "
	 << " patterns found in two disjoint itemsets of <db> with <support>."
         << endl;
    exit(-1);
  }
  string dbname(argv[1]);
  string freq1name(argv[2]);
  string freq2name(argv[3]);
  istringstream is(argv[4]);
  int support;
  is >> support;
  list<Pattern> freq1;
  read_patterns(freq1name, freq1);
  list<Pattern> freq2;
  read_patterns(freq2name, freq2);

  vector< list<Pattern> > freq1size;
  decompose_by_size(freq1, freq1size);
  vector< list<Pattern> > freq2size;
  decompose_by_size(freq2, freq2size);

  vector<int> F1_1; // frequent items
  vector<int> F1_2;
  vector<int> F1;

  //TODO: better way to find F1 ahh
  patternset_freq_items(freq1, F1_1);
  patternset_freq_items(freq2, F1_2);
  merge(F1_1.begin(), F1_1.end(), 
	F1_2.begin(), F1_2.end(), 
	back_inserter(F1));
  
  int num_freq_items = F1.size();
  int max_item = F1[F1.size()-1];

  vector<int> item_map(max_item+1, 0);
  for (int i=0; i<F1.size(); i++) {
    item_map[F1[i]] = i;
  }
  

#ifdef DEBUG
  cout << "F1 = " << F1 << endl;
#endif
  

  cout << num_freq_items << " frequent items in read patterns" << endl;


  typedef KDCI::set_of_itemsets < unsigned int, unsigned int > Set_Itemset;
  typedef KDCI::DCP_candidates< unsigned int, unsigned int > Candidates;
  typedef KDCI::set_of_frequents < unsigned int, unsigned int > Set_Freq;
  typedef KDCI::DCI_vertical_dataset<unsigned int> DCI_Dataset;

  cout << "reading transaction set " << dbname << endl;

  //KDCI::dci_items counters(support);  // dci counters for singleton
  //counters.copy_F1(F1);
  //int first_freq = counters.remap_items(false);  


  // do a pass over local db.
  ifstream in(dbname.c_str());
  int num_trans = 0;
  //int max_item = 0;
  int max_trans_len = 0;
  Transaction_Set db;
  while (!in.eof()) {
    Transaction t_i;
    if (t_i.read(in, Transaction::ascii)) {
      num_trans++;
      db.add(t_i);
      max_trans_len = max(max_trans_len, int(t_i.size()));
    }
  }
  cout << num_trans << " transactions read" << endl;
  //counters.set_num_of_trans(num_trans);

  DCI_Dataset dci_dataset;
  assert(dci_dataset.VD_can_be_allocated(num_freq_items, num_trans));
  dci_dataset.init_VD();

  int num_txn = 0;
  // POTENTIAL BUG: is this 1 parameter correct???
  KDCI::dci_transaction<unsigned int> t(max_trans_len, 1, max_item+1);
  Transaction_Set::Scanner* scanner = db.gen_scanner();
   // do a pass over transaction set
  while (!scanner->end()) {
    Transaction t_i;
    if (scanner->read(t_i)) {
      // remap transaction
      t.copy_noclique_txn_prune(t_i, item_map);
      if (t.t_len >= 2) {
	dci_dataset.write_t_in_VD(num_txn, t);
	num_txn++;
      }
    }
  }
  delete scanner;
 
  int k = 2; // starting level
  ostringstream os;
  os << dbname << ".out.merge." << support;
  ofstream out(os.str().c_str());


  Set_Freq freq_prev(2);
  freq_prev.reset(2);
  freq_prev.init_keys();
  
  //dci_dataset.chk_compact_vertical(num_freq_items);

  while (1) {
    cout << "mining level " << k << endl;
    int pruned = 0;

    Candidates candidates(k);

    if (k==2) {
      for (list<Pattern>::iterator x=freq1.begin();
	   x!=freq1.end();x++) {
	for (list<Pattern>::iterator y=freq2.begin();
	     y!=freq2.end();y++) {
	  unsigned int v[2];
	  v[0] = x->symbol_set[0];
	  v[1] = y->symbol_set[0];
	  candidates.add_itemset(v, 0);  
#ifdef DEBUG
#endif
	}
	
      }
    }
    else {
      
      for (int xlen=1; xlen<k; xlen++) {
	int ylen = k - xlen;
	if (xlen >= freq1size.size())
	  continue;
	for (list<Pattern>::iterator xs=freq1size[xlen].begin();
	     xs!=freq1size[xlen].end(); xs++) {
	  if (ylen >= freq2size.size())
	    continue;
	  for (list<Pattern>::iterator ys=freq2size[xlen].begin();
	       ys!=freq2size[xlen].end(); xs++) {

	    int zlen = xs->symbol_set.size() + ys->symbol_set.size();
	    unsigned int z[zlen];
	    copy(xs->symbol_set.begin(), xs->symbol_set.end(), &z[0]);
	    copy(ys->symbol_set.begin(), ys->symbol_set.end(), &z[xs->symbol_set.size()]);
	    sort(&z[0], &z[zlen]);

	    bool prune = false;

	    if (xlen > 1) {
	      for (vector<int>::iterator i=xs->symbol_set.begin(); 
		   i!=xs->symbol_set.end(); i++) {
		unsigned int item = *i;
		unsigned int zprev[zlen];
		remove_copy(&z[0], &z[zlen], &zprev[0], item);
		unsigned int key;
		int count;
		if (!freq_prev.find_one_subset(&zprev[0], key, count)) {
		  pruned++;
		  prune = true;
		  break;
		}
	      }
	    } // if xlen

	    if (!prune && ylen > 1) {
	      for (vector<int>::iterator i=ys->symbol_set.begin(); 
		   i!=ys->symbol_set.end(); i++) {
		unsigned int item = *i;
		unsigned int zprev[zlen];
		remove_copy(&z[0], &z[zlen], &zprev[0], item);
		unsigned int key;
		int count;
		if (!freq_prev.find_one_subset(&zprev[0], key, count)) {
		  pruned++;
		  prune = true;
		  break;
		}
	      }
	    } // if ylen

	    if (!prune) {
	      candidates.add_itemset(z, 0);
	    }

	  } // ys !=
	  
	} // xs !=
      } // for xlen

    } // else

    

    cout << candidates.get_num_itemsets()  << " candidates" << endl;

    if (candidates.get_num_itemsets()==0)
      break;


    freq_prev.reset(k);

//     freq_prev.initDA(num_freq_items);    
//     candidates.init_prefix_table(num_freq_items);
//     candidates.initDA(num_freq_items);

    freq_prev.initDA(max_item+1);    
    candidates.init_prefix_table(max_item+1);
    candidates.initDA(max_item+1);

//     Transaction_Set::Scanner* scanner = db.gen_scanner();

//     KDCI::dci_transaction<unsigned int> t(max_trans_len, k, num_freq_items);
    KDCI::dci_transaction<unsigned int> t(max_trans_len, k, max_item+1);


    // do a pass over transaction set
    unsigned int *cand, *cand_subset;
    unsigned int *CACHE_items;
    unsigned int key_pair[2];
    unsigned int count_pair[2];
    KDCI::DCI_statistics stats;
    stats.reset_stats();
    dci_dataset.reset_prune_mask();
    dci_dataset.init_cache(k);

    cand = new unsigned int[k];
    cand_subset = new unsigned int[k-1];
    CACHE_items = new unsigned int[k];
#ifdef DEBUG
    //cout << "counters.get_m1()=" << counters.get_m1() << endl;
#endif
    CACHE_items[0] = num_freq_items - 1; // init CACHE - surely different !!!
    
//     int prefix_len;
//     for (prefix_len = start; prefix_len < iter-1; prefix_len++) {
//       if (cand[prefix_len] != CACHE_items[prefix_len])
// 	break;
//     }
    int prefix_len=0;

    // find frequents and output them
    int count; // T1
    int  num_freq=0;
    unsigned int *v, *v_remapped; // T
    v = new unsigned int[k];
    v_remapped = new unsigned int[k];
    int ind;
    unsigned int tmp_c; // T1
    candidates.init_read_itemsets();
    while ((ind=candidates.read_next_itemset(v, tmp_c)) != -1) {
      for (int i=0; i<k; i++)
	v_remapped[i] = item_map[v[i]];
      dci_dataset.candidate_is_frequent_compact
	(v_remapped, 
	 prefix_len, k, 
	 (int) support, 
	 count, 
	 stats);

      
      if (count >= (unsigned int) support) {
	num_freq++;

// 	if (is_remapped) {
// 	  for (int i=0; i<iter; i++) 
// 	    v_remapped[i] = (T) counters.map[v[i]]; // re-map items
// 	  next_freq.add_itemset(v_remapped, count);
// 	}
// 	else
	freq_prev.add_itemset(v, count);
#ifdef DEBUG
      cout << "count for " << v[0] <<" and "  
	   << v[1] << " is " << count << endl;
#endif
      }
    }
    delete [] v;
    delete [] v_remapped;

    cout << num_freq << " frequent itemsets discovered" << endl;

    k++;

  } // while 1


  out.close();
  return 0;
};


#include "Vertical_DB.hxx"
#include "Tid_List.hxx"

Vertical_DB::Vertical_DB() 
: greater_freq(my_count), count(my_count), num_txn(0)
{
}

Vertical_DB::Vertical_DB(const set<int> &f, const vector<int> & c, 
			 int num_items, int _num_txn, Transaction_Set& ts)
  : greater_freq(c), count(c), num_txn(_num_txn)
{
  copy(f.begin(), f.end(), back_inserter(F));
  // sort by decreasing frequency
  sort(F.begin(), F.end(), greater_freq);
#ifdef DEBUG
  nlog << "count=" << count << endl;
  nlog << "F=" << F << endl;
#endif

  // make backwards map
  map_freq_item.resize(F.size(), -1);
  map_item_freq.resize(num_items, -1);
  int freq = 0;
  for (vector<int>::iterator i=F.begin(); i!=F.end(); i++) {
    map_freq_item[freq] = *i;
    map_item_freq[*i] = freq;
    freq++;
  }

  tidlists.resize(F.size());
  for (unsigned int i=0; i<F.size(); i++) {
    tidlists[i].reset(num_txn, count[freq_to_item(i)]);
  }

  Transaction_Set::Scanner* scanner = ts.gen_scanner();
  int txn_id = 0;
  while (!scanner->end()) {
    Transaction t_i;
    if (scanner->read(t_i)) {
      insert(t_i, txn_id);
    }
    txn_id++;
  }
  delete scanner;

}

// merge two vertical dbs
// TODO: copy tidlists by ref
Vertical_DB::Vertical_DB(Vertical_DB& db1, Vertical_DB& db2)
  : greater_freq(my_count), count(my_count)
{
  assert(db1.num_txn==db2.num_txn);
  num_txn = db1.num_txn;
  int num_items = max(db1.count.size(), db2.count.size());
  my_count.resize(num_items, 0);
  for (unsigned int i=0; i<db1.count.size(); i++) {
    if (db1.count[i] > 0)
      my_count[i] = db1.count[i];
  }
  for (unsigned int i=0; i<db2.count.size(); i++) {
    if (db2.count[i] > 0)
      my_count[i] = db2.count[i];
  }
//   vector<int> Fa = db1.F;
//   sort(Fa.begin(), Fa.end());
//   vector<int> Fb = db2.F;
//   sort(Fb.begin(), Fb.end());
  //TODO: assume the F's are disjoint, should we remove this assumption?
  copy(db1.F.begin(), db1.F.end(), back_inserter(F));
  copy(db2.F.begin(), db2.F.end(), back_inserter(F));
  // Sort by decreasing frequency
  sort(F.begin(), F.end(), greater_freq);
  tidlists.resize(F.size());

  // make backwards map
  map_freq_item.resize(F.size(), -1);
  map_item_freq.resize(num_items, -1);
  int freq = 0;
  for (vector<int>::iterator i=F.begin(); i!=F.end(); i++) {
    map_freq_item[freq] = *i;
    map_item_freq[*i] = freq;
    freq++;
  }

  for (vector<int>::iterator it=db1.F.begin(); it!=db1.F.end(); it++) {
    int item = *it;
    my_count[item] = db1.count[item];
    tidlists[map_item_freq[item]] = db1.tidlists[db1.map_item_freq[item]];
  }

  for (vector<int>::iterator it=db2.F.begin(); it!=db2.F.end(); it++) {
    int item = *it;
    my_count[item] = db2.count[item];
    tidlists[map_item_freq[item]] = db2.tidlists[db2.map_item_freq[item]];
  }

#ifdef DEBUG
  nlog << "merged count = " << count << endl; 
#endif
  
}

void Vertical_DB::clear() {
  tidlists.resize(0);
}


// TODO: in place merge: incomplete
void Vertical_DB::merge(Vertical_DB& other)
{
  assert(0);
//   assert(num_txn==other.num_txn);
//   copy(other.F.begin(), other.F.end(), back_inserter(F));
//   // sort by decreasing frequency
//   sort(F.begin(), F.end(), greater_freq);
//   int num_items = max(count.size(), other.count.size());
//   count = my_count;
//   my_count.resize(num_items, 0);
//   tidlists.resize(F.size());
//   for (vector<int>::iterator it=other.F.begin(); it!=other.F.end(); it++) {
//     int item = *it;
//     my_count[item] = other.count[item];
//   }
//   nlog << "merged count = " << count << endl; 
  
//   // make backwards map
//   map_freq_item.resize(F.size(), -1);
//   map_item_freq.resize(num_items, -1);
//   int freq = 0;
//   for (vector<int>::iterator i=F.begin(); i!=F.end(); i++) {
//     map_freq_item[freq] = *i;
//     map_item_freq[*i] = freq;
//     freq++;
//   }

  
}

void Vertical_DB::insert(const vector<int>& txn, int txn_id)
{
#ifdef DEBUG
  //cout << "inserting txn " << txn_id << " = " << txn << endl;
#endif
  for (vector<int>::const_iterator it=txn.begin(); it!=txn.end(); it++) {
    unsigned int item = *it;
    if (item < map_item_freq.size()) {
      int freq = map_item_freq[*it];
      if (freq!=-1) { // if frequent item
	tidlists[freq].add_transaction(txn_id);
      }
    }
  }
}



// number of times itemset occurs in dataset
int Vertical_DB::frequency(const vector<int>& itemset)
{
  /*
  // compare with cache, can be extended to a larger cache easily
  vector<int> cache_intersection[2];
  int cost[2];
  for (int i=0; i<2; i++) {
    set_intersection(itemset.begin(), itemset.end(),
		     cache_itemset[i].begin(), cache_itemset[i].end(),
		     back_inserter(cache_intersection[i]));
    cost[i] = itemset.size() + cache_itemset[i].size() - diff[i].size();
  }
  int *min_cost = min_element(&cost[0], &cost[2]);
  if (*min_cost < itemset.size()) {
    // if there is any point in using the cache
    int ix = min_diff_size-&cost[0];
    
  }
  */

  //TODO: decide which implementation to use
  Tid_List tid_list;
  tid_list.reset(num_txn, count[itemset[0]]);
  tidlist(itemset, tid_list);
  return tid_list.count();
}

void Vertical_DB::tidlist(const vector<int>& itemset, Tid_List& intersection)
{
  if (itemset.size() == 1)
    intersection = item(itemset[0]);
  else if (itemset.size() >=2) {
    int freq0 = map_item_freq[itemset[0]];
    int freq1 = map_item_freq[itemset[1]];
    
    //intersection = tidlists[freq0] & tidlists[freq1];
    tidlists[freq0].intersect(tidlists[freq1], intersection);
    for (unsigned int i=2; i<itemset.size(); i++) {
      intersection.intersect(tidlists[map_item_freq[itemset[i]]]);
    }
    //cache_itemset[0] = itemset; // mru
    //cache_tidlist[0] = tidlist;
  }
}

// sort items in a transaction, first in order of decreasing frequency
// and then sort blocks with the same frequency in order of increasing item
void Vertical_DB::sort_decreasing_freq(vector<int>& A) const
{
  // 1. sort in order of decreasing frequency!
  count_quiksort(A, 0, A.size()-1);

  // 2. scan for blocks with same count
  int i = 0;
  int n = A.size()-1;		// last index
  while (i < n) {
    int x = A[i];
    int j = i ;
    // find the last element with same count as x
    do {
      j++;
    } while (j<=n && count[A[j]] == count[x]);
    j--;
    if (j>i)
      quiksort(A,i,j);		// order in increasing indices!
    i++;
  }

}


// quick sort as we know it, sort in increasing order
void Vertical_DB::quiksort(vector<int>& A, int p, int r)
{
  if (p < r) {
    int q = partition(A, p, r);
    quiksort(A, p, q);
    quiksort(A, q+1, r);
  }
  
}

int Vertical_DB::partition(vector<int>& A, int p, int r)
{
  int x = A[p];
  int i = p - 1;
  int j = r + 1;
  while (1) {
    do {
      j = j - 1;
    } while (A[j] > x);
    do {
      i = i + 1;
    } while (A[i] < x);
    if (i < j)
      swap(A[i], A[j]);
    else
      return j;
  }
}

// sort according to *decreasing* count of items in array A
void Vertical_DB::count_quiksort(vector<int>& A, int p, int r) const
{
  if (p < r) {
    int q = count_partition(A, p, r);
    count_quiksort(A, p, q);
    count_quiksort(A, q+1, r);
  }
}

int Vertical_DB::count_partition(vector<int>& A, int p, int r) const
{
  int x = A[p];
  int i = p - 1;
  int j = r + 1;
  while (1) {
    do {
      j = j - 1;
    } while (count[A[j]] < count[x]);
    do {
      i = i + 1;
    } while (count[A[i]] > count[x]);
    if (i<j)
      swap(A[i], A[j]);
    else
      return j;
  }
}

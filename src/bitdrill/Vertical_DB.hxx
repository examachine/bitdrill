// Vertical DB representation

#ifndef Vertical_DB_Interface
#define Vertical_DB_Interface

#include <iostream>

#include "generic/General.hxx"
#include "transaction-db/Transaction_Set.hxx"

#include "Tid_List.hxx"

class Vertical_DB
{
public:
  Vertical_DB();

  Vertical_DB(const set<int> &f, const vector<int> & c, 
	      int num_items, int num_txn, Transaction_Set& ts);

  Vertical_DB(Vertical_DB& db1, Vertical_DB& db2);

  ~Vertical_DB() {}

  void clear();

  void merge(Vertical_DB& other);

  int frequency(const vector<int>& itemset);

  void tidlist(const vector<int>& itemset, Tid_List& intersection);

  const Tid_List& item(int item) const {
    return tidlists[item_to_freq(item)];
  }

  // directly give frequent item index
  const Tid_List& item_freq(int freq) {
#ifdef DEBUG
    assert(0<=freq && freq<(int)tidlists.size());
#endif
    return tidlists[freq];
  }

  int freq_to_item(int freq) const {
#ifdef DEBUG
    assert(0<=freq && freq<(int)map_freq_item.size());
#endif
    return map_freq_item[freq];
  }
  int item_to_freq(int item) const {
#ifdef DEBUG
    assert(0<=item && item<(int)map_item_freq.size());
    assert(map_item_freq[item] != -1);
#endif
    return map_item_freq[item];
  }

  int num_freq() const {
    return F.size();
  }

  // decreasing freq order (stable, secondary order: increasing key) 
  void reorder(vector<int>& itemset) const {
    sort_decreasing_freq(itemset);
  }

  // compare items according to their counts
  struct Greater_Freq
  {
    Greater_Freq(const vector<int>& c) : count(c) {}

    bool operator() (int i, int j) {
      if (count[i] == count[j])
	return i < j; // then index order
      else return (count[i] > count[j]);
    }
    const vector<int> & count;
  };
  Greater_Freq greater_freq;
  vector<int> F;
  int get_num_txn() const {return num_txn;}
  const vector<int> &count;

  void print(ostream& out) const {
    out << "Vertical DB:" << endl;
    for (int i=0; i<num_freq(); i++)
      out << "  tidlist[" << freq_to_item(i) << "]=" << tidlists[i] << endl;
  }

protected:
  vector<Tid_List> tidlists;

//   static const int cache_size = 2;
//   vector<int> cache_itemset[2];
//   Tid_List* cache_tidlist[2];

  vector<int> my_count;
  int num_txn;

  vector<int> map_freq_item; // convert frequent item to real item
  vector<int> map_item_freq; // and reverse map

  void insert(const vector<int>& txn, int txn_id);

  void sort_decreasing_freq(vector<int>& t_i) const;
  static void quiksort(vector<int>& A, int p, int r);
  static int partition(vector<int>& A, int p, int r);
  void count_quiksort(vector<int>& A, int p, int r) const;
  int count_partition(vector<int>& A, int p, int r) const;

};


inline ostream& operator<<(ostream& out, const Vertical_DB& db){
  db.print(out);
  return out;
}

#endif

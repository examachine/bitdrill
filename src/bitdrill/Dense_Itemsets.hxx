#include <generic/General.hxx>
#include <boost/dynamic_bitset.hpp>
//#include <slist>
#include "Vertical_DB.hxx"



class Dense_Itemsets
{
public:

  typedef list< boost::dynamic_bitset<>* > Itemset_List;
  typedef vector< boost::dynamic_bitset<>* > Itemset_Vector;

  Dense_Itemsets(const Vertical_DB &v) 
    : vertical_db(v) {}

  ~Dense_Itemsets() {
    for (Itemset_List::iterator i=itemset_list.begin();
	 i!=itemset_list.end();  i++) {
      delete *i;
    }
  }

  const Itemset_Vector &ref_itemset_vec() {
    return itemset_vec;
  }

  bool empty() {
    return itemset_vec.empty();
  }

  void insert(const boost::dynamic_bitset<> &itemset) {
    // copy on the heap for ease... TODO: avoid copying?
    itemset_list.push_back(new boost::dynamic_bitset<>(itemset));
  }

  void sort() {
    itemset_vec.resize(itemset_list.size());
    partial_sort_copy(itemset_list.begin(), itemset_list.end(),
		      itemset_vec.begin(), itemset_vec.end(),
		      bitstring_order);
  }

  // consider adding const
  bool find(boost::dynamic_bitset<> * itemset) {
    return binary_search(itemset_vec.begin(), itemset_vec.end(),
			 itemset, bitstring_order);
  }

  list< boost::dynamic_bitset<>* > itemset_list;
  vector< boost::dynamic_bitset<>* > itemset_vec;
  const Vertical_DB& vertical_db;
  struct Bitstring_Order {
    // less than op
//     bool operator() (const boost::dynamic_bitset<> *a,  
// 		     const boost::dynamic_bitset<> *b) {
//       int len = a->size();
//       for (int i=0; i<(int)a->size(); i++) {
// 	if (a->test(i) && !b->test(i))
// 	  return false;
// 	else if (!a->test(i) && b->test(i))
// 	  return true;
//       }
//       return false; // because they are equal
//     }
    
    bool operator() (boost::dynamic_bitset<>* a,  
		     boost::dynamic_bitset<>* b) {
      int len = a->size();
      for (int i=0; i<len; i++) {
	if (a->test(i) && !b->test(i))
	  return false;
	else if (!a->test(i) && b->test(i))
	  return true;
      }
      return false; // because they are equal
    }
  } bitstring_order;
};

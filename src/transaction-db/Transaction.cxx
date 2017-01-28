//
//
// C++ Implementation for module: Transaction
//
// Description: 
//
//
// Author: exa
//
// Copyright: See COPYING file that came with this distribution
//
//

#include "Transaction.hxx"
#include "Transaction_Set.hxx"

bool Transaction::read_unsorted(istream& in, Transaction::File_Format format)
{
  if (in.eof()) {
    // trying to read empty stream
    // issue failure
    return false;
  }
  if (format==Transaction::binary) {
    if (!in.read((char*)&customer_id, sizeof(int))) {
      // db may end normally
      return false;
    }
    if (!in.read((char*)&transaction_id, sizeof(int))) {
      cerr << "Transaction:: cannot read transaction id" << endl;
      exit(-1);
      return false;
    }
    int num_items;
    if (!in.read((char*)&num_items, sizeof(int))) {
      cerr << "Transaction:: cannot read num_items" << endl;
      exit(-1);
      return false;
    }
    if (num_items==0) {
      return true;
    }
    if (in.eof()) {
      cerr << "Transaction:: at eof while reading items" << endl;
      exit(-1);
      return false;
    }
    int* items = new int[num_items];
    if (!in.read((char*)items, num_items * sizeof(int))) {
      cerr << "Transaction:: error occured while reading items" << endl;
      exit(-1);
      return false;
    }
    resize(num_items);
    copy(items, items+num_items, begin());
    delete items;
    return true;
  }
  else if (format==binary_short) {
    int num_items;
    if (!in.read((char*)&num_items, sizeof(int))) {
      return false;
    }
    if (num_items==0) {
      return true;
    }
    if (in.eof()) {
      cerr << "Transaction:: at eof while reading items" << endl;
      exit(-1);
      return false;
    }
    int* items = new int[num_items];
    if (!in.read((char*)items, num_items * sizeof(int))) {
      cerr << "Transaction:: error occured while reading items" << endl;
      exit(-1);
      return false;
    }
    resize(num_items);
    copy(items, items+num_items, begin());
    delete items;
    return true;
  }
  else if (format==Transaction::ascii) {
    clear();			// clear item list
    //int tid, cid;
    //in >> tid >> cid;
    // read flat ascii file format -- efficient code by Bart Goethals
    char c;
    bool read=false;
    // read list of items
    while (!read && !in.eof()) { // read lines until we find a non-empty txn
      do {			// read a line of input (until newline or EOF)
	int item=0, pos=0;
	c = in.get();
	while((c >= '0') && (c <= '9')) {
	  item *=10;
	  item += int(c)-int('0');
	  c = in.get();
	  pos++;
	}
	if (pos) push_back(item);
      } while (c != '\n' && !in.eof()); // read a line
      read = size()!=0; // could we read a txn?
    }
    return read;
  }
  else {
    cerr << "*** ERROR: txn read: file format not implemented" << endl;
    exit(-1);
    return false;
  }
};

using namespace Utility;

void Transaction::write(ostream& out, Transaction::File_Format format)
{
  if (format==binary) {
    out.write((char*)&customer_id, sizeof(int));
    out.write((char*)&transaction_id, sizeof(int));
  }
  if (format==binary || format==binary_short) {
    int num_items = size();
    out.write((char*)&num_items, sizeof(int));
    out.write((char*)&front(), num_items * sizeof(int));
  }
  else if (format==ascii) {
    for (vector<int>::iterator i=begin(); i!=end(); i++) {
      out << *i << " ";
    }
    out << endl;
  }
  else {
    cerr << "*** ERROR: txn write: file format not implemented" << endl;
    exit(-1);
  }
};


void Transaction::encode_step(vector<int>& msg)
{
  //msg.push_back(customer_id);
  //msg.push_back(transaction_id);
  msg.push_back(size());
  copy(begin(), end(), back_inserter(msg));
}

void Transaction::decode(const vector<int>& vec)
{
  //customer_id = *(it++);
  //transaction_id = *(it++);
  int num_items = vec[0];
  resize(num_items);
#ifdef DEBUG2
  nlog << "SIZE " << size() << endl << flush;
  nlog << "NUM ITEMS " << vec[0] << endl << flush;
  nlog << "SIZE " << size() << endl << flush;
#endif
  copy(&vec[1], &vec[1+num_items], begin()); 
}


vector<int>::iterator Transaction::decode_step(vector<int>::iterator it)
{
  //customer_id = *(it++);
  //transaction_id = *(it++);
#ifdef DEBUG2
  nlog << "SIZE" << size() << endl << flush;
#endif  
  int num_items = *(it++);
  vector<int>::iterator last = it + num_items;
  resize(num_items);
  copy(it, last, begin()); 
  return last;
}

#include <algorithm>

void Transaction::make_set()
{
  // gives us a nice set view
  sort(begin(), end());
}

// slow pruning routine
void Transaction::prune_not_in(const set<int>& F)
{
  sort(begin(), end());
  not_in f(F);
  erase(remove_if(begin(), end(), f), end());
}

//
//
// C++ Interface for module: Transaction
//
// Description:
//
//
// Author: exa
//
// Copyright: See COPYING file that came with this distribution
//
//

#ifndef Transaction_Interface
#define Transaction_Interface

#include "generic/General.hxx"

class not_in {
public:
  not_in(const set<int> &x) : large(x) {}
  bool operator() (int x) {
    return large.find(x) == large.end();
  }
  const set<int>& large;
};

class Transaction : public vector<int> {
public:
  enum File_Format { ascii = 0, binary, binary_short, plain };
  typedef vector<int> Base;

  int customer_id;
  int transaction_id;

  Transaction() : customer_id(-1), transaction_id(-1) {}
  Transaction(int* beg, int* end)
    : Base(beg, end), customer_id(-1), transaction_id(-1) {}
  Transaction(const Base& v) : Base(v), customer_id(-1), transaction_id(-1) {}

  inline bool read(istream& in, File_Format format = binary) {
    if (read_unsorted(in,format)) {
      make_set();
      return true;
    }
    else
      return false;
  }
  bool read_unsorted(istream& in, File_Format format = binary);
  void write(ostream& out, File_Format format = binary);
  void encode_step(vector<int>& msg);
  void decode(const vector<int>& msg);
  vector<int>::iterator decode_step(vector<int>::iterator beg);
  void make_set();
//   void prune_not_in(const vector<int>& F);
  void prune_not_in(const set<int>& F);
};

#endif

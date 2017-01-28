//
//
// C++ Interface for module: Transaction_Set
//
// Description:
//
//
// Author: exa
//
// Copyright: See COPYING file that came with this distribution
//
//

#ifndef Transaction_Set_Interface
#define Transaction_Set_Interface

#include "generic/General.hxx"
#include "Transaction.hxx"

// implemented as a list of transactions
//TODO: base used to be private, made public for debugging purposes
class Transaction_Set
: public list<Transaction>
{
public:
  typedef list<Transaction> Base;
  enum Representation { file, encoding, plain};

  Transaction_Set(Representation rep = plain, 
		  Transaction::File_Format format = Transaction::ascii);
  Transaction_Set(const string& fn,
		  Transaction::File_Format format = Transaction::ascii);

  Transaction_Set(vector<int>* msg);

  void clear_memory() {
    Base::clear();
  }

  class Scanner {
  public:
    virtual ~Scanner() {}
    virtual bool read(Transaction& t_i) = 0;
    virtual bool end() = 0;
  };

  class File_Scanner : public Scanner {
  public:
    File_Scanner(Transaction_Set& ts);
    virtual ~File_Scanner() {}
    bool read(Transaction& t_i);
    bool end();
    Transaction_Set& ts;
    ifstream in;
    bool file_error;
  };
  friend class File_Scanner;

  class Buffer_Scanner : public Scanner  {
  public:
    Buffer_Scanner(Transaction_Set& ts);
    virtual ~Buffer_Scanner() {}
    bool read(Transaction& t_i);
    bool end();
    vector<int>* buffer;
    vector<int>::iterator it;
  };
  friend class Buffer_Scanner;

  class Plain_Scanner : public Scanner {
  public:
    Plain_Scanner(Transaction_Set& ts);
    virtual ~Plain_Scanner() {}
    bool read(Transaction& t_i);
    bool end();
    Transaction_Set& ts;
    Transaction_Set::iterator it;
  };
  friend class Plain_Scanner;

  Scanner* gen_scanner();

  void add(const Transaction& t_i) {
    push_back(t_i);
  }

  Transaction& new_transaction() {
    add(Transaction());
    Transaction& t_i = back();
    return t_i;
  }

  void clear();

  int size() { return int(Base::size()); }

  // vector's contents is the message for now
  void encode(vector<int>& msg);
  void decode(vector<int>& msg);

  bool get_representation() { return representation; }
  const string& get_filename() { return filename; }

private:
  string filename;
  Representation representation;
  Transaction::File_Format format;
  vector<int>* buffer;

// static routines

public:
  // count 1 items in a transaction db
  // int count_items(vector<int>& count);
  static int count_items(istream& in, vector<int>& count);

  //   static void count_2_items(istream& in, Graph& G, const set<int>& L);

  // split randomly a transaction set
  static void split(istream& in, int k, const string & outprefix,
		    Transaction::File_Format);
  static void par_split(istream& in, const string& outprefix,
			Transaction::File_Format);

// old routines

  // this surely doesn't fit in the memory
  void read(istream& in);
  void write(ostream& out);
  void large_items(set<int>& L, int epsilon) const;
  void prune_not_in(set<int>& items);
  void prune_small_items(int);
  void stats();
};

#endif

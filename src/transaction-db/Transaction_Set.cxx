//
//
// C++ Implementation for module: Transaction_Set
//
// Description: Representation for a transaction database
//
//
// Author: exa
//
// Copyright: See COPYING file that came with this distribution
//
//

#include "Transaction_Set.hxx"

using namespace Utility;

// empty transaction set constructor
Transaction_Set::Transaction_Set(Representation rep, Transaction::File_Format _format)
  : representation(rep), format(_format)
{
}

// file representation constructor
Transaction_Set::Transaction_Set(const string& fn,
				 Transaction::File_Format _format)
  : filename(fn), representation(file), format(_format)
{
}

// message encoding constructor
Transaction_Set::Transaction_Set(vector<int>* buf)
  : representation(encoding), buffer(buf)
{
}

Transaction_Set::Scanner* Transaction_Set::gen_scanner()
{
  switch (representation) {
  case file:
    return new File_Scanner(*this);
  case encoding:
    return new Buffer_Scanner(*this);
  case plain:
    return new Plain_Scanner(*this);
  }
  assert(false);
  return 0;
}

// scanner class does some real work
// simplistic implementation without caching
// assume that external file specified in ts exists
Transaction_Set::File_Scanner::File_Scanner(Transaction_Set& _ts)
  : ts(_ts), file_error(false)
{
  if (ts.format==Transaction::binary || ts.format==Transaction::binary_short)
    in.open(ts.filename.c_str(), ios::binary);
  else if (ts.format==Transaction::ascii)
    in.open(ts.filename.c_str());
  if (!in) {
    cerr << "error opening file " << ts.filename << endl;
    nlog << "error opening file " << ts.filename << endl;
    exit(-1);
  }
}

// scan next transaction
bool Transaction_Set::File_Scanner::read(Transaction& t_i)
{
  if (t_i.read(in, ts.format)) {
    return true;
  }
  else {
    if (!in.eof())
      cerr << endl
	   << "FILE ERROR: end of file not met after a null transaction"
	   << endl;
    file_error=true;
    return false;
  }
}

bool Transaction_Set::File_Scanner::end()
{
  bool end = in.eof() || file_error;
  return end;
}

Transaction_Set::Buffer_Scanner::Buffer_Scanner(Transaction_Set& ts)
{
  buffer = ts.buffer;
  it = buffer->begin();
}

// scan next transaction
bool Transaction_Set::Buffer_Scanner::read(Transaction& t_i)
{
  bool ret = !end();
  if (ret) 
    it = t_i.decode_step(it);
  return ret;
}

bool Transaction_Set::Buffer_Scanner::end()
{
  return it == buffer->end();
}

Transaction_Set::Plain_Scanner::Plain_Scanner(Transaction_Set& _ts)
  : ts(_ts)
{
  it = ts.begin();
}

// scan next transaction
bool Transaction_Set::Plain_Scanner::read(Transaction& t_i)
{
  bool ret = !end();
  if (ret) {
    t_i = *it;
    it++;
  }
  return ret;
}

bool Transaction_Set::Plain_Scanner::end()
{
  return it == ts.end();
}

void Transaction_Set::encode(vector<int>& msg)
{
  for (iterator i=begin(); i!=end(); i++) {
    Transaction &t_i = *i;
    t_i.encode_step(msg);
  }
}

void Transaction_Set::decode(vector<int>& msg)
{
  if (msg.size()==0)
    return;
  vector<int>::iterator msg_it = msg.begin();
  while (msg_it < msg.end()) {
    Transaction& t_i = new_transaction();
    msg_it = t_i.decode_step(msg_it);
  }
}

void Transaction_Set::read(istream& in)
{
  while (!in.eof()) {
    iterator i = insert(end(), Transaction());
    if (!i->read(in, format))
      erase(i);
  }
//   cout << "Transaction_Set: read " << size()
//        << " transactions" << endl;
}

void Transaction_Set::write(ostream& out)
{
  for (iterator i=begin(); i!=end(); i++) {
    i->write(out, format);
  }
//   cout << "Transaction_Set: wrote " << size() << " transactions" << endl;
}

class Less_Than_E
{
public:
  Less_Than_E(int _e): e(_e) {}
  Less_Than_E() {}
  bool operator()(pair<const int, int> x) { return x.second<e; }
  int e;
};

void Transaction_Set::large_items(set<int>& L, int epsilon) const
{
  typedef map<int,int> freq_map;
  freq_map freq;
  for (const_iterator i=begin(); i!=end(); i++) {
    const Transaction& t_i = *i;
//     cout << t_i << endl;
    for (Transaction::const_iterator itemi=t_i.begin();
	 itemi!=t_i.end();itemi++) {
      int item = *itemi;
      if (freq.find(item)!=freq.end())
	freq[item] = freq[item] + 1;
      else
	freq[item] = 0;
    }
  }
  //cout << "freq=" << freq << endl;
  for (map<int,int>::iterator j=freq.begin(); j!=freq.end(); j++) {
    if (j->second>=epsilon)
      L.insert(j->first);
  }
    //Less_Than_E less_than(epsilon);
  //remove_if(freq.begin(), freq.end(), less_than);
}

void Transaction_Set::prune_not_in(set<int>& F)
{
  for (iterator i=begin(); i!=end(); i++) {
    Transaction& t_i = *i;
//     t_i.make_set(); // maintain as set
    t_i.prune_not_in(F);
  }
}

void Transaction_Set::prune_small_items(int epsilon)
{
  set<int> L;
  large_items(L, epsilon);
  //cout << "L=" << L << endl;
//   cout << L.size() << " large items" << endl;
  prune_not_in(L);
}

void Transaction_Set::stats()
{
  int freq=0;
  for (const_iterator i=begin(); i!=end(); i++) {
    const Transaction& t_i = *i;
    freq += t_i.size();
  }
  nlog << "total number of items " << freq << endl;
}

void Transaction_Set::split(istream& in, int k, const string& outprefix, Transaction::File_Format format)
{
  int db = 0;
  vector<ofstream*> out(k); // a vector of k ofstream pointers
  for (int i=0; i<k; i++) { // open a file for each part
    string fn;
    ostringstream os(fn);
    os << outprefix << '.' << i;
    out[i] = new ofstream(os.str().c_str(), ios::binary);
  }
  while (!in.eof()) {
    Transaction t_i;
    if (t_i.read(in, format)) {
      // write it in db
     t_i.write(*out[db]);
#ifndef NDEBUG
     //cout << "wrote txn " << t_i << " to db " << db << endl;
#endif
    }
    db = (db + 1) % k;
  }
  for (int i=0; i<k; i++) {
    ofstream* o = out[i];
    delete o;
  }
//   cout << "split database into " << k << endl;
}

void Transaction_Set::clear()
{
  erase(begin(), end());
}


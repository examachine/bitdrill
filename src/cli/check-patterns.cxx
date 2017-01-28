//
// C++ Implementation for module: check-patterns
//
// Description: Check if a pattern out file has correct
// patterns
//
// Author: exa
//
// Copyright: See COPYING file that comes with this distribution
//
//

#include "generic/General.hxx"
#include "transaction-db/Transaction_Set.hxx"
#include "fp-tree/Pattern.hxx"

int main(int argc, char *argv[])
{
    cout << "check-patterns -- verify pattern output" << endl;

    if (argc!=3) {
      cout << "usage: check-patterns <transaction-db> <patterns>" << endl;
      exit(-1);
    }

    string pat_name(argv[2]);
    cout << "reading patterns from " << pat_name << endl;

    // read patterns
    ifstream pattern_input(pat_name.c_str());
    typedef pair<Pattern,int> Pattern_Info;
    list< Pattern_Info > patterns;
    while (!pattern_input.eof()) {
      Pattern p;
      if (p.read(pattern_input)) {
	patterns.push_back(Pattern_Info(p,0));
      }
    }
    cout << patterns.size() << " patterns read" << endl << endl;

    string db_name(argv[1]);
    cout << "checking patterns in transaction set " << db_name << endl;
    // do a pass over local db.
    ifstream in(db_name.c_str());
    int num_trans = 0;
    int max_item = 0;
    while (!in.eof()) {
      Transaction t_i;
      if (t_i.read(in, Transaction::ascii)) {
	num_trans++;
	t_i.make_set();
	for (Transaction::iterator i=t_i.begin(); i!=t_i.end(); i++) {
	  if (*i>max_item) max_item=*i;
	}
	// could be much faster. just doing it the easy way
	for (list<Pattern_Info>::iterator p=patterns.begin();
	     p!=patterns.end();p++) {
	  // all patterns and transactions are sets (sorted in asc. order)
	  if (includes(t_i.begin(), t_i.end(),
		       p->first.symbol_set.begin(),p->first.symbol_set.end()
		       )) {
	    p->second++;
	  }
	}
      }
      else {
	break;
      }
    }
    cout << num_trans << " transactions read" << endl;
    cout << "maximum item is " << max_item << endl;
    cout << "transaction set valid" << endl << endl;

    cout << "verifying patterns" << endl;
    int num_incorrect = 0;
    for (list<Pattern_Info>::iterator p=patterns.begin();
	 p!=patterns.end();p++) {
      if (p->first.count!=p->second) {
	cout << "pattern " << p->first << " mismatches actual frequency "
	     << p->second << endl;
	num_incorrect++;;
      }
    }
    if (num_incorrect == 0) {
      cout << "all patterns correct" << endl;
      return 0;
    }
    else {
      cout << "ERROR: " << num_incorrect
	   << " incorrect patterns detected" << endl;
      return 1;
    }
};

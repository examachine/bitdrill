//
//
// C++ Implementation for module: Count
//
// Description: 
// Count freq 2-items, removed parallel code
//
// Author: exa
//
// Copyright: See COPYING file that came with this distribution
//
//

#include "transaction-db/Transaction.hxx"
#include "Count.hxx"
#include "transaction-db/Transaction_Set.hxx"
#include "graph/Vertex_Induced_Subgraph.hxx"

#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#ifndef __USE_ISOC99
#error "ISOC99 not available!"
#endif

#ifdef USE_KDCI
#include "kdci.hxx"
#endif

string TS_Counter::temp_filename_prefix("/tmp/noclique.tmp");

TS_Counter::TS_Counter(Transaction_Set& ts, bool _serial)
  : num_items(0), serial(true), 
    transaction_set(ts)
{
}

TS_Counter::TS_Counter(const string& prefix, bool _serial)
  : num_items(0), serial(true), db_prefix(prefix), 
    transaction_set_ext(prefix,	::Transaction::ascii),
    transaction_set(transaction_set_ext)
{
}

TS_Counter::~TS_Counter()
{
  struct stat buf;
  nlog << "removing " << temp_filename << endl;
  if (stat(temp_filename.c_str(), &buf)==0)
    unlink(temp_filename.c_str());
}

void TS_Counter::count_items_relative(float sup, int _level)
{
  real_support = sup;
  relative_support = true;
  level = _level;
  if (!serial)
    count_items();
  else
    count_items_serial();
}

void TS_Counter::count_items_absolute(int sup, int _level)
{
  nlog << "count_items_absolute" << endl;
  support = sup;
  relative_support = false;
  level = _level;
  if (!serial)
    count_items();
  else
    count_items_serial();
}

// count items serial
void TS_Counter::count_items_serial()
{
  nlog << "count_items_serial" << endl << flush;
  count_1_items_local(count);
  large_items();
  if (level==2) {
    if (F.size()>0) {
      count_2_items_local();
      prune_2_items(two_items);
    }
    else
      nlog << "*** WARNING got no large items" << endl << flush;
  }
}

// count items parallel
void TS_Counter::count_items()
{
  cerr << "FATAL ERROR" << endl;
  exit(6);
}

// serial version
void TS_Counter::count_1_items_serial()
{
  Wall_Time t_start;
  cout << "[pass 1] count large items";
  count_1_items_local(count);
  if (relative_support)
    support = (int) trunc(num_transactions * real_support);
  large_items();
  Wall_Time t_end;
  cout << " (" << t_end-t_start << " sec)" << std::endl;
}

void TS_Counter::count_1_items_local(vector<int>& count)
{
  Wall_Time t_start;

  // calculate temporary filename
  ostringstream os;
  os << temp_filename_prefix << "-" << getpid();
  temp_filename = os.str();
  nlog << "temporary binary file: " << temp_filename << endl;
  ofstream temp_ts ( temp_filename.c_str() , ios::binary);

#ifdef USE_KDCI
  TEMPORARY_DATASET = temp_filename.c_str();
  write_output=false;
  int min_count;
  if (relative_support)
    min_count = 1;
  else
    min_count = support;
  dci_counters = kdci_first_scan(transaction_set, min_count,
				 max_support,
				 max_trans_len);
  num_items = dci_counters->get_m();
  num_transactions = dci_counters->get_num_of_trans();
  count.resize(num_items);
  for (vector<dci_item>::const_iterator i=dci_counters->get_counts()->begin();
       i!=dci_counters->get_counts()->end();i++) {
    count[i->id] = i->count;
  }
#else
  count.resize(0);		// make it 0
  //   ifstream ts(pardb_name(db_prefix).c_str(), ios::binary);
  //   num_transactions = Transaction_Set::count_items(ts, count);
  int num_trans = 0;
  // start scanning
  Transaction_Set::Scanner* scanner = transaction_set.gen_scanner();

  // do a pass over transaction set
  while (!scanner->end()) {
    Transaction t_i;
    if (scanner->read(t_i)) {
      num_trans++;
      for (Transaction::iterator i=t_i.begin(); i!=t_i.end(); i++) {
	int oldsize = count.size();
	int id = (*i);
	int nusize = id +1;
	if (oldsize < nusize) {
	  count.resize(nusize);
// 	  for (int j=oldsize; j<nusize; j++) {
// 	    count[j] = 0;
// 	  }
	}
// 	assert ( (unsigned int)*i < count.size() );
	count[*i]++;
      }
      if (t_i.size() >= 2)
	t_i.write(temp_ts, Transaction::binary_short);
    }
  }
  delete scanner;
  num_items = count.size();
  num_transactions=num_trans;
#endif

  nlog << "|I| = " << num_items  << endl << flush;
  nlog << "|T_local| = " << num_transactions  << endl << flush;

  Wall_Time t_end;
  nlog << "time for count_1_items_local " << t_end - t_start << endl;
}
 
void TS_Counter::large_items()
{
  for (int i=0; i<num_items; i++) {
    if (count[i] >= support) {
      F.insert(i);
    }
  }
  nlog << "|F| = " << F.size() << endl << flush;
#ifdef DEBUG
  nlog << "F = " << F  << endl;
#endif

}


void TS_Counter::count_2_items_local()
{
  Wall_Time t_start;
  // first prepare the graphs to hold info

  // resize graphs
  F2.resize(F.size());
  F2_load.resize(F.size());

  // initialize item map
  f_item.resize(F.size());
  item_f.resize(num_items);
  fill(item_f.begin(), item_f.end(), -100);
  int fix=0;			// frequent item index
  for (set<int>::iterator i=F.begin(); i!=F.end(); i++) {
    f_item[fix]=*i;
    item_f[*i] = fix;
    fix++;
  }

#ifdef DEBUG
  nlog << "f_item = " << f_item << endl;
  nlog << "item_f = " << item_f << endl;
#endif

  // write vertex weights of F2 graph
  for (set<int>::iterator i=F.begin(); i!=F.end(); i++) {
    int item = *i;
    F2.set_weight(item_f[item], count[item]); // set vertex weight in F2
  }

  nlog << "count local 2-items..." << flush;

#ifdef DEBUG
//   nlog << "F = " << F << endl;
#endif

  // allocate UT matrix
  two_items = new Undir_Mtx_Graph(F.size());

  Undir_Mtx_Graph& G = *two_items;

#ifdef USE_KDCI
  dci_counters->min_count = support; // set support

  //copy counts
  vector<dci_item>* dci_count = dci_counters->get_counts();
  dci_count->resize( count.size() );
  for (int i=0; i<num_items; i++) {
    (*dci_count)[i].id = i;
    (*dci_count)[i].count = count[i];
  }

  //  remap item identifiers to 1:m1
  //int first_freq = dci_counters->remap_items(false);

  // m1 is the number of items (singleton) 
  // that are found to be frequent
  int m1 = dci_counters->get_m1();

  if (!(max_support == 0 || m1 < 2)) {

    m1++; // we need one extra element to store the key-pattern flag (-1)

    dci_items& counters = *dci_counters;

    if (m1 < 256   &&    max_support < 256)
      only_second_scan<unsigned char, unsigned char>
	(max_trans_len, counters, G, item_f);
    else if (m1 < 256  && max_support < 256*256)
      only_second_scan<unsigned char, unsigned short int>
	(max_trans_len, counters, G, item_f);  
    else if (m1 < 256  && max_support >= 256*256)
      only_second_scan<unsigned char, unsigned int>
	(max_trans_len, counters, G, item_f);  
    else if (m1 < 256*256  && max_support < 256)
      only_second_scan<unsigned short int, unsigned char>
	(max_trans_len, counters, G, item_f);  
    else if (m1 >= 256*256  && max_support < 256)
      only_second_scan<unsigned int, unsigned char>
	(max_trans_len, counters, G, item_f);  
    else if (m1 < 256*256  && max_support < 256*256)
      only_second_scan<unsigned short int, unsigned short int>
	(max_trans_len, counters, G, item_f);
    else if (m1 < 256*256  && max_support >= 256*256)
      only_second_scan<unsigned short int, unsigned int>
	(max_trans_len, counters, G, item_f);
    else if (m1 >= 256*256 && max_support < 256*256)
      only_second_scan<unsigned int, unsigned short int>
	(max_trans_len, counters, G, item_f);
    else
      only_second_scan<unsigned int, unsigned int>
	(max_trans_len, counters, G, item_f);
  }
  

#else
  int num_transactions = 0;

  // start scanning
//   Transaction_Set::Scanner* scanner = transaction_set.gen_scanner();

  // use temporary binary file
  Transaction_Set ts(temp_filename, Transaction::binary_short);
  Transaction_Set::Scanner* scanner = ts.gen_scanner();
  // do a pass over ts
  while (!scanner->end()) {
    Transaction t_i;
    if (scanner->read(t_i)) {
      //       cout << t_i << endl;
      num_transactions++;

#ifdef DEBUG
      if (num_transactions % 10000 ==0) 
	nlog << "." << flush; 	//show progress
#endif

      //t_i.make_set(); // maintain as set
      t_i.prune_not_in(F);

      // opt: we're going to map to the smaller indices in 0..|F|-1
      Transaction t_f;
      unsigned int length = t_i.size();
//       nlog << "length = " << length << endl;
//       nlog << "t_i = " << t_i << endl;
      t_f.resize(length);
      for (unsigned int ix=0; ix<length; ix++) {
	t_f[ix] = item_f[t_i[ix]];
      }
//       nlog << "t_p = " << t_i << endl;
//       nlog << "t_f = " << t_f << endl;

      for (Transaction::iterator i=t_f.begin(); i!=t_f.end(); i++) {
	int u = *i;

 	// for each 2 combination (u,v)
 	for (Transaction::iterator j=i+1; j!=t_f.end(); j++) {
 	  int v = *j;
 	  if (u!=v)
 	    G.add(u,v);	// add (u,v) in G_F2 graph with proper index
 	}
      }
    }
  }
  delete scanner;
  nlog << "number of transactions processed = " << num_transactions << endl
       << flush ;
#endif

  Wall_Time t_end;
  nlog << "time for count_2_items_local " << t_end - t_start << endl;
}


void TS_Counter::prune_2_items(Undir_Mtx_Graph* total_two_items)
{
  nlog << "pruning total 2-items with support=" << support << endl << flush;
  F2_edge_weight = 0;
  F2_edges = 0;

  for (Undir_Mtx_Graph::Const_Walk w(*total_two_items);
       !w.end(); w.next()) {
    int weight = *w.ref();
//     nlog << "w(" << w.u() << "," << w.v() << ")=" << weight << endl;
    if (weight>=support && w.u()!=w.v()) {
      F2.addu(w.u(), w.v(), weight);
      F2_edge_weight += weight;
      F2_load.addu(w.u(), w.v(), weight);
      F2_edges++;
    }
  }
  nlog << "F2 "; F2.stats();
  nlog << flush;
}

void TS_Counter::project_items_from(TS_Counter& counter, set<int> items)
{
  // project items in counter to this counter

  nlog << "TS_Counter::project_items_from(counter,A)" << endl << flush;

  // make a db name
  string fn; ostringstream os(fn);
  os << counter.db_prefix;
  db_prefix = os.str();

  // copy support and other required variables
  support = counter.support;
  local_support = counter.local_support;
  relative_support = false;
  temp_filename = counter.temp_filename;

  // copy frequent item set and maps
  F = items;
  item_f = counter.item_f;
  f_item = counter.f_item;
//   nlog << "F=" << F << endl;
  nlog << "support = " << support << endl << flush;

  // copy necessary counts
  count.resize(counter.count.size());
  for (set<int>::iterator i=items.begin(); i!=items.end(); i++) {
    int item = *i;
//     cerr << "item " << item << " occurs: "
// 	 << counter.total_count[item] << endl;
    count[item] = counter.count[item];
    if (count[item] < support) {
      nlog << "*** ERROR: infrequent item " << item << " with support "
	   << count[item] << " given for projection" << endl << flush;
      exit(-1);
    }
  }

#ifdef DEBUG
  nlog << " vertex induced graph from " << items << endl << flush;
#endif

  // now copy vertex induced subgraph of F2

  // first compute the map of items in F using item_f map
  set<int> fitems;
  for (set<int>::iterator i=items.begin(); i!=items.end(); i++) {
    int item = *i;
    fitems.insert(item_f[item]); 
  }

  // we now have enough information to compute the vertex induced subgraph
  vertex_induced_subgraph(counter.F2, fitems, F2);
  vertex_induced_subgraph(counter.F2_load, fitems, F2_load);
  F2_edges = F2.num_uedges();

  nlog << "counter's number of edges, " << counter.F2.num_uedges()
       << "new number of edges " << F2_edges << endl << flush;  

}

void TS_Counter::compute_stats()
{
  total_tlen = 0;
  for (vector<int>::iterator i=count.begin();  i!=count.end(); i++)
    total_tlen += *i;

  if (num_transactions > 0)
    avg_tlen = double(total_tlen) / num_transactions;
  else
    avg_tlen = 0;

  int V = F2.real_num_vertices();

  if (F.size() > 0) {
    avg_F2_deg = double(F2.num_edges()) / V;
    F2_sparsity = double(F2.num_edges()) / double(V * V);
  }
  else {
    avg_F2_deg = 0;
    F2_sparsity = 0;
  }

  if (F.size()>0)
    avg_F2_edge_weight = double(F2_edge_weight) / V;
  else
    avg_F2_edge_weight = 0;

  Transaction_Set::Scanner* scanner = transaction_set.gen_scanner();
  // do a pass over transaction set
  double var_tlen = 0;
  while (!scanner->end()) {
    ::Transaction t_i;
    if (scanner->read(t_i)) {
      double v = t_i.size() - avg_tlen;
      var_tlen += v*v;
    }
  }
  delete scanner;
  assert(num_transactions > 0);
  var_tlen /= num_transactions;
  dev_tlen = sqrt(var_tlen);

  double var_F2_deg = 0;
  double var_F2_edge_weight = 0;
  int total_deg = 0;
  max_F2_deg = F2.degree(0);
  for (set<int>::iterator i=F.begin(); i!=F.end(); i++) {
    if (F2.degree(item_f[*i]) > max_F2_deg)
      max_F2_deg = F2.degree(item_f[*i]);   
  }
  min_F2_deg = max_F2_deg;
  int nv = 0;
  for (set<int>::iterator i=F.begin(); i!=F.end(); i++) {
    if (F2.degree(item_f[*i])>0) {
      nv ++;
      if (F2.degree(item_f[*i]) < min_F2_deg)
	min_F2_deg = F2.degree(item_f[*i]);
      double v = F2.degree(item_f[*i]) - avg_F2_deg;
      var_F2_deg += v*v;
      total_deg += F2.degree(item_f[*i]);
      for (Graph::Adjacency::iterator j=F2.adjacency[item_f[*i]].begin();
	   j!=F2.adjacency[item_f[*i]].end(); j++) {
	double v = j->weight - avg_F2_edge_weight;
	var_F2_edge_weight += v*v;
      }
    }
  }
  assert (F.size()>0);
  var_F2_deg /= V;
  dev_F2_deg = sqrt(var_F2_deg);
  assert(F2.num_edges()>0);
  var_F2_edge_weight /= F2.num_uedges();
  dev_F2_edge_weight = sqrt(var_F2_edge_weight);
}

void TS_Counter::print_db_stats(ostream& out)
{
  out << num_transactions << " " // static db stats
      << avg_tlen << " "
      << dev_tlen << " "
      << real_support << " "	// support threshold
      << F2.real_num_vertices() << " "  // number of 1-items
      << F2.num_uedges() << " "	// number of 2-items
      << F2_sparsity << " "
      << avg_F2_deg << " "
      << dev_F2_deg << " "
      << min_F2_deg << " "
      << max_F2_deg << " "
      << avg_F2_edge_weight << " "
      << dev_F2_edge_weight << " "
      << endl;
}

void TS_Counter::print_load_estimate(ostream& out)
{
  out << estimate_load_all(TS_Counter::fpgrowthload) << " "
      << estimate_load_all(TS_Counter::datumsize) << " "
      << estimate_load_all(TS_Counter::approx3patterns) << " "
      << endl;
}

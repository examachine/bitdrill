#include <fstream>
#include <cstring>
#include <stdio.h>
//#include <new>
#include "glob.h"
#include "gen.h"

using namespace std;

// prototypes
void command_line(TransPar &par);
void get_args(TransPar &par, int argc, char **argv);
void gen_rules(TransPar &par);
Transaction *mk_tran(StringSetIter &lits, LINT tlen, Taxonomy *tax = NULL);

void command_line(TaxPar &par);
void get_args(TaxPar &par, int argc, char **argv);
void gen_taxrules(TaxPar &par);

void command_line(SeqPar &par);
void get_args(SeqPar &par, int argc, char **argv);
void print_version(void);

void gen_seq(SeqPar &par);
CustSeq *mk_seq(Cid cid, StringSetIter &lseq, StringSet &lits, LINT slen, LINT tlen);

char data_file[256];
char pat_file[256];
char tax_file[256];


void memory_err(void)
{
  cout << "A memory allocation error occurred. \n";
  exit(1);
}


int main(int argc, char **argv)
{

  set_new_handler(memory_err);

  if (strcmp(argv[1], "lit") == 0) {
    // For Rules
    TransPar par;

    get_args(par, argc, argv);   // get arguments
    gen_rules(par);              // generate rules (really, just transactions)
  }

  else if (strcmp(argv[1], "seq") == 0) {
    // For Sequences
    SeqPar par;

    get_args(par, argc, argv);   // get arguments
    gen_seq(par);                // generate sequences
  }

  else if (strcmp(argv[1], "tax") == 0) {
    // For Rules with Taxonomies
    TaxPar par;

    get_args(par, argc, argv);   // get arguments
    gen_taxrules(par);           // generate rules (really, just transactions)
  }

  else if (strcmp(argv[1], "-version") == 0) {
    print_version();
    return 0;
  }

  else {
    cerr << "Synthetic Data Generation, ";
    print_version();
    cerr << "Usage:  " << argv[0] << " lit|tax|seq [options]\n";
    cerr << "        " << argv[0] 
      << " lit|tax|seq -help     For more detailed list of options\n";
    return 1;
  }

  return 0;
}


// Generate Transactions
//
void gen_rules(TransPar &par)
{
  StringSet *lits;
  StringSetIter *patterns;
  Transaction *trans;
  PoissonDist *tlen;

  ofstream data_fp;
  ofstream pat_fp;

  data_fp.open(data_file, ios::trunc);
  pat_fp.open(pat_file, ios::trunc);
  
  lits = new StringSet(par.nitems, par.lits);

  // Reset random seed generator for before generating transactions
  if (par.seed < 0) RandSeed::set_seed(par.seed);

  tlen = new PoissonDist(par.tlen-1);

  par.write(pat_fp);
  lits->display(pat_fp);

  patterns = new StringSetIter(*lits);
  for (LINT i = 0; i < par.ntrans; i ++)
    {
      trans = mk_tran(*patterns, (*tlen)()+1);
      if (par.ascii) 
	trans->write_asc(data_fp);
      else 
	trans->write(data_fp);
      delete trans;
    }
  
  data_fp.close();
  pat_fp.close();
}


// Generate Transactions and Taxonomy
//
void gen_taxrules(TaxPar &par)
{
  Taxonomy *tax;
  StringSet *lits;
  StringSetIter *patterns;
  Transaction *trans;
  PoissonDist *tlen;

  ofstream data_fp;
  ofstream pat_fp;
  ofstream tax_fp;

  data_fp.open(data_file, ios::trunc);
  pat_fp.open(pat_file, ios::trunc);
  tax_fp.open(tax_file, ios::trunc);
  if (data_fp.fail() || pat_fp.fail() || tax_fp.fail()) {
    cerr << "Error opening output file" << endl;
    exit(1);
  }
  
  // generate taxonomy and write it to file
  tax = new Taxonomy(par.nitems, par.nroots, par.fanout, par.depth_ratio);
  if (par.ascii) 
    tax->write_asc(tax_fp);
  else
    tax->write(tax_fp);

  tlen = new PoissonDist(par.tlen-1);

  lits = new StringSet(par.nitems, par.lits, tax);

  par.write(pat_fp);
  lits->display(pat_fp);

  patterns = new StringSetIter(*lits);
  for (LINT i = 0; i < par.ntrans; i ++)
    {
      trans = mk_tran(*patterns, (*tlen)()+1, tax);
      if (par.ascii) 
	trans->write_asc(data_fp);
      else 
	trans->write(data_fp);
      delete trans;
      delete trans;
    }
  
  data_fp.close();
  pat_fp.close();
  tax_fp.close();
}


// Generate a transaction
//
Transaction *mk_tran(StringSetIter &lits,  	// table of patterns
		     LINT tlen,			// transaction length
		     Taxonomy *tax
		     )
{
  Transaction *trans;
  StringP pat;
  
  trans = new Transaction(tlen);
  while (trans->size() < tlen)
    {
      pat = lits.get_pat();		// get a pattern
      if ( !trans->add(*pat) ) {
	// this pattern didn't fit in the transaction
	lits.unget_pat();
	break;
      }
    }
  return trans;
}


// Generate Sequences
//
void gen_seq(SeqPar &par)
{
  StringSet *lseq;	// potentially large sequences
  StringSetIter *patterns;
  StringSet *lits;	// potentially large itemsets	
  CustSeq *cust;	// 
  PoissonDist *slen;
  PoissonDist *tlen;

  ofstream data_fp;
  ofstream pat_fp;

  data_fp.open(data_file, ios::trunc);
  pat_fp.open(pat_file, ios::trunc);
  
  slen = new PoissonDist(par.slen-1);
  tlen = new PoissonDist(par.tlen-1);

  lits = new StringSet(par.nitems, par.lits);
  lseq = new StringSet(par.lits.npats, par.lseq, NULL, par.rept, par.rept_var);

//  pat_fp << "Large Itemsets:" << endl;
//  lits->write(pat_fp);
//  pat_fp << endl << endl << "Sequences:" << endl;
  par.write(pat_fp);
  lseq->display(pat_fp, *lits);

  patterns = new StringSetIter(*lseq);
  for (LINT i = 0; i < par.ncust; i ++)
    {
      cust = mk_seq(i+1, *patterns, *lits, (*slen)()+1, (*tlen)()+1);
      if (par.ascii) 
	cust->write_asc(data_fp);
      else 
	cust->write(data_fp);
      delete cust;
    }
  
  data_fp.close();
  pat_fp.close();
}


// Generate a customer-sequence
//
CustSeq *mk_seq(Cid cid,		// customer-id
		StringSetIter &lseq,	// table of large sequences
		StringSet &lits,	// table of large itemsets
		LINT slen,		// sequence length
		LINT tlen		// avg. transaction length
		)
{
  CustSeq *cust;
  StringP pat;
  
  cust = new CustSeq(cid, slen, tlen);
  while (cust->size() < slen * tlen)
    {
      pat = lseq.get_pat();      // get a pattern
      if ( !cust->add(*pat, lits) ) {	// transaction full
	lseq.unget_pat();
	break;
      }
    }
  return cust;
}



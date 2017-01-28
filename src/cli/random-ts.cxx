//
//
// C++ Implementation for module: random-ts
//
// Description: Generate a random transaction database with
// Gaussian distribution of transaction length for frequency mining task
//
// Author: exa
//
// Copyright: See COPYING file that comes with this distribution
//
//

#include "generic/General.hxx"
#include <gsl/gsl_rng.h>
#include <gsl/gsl_randist.h>

int main(int argc, char *argv[])
{
    cout << "random-ts -- generate a random transaction set." << endl;

    if ( argc!=6 ) {
      cout << "usage: random-ts <transaction-db> <num-items> <num-txns>"
	   << " <mean-length> <sigma-length>" << endl;
      return -1;
    }

    string db_name(argv[1]);
    ofstream out(db_name.c_str());
    
    int num_items, num_txns, mean_len, sigma_length;
    {
      istringstream is(argv[2]);
      is >> num_items;
    }
    {
      istringstream is(argv[3]);
      is >> num_txns;
    }
    {
      istringstream is(argv[4]);
      is >> mean_len;
    }
    {
      istringstream is(argv[5]);
      is >> sigma_length;
    }

    cout << "Generating random ts with " << num_items << " items and "
	 << num_txns << " transactions with mean length " << mean_len
	 << " stddev of length " << sigma_length << endl;

    // initialize RNG
    gsl_rng * r = gsl_rng_alloc (gsl_rng_mt19937);

    int b[num_items];
    for (int i = 0; i < num_items; i++) {
      b[i] = i;
    }

    for (int i=0; i<num_txns; i++) {
      int len = mean_len + int (gsl_ran_gaussian(r, double(sigma_length)));
      len = min(num_items,max (0,len));	// prevent user error
      if (len==0) {
	cout << "**\n";
	i--;
	continue;
      }
      // choose len items
      int a[len];
      gsl_ran_choose (r, a, len, b, num_items, sizeof (int));
      // output items
      for (int j=0; j<len; j++)
	out << a[j] << " ";
      out << endl;
    }

    gsl_rng_free(r);

    return 0;
}

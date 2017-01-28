//
//
// C++ Implementation for module: freq
//
// Description: 
//
//
// Author: exa
//
// Copyright: See COPYING file that comes with this distribution
//
//


#include "fp-tree/Pattern_Miner.hxx"
#include "Sample_TS.hxx"

int main(int argc, char *argv[])
{
    cout << "sample freq -- sequential frequency mining" << endl;

    int sample = 1;

    if (argc==2) {
      istringstream is(argv[1]);
      is >> sample;
    }

    Sample_TS* ts;
    switch(sample) {
    case 1: ts = new Sample_TS1(); break;
    case 2: ts = new Sample_TS2(); break;
    case 3: ts = new Sample_TS3(); break;
    case 4: ts = new Sample_TS4(); break;
    case 5: ts = new Sample_TS5(); break;
    case 6: ts = new Sample_TS6(); break;
    case 7: ts = new Sample_TS7(); break;
    default: cerr << "invalid sample number" << endl; exit(-1);
    }

    ts->dump(cout);
    Pattern_Miner miner(*ts, true);
    miner.discover_absolute(ts->support(), "sample");

    return 0; // success
}

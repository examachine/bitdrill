//
//
// C++ Implementation for module: thrash-cache
//
// Description: 
//
//
// Author: exa
//
// Copyright: See COPYING file that comes with this distribution
//
//

#include "generic/General.hxx"

int main()
{
  cout << "thrashing file system caches" << endl;
  int numlines = 1L << 19;
  {
    ofstream out("/tmp/dumbfile");
    for (int i=0; i<numlines; i++)
      out << "012345678901234567890123456789012345678901234567890123456789"
	  << "012345678901234567890123456789012345678901234567890123456789"
	  << endl;
  }
  {
    ifstream in("/tmp/dumbfile");
    char buf[256];
    for (int i=0; i<numlines; i++)
      in.getline(buf, 256);
  }
  remove("/tmp/dumbfile");

  return 0;
}

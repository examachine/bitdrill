//
//
// C++ Implementation for module: Sample_TS
//
// Description: 
//
//
// Author: exa
//
// Copyright: See COPYING file that comes with this distribution
//
//

#include "Sample_TS.hxx"

Sample_TS1::Sample_TS1()
{

  // insert items into transaction set
  int t1[] = {b,c,e,g};
  add(Transaction(t1, t1+4));

  int t2[] = {a,d,f,g};
  add(Transaction(t2, t2+4));

  int t3[] = {a,b,d,e,g};
  add(Transaction(t3, t3+5));

  int t4[] = {b,c,e,f};
  add(Transaction(t4, t4+4));

  int t5[] = {a,d,e,g};
  add(Transaction(t5, t5+4));

}

Sample_TS2::Sample_TS2()
{

  {int t[] = {a,b,c,f};
   add(Transaction(t, t+4));}

  {int t[] = {b,c};
   add(Transaction(t, t+2));}

  {int t[] = {a,d,e,f};
   add(Transaction(t, t+4));}

  {int t[] = {b,d,e,f};
   add(Transaction(t, t+4));}

  {int t[] = {a,c,d,e};
   add(Transaction(t, t+4));}

  {int t[] = {a,b,d,e};
   add(Transaction(t, t+4));}

  {int t[] = {b,c,e,f};
   add(Transaction(t, t+4));}

  {int t[] = {a,b,c,d,e};
   add(Transaction(t, t+5));}

  {int t[] = {b,c,f};
   add(Transaction(t, t+3));}


}

Sample_TS3::Sample_TS3()
{

  // insert items into transaction set
  {int t[] = {e,f,h};
  add(Transaction(t, t+3));}

  {int t[] = {a,d,e,h};
  add(Transaction(t, t+4));}

  {int t[] = {c,f,h};
  add(Transaction(t, t+3));}

  {int t[] = {a,b,c,d};
  add(Transaction(t, t+4));}

  {int t[] = {a,c};
  add(Transaction(t, t+2));}

  {int t[] = {e,f,g};
  add(Transaction(t, t+3));}

  {int t[] = {a,d,e,g};
  add(Transaction(t, t+4));}



}

Sample_TS4::Sample_TS4()
{

  // insert items into transaction set
  int t1[] = {a,c};
  add(Transaction(t1, t1+2));

  int t2[] = {a,b,c,d};
  add(Transaction(t2, t2+4));

  int t3[] = {a,d,e};
  add(Transaction(t3, t3+3));

  int t4[] = {a,d,e,g};
  add(Transaction(t4, t4+4));

  int t5[] = {e,f,h};
  add(Transaction(t5, t5+3));

  int t6[] = {e,f,g,h};
  add(Transaction(t6, t6+4));

}

Sample_TS5::Sample_TS5()
{

  {int t[] = {d,a};
   add(Transaction(t, t+2));}

  {int t[] = {a,d,f};
   add(Transaction(t, t+3));}

  {int t[] = {d,a,f};
   add(Transaction(t, t+3));}

  {int t[] = {a,d,f};
   add(Transaction(t, t+3));}

  {int t[] = {f};
   add(Transaction(t, t+1));}

  {int t[] = {f};
   add(Transaction(t, t+1));}

}

Sample_TS6::Sample_TS6()
{

  {int t[] = {h,b,i,c};
   add(Transaction(t, t+4));}

  {int t[] = {b,i,c};
   add(Transaction(t, t+3));}

  {int t[] = {h,i,c};
   add(Transaction(t, t+3));}

}

Sample_TS7::Sample_TS7()
{

  // insert items into transaction set

  {int t[] = {e,f,h};
  add(Transaction(t, t+3));}

  {int t[] = {a,d,e,h};
  add(Transaction(t, t+4));}

  {int t[] = {d,f,g};
  add(Transaction(t, t+3));}

  {int t[] = {a,b,c,d};
  add(Transaction(t, t+4));}

  {int t[] = {a,c,f,h};
  add(Transaction(t, t+3));}

  {int t[] = {e,f,g};
  add(Transaction(t, t+3));}

  {int t[] = {a,b,d,e};
  add(Transaction(t, t+4));}

  {int t[] = {c,d,h};
  add(Transaction(t, t+3));}

  {int t[] = {a,c,f,h};
  add(Transaction(t, t+4));}

  {int t[] = {e,f,g,h};
  add(Transaction(t, t+4));}

  {int t[] = {a,d,e,g};
  add(Transaction(t, t+4));}

}

void Sample_TS::dump(ostream& out) {
  int num_trans=0;
  Scanner* scanner = gen_scanner();
  while (!scanner->end()) {
    Transaction t_i;
    if (scanner->read(t_i)) {
      num_trans++;
      cout << "t" << num_trans << ": ";
      for (Transaction::iterator i=t_i.begin(); i!=t_i.end(); i++) {
	cout << (char)(*i + 'a') << " ";
      }
      cout << endl;
    }
  }
  delete scanner;
}

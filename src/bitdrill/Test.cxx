//
//
// C++ Implementation for module: Test
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
#include "Trie.hxx"
#include "Hash_Trie.hxx"
#include "Bit_Vector.hxx"

int main()
{
  {
    Bit_Vector a(15);
    Bit_Vector b(15);
    a.set(3);
    a.set(5);
    a.set(7);
    a.set(11);
    b.set(3);
    b.set(5);
    b.set(9);
    b.set(11);
    cout << "a&b has " << a.intersect_and_count(b) << " bits" << endl;
    
  }

  {
    Bit_Vector a(100);
    Bit_Vector b(100);
    a.set(3);
    a.set(5);
    a.set(7);
    a.set(11);
    a.set(25);
    a.set(40);
    a.set(60);
    a.set(70);
    a.set(99);
    b.set(3);
    b.set(5);
    b.set(9);
    b.set(11);
    b.set(21);
    b.set(25);
    b.set(40);
    b.set(61);
    b.set(70);
    b.set(99);
    a.print();
    b.print();
    Bit_Vector c(100);
    a.intersect(b, c);
    c.print();
    cout << "a&b has " << c.count() << " bits" << endl;
    cout << "a&b has " << a.count_intersection(b) << " bits" << endl;
    cout << "a&b has " << a.intersect_and_count(b) << " bits" << endl;
    
  }


  {
    Trie<char> trie;

    char t1[] = "ced";
    trie.insert(vector<char>(t1,t1+3));
    char t2[] = "cdc";
    trie.insert(vector<char>(t2,t2+3));
    char t3[] = "ceda";
    trie.insert(vector<char>(t3,t3+4));
    char t4[] = "ceg";
    trie.insert(vector<char>(t4,t4+3));
    char t5[] = "ded";
    trie.insert(vector<char>(t5,t5+3));

    cout << "query ceda " << trie.query(vector<char>(t3, t3+4)) << endl;
    cout << "query ce " << trie.query(vector<char>(t1, t1+2)) << endl;

    Trie<char>::Walk walk(&trie);

    while (!walk.end()) {
      if (walk.next()) {
	vector<char> str = walk.ref();
	cout << "element: " << str << endl;
      }
    }
  }
  {
    cout << "test transmission 1" << endl;
    Trie<char> trie1;
    char t1[] = "ab";
    trie1.insert(vector<char>(t1,t1+2));
    char t2[] = "ac ";
    trie1.insert(vector<char>(t2,t2+2));

    vector<int> M;
    trie1.encode(M);
    cout << "trie1"<< endl;
    trie1.print();
    cout << "M=" << M << endl;
    Trie<char> trie2;
    trie2.decode(M);
    cout << "transmitted contents into a second trie:" << endl;
    trie2.print();
    cout << "trie2 end" << endl;

  }

  {
    Hash_Trie<char> trie;
    trie.insert(vector<char>(1, 'a'));
    trie.insert(vector<char>(1, 'b'));
    trie.insert(vector<char>(2, 'c'));

    trie.print();

    Hash_Trie<char>::Walk walk(&trie);

    while (!walk.end()) {
      if (walk.next()) {
	vector<char> str = walk.ref();
	cout << "element: " << str << endl;
      }
    }

  }

}

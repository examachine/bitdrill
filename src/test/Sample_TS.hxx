//
//
// C++ Interface for module: Sample_TS
//
// Description: 
//
//
// Author: exa
//
// Copyright: See COPYING file that comes with this distribution
//
//

#ifndef Sample_TS_Interface
#define Sample_TS_Interface

#include "generic/General.hxx"

#include "transaction-db/Transaction_Set.hxx"

class Sample_TS : public Transaction_Set {
public:
  enum Items {a='a', b, c, d, e, f, g, h, i};
  Sample_TS (){}
  virtual ~Sample_TS () {}
  void dump(ostream&);
  virtual int support() { return 3; }
};

// these are example db's in the thesis

class Sample_TS1 : public Sample_TS {
public:
  Sample_TS1 ();
  virtual ~Sample_TS1 () {}
};

class Sample_TS2 : public Sample_TS {
public:
  Sample_TS2 ();
  virtual ~Sample_TS2 () {}
  virtual int support() { return 4; }
};

class Sample_TS3 : public Sample_TS {
public:
  Sample_TS3 ();
  virtual ~Sample_TS3 () {}
  virtual int support() { return 2; }
};

class Sample_TS4 : public Sample_TS {
public:
  Sample_TS4 ();
  virtual ~Sample_TS4 () {}
  virtual int support() { return 2; }
};

// these are for demonstrating flaws in fp-growth
// and how we fixed them ;)

class Sample_TS5 : public Sample_TS {
public:
  Sample_TS5 ();
  virtual ~Sample_TS5 () {}
  virtual int support() { return 4; }
};

class Sample_TS6 : public Sample_TS {
public:
  Sample_TS6 ();
  virtual ~Sample_TS6 () {}
  virtual int support() { return 2; }
};

class Sample_TS7 : public Sample_TS {
public:
  Sample_TS7 ();
  virtual ~Sample_TS7 () {}
};

#endif

/*----------------------------------
File     : data.cpp
Contents : data set management
----------------------------------*/

#include "StdAfx.h"
#include <vector>
#include <algorithm>
using namespace std;
#include "data.h"

Data::Data(Transaction_Set &t)
  : ts(t)
{
  scanner = ts.gen_scanner();
}

Data::~Data()
{
  delete scanner;
}

int Data::isOpen()
{
  return !scanner->end();
}

bool Data::getNextTransaction(Transaction &t)
{
  return scanner->read(t);
}

//
//
// C++ Interface for module: Matrix
//
// Description: 
//
//
// Author: exa
//
// Copyright: See COPYING file that comes with this distribution
//
//

#ifndef Matrix_Interface
#define Matrix_Interface

#include "generic/General.hxx"

// modified: we don't want numeric stuff here

// Generic Dense Matrix class
// Number: numerical type to store in each element
// Rows: number of rows
// Columns: number of columns

template <class T>
class Matrix
{
public:
  typedef T Component;

  typedef Matrix<Component> Self;

  //constructors
  Matrix(int r, int c) : rows(r), cols(c) {
    M = new Component[rows * cols];
  }

  // square matrix
  Matrix(int r) : rows(r), cols(r) {
    M = new Component[rows * cols];
  }

  // generic parameter selectors
  int sel_rows() const { return rows; }
  int sel_columns() const { return cols; }

  Component& operator()(int i, int j) {
    return *rep(i,j);
  }

  // access element first row, second column
  // we keep it like this to remain compatible with mathematical notation
  Component sel_element(int i, int j) {
    return *rep(i,j);
  }
  void mod_element(int i, int j, Component val) {
    *rep(i,j) = val;
  }
  Component elt(int i, int j) const {
     return sel_element(i,j);
  }

  // row i, col j
  Component* rep(int i,int j)
  {
    return M + i*cols + j;
  }

protected:

  // Matrix is stored in a row major order array
  Component* M;

  int rows, cols;
};

template<class N>
ostream &operator <<(ostream& out, Matrix<N> & A)
{
  for (int i=0; i<A.sel_rows(); i++) {
    for (int j=0; j<A.sel_columns(); j++)
      out << setw(5) << " " << A.sel_element(i,j);
    out << endl;
  }

  return out;
};

#endif

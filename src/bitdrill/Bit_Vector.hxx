#ifndef Bit_Vector_Interface
#define Bit_Vector_Interface

#include "generic/General.hxx"
#include <stdint.h>

//TODO: optimizations for 64-bit

// bit vector with given size, not extensible at the moment
// we store the bits in an array of unsigned longs
// the least significant bits in a word contain the smaller indices
class Bit_Vector
{
public:

  typedef uint32_t ulong;

  Bit_Vector(int _size = 0) {
    resize(_size);
  }

  void resize(int _size) {
    num_bits = _size;
    // store it here
    int num_longs = int(ceil(double(num_bits)/32));
    vec.resize(num_longs); 
    fill(vec.begin(), vec.end(), 0); // make zero initially
//     start_address = &vec.front();
    start_offset = 0;
//     end_address = (&vec.front()) + num_longs - 1;
    end_offset = num_bits % 32;
  }
  /* used to be able to define slices of a bitvector as bitvecs themselves.
  Bit_Vector(long* s_address, int s_offset, long *e_address, int e_offset)
  {
  }
  */

  int size() const {
    return num_bits;
  }

  bool get(int ix) const {
    return start_address()[ix / 32] &  1L << (ix % 32); 
  }

  void set(int ix) {
    start_address()[ix / 32] |=  1L << (ix % 32); 
  }

  void unset(int ix) {
    start_address()[ix / 32] &=  0xfffffffe << (ix % 32);
  }

  // intersect with another bit vector destructively
  void intersect(const Bit_Vector& bv) {
    if (bv.num_bits != num_bits) {
      cerr << "BitVector: rhs num_bits not equal" << endl;
      assert(false);
    }
    else {
      ulong *a = start_address();
      const ulong *b = bv.start_address();
      if (start_offset != bv.start_offset) {
	// TODO: necessary for intersection in 2D bitvecs
	cerr << "BitVector: 2d intersection not implemented" << endl;
	assert(false);
      }
      else {
	assert(start_offset==0); // TODO: remove this restriction 
	ulong *end = end_address();
	if (end_offset!=0)
	  end--;
	for (a = start_address(); a <= end; a++, b++) {
	  ulong c = *a & *b;
	  *a = c;
	}
	if (end_offset!=0) {
	  ulong mask = 0xffffffff << (end_offset);
	  *a &= *b | mask;
	}
      }
    }
  }


  // intersect with another bit vector and write the result
  void intersect(const Bit_Vector& bv, Bit_Vector& result) const {
    if (bv.num_bits != num_bits) {
      cerr << "BitVector: rhs num_bits not equal" << endl;
      assert(false);
    }
    else {
      if (result.num_bits!=bv.num_bits)
	result.resize(bv.num_bits);
      const ulong *a = start_address();
      const ulong *b = bv.start_address();
      ulong *c = result.start_address();
      if (start_offset != bv.start_offset) {
	// TODO: necessary for intersection in 2D bitvecs
	cerr << "BitVector: 2d intersection not implemented" << endl;
	exit(-1);
      }
      else {
	assert(start_offset==0); // TODO: remove this restriction 
	const ulong *end = end_address();
	if (end_offset!=0)
	  end--;
	for (a = start_address(); a <= end; a++, b++, c++) {
	  *c = *a & *b;
	}
	if (end_offset!=0) {
	  ulong mask = 0xffffffff << (end_offset);
	  *c = *a & (*b | mask);
	}
      }
    }
  }

  // count number of 1's
  int count() const {
    int count = 0;
    const ulong *a = start_address();
    assert(start_offset==0); // TODO: remove this restriction 
    const ulong *end = end_address();
    if (end_offset!=0)
      end--;
    for (a = start_address(); a <= end; a++) {
      ulong c = *a;
      count += Bit_Vector::numbits(c);
    }
    if (end_offset!=0) {
      ulong mask = 0xffffffff << (end_offset);
      count += numbits(*a & ~mask);
    }
    
    return count;
  }


  // intersect with another bit vector destructively and count number of 1's
  int intersect_and_count(const Bit_Vector& bv) {
    if (bv.num_bits != num_bits) {
      cerr << "BitVector: rhs num_bits not equal" << endl;
      assert(false);
    }
    else {
      int count = 0;
      ulong *a = start_address();
      const ulong *b = bv.start_address();
      if (start_offset != bv.start_offset) {
	// TODO: necessary for intersection in 2D bitvecs
	cerr << "BitVector: 2d intersection not implemented" << endl;
	exit(-1);
      }
      else {
	assert(start_offset==0); // TODO: remove this restriction 
	ulong *end = end_address();
	if (end_offset!=0)
	  end--;
	for (a = start_address(); a <= end; a++, b++) {
	  ulong c = *a & *b;
	  *a = c;
	  count += numbits(c);
	}
	if (end_offset!=0) {
	  ulong mask = 0xffffffff << (end_offset);
	  *a &= *b | mask ;
	  count += numbits(*a & ~mask);
	}
      }
      return count;
    }
  }

  // count number of bits in intersection, but don't write the result
  int count_intersection(const Bit_Vector& bv) const {
    if (bv.num_bits != num_bits) {
      cerr << "BitVector: rhs num_bits not equal" << endl;
      assert(false);
      return 0;
    }
    else {
      int count = 0;
      const ulong *a = start_address();
      const ulong *b = bv.start_address();
      if (start_offset != bv.start_offset) {
	// TODO: necessary for intersection in 2D bitvecs
	cerr << "BitVector: 2d intersection not implemented" << endl;
	exit(-1);
      }
      else {
	assert(start_offset==0); // TODO: remove this restriction 
	const ulong *end = end_address();
	if (end_offset!=0)
	  end--;
	for (a = start_address(); a <= end; a++, b++) {
	  ulong c = *a & *b;
	  count += numbits(c);
	}
	if (end_offset!=0) {
	  ulong mask = 0xffffffff << (end_offset);
	  ulong c = *a & (*b | mask) ;
	  count += numbits(c & ~mask);
	}
      }
      return count;
    }
  }

  // for debugging
  void print(ostream& out = cout) const
  {
    for (int i=0; i<num_bits; i++) {
      if (get(i))
	out << '1';
      else
	out << '0';
    }
  }


private:
  int num_bits;

//   ulong* start_address;
  int  start_offset;
//   ulong* end_address;
  int end_offset;

  vector<ulong> vec;

  ulong* start_address() {
    return &vec.front();
  }

  ulong* end_address() {
    return (&vec.front()) + vec.size() - 1;
  }

  const ulong* start_address() const {
    return &vec.front();
  }

  const ulong* end_address() const {
    return (&vec.front()) + vec.size() - 1;
  }


  // count number of 1 bits in a word
  static int numbits(ulong v)
  {
    v = v - ((v >> 1) & 0x55555555);                 // reuse input as temporary
    v = (v & 0x33333333) + ((v >> 2) & 0x33333333);  // temp
    int c = ((v + (v >> 4) & 0xF0F0F0F) * 0x1010101) >> 24; // count

    return c;
  }

};

inline ostream& operator<<(ostream& out, const Bit_Vector &bv) {
  bv.print(out);
  return out;
}

#endif


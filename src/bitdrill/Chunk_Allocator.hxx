// a lame attempt at providing a state-ful stl allocator
// which is a futile exercise using g++ stdlib implementation

namespace std {
  template <class T> class chunk_allocator;

  template <> class chunk_allocator<void> {
  public:
    typedef void*       pointer;
    typedef const void* const_pointer;
    // reference to void members are impossible.
    typedef void value_type;
    template <class U> struct rebind { typedef allocator<U>
    other; };
  };

  template <class T> class chunk_allocator {
  public:
    typedef size_t    size_type;
    typedef ptrdiff_t difference_type;
    typedef T*        pointer;
    typedef const T*  const_pointer;
    typedef T&        reference;
    typedef const T&  const_reference;
    typedef T         value_type;
    template <class U> struct rebind { typedef chunk_allocator<U>
    other; };

    static const int chunk_size = 10000; 
    struct chunk {
      chunk() {
	first = available = new value_type[chunk_size];
      }
      pointer first;
      pointer available;
    };

    chunk_allocator() throw() {
      // allocate first chunk
      chunks.push_back(new chunk);
    }
    chunk_allocator(const chunk_allocator&) throw();
    template <class U> chunk_allocator(const chunk_allocator<U>&) throw();

    ~chunk_allocator() throw() {
      for (typename list<chunk*>::iterator i=chunks.begin(); i!=chunks.end(); i++)
	delete[] *i;
    }

    pointer address(reference x) const {
      return &x;
    }
    const_pointer address(const_reference x) const {
      return &x;
    }

    pointer allocate(size_type n,
		     chunk_allocator<void>::const_pointer hint = 0) {
      if (n > chunk_size)
	std::__throw_bad_alloc();

      chunk *current_chunk = chunks.back();
      if (current_chunk->available + n > current_chunk->first + chunk_size) {
	// need a new chunk? totally waste memory :)
	chunks.push_back(new chunk);
	chunk *current_chunk = chunks.back();
      }
      pointer addy = current_chunk->available;
      current_chunk->available += n;
      return addy;
    }
    void deallocate(pointer p, size_type n) {
      // here, we do precisely nothing
    }
    size_type max_size() const throw() {
      return chunk_size;
    }

    void construct(pointer p, const T& val)
    { ::new(p) T(val); }
    void destroy(pointer p) { p->~T(); }

    // we store a list of chunks
    list<chunk*> chunks;
 
  };

//   template<typename T>
//   inline bool
//   operator==(const chunk_allocator<T>&x, const chunk_allocator<T>&y)
//   { return false; }
  
//   template<typename _Tp>
//   inline bool
//   operator!=(const chunk_allocator<T>&x, const chunk_allocator<T>&y)
//   { return true; }
}

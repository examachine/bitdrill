#ifndef __SPARSEBITARRAYPOOL
#define __SPARSEBITARRAYPOOL

#include <stdlib.h>
using namespace std;

typedef void* pvoid;

// Template for object pool (of class T)
template <class T> class CObjectPool
{
public:
	CObjectPool(void);
	~CObjectPool(void);

	T *Alloc();
	void Free(T *);
private:
	int pos;
	int length;
	T **pool;
};

template <class T>
CObjectPool<T>::CObjectPool(void)
{
	pos = 0;
	length = 1024 * 10;
	pool = (T **)new pvoid[1024 * 10];
	for (int loop = 0 ; loop < length; loop++)
		pool[loop] = NULL;
}

template <class T>
CObjectPool<T>::~CObjectPool(void)
{
 	for (int loop = 0; loop < pos; loop++) 
 		delete pool[loop]; 

 	delete pool; 
}

template <class T>
T *CObjectPool<T>::Alloc()
{
	if (pos == 0)
		return new T();

	return pool[--pos];
}

template <class T>
void CObjectPool<T>::Free(T *i)
{
	if (pos == length)
	{
		delete i;
		return;
	}


	i->Clear();
	pool[pos++] = i;
}

#endif

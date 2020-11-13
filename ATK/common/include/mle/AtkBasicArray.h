/** @defgroup MleATK Magic Lantern Authoring Toolkit */

/**
 * @file AtkBasicArray.h
 * @ingroup MleATK
 *
 * This file contains a class that manages a growable
 * array of objects.
 *
 * @author Mark S. Millard
 */

// COPYRIGHT_BEGIN
//
// The MIT License (MIT)
//
// Copyright (c) 2015-2020 Wizzer Works
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
//
//  For information concerning this header file, contact Mark S. Millard,
//  of Wizzer Works at msm@wizzerworks.com.
//
//  More information concerning Wizzer Works may be found at
//
//      http://www.wizzerworks.com
//
// COPYRIGHT_END

#ifndef __ATK_BASICARRAY_H_
#define __ATK_BASICARRAY_H_

// Include Magic Lantern header files.
#include <mle/mlAssert.h>
#include <mle/mlMalloc.h>
#include <mle/mleatk_rehearsal.h>

/**
 * This class is a growable array of objects.
 *
 * It is designed to be extremely light weight with just enough function for a tinyUI.
 */
class MLEATK_REHEARSAL_API AtkBasicArray
{
  public:

    AtkBasicArray(int elementSize);
    virtual ~AtkBasicArray();

    AtkBasicArray& operator=(const AtkBasicArray&);

    // Insert count elements at index.  The new elements are
    // initialized to zero.  length is adjusted by count.
    void insert(int index, int count = 1);

    // Remove count elements at index.  The length is adjusted
    // by count.  If the stuff in the array is made up of objects
    // then the caller is responsible for invoking destructors.
    void remove(int index, int count = 1);

    // Make space for count elements at the end of the array.
    // The new elements are initialized to zero.
    void append(int count) { grow(m_length + count); }

    // Make array large enough to contain at least newLength elements.
    // Uses elementSize to convert to a byte offset.  Length is
    // increased if necessary.
    // While grow actually makes array bigger - prealloc just allocates
    // the space - This can be used for more efficient malloc operations
    // it does not actually reset the length of the array
    void grow(int newLength);
    void preAlloc(int newLength);

    // Make array small enough so that it contains newLength elements.
    void shrink(int newLength);

    // Get what the current length is.
    int getLength() { return m_length; }

    // Remove all the elements in the list.
    void removeAll() { shrink(0); }

    // This is ugly - but necessary for qsort
    void* getBase() { return m_base; }

	/**
	 * Override operator new.
	 *
	 * @param tSize The size, in bytes, to allocate.
	 */
	void* operator new(size_t tSize);

	/**
	 * Override operator delete.
	 *
	 * @param p A pointer to the memory to delete.
	 */
    void  operator delete(void *p);

  protected:

    // Find the given element.  If it doesn't appear in the array,
    // return -1.
    int findptr(void* element);

	/** The number of elements in the array. */
    int m_length;
	/** The base of the array. */
    char* m_base;
	/** The size of an element, in bytes. */
    int m_elementSize;
	/** The size to grow the array by, in elements; default is 5. */
    int m_growBy;
	/** The unused space in the array, in elements. */
    int m_space;
};

//----------------------------------------------------------------------

// Macros which implement a typed interface to AtkBasicArray.  If you want
// an array of things which need to have destructors invoked, this
// macro is not for you.  Also, bcmp() is used to compare elements to
// implement find() and remove(); if bcmp() is not appropriate, this
// macro is not for you.
//
// XXX *NOTE* There is no assignment operator right now; you can't
// assign one AtkBasicArray to another.  So don't make a method with return
// type AtkBasicArray!
//
// Normal subscripting can be used to get a reference to the nth
// element.
//
// add() adds an element to the end of the array.
//
// find() finds the subscript for the given element.  If the element
// doesn't appear in the array, returns -1.
//
// remove() removes the element from the array.  The element must be
// in the array somewhere.


#define MLE_DECLARE_ARRAY(name,type)												\
class MLEATK_REHEARSAL_API name : public AtkBasicArray {							\
  public:																			\
    name() : AtkBasicArray(sizeof(type)) { }										\
    virtual ~name() { }																\
																					\
    type& operator[](int i) {														\
	    MLE_ASSERT(i >= 0 && i < m_length);											\
	    return *((type*)m_base + i);												\
    }																				\
																					\
    void add(type& t) {append(1); (*this)[m_length - 1] = t;}						\
																					\
    void addBefore(int i, type& t) {insert(i, 1); (*this)[i] = t;}					\
																					\
    int find(type& t) {return findptr(&t);}											\
																					\
    void remove(type& t) {AtkBasicArray::remove(find(t));}							\
																					\
    void remove(int index, int count = 1) {AtkBasicArray::remove(index, count);}	\
																					\
	void removeAll() {AtkBasicArray::removeAll();}									\
}

#endif /* __ATK_BASICARRAY_H_ */

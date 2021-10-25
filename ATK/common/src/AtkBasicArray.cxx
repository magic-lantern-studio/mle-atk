/** @defgroup MleATK Magic Lantern Authoring Toolkit */

/**
 * @file AtkBasicArray.cxx
 * @ingroup MleATK
 *
 * This file contains the implementation of a class that manages a growable
 * array of objects.
 */

// COPYRIGHT_BEGIN
//
// The MIT License (MIT)
//
// Copyright (c) 2015-2021 Wizzer Works
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
//  For information concerning this source file, contact Mark S. Millard,
//  of Wizzer Works at msm@wizzerworks.com.
//
//  More information concerning Wizzer Works may be found at
//
//      http://www.wizzerworks.com
//
// COPYRIGHT_END

// Include system header files.
#include <string.h>

// Include Magic Lantern header files.
#include "mle/AtkBasicArray.h"

AtkBasicArray::AtkBasicArray(int es)
{
    m_length = 0;
    m_base = 0;
    m_elementSize = es;
    m_growBy = 5;
    m_space = 0;
}


AtkBasicArray::~AtkBasicArray()
{
    if (m_base) mlFree(m_base);
}

AtkBasicArray& AtkBasicArray::operator=(const AtkBasicArray& t)
{
    MLE_ASSERT(m_elementSize == t.m_elementSize);
    grow(t.m_length);
    m_length = t.m_length;
    //bcopy(t.m_base, m_base, m_space * m_elementSize);
	memcpy(m_base, t.m_base, m_space * m_elementSize);
    return *this;
}
    

void AtkBasicArray::preAlloc(int newLength)
{
    if (newLength > m_space)
	{
		int newSpace = newLength + m_growBy;
		if (!m_base)
		{
			m_base = (char*) mlMalloc(newSpace*m_elementSize);
		} else
		{
			m_base = (char*) mlRealloc(m_base, newSpace*m_elementSize);
		}
		m_space = newSpace;
    }
}

void AtkBasicArray::grow(int newLength)
{
    if (newLength > m_space)
	{
		int newSpace = newLength + m_growBy;
		if (!m_base)
		{
			m_base = (char*) mlMalloc(newSpace*m_elementSize);
		} else
		{
			m_base = (char*) mlRealloc(m_base, newSpace*m_elementSize);
		}
		m_space = newSpace;
	}
	if (m_length < newLength)
	{
		m_length = newLength;
    }
}

void AtkBasicArray::shrink(int newLength)
{
    MLE_ASSERT((newLength >= 0) && (newLength <= m_length));
    if (newLength < m_length)
	{
		if (!m_base)
		{
			m_base = (char*) mlMalloc(newLength*m_elementSize);
		} else
		{
			m_base = (char*) mlRealloc(m_base, newLength*m_elementSize);
		}
		m_length = newLength;
		m_space = newLength;
    }
}

void AtkBasicArray::insert(int index, int count)
{
    if (index >= m_length)
	{
		// Increase array size to accomodate index+count.  Because index
		// is larger than len new data added will be properly zero'd
		// without any excess zeroing.
		grow(index + count);
    } else
	{
		if (m_length + count > m_space)
		{
			// Need room in the array to grow.
			int newSpace = m_length + count + m_growBy;
			if (!m_base)
			{
				m_base = (char*) mlMalloc(newSpace * m_elementSize);
			} else
			{
				m_base = (char*) mlRealloc(m_base, newSpace * m_elementSize);
			}

			// Clear out the tail portion of the array that will not
			// be copied over and will not used immediately.  This
			// happens because of growBy.
			//::bzero(m_base + m_space * m_elementSize,
			//	(newSpace - m_space) * m_elementSize);
			::memset(m_base + m_space * m_elementSize, '\0',
				(newSpace - m_space) * m_elementSize);
			m_space = newSpace;
		}

		// AtkBasicArray memory is now large enough.  Slide tail portion
		// towards higher addresses, then clear out the inserted area.
		int bytes = count * m_elementSize;
		//char* bi = m_base + index * m_elementSize;
		//::bcopy(bi, bi + bytes, (m_length - index) * m_elementSize);
		//::bzero(bi, bytes);
		char* bi = m_base + index * m_elementSize;
		::memcpy(bi + bytes, bi, (m_length - index) * m_elementSize);
		::memset(bi, '\0', bytes);

		m_length += count;
    }
}

void AtkBasicArray::remove(int index, int count)
{
    MLE_ASSERT((index >= 0) && (index + count <= m_length));
    int ic = index + count;
    int numMove = m_length - ic;
    if (numMove)
	{
		// Slide data past hole into hole
		//::bcopy(m_base + ic * m_elementSize, m_base + index * m_elementSize,
		//	numMove * m_elementSize);
		::memcpy(m_base + index * m_elementSize, m_base + ic * m_elementSize,
			numMove * m_elementSize);
    }

    // Zero out data past moved area.
    //::bzero(m_base + (index + numMove) * m_elementSize, count * m_elementSize);
	::memset(m_base + (index + numMove) * m_elementSize, '\0', count * m_elementSize);
    m_length -= count;
}



int AtkBasicArray::findptr(void* element)
{
    for (int i=0 ; i<m_length ; i++)
	{
		//if (bcmp(m_base + (i * m_elementSize), element, m_elementSize) == 0)
		if (memcmp(m_base + (i * m_elementSize), element, m_elementSize) == 0)
		{
			return i;
		}
    }
    return -1;
}

void *
AtkBasicArray::operator new(size_t tSize)
{
	void *p = mlMalloc(tSize);
	return p;
}

void
AtkBasicArray::operator delete(void *p)
{
	mlFree(p);
}

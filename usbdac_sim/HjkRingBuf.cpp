#include "stdafx.h"
#include "HjkRingBuf.h"

HjkRingBuf::HjkRingBuf(const UINT MaxElements) :
	MaxElements(MaxElements)
{
	HJK_ATOMIC_START
	{
		_numElements = 0;
		_head = 0;
	}	
	HJK_ATOMIC_END
}

	bool HjkRingBuf::add(const void * obj, bool overwrite)
	{
		bool full = false;
		HJK_ATOMIC_START
		{
			full = isFull();
			if (!full || overwrite) {
				_buf[_head] = &obj;
				_head = (_head + 1) % MaxElements;
				_numElements = full ? _numElements : (_numElements + 1);
			}
		}
		
		HJK_ATOMIC_END

			return !full;
	}
	bool HjkRingBuf::pull(void ** dest)
		{
		bool ret = false;
		size_t tail;

		HJK_ATOMIC_START
		{
			if (!isEmpty()) {
				tail = getTail();
				*dest = _buf[tail];
				_numElements--;

				ret = true;
			}
		
		}
		HJK_ATOMIC_END

			return ret;
		}
	bool HjkRingBuf::isFull() const
		{
		bool ret;

		HJK_ATOMIC_START
		{
			ret = _numElements >= MaxElements;
		}
			
		HJK_ATOMIC_END

		return ret;
		}
	UINT HjkRingBuf::numElements() const
		{
		size_t ret;

		HJK_ATOMIC_START
		{
			ret = _numElements;
		}
			
			HJK_ATOMIC_END

			return ret;
		}
bool HjkRingBuf::isEmpty() const
		{
	bool ret;

	HJK_ATOMIC_START
	{
		ret = !_numElements;
	}
		
	HJK_ATOMIC_END

		return ret;
		}
UINT HjkRingBuf::getTail() const
	{
	return (_head + (MaxElements - _numElements)) % MaxElements;
	}
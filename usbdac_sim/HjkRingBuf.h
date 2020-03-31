#pragma once
#ifndef __HJKRINGBUF_
#define __HJKRINGBUF_

#include <windows.h>
#define HJKMUTEX					hjkMutex
#define HJK_ATOMIC_START			WaitForSingleObject(HJKMUTEX, INFINITE);
#define HJK_ATOMIC_END				ReleaseMutex(HJKMUTEX);






typedef unsigned int UINT;

class HjkRingBuf
{
protected:
	void*			*_buf;
	const UINT		MaxElements;
	UINT			_head;
	UINT			_numElements;
	HANDLE			HJKMUTEX = CreateMutex(NULL,false,NULL);
public:
	HjkRingBuf(const UINT MaxElements);
	~HjkRingBuf();
	/*
	*向缓存中添加节点
	* If there is already MaxElements in the buffer,
	* the oldest element will either be overwritten (when overwrite is true) or
	* this add will have no effect (when overwrite is false).
	*
	* Return: true if there was room in the buffer to add this element
	*/
	bool add(const void* obj, bool overwrite = false);
	/**
	* Remove last element from buffer, and copy it to dest
	* Return: true on success
	*/
	bool pull(void* *dest);
	/**
	* Return: true if buffer is full
	*/
	bool isFull() const;
	/**
	* Return: number of elements in buffer
	*/
	UINT numElements() const;
	/**
	* Return: true if buffer is empty
	*/
	bool isEmpty() const;
protected:
	/**
	* Calculates the index in the array of the oldest element
	* Return: index in array of element
	*/
	UINT getTail() const;

};
#endif


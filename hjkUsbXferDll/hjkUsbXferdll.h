#pragma once

#ifdef HJKUSBXFERDLL_EXPORTS
#define HJKUSBXFERDLL_API __declspec(dllexport)
#else
#define HJKUSBXFERDLL_API __declspec(dllimport)
#endif
#define IN_RQUEUE_LEN	4
#define OUT_RQUEUE_LEN	4
//int32 in func
//extern "C" HJKUSBXFERDLL_API void  initQueueBuf(int ch,int outBytes);

extern "C" HJKUSBXFERDLL_API void queueBufStart(__int32 *allChIn1, __int32 *out, int ch, int outBytes);
/*
extern "C" HJKUSBXFERDLL_API bool fibonacci_next();

// Get the current value in the sequence.
extern "C" HJKUSBXFERDLL_API unsigned long long fibonacci_current();

// Get the position of the current value in the sequence.
extern "C" HJKUSBXFERDLL_API unsigned fibonacci_index();
*/
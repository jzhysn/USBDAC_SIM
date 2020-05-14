#pragma once
#ifdef HJKSIMPLEDLL_EXPORTS
#define HJKSIMPLEDLL_API __declspec(dllexport)
#else
#define HJKSIMPLEDLL_API __declspec(dllimport)
#endif
#define IN_RQUEUE_LEN	4
#define OUT_RQUEUE_LEN	4
//int32 in func
extern "C" HJKSIMPLEDLL_API void   initQueueBuf(int ch);
extern "C" HJKSIMPLEDLL_API void queueBufStart(__int32 *allChIn1, __int32 *out);

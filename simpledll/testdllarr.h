#pragma once
#ifdef HJKSIMPLEDLL_EXPORTS
#define HJKSIMPLEDLL_API __declspec(dllexport)
#else
#define HJKSIMPLEDLL_API __declspec(dllimport)
#endif
#define IN_RQUEUE_LEN	4
#define OUT_RQUEUE_LEN	4
//int32 in func
extern "C" HJKSIMPLEDLL_API void  testBuf(int *in, int *out, int len);
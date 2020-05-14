#include "pch.h"
#include "testdllarr.h"
#include "pch.h"
#include <stdlib.h>
#include <process.h>


// DLL internal state variables:

static __int32 *outIntBuf;
static __int32      *inData;
static int			chNum;
static bool			haveInit;
//需要初始化设置
static UCHAR sizeOfInt32;
static int									MaxPktSize;

//同步相关
//只有获取数据后，in线程才能入队，且入队之前，数据不能变，
static HANDLE			dataInEvent;
//在填充一个节点的时候不能pull；
static int				num;//计数

//函数声明
//void queueBufStart(__int32 *allChIn1, int ch, __int32 *out);
void testIn(void *);
void testFormat(void *);
void testOut(void *);

void initQueueBuf(int ch)
{
	haveInit = false;
	chNum = ch;
	inData = NULL;
	
	








	
	//同步相关
	num = 0;
	dataInEvent = CreateEvent(NULL, false, false, NULL);


	_beginthread(testIn, 0, NULL);
	

	haveInit = true;
}
//必须首先设置通道数
void queueBufStart(__int32 *allChIn1, __int32 *out)
{
	/*if (!haveInit)
	{
		initQueueBuf(ch, outBytes);
		out[0] = 123456;
	}*/

	/*else
	{*/
		//获取int型的数组地址，获取后才能入队
		inData = allChIn1;
		//获取数据事件发生
		SetEvent(dataInEvent);
		outIntBuf = out;

		//out[0] = 654321;
	/*}*/

}

//输入为所有用到的一次采样数据,输入为chNum个__int32的数组
void testIn(void *)
{
	while (1)
	{
		WaitForSingleObject(dataInEvent, INFINITE);
		for (int i = 0; i < chNum; i++)
		{
			inData[i] += 3000;
		}
		memcpy(outIntBuf, inData, chNum * 4);
	}
}

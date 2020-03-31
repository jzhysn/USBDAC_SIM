#include "pch.h"
#include <stdlib.h>
#include <process.h>
#include "hjkUsbXferdll.h"
#include "HjkDataDefine.h"
#include "RingBufCPP.h"
#include "hjkDataFormat.h"
// DLL internal state variables:
static qnodeChInt32		inNode[IN_RQUEUE_LEN];
static qnodeChInt32		outNode[OUT_RQUEUE_LEN];
static RingBufCPP<qnodeChInt32, IN_RQUEUE_LEN> inRQue;
static RingBufCPP<qnodeChInt32, OUT_RQUEUE_LEN> outRQue;
static qnodeChInt32 *nodeIn;
static qnodeChInt32 *nodeOut;
static qnodeChInt32 *nodeInbuf;
static qnodeChInt32 *nodeOutbuf;
static __int32 *outIntBuf;
static __int32      *inData;
static int			chNum;
static bool			haveInit;
//需要初始化设置
static UCHAR sizeOfInt32;
static int									MaxPktSize;
static HjkDataFormat						hjkdataf(0);
//同步相关
//只有获取数据后，in线程才能入队，且入队之前，数据不能变，
static HANDLE			dataInEvent;
//在填充一个节点的时候不能pull；
static int				num;//计数

//函数声明
void queueBufStart(__int32 *allChIn1, int ch, __int32 *out);
void testIn(void *);
void testFormat(void *);
void testOut(void *);

void initQueueBuf(int ch, int outBytes)
{
	haveInit = false;
	chNum = ch;
	inData = NULL;
	sizeOfInt32 = sizeof(__int32);
	//连接USB设置该值
	MaxPktSize = outBytes;
	//1.初始化输入
	for (int i = 0; i < IN_RQUEUE_LEN; i++)
	{
		//datalen一般为通道数
		inNode[i].datalen = chNum ;
		inNode[i].data = (__int32 *)malloc(inNode[i].datalen*sizeOfInt32);
		
		//入队
		inRQue.add(inNode[i]);
	}



	//2.初始化节点缓存	
	nodeInbuf = new qnodeChInt32;
	nodeOutbuf = new qnodeChInt32;
	//outIntBuf = (__int32 *)malloc(MaxPktSize);

	


	//初始化输出
	for (int i = 0; i < OUT_RQUEUE_LEN; i++)
	{
		outNode[i].datalen = MaxPktSize/sizeOfInt32;
		outNode[i].data = (__int32 *)malloc(MaxPktSize);
		
		outRQue.add(outNode[i]);
	}
	//同步相关
	num = 0;
	dataInEvent = CreateEvent(NULL, false, false, NULL);
	

	_beginthread(testIn, 0, NULL);
	_beginthread(testFormat, 0, NULL);
	_beginthread(testOut, 0, NULL);

	haveInit = true;
}
//必须首先设置通道数
 void queueBufStart(__int32 *allChIn1,__int32 *out, int ch, int outBytes)
{
	 if (!haveInit)
	 {
		initQueueBuf(ch, outBytes);
		out[0] = 123456;
	 }
		
	else
	{ 
		//获取int型的数组地址，获取后才能入队
		inData = allChIn1;
		//获取数据事件发生
		SetEvent(dataInEvent);
		outIntBuf = out;
		
		//out[0] = 654321;
	}
	
}

//输入为所有用到的一次采样数据,输入为chNum个__int32的数组
 void testIn(void *)
{
	while(1)
	{

		if (chNum > 0)
		{
			WaitForSingleObject(dataInEvent, INFINITE);
			nodeIn = (qnodeChInt32 *)inRQue.getHead();
			memcpy(nodeIn->data,inData,nodeIn->datalen * sizeOfInt32);
			
			//add第二个参数为true，可能造成数据丢失！！
			inRQue.add(*nodeIn, true);
			if (num < IN_RQUEUE_LEN)
				num++;
			else num = IN_RQUEUE_LEN;
			/*
			if (inRQue.add(*nodeIn,true))
			{
				if(num < IN_RQUEUE_LEN)
					num++;
				else num = IN_RQUEUE_LEN;
				
			}
			else
			{
				//printf("add head fail");
			}
			*/
		}
	}
}
 void  testFormat(void *)
{
	UINT index = 0;
	UINT x = 0;


	while (1)
	{
		if (!inRQue.isEmpty() && num== IN_RQUEUE_LEN)
		{
			//获取队头的节点，注意nodeOut可能的改变引起的不安全
			nodeOut = outRQue.getHead();
			if (index < nodeOut->datalen*sizeOfInt32)
			{
				inRQue.pull(nodeInbuf);//将队头的数据传入一个缓存

				//处理数据
				//放入inNode，直到tempMidNode满,
				index += hjkdataf.waveDataFormat(nodeInbuf, nodeOut, index);
				//for (int i = 0; i < nodeOut->datalen; i += (sizeof(dacData)))
				//	printf("out data is %d\n", *(dacData*)(nodeOut->data + index+i));


			}

			else ////当tempMidNode中的数据满时，送入输出队列
			{
				//索引重置
				index = 0;

				if (!outRQue.isFull())
				{
					//写入时保护
					outRQue.add(*nodeOut, false);
					//ReleaseSemaphore(midNode2OutQSem, 1, NULL);//传入队列后，信号量+1
				}
				else
				{
					outRQue.add(*nodeOut, true);
					//ReleaseSemaphore(midNode2OutQSem, 1, NULL);//传入队列后，信号量+1
				//	printf("add nodeOut overwrite!\n");
				}

			}

		}
		else
		{
			//等待超时，输出debug信息等，时间太长USB设备进入省电模式等其他操作
		}
	}
}
 void  testOut(void *)
{

	int i = 0;
	while (1)
	{
		if (!outRQue.isEmpty() && num == IN_RQUEUE_LEN)//队列非空就pull
		{
			if (outRQue.pull(nodeOutbuf))
			{
				memcpy(outIntBuf, nodeOutbuf->data, nodeOutbuf->datalen*sizeOfInt32);

				//for (i = 0; i < nodeOutbuf->datalen; i+=sizeof(__int32))
				//	printf("nodeOutbuf data is %d\n", *(dacData *)(nodeOutbuf->data + i));
			//	if (hjkUdev.USBDevice->DeviceCount() > 0)
				//	usbxfer.xferDataToUSBEp(hjkUdev.USBDevice->BulkOutEndPt, nodeOutbuf->data, nodeOutbuf->datalen, hjkOutOvLap[outRQue.numElements()]);

			}
		}
	}
}
//输入为一次采样所有通道，且不用建立新的队列
 void dacDataIn(void *)
{
	while (1) 
	{

	
		if (chNum > 0)
		{
			//nodeIn = (qnodeChInt32 *)inRQue.getHead();
			nodeIn->datalen = chNum;
			nodeIn->data = inData;
			inRQue.add(*nodeIn);

		}
	}
}
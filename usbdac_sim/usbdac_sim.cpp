// usbdac_sim.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include "usbdac_sim.h"

#include "sine_wave.h"


#include "InterfaceOfUSB.h"
#include "HjkDataDefine.h"
#include "HjkUSBInterface.h"
#include "HjkUSBDevice.h"
#include "HjkUSBXfer.h"

#define interfaceofusb 0
using namespace std;
//定义接口类
//定义缓存
static UCHAR			*inQueBuf;
static UCHAR			*outQueBuf;
//定义节点
static QUEUE_NODE		inNode[IN_RQUEUE_LEN];
static QUEUE_NODE		midNode;
static QUEUE_NODE		outNode[OUT_RQUEUE_LEN];
static QUEUE_NODE		*temNode;
//static InterfaceOfUSB interOfUSB(4, 2);//4和2目前无用，源程序默认为4和2（03.23）
//static InterfaceOfUSB interOfUSB(&inNode[0], &midNode,0,0,0,0);
static InterfaceOfUSB interOfUSB;
static HjkUSBInterface <QUEUE_NODE_A, QUEUE_NODE_B,IN_RQUEUE_LEN, OUT_RQUEUE_LEN> hjkInterf;
static HANDLE inMutex[IN_RQUEUE_LEN];
static HANDLE outMutex[OUT_RQUEUE_LEN];
static HANDLE *temMutex;
static HANDLE hjkMutex;
static OVERLAPPED hjkInOvLap[IN_RQUEUE_LEN];
static OVERLAPPED hjkOutOvLap[OUT_RQUEUE_LEN];
static HjkDataFormat hjkdataf(0);
static UINT inQueIndex;
//方法1
static RingBufCPP<QUEUE_NODE_A, IN_RQUEUE_LEN> inQue;
static RingBufCPP<QUEUE_NODE_B, OUT_RQUEUE_LEN> outQue;
static QUEUE_NODE_A *nodea;
static QUEUE_NODE_B  *nodeb;
static QUEUE_NODE_A *nodeabuf;
static QUEUE_NODE_B *nodebbuf;
//方法2 interfaceofusb 
static RingBufCPP<QUEUE_NODE, IN_RQUEUE_LEN> inRQue;
static RingBufCPP<QUEUE_NODE, OUT_RQUEUE_LEN> outRQue;
static QUEUE_NODE *nodeIn;
static QUEUE_NODE *nodeOut;
static QUEUE_NODE *nodeInbuf;
static QUEUE_NODE *nodeOutbuf;
//hjkusb
HjkUSBDevice hjkUdev;
HjkUSBXfer usbxfer;
//static bool  first;
void testIn(void *);
void testFormat(void *);
void testOut(void *);
unsigned __stdcall testThreadDatain(void *);
unsigned __stdcall  testXferDataToUSB(void *);
int _tmain(int argc, _TCHAR* argv[])
{
	/*******************字符流设置测试2020.03.***********/
	/*
	char wavBuf[512];
	stringbuf sbuf;
	sbuf.pubsetbuf(wavBuf,512);
	streamsize i = sbuf.in_avail();
	char sen []={65,66,67,68};
	sbuf.sputn(sen,10);
	sbuf.str();

	char wBuf[32];//波形的缓冲数组
	
	wave(wBuf,sizeof(wBuf));
	cout<<sbuf.str();
	*/
	//异步操作初始化
	//boolContr = false;
	
	//hjkOvLap.hEvent = CreateEvent(NULL, false, false, NULL);
	//hjkOvLap.hEvent = CreateMutex(NULL, false, NULL);
	//hjkOvLap.hEvent = CreateSemaphore(NULL, 1, 10, NULL);
	inQueIndex = 0;
	//控制第一次输出。！！若不用first控制初始值输出，可以将所有初始数据设为0
	//first = true;
	hjkMutex = CreateMutex(NULL, false, NULL);
	//1.初始化输入,缓存长度为实际字节长度
	for (int i = 0; i < IN_RQUEUE_LEN; i++)
	{
		inMutex[i] = CreateMutex(NULL, false, NULL);
		hjkInOvLap[i].hEvent = CreateEvent(NULL, false, false, NULL);
		inNode[i].datalen = 4 * sizeof(dacData);
		inNode[i].data = (UCHAR *)malloc(inNode[i].datalen );

		//方法2
		inRQue.add(inNode[i]);
		
		
	}
	
	//inQueBuf =(UCHAR *) malloc(IN_RQUEUE_LEN*inNode.datalen);
	//inNode.data = inQueBuf;//初始地址
	//thread 
	
	//2.初始化接口
	//注意长度的定义要准确
	midNode.datalen = inNode[0].datalen * 5;
	midNode.data = (UCHAR *)malloc(midNode.datalen * sizeof(dacData));

	nodeabuf = new QUEUE_NODE_A;
	nodebbuf = new QUEUE_NODE_B;
	nodeInbuf = new QUEUE_NODE;
	nodeOutbuf = new QUEUE_NODE;


	

	//3.初始化USB设备
	
	//HjkUSBDevice hUdevice;
	//初始化输出
	for(int i = 0; i <OUT_RQUEUE_LEN;i++)
	{
		outMutex[i] = CreateMutex(NULL, false, NULL);
		outNode[i].datalen = inNode[i].datalen * 5;
		outNode[i].data = (UCHAR *)malloc(outNode[i].datalen );
		hjkOutOvLap[i].hEvent = CreateEvent(NULL, false, false, NULL);
		//方法2
		
		outRQue.add(outNode[i]);
	}
	//outQueBuf = (UCHAR *)malloc(OUT_RQUEUE_LEN*outNode.datalen);
	//初始化和设定数据处理类
	//hjkdataf.waveDataFormat(&inNode, &midNode, 0);
	
	//4.启动数据发生线程	
	/*
	_beginthreadex(
		NULL,					//安全性NULL，不能被继承
		0,						//默认堆栈大小	
		testThreadDatain,		//函数入口地址
		NULL,					//参数列表
		0,						//初始化状态，0 for running or CREATE_SUSPENDED for suspended
		NULL					//Points to a 32-bit variable that receives the thread identifier. Might be NULL, in which case it is not used.
	);*/
	//5.启动接口的数据输入线程、处理线程和数据输出线程

	_beginthread(testIn, 0, NULL);
	_beginthread(testFormat, 0, NULL);
	_beginthread(testOut, 0,NULL);
	//6.启动USB传输线程
	//_beginthread(testXferDataToUSB, 0, NULL);

	while (1);
	return 0;
}

static void  testIn(void *)
{
	int i = 0;
	dacData dda = 0;
#ifdef interfaceofusb
	while (1)
	{
		


		//	WaitForSingleObject(inMutex[index], INFINITE);//需要等待数据生产者产生数据
	//获取头节点地址
		nodeIn = (QUEUE_NODE *)inRQue.getHead();
		WaitForSingleObject(hjkMutex, INFINITE);
		for (i = 0; i < nodeIn->datalen; i += (sizeof(dacData)))
		{
			
			memcpy(nodeIn->data + i, &dda, sizeof(dacData));
			//printf("in data is %d\n", *(dacData*)(nodeIn->data + i));

			dda++;
		}
		ReleaseMutex(hjkMutex);
		Sleep(50);
		//入队
		inRQue.add(*nodeIn);
		
	}	
#else
	
	while (1)
	{		
		

			//	WaitForSingleObject(inMutex[index], INFINITE);//需要等待数据生产者产生数据
			//获取头节点地址
			nodea = (QUEUE_NODE_A *) inQue.getHead();
			WaitForSingleObject(hjkMutex, INFINITE);
			for (i = 0; i < nodea->datalen; i += (sizeof(dacData)))
			{
				//uc = (UCHAR *)&dda;
				memcpy(nodea->data + i, &dda, sizeof(dacData));
			//	printf("in data is %d\n", *(dacData*)(nodea->data + i));

				dda++;
			}
			ReleaseMutex(hjkMutex);
			Sleep(50);
			//入队
			inQue.add(*nodea);

	}
#endif
}
static void  testFormat(void *)
{
	int index = 0;
	UINT x = 0;

#ifdef interfaceofusb
	while (1)
	{
		if (!inRQue.isEmpty())
		{
			//获取队头的节点
			nodeOut = outRQue.getHead();			
			if (index < nodeOut->datalen)
			{
				//WaitForSingleObject(inQ2MidNodeSem, TIMEOUT_MS);//信号量为0时无法继续执行
				inRQue.pull(nodeInbuf);//将队头的数据传入一个缓存
				//处理数据
				//放入inNode，直到tempMidNode满
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
					printf("add nodeOut overwrite!\n");
				}		
				
			}

		}
		else
		{
			//等待超时，输出debug信息等，时间太长USB设备进入省电模式等其他操作
		}
	}
#else
	while (1)
	{
		//传入处理方法，处理队头节点数据
		//interOfUSB.interfaceStart(&hjkdataf,&inMutex[0],&outMutex[0]);
		//hjkInterf.interfaceStart(&hjkdataf, &inMutex[0], &outMutex[0]);
		/*********test***********/
		if (!inQue.isEmpty())
		{
			//获取队头的节点
			nodeb = outQue.getHead();
			//outRQueue.pull(tempMidNode);
			if (index < nodeb->datalen)
			{
				//WaitForSingleObject(inQ2MidNodeSem, TIMEOUT_MS);//信号量为0时无法继续执行
				inQue.pull(nodeabuf);//将队头的数据传入一个缓存
				//处理数据
				//放入inNode，直到tempMidNode满
				index += hjkdataf.waveDataFormat(nodeabuf, nodeb, index);
				//for (int i = 0; i < nodea->datalen; i += (sizeof(dacData)))
				//	printf("out data is %d\n", *(dacData*)(nodeb->data + index+i));
				 

			}

			else ////当tempMidNode中的数据满时，送入输出队列
			{
				//索引重置
				index = 0;

				if (!outQue.isFull())
				{
					//写入时保护

					outQue.add(*nodeb, false);
					
					//ReleaseSemaphore(midNode2OutQSem, 1, NULL);//传入队列后，信号量+1
				}
				else
				{
					outQue.add(*nodeb, true);
					//ReleaseSemaphore(midNode2OutQSem, 1, NULL);//传入队列后，信号量+1
					printf("add tempMidNode overwrite!\n");
				}
			}

		}
		else
		{
			//等待超时，输出debug信息等，时间太长USB设备进入省电模式等其他操作
		}

	}
#endif
}
static void  testOut(void *)
{
	
	int i = 0;
#ifdef interfaceofusb
	
		while (1)
		{
			if (!outRQue.isEmpty())//队列非空就pull
			{
				if (outRQue.pull(nodeOutbuf))
				{
					for (i = 0; i < nodeOutbuf->datalen; i += sizeof(dacData))
							printf("nodeOutbuf data is %d\n", *(dacData *)(nodeOutbuf->data + i));
					if(hjkUdev.USBDevice->DeviceCount() > 0)
					usbxfer.xferDataToUSBEp(hjkUdev.USBDevice->BulkOutEndPt, nodeOutbuf->data, nodeOutbuf->datalen, hjkOutOvLap[outRQue.numElements()]);
						
				}
			}
		}
	
#else
	while (1)
	{
		/*for (i = 0; i < OUT_RQUEUE_LEN; i++)
		{
			interOfUSB.interfaceForUSBOut(outNode[i]);
		}

		*/
		if (!outQue.isEmpty())//队列非空就pull
		{
			outQue.pull(nodebbuf);
			for (i = 0; i < nodebbuf->datalen; i += sizeof(dacData))
				printf("out data is %d\n",*(dacData *)(nodebbuf->data+i));
		}
		
		
	}
#endif
}
unsigned __stdcall testThreadDatain(void *)
{
	dacData dda = 0;
	UINT i = 0;
	//int index = 0;
	UCHAR *uc;

		
	while (1)
	{
		//SetEvent(hjkOvLap.hEvent);
		//重复向队列中填入数据
		
			if (inQueIndex < IN_RQUEUE_LEN)
			{
				
			//	WaitForSingleObject(inMutex[index], INFINITE);//需要等待数据生产者产生数据
				
				for (i = 0; i < inNode[inQueIndex].datalen; i += (sizeof(dacData)))
				{
					//uc = (UCHAR *)&dda;
					memcpy(inNode[inQueIndex].data + i, &dda, sizeof(dacData));
					printf("in data is %d\n", *(dacData*)(inNode[inQueIndex].data + i));
					dda++;
				}
				//	inNode.data += inNode.datalen;
				Sleep(1);
				//temNode = &inNode[inQueIndex];
				//temMutex = &inMutex[index];
				
				SetEvent(hjkInOvLap[inQueIndex].hEvent);//事件发生:产生一个节点
				//memset((dacData*)inNode.data, 0xef, inNode.datalen*sizeof(dacData));//填充固定的数据

				
				WaitForSingleObject(inMutex[inQueIndex],INFINITE);
				inQueIndex++;
	
		//		ReleaseMutex(inMutex[index]);//写入后可读
			}
			else inQueIndex = 0;
		
	}
	
	return 1;
}
unsigned __stdcall  testXferDataToUSB(void *)
{
	while (1)
	{
		
		//memset(&outNode, 0x12, outNode.datalen * sizeof(dacData));
		
	}
	return 1;
}

//输入为一个2维数组，ch为通道数，n为每通道采样数
static void dacDataArrIn(__int32 ** arr2D, int ch, int n)
{
	//循环获取每次采样所有通道的数据
	for (int i = 0; i < n;i++)
	{

	}
}
//输入为所有用到的一次采样数据
static void dacDataArrIn(__int32 * allChIn1, int ch)
{
	int i = 0;
	dacData dda = 0;
	while (1)
	{



		
	//获取头节点地址
		nodeIn = (QUEUE_NODE *)inRQue.getHead();
		WaitForSingleObject(hjkMutex, INFINITE);
		for (i = 0; i < nodeIn->datalen; i += (sizeof(dacData)))
		{

			memcpy(nodeIn->data + i, &dda, sizeof(dacData));
			//printf("in data is %d\n", *(dacData*)(nodeIn->data + i));

			dda++;
		}
		ReleaseMutex(hjkMutex);
		Sleep(50);
		//入队
		inRQue.add(*nodeIn);

	}
}
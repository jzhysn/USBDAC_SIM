#include "pch.h"
#include <stdlib.h>
#include <process.h>
#include <iostream>
#include <time.h>
#include "hjkUsbXferdll.h"
#include "HjkDataDefine.h"
#include "RingBufCPP.h"
#include "hjkDataFormat.h"
#include "HjkUSBDevice.h"
// DLL internal state variables:
static qnodeChInt32								inNode[IN_RQUEUE_LEN];
static qnodeChInt32								outNode[OUT_RQUEUE_LEN];
static RingBufCPP<qnodeChInt32, IN_RQUEUE_LEN>	inRQue;
static RingBufCPP<qnodeChInt32, OUT_RQUEUE_LEN>	outRQue;
static qnodeChInt32								*nodeIn;
static qnodeChInt32								*nodeOut;
static qnodeChInt32								*nodeInbuf;
static qnodeChInt32								*nodeOutbuf;
static __int32									*outIntBuf;
static __int32									*inData;
static int										chNum;
static bool										haveInit;
//需要初始化设置
static UCHAR									sizeOfInt32;
static int										MaxPktSize;
static long										xferUSBDataSize;
//需要放入初始化函数中
static HjkDataFormat							hjkdataf;
//同步相关
//只有获取数据后，in线程才能入队，且入队之前，数据不能变，
static HANDLE									dataInEvent;
static HANDLE									dataInMutex;
//在填充一个节点的时候不能pull；
static int										num;//计数
//usb相关
static string									hjkUSBdevices;
static HjkUSBDevice								hjkUSB;
static CCyBulkEndPoint							*hjkBulkOutEndpt;


static int xferIntNum;
//static OVERLAPPED								hjkOutOvLap[OUT_RQUEUE_LEN];
//static UCHAR									hjkOutContxt[OUT_RQUEUE_LEN];
//函数声明
//void queueBufStart(__int32 *allChIn1, int ch, __int32 *out);
void testIn(void *);
void testFormat(void *);
void testOut(void *);
void abortXfer(int pending);
/*
usb设备初始化操作
查找usb设备，如果设备数小于1，则退出
等待选择要使用到设备
选择后，如果设备不可用，退出
若设备可用
设置端口和传输帧信息
初始化完成返回
*/
long  initHjkUSBXfer(string usbdevices, int ppx)
{
	
	string endinfos;
	long len ;
	int slecd = -1;
	int n = hjkUSB.getHjkUSBDevice(&usbdevices);
	if (n > 0)
	{
#ifdef CMD
		cout << usbdevices;
		while (slecd < 0 || slecd >= n)
		{

			cout << "Slect one  hjkUSB device to xfer." << endl;
			cin >> slecd;
		}
#else 
		slecd = 0;
#endif
		if (hjkUSB.USBDevice->Open(slecd))
		{
			//获取块端口，设置最大传输字节数
			hjkBulkOutEndpt = hjkUSB.USBDevice->BulkOutEndPt;

			if (hjkBulkOutEndpt == NULL)
			{
				cout << "The USB bulk end point is null, program exit." << endl;
				return 0;
			}
			//每次传输的字节数为xferlen，不超过4m bytes。
			len = hjkBulkOutEndpt->MaxPktSize*ppx;
			if (len > 0x400000)
			{
				len = 0x400000;
			}
			hjkBulkOutEndpt->SetXferSize(len);
			return len;
		}
		else
		{
			cout << "USB device is not a hjkUSB device, program exit." << endl;
			return 0;
		}

	}
	else
	{
		cout << "USB device is 0, program exit,please check the USB device or driver."<<endl;
		return 0;
	}
	//

}
void initQueueBuf(int ch, int outBytes)
{
	haveInit = false;
	chNum = ch;
	inData = NULL;
	sizeOfInt32 = sizeof(__int32);
	//连接USB设置该值
	
	xferUSBDataSize=initHjkUSBXfer(hjkUSBdevices, chNum);
	if (xferUSBDataSize == 0)
	{
		cout << "USB device init failed, progarma exit. " << endl;
		exit(1);
	}
	//hjkOutOvLap.hEvent = CreateEvent(NULL, false, false, L"hjk_bulk_out");

	//MaxPktSize = outBytes;
	//输入数据处理方法
	//hjkdataf = HjkDataFormat(ch, 0);

	//2.初始化节点缓存
	//*inNode = new qnodeChInt32;
	nodeInbuf = new qnodeChInt32;
	nodeOutbuf = new qnodeChInt32;


	//1.初始化输入节点
	for (int i = 0; i < IN_RQUEUE_LEN; i++)
	{
		//datalen一般为通道数
		inNode[i].datalen = chNum ;
		inNode[i].data = (__int32 *)malloc(inNode[i].datalen*sizeOfInt32);
		
		//入队
		inRQue.add(inNode[i]);
		
	}
	//清空队列
	for (int i = 0; i < IN_RQUEUE_LEN; i++)
		inRQue.pull(nodeInbuf);
	
	
	


	
	//outIntBuf = (__int32 *)malloc(MaxPktSize);

	


	//初始化输出
	for (int i = 0; i < OUT_RQUEUE_LEN; i++)
	{
		outNode[i].datalen = xferUSBDataSize /sizeOfInt32;
		outNode[i].data = (__int32 *)malloc(xferUSBDataSize);
		//usb异步相关
		outNode[i].ovLap.hEvent = CreateEvent(NULL, false, false, NULL);
		
		outRQue.add(outNode[i]);
	
	}
	for (int i = 0; i < OUT_RQUEUE_LEN; i++)	
		outRQue.pull(nodeOutbuf);

	
	//同步相关
	num = 0;
	dataInEvent = CreateEvent(NULL, false, false, NULL);
	dataInMutex = CreateMutex(NULL, false, NULL);
	//debug
	xferIntNum = 0;
	

	//_beginthread(testIn, 0, NULL);
	_beginthread(testFormat, 0, NULL);
	_beginthread(testOut, 0, NULL);

	haveInit = true;
}
//必须首先设置通道数
//allChIn1是输入的地址，ch是出入到32位int数据的个数（一个节点的int个数），
//out是输出地址，outBytes是输出（一个节点）的字节数
 void queueBufStart(__int32 *allChIn1,__int32 *out, int ch, int outBytes)
{
	 if (!haveInit)
	 {
		initQueueBuf(ch, outBytes);
		out[0] = 123456;//测试数据
	 }
		
	else
	{ 
		//获取int型的数组地址，获取后才能入队
		 WaitForSingleObject(dataInMutex, INFINITE);
		 inData = allChIn1;
		 nodeIn = (qnodeChInt32 *)inRQue.getHead();
		 memcpy(nodeIn->data, inData, nodeIn->datalen * sizeOfInt32);
		 if (!inRQue.add(*nodeIn, false))
		 {
			 cout << "Data in false." << endl;
			 //可不停到添加直到超时错误
		 }
		//获取数据事件发生
		//SetEvent(dataInEvent);
		outIntBuf=out  ;
		ReleaseMutex(dataInMutex);
		
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
			//WaitForSingleObject(dataInEvent, INFINITE);
			//inRQue.pull(nodeIn);
			WaitForSingleObject(dataInMutex, INFINITE);
			nodeIn = (qnodeChInt32 *)inRQue.getHead();
			memcpy(nodeIn->data,inData,nodeIn->datalen * sizeOfInt32);
			
			//add第二个参数为true，可能造成数据丢失！！
			if (!inRQue.add(*nodeIn, false))
			{
				cout << "Data in false." << endl;
			}
			ReleaseMutex(dataInMutex);
			/*	
			if (num < IN_RQUEUE_LEN)
				num++;
			else num = IN_RQUEUE_LEN;
			*/
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
		if (!inRQue.isEmpty() )
		{
			//获取队头的节点，注意nodeOut可能的改变引起的不安全
			if(index == 0)nodeOut = outRQue.getHead();
			if (index < nodeOut->datalen*sizeOfInt32)
			{
				inRQue.pull(nodeInbuf);//将队头的数据传入一个缓存

				//处理数据
				//放入inNode，直到tempMidNode满,
				index += hjkdataf.waveDataFormat(nodeInbuf, nodeOut, index);
				//for (int i = 0; i < nodeOut->datalen; i += (sizeof(dacData)))
				//	printf("out data is %d\n", *(dacData*)(nodeOut->data + index+i));


			}
			else if(index >= nodeOut->datalen*sizeOfInt32)////当tempMidNode中的数据满时，送入输出队列
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
					cout << "Data out false." << endl;
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
		 if (!outRQue.isEmpty())//队列非空就pull
		 {
			 if (outRQue.pull(nodeOutbuf))
			 {
				 //usb异步传输
				 nodeOutbuf->context = hjkBulkOutEndpt->BeginDataXfer(
					 (UCHAR *)nodeOutbuf->data, xferUSBDataSize, &nodeOutbuf->ovLap);
				 if (hjkBulkOutEndpt->NtStatus || hjkBulkOutEndpt->UsbdStatus) // BeginDataXfer failed
				 {
					 //Display(String::Concat("Xfer request rejected. NTSTATUS = ", EndPt->NtStatus.ToString("x")));
				 //	AbortXferLoop(i + 1, buffers, isoPktInfos, contexts, inOvLap);
					 return;
				 }

				 if (!hjkBulkOutEndpt->WaitForXfer(&nodeOutbuf->ovLap, XFER_TIME_OUT))
				 {
					 hjkBulkOutEndpt->Abort();
					 if (hjkBulkOutEndpt->LastError == ERROR_IO_PENDING)
						 WaitForSingleObject(nodeOutbuf->ovLap.hEvent, 2000);
				 }

				 if (hjkBulkOutEndpt->FinishDataXfer(
					 (UCHAR *)nodeOutbuf->data, xferUSBDataSize, &nodeOutbuf->ovLap, nodeOutbuf->context))
				 {
					 //debug
					 /*
					 xferIntNum += xferUSBDataSize / 4;

					 if (xferIntNum % 1048576 == 0)
					 {
						 cout << "xferIntNum = " << xferIntNum << ",time is " << time(NULL) << endl;
					 }
					 */
				 }


				 //memcpy(outIntBuf, nodeOutbuf->data, nodeOutbuf->datalen*sizeOfInt32);

				 //for (i = 0; i < nodeOutbuf->datalen; i+=sizeof(__int32))
				 //	printf("nodeOutbuf data is %d\n", *(dacData *)(nodeOutbuf->data + i));
			 //	if (hjkUdev.USBDevice->DeviceCount() > 0)
				 //	usbxfer.xferDataToUSBEp(hjkUdev.USBDevice->BulkOutEndPt, nodeOutbuf->data, nodeOutbuf->datalen, hjkOutOvLap[outRQue.numElements()]);

			 }
		 }
	 }
	 //清理函数 

	 abortXfer(OUT_RQUEUE_LEN);
 }


 void abortXfer(int pending)
 {
	// xferUSBDataSize = hjkBulkOutEndpt->MaxPktSize * chNum;
	 hjkBulkOutEndpt->Abort();
	 for (int i = 0; i < OUT_RQUEUE_LEN; i++)
	 {
		 if (i < pending)
		 {
			 hjkBulkOutEndpt->WaitForXfer(&outNode[i].ovLap, XFER_TIME_OUT);
			 hjkBulkOutEndpt->FinishDataXfer(
				 (UCHAR *)outNode[i].data, xferUSBDataSize, &outNode[i].ovLap, outNode[i].context);
		 }
		 CloseHandle(outNode[i].ovLap.hEvent);
	 }
	 delete[] outNode;

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
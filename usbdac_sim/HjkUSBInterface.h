#pragma once
#ifndef __HJKUSBINTERFACE_H_
#define __HJKUSBINTERFACE_H_
#include "stdafx.h"
#include "RingBufCPP.h"

#include "usbdac_sim.h"
#include "HjkDataDefine.h"
#include "HjkDataFormat.h"


/*
InterfaceOfUSB接口实现双路缓冲队列(循环队列)，包含外部数据到接口的队列rToInterfaceQue,和接口到USB的队列rToUSBQue
外部由n个bytes组成的数据作为一个节点，只要rToInterfaceQue队列不满，就add一个节点放入，
否则删除最旧的节点元素再add（此处注意DEBUG），waveDataFormat函数负责从rToInterfaceQue
中取出队头节点，处理后放入一个临时输出节点，当节点满时，放入rToUSBQue队列中（队列满则删除最旧的节点元素）
临时的输出节点大小一般为MaxPktSize*PPX，即USB的数据包最大值*每次发送的数据包数。
！！注意：MaxPktSize*PPX要能被rToInterfaceQue的节点len整除，所以PPX可设为该节的len（但容易过大）
*/
#define TIMEOUT_MS INFINITE
using namespace std;
template <typename inType, typename outType, UINT inQueElems, UINT outQueElems>

class HjkUSBInterface
{
public:
	//输入队列
	 RingBufCPP<inType, inQueElems>		inRQueue;
	//一个长度为QUEUE_LEN的循环队列，队列中的节点元素为QUEUE_NODE类型
	 RingBufCPP<outType, outQueElems>		outRQueue;
private:
	//定义缓存
	UCHAR										*inQueBuf;
	UCHAR										*outQueBuf;

	//控制中间节点的索引
	UINT										index;
	HANDLE										inQ2MidNodeMutex;
	HANDLE										midNode2OutQMuetx;
	//设置信号量，控制中间节点读取和写入时不会与队列产生冲突
	HANDLE										inQ2MidNodeSem;
	HANDLE										midNode2OutQSem;

	static int									MaxPktSize;
	static int									PPX;
	static int									QueueSize;
	static int									TimeOut;
	static int									dacDataSize;
	//输入端点
	//static QUEUE_NODE							*inNode;
	//临时的输入节点
	inType							*tempInNode;
	//一个临时的输出队列节点，将处理后的一组数据不断放入
	outType							*tempMidNode;
	//一个临时的输出队列节点，将处理后的一组数据放入
	// QUEUE_NODE							*tempOutNode;


public:
	//InterfaceOfUSB(QUEUE_NODE * inNode, QUEUE_NODE * outNode);
	HjkUSBInterface(QUEUE_NODE * inNode, QUEUE_NODE * midNode, const UINT inNodeLen, const UINT outNodeLen, const UINT inQueLen, const UINT outQueLen) :
	tempInNode(inNode), tempMidNode(midNode)
{
	index = 0;
}

//InterfaceOfUSB::InterfaceOfUSB(QUEUE_NODE * inNode, QUEUE_NODE * outNode) :
	HjkUSBInterface() :
	index(0),
	inQ2MidNodeSem(CreateSemaphore(NULL, 0, inRQueue.getMaxEles(), NULL)),
	midNode2OutQSem(CreateSemaphore(NULL, 0, outRQueue.getMaxEles(), NULL))

{
	tempInNode = new inType;
	tempMidNode = new outType;
	//tempInNode->datalen = 4 * sizeof(dacData);
	//tempInNode->data = (UCHAR *)malloc(tempInNode->datalen * sizeof(dacData));
	//tempMidNode->datalen = tempInNode->datalen * 5;
	//tempMidNode->data = (UCHAR *)malloc(tempMidNode->datalen * sizeof(dacData));
	//setInRQueue();
	//setOutRQueue();
}

~HjkUSBInterface()
{
	CloseHandle(inQ2MidNodeSem);
	CloseHandle(midNode2OutQSem);
}
bool interfaceForIn(QUEUE_NODE inNode, HANDLE *mutex)
{
	if (!inRQueue.isFull())//add函数中已有判断，即当参数2 false时，队列满会返回错误
	{

		if (inRQueue.add(inNode, false))
		{
			ReleaseSemaphore(inQ2MidNodeSem, 1, NULL);//入队后信号量加1
			return true;
		}

		else//不会执行到的部分
		{
			printf("interfaceForIn rToInterfaceQue.add() failed.\n");
			return false;
		}
	}
	else
	{
		if (inRQueue.add(inNode, true))
		{
			ReleaseSemaphore(inQ2MidNodeSem, 1, NULL);//入队后信号量加1
			printf("add inNode overwrite!\n");
			return true;
		}

		else
		{
			printf("interfaceForIn rToInterfaceQue.add() overwrite failed.\n");
			return false;
		}

	}
}
bool interfaceStart(HjkDataFormat *hjkDataFormat, HANDLE *inMutex, HANDLE *outMutex)
{

	//队头数据传入时，要保证：此时队列非空，且队头节点没有在写入操作
	if (!inRQueue.isEmpty())
	{
		//获取队头的节点
		tempMidNode = outRQueue.getHead();
		//outRQueue.pull(tempMidNode);
		if (index < tempMidNode->datalen)
		{
			//WaitForSingleObject(inQ2MidNodeSem, TIMEOUT_MS);//信号量为0时无法继续执行
			inRQueue.pull(tempInNode);//将队头的数据传入一个缓存
			//处理数据
			//放入inNode，直到tempMidNode满
			index += hjkDataFormat->waveDataFormat(tempInNode, tempMidNode, index);

		}

		else ////当tempMidNode中的数据满时，送入输出队列
		{
			//索引重置
			index = 0;

			if (!outRQueue.isFull())
			{
				//写入时保护

				outRQueue.add(*tempMidNode, false);
				//ReleaseSemaphore(midNode2OutQSem, 1, NULL);//传入队列后，信号量+1
			}
			else
			{
				outRQueue.add(*tempMidNode, true);
				//ReleaseSemaphore(midNode2OutQSem, 1, NULL);//传入队列后，信号量+1
				printf("add tempMidNode overwrite!\n");
			}
		}

	}
	else
	{
		//等待超时，输出debug信息等，时间太长USB设备进入省电模式等其他操作
	}
	return true;
}
bool interfaceForUSBOut(QUEUE_NODE outNode)
{
	//rToUSBQue.pull(tempOutNode);//将队头的数据传入一个缓存

	if (!outRQueue.isEmpty())//pull函数内已有判断，当队列空时，返回false
	{
		WaitForSingleObject(midNode2OutQSem, TIMEOUT_MS);//信号量大于0，否则队头入队没有结束，无法出队
		if (outRQueue.pull(&outNode))//放入节点地址
		{
			return true;
		}
		else
		{
			printf("interfaceForUSBOut rToUSBQue.pull() failed.\n");
			return false;
		}
	}
	else
	{
		printf("rToUSBQue is empty.\n");
		return true;
	}

}

	





private:
	//设置输入节点,一般为nLen个bytes
	UINT setToInterfaceNode(const UINT nLen);
	//设置输出节点，一般为nLen个输入节点大小
	UINT setToUSBNode(const UINT nLen);
	//设置输入队列
	UINT setInRQueue()
	{
		//inRQueue.setMaxEles(qLen);
		//inRQueue = malloc(sizeof(QUEUE_NODE)*inRQueue.getMaxEles());
		inQueBuf = (UCHAR *)malloc(inRQueue.getMaxEles()*tempInNode->datalen);
		return sizeof(*inQueBuf);
	}
	//设置输出队列
	UINT setOutRQueue()
	{
		//outRQueue.setMaxEles(qLen);
		outQueBuf = (UCHAR *)malloc(outRQueue.getMaxEles()*tempMidNode->datalen);

		return sizeof(*outQueBuf);
	}
	//设置临时输出节点
	UINT setUSBNodeBuf();
	//处理波形数据
	QUEUE_NODE* waveDataFormat();
	//index 是midNode的有效字节索引，其值为字节数，即记录下一个写入位置,返回midNode的
	//处理inNode中的数据，将处理后的数据放入midNode的data中从index开始的内存中，返回处理后的数据大小.
	//注意：此处插入的数据不能超出midNode的范围！！（需要加强）
	UINT waveDataFormat(QUEUE_NODE * inNode, QUEUE_NODE * midNode, UINT index);
	//将处理后的波形数据放入队列中  
	void formatDataToRqueue(QUEUE_NODE *USBNodeBuf, QUEUE_NODE *formatDataNode);

};

#endif




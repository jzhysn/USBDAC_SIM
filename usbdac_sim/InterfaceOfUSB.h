#pragma once
#ifndef __INTERFACEOFUSB_H_
#define __INTERFACEOFUSB_H_
#include "usbdac_sim.h"
#include "HjkDataDefine.h"
#include "HjkDataFormat.h"
#include "RingBufCPP.h"

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


class InterfaceOfUSB
{
public:
	//输入队列
	RingBufCPP<QUEUE_NODE, IN_RQUEUE_LEN>		inRQueue;
	//一个长度为QUEUE_LEN的循环队列，队列中的节点元素为QUEUE_NODE类型
	RingBufCPP<QUEUE_NODE, OUT_RQUEUE_LEN>		outRQueue;
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
	 QUEUE_NODE							*tempInNode;
	//一个临时的输出队列节点，将处理后的一组数据不断放入
	 QUEUE_NODE							*tempMidNode;
	//一个临时的输出队列节点，将处理后的一组数据放入
	// QUEUE_NODE							*tempOutNode;
	

public:
	//InterfaceOfUSB(QUEUE_NODE * inNode, QUEUE_NODE * outNode);
	InterfaceOfUSB();
	
	InterfaceOfUSB(
		QUEUE_NODE		*inNode,//外部数据的入口
		QUEUE_NODE		*midNode,
		const UINT		inNodeLen,//外部数据的长度
		const UINT		outNodeLen,
		const UINT		inQueLen,
		const UINT		outQueLen
		);
	
	
	~InterfaceOfUSB(); 
	//输入线程中启动。输入inNode到rToInterfaceQue队列，只要输入队列不为空就添加节点,
	 bool interfaceForIn(QUEUE_NODE inNode,HANDLE *mutex);
	//接口线程中启动。输出rToInterfaceQue队列头节点到tempInNode、处理数据、输入tempMidNode到rToUSBQue队列
	 bool interfaceStart();
	 bool interfaceStart(HjkDataFormat *hjkDataFormat, HANDLE *inMutex,HANDLE *outMutex);
	//USB线程中启动。输出rToUSBQue队列头节点地址。
	 bool interfaceForUSBOut(QUEUE_NODE outNode);
	//USB线程中启动。输出rToUSBQue队列头节点。
	//static bool interfaceForUSBOut( CCyUSBEndPoint *EndPt, OVERLAPPED inOvLap);
	
	
private:
	//设置输入节点,一般为nLen个bytes
	UINT setToInterfaceNode(const UINT nLen);
	//设置输出节点，一般为nLen个输入节点大小
	UINT setToUSBNode(const UINT nLen);
	//设置输入队列
	UINT setInRQueue();
	//设置输出队列
	UINT setOutRQueue();
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
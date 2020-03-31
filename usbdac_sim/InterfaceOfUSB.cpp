#include "stdafx.h"
#include "RingBufCPP.h"
#include "InterfaceOfUSB.h"
/*波形数据和usb的接口
控制USB、DAC等硬件设备，并接收波形发生器的数据，经处理后发送给USB设备。接口有2个缓冲区in_buf（队列）和out_buf，缓冲区大小为n*len，len为一个波形一次的缓冲长度，且len为DAC_BIT的整数倍。将数据做如下处理：
	1）接收波形发生器的n组数据放在in_buf中，当in_buf满的时候，移除第一组数据，放入新的一组数据，直到填满，并循环执行，知道收到停止命令
	2）对x个数据转置后存入out_buf.
**/




/*
InterfaceOfUSB::InterfaceOfUSB(
	QUEUE_NODE		*inNode,
	const UINT		inNodeLen,
	const UINT		outNodeLen,
	const UINT		inQueLen,
	const UINT		outQueLen,
	UINT			MaxPktSize, 
	UINT			PPX
	)
	//:
	//USBDevice(USBDevice)
{
	//初始化成员
	this->inNode = inNode;
	this->MaxPktSize = MaxPktSize;
	this->PPX = PPX;
	this->tempInNode->datalen = inNode->datalen;
	//初始化队列
	//setToInterfaceNode(inNodeLen);//类初始化前会再外部定义
	setToUSBNode(outNodeLen);
	setInRQueue(inQueLen);
	setOutRQueue(outQueLen);
	setUSBNodeBuf();
	
	
	*/
	
	/*
	//初始化输出队列（因为是循环队列，可以不用初始化。）
	QUEUE_NODE qnode;
	qnode.datalen = QUEUE_NODE_LEN * dacDataSize;
	memset(qnode.data, 0xff, qnode.datalen);
	for (int i = 0; i < QUEUE_LEN; i++)
	{
		if (!rQue.add(qnode, true))
			printf("init queue faild \n");
	}
	*/

//}

InterfaceOfUSB::InterfaceOfUSB(QUEUE_NODE * inNode, QUEUE_NODE * midNode, const UINT inNodeLen, const UINT outNodeLen, const UINT inQueLen, const UINT outQueLen):
	tempInNode(inNode),tempMidNode(midNode)
{
	index = 0;
}

//InterfaceOfUSB::InterfaceOfUSB(QUEUE_NODE * inNode, QUEUE_NODE * outNode) :
InterfaceOfUSB::InterfaceOfUSB() :
	index(0), 
	inQ2MidNodeSem(CreateSemaphore(NULL, 0, inRQueue.getMaxEles(), NULL)),
	midNode2OutQSem(CreateSemaphore(NULL, 0, outRQueue.getMaxEles(), NULL))
	
{
	tempInNode = new QUEUE_NODE;
	tempMidNode = new QUEUE_NODE;
	tempInNode->datalen = 4 * sizeof(dacData);
	tempInNode->data = (UCHAR *)malloc(tempInNode->datalen * sizeof(dacData));
	tempMidNode->datalen = tempInNode->datalen * 5;
	tempMidNode->data = (UCHAR *)malloc(tempMidNode->datalen * sizeof(dacData));
	//setInRQueue();
	//setOutRQueue();
}

InterfaceOfUSB::~InterfaceOfUSB()
{
	CloseHandle(inQ2MidNodeSem);
	CloseHandle(midNode2OutQSem);
}
bool InterfaceOfUSB::interfaceForIn(QUEUE_NODE inNode, HANDLE *mutex)
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
bool InterfaceOfUSB::interfaceStart(HjkDataFormat *hjkDataFormat, HANDLE *inMutex, HANDLE *outMutex)
{
	
	//队头数据传入时，要保证：此时队列非空，且队头节点没有在写入操作
	if (!inRQueue.isEmpty())
	{
		
		
		if (index < tempMidNode->datalen)
		{	
			WaitForSingleObject(inQ2MidNodeSem, TIMEOUT_MS);//信号量为0时无法继续执行
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
				ReleaseSemaphore(midNode2OutQSem, 1, NULL);//传入队列后，信号量+1
			}
			else
			{
				outRQueue.add(*tempMidNode, true);
				ReleaseSemaphore(midNode2OutQSem, 1, NULL);//传入队列后，信号量+1
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
bool InterfaceOfUSB::interfaceForUSBOut(QUEUE_NODE outNode)
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
/*
bool InterfaceOfUSB::interfaceForUSBOut( CCyUSBEndPoint * EndPt,OVERLAPPED inOvLap)
{
	if (!outRQueue.isEmpty)
		{
			//rToUSBQue.pull(tempOutNode);//将队头的数据传入一个缓存
			//传出数据到sub
			if (outRQueue.pull(tempOutNode))//放入节点地址
			{
				
			}
			else printf("xferQueueNodeToUSB rToUSBQue.pull() failed.\n");

		}
		else
		{
			//等待超时，输出debug信息等，时间太长USB设备进入省电模式等其他操作
		}
	return true;
}
*/
//传输逻辑代码
/*
void xferThread()
{
	while (1) //输入inNode到rToInterfaceQue队列，只要输入队列不为空就添加节点,
	{
		if(!rToInterfaceQue.isFull)
			rToInterfaceQue.add(*inNode,false);
		else
		{
			rToInterfaceQue.add(*inNode, true);
			printf("add in node overwrite!\n");
		}
	}
	while (1)//输出rToInterfaceQue队列头节点到tempInNode、处理数据、输入tempMidNode到rToUSBQue队列
	{
		if (!rToInterfaceQue.isEmpty)
		{
			rToInterfaceQue.pull(tempInNode);//将队头的数据传入一个缓存
			//处理数据
			//循环放入，直到tempMidNode满
			for(UINT index = 0; index < tempMidNode->datalen;)
				index+=waveDataFormat(tempInNode, tempMidNode,index);

			//当tempMidNode中的数据满时，送入输出队列
			if (!rToUSBQue.isFull)
			{
				rToUSBQue.add(*tempMidNode, false);
			}
			else
			{
				rToUSBQue.add(*tempMidNode, true);
				printf("add tempMidNode overwrite!\n");
			}

		}
		else
		{
			//等待超时，输出debug信息等，时间太长USB设备进入省电模式等其他操作
		}
	}
	while (1)//输出rToUSBQue队列头节点
	{
		
		if (!rToUSBQue.isEmpty)
		{
			//rToUSBQue.pull(tempOutNode);//将队头的数据传入一个缓存
			//传出数据到sub
			xferQueueNodeToUSB(tempOutNode);

		}
		else
		{
			//等待超时，输出debug信息等，时间太长USB设备进入省电模式等其他操作
		}

		//
	}

}
*/

UINT InterfaceOfUSB::setInRQueue()
{
	//inRQueue.setMaxEles(qLen);
	//inRQueue = malloc(sizeof(QUEUE_NODE)*inRQueue.getMaxEles());
	inQueBuf = (UCHAR *)malloc(inRQueue.getMaxEles()*tempInNode->datalen);
	return sizeof(*inQueBuf);
}
UINT InterfaceOfUSB::setOutRQueue()
{
	//outRQueue.setMaxEles(qLen);
	outQueBuf = (UCHAR *)malloc(outRQueue.getMaxEles()*tempMidNode->datalen);

	return sizeof(*outQueBuf);
}
/*
QUEUE_NODE * InterfaceOfUSB::waveDataFormat()
{
	//获取inNode并处理
	if(!inRQueue.isEmpty())
	{
		inRQueue.pull(tempInNode);//将队头的数据传入一个缓存
		//处理数据
		{
			UINT size_x = tempInNode->datalen;
			//转为可为DAC使用的数据，
			//处理后数据的长度为size_x
		}
		
	}
	return nullptr;
}
*/
UINT InterfaceOfUSB::waveDataFormat(QUEUE_NODE * inNode, QUEUE_NODE * midNode, UINT index)
{
	//处理inNode中的数据
	//转为可为DAC使用的数据，
	//处理后数据的长度为size_x,存在tem中
	
	UINT size_x = inNode->datalen;
	UCHAR *tem = inNode->data;

	//处理过程


	//处理后检测index，不能超过midNode的datalen
	if (index + size_x > midNode->datalen -1)
	{
		printf("SERROR:out of the len of midNode's data.\n");
		exit(1);
	}
		
	//将处理结果写入从index位置开始的midNode中
	(UCHAR*)memcpy(midNode->data + index, tem, size_x);	
	
	return size_x;
}/*
UINT InterfaceOfUSB::setUSBNodeBuf()
{
	//初始化临时节点
	tempOutNode->datalen = MaxPktSize * PPX;
	memset(tempOutNode->data, 0xff, tempOutNode->datalen);
	return tempOutNode->datalen;
}
*/






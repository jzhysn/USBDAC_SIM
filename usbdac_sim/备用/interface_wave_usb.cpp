/*波形数据和usb的接口
控制USB、DAC等硬件设备，并接收波形发生器的数据，经处理后发送给USB设备。接口有2个缓冲区in_buf（队列）和out_buf，缓冲区大小为n*len，len为一个波形一次的缓冲长度，且len为DAC_BIT的整数倍。将数据做如下处理：
	1）接收波形发生器的n组数据放在in_buf中，当in_buf满的时候，移除第一组数据，放入新的一组数据，直到填满，并循环执行，知道收到停止命令
	2）对x个数据转置后存入out_buf.
**/

#include "stdafx.h"
#include <queue>
#include "RingBufCPP.h"
#include "interface_wave_usb.h"

using namespace std;


static dacData *inbuf;
static dacData *outbuf;
const static int inbuf_size = MIN_BUF_SIZE;//缓冲大小，每个数据大小由dacData决定
const static int outbuf_size = MIN_BUF_SIZE;


void interfaceToUSB() {}
bufSource *setBufsource(bufSource *bufs, int buf_len)
{
	bufs->dac_data_buf = (dacData *)calloc(buf_len, sizeof(dacData));
	if (bufs->dac_data_buf == NULL)
	{
		printf("interface_wave_usb exception,inbuf is null.\n");
		exit(1);
	}

	return bufs;

}
/*
dacData *setOutbuf(int outbuf_size)
{
	dacData *outbuf;
	outbuf = (dacData *)calloc(outbuf_size,sizeof(dacData));
	if(outbuf_size==NULL)
	{
		printf("interface_wave_usb exception,outbuf_size is null.\n");
		exit(1);
	}

	return outbuf_size;

}
*/
//向buf中添加数据
void addSample(bufSource *bufs, dacData dataSample)
{
	//dacData *databuf=bufs->dac_data_buf;
	//如果可以则写入数据
	if (bufs->next_pos < bufs->buf_len)//写入数据时的条件
	{
		//databuf+=bufs->next_pos*sizeof(dacData);
		//*databuf = data;
		bufs->dac_data_buf[bufs->next_pos] = dataSample;
		bufs->next_pos++;

	}
	else bufs->next_pos = 0;
}
//重新整理输入的数据
void dataFromat(bufSource *inbufs, bufSource *outbufs)
{
	outbufs = inbufs;
}
/*
处理后的数据作为队列元素，push入队列，
只要队列不满，就可以push，
若队列满，执行pop，元素送入固定缓存区。并报告向usb发送时有丢失数据
如果队列不为空，pop队列元素到usb设备，即发送队头元素后，删除队头
若队列为空，usb发送固定缓存区的元素数据。

*/
void bufToUSBLoop(std::queue<dacData> bufQueue, bufSource *bufs, dacData *tembuf, const int bufsize, bool ready)
{
	//填满队列
	while (bufQueue.size()!=MAX_QUEUE_SZ)
	{
		bufQueue.push(bufs->dac_data_buf[bufsize]);

	}
	for(int i = 0; i < bufQueue.size(); i++)
	{
		if(!bufQueue.empty())
		xferQueueData(bufQueue.front());
		else xferQueueData(tempbuf);
	}
	//队列满时，出队并进队
	if (bufQueue.size() == MAX_QUEUE_SZ) {}
	
	{
	
	}

}


static int PPX = PACKETS_PER_XFER;//每次传递的包数
static int QueueSize = QUEUE_SIZE;
static int TimeOut = TIMEOUT;
static UCHAR eptAddr = BULK_OUT_ADDR;

static UCHAR *outBuffer;
static UCHAR *outContext;

/*初始化一个端点*/
void initUSBEndPt()
{
	static CCyUSBEndPoint		*EndPt;
	long len = EndPt->MaxPktSize*PPX;//每次传输PPX个数据包
	EndPt->SetXferSize(len);
	// Allocate the arrays needed for queueing
	PUCHAR			*buffers = new PUCHAR[QueueSize];
	CCyIsoPktInfo	**isoPktInfos = new CCyIsoPktInfo*[QueueSize];
	PUCHAR			*contexts = new PUCHAR[QueueSize];
	OVERLAPPED		inOvLap[MAX_QUEUE_SZ];
	int i = 0;
	// Allocate all the buffers for the queues
	for (i = 0; i < QueueSize; i++)
	{
		buffers[i] = new UCHAR[len];
		isoPktInfos[i] = new CCyIsoPktInfo[PPX];
		inOvLap[i].hEvent = CreateEvent(NULL, false, false, NULL);

		memset(buffers[i], 0xEF, len);
	}
	//inOvLap.hEvent = CreateEvent(NULL, false, false, L"USB_OUT");
}

/*
处理后的数据会放入缓冲队列，缓冲队列有queue_size个buffer组成的内存区域，

*/
//传送队列中的数据
static void xferQueueData(queue<UCHAR> *xferQueue,CCyUSBEndPoint *EndPt,int queueindex, PUCHAR *buffers, long len, CCyIsoPktInfo **isoPktInfos, PUCHAR *contexts, OVERLAPPED inOvLap [])
{
	//OVERLAPPED outOvLab, inOvLab;
	
	
	
	
	contexts[queueindex] = EndPt->BeginDataXfer(buffers[queueindex], len, &inOvLap[queueindex]);
	if (EndPt->NtStatus || EndPt->UsbdStatus) // BeginDataXfer failed
	{
		printf("Xfer request rejected. NTSTATUS = %lld", EndPt->NtStatus);
		AbortXferLoop(EndPt, queueindex + 1, buffers, isoPktInfos, contexts, inOvLap);
		return;
	}
	


	
}
static bool					bStreaming;
static void AbortXferLoop(CCyUSBEndPoint *EndPt,int pending, PUCHAR *buffers, CCyIsoPktInfo **isoPktInfos, PUCHAR *contexts, OVERLAPPED inOvLap [])
{
	//EndPt->Abort(); - This is disabled to make sure that while application is doing IO and user unplug the device, this function hang the app.
	long len = EndPt->MaxPktSize * PPX;
	EndPt->Abort();

	for (int j = 0; j < QueueSize; j++)
	{
		if (j < pending)
		{
			EndPt->WaitForXfer(&inOvLap[j], TimeOut);
			/*{
				EndPt->Abort();
				if (EndPt->LastError == ERROR_IO_PENDING)
					WaitForSingleObject(inOvLap[j].hEvent,2000);
			}*/
			EndPt->FinishDataXfer(buffers[j], len, &inOvLap[j], contexts[j]);
		}

		CloseHandle(inOvLap[j].hEvent);

		delete[] buffers[j];
		delete[] isoPktInfos[j];
	}

	delete[] buffers;
	delete[] isoPktInfos;
	delete[] contexts;


	bStreaming = false;

	
}

	//建立一个固定空间的队列
	//只要不满，就push
	//只要非空就传入到usb后，pop
const static int dacDataSize = sizeof(dacData);
bool initInerface()
{
	//建立一个长度为QUEUE_LEN的循环队列，队列中的节点元素为QUEUE_NODE类型
	RingBufCPP<QUEUE_NODE,QUEUE_LEN> rQue;
	//初始化队列（因为是循环队列，可以不用初始化。）
	QUEUE_NODE qnode;
	qnode.datalen = QUEUE_NODE_LEN*dacDataSize;
	
	memset(qnode.data, 0xff, qnode.datalen);
	for (int i = 0; i < QUEUE_LEN; i++)
	{
		if (!rQue.add(qnode, true))
			printf("init queue faild \n");
	}


	
	return true;

}
void xferToUSBQueueNode(RingBufCPP<QUEUE_NODE, QUEUE_LEN> rQue)
{
	contexts[queueindex] = EndPt->BeginDataXfer(buffers[queueindex], len, &inOvLap[queueindex]);
	if (EndPt->NtStatus || EndPt->UsbdStatus) // BeginDataXfer failed
	{
		printf("Xfer request rejected. NTSTATUS = %lld", EndPt->NtStatus);
		AbortXferLoop(EndPt, queueindex + 1, buffers, isoPktInfos, contexts, inOvLap);
		return;
	}
}
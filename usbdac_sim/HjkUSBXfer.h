#pragma once
#ifndef __HJKUSBXFER_H_
#define __HJKUSBXFER_H_
#include <wtypes.h>
#include <dbt.h>
#include "CyAPI.h"


//#include "HjkDataDefine.h"
#define TIMEOUT_100_MS 100



class HjkUSBXfer
{
private:

	static int									MaxPktSize;
	static int									PPX;
	static int									QueueSize;
	static int									TimeOut;
	static int									dacDataSize;

	// Allocate the arrays needed for queueing
	//static PUCHAR			*buffers = new PUCHAR[QueueSize];
	//static CCyIsoPktInfo	**isoPktInfos = new CCyIsoPktInfo*[QueueSize];
	//记录发送队列
	PUCHAR								*contexts;
	OVERLAPPED							inOvLap;

	UCHAR								*context;

	

public:
	HjkUSBXfer();
	~HjkUSBXfer();




	
	//向USB端点传送数据，用异步传送方式，即传送后需等待结束信息
	bool xferDataToUSBEp(CCyUSBEndPoint *EndPt, PUCHAR buf, LONG len,OVERLAPPED ov);
	//用同步的方法，向USB的端点传送数据，适用发送数据而不需要等待发送是否结束
	bool xferDataToUSBEp(CCyUSBEndPoint *EndPt, PUCHAR buf, LONG len);
	
	//强制PPX有效
	void EnforceValidPPX();
	//终止传输
	void AbortXfer();

};
#endif

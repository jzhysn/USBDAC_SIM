
#include "stdafx.h"
#include "HjkUSBXfer.h" 
//using namespace std;
HjkUSBXfer::HjkUSBXfer()
{
	
	//初始化usb设备和端点

	//contexts = new PUCHAR[QueueSize];
	inOvLap.hEvent = CreateEvent(NULL, false, false, NULL);
}

HjkUSBXfer::~HjkUSBXfer()
{
}




UINT test=0;
bool HjkUSBXfer::xferDataToUSBEp(CCyUSBEndPoint *EndPt,PUCHAR buf, LONG len, OVERLAPPED ov)
{
	if (EndPt->Attributes == 1) // ISOC Endpoint
	{
		printf("we have can't develop the ISOC endpoint function!\n");
		return false;
	}
	context = EndPt->BeginDataXfer(buf, len, &ov);
	if (EndPt->NtStatus || EndPt->UsbdStatus) // BeginDataXfer failed
	{
		printf("Xfer request rejected. NTSTATUS = %lld", EndPt->NtStatus);
		//AbortXferLoop(EndPt, queueindex + 1, buffers, isoPktInfos, contexts, inOvLap);
		return false;
	}
	if (!EndPt->WaitForXfer(&ov, TIMEOUT_100_MS))
	{
		EndPt->Abort();
		if (EndPt->LastError == ERROR_IO_PENDING)
			WaitForSingleObject(ov.hEvent, 2000);
	}
	else
	{
		if (!EndPt->FinishDataXfer(buf, len, &ov, context))
		{
			printf("finnish a data buf xfer fail!\n");
		}
		else printf("finnish %d data buf xfer.\n",++test);
	}
	return true;
}

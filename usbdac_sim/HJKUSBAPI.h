#pragma once
#ifndef __HJKUSBAPI_H_
#define __HJKUSBAPI_H_
#include "usbdac_sim.h"
/* {AE18AA60-7F6A-11d4-97DD-00010229B959} */
static GUID CYUSBDRV_GUID = { 0xae18aa60, 0x7f6a, 0x11d4, 0x97, 0xdd, 0x0, 0x1, 0x2, 0x29, 0xb9, 0x59 };
typedef struct _USB_ENDPOINT_DESCRIPTOR {
	UCHAR   bLength;
	UCHAR   bDescriptorType;
	UCHAR   bEndpointAddress;
	UCHAR   bmAttributes;
	USHORT  wMaxPacketSize;
	UCHAR   bInterval;
} USB_ENDPOINT_DESCRIPTOR, *PUSB_ENDPOINT_DESCRIPTOR;
typedef struct _USB_SUPERSPEED_ENDPOINT_COMPANION_DESCRIPTOR
{
	UCHAR bLength;
	UCHAR bDescriptorType;
	UCHAR bMaxBurst;
	UCHAR bmAttributes;
	USHORT bBytesPerInterval;
}USB_SUPERSPEED_ENDPOINT_COMPANION_DESCRIPTOR, *PUSB_SUPERSPEED_ENDPOINT_COMPANION_DESCRIPTOR;
/********************************************************************************
*
* The USB BOS Class
*
********************************************************************************/
class CCyUSBBOS
{
public:

	CCyBosContainerID           *pContainer_ID;
	CCyBosUSB20Extesnion        *pUSB20_DeviceExt;
	CCyBosSuperSpeedCapability  *pSS_DeviceCap;

	UCHAR   bLength;            /* Descriptor length */
	UCHAR   bDescriptorType;    /* Descriptor Type */
	USHORT  wTotalLength;      /* Total length of descriptor ( icluding device capabilty */
	UCHAR   bNumDeviceCaps;     /* Number of device capability descriptors in BOS */

	CCyUSBBOS(void);
	CCyUSBBOS(HANDLE h, PUSB_BOS_DESCRIPTOR pBosDescrData);
	~CCyUSBBOS();
};

typedef enum { XMODE_BUFFERED, XMODE_DIRECT } XFER_MODE_TYPE;
class CCyUSBDevice
{
	/* The public members are accessible (i.e. corruptible) by the user of the library
	 * Algorithms of the class don't rely on any public members.  Instead, they use the
	 * private members of the class for their calculations. */

public:

	CCyUSBDevice(HANDLE hnd = NULL, GUID guid = CYUSBDRV_GUID, BOOL bOpen = true);
	~CCyUSBDevice(void);

	CCyUSBEndPoint  **EndPoints;     /* Shortcut to USBCfgs[CfgNum]->Interfaces[IntfcIndex]->Endpoints */
	CCyUSBEndPoint  *EndPointOf(UCHAR addr);

	CCyUSBBOS               *UsbBos;
	CCyIsocEndPoint         *IsocInEndPt;
	CCyIsocEndPoint         *IsocOutEndPt;
	CCyBulkEndPoint         *BulkInEndPt;
	CCyBulkEndPoint         *BulkOutEndPt;
	CCyControlEndPoint      *ControlEndPt;
	CCyInterruptEndPoint    *InterruptInEndPt;
	CCyInterruptEndPoint    *InterruptOutEndPt;


	USHORT      StrLangID;
	ULONG       LastError;
	ULONG       UsbdStatus;
	ULONG       NtStatus;
	ULONG       DriverVersion;
	ULONG       USBDIVersion;
	char        DeviceName[USB_STRING_MAXLEN];
	char        FriendlyName[USB_STRING_MAXLEN];
	wchar_t     Manufacturer[USB_STRING_MAXLEN];
	wchar_t     Product[USB_STRING_MAXLEN];
	wchar_t     SerialNumber[USB_STRING_MAXLEN];

	CHAR        DevPath[USB_STRING_MAXLEN];

	USHORT      BcdUSB;
	USHORT      VendorID;
	USHORT      ProductID;
	UCHAR       USBAddress;
	UCHAR       DevClass;
	UCHAR       DevSubClass;
	UCHAR       DevProtocol;
	INT       MaxPacketSize;
	USHORT      BcdDevice;

	UCHAR       ConfigValue;
	UCHAR       ConfigAttrib;
	UCHAR       MaxPower;

	UCHAR       IntfcClass;
	UCHAR       IntfcSubClass;
	UCHAR       IntfcProtocol;
	bool        bHighSpeed;
	bool        bSuperSpeed;

	DWORD       BytesXfered;

	UCHAR       DeviceCount(void);
	UCHAR       ConfigCount(void);
	UCHAR       IntfcCount(void);
	UCHAR       AltIntfcCount(void);
	UCHAR       EndPointCount(void);

	void        SetConfig(UCHAR cfg);
	UCHAR       Config(void) { return CfgNum; }    /* Normally 0 */
	UCHAR       Interface(void) { return IntfcNum; }  /* Usually 0 */

	/* No SetInterface method since only 1 intfc per device (per Windows) */
	UCHAR       AltIntfc(void);
	bool        SetAltIntfc(UCHAR alt);

	GUID        DriverGUID(void) { return DrvGuid; }
	HANDLE      DeviceHandle(void) { return hDevice; }
	void        UsbdStatusString(ULONG stat, PCHAR s);
	bool        CreateHandle(UCHAR dev);
	void        DestroyHandle();

	bool        Open(UCHAR dev);
	void        Close(void);
	bool        Reset(void);
	bool        ReConnect(void);
	bool        Suspend(void);
	bool        Resume(void);
	bool        IsOpen(void) { return (hDevice != INVALID_HANDLE_VALUE); }

	UCHAR       PowerState(void);

	bool        GetBosDescriptor(PUSB_BOS_DESCRIPTOR descr);
	bool        GetBosUSB20DeviceExtensionDescriptor(PUSB_BOS_USB20_DEVICE_EXTENSION descr);
	bool        GetBosContainedIDDescriptor(PUSB_BOS_CONTAINER_ID descr);
	bool        GetBosSSCapabilityDescriptor(PUSB_BOS_SS_DEVICE_CAPABILITY descr);

	void        GetDeviceDescriptor(PUSB_DEVICE_DESCRIPTOR descr);
	void        GetConfigDescriptor(PUSB_CONFIGURATION_DESCRIPTOR descr);
	void        GetIntfcDescriptor(PUSB_INTERFACE_DESCRIPTOR descr);
	CCyUSBConfig    GetUSBConfig(int index);

private:

	USB_DEVICE_DESCRIPTOR           USBDeviceDescriptor;
	PUSB_CONFIGURATION_DESCRIPTOR   USBConfigDescriptors[2];
	PUSB_BOS_DESCRIPTOR             pUsbBosDescriptor;

	CCyUSBConfig    *USBCfgs[2];

	HANDLE      hWnd;
	HANDLE      hDevice;
	HANDLE      hDevNotification;
	HANDLE      hHndNotification;

	GUID        DrvGuid;

	UCHAR       Devices;
	UCHAR       Interfaces;
	UCHAR       AltInterfaces;
	UCHAR       Configs;

	UCHAR       DevNum;
	UCHAR       CfgNum;
	UCHAR       IntfcNum;   /* The current selected interface's bInterfaceNumber */
	UCHAR       IntfcIndex; /* The entry in the Config's interfaces table matching to IntfcNum and AltSetting */

	bool        GetInternalBosDescriptor();
	void        GetDevDescriptor(void);
	void        GetCfgDescriptor(int descIndex);
	void        GetString(wchar_t *s, UCHAR sIndex);
	void        SetStringDescrLanguage(void);
	void        SetAltIntfcParams(UCHAR alt);
	bool        IoControl(ULONG cmd, PUCHAR buf, ULONG len);

	void        SetEndPointPtrs(void);
	void        GetDeviceName(void);
	void        GetFriendlyName(void);
	void        GetDriverVer(void);
	void        GetUSBDIVer(void);
	void        GetSpeed(void);
	void        GetUSBAddress(void);
	//void      CloseEndPtHandles(void);

	bool        RegisterForPnpEvents(HANDLE h);
};


class CCyUSBEndPoint
{
protected:
	bool WaitForIO(OVERLAPPED *ovLapStatus);

	virtual PUCHAR BeginDirectXfer(PUCHAR buf, LONG bufLen, OVERLAPPED *ov);
	virtual PUCHAR BeginBufferedXfer(PUCHAR buf, LONG bufLen, OVERLAPPED *ov);

public:

	CCyUSBEndPoint(void);
	CCyUSBEndPoint(CCyUSBEndPoint& ept);
	CCyUSBEndPoint(HANDLE h, PUSB_ENDPOINT_DESCRIPTOR pEndPtDescriptor);
	CCyUSBEndPoint(HANDLE hDev, PUSB_ENDPOINT_DESCRIPTOR pEndPtDescriptor, USB_SUPERSPEED_ENDPOINT_COMPANION_DESCRIPTOR* SSEndPtDescriptor);

	HANDLE  hDevice;

	/* The fields of an EndPoint Descriptor */
	UCHAR   DscLen;
	UCHAR   DscType;
	UCHAR   Address;
	UCHAR   Attributes;
	USHORT  MaxPktSize;
	USHORT  PktsPerFrame;
	UCHAR   Interval;
	/* This are the fields for Super speed endpoint */
	UCHAR   ssdscLen;
	UCHAR   ssdscType;
	UCHAR   ssmaxburst;     /* Maximum number of packets endpoint can send in one burst */
	UCHAR   ssbmAttribute;  /* store endpoint attribute like for bulk it will be number of streams */
	USHORT  ssbytesperinterval;

	/* Other fields */
	ULONG   TimeOut;
	ULONG   UsbdStatus;
	ULONG   NtStatus;

	DWORD   bytesWritten;
	DWORD   LastError;
	bool    bIn;

	XFER_MODE_TYPE  XferMode;

	bool    XferData(PUCHAR buf, LONG &len, CCyIsoPktInfo* pktInfos = NULL);
	bool    XferData(PUCHAR buf, LONG &bufLen, CCyIsoPktInfo* pktInfos, bool pktMode);
	virtual PUCHAR BeginDataXfer(PUCHAR buf, LONG len, OVERLAPPED *ov) = 0;
	virtual bool FinishDataXfer(PUCHAR buf, LONG &len, OVERLAPPED *ov, PUCHAR pXmitBuf, CCyIsoPktInfo* pktInfos = NULL);
	bool    WaitForXfer(OVERLAPPED *ov, ULONG tOut);
	ULONG   GetXferSize(void);
	void    SetXferSize(ULONG xfer);

	bool Reset(void);
	bool Abort(void);
};
class CCyIsoPktInfo {
public:
	LONG Status;
	LONG Length;
};
#endif
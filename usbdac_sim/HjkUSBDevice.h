#pragma once
#ifndef __HJKUSBDEVICE_H_
#define __HJKUSBDEVICE_H_
#include <wtypes.h>
#include <dbt.h>
#include "CyAPI.h"

#include "usbdac_sim.h"
#define USB_DEVICE_VENDOR_ID 0x04B4;
#define USB_DEVICE_PRODUCT_ID 0x00F1;


class HjkUSBDevice
{
public:
	//USB设备
	CCyUSBDevice						*USBDevice;
private:
	int							USBDevcieCounts;
	const int					VENDOR_ID = USB_DEVICE_VENDOR_ID;
	const int					PRODUCT_ID = USB_DEVICE_PRODUCT_ID;
	//usb端点
	CCyUSBEndPoint				*EndPt;
	//Handle
	HANDLE										hjkHandle;


	bool							bHighSpeedDevice;
	bool							bSuperSpeedDevice;
public:
	HjkUSBDevice();
	//~HjkUSBDevice();
	//获取USB设备，返回设备数量,设备号和名称放入devicelist中
	int getUSBDevice(char * deviceList);

	//获取USB端点信息，返回端点数量
	int getUSBEndPt(CCyUSBDevice *usbDevice, int deviceNum, char *EndPtInfo);

	//设置USB设备
	bool setUSBDevice();

	//设置USB端点
	bool setUSBEndPt();
	//设置传输模式
	bool setUSBXferMode();



};
#endif

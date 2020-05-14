#pragma once
#ifndef __HJKUSBDEVICE_H_
#define __HJKUSBDEVICE_H_

#include <wtypes.h>
#include <String>
#include <sstream>
#include <iostream>
#include <dbt.h>
#include "CyAPI.h"


#define USB_DEVICE_VENDOR_ID 0x04B4;
#define USB_DEVICE_PRODUCT_ID 0x00F1;
using namespace std;

class HjkUSBDevice
{
public:
	//USB�豸
	CCyUSBDevice						*USBDevice;
private:
	int							USBDevcieCounts;
	const int					VENDOR_ID = USB_DEVICE_VENDOR_ID;
	const int					PRODUCT_ID = USB_DEVICE_PRODUCT_ID;
	//usb�˵�
	CCyUSBEndPoint				*EndPt;
	//Handle
	HANDLE										hjkHandle;


	bool							bHighSpeedDevice;
	bool							bSuperSpeedDevice;
public:
	HjkUSBDevice();
	//~HjkUSBDevice();
	//��ȡUSB�豸�������豸����,�豸�ź����Ʒ���devicelist�У�ÿ���豸�û��з�������
//private:
	int getHjkUSBDevice(string *deviceList);
	//���豸��Ϊn��usb�豸
	bool openHjkUSBDevice(int deviceNum);
	//��ȡUSB�˵���Ϣ�����ض˵�����
	int getHjkUSBEndPt(int deviceNum, string EndPtInfo);
	//��ȡbulk out
	CCyBulkEndPoint *getHjkUSBBulkOutEndPt(CCyUSBDevice *hjkUDve);
	//����USB�豸
	bool setUSBDevice();

	//����USB�˵�
	bool setUSBEndPt();
	//���ô���ģʽ
	bool setUSBXferMode();



};
#endif
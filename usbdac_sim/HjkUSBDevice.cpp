#include "stdafx.h"
#include "HjkUSBDevice.h"
#include <String>
HjkUSBDevice::HjkUSBDevice()
{
	hjkHandle = new HANDLE();
	char list[2000];
	int i = getUSBDevice(list);

	printf("usb device counts is %d\n", i);
	//getUSBEndPt();测试状态时不用
}

int HjkUSBDevice::getUSBDevice(char* deviceList)
{
	USBDevice = new CCyUSBDevice(NULL, CYUSBDRV_GUID, true);

	char  tempstr[200];
	
	if (USBDevice == NULL)
	{
		printf("USBDevice is null.\n");
		return 0;
	}

	int n = USBDevice->DeviceCount();

	/////////////////////////////////////////////////////////////////
	// Walk through all devices looking for VENDOR_ID/PRODUCT_ID
	// We No longer got restricted with vendor ID and Product ID.
	// Check for vendor ID and product ID is discontinued.
	///////////////////////////////////////////////////////////////////
	char VendorID[50];
	char ProductID[50];
	for (int i = 0; i < n; i++)
	{
		//if ((USBDevice->VendorID == VENDOR_ID) && (USBDevice->ProductID == PRODUCT_ID)) 
		//    break;
		USBDevice->Open(i);
		memset(tempstr, 0, 200);
		
		strcat(tempstr,(char *)&i);
		/*
		//printf("(0x");
		strcat(tempstr+1, ".(0x");
		//printf("%x", USBDevice->VendorID);
		sprintf(VendorID, "%x", USBDevice->VendorID);
		strcat(tempstr+5,VendorID);
		//printf(" - 0x");
		strcat(tempstr+55, " - 0x");		
		//printf("%x", USBDevice->ProductID);
		sprintf(ProductID, "%x", USBDevice->ProductID);
		strcat(tempstr, ProductID);
		//printf(") ");
		strcat(tempstr, ") ");
		//printf("%c\n", USBDevice->FriendlyName);
		*/
		strcat(tempstr+1, USBDevice->FriendlyName);
		printf(tempstr); printf("\n");
		//strcat(tempstr, "\n");
		//strcat(deviceList, tempstr);
	}
	//默认测试
	if (n > 0) {
		//打开默认第一个
		USBDevice->Open(0);
	}


	//if ((USBDevice->VendorID == VENDOR_ID) && (USBDevice->ProductID == PRODUCT_ID)) 
	{


		int interfaces = USBDevice->AltIntfcCount() + 1;

		bHighSpeedDevice = USBDevice->bHighSpeed;
		bSuperSpeedDevice = USBDevice->bSuperSpeed;

		for (int i = 0; i < interfaces; i++)
		{
			if (USBDevice->SetAltIntfc(i) == true)
			{

				int eptCnt = USBDevice->EndPointCount();

				// Fill the EndPointsBox
				for (int e = 1; e < eptCnt; e++)
				{
					CCyUSBEndPoint *ept = USBDevice->EndPoints[e];
					// INTR, BULK and ISO endpoints are supported.
					if ((ept->Attributes >= 1) && (ept->Attributes <= 3))
					{
						/*一些信息显示
						String *s = "";
						s = String::Concat(s, ((ept->Attributes == 1) ? "ISOC " :
							((ept->Attributes == 2) ? "BULK " : "INTR ")));
						s = String::Concat(s, ept->bIn ? "IN,       " : "OUT,   ");
						s = String::Concat(s, ept->MaxPktSize.ToString(), " Bytes,");
						if (USBDevice->BcdUSB == USB30MAJORVER)
							s = String::Concat(s, ept->ssmaxburst.ToString(), " MaxBurst,");

						s = String::Concat(s, "   (", i.ToString(), " - ");
						s = String::Concat(s, "0x", ept->Address.ToString("X02"), ")");
						EndPointsBox->Items->Add(s);
						*/
					}
				}
			}
		}

	}
	return n;
}

int HjkUSBDevice::getUSBEndPt(CCyUSBDevice *usbDevice, int deviceNum,char *EndPtInfo)
{
	int eptCnt = 0;
	//if ((USBDevice->VendorID == VENDOR_ID) && (USBDevice->ProductID == PRODUCT_ID))
	if(usbDevice->DeviceCount() > 0)
	{
		usbDevice->Open(deviceNum);

		int interfaces = usbDevice->AltIntfcCount() + 1;

		bHighSpeedDevice = usbDevice->bHighSpeed;
		bSuperSpeedDevice = usbDevice->bSuperSpeed;
		
		for (int i = 0; i < interfaces; i++)
		{
			if (usbDevice->SetAltIntfc(i) == true)
			{

				eptCnt = usbDevice->EndPointCount();

				// Fill the EndPointsBox
				for (int e = 1; e < eptCnt; e++)
				{
					CCyUSBEndPoint *ept = usbDevice->EndPoints[e];
					// INTR, BULK and ISO endpoints are supported.
					if ((ept->Attributes >= 1) && (ept->Attributes <= 3))
					{
						/*一些信息显示
						String *s = "";
						s = String::Concat(s, ((ept->Attributes == 1) ? "ISOC " :
							((ept->Attributes == 2) ? "BULK " : "INTR ")));
						s = String::Concat(s, ept->bIn ? "IN,       " : "OUT,   ");
						s = String::Concat(s, ept->MaxPktSize.ToString(), " Bytes,");
						if (USBDevice->BcdUSB == USB30MAJORVER)
							s = String::Concat(s, ept->ssmaxburst.ToString(), " MaxBurst,");

						s = String::Concat(s, "   (", i.ToString(), " - ");
						s = String::Concat(s, "0x", ept->Address.ToString("X02"), ")");
						EndPointsBox->Items->Add(s);
						*/
					}
				}
				
			}
		}
	return eptCnt;
	}
	else return 0;
}

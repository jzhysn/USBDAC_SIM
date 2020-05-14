#include "stdafx.h"
#include "HjkUSBDevice.h"
#include <String>
HjkUSBDevice::HjkUSBDevice()
{
	
}

int HjkUSBDevice::getHjkUSBDevice(string deviceList)
{
	deviceList = "";
	USBDevice = new CCyUSBDevice(NULL, CYUSBDRV_GUID, true);	
	
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
	string str;
	stringstream sstem;
	for (int i = 0; i < n; i++)
	{
		//if ((USBDevice->VendorID == VENDOR_ID) && (USBDevice->ProductID == PRODUCT_ID)) 
		//    break;
		USBDevice->Open(i);		
				
		sstem << i << ".(0x" << hex << USBDevice->VendorID
			<< " - 0x" << hex << USBDevice->ProductID << ") "
			<< USBDevice->FriendlyName << "\n";

		deviceList.append(sstem.str());	
		cout << sstem.str();
		sstem.clear();
		sstem.str("");
		
		
	}	
	return n;
}

int HjkUSBDevice::getHjkUSBEndPt(int deviceNum, string EndPtInfo)
{
	EndPtInfo = "";
	int eptCnt = 0;
	stringstream ss;
	//if ((USBDevice->VendorID == VENDOR_ID) && (USBDevice->ProductID == PRODUCT_ID))
	if (USBDevice->DeviceCount() > 0)
	{
		USBDevice->Open(deviceNum);

		int interfaces = USBDevice->AltIntfcCount() + 1;

		bHighSpeedDevice = USBDevice->bHighSpeed;
		bSuperSpeedDevice = USBDevice->bSuperSpeed;

		for (int i = 0; i < interfaces; i++)
		{
			if (USBDevice->SetAltIntfc(i) == true)
			{

				eptCnt = USBDevice->EndPointCount();

				// Fill the EndPtInfo
				for (int e = 1; e < eptCnt; e++)
				{
					CCyUSBEndPoint *ept = USBDevice->EndPoints[e];
					// INTR, BULK and ISO endpoints are supported.
					if ((ept->Attributes >= 1) && (ept->Attributes <= 3))
					{
						ss << ((ept->Attributes == 1) ? "ISOC " :
							((ept->Attributes == 2) ? "BULK " : "INTR "))
							<< (ept->bIn ? "IN,       " : "OUT,   ")
							<<dec<< ept->MaxPktSize
							<< " Bytes,";
						if (USBDevice->BcdUSB == USB30MAJORVER)
							ss << ept->ssmaxburst<<" MaxBurst,";
						ss << "   (" << i << " - "
							<< "0x" << hex << ept->Address << ")\n";
						EndPtInfo.append(ss.str());
						cout << ss.str();
						ss.clear();
						ss.str("");
					}
				}

			}
		}
		return eptCnt;
	}
	else return 0;
}


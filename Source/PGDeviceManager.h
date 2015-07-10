/*
  ==============================================================================

    PGDeviceManager.h
    Created: 9 Jul 2015 6:12:21pm
    Author:  bb

  ==============================================================================
*/

#ifndef PGDEVICEMANAGER_H_INCLUDED
#define PGDEVICEMANAGER_H_INCLUDED


#include <vector>
#include <string>

#include "PGDevice.h"


typedef void(*OnDeviceSearched)(DeviceDesc desc, void *userData);

class PGDeviceManager
{
public:
	static std::vector<DeviceDesc> Devices;

	// this function will also update member _devices
	static void getAllDevices(OnDeviceSearched handleDeviceSearched, void *userData);
	static PGDevice *CreateDeviceInstance(int deviceIndex);

private:
};


#endif  // PGDEVICEMANAGER_H_INCLUDED

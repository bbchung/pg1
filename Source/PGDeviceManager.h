#ifndef PG_DEVICE_MANAGER_H
#define PG_DEVICE_MANAGER_H

/*
No use for current
*/

#include <vector>
#include <string>

#include "PGDevice.h"

typedef void(*OnDeviceSearched)(PGMidiDeviceDesc desc, void *userData);

class PGDeviceManager
{
public:
	static std::vector<PGMidiDeviceDesc> Devices;

	static void getAllDevices(OnDeviceSearched handleDeviceSearched, void *userData);
	static PGDevice *CreateDeviceInstance(int deviceIndex);

private:
};


#endif  // PGDEVICEMANAGER_H_INCLUDED

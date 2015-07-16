/*
  ==============================================================================

    This file was auto-generated!

    It contains the basic startup code for a Juce application.

  ==============================================================================
*/

#include "PGDevice.h"

using namespace std;

//==============================================================================
int main (int argc, char* argv[])
{
	vector<PGMidiDeviceDesc> devices = PGMidiDevice::getDevices();
	PGMidiDeviceDesc desc;
	PGMidiDevice *d = new PGMidiDevice(desc);
	if (!d->open())
		return 0;


    // ..your code goes here!


    return 0;
}

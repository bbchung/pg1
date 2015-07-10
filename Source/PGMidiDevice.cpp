/*
  ==============================================================================

  PGDevice.cpp
  Created: 9 Jul 2015 6:41:30pm
  Author:  bb

  ==============================================================================
  */

#include "PGDevice.h"

using namespace std;


void PGMidiDevice::PGMidiInputCallback::handleIncomingMidiMessage(MidiInput *source, const MidiMessage &message)
{
	if (!message.isSysEx())
		return;
}

PGMidiDevice::PGMidiDevice(DeviceDesc desc) :
PGDevice(desc),
_midiIn(NULL),
_midiOut(NULL),
_midiInputCallback(new PGMidiInputCallback())
{

}

vector<DeviceDesc> PGMidiDevice::getDevices()
{
	vector<DeviceDesc> pgDevices;

	StringArray midiInDevicesName = MidiInput::getDevices();

	for (int i = 0; i < midiInDevicesName.size(); ++i)
	{
		if (midiInDevicesName[i] == "G1")
		{
			DeviceDesc desc;
			desc.type = USB;
			desc.midiInIndex = i;
			desc.midiOutIndex = -1;

			pgDevices.push_back(desc);
		}
	}

	StringArray midiOutDevicesName = MidiOutput::getDevices();

	vector<DeviceDesc>::iterator it = pgDevices.begin();

	for (int i = 0; i < midiOutDevicesName.size(); ++i)
	{
		if (midiOutDevicesName[i] == "G1")
		{
			if (it == pgDevices.end())
				return pgDevices; // should not happen

			it->midiOutIndex = i;
			it++;
		}
	}

	return pgDevices;
}

PGMidiDevice *PGMidiDevice::openDevice(DeviceDesc desc)
{
	if (desc.type != USB)
		return NULL;

	PGMidiDevice *device = new PGMidiDevice(desc);
	if (device->open())
		return device;

	delete device;
	return NULL;
}

bool PGMidiDevice::open()
{
	_midiIn = MidiInput::openDevice(Desc.midiInIndex, _midiInputCallback);
	_midiOut = MidiOutput::openDevice(Desc.midiOutIndex);

	return _midiIn && _midiOut;
}

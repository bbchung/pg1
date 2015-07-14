/*
  ==============================================================================

  PGDevice.cpp
  Created: 9 Jul 2015 6:41:30pm
  Author:  bb

  ==============================================================================
  */

#include "PGDevice.h"

#include "PGSysEx.h"


using namespace std;

void PGMidiDevice::PGMidiInputCallback::handleIncomingMidiMessage(MidiInput *source, const MidiMessage &message)
{
	if (!message.isSysEx())
		return;

	const uint8 *buf = message.getSysExData();
	
	uint8 mfrs = buf[1];
	uint8 op_msb = buf[2];
	uint8 op_lsb = buf[3];

	if (mfrs != MFRS)
		return;

	if (Device->CVMap.find(op_msb << 2 | op_lsb) != Device->CVMap.end())
	{
		std::unique_lock<std::mutex> lck(Device->Mutex);
		Device->CVMap[op_msb << 2 | op_lsb].msg = message;
		Device->CVMap[op_msb << 2 | op_lsb].cv->notify_one();
	}
}

PGMidiDevice::PGMidiDevice(DeviceDesc desc) :
PGDevice(desc),
_midiIn(NULL),
_midiOut(NULL),
_midiInputCallback(new PGMidiInputCallback(this))
{
	CVMap[ACK << 2 | CUSTOM_CC] = MessageNotify();
	CVMap[ACK << 2 | CUSTOM_PC] = MessageNotify();
}

PGMidiDevice::~PGMidiDevice()
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
	if (!_midiIn)
		return false;

	_midiOut = MidiOutput::openDevice(Desc.midiOutIndex);
	if (!_midiOut)
	{
		delete _midiIn;
		_midiIn = NULL;
		return false;
	}

	_midiIn->start();
	return true;
}

bool PGMidiDevice::saveCustomizedCC(int count, CustomCCData *udm)
{
	uint8 *sysex_buf; // need free
	size_t sysex_buf_len = ComposeCustomCCSysEx(count, udm, &sysex_buf);

	MidiMessage msg = MidiMessage(sysex_buf, sysex_buf_len, 0);
	_midiOut->sendMessageNow(msg);

	free(sysex_buf);

	unique_lock <mutex> lck(Mutex);
	if (CVMap[ACK << 2 | CUSTOM_CC].cv->wait_for(lck, chrono::milliseconds(5000)) == cv_status::no_timeout)
		return true;
	else
		return false;
}

bool PGMidiDevice::saveCustomizedPC(int count, CustomPCData *udm)
{
	uint8 *sysex_buf; // need free
	size_t sysex_buf_len = ComposeCustomPCSysEx(count, udm, &sysex_buf);

	MidiMessage msg = MidiMessage(sysex_buf, sysex_buf_len, 0);
	_midiOut->sendMessageNow(msg);

	free(sysex_buf);

	unique_lock <mutex> lck(Mutex);
	if (CVMap[ACK << 2 | CUSTOM_PC].cv->wait_for(lck, chrono::milliseconds(5000)) == cv_status::no_timeout)
		return true;
	else
		return false;
}
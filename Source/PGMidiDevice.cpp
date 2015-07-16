#include "PGDevice.h"

using namespace std;

void PGMidiDevice::PGMidiInputCallback::handleIncomingMidiMessage(MidiInput *source, const MidiMessage &message)
{
	// currently, G1 should only send SysEx to app
	if (!message.isSysEx())
		return;

	const uint8 *buf = message.getSysExData();
	int size = message.getSysExDataSize();

	if (!PGSysExParser::IsPGSysEx(buf, size))
		return;

	switch (PGSysExParser::GetOpMsb(buf, size))
	{
	case REPLY_GRP:
		OwnerMidiDevice->MMBox.NotifyReply(message);
		break;
	case ACK_GRP:
	case NAK_GRP:
		OwnerMidiDevice->MMBox.NotifyAck(message);
		break;
	default:
		break;
	}
}

PGMidiDevice::PGMidiDevice(const PGMidiDeviceDesc &desc) :
PGDevice(desc),
_midiIn(NULL),
_midiOut(NULL),
_midiInputCallback(new PGMidiInputCallback(this))
{
}

PGMidiDevice::~PGMidiDevice()
{
	close();
}

vector<PGMidiDeviceDesc> PGMidiDevice::getDevices()
{
	vector<PGMidiDeviceDesc> pgDevices;

	StringArray midiInDevicesName = MidiInput::getDevices();

	for (int i = 0; i < midiInDevicesName.size(); ++i)
	{
		if (midiInDevicesName[i] == "G1")
		{
			PGMidiDeviceDesc desc;
			desc.midiInIndex = i;
			desc.midiOutIndex = -1;

			pgDevices.push_back(desc);
		}
	}

	StringArray midiOutDevicesName = MidiOutput::getDevices();

	vector<PGMidiDeviceDesc>::iterator it = pgDevices.begin();

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

bool PGMidiDevice::open()
{
	if (_midiIn || _midiOut)
		return false;

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

void PGMidiDevice::close()
{
	if (_midiIn)
	{
		delete _midiIn;
		_midiIn = NULL;
	}

	if (_midiOut)
	{
		delete _midiOut;
		_midiOut = NULL;
	}
}

bool PGMidiDevice::saveCustomizedCC(int count, CustomCC *ccc)
{
	uint8 sysex_buf[MAX_SYSEX_SIZE] = { 0 };
	size_t sysex_size = PGSysExComposer::ComposeSaveCustomCCSysEx(count, ccc, sysex_buf);

	MidiMessage msg = MidiMessage(sysex_buf, sysex_size, 0);

	if (MMBox.SendMessageAndWaitAck(_midiOut, msg, 5000) == cv_status::timeout)
		return false;

	const uint8 *sysex_ack = MMBox.AckMessage.getSysExData();
	int sysex_ack_size = MMBox.ReplyMessage.getSysExDataSize();

	if (PGSysExParser::GetOpMsb(sysex_ack, sysex_ack_size) == NAK_GRP)
		return false;

	if (PGSysExParser::GetOpLsb(sysex_ack, sysex_ack_size) != CUSTOM_CC_OBJ)
		return false;

	return true;
}

bool PGMidiDevice::saveCustomizedPC(int count, CustomPC *cpc)
{
	uint8 sysex_buf[MAX_SYSEX_SIZE] = { 0 };
	size_t sysex_size = PGSysExComposer::ComposeSaveCustomPCSysEx(count, cpc, sysex_buf);

	MidiMessage msg = MidiMessage(sysex_buf, sysex_size, 0);

	if (MMBox.SendMessageAndWaitAck(_midiOut, msg, 5000) == cv_status::timeout)
		return false;

	const uint8 *sysex_ack = MMBox.AckMessage.getSysExData();
	int sysex_ack_size = MMBox.ReplyMessage.getSysExDataSize();

	if (PGSysExParser::GetOpMsb(sysex_ack, sysex_ack_size) == NAK_GRP)
		return false;

	if (PGSysExParser::GetOpLsb(sysex_ack, sysex_ack_size) != CUSTOM_PC_OBJ)
		return false;

	return true;
}

bool PGMidiDevice::requestCustomizedCC(int count, CustomCC *ccc)
{
	uint8 sysex_buf[MAX_SYSEX_SIZE] = {0};
	size_t sysex_size = PGSysExComposer::ComposeRequestCustomCCSysEx(count, ccc, sysex_buf);

	MidiMessage msg = MidiMessage(sysex_buf, sysex_size, 0);

	if (MMBox.SendMessageAndWaitReply(_midiOut, msg, 5000) == cv_status::timeout)
		return false;

	const uint8 *sysex_reply = MMBox.ReplyMessage.getSysExData();
	int sysex_reply_size = MMBox.ReplyMessage.getSysExDataSize();

	if (PGSysExParser::GetOpLsb(sysex_reply, sysex_reply_size) != CUSTOM_CC_OBJ)
		return false;

	return PGSysExParser::GetCustomCC(sysex_reply, sysex_reply_size, ccc, count);
}

bool PGMidiDevice::requestCustomizedPC(int count, CustomPC *cpc)
{
	uint8 sysex_buf[MAX_SYSEX_SIZE] = { 0 };
	size_t sysex_buf_size = PGSysExComposer::ComposeRequestCustomPCSysEx(count, cpc, sysex_buf);

	MidiMessage msg = MidiMessage(sysex_buf, sysex_buf_size, 0);

	if (MMBox.SendMessageAndWaitReply(_midiOut, msg, 5000) == cv_status::timeout)
		return false;

	const uint8 *sysex_reply = MMBox.ReplyMessage.getSysExData();
	int sysex_reply_size = MMBox.ReplyMessage.getSysExDataSize();

	if (PGSysExParser::GetOpLsb(sysex_reply, sysex_reply_size) != CUSTOM_PC_OBJ)
		return false;

	return PGSysExParser::GetCustomPC(sysex_reply, sysex_reply_size, cpc, count);
}
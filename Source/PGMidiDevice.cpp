#include "PGDevice.h"

using namespace std;

void PGMidiDevice::PGMidiInputCallback::handleIncomingMidiMessage(MidiInput *source, const MidiMessage &msg)
{
	Logger::getCurrentLogger()->writeToLog("midiin: recv message:");
	string dbg = ToHexString(msg.getRawData(), msg.getRawDataSize());
	Logger::getCurrentLogger()->writeToLog(dbg);

	// currently, G1 should only send SysEx to app
	if (!msg.isSysEx())
		return;

	const uint8 *buf = msg.getSysExData();
	int size = msg.getSysExDataSize();

	if (!PGSysExParser::IsPGSysEx(buf, size))
		return;

	switch (PGSysExParser::GetOpMsb(buf, size))
	{
	case REPLY_GRP:
		OwnerMidiDevice->MidiMessageBox.NotifyReply(msg);
		break;
	case ACK_GRP:
	case NAK_GRP:
		OwnerMidiDevice->MidiMessageBox.NotifyAck(msg);
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

bool PGMidiDevice::saveCustomizedCC(const vector<CustomCC> &ccc)
{
	uint8 sysex_buf[MAX_SYSEX_SIZE] = { 0 };
	size_t sysex_size = PGSysExComposer::ComposeSaveCustomCCSysEx(ccc, sysex_buf);

	MidiMessage msg = MidiMessage(sysex_buf, sysex_size, 0);

	if (!MidiMessageBox.SendMessageAndWaitAckOrNak(_midiOut, msg, 5000))
		return false;

	const uint8 *sysex_ack = MidiMessageBox.AckMessage.getSysExData();
	int sysex_ack_size = MidiMessageBox.ReplyMessage.getSysExDataSize();

	if (PGSysExParser::GetOpMsb(sysex_ack, sysex_ack_size) == NAK_GRP)
	{
		Logger::getCurrentLogger()->writeToLog("recv a nak");
		return false;
	}

	if (PGSysExParser::GetOpLsb(sysex_ack, sysex_ack_size) != CUSTOM_CC_OBJ)
	{
		Logger::getCurrentLogger()->writeToLog("ack is not for CUSTOM_CC");
		return false;
	}

	return true;
}

bool PGMidiDevice::saveCustomizedPC(const vector<CustomPC> &cpc)
{
	uint8 sysex_buf[MAX_SYSEX_SIZE] = { 0 };
	size_t sysex_size = PGSysExComposer::ComposeSaveCustomPCSysEx(cpc, sysex_buf);

	MidiMessage msg = MidiMessage(sysex_buf, sysex_size, 0);

	if (!MidiMessageBox.SendMessageAndWaitAckOrNak(_midiOut, msg, 5000))
		return false;

	const uint8 *sysex_ack = MidiMessageBox.AckMessage.getSysExData();
	int sysex_ack_size = MidiMessageBox.ReplyMessage.getSysExDataSize();

	if (PGSysExParser::GetOpMsb(sysex_ack, sysex_ack_size) == NAK_GRP)
	{
		Logger::getCurrentLogger()->writeToLog("recv a nak");
		return false;
	}

	if (PGSysExParser::GetOpLsb(sysex_ack, sysex_ack_size) != CUSTOM_PC_OBJ)
	{
		Logger::getCurrentLogger()->writeToLog("ack is not for CUSTOM_PC");
		return false;
	}

	return true;
}

bool PGMidiDevice::requestCustomizedCC(vector<CustomCC> &ccc)
{
	uint8 sysex_buf[MAX_SYSEX_SIZE] = { 0 };
	size_t sysex_size = PGSysExComposer::ComposeRequestCustomCCSysEx(ccc, sysex_buf);

	MidiMessage msg = MidiMessage(sysex_buf, sysex_size, 0);

	if (!MidiMessageBox.SendMessageAndWaitReply(_midiOut, msg, 5000))
		return false;

	const uint8 *sysex_reply = MidiMessageBox.ReplyMessage.getSysExData();
	int sysex_reply_size = MidiMessageBox.ReplyMessage.getSysExDataSize();

	if (PGSysExParser::GetOpLsb(sysex_reply, sysex_reply_size) != CUSTOM_CC_OBJ)
	{
		Logger::getCurrentLogger()->writeToLog("reply is not for CUSTOM_CC");
		return false;
	}

	return PGSysExParser::GetCustomCC(sysex_reply, sysex_reply_size, ccc);
}

bool PGMidiDevice::requestCustomizedPC(vector<CustomPC> &cpc)
{
	uint8 sysex_buf[MAX_SYSEX_SIZE] = { 0 };
	size_t sysex_size = PGSysExComposer::ComposeRequestCustomPCSysEx(cpc, sysex_buf);

	MidiMessage msg = MidiMessage(sysex_buf, sysex_size, 0);

	if (!MidiMessageBox.SendMessageAndWaitReply(_midiOut, msg, 5000))
		return false;

	const uint8 *sysex_reply = MidiMessageBox.ReplyMessage.getSysExData();
	int sysex_reply_size = MidiMessageBox.ReplyMessage.getSysExDataSize();

	if (PGSysExParser::GetOpLsb(sysex_reply, sysex_reply_size) != CUSTOM_PC_OBJ)
	{
		Logger::getCurrentLogger()->writeToLog("reply is not for CUSTOM_PC");
		return false;
	}

	return PGSysExParser::GetCustomPC(sysex_reply, sysex_reply_size, cpc);
}

MidiMessageBox::MidiMessageBox()
{

}

void MidiMessageBox::NotifyAck(const MidiMessage &msg)
{
	std::unique_lock<std::mutex> lck(_mutex);
	AckMessage = msg;
	_ack_cv.notify_all();
}

void MidiMessageBox::NotifyReply(const MidiMessage &msg)
{
	std::unique_lock<std::mutex> lck(_mutex);
	ReplyMessage = msg;
	_reply_cv.notify_all();
}

bool MidiMessageBox::SendMessageAndWaitAckOrNak(MidiOutput *out, MidiMessage &msg, int msec)
{
	if (!out)
		return false;

	Logger::getCurrentLogger()->writeToLog("midiout: send message and wait ack:");
	string dbg = ToHexString(msg.getRawData(), msg.getRawDataSize());
	Logger::getCurrentLogger()->writeToLog(dbg);

	std::unique_lock<std::mutex> lck(_mutex);
	out->sendMessageNow(msg);
	return std::cv_status::no_timeout == _ack_cv.wait_for(lck, std::chrono::milliseconds(msec));
}

bool MidiMessageBox::SendMessageAndWaitReply(MidiOutput *out, MidiMessage &msg, int msec)
{
	if (!out)
		return false;

	Logger::getCurrentLogger()->writeToLog("midiout: send message and wait reply");
	string dbg = ToHexString(msg.getRawData(), msg.getRawDataSize());
	Logger::getCurrentLogger()->writeToLog(dbg);

	std::unique_lock<std::mutex> lck(_mutex);
	out->sendMessageNow(msg);
	return std::cv_status::no_timeout == _reply_cv.wait_for(lck, std::chrono::milliseconds(msec));
}
#ifndef PG_DEVICE_H
#define PG_DEVICE_H

#include <JuceHeader.h>
#include <vector>
#include <mutex>
#include <condition_variable>

#include "PGSysEx.h"

enum DeviceType
{
	UNKNOWN,
	USB,
	BLE
};

enum ConnectStatus
{
	CONNECTED,
	DISCONNECTED
};

struct PGMidiDeviceDesc
{
	int midiInIndex;
	int midiOutIndex;
};


typedef void(*OnConnectStatusChanged)(ConnectStatus conn_status, void *userData);
typedef void(*OnG1ControlChange)(unsigned int ctrl_id, short value, void *userData);
typedef void(*OnFirmwareTransmitFinished)(bool succ);

class MidiMessageBox
{
	std::mutex _mutex;
	std::condition_variable _ack_cv;
	std::condition_variable _reply_cv;
public:
	MidiMessage AckMessage;
	MidiMessage ReplyMessage;

	MidiMessageBox();

	void NotifyAck(const MidiMessage &msg);
	void NotifyReply(const MidiMessage &msg);
	bool SendMessageAndWaitAckOrNak(MidiOutput *out, MidiMessage &msg, int msec);
	bool SendMessageAndWaitReply(MidiOutput *out, MidiMessage &msg, int msec);

};


class PGDevice
{
	friend class PGDeviceManager;

public:
	const PGMidiDeviceDesc Desc;


	// following virtual function need to be impl
	virtual bool saveCustomizedCC(const std::vector<CustomCC> &ccc){ return false; }
	virtual bool saveCustomizedPC(const std::vector<CustomPC> &cpc){ return false; }
	virtual bool requestCustomizedCC(std::vector<CustomCC> &ccc){ return false; }
	virtual bool requestCustomizedPC(std::vector<CustomCC> &cpc){ return false; }
	/*
	virtual bool connect(OnConnectStatusChanged handlConnStatusChange, void *connStatusParam, OnG1ControlChange handleG1ControlChange, void *g1CcParam);
	virtual void disconnect();

	virtual int savePreset(uint8 bank_id, uint8 preset_id, size_t size, char *data);
	virtual int tempSavePreset(size_t size, char *data);
	virtual int switchPreset(uint8 bank_id, uint8 preset_id);
	virtual int setDspParameter(uint8 bank_id, uint8 control_id_msb, unsigned int control_id_lsb);

	virtual int requestPreset(uint8 bank_id, uint8 preset_id, size_t *size, char **data);
	virtual int requestDeviceInfo(size_t *size, char **data);
	virtual int requestBattery(size_t *size, char **data);
	virtual int requestLed(size_t *size, char **data);
	virtual int requestButton(size_t *size, char **data);
	virtual int requestMIDIControlMap(size_t *size, char **data);
	virtual int requestPresetMap(size_t *size, char **data);
	virtual int updateFirmware(size_t size, char *data);
	virtual int updateFirmwareAsync(size_t size, char *data, OnFirmwareTransmitFinished handleFwTransmitFinish);

	virtual void sendRawMidiMessage(size_t size, char *raw_midi); //保留做新增Command的實驗
	*/

protected:
	bool _is_connected;
	DeviceType _deviceType;
	unsigned int _handle_in;
	unsigned int _handle_out;

	OnConnectStatusChanged _handlConnStatusChange;
	void *_connStatusParam;

	OnG1ControlChange _handleG1ControlChange;
	void *_g1CcParam;

	PGDevice(const PGMidiDeviceDesc &desc) :
		Desc(desc)
	{
	}

private:
};

class PGMidiDevice : public PGDevice
{
	class PGMidiInputCallback : public MidiInputCallback
	{
	public:
		PGMidiDevice *OwnerMidiDevice;
		PGMidiInputCallback(PGMidiDevice *device)
		{
			OwnerMidiDevice = device;
		}
		void handleIncomingMidiMessage(MidiInput *source, const MidiMessage &message);
	};

public:
	MidiMessageBox MidiMessageBox;

	PGMidiDevice(const PGMidiDeviceDesc &desc);
	~PGMidiDevice();

	static std::vector<PGMidiDeviceDesc> getDevices();
	bool saveCustomizedCC(const std::vector<CustomCC> &ccc);
	bool saveCustomizedPC(const std::vector<CustomPC> &cpc);
	bool requestCustomizedCC(std::vector<CustomCC> &ccc);
	bool requestCustomizedPC(std::vector<CustomPC> &cpc);
	bool open();
	void close();

private:
	MidiInput *_midiIn;
	MidiOutput *_midiOut;
	PGMidiInputCallback *_midiInputCallback;
};


/*
class PGBleDevice : public PGDevice
{
friend class PGDeviceManager;

private:
PGBleDevice(DeviceDesc desc);
};
*/


#endif  // PG_DEVICE_H

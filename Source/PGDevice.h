/*
  ==============================================================================

    PGDevice.h
    Created: 9 Jul 2015 6:13:46pm
    Author:  bb

  ==============================================================================
*/

#ifndef PGDEVICE_H_INCLUDED
#define PGDEVICE_H_INCLUDED

#include <JuceHeader.h>
#include <string>
#include <vector>
#include <map>
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

// maybe ble does not use handlIn and handleOut, if that, we need define seperatly
struct DeviceDesc
{
	int midiInIndex;
	int midiOutIndex;
	DeviceType type;
};


typedef void(*OnConnectStatusChanged)(ConnectStatus conn_status, void *userData);
typedef void(*OnG1ControlChange)(unsigned int ctrl_id, short value, void *userData);
typedef void(*OnFirmwareTransmitFinished)(bool succ);

class PGDevice
{
	friend class PGDeviceManager;

public:
	DeviceDesc Desc;


	// following virtual function need to be impl
	virtual bool saveCustomizedCC(int count, CustomCCData *udm){ return false; }
	virtual bool saveCustomizedPC(int count, CustomPCData *udm){ return false; }
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

	PGDevice(DeviceDesc desc) :
		Desc(desc)
	{
	}

private:
};

class PGMidiDevice : public PGDevice
{
	friend class PGDeviceManager;

	struct MessageNotify
	{
		std::condition_variable *cv;
		MidiMessage msg;

		MessageNotify()
		{
			cv = new std::condition_variable();
		}

		~MessageNotify()
		{
			delete cv;
		}
	};


	class PGMidiInputCallback : public MidiInputCallback
	{
	public:
		PGMidiDevice *Device;
		PGMidiInputCallback(PGMidiDevice *device)
		{
			Device = device;
		}
		void handleIncomingMidiMessage(MidiInput *source, const MidiMessage &message);
	};

public:
	~PGMidiDevice();

	std::map<int, MessageNotify> CVMap;
	std::mutex Mutex;

	static std::vector<DeviceDesc> getDevices();
	static PGMidiDevice *openDevice(DeviceDesc desc);

	bool saveCustomizedCC(int count, CustomCCData *udm);
	bool saveCustomizedPC(int count, CustomPCData *udm);
	void close();
private:

	MidiInput *_midiIn;
	MidiOutput *_midiOut;

	PGMidiInputCallback *_midiInputCallback;

	bool open();
	


private:
	PGMidiDevice(DeviceDesc desc);
};

class PGBleDevice : public PGDevice
{
	friend class PGDeviceManager;

private:
	PGBleDevice(DeviceDesc desc);
};




#endif  // PGDEVICE_H_INCLUDED

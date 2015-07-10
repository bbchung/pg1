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
typedef struct _DeviceDesc
{
	int midiInIndex;
	int midiOutIndex;
	DeviceType type;
} DeviceDesc;


typedef void(*OnConnectStatusChanged)(ConnectStatus conn_status, void *userData);
typedef void(*OnG1ControlChange)(unsigned int ctrl_id, short value, void *userData);
typedef void(*OnFirmwareTransmitFinished)(bool succ);

class PGDevice
{
	friend class PGDeviceManager;

public:
	DeviceDesc Desc;


	// following virtual function need to be impl
	/*
	virtual bool connect(OnConnectStatusChanged handlConnStatusChange, void *connStatusParam, OnG1ControlChange handleG1ControlChange, void *g1CcParam);
	virtual void disconnect();

	virtual int savePreset(unsigned char bank_id, unsigned char preset_id, size_t size, char *data);
	virtual int tempSavePreset(size_t size, char *data);
	virtual int switchPreset(unsigned char bank_id, unsigned char preset_id);
	virtual int setDspParameter(unsigned char bank_id, unsigned char control_id_msb, unsigned int control_id_lsb);

	virtual int requestPreset(unsigned char bank_id, unsigned char preset_id, size_t *size, char **data);
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

	class PGMidiInputCallback : public MidiInputCallback
	{
		void handleIncomingMidiMessage(MidiInput *source, const MidiMessage &message);
	};

public:
	static std::vector<DeviceDesc> getDevices();
	static PGMidiDevice *openDevice(DeviceDesc desc);

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

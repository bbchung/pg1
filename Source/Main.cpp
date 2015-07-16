#include "PGDevice.h"

using namespace std;

//==============================================================================
int main(int argc, char* argv[])
{
	FileLogger *logger = new FileLogger(File(File::getCurrentWorkingDirectory().getChildFile("midi_comm.log")), "midi comm log", 0);
	Logger::setCurrentLogger(logger);

	vector<PGMidiDeviceDesc> devices;

	while (1)
	{
		cout << "search G1 midi device... ";
		devices = PGMidiDevice::getDevices();
		PGMidiDeviceDesc d;

		cout << devices.size() << " devices found" << endl;

		if (devices.size() > 0)
			break;

		cout << "no G1 midi device was searched, press any key to search again..." << endl;
		cin.get();
	}

	int index;
	PGMidiDevice *device;

	while (1)
	{
	cout << "select(0 - " << devices.size() - 1 << ") to open G1: ";


	if (!(cin >> index) || index > devices.size() - 1)
	{
	cin.clear();
	cin.ignore(numeric_limits<streamsize>::max(), '\n');
	cout << "invalid input, try again" << endl << endl;
	continue;
	}

	cout << "open G1 device " << index << "... ";

	device = new PGMidiDevice(devices[index]);
	if (device->open())
	{
	cout << "ok" << endl;
	break;
	}

	cout << "failed" << endl;
	}

	while (1)
	{
		cout << "1: save custom cc to g1" << endl;
		cout << "2: save custom pc to g1" << endl;
		cout << "3: request custom pc to g1" << endl;
		cout << "4: request custom pc to g1" << endl;
		cout << endl;
		cout << "select midi function: ";

		if (!(cin >> index) || index < 1 || index > 2)
		{
			cin.clear();
			cin.ignore(numeric_limits<streamsize>::max(), '\n');
			cout << "invalid input, try again" << endl << endl;
			continue;
		}

		switch (index)
		{
		case 1:
		{
			cout << "number to custom? ";
			int number;
			if (!(cin >> number))
			{
				cin.clear();
				cin.ignore(numeric_limits<streamsize>::max(), '\n');
				cout << "invalid input, try again" << endl << endl;
				continue;
			}

			vector<CustomCC> customs;
			for (int i = 0; i < number; ++i)
			{
				CustomCC ccc;
				cout << number - i << " left..." << endl;

				cout << "channel: ";
				cin >> ccc.channel;

				cout << "control_number: ";
				cin >> ccc.control_number;

				cout << "knobId: ";
				cin >> ccc.knobId;

				cout << endl;

				customs.push_back(ccc);
			}

			cout << "start save custom cc" << endl;
			device->saveCustomizedCC(customs);
			break;
		}
		case 2:
		{
			cout << "number to custom? ";
			int number;
			if (!(cin >> number))
			{
				cin.clear();
				cin.ignore(numeric_limits<streamsize>::max(), '\n');
				cout << "invalid input, try again" << endl;
				continue;
			}

			vector<CustomPC> customs;
			for (int i = 0; i < number; ++i)
			{
				CustomPC cpc;
				cout << number - i << " left..." << endl;

				cout << "channel: ";
				cin >> cpc.channel;

				cout << "preset_number: ";
				cin >> cpc.preset_number;

				cout << "model: ";
				cin >> cpc.model;

				cout << "type: ";
				cin >> cpc.type;

				cout << endl;
				customs.push_back(cpc);
			}

			cout << "start save custom pc" << endl;
			device->saveCustomizedPC(customs);

			break;
		}
		case 3:
		{
			cout << "number to request? ";
			int number;
			if (!(cin >> number))
			{
				cin.clear();
				cin.ignore(numeric_limits<streamsize>::max(), '\n');
				cout << "invalid input, try again" << endl;
				continue;
			}

			vector<CustomCC> customs;
			for (int i = 0; i < number; ++i)
			{
				CustomCC ccc;
				cout << number - i << " left..." << endl;

				cout << "channel: ";
				cin >> ccc.channel;

				cout << "control_number: ";
				cin >> ccc.control_number;

				cout << endl;
				customs.push_back(ccc);
			}

			cout << "start requset custom cc" << endl;
			device->requestCustomizedCC(customs);
			break;
		}
		case 4:
		{
			cout << "number to request? ";
			int number;
			if (!(cin >> number))
			{
				cin.clear();
				cin.ignore(numeric_limits<streamsize>::max(), '\n');
				cout << "invalid input, try again" << endl;
				continue;
			}

			vector<CustomPC> customs;
			for (int i = 0; i < number; ++i)
			{
				CustomPC cpc;
				cout << number - i << " left..." << endl;

				cout << "channel: ";
				cin >> cpc.channel;

				cout << "preset_number: ";
				cin >> cpc.preset_number;

				cout << endl;
				customs.push_back(cpc);
			}

			cout << "start request custom pc" << endl;
			device->requestCustomizedPC(customs);

			break;

		}
		default:
			break;
		}
	}


	cin.get();
	return 0;
}

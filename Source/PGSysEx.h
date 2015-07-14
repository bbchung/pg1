/*
  ==============================================================================

    CommandData.h
    Created: 13 Jul 2015 12:00:18pm
    Author:  bb

  ==============================================================================
*/

#ifndef COMMANDDATA_H_INCLUDED
#define COMMANDDATA_H_INCLUDED

#define MAX_OP_DATA_SIZE 256

#define MFRS 0x00

#define BIT0_MASK 0x00000001
#define BIT1_MASK 0x00000002
#define BIT2_MASK 0x00000004
#define BIT3_MASK 0x00000008
#define BIT4_MASK 0x00000010
#define BIT5_MASK 0x00000020
#define BIT6_MASK 0x00000040
#define BIT7_MASK 0x00000080
#define BIT8_MASK 0x00000100

enum OP_GROUP
{
	SAVE = 0x01,
	REQUEST,
	REPLY,
	ACK
};

enum OP_TARGET
{
	CUSTOM_CC = 0x01,
	CUSTOM_PC,
};

#pragma pack(push)
#pragma pack(1)
struct CustomCCData
{
	unsigned char channel;
	unsigned char control_number;
	unsigned int knob_id;
};

struct CustomPCData
{
	unsigned char channel;
	unsigned char preset;
	unsigned int type;
	unsigned int model;
};
#pragma pack(pop)


static unsigned int SetupGeneralDataFromMidiData(unsigned char *midiBuf, unsigned char *dataBuf, unsigned int dataLen);
static unsigned int SetupMidiDataFromGeneralData(unsigned char *midiBuf, unsigned char *dataBuf, unsigned int dataLen);
static int Encode7Bit(int raw_size, unsigned char *raw_buf, unsigned char **enc_buf);
static int Decode7Bit(int raw_size, unsigned char *raw_buf, unsigned char **dec_buf);
static size_t ComposeSysExBuf(unsigned char op_msb, unsigned char op_lsb, unsigned char *op_data, int op_data_size, unsigned char **sysex_buf);

size_t ComposeCustomCCSysEx(int count, CustomCCData *udm, unsigned char **sysex_buf);
size_t ComposeCustomPCSysEx(int count, CustomPCData *udm, unsigned char **sysex_buf);



#endif  // COMMANDDATA_H_INCLUDED

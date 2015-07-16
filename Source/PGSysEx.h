#ifndef PG_SYSEX_H
#define PG_SYSEX_H

#define MAX_OP_8_SIZE 768
#define MAX_OP_7_SIZE (MAX_OP_8_SIZE * 8 + 6)/7
#define MAX_SYSEX_SIZE 1024

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
	SAVE_GRP = 0x01,
	REQUEST_GRP,
	REPLY_GRP,
	ACK_GRP,
	NAK_GRP,
};


enum OP_OBJECT
{
	PRESET_OBJ = 0x01,
	CUSTOM_CC_OBJ,
	CUSTOM_PC_OBJ,
};

#pragma pack(push)
#pragma pack(1)
struct CustomCC
{
	unsigned char channel;
	unsigned char control_number;
	unsigned int knobId;
};

struct CustomPC
{
	unsigned char channel;
	unsigned char preset_number;
	unsigned int type;
	unsigned int model;
};
#pragma pack(pop)


static unsigned int SetupGeneralDataFromMidiData(const unsigned char *midiBuf, unsigned char *dataBuf, unsigned int dataLen);
static unsigned int SetupMidiDataFromGeneralData(unsigned char *midiBuf, unsigned char *dataBuf, unsigned int dataLen);
static int Encode7Bit(int raw_size, unsigned char *raw_buf, unsigned char **enc_buf);
static int Decode7Bit(int raw_size, unsigned char *raw_buf, unsigned char **dec_buf);


class PGSysExComposer
{
public:
	static size_t ComposeSaveCustomCCSysEx(int count, CustomCC *ccc, unsigned char *sysex_buf);
	static size_t ComposeSaveCustomPCSysEx(int count, CustomPC *cpc, unsigned char *sysex_buf);
	static size_t ComposeRequestCustomCCSysEx(int count, CustomCC *ccc, unsigned char *sysex_buf);
	static size_t ComposeRequestCustomPCSysEx(int count, CustomPC *cpc, unsigned char *sysex_buf);

private:
	static size_t ComposePGSysEx(unsigned char op_msb, unsigned char op_lsb, unsigned char *opdata, int opdata_size, unsigned char *sysex_buf);

};

class PGSysExParser
{
public:
	static bool IsPGSysEx(const unsigned char *sysex_buf, size_t sysex_size);
	static unsigned char GetOpMsb(const unsigned char *sysex_buf, size_t sysex_size);
	static unsigned char GetOpLsb(const unsigned char *sysex_buf, size_t sysex_size);
	static size_t GetOpData(const unsigned char *sysex_buf, size_t sysex_size, const unsigned char **opdata);
	static bool GetCustomCC(const unsigned char *sysex_buf, size_t sysex_size, CustomCC *ccc, size_t size);
	static bool GetCustomPC(const unsigned char *sysex_buf, size_t sysex_size, CustomPC *cpc, size_t size);
};

#endif  // COMMANDDATA_H_INCLUDED
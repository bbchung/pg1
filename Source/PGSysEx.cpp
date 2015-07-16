#include "PGSysEx.h"

#include <stdlib.h>
#include <string.h>

unsigned int SetupMidiDataFromGeneralData(unsigned char *midiBuf, unsigned char *dataBuf, unsigned int dataLen)
{
	unsigned char *p_cc;
	unsigned int mididatalen;
	mididatalen = (unsigned int)midiBuf;
	while (dataLen)
	{
		p_cc = midiBuf;
		*p_cc = 0x00;
		midiBuf++;      //bit7 group char is the 0# 

		*midiBuf = (*dataBuf) & 0x7f;
		if ((*dataBuf) & 0x80) *p_cc |= BIT0_MASK;
		midiBuf++;
		dataBuf++;
		dataLen--;
		if (!dataLen) break;    //1#

		*midiBuf = (*dataBuf) & 0x7f;
		if ((*dataBuf) & 0x80) *p_cc |= BIT1_MASK;
		midiBuf++;
		dataBuf++;
		dataLen--;
		if (!dataLen) break;    //2#

		*midiBuf = (*dataBuf) & 0x7f;
		if ((*dataBuf) & 0x80) *p_cc |= BIT2_MASK;
		midiBuf++;
		dataBuf++;
		dataLen--;
		if (!dataLen) break;    //3#

		*midiBuf = (*dataBuf) & 0x7f;
		if ((*dataBuf) & 0x80) *p_cc |= BIT3_MASK;
		midiBuf++;
		dataBuf++;
		dataLen--;
		if (!dataLen) break;    //4#

		*midiBuf = (*dataBuf) & 0x7f;
		if ((*dataBuf) & 0x80) *p_cc |= BIT4_MASK;
		midiBuf++;
		dataBuf++;
		dataLen--;
		if (!dataLen) break;    //5#

		*midiBuf = (*dataBuf) & 0x7f;
		if ((*dataBuf) & 0x80) *p_cc |= BIT5_MASK;
		midiBuf++;
		dataBuf++;
		dataLen--;
		if (!dataLen) break;    //6#

		*midiBuf = (*dataBuf) & 0x7f;
		if ((*dataBuf) & 0x80) *p_cc |= BIT6_MASK;
		midiBuf++;
		dataBuf++;
		dataLen--;
		if (!dataLen) break;    //7#

	}
	mididatalen = (unsigned int)midiBuf - mididatalen;
	return mididatalen;
}

unsigned int SetupGeneralDataFromMidiData(const unsigned char *midiBuf, unsigned char *dataBuf, unsigned int dataLen)
{
	unsigned char cc;
	unsigned char *start = dataBuf;

	while (dataLen)
	{
		cc = *midiBuf;      //bit7 group char
		midiBuf++;
		dataLen--;

		*dataBuf = *midiBuf;
		if (cc&BIT0_MASK)
			*dataBuf |= 0x80;
		midiBuf++;
		dataBuf++;
		dataLen--;
		if (!dataLen) break;    //1#

		*dataBuf = *midiBuf;
		if (cc&BIT1_MASK)
			*dataBuf |= 0x80;
		midiBuf++;
		dataBuf++;
		dataLen--;
		if (!dataLen) break;    //2#

		*dataBuf = *midiBuf;
		if (cc&BIT2_MASK)
			*dataBuf |= 0x80;
		midiBuf++;
		dataBuf++;
		dataLen--;
		if (!dataLen) break; //3#

		*dataBuf = *midiBuf;
		if (cc&BIT3_MASK)
			*dataBuf |= 0x80;
		midiBuf++;
		dataBuf++;
		dataLen--;
		if (!dataLen) break;    //4#

		*dataBuf = *midiBuf;
		if (cc&BIT4_MASK)
			*dataBuf |= 0x80;
		midiBuf++;
		dataBuf++;
		dataLen--;
		if (!dataLen) break; //5#

		*dataBuf = *midiBuf;
		if (cc&BIT5_MASK)
			*dataBuf |= 0x80;
		midiBuf++;
		dataBuf++;
		dataLen--;
		if (!dataLen) break;    //6#

		*dataBuf = *midiBuf;
		if (cc&BIT6_MASK)
			*dataBuf |= 0x80;
		midiBuf++;
		dataBuf++;
		dataLen--;
		if (!dataLen) break;    //7#
	}
	return unsigned int(dataBuf - start);
}

int Encode7Bit(int raw_size, unsigned char *raw_buf, unsigned char **enc_buf)
{
	if (!raw_buf || !enc_buf)
		return -1;

	int enc_size = (raw_size * 8 + 6) / 7;

	*enc_buf = (unsigned char *)malloc(enc_size);
	memset(*enc_buf, 0, enc_size);

	unsigned char mask[] = { 0x80, 0xc0, 0xe0, 0xf0, 0xf8, 0xfc, 0xfe };

	int pr = 0; // raw_buf pointer
	int pe = 0; // enc_buf pointer

	while (pr < raw_size)
	{
		int mod = pe % 8;

		if (mod == 0)
		{
			(*enc_buf)[pe] = 0x7f & raw_buf[pr];
		}
		else
		{
			(*enc_buf)[pe] = (raw_buf[pr] & mask[mod - 1]) >> (8 - mod);

			if (pr + 1 < raw_size && mod < 7)
				(*enc_buf)[pe] |= ((raw_buf[pr + 1] & ~mask[mod])) << mod;

			pr++;
		}

		pe++;
	}

	return enc_size;
}

int Decode7Bit(int raw_size, unsigned char *raw_buf, unsigned char **dec_buf)
{
	if (!raw_buf || !dec_buf)
		return -1;

	int dec_size = (raw_size * 7) / 8;

	*dec_buf = (unsigned char *)malloc(dec_size);
	memset(*dec_buf, 0, dec_size);

	unsigned char mask[] = { 0x01, 0x03, 0x07, 0x0f, 0x1f, 0x3f, 0x7f };

	int pr = 0; // raw_buf pointer
	int pd = 0; // dec_buf pointer

	while (pr < raw_size)
	{
		int mod = pr % 8;

		if (pr + 1 < raw_size && mod <= 6)
		{
			(*dec_buf)[pd] = ((raw_buf[pr] >> mod) & mask[6 - mod]);
			(*dec_buf)[pd] |= ((raw_buf[pr + 1] & mask[mod])) << (7 - mod);
			pd++;
		}

		pr++;
	}

	return dec_size;
}

size_t PGSysExComposer::ComposePGSysEx(unsigned char op_msb, unsigned char op_lsb, unsigned char *opdata, int opdata_size, unsigned char *sysex_buf)
{
	if (!opdata || !sysex_buf)
		return 0;

	size_t sysex_size = 5 + opdata_size;

	sysex_buf[0] = 0xf0;
	sysex_buf[1] = MFRS;
	sysex_buf[2] = op_msb;
	sysex_buf[3] = op_lsb;
	memcpy(sysex_buf + 4, opdata, opdata_size);
	sysex_buf[sysex_size - 1] = 0xf7;

	return sysex_size;
}

size_t PGSysExComposer::ComposeSaveCustomCCSysEx(int count, CustomCC *ccc, unsigned char *sysex_buf)
{
	if (!ccc || !sysex_buf)
		return 0;

	unsigned char opdata8[MAX_OP_8_SIZE] = { 0 };
	size_t size = 4 + sizeof(CustomCC) * count;
	memcpy(opdata8, &count, 4);
	memcpy(opdata8 + 4, ccc, sizeof(CustomCC) * count);

	unsigned char opdata7[MAX_OP_7_SIZE] = { 0 };
	int opdata7_size = SetupMidiDataFromGeneralData(opdata8, opdata7, size);

	size_t sysex_size = ComposePGSysEx(SAVE_GRP, CUSTOM_CC_OBJ, opdata7, opdata7_size, sysex_buf);
	return sysex_size;
}

size_t PGSysExComposer::ComposeSaveCustomPCSysEx(int count, CustomPC *cpc, unsigned char *sysex_buf)
{
	if (!cpc || !sysex_buf)
		return 0;

	unsigned char opdata8[MAX_OP_8_SIZE] = { 0 };
	size_t size = 4 + sizeof(CustomPC) * count;
	memcpy(opdata8, &count, 4);
	memcpy(opdata8 + 4, cpc, sizeof(CustomPC) * count);

	unsigned char opdata7[MAX_OP_7_SIZE] = { 0 };
	int opdata7_size = SetupMidiDataFromGeneralData(opdata8, opdata7, size);

	size_t sysex_size = ComposePGSysEx(SAVE_GRP, CUSTOM_PC_OBJ, opdata7, opdata7_size, sysex_buf);
	return sysex_size;
}

size_t PGSysExComposer::ComposeRequestCustomCCSysEx(int count, CustomCC *ccc, unsigned char *sysex_buf)
{
	if (!ccc || !sysex_buf)
		return 0;

	unsigned char opdata8[MAX_OP_8_SIZE] = { 0 };
	memcpy(opdata8, &count, 4);

	int size = 4;
	for (int i = 0; i < count; ++i)
	{
		opdata8[size++] = (ccc + i)->channel;
		opdata8[size++] = (ccc + i)->control_number;
	}

	unsigned char opdata7[MAX_OP_7_SIZE] = { 0 };
	int opdata7_size = SetupMidiDataFromGeneralData(opdata8, opdata7, size);

	size_t sysex_size = ComposePGSysEx(REQUEST_GRP, CUSTOM_CC_OBJ, opdata7, opdata7_size, sysex_buf);
	return sysex_size;
}

size_t PGSysExComposer::ComposeRequestCustomPCSysEx(int count, CustomPC *cpc, unsigned char *sysex_buf)
{
	if (!cpc || !sysex_buf)
		return 0;

	unsigned char opdata8[MAX_OP_8_SIZE] = { 0 };
	memcpy(opdata8, &count, 4);

	int size = 4;
	for (int i = 0; i < count; ++i)
	{
		opdata8[size++] = (cpc + i)->channel;
		opdata8[size++] = (cpc + i)->preset_number;
	}

	unsigned char opdata7[MAX_OP_7_SIZE] = { 0 };
	int opdata7_size = SetupMidiDataFromGeneralData(opdata8, opdata7, size);

	size_t sysex_size = ComposePGSysEx(REQUEST_GRP, CUSTOM_PC_OBJ, opdata7, opdata7_size, sysex_buf);
	return sysex_size;
}

bool PGSysExParser::IsPGSysEx(const unsigned char *sysex_buf, size_t sysex_size)
{
	return sysex_buf[1] == MFRS;
}

unsigned char PGSysExParser::GetOpMsb(const unsigned char *sysex_buf, size_t sysex_size)
{
	return sysex_buf[2];
}

unsigned char PGSysExParser::GetOpLsb(const unsigned char *sysex_buf, size_t sysex_size)
{
	return sysex_buf[3];
}

size_t PGSysExParser::GetOpData(const unsigned char *sysex_buf, size_t sysex_size, const unsigned char **opdata)
{
	*opdata = sysex_buf + 4;
	return sysex_size - 5;
}

bool PGSysExParser::GetCustomCC(const unsigned char *sysex_buf, size_t sysex_size, CustomCC *ccc, size_t count)
{
	unsigned char opdata8[MAX_OP_8_SIZE] = { 0 };
	const unsigned char *opdata7;
	size_t opdata7_size = GetOpData(sysex_buf, sysex_size, &opdata7);
	int opdata8_size = SetupGeneralDataFromMidiData(opdata7, opdata8, opdata7_size);

	int reply_count = *((int *)opdata8);

	if (reply_count != count)
		return false;

	if (opdata8_size != 4 + sizeof(CustomCC) * count)
		return false;

	int offset = 4;
	for (int i = 0; i < count; ++i)
	{
		(ccc + i)->channel = opdata8[offset];
		++offset;

		(ccc + i)->control_number = opdata8[offset];
		++offset;

		(ccc + i)->knobId = *((unsigned int *)(opdata8 + offset));
		offset += 4;
	}

	return true;
}

bool PGSysExParser::GetCustomPC(const unsigned char *sysex_buf, size_t sysex_size, CustomPC *cpc, size_t count)
{
	unsigned char opdata8[MAX_OP_8_SIZE] = { 0 };
	const unsigned char *opdata7;
	size_t opdata7_size = GetOpData(sysex_buf, sysex_size, &opdata7);
	int opdata8_size = SetupGeneralDataFromMidiData(opdata7, opdata8, opdata7_size);

	int reply_count = *((int *)opdata8);

	if (reply_count != count)
		return false;

	if (opdata8_size != 4 + sizeof(CustomPC) * count)
		return false;

	int offset = 4;
	for (int i = 0; i < count; ++i)
	{
		(cpc + i)->channel = opdata8[offset];
		++offset;

		(cpc + i)->preset_number = opdata8[offset];
		++offset;

		(cpc + i)->type = *((unsigned int *)(opdata8 + offset));
		offset += 4;

		(cpc + i)->model = *((unsigned int *)(opdata8 + offset));
		offset += 4;
	}

	return true;
}
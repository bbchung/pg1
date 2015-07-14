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

unsigned int SetupGeneralDataFromMidiData(unsigned char *midiBuf, unsigned char *dataBuf, unsigned int dataLen)
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

size_t ComposeSysExBuf(unsigned char op_msb, unsigned char op_lsb, unsigned char *op_data, int op_data_size, unsigned char **sysex_buf)
{
	if (!op_data || !sysex_buf)
		return 0;

	size_t len = 5 + op_data_size;
	unsigned char *buf = (unsigned char *)malloc(len);

	buf[0] = 0xf0;
	buf[1] = MFRS;
	buf[2] = op_msb;
	buf[3] = op_lsb;
	memcpy(buf + 4, op_data, op_data_size);
	buf[len - 1] = 0xf7;

	*sysex_buf = buf;

	return len;
}

size_t ComposeCustomCCSysEx(int count, CustomCCData *udm, unsigned char **sysex_buf)
{
	if (!udm || !sysex_buf)
		return 0;

	unsigned char op_data[MAX_OP_DATA_SIZE] = { 0 };
	size_t op_data_size = 4 + sizeof(CustomCCData) * count;
	memcpy(op_data, &count, 4);
	memcpy(op_data + 4, udm, sizeof(CustomCCData) * count);

	unsigned char enc_buf[256] = { 0 };
	int enc_size = SetupMidiDataFromGeneralData(op_data, enc_buf, op_data_size);

	size_t sysex_buf_len = ComposeSysExBuf(SAVE, CUSTOM_CC, enc_buf, enc_size, sysex_buf);
	return sysex_buf_len;
}

size_t ComposeCustomPCSysEx(int count, CustomPCData *udm, unsigned char **sysex_buf)
{
	if (!udm || !sysex_buf)
		return 0;

	unsigned char op_data[MAX_OP_DATA_SIZE] = { 0 };
	size_t op_data_size = 4 + sizeof(CustomPCData) * count;
	memcpy(op_data, &count, 4);
	memcpy(op_data + 4, udm, sizeof(CustomPCData) * count);

	unsigned char enc_buf[256] = { 0 };
	int enc_size = SetupMidiDataFromGeneralData(op_data, enc_buf, op_data_size);

	size_t sysex_buf_len = ComposeSysExBuf(SAVE, CUSTOM_PC, enc_buf, enc_size, sysex_buf);
	return sysex_buf_len;
}
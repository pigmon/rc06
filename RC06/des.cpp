#include <assert.h>
#include <iostream>

#include "des.h"
#include "tables.h"

DES::DES(const std::string& _key)
{
	m_key = encode_key(_key);
	m_sub_keys.resize(16);
	gen_sub_keys();
}

DES::~DES()
{
	m_sub_keys.clear();
}

BitArray_32 DES::f(const BitArray_32& _r, const BitArray_48& _key)
{
	BitArray_32 ret;
	BitArray_48 expand_r;

	// 1. E扩展置换
	for (int i = 0; i < 48; ++i)
		expand_r[47 - i] = _r[32 - E[i]];

	// 2. 异或
	expand_r = expand_r ^ _key;

	// 3. S-Box 置换
	int x = 0;
	for (int i = 0; i < 48; i += 6)
	{
		int row = expand_r[47 - i] * 2 + expand_r[47 - i - 5];
		int col = expand_r[47 - i - 1] * 8 + expand_r[47 - i - 2] * 4 + expand_r[47 - i - 3] * 2 + expand_r[47 - i - 4];
		int num = S_BOX[i / 6][row][col];
		BitArray_4 binary(num);
		ret[31 - x] = binary[3];
		ret[31 - x - 1] = binary[2];
		ret[31 - x - 2] = binary[1];
		ret[31 - x - 3] = binary[0];
		x += 4;
	}

	// 4. P置换
	BitArray_32 tmp = ret;
	for (int i = 0; i < 32; ++i)
		ret[31 - i] = tmp[32 - P[i]];

	return ret;
}

void DES::left_shift_in_key_56(BitArray_28& _key, const int _shift)
{
	BitArray_28 tmp = _key;
	for (int i = 27; i >= 0; --i)
	{
		if (i - _shift < 0)
			_key[i] = tmp[i - _shift + 28];
		else
			_key[i] = tmp[i - _shift];
	}
}

BitArray_64 DES::encode_key(const char _key[8])
{
	BitArray_64 bits;
	for (int i = 0; i < 8; ++i)
		for (int j = 0; j < 8; ++j)
			bits[i * 8 + j] = ((_key[i] >> j) & 1);
	return bits;
}

BitArray_64 DES::encode_key(const std::string& _key)
{
	assert(_key.length() == 8);

	return encode_key(_key.c_str());
}

void DES::gen_sub_keys()
{
	BitArray_56 key_56;
	BitArray_28 left;
	BitArray_28 right;
	BitArray_48 key_48;

	// 1. 秘钥置换，去掉每个字节的第8位，得到56位key
	for (int i = 0; i < 56; ++i)
		key_56[55 - i] = m_key[64 - PC_1[i]];

	// 2. 得到16轮迭代用的sub key
	for (int round = 0; round < 16; ++round)
	{
		BitArray_28 left(key_56.to_string());
		BitArray_28 right((key_56 << 28).to_string());

		left_shift_in_key_56(left, shiftBits[round]);
		left_shift_in_key_56(right, shiftBits[round]);

		for (int i = 28; i < 56; ++i)
			key_56[i] = left[i - 28];
		for (int i = 0; i < 28; ++i)
			key_56[i] = right[i];

		for (int i = 0; i < 48; ++i)
			key_48[47 - i] = key_56[56 - PC_2[i]];

		m_sub_keys[round] = key_48;
	}
}

BitArray_64 DES::encrypt(const BitArray_64& _plain)
{
	BitArray_64 bits;
	BitArray_32 tmp;

	// 1. IP置换
	for (int i = 0; i < 64; ++i)
		bits[63 - i] = _plain[64 - IP[i]];

	// 2. 得到 L 和 R
	BitArray_32 left((unsigned long)((bits >> 32).to_ullong()));
	BitArray_32 right((unsigned long)(bits.to_ullong()));

	// 3. 16 轮迭代
	for (int round = 0; round < 16; ++round)
	{
		tmp = right;
		right = left ^ f(right, m_sub_keys[round]);
		left = tmp;
	}

	// 4. 迭代最后组合，R16L16
	BitArray_64 cipher((right.to_ullong()) << 32 | left.to_ullong());

	// 5. IP-1 置换
	bits = cipher;	
	for (int i = 0; i < 64; ++i)
		cipher[63 - i] = bits[64 - IP_1[i]];

	return cipher;
}

BitArray_64 DES::encrypt(const char* _str)
{
	BitArray_64 plain = encode_key(_str);
	return encrypt(plain);
}

BitArray_64 DES::decrypt(const BitArray_64& _cipher)
{
	BitArray_64 bits;
	BitArray_32 newLeft;

	// 1. IP置换
	for (int i = 0; i < 64; ++i)
		bits[63 - i] = _cipher[64 - IP[i]];

	// 2. 得到 L 和 R
	BitArray_32 left((unsigned long)((bits >> 32).to_ullong()));
	BitArray_32 right((unsigned long)(bits.to_ullong()));

	// 3. 16 轮迭代，sub key顺序要反过来
	for (int round = 0; round < 16; ++round)
	{
		newLeft = right;
		right = left ^ f(right, m_sub_keys[15 - round]);
		left = newLeft;
	}

	// 4. 迭代最后组合，R16L16
	BitArray_64 plain((right.to_ullong()) << 32 | left.to_ullong());

	// 5. IP-1 置换
	bits = plain;
	for (int i = 0; i < 64; ++i)
		plain[63 - i] = bits[64 - IP_1[i]];

	return plain;
}
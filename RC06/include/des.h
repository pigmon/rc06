#pragma once

#include <string>
#include <vector>

#include "defines.h"

class DES
{
private:
	BitArray_64 m_key;
	std::vector<BitArray_48> m_sub_keys;

public:
	DES(const std::string & _key);
	~DES();

private:
	static BitArray_32 f(const BitArray_32& _r, const BitArray_48& _key);
	static void left_shift_in_key_56(BitArray_28& _key, const int _shift);
	
	void gen_sub_keys();

public:
	BitArray_64 encrypt(const BitArray_64& _plain);
	BitArray_64 encrypt(const char* _str);
	BitArray_64 decrypt(const BitArray_64& _cipher);

	static BitArray_64 encode_key(const char _key[8]);
	static BitArray_64 encode_key(const std::string& _key);
};
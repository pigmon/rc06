#pragma once

#include <iostream>
#include <sstream>
#include <assert.h>
#include <string.h>

#include "des.h"

using namespace std;


/* -----------------------------------------------------------------------------------------
 ˵����
 ���ڴ���ṹ�����ͱ��ĵļ��ܣ����Ͷˣ��ͽ��ܣ����նˣ��Ĺ��߼���

 Use Case:

	const char* key = "T&^9c=A`";

	DGram_RemoteControl obj(1);
	obj.m_unuse2 = 121; // ���һ����Ա���һ������ֵ�Թ����

	/// Step 1:
	/// ���Ͷ� - ���ṹ�������ܣ��õ��ֽ�����
	char* arr_encrypt = DGramUtil::EncryptStruct<DGram_RemoteControl>(obj, key);


	/// Step 2:
	/// ���ն� - ���ֽ�������ܺ�ת���ɽṹ�����
	if (arr_encrypt)
	{
		/// Step 2.1 : �������ڽ������ݵĽṹ�����
		DGram_RemoteControl obj2(2);
		/// Step 2.2 : �ؼ����裬���ý��ܲ�ת���ɽṹ�����ĺ���
		bool result = DGramUtil::DecryptToStruct<DGram_RemoteControl>(arr_encrypt, key, obj2);

		// ��֤ - ����ṹ����ǰ�˺����˵��ֶν�����֤
		std::cout << std::hex << obj2.m_gram_header << ", " << std::dec << obj2.m_unuse2 << std::endl;

		/// Step 2.3 : �����Դ��Ҫ�ֶ��ͷţ�����
		delete[] arr_encrypt;
	}
 ----------------------------------------------------------------------------------------- */

namespace DGramUtil
{
	/// <summary>
	/// ���ṹ��ת����Byte���飬����Socket����
	/// </summary>
	/// <typeparam name="T">[in type] �ṹ������</typeparam>
	/// <param name="_struct">[in] �ṹ�����</param>
	/// <returns>ת�����Byte����</returns>
	template<typename T>
	char* StructToBytes(T _struct)
	{
		size_t len = sizeof(_struct);
		char* ret = new char[len];

#ifdef __linux__
		auto err_num = memmove(ret, &_struct, len);
#else
		auto err_num = memmove_s(ret, len, &_struct, len);
#endif

		if (err_num != 0)
		{
			std::cerr << "Memmove failed in <StructToBytes> with error code " << err_num << std::endl;
			return NULL;
		}
		else
			return ret;
	}

	/// <summary>
	/// ��Byte����ת��������T�Ľṹ�壬����Socket����
	/// </summary>
	/// <typeparam name="T">[out type] �ṹ������</typeparam>
	/// <param name="_arr">[in] ��ת���ɽṹ��T��Byte����</param>
	/// <param name="_out">[out] �ṹ��T����</param>
	/// <returns>�ɹ� true������ false</returns>
	template<typename T>
	bool BytesToStruct(char _arr[], T& _out)
	{
		size_t len = sizeof(T);
		assert(_arr != 0);
		assert(len % 8 == 0);

#ifdef __linux__
		auto err_num = memmove(&_out, _arr, len);
#else
		auto err_num = memmove_s(&_out, len, _arr, len);
#endif

		bool ret = err_num == 0;

		if (err_num != 0)
		{
			std::cerr << "Memmove failed in <BytesToStruct> with error code " << err_num << std::endl;
		}

		return ret;
	}

	/// <summary>
	/// ��������ʱ�����ֽ��������
	/// </summary>
	/// <param name="_arr">[in] �ֽ�����</param>
	/// <param name="_key">[in] DES�����õ�64λ��Կ</param>
	/// <param name="_size">[in] �ֽ����鳤��</param>
	/// <returns>���ܺ��Byte����</returns>
	char* EncryptBytes(char _arr[], const char* _key, const int _size)
	{
		assert(_arr != NULL);
		assert(strlen(_key) == 8);
		assert(_size % 8 == 0);

		int batch_count = _size / 8;

		DES* des = new DES(_key);
		char* ret = new char[_size];
		std::stringstream ss;

		std::string str_arr = _arr;

		for (int i = 0; i < batch_count; i++)
		{
			char tmp[8];
#ifdef __linux__
			auto err_num = memmove(tmp, _arr + i * 8, 8);
#else
			memmove_s(tmp, 8, _arr + i * 8, 8);
#endif
			std::bitset<64> cipher = des->encrypt(tmp);
			ss.write((char*)&cipher, 8);
		}

		delete des;

#ifdef __linux__
		auto err_num = memmove(ret, ss.str().c_str(), _size);
#else
		auto err_num = memmove_s(ret, _size, ss.str().c_str(), _size);
#endif

		if (err_num != 0)
		{
			std::cerr << "Memmove failed in <Encryptchars> with error code " << err_num << std::endl;
			return NULL;
		}
		else
			return ret;
	}

	/// <summary>
	/// ��������ʱ�����ֽ��������
	/// </summary>
	/// <param name="_arr">[in] �ֽ�����</param>
	/// <param name="_key">[in] DES�����õ�64λ��Կ</param>
	/// <param name="_size">[in] �ֽ����鳤��</param>
	/// <returns>���ܺ��Byte����</returns>
	char* DecryptBytes(char _arr[], const char* _key, const int _size)
	{
		assert(_arr != NULL);
		assert(strlen(_key) == 8);
		assert(_size % 8 == 0);

		int batch_count = _size / 8;

		DES* des = new DES(_key);
		std::stringstream ss;
		std::string str_arr = _arr;

		for (int i = 0; i < batch_count; i++)
		{
			char tmp[8];
#ifdef __linux__
			auto err_num = memmove(tmp, _arr + i * 8, 8);
#else			
			memmove_s(tmp, 8, _arr + i * 8, 8);
#endif

			std::bitset<64> part = DES::encode_key(tmp);
			std::bitset<64> plain = des->decrypt(part);
			ss.write((char*)&plain, 8);
		}

		delete des;

		char* ret = new char[_size];
#ifdef __linux__
		auto err_num = memmove(ret, ss.str().c_str(), _size);
#else			
		auto err_num = memmove_s(ret, _size, ss.str().c_str(), _size);
#endif 

		if (err_num != 0)
		{
			std::cerr << "Memmove failed in <DecryptBytes> with error code " << err_num << std::endl;
			return NULL;
		}
		else
			return ret;
	}

	/// <summary>
	/// ���ṹ��ת����Byte���飬Ȼ����ܳ��ֽ����飬����Socket����
	/// </summary>
	/// <typeparam name="T">[in type] �ṹ������</typeparam>
	/// <param name="_struct">[in] �ṹ�����</param>
	/// <param name="_key">[in] DES�����õ�64λ��Կ</param>
	/// <returns>���ܺ��Byte����</returns>
	template<typename T>
	char* EncryptStruct(T _struct, const char* _key)
	{
		assert(strlen(_key) == 8);

		// Step 1. �ṹ��ת�ֽ�����
		char* arr_original = StructToBytes<T>(_struct);

		if (!arr_original)
		{
			std::cerr << "StructToBytes failed in <EncryptStruct>.\n";
			return NULL;
		}

		// Step 2. �ֽ�������
		char* arr_encrypt = EncryptBytes(arr_original, _key, sizeof(T));
		delete[] arr_original;

		if (!arr_encrypt)
		{
			std::cerr << "Encrypt Failed.\n";
			return NULL;
		}
		else
			return arr_encrypt;
	}

	/// <summary>
	/// ��Byte������ܣ�Ȼ��ת��������T�Ľṹ�壬����Socket����
	/// </summary>
	/// <typeparam name="T">[out type] �ṹ������</typeparam>
	/// <param name="_arr">[in] ��ת���ɽṹ��T��Byte����</param>
	/// <param name="_key">[in] DES�����õ�64λ��Կ</param>
	/// <param name="_out">[out] �ṹ��T����</param>
	/// <returns>�ɹ� true������ false</returns>
	template<typename T>
	bool DecryptToStruct(char _arr[], const char* _key, T& _out)
	{
		assert(_arr != NULL);
		assert(strlen(_key) == 8);
		assert(sizeof(T) % 8 == 0);

		// Step 1. �ֽ�������
		char* arr_decrypt = DecryptBytes(_arr, _key, sizeof(T));

		if (!arr_decrypt)
		{
			std::cerr << "Decrypt Failed.\n";
			return false;
		}

		// Step 2. �ֽ����鵽�ṹ��
		bool ret = BytesToStruct<T>(arr_decrypt, _out);

		delete[] arr_decrypt;

		return ret;
	}
}
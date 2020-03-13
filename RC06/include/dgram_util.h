#pragma once

#include <iostream>
#include <sstream>
#include <assert.h>
#include <string.h>

#include "des.h"

using namespace std;


/* -----------------------------------------------------------------------------------------
 说明：
 用于处理结构体类型报文的加密（发送端）和解密（接收端）的工具集。

 Use Case:

	const char* key = "T&^9c=A`";

	DGram_RemoteControl obj(1);
	obj.m_unuse2 = 121; // 最后一个成员标记一个特殊值以供检查

	/// Step 1:
	/// 发送端 - 将结构体对象加密，得到字节数组
	char* arr_encrypt = DGramUtil::EncryptStruct<DGram_RemoteControl>(obj, key);


	/// Step 2:
	/// 接收端 - 将字节数组解密后，转换成结构体对象
	if (arr_encrypt)
	{
		/// Step 2.1 : 建立用于接收数据的结构体对象
		DGram_RemoteControl obj2(2);
		/// Step 2.2 : 关键步骤，调用解密并转换成结构体对象的函数
		bool result = DGramUtil::DecryptToStruct<DGram_RemoteControl>(arr_encrypt, key, obj2);

		// 验证 - 输出结构体最前端和最后端的字段进行验证
		std::cout << std::hex << obj2.m_gram_header << ", " << std::dec << obj2.m_unuse2 << std::endl;

		/// Step 2.3 : 这个资源需要手动释放！！！
		delete[] arr_encrypt;
	}
 ----------------------------------------------------------------------------------------- */

namespace DGramUtil
{
	/// <summary>
	/// 将结构体转换成Byte数组，用于Socket发送
	/// </summary>
	/// <typeparam name="T">[in type] 结构体类型</typeparam>
	/// <param name="_struct">[in] 结构体对象</param>
	/// <returns>转换后的Byte数组</returns>
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
	/// 将Byte数组转换成类型T的结构体，用于Socket接收
	/// </summary>
	/// <typeparam name="T">[out type] 结构体类型</typeparam>
	/// <param name="_arr">[in] 将转换成结构体T的Byte数组</param>
	/// <param name="_out">[out] 结构体T对象</param>
	/// <returns>成功 true，否则 false</returns>
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
	/// 发送数据时，将字节数组加密
	/// </summary>
	/// <param name="_arr">[in] 字节数组</param>
	/// <param name="_key">[in] DES加密用的64位秘钥</param>
	/// <param name="_size">[in] 字节数组长度</param>
	/// <returns>加密后的Byte数组</returns>
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
	/// 接收数据时，将字节数组解密
	/// </summary>
	/// <param name="_arr">[in] 字节数组</param>
	/// <param name="_key">[in] DES加密用的64位秘钥</param>
	/// <param name="_size">[in] 字节数组长度</param>
	/// <returns>解密后的Byte数组</returns>
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
	/// 将结构体转换成Byte数组，然后加密成字节数组，用于Socket发送
	/// </summary>
	/// <typeparam name="T">[in type] 结构体类型</typeparam>
	/// <param name="_struct">[in] 结构体对象</param>
	/// <param name="_key">[in] DES加密用的64位秘钥</param>
	/// <returns>加密后的Byte数组</returns>
	template<typename T>
	char* EncryptStruct(T _struct, const char* _key)
	{
		assert(strlen(_key) == 8);

		// Step 1. 结构体转字节数组
		char* arr_original = StructToBytes<T>(_struct);

		if (!arr_original)
		{
			std::cerr << "StructToBytes failed in <EncryptStruct>.\n";
			return NULL;
		}

		// Step 2. 字节流加密
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
	/// 将Byte数组解密，然后转换成类型T的结构体，用于Socket接收
	/// </summary>
	/// <typeparam name="T">[out type] 结构体类型</typeparam>
	/// <param name="_arr">[in] 将转换成结构体T的Byte数组</param>
	/// <param name="_key">[in] DES加密用的64位秘钥</param>
	/// <param name="_out">[out] 结构体T对象</param>
	/// <returns>成功 true，否则 false</returns>
	template<typename T>
	bool DecryptToStruct(char _arr[], const char* _key, T& _out)
	{
		assert(_arr != NULL);
		assert(strlen(_key) == 8);
		assert(sizeof(T) % 8 == 0);

		// Step 1. 字节流解密
		char* arr_decrypt = DecryptBytes(_arr, _key, sizeof(T));

		if (!arr_decrypt)
		{
			std::cerr << "Decrypt Failed.\n";
			return false;
		}

		// Step 2. 字节数组到结构体
		bool ret = BytesToStruct<T>(arr_decrypt, _out);

		delete[] arr_decrypt;

		return ret;
	}
}
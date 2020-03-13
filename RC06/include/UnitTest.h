#pragma once

#include <stdio.h>

#include "Carage.h"


void TestCarage()
{
	Carage carage;
	carage.AddCar(0x00A, "192.168.169.100", 1000, 2000);
	carage.AddCar(0x00B, "192.168.169.101", 1001, 2001);
	carage.AddCar(0x00C, "192.168.169.102", 1002, 2002);

	// Has Car Test:
	printf("Car Count: %d\n", carage.CarCount());
	printf("Has Car Test: Has 0x00A: %d, 0x00B: %d, 0x00C: %d\n",
		carage.HasCar(0x00A), carage.HasCar(0x00B), carage.HasCar(0x00C));

	// Remove Test:
	printf("Removed 0x00C.\n");
	carage.RemoveCar(0x00C);
	printf("Car Count: %d\n", carage.CarCount());
	printf("Has Car Test Has 0x00A: %d, 0x00B: %d, 0x00C: %d\n",
		carage.HasCar(0x00A), carage.HasCar(0x00B), carage.HasCar(0x00C));


	// Car ID List Test:
	std::vector<short> id_list = carage.CarID_List();
	for (int i = 0; i < id_list.size(); i++)
		printf("0x%03X, ", id_list[i]);
	printf("\n");

	printf("IP of CarA: %s\n", carage.IP_of_Car(0x00A).c_str());
}

struct DGram
{
	int m_data1;
	int m_data2;
};

class Foo
{
private:
	void* m_dgram;

public:
	Foo() {}
	~Foo() { delete m_dgram; m_dgram = 0; }

	template <typename T>
	void Bar(T _dgram)
	{
		m_dgram = new T(_dgram);
	}

	void Test()
	{
		printf("%d\n", ((DGram*)m_dgram)->m_data2);
	}
};
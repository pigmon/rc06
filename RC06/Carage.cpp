#include "Carage.h"

Carage* Carage::Instance = NULL;

Carage::~Carage()
{
	m_car_list.clear();
	m_id_list.clear();
}

Carage* Carage::GetInstance()
{
	if (!Instance)
	{
		delete Instance;
		Instance = new Carage();
	}

	return Instance;
}

void Carage::AddCar(const VehicleInfo& _car)
{
	short id = _car.ID;

	if (HasCar(id))
		return;

	m_car_list[id] = _car;

	CarID_List();
}

void Carage::AddCar(const short _id, const char* _ip, const int _port_send, const int _port_recv)
{
	if (HasCar(_id))
		return;

	VehicleInfo info(_id, "", 510, _ip, _port_send, _port_recv);
	m_car_list[_id] = info;

	CarID_List();
}

void Carage::AddCar(const short _id, const char* _name, const short _max_angle, const char* _ip, const int _port_send, const int _port_rcv)
{
	if (HasCar(_id))
		return;

	VehicleInfo info(_id, _name, _max_angle, _ip, _port_send, _port_rcv);
	m_car_list[_id] = info;

	CarID_List();
}

void Carage::RemoveCar(const short _id)
{
	std::map<short, VehicleInfo>::iterator it;
	it = m_car_list.find(_id);

	if (it != m_car_list.end())
		m_car_list.erase(it);

	CarID_List();
}

std::string Carage::IP_of_Car(const short _id) const
{
	std::string ret = "0.0.0.0";

	if (HasCar(_id))
		ret = m_car_list.at(_id).IP;

	return ret;
}

int Carage::PortSend_of_Car(const short _id) const
{
	int ret = 0;

	if (HasCar(_id))
		ret = m_car_list.at(_id).PORT_SEND;

	return ret;
}

int Carage::PortRecv_of_Car(const short _id) const
{
	int ret = 0;

	if (HasCar(_id))
		ret = m_car_list.at(_id).PORT_RECV;

	return ret;
}

short Carage::MaxAngle_of_Car(const short _id) const
{
	int ret = 510;

	if (HasCar(_id))
		ret = m_car_list.at(_id).PORT_RECV;

	return ret;
}

std::vector<short> Carage::CarID_List()
{
	m_id_list.clear();

	std::map<short, VehicleInfo>::iterator it;
	for (it = m_car_list.begin(); it != m_car_list.end(); it++)
		m_id_list.push_back(it->first);

	return m_id_list;
}

short Carage::CurrentID()
{	
	return m_current_id;
}

short Carage::RefetchCurrentID()
{
	if (m_id_list.empty())
	{
		printf("No Vechicle in Carage.\n");
		return -1;
	}

	if (m_current_id == 0)
		m_current_id = m_id_list[0];

	return m_current_id;
}

short Carage::NextID()
{
	if (m_id_list.empty())
	{
		printf("No Vechicle in Carage.\n");
		return -1;
	}

	short ret_id = m_current_id;

	if (m_current_id == 0)
	{
		if (!HasCar(m_id_list[0]))
		{
			CarID_List();
			m_current_id = m_id_list[0];
		}
			
		ret_id = m_id_list[0];
	}
	else
	{
		for (size_t i = 0; i < m_id_list.size(); i++)
		{
			if (m_id_list[i] == m_current_id)
			{
				if (i >= m_id_list.size() - 1)
					ret_id = m_id_list[0];
				else
					ret_id = m_id_list[i + 1];

				break;
			}
		}
	}

	m_current_id = ret_id;

	return ret_id;
}

short Carage::PrevID()
{
	if (m_id_list.empty())
	{
		printf("No Vechicle in Carage.\n");
		return -1;
	}

	short ret_id = m_current_id;

	if (m_current_id == 0)
	{
		if (!HasCar(m_id_list[0]))
		{
			CarID_List();
			m_current_id = m_id_list[0];
		}

		ret_id = m_id_list[0];
	}
	else
	{
		for (size_t i = 0; i < m_id_list.size(); i++)
		{
			if (m_id_list[i] == m_current_id)
			{
				if (i == 0)
					ret_id = m_id_list[m_id_list.size() - 1];
				else
					ret_id = m_id_list[i - 1];

				break;
			}
		}
	}

	m_current_id = ret_id;

	return ret_id;
}

bool Carage::SetCurrentID(short _id)
{
	bool ret = false;
	if (HasCar(_id)) {
		m_current_id = _id;
		ret = true;
	}
	else {
		printf("No Such Car ID in Carage : 0x%04x\n", _id);
	}
	return ret;
}

bool Carage::GetCurrentCarInfo(VehicleInfo& _out)
{
	if (m_car_list.empty())
		return false;

	if (!HasCar(m_current_id))
		RefetchCurrentID();

	_out = m_car_list[m_current_id];

	return HasCar(_out.ID);
}
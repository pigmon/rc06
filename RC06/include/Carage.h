#pragma once

#include <string>
#include <map>
#include <vector>

struct VehicleInfo
{
	short ID;
	std::string Name;
	short MaxAngle;
	std::string IP;
	int PORT_SEND;
	int PORT_RECV;

	VehicleInfo() {};

	VehicleInfo(
		const short _id,
		const char* _name,
		const short _max_angle,
		const char* _ip,
		const int _port_send,
		const int _port_recv
		)
		:
		ID(_id),
		Name(_name),
		MaxAngle(_max_angle),
		IP(_ip),
		PORT_SEND(_port_send),
		PORT_RECV(_port_recv)
	{

	}
};

class Carage
{
private:
	std::map<short, VehicleInfo> m_car_list;
	std::vector<short> m_id_list;
	short m_current_id;

	// 切换车辆时提示各个通信线程更新车辆信息
	bool m_need_update_controller;  // 车辆控制线程
	bool m_need_update_rcver;

public:
	Carage() { m_current_id = 0; }
	~Carage();

	void AddCar(const VehicleInfo& _car);
	void AddCar(const short _id, const char* _ip, const int _port_send, const int _port_recv);
	void AddCar(const short _id, const char* _name, const short _max_angle, const char* _ip, const int _port_send, const int _port_rcv);
	void RemoveCar(const short _id);

	bool HasCar(const short _id) const { return m_car_list.find(_id) != m_car_list.end(); }
	std::string IP_of_Car(const short _id) const;
	int PortSend_of_Car(const short _id) const;
	int PortRecv_of_Car(const short _id) const;
	short MaxAngle_of_Car(const short _id) const;

	int CarCount() const { return m_car_list.size(); }
	std::vector<short> CarID_List();

	bool SetCurrentID(short _id);
	short RefetchCurrentID();
	short CurrentID();
	short NextID();
	short PrevID();
	bool GetCurrentCarInfo(VehicleInfo& _out);
	void Reset() { m_current_id = 0; }

	// 更新通知
	bool IsNeedUpdateCtrl() const { return m_need_update_controller; }
	void SetNeedUpdateCtrl(bool _need) { m_need_update_controller = _need; }

	bool IsNeedUpdateRcver() const { return m_need_update_rcver; }
	void SetNeedUpdateRcver(bool _need) { m_need_update_rcver = _need; }

	static Carage* GetInstance();

private:
	static Carage* Instance;
};
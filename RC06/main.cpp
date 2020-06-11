#include <winsock2.h>
#include <pthread.h>
#include <math.h>

#pragma comment(lib, "ws2_32.lib")

#pragma comment(lib, "LogitechSteeringWheelLib.lib")
#include "G29WheelSync.h"

#include "Carage.h"
#include "SocketUnit.h"
#include "VehicleController.h"
#include "DGram.h"
#include "global.h"

HomeDgram_Ctrl homedgram;

void* Control(void* param)
{
	int *run = (int *)param;

	SocketSender sender;
	VehicleInfo info;
	DownlinkDGram downdgram;

	Carage::GetInstance()->GetCurrentCarInfo(info);
	sender.SetSocket(info.PORT_SEND, info.IP);

	while ((*run) & 0x0f)
	{

		bool quit = VehicleController::OnInput();
		if (quit)
			break;

		// For sync
		
		int max_angle = Carage::GetInstance()->MaxAngle_of_Car(Carage::GetInstance()->CurrentID());
		int counter = 0;
		int current_wheel = -65535;
		
		if (VehicleController::CtrlState == STATE_REMOTE_CTRL)
		{

			
			downdgram.m_gram_header = 0xAAA;
			downdgram.m_gram_id = 0xAF1;

			downdgram.m_vehicle_id = Carage::GetInstance()->CurrentID();
			downdgram.m_rc_speed = G29Sync::GetInstance()->TargetSpeedValue() * 100;
			downdgram.m_rc_acc_pedal = G29Sync::GetInstance()->AccPedalValue();
			downdgram.m_rc_brake_pedal = G29Sync::GetInstance()->BrkPedalValue();
			downdgram.m_rc_steer = G29Sync::GetInstance()->SteeringValue() * 100;
			downdgram.m_rc_gear = VehicleController::TargetGear;
			downdgram.m_rc_hopper = VehicleController::BasketState;
			downdgram.m_ctrl_sign = STATE_REMOTE_CTRL;
			downdgram.m_arrive_port = VehicleController::ArrivePort;
			printf("ArrivePort: %d", (int)downdgram.m_arrive_port);
			//
			homedgram.m_key_cmd = 7;

			// 切换车辆时更新info并重新建立socket
			if (Carage::GetInstance()->IsNeedUpdateCtrl())
			{
				sender.Close();
				Sleep(10);

				Carage::GetInstance()->GetCurrentCarInfo(info);
				sender.SetSocket(info.PORT_SEND, info.IP);
				Carage::GetInstance()->SetNeedUpdateCtrl(false);
			}

			int cnt = sender.MainLoop((const char*)&downdgram, sizeof(DownlinkDGram));
			if (cnt <= 0)
			{
				int ret_err = WSAGetLastError();
				printf("Remote Control Sender Error code is: %d\n", ret_err);
			}

			downdgram.Print();

			Sleep(20);
		}
		else if (VehicleController::CtrlState == STATE_SYNC)
		{
			if (!LogiUpdate())
				continue;

			float angle = VehicleController::TargetAngle;
			int target = (angle / max_angle) * (-100);
			current_wheel = G29Sync::GetInstance()->Sync(target);
			counter++;

			Sleep(10);

			if (G29Sync::GetInstance()->SyncDone(target) || counter > 10000)
			{
				//VehicleController::CtrlState = STATE_AUTO;
				printf("Sync Done!\n");
				continue;
			}
			else
			{
				printf("%d\n", current_wheel);
			}
		}
		else if (VehicleController::CtrlState == STATE_RELEASING)
		{
			downdgram.m_gram_header = 0xAAA;
			downdgram.m_gram_id = 0xAF1;

			downdgram.m_vehicle_id = Carage::GetInstance()->CurrentID();
			downdgram.m_rc_speed = 0;
			downdgram.m_rc_acc_pedal = 0;
			downdgram.m_rc_brake_pedal = 0;
			downdgram.m_rc_steer = 0;
			downdgram.m_rc_gear = GEAR_N;
			downdgram.m_rc_hopper = BASKET_NONE;
			downdgram.m_ctrl_sign = STATE_AUTO;
			VehicleController::ArrivePort = downdgram.m_arrive_port = 0;

			//homedgram.m_key_cmd = 7;

			int cnt = sender.MainLoop((const char*)&downdgram, sizeof(DownlinkDGram));
			if (cnt <= 0)
			{
				int ret_err = WSAGetLastError();
				printf("Remote Control Sender Error code is: %d\n", ret_err);
			}

			Sleep(80);
		}
		else // AUTO or NOTHING
		{
			homedgram.m_key_cmd = 8;
		}
	}
	
	sender.Close();
	WSACleanup();

	printf("Sender Thread Closing.\n");
	pthread_exit(0);
	

	return 0;
}

void* recv_func(void* param)
{
	
	int *run = (int *)param;
	SocketRecv_Vehicle recver;
	SocketSender sender_ui;
	VehicleInfo info;

	Carage::GetInstance()->GetCurrentCarInfo(info);
	recver.SetSocket(sizeof(UplinkDGram), info.PORT_RECV, info.IP);

	while ((*run) & 0x0f)
	{
		bool quit = VehicleController::CheckQuit();
		if (quit)
			break;

		// 切换车辆时更新info并重新建立socket
		if (Carage::GetInstance()->IsNeedUpdateRcver())
		{
			recver.Close();





			Sleep(10);

			Carage::GetInstance()->GetCurrentCarInfo(info);
			recver.SetSocket(sizeof(UplinkDGram), info.PORT_RECV, info.IP);
			Carage::GetInstance()->SetNeedUpdateRcver(false);
		}

		//VehicleController::Tongli_SpeedGuard = false;

		int n = recver.MainLoop();
		printf("%d bytes from vehicle.\n", n);
		//Sleep(100);

		if (n > 0)
		{
			/*
			int ui_cnt = sender_ui.MainLoop((const char*)(&(recver.m_up_dgram)), sizeof(UplinkDGram));
			//printf("UI Sender Bytes: %d\n", ui_cnt);
			if (ui_cnt <= 0)
			{
				int ret_err = WSAGetLastError();
				printf("UI Sender Error code is: %d\n", ret_err);
			}
			Sleep(10);
			*/
	
			VehicleController::Tongli_SpeedGuard = recver.m_up_dgram.m_speed > 10 || recver.m_up_dgram.m_speed < -10;
			homedgram.m_vehicle_speed = recver.m_up_dgram.m_speed;
			homedgram.m_vehicle_steering = recver.m_up_dgram.m_steering;
			homedgram.m_vehicle_gear = recver.m_up_dgram.m_gear;
			homedgram.m_hopper_state = recver.m_up_dgram.m_hopper_angle;
			homedgram.m_cmd = (VehicleController::CtrlState == STATE_REMOTE_CTRL) ? 0 : recver.m_up_dgram.m_ctrl_info;
			homedgram.m_lat_error = recver.m_up_dgram.m_lat_error;
			homedgram.m_lon_error = recver.m_up_dgram.m_lon_error;
			homedgram.m_angle_error = recver.m_up_dgram.m_angle_error;
			homedgram.m_path_x = recver.m_up_dgram.m_path_x;
			homedgram.m_path_y = recver.m_up_dgram.m_path_y;
			homedgram.m_path_heading = recver.m_up_dgram.m_path_heading;
			homedgram.m_path_id = recver.m_up_dgram.m_path_id;
			homedgram.m_basket_filled = recver.m_up_dgram.m_basket_filled;

			if (recver.m_up_dgram.m_release_feedback == 1 && VehicleController::CtrlState == STATE_RELEASING)
			{
				VehicleController::CtrlState = STATE_NOTHING;
			}
			
				
			//printf("Rcv <Steering, Speed, Gear, SpeedGuard, Ctrl State>: <%d, %d, %d, %d, %d>\n", homedgram.m_vehicle_steering, recver.m_up_dgram.m_speed, homedgram.m_vehicle_gear, VehicleController::Tongli_SpeedGuard, VehicleController::CtrlState);
			//printf("Ctrl State: %d\n", VehicleController::CtrlState);
			//printf("Errors: <%d, %d, %d>\n",recver.m_up_dgram.m_lat_error, recver.m_up_dgram.m_lon_error, recver.m_up_dgram.m_angle_error);
		}
	}

	recver.Close();
	Sleep(10);
	sender_ui.Close();
	Sleep(10);

	WSACleanup();

	printf("Recver Thread Closing.\n");
	pthread_exit(0);

	return 0;
}

void* local_send(void* param)
{
	int *run = (int *)param;

	SocketSender sender;
	sender.SetSocket(UI_PORT, UI_IP);

	while ((*run) & 0x0f)
	{
		//VehicleController::OnInput();

		if (!LogiUpdate() || !LogiIsConnected(0))
			continue;
		
		homedgram.Reset();
		
		homedgram.m_vehicle_id = Carage::GetInstance()->CurrentID();
		homedgram.m_rc_steering = (int)(-1 * G29Sync::GetInstance()->SteeringValue() * 100);
		homedgram.m_rc_speed = (short)(G29Sync::GetInstance()->TargetSpeedValue() * 100);
		homedgram.m_rc_acc_pedal = G29Sync::GetInstance()->AccPedalValue();
		homedgram.m_rc_brk_pedal = G29Sync::GetInstance()->BrkPedalValue();
		homedgram.m_rc_gear = VehicleController::TargetGear;
		//homedgram.m_hopper_state = VehicleController::BasketState;

		int cnt = sender.MainLoop((const char*)&homedgram, sizeof(homedgram), false);
		if (cnt <= 0)
		{
			int ret_err = WSAGetLastError();
			printf("Local Sender Error code is: %d\n", ret_err);
		}

		Sleep(10);
	}

	sender.Close();
	WSACleanup();

	printf("Local Sender Thread Closing.\n");
	pthread_exit(0);


	return 0;
}

void MakeTestData();

int main()
{
	MakeTestData();
	
	WSADATA wsaData;
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
	{
		printf("WSAStartup failed\n");
		return 1;
	}

	//if (!LogiIsConnected(0)) {
	//	printf("No Logitech Steering Wheel Connected.\n");
	//	return 1;
	//}
		

	bool g29 = LogiSteeringInitialize(true);
	Sleep(100);

	if (!g29) {
		printf("Init G29 Failed.\n");
		WSACleanup();
		return 1;
	}
	else
		printf("G29 is ready!\n");
	
	int run1 = 1;
	pthread_t socket_send;
	int ret1 = pthread_create(&socket_send, NULL, Control, &run1);
	
	int run2 = 1;
	pthread_t socket_recv;
	int ret2 = pthread_create(&socket_recv, NULL, recv_func, &run2);
	
	int run3 = 1;
	pthread_t socket_local;
	int ret3 = pthread_create(&socket_local, NULL, local_send, &run3);
	
	while (1)
	{
		if (!LogiUpdate() || !LogiIsConnected(0))
			continue;

		if (LogiGetState(0)->rgbButtons[3] == 128)
			break;

		Sleep(10);
	}

	run1 = 0;
	run2 = 0;
	run3 = 0;
	pthread_join(socket_send, NULL);
	pthread_join(socket_recv, NULL);
	pthread_join(socket_local, NULL);

	WSACleanup();

	G29Sync::GetInstance()->PostSync();

	return 0;
}

void MakeTestData()
{
	//Carage::GetInstance()->AddCar(0x00A, "10.0.2.100", 9111, 9111);
	//Carage::GetInstance()->AddCar(0x00B, "10.0.1.100", 9112, 9112);
	//Carage::GetInstance()->AddCar(0x00C, "10.0.4.100", 9113, 9113);
	//Carage::GetInstance()->AddCar(0x00D, "10.0.3.100", 9114, 9114);

	Carage::GetInstance()->AddCar(0x00A, "10.0.2.100", 9117, 9117);
	Carage::GetInstance()->AddCar(0x00B, "10.0.4.100", 9113, 9113);
	Carage::GetInstance()->AddCar(0x00C, "10.0.1.100", 9110, 9110);
	Carage::GetInstance()->AddCar(0x00D, "10.0.3.100", 9112, 9112);
}
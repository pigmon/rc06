#pragma once

class VehicleController
{
private:
	static short WatchCar(short _id);
public:
	//static bool bControlling;
	static int CtrlState; // 2 - ң�أ�3 - ͬ����0 - nothing; 1 - �Զ�
	static int TargetAngle;
	static short TargetGear;
	static bool StateUpdate;
	static bool Tongli_SpeedGuard;  // true - Vehicle Speed is not 0, U shall not pass;
	static short BasketState;
	static bool ArrivePort;

	static bool SyncCurrentCar();
	static short WatchPrevCar();
	static short WatchNextCar();
	static bool ControlCurrentCar();
	static void ReleaseControl();

	static bool OnInput();
	static bool CheckQuit();
};

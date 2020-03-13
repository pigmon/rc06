
#include "VehicleController.h"
#include "Carage.h"
#include "G29WheelSync.h"
#include "DGram.h"

int VehicleController::CtrlState = STATE_AUTO;
int VehicleController::TargetAngle = 0;
short VehicleController::TargetGear = 0;
bool VehicleController::StateUpdate = false;
bool VehicleController::Tongli_SpeedGuard = false;
short VehicleController::BasketState = BASKET_NONE;
bool VehicleController::ArrivePort = false;

short VehicleController::WatchCar(short _id)
{
	Carage::GetInstance()->SetCurrentID(_id);
	Carage::GetInstance()->SetNeedUpdateCtrl(true);
	Carage::GetInstance()->SetNeedUpdateRcver(true);

	return Carage::GetInstance()->CurrentID();
}

short VehicleController::WatchPrevCar()
{
	if (CtrlState == STATE_REMOTE_CTRL)
	{
		printf("Please Release the Controlling before Switching.");
		return -2;
	}

	short id = Carage::GetInstance()->PrevID();
	if (!Carage::GetInstance()->HasCar(id))
	{
		printf("No Such CarID in Carage: 0x%04x\n", id);
		return -1;
	}

	WatchCar(id);

	TargetGear = GEAR_N;

	Sleep(200);

	return id;
}

short VehicleController::WatchNextCar()
{
	if (CtrlState == STATE_REMOTE_CTRL)
	{
		printf("Please Release the Controlling before Switching.");
		return -2;
	}

	short id = Carage::GetInstance()->NextID();
	if (!Carage::GetInstance()->HasCar(id))
	{
		printf("No Such CarID in Carage: 0x%04x\n", id);
		return -1;
	}

	WatchCar(id);

	TargetGear = GEAR_N;

	Sleep(200);

	return id;
}

bool VehicleController::SyncCurrentCar()
{
	short id = Carage::GetInstance()->RefetchCurrentID();

	if (!Carage::GetInstance()->HasCar(id))
	{
		printf("No Such CarID in Carage: 0x%04x\n", id);
		return false;
	}

	CtrlState = STATE_SYNC;
	return true;
}

bool VehicleController::ControlCurrentCar()
{
	short id = Carage::GetInstance()->RefetchCurrentID();

	if (!Carage::GetInstance()->HasCar(id))
	{
		printf("No Such CarID in Carage: 0x%04x\n", id);
		return false;
	}

	CtrlState = STATE_REMOTE_CTRL;
	return true;
}

void VehicleController::ReleaseControl()
{
	CtrlState = STATE_RELEASING;
}

bool VehicleController::OnInput()
{
	if (!LogiUpdate() || !LogiIsConnected(0))
		return false;

	BasketState = BASKET_NONE;
	unsigned int clutch_value = G29Sync::GetInstance()->BrkPedalValue();

	if (LogiGetState(0)->rgbButtons[5] == 128) // Left slide switch prev vehicle
	{
		if (clutch_value < 40)
		{
			short id = WatchPrevCar();
			printf("Switch To 0x%03X\n", id);
		}
		else
		{
			if (Tongli_SpeedGuard)
			{
				printf("Only Switch Gear When the Truck's Stopped.\n");
			}
			else
			{
				TargetGear -= 1;
				if (TargetGear < GEAR_R)
					TargetGear = GEAR_R;

				printf("Gear Down to %d.\n", TargetGear);
				
			}
			
		}
		StateUpdate = true;
		Sleep(500);
	}	
	else if (LogiGetState(0)->rgbButtons[4] == 128) // Right slide switch next vehicle
	{
		if (clutch_value < 40)
		{
			short id = WatchNextCar();
			printf("Switch To 0x%03X\n", id);
		}
		else
		{
			if (Tongli_SpeedGuard)
			{
				printf("Only Switch Gear When the Truck's Stopped.\n");
			}
			else
			{
				TargetGear += 1;
				if (TargetGear > GEAR_D)
					TargetGear = GEAR_D;

				printf("Gear Up to %d.\n", TargetGear);
				
			}
			
		}
		StateUpdate = true;
		Sleep(500);
	}		
	else if (LogiGetState(0)->rgbButtons[10] == 128)  // R3 Sync
	{
		bool ret = SyncCurrentCar();
		if (ret) {
			StateUpdate = true;
			printf("Syncing 0x%03X's Steering Wheel.\n", Carage::GetInstance()->CurrentID());
		}
			
	}
	else if (LogiGetState(0)->rgbButtons[23] == 128) // Enter Takingover
	{
		G29Sync::GetInstance()->PostSync();
		bool ret = ControlCurrentCar();
		if (ret) {
			StateUpdate = true;
			printf("Takingover of 0x%03X\n", Carage::GetInstance()->CurrentID());
		}
			
	}
	else if (LogiGetState(0)->rgbButtons[11] == 128) // L3 Release
	{
		if (Tongli_SpeedGuard)
		{
			printf("Only Release Control When the Truck's Stopped.\n");
		}
		else
		{
			G29Sync::GetInstance()->PostSync();
			ReleaseControl();
			StateUpdate = true;
			printf("Released Control of 0x%03X\n", Carage::GetInstance()->CurrentID());
		}

	}
	else if (LogiGetState(0)->rgbButtons[24] == 128) // Logo Reset
	{
		CtrlState = STATE_AUTO;
		StateUpdate = true;
		Carage::GetInstance()->Reset();
		G29Sync::GetInstance()->PostSync();
	} 
	else if (LogiGetState(0)->rgbButtons[19] == 128) // + Up the basket
	{
		BasketState = BASKET_UP;
		StateUpdate = true;
		Sleep(30);
	}
	else if (LogiGetState(0)->rgbButtons[20] == 128) // - Down the basketc
	{
		BasketState = BASKET_DOWN;
		StateUpdate = true;
		Sleep(30);
	}
	else if (LogiGetState(0)->rgbButtons[8] == 128) // Share buuton to Send Arrive port
	{
		ArrivePort = !ArrivePort;
		Sleep(500);
	}
	else if (LogiGetState(0)->rgbButtons[3] == 128) // trangle quit
	{
		return true;
	}

	Sleep(20);

	return false;
}

bool VehicleController::CheckQuit()
{
	if (!LogiUpdate() || !LogiIsConnected(0))
		return false;

	unsigned int clutch_value = G29Sync::GetInstance()->ClutchPedalValue();

	if (LogiGetState(0)->rgbButtons[10] == 128)  // R3 Sync
	{
		bool ret = SyncCurrentCar();
		if (ret) {
			StateUpdate = true;
			printf("Syncing 0x%03X's Steering Wheel.\n", Carage::GetInstance()->CurrentID());
		}

	}
	else if (LogiGetState(0)->rgbButtons[23] == 128) // Enter Takingover
	{
		G29Sync::GetInstance()->PostSync();
		bool ret = ControlCurrentCar();
		if (ret) {
			StateUpdate = true;
			printf("Takingover of 0x%03X\n", Carage::GetInstance()->CurrentID());
		}

	}
	else if (LogiGetState(0)->rgbButtons[11] == 128) // L3 Release
	{
		if (Tongli_SpeedGuard)
		{
			printf("Only Release Control When the Truck's Stopped.\n");
		}
		else
		{
			G29Sync::GetInstance()->PostSync();
			ReleaseControl();
			StateUpdate = true;
			printf("Released Control of 0x%03X\n", Carage::GetInstance()->CurrentID());
		}

	}
	else if (LogiGetState(0)->rgbButtons[24] == 128) // Logo Reset
	{
		CtrlState = STATE_AUTO;
		StateUpdate = true;
		Carage::GetInstance()->Reset();
		G29Sync::GetInstance()->PostSync();
	}
	else if (LogiGetState(0)->rgbButtons[3] == 128) // trangle quit
	{
		return true;
	}

	Sleep(20);

	return false;
}
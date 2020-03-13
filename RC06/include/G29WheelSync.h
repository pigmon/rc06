#pragma once

#include <string>

#include "LogitechSteeringWheelLib.h"

class G29Sync
{
public:
	G29Sync();
	bool IsInited();

	int Sync(const int _target);
	void PostSync() const;
	bool SyncDone(int _target, int _eps = 3);
	DIJOYSTATE2* CurrentState();
	int CurrentWheelPct();
	int BrakePct() const;
	int AccPct() const;
	int ClutchPct() const;

	float SteeringValue();
	float TargetSpeedValue();
	unsigned int AccPedalValue();
	unsigned int BrkPedalValue();
	unsigned int ClutchPedalValue();

	static unsigned int PedalPos(double _value) { return (unsigned int)(((32767 - _value) / 65536) * 100); }
	static void ErrMsg(const std::string& _msg);
	static int Clamp(int _val, const int _min, const int _max);
	static G29Sync* GetInstance();
	static int WheelAngleToPos(int _angle);
	

private:
	bool m_b_init;
	DIJOYSTATE2* m_state;

	static G29Sync* Instance;
};
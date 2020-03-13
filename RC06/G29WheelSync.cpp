#include <iostream>
#include <windows.h>

#include "G29WheelSync.h"

#define DEBUG_G29

#ifdef DEBUG_G29
#include <iostream>
#endif

// Singleton
G29Sync* G29Sync::Instance = NULL;

G29Sync::G29Sync()
{
	bool init = LogiSteeringInitialize(false);
	//if (!init)
	//{
	//	G29Sync::ErrMsg("G29Sync Init Failed.");
	//	return;
	//}

	m_state = LogiGetState(0);
	m_b_init = m_state != NULL;
}

// Singleton
G29Sync* G29Sync::GetInstance()
{
	if (!Instance || !Instance->IsInited())
	{
		delete Instance;
		Instance = new G29Sync();
	}
		
	return Instance;
}

/*
 @Func:
	IsInited
 @Summary:
	是否经过初始化且获取过 state
 @Params:
 @Return:
	Bool
*/
bool G29Sync::IsInited()
{
	return m_state != NULL;
}

/*
 @Func: 
	Sync
 @Summary: 
	将方向盘转角同步到输入的目标转向比例。（通过设置方向盘回弹位置）
	每帧调用，如在 Timer, Update, Tick 等函数中调用。单次调用无效
 @Params:
	_target:  目标转向比例，取值 [-100, 100]. 100: 右转满; -100: 左转满
 @Return:
	当前方向盘转向，取值 [-100, 100]. 100: 右转满; -100: 左转满
*/
int G29Sync::Sync(const int _target)
{
	if (!m_b_init)
	{
		G29Sync::ErrMsg("G29Sync Hasn't been Inited.");
		return -1024;
	}

	m_state = LogiGetState(0);
	if (!m_state)
	{
		G29Sync::ErrMsg("State Failed in G29Sync::Sync.");
		return -2048;
	}
	
	if (!LogiUpdate())
		return m_state->lX;

	int target = G29Sync::Clamp(_target, -99, 99);
	LogiPlaySpringForce(0, target, 80, 80);
	
	return G29Sync::WheelAngleToPos(m_state->lX);
}

/*
 @Func:
	PostSync
 @Summary:
	强制结束同步，将方向盘回弹位置回零
 @Params:
 @Return:
*/
void G29Sync::PostSync() const
{
	if (!m_b_init)
	{
		G29Sync::ErrMsg("G29Sync Hasn't been Inited.");
		return;
	}

	int safe_counter = 0;
	while (!LogiUpdate() && safe_counter < 10000)
		safe_counter++;

	LogiPlaySpringForce(0, 0, 30, 30);
}

/*
 @Func:
	SyncDone
 @Summary:
	 判断同步是否完成
 @Params:
	_target: 同步目标
	_eps：判断2个角度相等的阈值，默认3
 @Return:
	True: 完成; False：未完成
*/
bool G29Sync::SyncDone(int _target, int _eps)
{
	m_state = LogiGetState(0);
	if (!m_state)
	{
		G29Sync::ErrMsg("State Failed in G29Sync::Sync.");
		return false;
	}

	return std::abs(G29Sync::WheelAngleToPos(m_state->lX) - _target) <= _eps;
}

/*
 @Func:
	CurrentState
 @Summary:
	更新 m_state ; 在 LogiUpdate() 后调用
 @Params:
 @Return:
	m_state : DIJOYSTATE2*
*/
DIJOYSTATE2* G29Sync::CurrentState()
{
	m_state = LogiGetState(0);
	if (!m_state)
	{
		G29Sync::ErrMsg("State Failed in G29Sync::CurrentWheelPct.");
		return NULL;
	}

	return m_state;
}

/*
 @Func:
	CurrentWheelPct
 @Summary:
	得到当前方向盘转角。取值 [-100, 100]. 100: 右转满; -100: 左转满
 @Params:
 @Return:
	转角：int  [-100, 100]
*/
int G29Sync::CurrentWheelPct()
{
	return G29Sync::WheelAngleToPos(m_state->lX);
}

/*
 @Func:
	WheelAngleToPos
 @Summary:
	将方向盘[-32767, 32767] 转角 归一化到 [-100, 100]
 @Params:
	_angle: int ，state.lx
 @Return:
	转角：int  [-100, 100]
*/
int G29Sync::WheelAngleToPos(int _angle)
{
	double ret = G29Sync::Clamp(_angle, -32767, 32767);
	return (int)((ret / 32767) * 100);
}

void G29Sync::ErrMsg(const std::string& _msg)
{
#ifdef DEBUG_G29
	std::cout << _msg.c_str() << std::endl;
#endif
}

int G29Sync::Clamp(int _val, const int _min, const int _max)
{
	int ret = _val;
	if (_val < _min) ret = _min;
	else if (_val > _max) ret = _max;

	return ret;
}

int G29Sync::BrakePct() const
{
	if (!LogiUpdate() || !LogiGetState(0))
		return 0;

	return G29Sync::PedalPos(LogiGetState(0)->lRz);
}

int G29Sync::AccPct() const
{
	if (!LogiUpdate() || !LogiGetState(0))
		return 0;

	return G29Sync::PedalPos(LogiGetState(0)->lY);
}

int G29Sync::ClutchPct() const
{
	if (!LogiUpdate() || !LogiGetState(0))
		return 0;

	return G29Sync::PedalPos(LogiGetState(0)->rglSlider[0]);
}

float G29Sync::SteeringValue()
{
	if (!LogiUpdate() || !LogiGetState(0))
		return 0;

	return -450 * float(LogiGetState(0)->lX) / 32767;
}

float G29Sync::TargetSpeedValue()
{
	if (!LogiUpdate() || !LogiGetState(0))
		return 0;

	return -20 * float(LogiGetState(0)->lY) / 32767 + 20;
}

unsigned int G29Sync::AccPedalValue()
{
	if (!LogiUpdate() || !LogiGetState(0))
		return 0;

	return PedalPos(LogiGetState(0)->lY);
}

unsigned int G29Sync::BrkPedalValue()
{
	if (!LogiUpdate() || !LogiGetState(0))
		return 0;

	return PedalPos(LogiGetState(0)->lRz);
}

unsigned int G29Sync::ClutchPedalValue()
{
	if (!LogiUpdate() || !LogiGetState(0))
		return 0;

	return PedalPos(LogiGetState(0)->rglSlider[0]);
}


#pragma once

enum CTRL_STATE
{
	STATE_NOTHING,
	STATE_AUTO,
	STATE_REMOTE_CTRL,
	STATE_SYNC,
	STATE_RELEASING
};

enum GEAR
{
	GEAR_R = -1,
	GEAR_N = 0,
	GEAR_D = 1
};

enum BASKET_STATE
{
	BASKET_DOWN = 0,
	BASKET_UP = 1,
	BASKET_NONE = 0x11
};



// 车辆上行信息，接收到就发给UI和PanoSim
struct UplinkDGram
{
	short m_gram_header;      // 报文头 0xAAA
	short m_gram_id;          // 报文标识 0xAF1
	short m_vehicle_id;       // 车辆ID 0x00A ~ 0x00C
	short m_speed;            // 速度
	int m_steering;           // 方向盘转角
	short m_gear;	          // 挡位
	short m_acc_pedal;
	short m_brake_pedal;
	short m_rpm;              // RPM 
	short m_hopper_state;
	short m_hopper_angle;
	short m_vehicle_state;    // 0:正常; 1:故障
	//unsigned int m_longitude; // 经度
	short m_ctrl_sign;        // 1:自动; 2:遥控
	short m_ctrl_info;        // 控制信息 1:车辆主动请求接管; 2:车辆故障 ...
	short m_release_feedback;          // 
	int m_lat_error;
	int m_lon_error;
	int m_angle_error;
	int m_path_x;
	int m_path_y;
	short m_path_heading;
	short m_path_id;
	short m_basket_filled;
	short unused0;
	short unused1;
	short unused2;


	void Print()
	{
		printf("DGram CarID: %d\n", m_vehicle_id);
		printf("DGram m_speed: %d\n", m_speed);
		printf("DGram m_steering: %d\n", m_steering);
	}
};


struct DownlinkDGram
{
	short m_gram_header;      // 报文头 0xAAA
	short m_gram_id;          // 报文标识 0xAF1  
	short m_vehicle_id;       // 车辆ID 0x00A ~ 0x00C
	short m_rc_acc_pedal;
	short m_rc_brake_pedal;
	short m_rc_hopper;        // 遥控斗状态，0下降，1上升，2无动作
	short m_rc_gear;          // 遥控档位 R:-1, N:0, D:1
	short m_rc_speed;         // 遥控速度
	short m_ctrl_sign;        // 遥控状态, 1: 遥控; 0: 自动
	short m_unused;
	int m_rc_steer;           // 遥控方向盘转角
	short m_arrive_port;	  // 到达装载点信号
	short m_unuse0;
	short m_unuse1;
	short m_unuse2;

	void Print()
	{
		printf("VID: 0x%03x - RCGear: %d - RCSteering: %d, Pedal: <%d, %d>, Basket: %d\n", m_vehicle_id, m_rc_gear, m_rc_steer, m_rc_acc_pedal, m_rc_brake_pedal, m_rc_hopper);
	}
};

#pragma pack(push)
#pragma pack(1)

// 模拟器状态 报文，一直发送
typedef struct HomeDgram_Ctrl
{
	short m_gram_header;			// 报文头 0xA00
	short m_gram_id;				// 报文标识 0xA01  
	short m_vehicle_id;				// 0 - 当前没有观察任何车辆；或 车辆ID
	short m_state;
	unsigned int m_longitude = 0; // 经度
	unsigned int m_latitude = 0;  // 纬度
	unsigned short m_heading = 0; // 航向
	short m_vehicle_speed = 0;
	int m_vehicle_steering = 0;
	short m_vehicle_gear;
	short m_rc_gear;
	int m_rc_steering;				// 模拟方向盘转角
	short m_rc_speed;				// 模拟器速度
	short m_cmd;//1 请求接管
	short m_key_cmd;//2 3 4 同步 5切入遥控 6切回自动
	short m_rc_acc_pedal;			// 模拟器油门踏板开度 (0-100)
	short m_rc_brk_pedal;			// 模拟器刹车踏板开度 (0-100)
	short m_rotate;
	short m_ping_time;				// RTT
	short m_hopper_state;
	int m_lat_error;
	int m_lon_error;
	int m_angle_error;
	int m_path_x;
	int m_path_y;
	short m_path_heading;
	short m_path_id;
	short m_basket_filled;
	short m_unused0;

	HomeDgram_Ctrl()
	{
		m_gram_header = 0xA00;
		m_gram_id = 0xA01;
		m_ping_time = 5;
	}

	void Reset()
	{
		m_gram_header = 0xA00;
		m_gram_id = 0xA01;
		m_ping_time = 5;

		m_longitude = 0;
		m_latitude = 0;
		m_heading = 0;
	}
};
// 控制更改报文，有按键消息再发送
typedef struct HomeDgram_Trigger
{
	short m_gram_header;			// 报文头 0x010
	short m_gram_id;				// 报文标识 0x011
	short m_ctrl_state;				// 控制状态 0 - 啥也不做；1 - 遥控； 2 - 同步

	HomeDgram_Trigger()
	{
		m_gram_header = 0x010;
		m_gram_id = 0x011;
	}
};
#pragma pack(pop)
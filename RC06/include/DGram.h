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



// ����������Ϣ�����յ��ͷ���UI��PanoSim
struct UplinkDGram
{
	short m_gram_header;      // ����ͷ 0xAAA
	short m_gram_id;          // ���ı�ʶ 0xAF1
	short m_vehicle_id;       // ����ID 0x00A ~ 0x00C
	short m_speed;            // �ٶ�
	int m_steering;           // ������ת��
	short m_gear;	          // ��λ
	short m_acc_pedal;
	short m_brake_pedal;
	short m_rpm;              // RPM 
	short m_hopper_state;
	short m_hopper_angle;
	short m_vehicle_state;    // 0:����; 1:����
	//unsigned int m_longitude; // ����
	short m_ctrl_sign;        // 1:�Զ�; 2:ң��
	short m_ctrl_info;        // ������Ϣ 1:������������ӹ�; 2:�������� ...
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
	short m_gram_header;      // ����ͷ 0xAAA
	short m_gram_id;          // ���ı�ʶ 0xAF1  
	short m_vehicle_id;       // ����ID 0x00A ~ 0x00C
	short m_rc_acc_pedal;
	short m_rc_brake_pedal;
	short m_rc_hopper;        // ң�ض�״̬��0�½���1������2�޶���
	short m_rc_gear;          // ң�ص�λ R:-1, N:0, D:1
	short m_rc_speed;         // ң���ٶ�
	short m_ctrl_sign;        // ң��״̬, 1: ң��; 0: �Զ�
	short m_unused;
	int m_rc_steer;           // ң�ط�����ת��
	short m_arrive_port;	  // ����װ�ص��ź�
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

// ģ����״̬ ���ģ�һֱ����
typedef struct HomeDgram_Ctrl
{
	short m_gram_header;			// ����ͷ 0xA00
	short m_gram_id;				// ���ı�ʶ 0xA01  
	short m_vehicle_id;				// 0 - ��ǰû�й۲��κγ������� ����ID
	short m_state;
	unsigned int m_longitude = 0; // ����
	unsigned int m_latitude = 0;  // γ��
	unsigned short m_heading = 0; // ����
	short m_vehicle_speed = 0;
	int m_vehicle_steering = 0;
	short m_vehicle_gear;
	short m_rc_gear;
	int m_rc_steering;				// ģ�ⷽ����ת��
	short m_rc_speed;				// ģ�����ٶ�
	short m_cmd;//1 ����ӹ�
	short m_key_cmd;//2 3 4 ͬ�� 5����ң�� 6�л��Զ�
	short m_rc_acc_pedal;			// ģ��������̤�忪�� (0-100)
	short m_rc_brk_pedal;			// ģ����ɲ��̤�忪�� (0-100)
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
// ���Ƹ��ı��ģ��а�����Ϣ�ٷ���
typedef struct HomeDgram_Trigger
{
	short m_gram_header;			// ����ͷ 0x010
	short m_gram_id;				// ���ı�ʶ 0x011
	short m_ctrl_state;				// ����״̬ 0 - ɶҲ������1 - ң�أ� 2 - ͬ��

	HomeDgram_Trigger()
	{
		m_gram_header = 0x010;
		m_gram_id = 0x011;
	}
};
#pragma pack(pop)
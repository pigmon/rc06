#pragma once

const char* UI_IP = "127.0.0.1";
const int UI_PORT = 12345;	   // 对应 UplinkDGram

const char* PANOSIM_IP = "10.130.210.63";
const int PANOSIM_PORT = 10012; // 对应 UplinkDGram


const int STATE_PORT = 10013;  // 按键事件触发报文的端口，对应 HomeDgram_Trigger
const int LOCAL_PORT = 10015;  // 模拟器输入实时状态的发送端口 HomeDgram_Ctrl

#ifndef PAYLOADSDK_H_
#define PAYLOADSDK_H_

// mavlink communication
#include <common/mavlink.h>
#include <signal.h>
#include "autopilot_interface.h"
#include "serial_port.h"
#include "udp_port.h"

#if defined GHADRON
#include "ghadron_sdk.h"
#elif defined VIO
#include "vio_sdk.h"
#elif defined ZIO
#include "zio_sdk.h"
#endif

#define PAYLOAD_SYSTEM_ID 1
#define PAYLOAD_COMPONENT_ID MAV_COMP_ID_CAMERA2 // <- Fix me with your MAVLINK Camera Component ID on Web setting (http://192.168.***.***:8000/setup-systems)

#define GIMBAL_SYSTEM_ID 1
#define GIMBAL_COMPONENT_ID MAV_COMP_ID_GIMBAL

#define CONTROL_UART    0
#define CONTROL_UDP     1
#define CONTROL_METHOD CONTROL_UDP

static char *payload_uart_port = (char*)"/dev/ttyUSB0";
static int payload_uart_baud = 115200;
//static char *udp_ip_target = (char*)"192.168.12.250";   // This is an ip address of the payload
static char *udp_ip_target = (char*)"10.4.1.13";         // This is for GREMSY-01
static int udp_port_target = 14566;                     // Do not change

typedef struct{
    char* name;
    int baudrate;
} T_ConnInfo_Uart;

typedef struct{
    char* ip;
    int port;
} T_ConnInfo_UDP;

typedef struct {
    uint8_t type;
    union {
        T_ConnInfo_Uart uart;
        T_ConnInfo_UDP  udp;
    } device;
} T_ConnInfo;

#define CAM_PARAM_ID_LEN 16
#define CAM_PARAM_VALUE_LEN 128
typedef struct {
    union {
        float param_float;
        int32_t param_int32;
        uint32_t param_uint32;
        int16_t param_int16;
        uint16_t param_uint16;
        int8_t param_int8;
        uint8_t param_uint8;
        char bytes[CAM_PARAM_VALUE_LEN];
    };
    uint8_t type;
} cam_param_union_t;

enum param_type {
    PARAM_TYPE_UINT8 = 1,
    PARAM_TYPE_INT8,
    PARAM_TYPE_UINT16,
    PARAM_TYPE_INT16,
    PARAM_TYPE_UINT32,
    PARAM_TYPE_INT32,
    PARAM_TYPE_UINT64,
    PARAM_TYPE_INT64,
    PARAM_TYPE_REAL32,
    PARAM_TYPE_REAL64
};


// ------------------------------------------------------------------------------
//  GIMABAL Data Structures
// ------------------------------------------------------------------------------
template<typename T>
struct attitude {
    T roll  = 0;
    T pitch = 0;
    T yaw   = 0;

    static constexpr float DEG2RAD = M_PI / 180.f;
    static constexpr float RAD2DEG = 180.f / M_PI;

    attitude() = default;
    attitude(T val) : roll(val), pitch(val), yaw(val) {};
    attitude(T r, T p, T y) : roll(r), pitch(p), yaw(y) {};

    attitude &to_rad(void)
    {
        roll  *= DEG2RAD;
        pitch *= DEG2RAD;
        yaw   *= DEG2RAD;
        return *this;
    }

    attitude &to_deg(void)
    {
        roll  *= RAD2DEG;
        pitch *= RAD2DEG;
        yaw   *= RAD2DEG;
        return *this;
    }
};

// Helper
static float to_deg(float rad) {
    static constexpr float RAD2DEG = 180.f / M_PI;
    return rad * RAD2DEG;
}

static float to_rad(float deg) {
    static constexpr float DEG2RAD = M_PI / 180.f;
    return deg * DEG2RAD;
}

enum input_mode_t {
    INPUT_ANGLE = 1,
    INPUT_SPEED = 2
};

#endif

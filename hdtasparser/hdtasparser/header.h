//
// Created by wanghaiyang on 9/20/16.
//
#include <sys/types.h>
#include <string>
#include <vector>
#include <list>
#include <map>

#ifndef HDTAS_HEADER_H
#define HDTAS_HEADER_H

#define LEN_ERR_BUF 128 // Max length for error message buffer.
#define LEN_NET_BUF 65536 // Max length for net data buffer.
#define LEN_NET_LITTLE_BUF 1024 // Little buffer
#define LEN_MAX_PIPE_BUF 512 // Pipe data package max length, <= 512 is thread safe. for memory buffer.
#define LEN_MAX_PIPE_PKG 512 // Pipe data package max length, <= 512 is thread safe. for protocol package.
#define LEN_MAX_UDP_BUF 65536 // Max single UDP data package length. for memory buffer.
#define LEN_MAX_UDP_PKG 512 // Max single udp data package length. for protocol package.
#define LEN_MAX_IP_ADDRESS 16 // Include the '\n'
#define LEN_MAX_IP_ADDRESS_6 46 // Include the '\n'
#define LEN_MAX_MAC_ADDRESS 13 // Include the '\n'
#define LEN_MAX_TCP_BUF 65536 // Tcp client buffer lenght.
#define LEN_MAX_CTL_TCP_BUF 2048
#define LEN_MAX_TCP_PKG 512
#define LEN_MR_ID_STR 12*2
#define LEN_MAX_MR_UPDATE_FILE_NAME (12)
#define LEN_MAX_DOMAIN_STRING (64)
#define LEN_MAX_VERSION_STRING (16)
#define LEN_MAX_MAC_STRING (12)


#define NUM_UDP_PKG_CLASS 5 // UDP package class number.

#define BUFFER_SIZE_ERROR 128

#define ERR_TITLE_UDP_MASTER "[udp master] "
#define ERR_TITLE_UDP_CLIENT "[udp client] "

#define ERR_TITLE_TCP_CLIENT "[tcp client] "

#define ERR_TITLE_UDP_MASTER_SERVER "[udp master server]"

#define NAME_ZK_SERVER "master/slaver"
#define NAME_ZK_ROOT_PATH "/hdtas"
#define NAME_ZK_MASTER_PATH "/hdtas/master"
#define NAME_ZK_MASTER_MAIN_PATH "/hdtas/master/main"
#define NAME_ZK_SLAVERS_PATH "/hdtas/slavers"
#define NAME_ZK_SLAVER_ONE_PATH "/hdtas/slavers/slaver"
#define NAME_TITLE_ZOOKEEPER "[zookeeper]"
#define TITLE_MESSAGE_QUEUE "[message queue]"
#define TITLE_MSG_SUB "[msg sub]"
#define TITLE_MSG_PUB "[msg pub]"

#define TITLE_HTTP_SERVER "[http server]"
#define TITLE_BASESTATION_CONTROL_SERVER "[bs ctl server]"

#define TITLE_MASTER_WORKER "[worker %u]"
#define TITLE_MASTER_TCP_SERVER "[tcp server]"
#define TITLE_MASTER_CTL_SERVER "[ctl server]"
#define TITLE_MASTER_UDP_SERVER "[udp server]"
#define TITLE_MASTER_SERVER "[master]"
#define TITLE_MASTER_MQ_SUB "[mq sub]"
#define TITLE_MASTER_MQ_PUB "[mq pub]"
#define TITLE_MASTER_GENERAL_TIMER "[general timer]"
#define TITLE_MASTER_MAIN_MSG "[main imp]"
#define TITLE_MAIN_THREAD "[main]"
#define TITLE_SLAVER_WORKER "[worker %u]"
#define TITLE_SLAVER_UDP_SERVER "[udp server]"
#define TITLE_SLAVER_SERVER "[slaver]"
#define TITLE_SLAVER_MQ_SUB "[mq sub]"
#define TITLE_SLAVER_MQ_PUB "[mq pub]"
#define TITLE_SLAVER_GENERAL_TIMER "[general timer]"
#define TITLE_SLAVER_MAIN_MSG "[main imp]"
#define TITLE_KAFKA_PUBLISH "[kafka pub]"
#define TITLE_WEB_SOCKET "[websocket]"

#define TITLE_BALANCING "[Balancing]"
#define TITLE_DISTRIBUTE "[Distribute]"

#define TITLE_INNER_MSG_PIPE "[inner msg pipe]"
//#define TITLE_UDP_IMP "[udp imp]" // UDP server thread inner msg pipe.
//#define TITLE_TCP_IMP "[tcp imp]" // TCP server thread inner msg pipe.
#define TITLE_MAIN_IMP "[main imp]" // Main thread inner msg pipe.

#define TITLE_DATA_BASE "[mysql]"

#define NAME_MASTER_MR_ALIAS "MR"

#define NAME_MASTER_WORKER "master-worker"
#define NAME_MASTER_TCP_SERVER "master-tcp-server"
#define NAME_MASTER_UDP_SERVER "master-udp-server"
#define NAME_MASTER_SERVER "master-server"

#define NAME_SLAVER_WORKER "slaver-worker"
#define NAME_SLAVER_UDP_SERVER "slaver-udp-server"
#define NAME_SLAVER_SERVER "slaver-server"

#define NAME_FIELD_STADIUM_ID "field_id"
#define NAME_FIELD_MR_M_ID "mrm_id"
#define NAME_FIELD_LOCATIONS "pos"

#define NAME_STARTING "starting..."
#define NAME_SHUTDOWN "shutdown"

#define NAME_JSON_KEY_TYPE "t" // Type.
#define NAME_JSON_KEY_ISPDID "iid" // Ispd id.
#define NAME_JSON_KEY_SID "sid" // Stadium id.
#define NAME_JSON_KEY_MRID "mid" // Mr id.
#define NAME_JSON_KEY_MMRID "mmid" // Mmr id.
#define NAME_JSON_KEY_UWBID "uwbid" // Uwb id.
#define NAME_JSON_KEY_CFGID "cfg" // Cfg id.
#define NAME_JSON_KEY_MSG_ID "msg" // Msg id.
#define NAME_JSON_KEY_DATE_TIME "dt" // Date time.
#define NAME_JSON_KEY_TIME_DIFF "td" // Time diff
#define NAME_JSON_KEY_TIMESTAMP "ts" // Timestamp
#define NAME_JSON_KEY_LOCT "lc" // Location
#define NAME_JSON_KEY_GYRO "gy" // Gyroscope
#define NAME_JSON_KEY_ACCE "ac" // Acceleration
#define NAME_JSON_KEY_HR "ht" // Heart rate
#define NAME_JSON_KEY_POWER "po" // Power
#define NAME_JSON_KEY_CHARGE "ch" // Change
#define NAME_JSON_KEY_LOST_RATE "lr" // Lost rate.
#define NAME_JSON_KEY_COUNTER "cnt" // Counter
#define NAME_JSON_KEY_PKG_TOTAL "pt" // Package total.
#define NAME_JSON_KEY_PKG_RECEIVE "pr" // Package receive.
#define NAME_JSON_KEY_PKG_LOST "pl" // Package lost.
#define NAME_JSON_KEY_EC "ec" // Error code.
#define NAME_JSON_KEY_EMSG "em" // Error message.
#define NAME_JSON_KEY_TS_SEC "tss" // Timestamp Second.
#define NAME_JSON_KEY_TS_USEC "tsu" // Timestamp Microseconds
#define NAME_JSON_KEY_NAME "n" // Name.
#define NAME_JSON_KEY_PROTOCOL "pro" // Protocol.
#define NAME_JSON_KEY_RESERVE "re" // Reserve.
#define NAME_JSON_KEY_STATUS "sta" // Status.
#define NAME_JSON_KEY_LOCAL_IP "lip" // Local ip.
#define NAME_JSON_KEY_NETMASK "nip" // Netmask ip.
#define NAME_JSON_KEY_GATEWAY "gip" // Gateway ip.
#define NAME_JSON_KEY_DNS_ADDR "dip" // DNS ip.
#define NAME_JSON_KEY_LOCAL_IP2 "lip2" // Local ip.
#define NAME_JSON_KEY_NETMASK2 "nip2" // Netmask ip.
#define NAME_JSON_KEY_GATEWAY2 "gip2" // Gateway ip.
#define NAME_JSON_KEY_DNS_ADDR2 "dip2" // DNS ip.
#define NAME_JSON_KEY_DOMAIN "domain" // Domain.
#define NAME_JSON_KEY_MAC_ADDR "mac" // Mac address.
#define NAME_JSON_KEY_TCP_IP "tip" // Tcp ip.
#define NAME_JSON_KEY_TCP_PORT "tp" // Tcp port.
#define NAME_JSON_KEY_UDP_IP "uip" // Udp ip.
#define NAME_JSON_KEY_UDP_PORT "up" // Udp port.
#define NAME_JSON_KEY_VALUE "v" // Value.
#define NAME_JSON_KEY_WIFI "wifi"
#define NAME_JSON_KEY_VERSION "ver"
#define NAME_JSON_KEY_NETWORK_TYPE "nt"
#define NAME_JSON_KEY_NETWORK_MODE "nm"
#define NAME_JSON_KEY_WIFI_MODE "wm"
#define NAME_JSON_KEY_DHCP_STATUS "ds"
#define NAME_JSON_KEY_LENGTH "len"
#define NAME_JSON_KEY_FILE_STATUS "fstatus"
#define NAME_JSON_KEY_FILE_TYPE "ftype"
#define NAME_JSON_KEY_FILE_NAME "fname"
#define NAME_JSON_KEY_FILE_SIZE "fsize"
#define NAME_JSON_KEY_FILE_HASH "fhush"
#define NAME_JSON_KEY_UPDATE_RATE "rate"

#define NAME_JSON_KEY_AUTO_REPORT "ar"
#define NAME_JSON_KEY_AUTO_REPORT_INTERVAL "ari"

//#define NAME_JSON_KEY_PB_ECODE "pec"
#define NAME_JSON_KEY_PB_PRE_STU "ps" /** Power board pre status. */
#define NAME_JSON_KEY_PB_CUR_STU "cs" /** Power board current status. */
#define NAME_JSON_KEY_PB_SWITCH "swi" /** Power board switch status: open/close. */
#define NAME_JSON_KEY_PB_ROUTER "rou" /** Power board router status: open/close. */
#define NAME_JSON_KEY_PB_LOCATE "loc" /** Power board locate status: open/close. */
#define NAME_JSON_KEY_PB_MICROPHONE "mic"
#define NAME_JSON_KEY_PB_CAMEAR "cam" /** Power board camera status: open/close. */
#define NAME_JSON_KEY_PB_PRELOCAD "pre" /** Power board pre-load status: open/close. */
//#define NAME_JSON_KEY_PB_BATTERY_CONTROL "bat" /** Power board battery control status: ???/??? */
#define NAME_JSON_KEY_PB_SOURCE "sou" /** Power board  energy source: battery/supple. */
#define NAME_JSON_KEY_PB_BATTERY_CHARGE "batc" /** Power board battery charge: charge/discharge. */
#define NAME_JSON_KEY_PB_BATTERY_STATUS "bats" /** Power board battery status: open/close. */
#define NAME_JSON_KEY_PB_INTRENET "net" /** Power board internet status: open/close. */
#define NAME_JSON_KEY_PB_BATTERY_CONNECT "batn"
#define NAME_JSON_KEY_PB_BATTERY_CONTROL "batl"
#define NAME_JSON_KEY_PB_BOX_TEMPERATURE "bt" /** Power board box temperature: 0.0 degree. */
#define NAME_JSON_KEY_PB_BATTERY_VOLTAGE "bv" /** Power board battery voltage: 0.0 V. */
#define NAME_JSON_KEY_PB_BATTERY_CAPACITY "bc" /** Power board battery capacity: 0 %. */
#define NAME_JSON_KEY_PB_CAMERA_VOLTAGE "cv" /** Power board camera voltage: 0.0 V. */
#define NAME_JSON_KEY_PB_CAMERA_CURRENT "cc" /** Power board camera current: 0.0 A. */
#define NAME_JSON_KEY_PB_LOCATE_VOLTAGE "lv" /** Power board locate voltage: 0.0 V. */
#define NAME_JSON_KEY_PB_LOCATE_CURRENT "lc" /** Power board locate current: 0.0 A. */
#define NAME_JSON_KEY_PB_ROUTER_VOLTAGE "rv" /** Power board router voltage: 0.0 V. */
#define NAME_JSON_KEY_PB_ROUTER_CURRENT "rc" /** Power board router current: 0.0 A. */
#define NAME_JSON_KEY_PB_SWITCH_VOLTAGE "sv" /** Power board switch voltage: 0.0 V. */
#define NAME_JSON_KEY_PB_SWITCH_CURRENT "sc" /** Power board switch current: 0.0 A. */
#define NAME_JSON_KEY_PB_PRELOAD_VOLTAGE "pv" /** Power board pre-load voltage: 0.0 V. */
#define NAME_JSON_KEY_PB_PRELOAD_CURRENT "pc" /** Power board pre-load current: 0.0 A. */
#define NAME_JSON_KEY_PB_AUTO_REPORT "ar" /** Power board auto report: 1/0. */
#define NAME_JSON_KEY_PB_REPORT_INTERVAL "ari" /** Power board auto report interval: 30s. */
#define NAME_JSON_KEY_PB_OPEN_CLOSE_CAMERA "occ" /** Power board open/close camera: 1/0. */
#define NAME_JSON_KEY_PB_OPEN_CLOSE_MICROPHONE "ocm" /** Power board open/close microphone: 1/0. */
#define NAME_JSON_KEY_PB_OPEN_CLOSE_PRELOAD "ocp" /** Power board open/close prelocad: 1/0. */
#define NAME_JSON_KEY_PB_OPEN_CLOSE "oc"
#define NAME_JSON_KEY_PB_VERSION "ver"

#define NAME_CFG_TYPE "type"
#define NAME_CFG_ZK_INFO "zk_info"
#define NAME_CFG_SS_INFO "ss_info"
#define NAME_CFG_WT_INFO "wt_num"
#define NAME_CFG_TCP_INFO "tcp_info"
#define NAME_CFG_UCP_INFO "udp_info"
#define NAME_CFG_CONTROL_INFO "control_info"
#define NAME_CFG_WS_INFO "ws_info"
#define NAME_CFG_RMQ_INFO "rmq_info"
#define NAME_CFG_DB_INFO "db_info"
#define NAME_CFG_MQ_INFO "mq_info"
#define NAME_CFG_FS_INFO "fs_info"
#define NAME_CFG_UF_INFO "update_path"
#define NAME_CFG_LOG_PATH "log_path"
#define NAME_CFG_DAEMON "daemon"



#define NAME_TABLE_LOCATION "c_field_mr"
#define NAME_TABLE_SID_CFG "hisee_mmr_info"

#define NAME_ISPD_DATA "ispd_data"
#define NAME_ISPD_POSITION "ispd_position"
#define NAME_ISPD_POSITION_BACKUP "ispd_position_backup"
#define NAME_ISPD_DISTANCE "ispd_distance"
#define NAME_ISPD_EXTENTION "ispd_extention"
#define NAME_MR_STATUS "mr_status"
#define NAME_MR_EXTENTION "mr_extention"
#define NAME_PB_STATUS "pb_status"


#ifndef NAME_TOPIC_ISPD_POS
#define NAME_TOPIC_ISPD_POS "htb_position_test"
#endif

#ifndef NAME_TOPIC_ISPD_DIS
#define NAME_TOPIC_ISPD_DIS "htb_distance_test"
#endif

#ifndef NAME_TOPIC_ISPD_EXT
#define NAME_TOPIC_ISPD_EXT "htb_extention_test"
#endif

#ifndef NAME_TOPIC_MR_STU
#define NAME_TOPIC_MR_STU "htb_status_test"
#endif

#ifndef NAME_TOPIC_MR_EXT
#define NAME_TOPIC_MR_EXT "htb_extention_test"
#endif

#ifndef NAME_TOPIC_TEST
#define NAME_TOPIC_TEST "htb_test"
#endif

#define CHANNEL_MASTER_MSG_QUEUE "hdtas_master_msg"
#define CHANNEL_SLAVER_MSG_QUEUE "hdtas_slaver_msg"
#define CHANNEL_WEB_MANAGER_MSG_QUEUE "web_manager_msg"

#define NUM_MASTER_TCP_PORT 9527
#define NUM_MASTER_UDP_PORT 9527
#define NUM_SLAVER_UDP_PORT 9527

#define NAME_DATA_DIRECTORY "./"

#define H_SUCCESS 0

#define H_ERR_STRING(head) head ## _STR

#define H_ERR_SYSTEM -1
#define H_ERR_PARAMETER -10
#define H_ERR_BUFFER_INADEQUATE -11
#define H_ERR_NO_MATCH -12

#define ERR_UNKNOWN_STR "unkonwn error"
#define ERR_SERVER_INNER -100 /** Server inner common error */
#define ERR_SERVER_INNER_STR "server inner common error"
#define ERR_SERVER_INNER_PIPE -101 /** Server inner pipe error */
#define ERR_SERVER_INNER_PIPE_STR "server inner pipe error"
#define ERR_SERVER_TIME_OUT -102
#define ERR_SERVER_TIME_OUT_STR "server time out"
#define ERR_SERVER_BUFFER_TOO_SHORT -103
#define ERR_REQUEST_DATA_TOO_SHORT -104
#define ERR_REQUEST_PROTOCOL_ILLEGAL -105
#define ERR_NO_FIND_MMID -108
#define ERR_NO_FIND_DID -109
#define ERR_NO_FIND_DID_STR "can't find the device in the current stadium"
#define ERR_NO_FIND_MID -110
#define ERR_NO_FINE_MID_STR "can't find the mr in the current stadium"
#define ERR_NO_FIND_SID -111
#define ERR_NO_FIND_SID_STR "can't find the stadium in server"
#define ERR_DATABASE_READ -112 /** read data from database failed */
#define ERR_TCP_WRITE -113 /** write data to tcp client failed */
#define ERR_DATA_SERIALIZE -114 /** data serialize failed */
#define ERR_JSON_STRING -115 /** wrong json string format */
#define ERR_JSON_STRING_STR "bad json data format"
#define ERR_JSON_STRING -115 /** wrong json string format */
#define ERR_WRITE_MMR_CFG -116 /** write mmr's configuration failed */
#define ERR_READ_MMR_CFG -117 /** read mmr's configuration failed */
#define ERR_JSON_BACK_VALUE -118 /** Wrong json parameter of value */
#define ERR_JSON_BACK_VALUE_STR "bad json parameter with key(value)"
#define ERR_MR_GENERAL -119
#define ERR_MR_INVALI_PARAM -120
#define ERR_MR_TIMEOUT -121
#define ERR_MR_ACCESS_DENINE -122
#define ERR_MR_RESOURCE_NOT_FOUND -123
#define ERR_MR_BUSY -124
#define ERR_MR_TRANSCATION_INPROGRESS -125
#define ERR_MR_NP_RESPONSE -126
#define ERR_PB_NO_RESPONSE -127
#define ERR_MSG_QUEUE_EMPTY -128

#define ERR_SEND_2_MR_FAILED -200
#define ERR_PB_WT_ERROR -300 // Write data to power board failed.


#define ERR_CONTROL_REQUEST_DATA_UNRECOGNIZED -400 // 不能识别请求的数据格式（如：可能请求数据不是JSON，等...）
#define ERR_CONTROL_SERVER_INNER -401
#define ERR_CONTROL_NOT_FIND_SID -402
#define ERR_CONTROL_OPERATION_NOT_SUPPORT -403
#define ERR_CONTROL_PROTOCOL_NOT_SUPPORT -404
#define ERR_UPDATE_FILE_NAME_TOO_LONG -403
#define ERR_UPDATE_FILE_TYPE_NOT_SUPPORT -405
#define ERR_UPDATE_FILE_CORRUPED -406
#define ERR_UPDATE_FILE_NOT_FIND -407
#define ERR_UPDATE_DATA_UN_CONSISTENCY -408





#define H_STATUS_OPEN 1
#define H_STATUS_CLOSE 0
#define H_STATUS_UNKNOWN 2
#define H_STATUS_SUCCESS 0
#define H_STATUS_FAILURE -1

#ifdef HDTAS_WINDOWS
#include <stdint.h>
typedef uint64_t union_id;
typedef uint64_t ispd_id;
typedef uint64_t stadium_id; // Redefine type for stadium(field) id.
typedef uint32_t device_mr_id; // Redefine type for mr(device) id.
typedef uint32_t worker_id; // Redefine type for worker(thread) id.
typedef uint32_t http_handle_id;
typedef uint32_t h_pair_id;
typedef uint8_t pkg_t;// Define type for protocol pacakge type.
typedef uint64_t pkg_id_t;  // Define type for serial number of protocol package.
typedef uint16_t pkg_len_t; // Define type for protocol package length.
typedef uint64_t pkg_cnt_t; // Define type for protocol package count.
typedef uint64_t pkg_time_t; // Define type for time(Unix timestamp: second from 19701.1)
#else
typedef u_int64_t union_id;
typedef u_int64_t ispd_id;
typedef u_int64_t stadium_id; // Redefine type for stadium(field) id.
typedef u_int32_t device_mr_id; // Redefine type for mr(device) id.
typedef u_int32_t worker_id; // Redefine type for worker(thread) id.
typedef u_int32_t http_handle_id;
typedef u_int32_t h_pair_id;
typedef u_int8_t pkg_t;// Define type for protocol pacakge type.
typedef u_int64_t pkg_id_t;  // Define type for serial number of protocol package.
typedef u_int16_t pkg_len_t; // Define type for protocol package length.
typedef u_int64_t pkg_cnt_t; // Define type for protocol package count.
typedef u_int64_t pkg_time_t; // Define type for time(Unix timestamp: second from 19701.1)
typedef u_int64_t h_ctl_server_msg_id;
typedef u_int32_t h_ctl_client_msg_id;
#endif

#ifdef HDTAS_WINDOWS
typedef int64_t h_int64;
typedef int32_t h_int32;
typedef int16_t h_int16;
typedef int32_t h_int;
typedef int8_t h_int8;

typedef uint64_t h_uint64;
typedef uint32_t h_uint32;
typedef uint16_t h_uint16;
typedef uint32_t h_uint;
typedef uint8_t h_uint8;
#else
typedef int64_t h_int64;
typedef int32_t h_int32;
typedef int16_t h_int16;
typedef int32_t h_int;
typedef int8_t h_int8;

typedef u_int64_t h_uint64;
typedef u_int32_t h_uint32;
typedef u_int16_t h_uint16;
typedef u_int32_t h_uint;
typedef u_int8_t h_uint8;
#endif

#ifndef SNPRINTF
    #ifdef HDTAS_WINDOWS
        #define SNPRINTF sprintf_s
        #else
        #define SNPRINTF snprintf
    #endif
#endif

#ifndef STRNCASECMP
	#ifdef HDTAS_WINDOWS
		#define STRNCASECMP _strnicmp
	#else
		#define STRNCASECMP strncasecmp
	#endif 
#endif

#ifndef STRCASECMP
	#ifdef HDTAS_WINDOWS
		#define STRCASECMP _stricmp
	#else
		#define STRCASECMP strcasecmp
	#endif 
#endif

#ifndef STRNCPY
    #ifdef HDTAS_WINDOWS
        #define STRNCPY(d, l, s, c) strncpy_s(d, l, s, c)
    #else
        #define STRNCPY(d, l, s, c) strncpy(d, s, l)
    #endif
#endif

#ifndef STRLEN
#ifdef HDTAS_WINDOWS
        #define STRLEN strlen
    #else
        #define STRLEN strlen
    #endif
#endif

#ifdef GDB_DUBAG
#else
#endif

#define HDTAS_TEST_PROTOCOL

enum udp_protocol_version
{
    upv_test = 0,
    upv_test1 = 1,
    upv_20161206 =2
};

enum tcp_protocol_version
{
    pt_old_version = 0, /** Simple hdtas communication protocol. */
    pt_gpb_version = 1, /** Google protocol buffer. */
    pt_real_version = 2, /** Real communication protocol betweent HDTAS server and MRs(devices). */
    tpv_20161206 = 3
};

template< typename T >
struct point_3d
{
    T x, y, z;

    point_3d()
    {
        this->x = this->y = this->z = 0;
    }

    point_3d( T x, T y, T z )
    {
        this->x = x;
        this->y = y;
        this->z = z;
    }
};

typedef std::vector<point_3d<double>> point3d_array;

typedef std::map<device_mr_id, point_3d<double>> point3d_map;

enum file_system_type
{
    fst_local = 1,
    fst_test1 = 2,
};

enum data_compress_type
{
    dct_none = 0,
    dct_snappy = 1
};

/**
 * @brief The state of connection with remote server.
 */
enum remote_connect_state
{
    rcs_connected = 0,
    rcs_connecting = 1,
    rcs_disconnected = 2
};

#define HDTAS_SLAVER_SERVER
#define HDTAS_MASTER_SERVER

//static g_config_protocol_data_debug = 0;
#define DEBUG_PROTOCOL_DATA 0 //1

#define DEBUG_TCP_HB_DATA 0 //1
#define DEBUG_UDP_DATA 1
#define DEBUG_NO_DATABASE 1

#define HDTAS_ERR_BUF_CAP 1024
#define HDTAS_GLOBAL_THREAD_BUF_CAP 1024


#endif //HDTAS_HEADER_H

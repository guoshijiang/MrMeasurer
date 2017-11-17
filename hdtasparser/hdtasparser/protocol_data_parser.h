//
// Created by wanghaiyang on 12/6/16.
//

#ifndef HDTAS_PROTOCOL_DATA_PARSER_H
#define HDTAS_PROTOCOL_DATA_PARSER_H

#include "header.h"
#include "struct.h"
#include "utility.h"
#include <vector>
#include <string>
#include <queue>
#include <list>
#include <map>
#include <cstring>

namespace hdtas
{
/** Unkonwn message type. */
#define PO_MSG_RESERVED 0X00
/** Master device register message type, this is the first message that device sending
    to HDTAS server. */
#define PO_MSG_REG_REQ 0X03
/** HDTAS Server will send this message after receiving the PO_MSG_REG_REQ
 * from master device. */
#define PO_MSG_REG_RPD 0X04
/** Master device will send this message after receiving the PO_MSG_REG_RPD
 *  from HDTAS server. */
#define PO_MSG_REG_CFM 0X05
/** Master device will frequently send this message, keep in touch with
 *  HDTAS server. */
#define PO_MSG_HB_REQ 0X07
/** HDTAS server will send this message to master device after receiving
 *  PO_MSG_HB_REQ. */
#define PO_MSG_HB_RPD 0X08
/** HDTAS server will send this message to request the configuration from
 *  Master device. */
#define PO_MSG_CFG_RD_REQ 0X10
/** Master device will send this message to HDTAS server after receiving
 *  requset message. */
#define PO_MSG_CFG_RD_RPD 0X11
/** HDTAS server will send this message to update the configuration. */
#define PO_MSG_CFG_WT_REQ 0X12
/** Master device will send this message to HDTAS server after receiving
 *  request message. */
#define PO_MSG_CFG_WT_RPD 0X13
/** HDTAS server will send this message to reboot the master device. */
#define PO_MSG_CONTROL_REQ 0X14
/** Master device will send back this mesaage to HDTAS sefver after receiving
 *  reboot request message. */
#define PO_MSG_CONTROL_RPD 0X15

/** UDP data.*/
#define PO_MSG_TAG_DATA_RESP 0XB5
/** UDP MR status data. */
#define PO_MSG_MR_STATUS_RESP 0XB7
/** UDP MR power board data. */
#define PO_MSG_POWER_BOARD_RESP 0XB9

/** Command: reset mr(device). */
#define MMR_CFG_CMD_RESET 0X00000004
/** Command: reboot mr(device). */
#define MMR_CFG_CMD_REBOOT 0X00000000
/** Command: request mr status only once. */
#define MMR_CFG_CMD_MR_STATUS_ONCE 0X00000001
/** Command: write the interval of mr status report. */
#define MMR_CFG_CMD_MR_WT_STATUS_INTERVAL 0X00000002
/** Command: read the interval of mr status report. */
#define MMR_CFG_CMD_MR_RD_STATUS_INTERVAL 0X00000003
/** Command: read the power board information. */
#define MMR_CFG_CMD_MR_POWER_BOARD 0X00001000
/***/
#define MMR_CFG_CMD_MR_UPDATE_BEGIN 0X00002000
#define MMR_CFG_CMD_MR_UPDATE_DATA 0X00002001
#define MMR_CFG_CMD_MR_UPDATE_FINISH 0X00002002



/** */
#define MMR_CFG_ARG_REBOOT 0XABAB5A5A

#define PO_SUCCESS 0x0000 //成功、正常等
#define PO_ERR_GENERAL 0x0001 //通用错误
#define PO_ERR_WRONG_PARAMERR_GENERAL 0X0002 //参数错误
#define PO_ERR_TIMEOUT 0X0003 //超时
#define PO_ERR_ACCESS_DENIED 0X0004 //禁止访问
#define ERR_RESOURCE_NOT_FOUND 0X0005 //注册失败
#define ERR_DEVICE_BUSY 0X0006
#define ERR_TRANSCATION_IN_PROGRESS 0X0007
#define ERR_NO_RESPONSE 0X0008


#define DEFAULT_PO_PKG_HEAD_FLAG_CHAR 0X7E
#define DEFAULT_PO_PKG_HEAD_FLAG_LEN (4)
#define DEFAULT_PO_PKG_HEAD_LEN2 (4+2+1+1+4+4) // Total: 16 bytes.
#define DEFAULT_PO_PKG_MAX_CAP (512)
#define DEFAULT_PO_PKG_MRID_OFF2 (4+2+1+1+4) // Off: 16 bytes.

#define DEFAULT_PO_MSG_HEAD_LEN2 (1+1+1+1) // Total: 4 bytes.
#define DEFAULT_PO_MSG_MAX_CAP (1024)

#define DEFAULT_TCP_REG_REG_LEN2 (2) // Total: 16 + 4 + 2 = 22 bytes.
#define DEFAULT_TCP_REG_RPD_LEN2 (2+1+1+1+1+1+1+2) // Total: 16 + 4 + 10 = 30 bytes.
#define DEFAULT_TCP_REG_CFM_LEN2 (2) // Total: 16 + 4 + 2 = 22 bytes.
#define DEFAULT_TCP_HB_REQ_LEN2 (2) // Total: 16 + 4 + 2 = 22 bytes.
#define DEFAULT_TCP_HB_RPD_LEN2 (2) // Total: 16 + 4 + 2 = 22 bytes.
#define DEFAULT_TCP_CFG_RD_REQ_LEN (2+4+4) // Total: 16+4 + 10 = 30 bytes.
#define DEFAULT_TCP_CFG_RD_RPD_LEN (2+4+4+4+4+4+4+6+4+4+4+4+4+4+4+64+1+1+1+1) //+N Total: 16+4 + 128 = 148 bytes.
#define DEFAULT_TCP_CFG_WT_REQ_LEN (2+4+4+4+4+4+4+4+4+64+1) //+N Total: 16+4 + 99 + N = 119+N bytes.
#define DEFAULT_TCP_CFG_WT_RPD_LEN (2+4+4) // Total: 16+4 + 10 = 30 bytes.
#define DEFAULT_TCP_CTL_REQ_STATIC_LEN (2+4+4) //+N Total: 16 + 4 + 10 + N = 30 + N bytes.
#define DEFAULT_TCP_CTL_RPD_STATIC_LEN (2+4+4) //+N Total: 16 + 4 + 10 + N = 30 + N bytes.
#define DEFAULT_TCP_CTL_RESET_REQ_LEN (DEFAULT_TCP_CTL_REQ_STATIC_LEN+4) // Total: 16+4 +10 +4 = 34 bytes.
#define DEFAULT_TCP_CTL_REBOOT_REQ_LEN (DEFAULT_TCP_CTL_REQ_STATIC_LEN+4) // Total: 16 + 4 + 10 + 4 = 34 bytes.
#define DEFAULT_TCP_CTL_REBOOT_RPD_LEN (DEFAULT_TCP_CTL_RPD_STATIC_LEN+4) // Total: 16 + 4 + 10 + 4 = 34 bytes.
#define DEFAILT_TCP_CTL_STATUS_REQ_LEN (DEFAULT_TCP_CTL_REQ_STATIC_LEN) // Total: 16 + 4 + 10 = 30 bytes.
#define DEFAULT_TCP_CTL_STATUS_STATIC_RPD_LEN (DEFAULT_TCP_CTL_RPD_STATIC_LEN+1)
#define DEFAULT_TCP_CTL_STATUS_DYNAMIC_RPD_LEN (4+2) // Total: 16 + 4 + 10 + 1 + N*6 = 31 + N*6 bytes.
#define DEFAULT_TCP_CTL_MR_AUTO_REPORT_RD_REQ_LEN (DEFAULT_TCP_CTL_REQ_STATIC_LEN+0) // Total: 10 + 0 = 30 (+20=30)bytes.
#define DEFAULT_TCP_CTL_MR_AUTO_REPORT_RD_RPD_LEN (DEFAULT_TCP_CTL_RPD_STATIC_LEN+1) // Total: 10 + 1 = 11 (+20=31)bytes.
#define DEFAULT_TCP_CTL_MR_AUTO_REPORT_WT_REQ_LEN (DEFAULT_TCP_CTL_RPD_STATIC_LEN+1) // Total: 10 + 1 = 11 (+20=31)bytes.
#define DEFAULT_TCP_CTL_MR_AUTO_REPORT_WT_RPD_LEN (DEFAULT_TCP_CTL_RPD_STATIC_LEN+1) // Total: 10 + 1 = 31 (+20=31)bytes.
#define DEFAULT_TCP_CTL_MR_UPDATE_START_REQ (DEFAULT_TCP_CTL_REQ_STATIC_LEN+4+12+1+4+4) // Total: 16 + 4 + 10 + 25 = 55 bytes.
#define DEFAULT_TCP_CTL_MR_UPDATE_START_RPD (DEFAULT_TCP_CTL_RPD_STATIC_LEN+4) // Total: 10 + 4 = 14 (+20=34)bytes.
#define DEFAULT_TCP_CTL_MR_UPDATE_DATA_REQ (DEFAULT_TCP_CTL_REQ_STATIC_LEN+4+4) //+N Total: 10 + 8 + N = 18 + N (+20=38)bytes.
#define DEFAULT_TCP_CTL_MR_UPDATE_DATA_RPD (DEFAULT_TCP_CTL_RPD_STATIC_LEN+4+4) // Total: 10 + 8 = 18 (+20=38)bytes.
#define DEFAULT_TCP_CTL_MR_UPDATE_FINISH_REQ (DEFAULT_TCP_CTL_REQ_STATIC_LEN+4) // Total: 10 + 4 = 14 (+20=34)bytes.
#define DEFAULT_TCP_CTL_MR_UPDATE_FINISH_RPD (DEFAULT_TCP_CTL_RPD_STATIC_LEN+4) // Total: 10 + 4 = 14 (+20=34)bytes.

#define DEFAULT_TCP_CTL_PB_STATIC_RPD_LEN (1+1+1+1+2+1 + 4) // Total: 11 bytes.
#define DEFAULT_TCP_CTL_PB_STATUS_RPD_LEN (2) // Total: 2 = 2 (+11=13)bytes.
#define DEFAULT_TCP_CTL_PB_TEMPERATURE_RPD_LEN (2) // Total: 2 = 2 (+11=13)bytes.
#define DEFAULT_TCP_CTL_PB_BATTERY_RPD_LEN (2+1) // Total: 2+1 = 3 (+11=14)bytes.
#define DEFAULT_TCP_CTL_PB_CAMERA_RPD_LEN (2+2) // Total: 2+2 = 4 (+11=15)bytes.
#define DEFAULT_TCP_CTL_PB_LOCATE_RPD_LEN (2+2) // Total: 2+2 = 4 (+11=15)bytes.
#define DEFAULT_TCP_CTL_PB_ROUTER_RPD_LEN (2+2) // Total: 2+2 = 4 (+11=15)bytes.
#define DEFAULT_TCP_CTL_PB_SWITCH_RPD_LEN (2+2) // Total: 2+2 = 4 (+11=15)bytes.
#define DEFAULT_TCP_CTL_PB_PRELOAD_RPD_LEN (2+2) // Total: 2+2 = 4 (+11=15)bytes.
#define DEFAULT_TCP_CTL_PB_AUTO_REPORT_RPD_LEN (1) // Total: 1 = 1 (+11=12)bytes.
#define DEFAULT_TCP_CTL_PB_AUTO_REPORT_INTERVAL_RPD_LEN (1) // Total: 1 = 1 (+11=12)bytes.
#define DEFAULT_TCP_CTL_PB_SHORTCUT_RPD_LEN (1+1 + 4+4+4+4+4 + 3 + 2 + 2) // Total: 2+20+7 = 29 (+11=40)bytes.
#define DEFAULT_TCP_CTL_PB_SOFT_VERSION_LEN (3) // Total: 3 = 3 (+11=14)bytes.
#define DEFAULT_TCP_CTL_PB_HARD_VERSION_LEN (2) // Total: 2 = 2 (+11=13)bytes.

// Total: 16 + 4 + 10 + N*(20 + M*8) = 30 + N*(20 + M*8) bytes.
#define DEFAULT_UDP_TAG_DATE_RESP_DYNAMIC_TD_LEN (4+4) // Total: 8 bytes.
#define DEFAULT_UDP_TAG_DATE_RESP_DYNAMIC_SENSOR_LEN (15+1+4) // Total: 20 bytes.
#define DEFAULT_UDP_TAG_DATE_RESP_STATIC_LEN (1+1+6+2) // Total: 10 bytes.

// Total: 16 + 4 + 1 + N*6 = 21 + N*6 bytes.
#define DEFAULT_UDP_MR_STATUS_ESP_DYNAMIC_LEN (4+2) // Total: 6 bytes.
#define DEFAULT_UDP_MR_STATUS_ESP_STATIC_LEN (1) // Total: 1 bytes.

// Total: 16 + 4 + 40 = 60 bytes.
#define DEFAULT_UDP_MR_POWER_BOARD_LEN (4+ 1+1+1+1+ 2+2+2+1+2+2+2+2+2+2+2+2+2+2+1+1 +2+1) // Total: 4 + 4 + 29 + 3 = 40 bytes.

#define POWER_BOARD_HEADER 0X84
#define POWER_BOARD_TAILER 0X16
#define POWER_BOARD_CTL_RD 0X55
#define POWER_BOARD_CTL_WT 0XAA
#define POWER_BOARD_CTL_WT_SUCCESS 0X06
#define POWER_BOARD_ADDR_RD_PRE_STATUS 0X01
#define POWER_BOARD_ADDR_RD_CUR_STATUS 0X02
#define POWER_BOARD_ADDR_RD_BOX_TEMPERATURE 0X03
#define POWER_BOARD_ADDR_RD_BATTERY_VOLTAGE_CAPACITY 0X04
#define POWER_BOARD_ADDR_RD_CAMERA_VOLTAGE_CURRENT 0X05
#define POWER_BOARD_ADDR_RD_LOCATE_VOLTAGE_CURRENT 0X06
#define POWER_BOARD_ADDR_RD_ROUTER_VOLTAGE_CURRENT 0X07
#define POWER_BOARD_ADDR_RD_SWITCH_VOLTAGE_CURRENT 0X08
#define POWER_BOARD_ADDR_RD_RELOAD_VOLTAGE_CURRENT 0X09
#define POWER_BOARD_ADDR_RDWT_AUTO_REPORT 0X0A
#define POWER_BOARD_ADDR_RDWT_REPORT_INTERVAL 0X0B
#define POWER_BOARD_ADDR_WT_OPEN_CLOSE 0X0C
#define POWER_BOARD_ADDR_WT_REBOOT 0X0D
#define POWER_BOARD_ADDR_WT_RESET 0X0E
#define POWER_BOARD_ADDR_RD_SHORTCUT_CMD 0X0F
#define POWER_BOARD_ADDR_RD_SOFT_VERSION 0X10
#define POWER_BOARD_ADDR_RD_HARD_VERSION 0X11

#define POWER_BOARD_MICROPHONE_OPEN 0X08
#define POWER_BOARD_MICROPHONE_CLOSE 0XF7
#define POWER_BOARD_CAMERA_OPEN 0X10
#define POWER_BOARD_CAMERA_CLOSE 0XEF
#define POWER_BOARD_PRELOAD_OPEN 0X20
#define POWER_BOARD_PRELOAD_CLOSE 0XDF

#define POWER_BOARD_STATUS_SWITCH 0X01
#define POWER_BOARD_STATUS_ROUTER 0X02
#define POWER_BOARD_STATUS_LOCATE 0X04
#define POWER_BOARD_STATUS_MICROPHONE 0X08
#define POWER_BOARD_STATUS_CAMERA 0X10
#define POWER_BOARD_STATUS_PRELOAD 0X20
#define POWER_BOARD_STATUS_BATTERY 0X40
#define POWER_BOARD_STATUS_SOURCE 0X01 /** True:supply, False:battery */
#define POWER_BOARD_STATUS_BATTERY_CHARGE 0X02 /** True:charge, False:discharge */
#define POWER_BOARD_STATUS_BATTERY_STATUS 0X04 /** True:charge over, False:charging */
#define POWER_BOARD_STATUS_INTERNET 0X08 /** True:internet, False:no internet */
#define POWER_BOARD_STATUS_BATTERY_CONNECT 0X10
#define POWER_BOARD_STATUS_BATTERY_COLTROL 0X20
#define POWER_BOARD_AUTO_REPORT_OPEN 0X77
#define POWER_BOARD_AUTO_REPORT_CLOSE 0X44


    class po_package2
    {
    public:
        static int get_mmr_id_from_buffer( const unsigned char* b, size_t l, device_mr_id& id,
                                          size_t off = DEFAULT_PO_PKG_MRID_OFF2 );

    public:
        po_package2( unsigned int h = DEFAULT_PO_PKG_HEAD_LEN2,
                     unsigned int c = DEFAULT_PO_PKG_MAX_CAP );
        virtual ~po_package2();

        const std::string& get_error() const { return this->strerr_; }

        int initialize();
        void uninitialize();

        inline h_uint32 get_pkg_len() const { return this->pkg_len; }

        inline const unsigned char* get_data_buf() const { return this->data_buf_; }
        inline size_t get_data_buf_len() const { return this->data_buf_len_; }

        inline size_t get_avai_capacity() const { return this->data_buf_cap_-this->head_flag_len_; }

        inline h_uint32 get_pkg_cnt() const { return this->pkg_cnt; }
        inline void set_pkg_cnt( h_uint32 c ) { this->pkg_cnt = c; }

        inline h_uint32 get_pkg_id() const { return this->pkg_id; }
        inline void set_pkg_id( h_uint32 id ) { this->pkg_id = id; }

        inline h_uint32 get_msg_id() const { return this->msg_id; }
        inline void set_msg_id( h_uint32 id ) { this->msg_id = id; }

        inline h_uint64 get_mmr_id() const { return this->mmr_id; }
        inline void set_mmr_id( h_uint64 id ) { this->mmr_id = id; }

        virtual int serialize( const unsigned char* buffer, size_t len, size_t& off );

        /**
         *
         * @param buffer
         * @param len
         * @param off
         * @return -1 缓存中有错误，但是可以越过这部分(off)错误继续读；
         *           1 缓存中数据不完整，需要继续读socket后面的数据；
         */
        virtual int deserialize( const unsigned char* buffer, size_t len, size_t& off );

    private:
        size_t find_head_flag( const unsigned char* buffer, size_t len );

    public:
        h_uint32 pkg_len;
        h_uint32 pkg_cnt;
        h_uint32 pkg_id; // Start from 0.
        h_uint32 msg_id;
        h_uint64 mmr_id;

        unsigned char* head_flag_chars_;
        unsigned int head_flag_len_;
        const unsigned int package_head_len_;

        const size_t data_buf_cap_;
        size_t data_buf_len_;
        unsigned char* data_buf_;

        std::string strerr_;
    };

    typedef std::vector<po_package2*> po_package_array2;

    typedef std::list<po_package2*> po_package_list2;
    typedef po_package_list2::iterator po_package_list_it2;

    typedef std::map<h_uint32, po_package_array2> po_package_container2;
    typedef po_package_container2::iterator po_package_container_it2;

    typedef struct po_package_compare
    {
        bool operator () ( po_package2* p1, po_package2* p2 )
        {
            return p1->msg_id > p2->msg_id;
        }

        bool operator () ( const po_package2* p1, const po_package2* p2 )
        {
            return p1->msg_id > p2->msg_id;
        }
    } po_pacage_compare;



    typedef std::vector<po_package2*> po_package_array2;
    typedef std::priority_queue<po_package2*,
                                std::vector<po_package2*>,
                                po_package_compare> po_package_queue2;
    class protocol_base
    {
    public:
        protocol_base() {}
        virtual ~protocol_base() {}

        const std::string& get_error() const { return this->strerr; }

    protected:
        virtual int serialize_in( unsigned char* buffer, size_t len, size_t& off ) = 0;
        virtual int deserialize_in( const unsigned char* buffer, size_t len, size_t& off ) = 0;

    protected:
        std::string strerr;
    };

    class po_message2 : public protocol_base
    {
    public:
        po_message2( unsigned int l = DEFAULT_PO_MSG_HEAD_LEN2,
                    unsigned int c = DEFAULT_PO_MSG_MAX_CAP);
        virtual ~po_message2();

        virtual void initialize();
        virtual void uninitialize();

        inline device_mr_id get_mmr_id() const { return this->mmr_id_; }
        inline h_uint32 get_msg_id() const { return this->msg_id_; }

        inline const unsigned char* get_msg_buf() const 
		{ 
			if (this->head_) return this->msg_buf_; 
			else return this->data_buf_; 
		}
        inline size_t get_msg_buf_len() const 
		{
			if (this->head_) return this->msg_buf_len_;
			else return this->data_buf_len_;
		}

        inline pkg_t get_msg_type() const { return this->type; }
        inline void set_msg_type( pkg_t t ) { this->type = t; }

        inline pkg_t get_msg_ack() const { return this->ack; }
        inline void set_msg_ack( pkg_t a ) { this->ack = a; }

        inline pkg_t get_msg_ver() const { return this->version; }
        inline void set_msg_ver( pkg_t v ) { this->version = v; }

        inline pkg_t get_msg_res() const { return this->reserve; }
        inline void set_msg_res( pkg_t r ) { this->reserve = r; }

        virtual int serialize( bool head = true );

        virtual int deserialize( const po_package_array2& items );
        virtual int deserialize( const po_package2* item );

    protected:
        virtual int serialize_in( unsigned char* buffer, size_t len, size_t& off ) override;
        virtual int deserialize_in( const unsigned char* buffer, size_t len, size_t& off ) override;

    public:
        pkg_t type;
        pkg_t ack;
        pkg_t version;
        pkg_t reserve;

		bool head_ = true;

        device_mr_id mmr_id_;
        h_uint32 msg_id_;

        // 需要两个缓存，分别用于缓存序列化时使用的Buffer，和反序列化时使用的Buffer，
        // 由于反序列化时需要将多个Package组合成为一个Message，所以需要将Package中的
        // Message片段组合到一个整体的Buffer中，固需要两个缓存。

        const unsigned int msg_head_len_;

        const size_t data_buf_cap_;
        size_t data_buf_len_;
        unsigned char* data_buf_; // Read data buffer.

        const size_t msg_buf_cap_;
        size_t msg_buf_len_;
        unsigned char* msg_buf_; // Write data buffer.
    };

    class po_protocol_middle : public protocol_base
    {
    public:
        po_protocol_middle( pkg_t t) : type_(t) {}
        virtual ~po_protocol_middle() {}

        inline pkg_t get_type() const { return this->type_; }
        inline device_mr_id get_mmr_id() const { return this->mmr_id_; }
		inline void set_mmr_id(device_mr_id id) { this->mmr_id_ = id; }
        inline h_uint32 get_msg_id() const { return this->msg_id_; }
		inline void set_msg_id(h_uint32 id) { this->msg_id_ = id; }

        int serialize( po_message2* msg );
        int deserialize( const po_message2* msg );

    protected:
        virtual int serialize_in( unsigned char* buffer, size_t len, size_t& off ) override { return -1; }
        virtual int deserialize_in( const unsigned char* buffer, size_t len, size_t& off ) override { return -1; };

    protected:
        pkg_t type_;
        device_mr_id mmr_id_;
        h_uint32 msg_id_;
    };

    class po_tcp_reg_req2 : public po_protocol_middle
    {
    public:
        po_tcp_reg_req2();
        virtual ~po_tcp_reg_req2();

        inline h_uint32 get_status() const { return this->status; }
        inline void set_status( h_uint32 s ) { this->status = s; }

    protected:
        virtual int serialize_in( unsigned char* buffer, size_t len, size_t& off ) override;
        virtual int deserialize_in( const unsigned char* buffer, size_t len, size_t& off ) override;

    private:
        h_uint32 status;
    };

    class po_tcp_reg_rpd2 : public po_protocol_middle
    {
    public:
        po_tcp_reg_rpd2();
        virtual ~po_tcp_reg_rpd2();

        inline const ispd_date_time& get_date_time() const { return this->date_time; }
        inline void set_date_time( const ispd_date_time& dt ) { this->date_time = dt; }

        inline h_uint32 get_reg_error() const { return this->error; }
        inline void set_reg_error( h_uint32 e ) { this->error = e; }

    protected:
        virtual int serialize_in( unsigned char* buffer, size_t len, size_t& off ) override;
        virtual int deserialize_in( const unsigned char* buffer, size_t len, size_t& off ) override;

    private:
        ispd_date_time date_time;
        h_uint32 error;
    };

    class po_tcp_reg_cfm2 : public po_protocol_middle
    {
    public:
        po_tcp_reg_cfm2();
        virtual ~po_tcp_reg_cfm2();

        inline h_uint32 get_error_code() const { return this->error; }
        inline void set_error_code( h_uint32 e ) { this->error = e; }

    protected:
        virtual int serialize_in( unsigned char* buffer, size_t len, size_t& off ) override;
        virtual int deserialize_in( const unsigned char* buffer, size_t len, size_t& off ) override;

    private:
        h_uint32 error;
    };

    class po_tcp_hb_req2 : public po_protocol_middle
    {
    public:
        po_tcp_hb_req2();
        virtual ~po_tcp_hb_req2();

        inline h_uint32 get_status() const { return this->status; }
        inline void set_status( h_uint32 s ) { this->status = s; }

    protected:
        virtual int serialize_in( unsigned char* buffer, size_t len, size_t& off ) override;
        virtual int deserialize_in( const unsigned char* buffer, size_t len, size_t& off ) override;

    private:
        h_uint32 status;
    };

    class po_tcp_hb_rpd2 : public po_protocol_middle
    {
    public:
        po_tcp_hb_rpd2();
        virtual ~po_tcp_hb_rpd2();

        inline h_uint32 get_error_code() const { return this->error; }
        inline void set_error_code( h_uint32 e ) { this->error = e; }

    protected:
        virtual int serialize_in( unsigned char* buffer, size_t len, size_t& off ) override;
        virtual int deserialize_in( const unsigned char* buffer, size_t len, size_t& off ) override;

    private:
        h_uint32 error;
    };

    class po_tcp_cfg_read_req : public po_protocol_middle
    {
    public:
        po_tcp_cfg_read_req();
        virtual ~po_tcp_cfg_read_req();

        inline void set_status( h_uint32 s ) { this->status = s; }
        inline void set_pair_id( h_pair_id id ) { this->pair_id_  = id; }
        inline void set_des_mr_id( device_mr_id id ) { this->des_mr_id_ = id; }

    protected:
        virtual int serialize_in( unsigned char* buffer, size_t len, size_t& off ) override;
        virtual int deserialize_in( const unsigned char* buffer, size_t len, size_t& off ) override;

    private:
        device_mr_id des_mr_id_ = 0;
        h_pair_id pair_id_ = 0;
        h_uint32 status = 0;
    };

    enum po_network_type
    {
        pnt_unknown = 0,
        pnt_line,
        pnt_wifi
    };

    enum po_network_select
    {
        pns_unknown = 0,
        pns_auto,
        pns_line,
        pns_wifi
    };

    enum po_wifi_mode
    {
        pwm_unknown = 0,
        pwm_auto,
        pwm_manual
    };

    enum po_dhcp_mode
    {
        pdm_unknown = 0,
        pdm_open,
        pdm_close
    };

    class po_tcp_cfg_read_rpd : public po_protocol_middle
    {
    public:
        po_tcp_cfg_read_rpd();
        virtual ~po_tcp_cfg_read_rpd();

        inline h_uint32 get_ecode() const { return this->ecode; }

        inline h_pair_id get_pair_id() const { return this->pair_id_; }

        inline device_mr_id get_des_mr_id() const { return this->des_mr_id_; }

        inline const char* get_mac_addr() const { return this->mac_addr_.c_str(); }

        inline h_uint32 get_static_ip() const { return this->static_ip_; }
        inline h_uint32 get_static_netmask() const { return this->static_netmask_; }
        inline h_uint32 get_static_gateway() const { return this->static_gateway_; }
        inline h_uint32 get_static_dns() const { return this->static_dns_; }

        inline h_uint32 get_dhcp_ip() const { return this->dhcp_ip_; }
        inline h_uint32 get_dhcp_netmask() const { return this->dhcp_netmask_; }
        inline h_uint32 get_dhcp_gateway() const { return this->dhcp_gateway_; }
        inline h_uint32 get_dhcp_dns() const { return this->dhcp_dns_; }

        inline const char* get_server_domain() const { return this->domain_.c_str(); }
        inline h_uint32 get_server_tcp_port() const { return this->tcp_port_; }
        inline h_uint32 get_server_udp_port() const { return this->udp_port_; }

        inline h_uint32 get_version() const { return this->version_; }

        inline po_network_type get_network_type() const { return this->network_type_; }
        inline po_network_select get_network_select() const { return this->netmask_select_; }

        inline po_wifi_mode get_wifi_mode() const { return this->wifi_mode_; }

        inline po_dhcp_mode get_dhcp_mode() const { return this->dhcp_mode_; }


    protected:
        virtual int serialize_in( unsigned char* buffer, size_t len, size_t& off ) override;
        virtual int deserialize_in( const unsigned char* buffer, size_t len, size_t& off ) override;

    private:
        h_pair_id pair_id_ = 0;
        device_mr_id des_mr_id_ = 0;
        h_uint32 ecode = 0;

        std::string mac_addr_;

        h_uint32 static_ip_ = 0;
        h_uint32 static_netmask_ = 0;
        h_uint32 static_gateway_ = 0;
        h_uint32 static_dns_ = 0;

        h_uint32 dhcp_ip_ = 0;
        h_uint32 dhcp_netmask_ = 0;
        h_uint32 dhcp_gateway_ = 0;
        h_uint32 dhcp_dns_ = 0;

        std::string domain_;
        h_uint32 tcp_port_ = 0;
        h_uint32 udp_port_ = 0;
        h_uint32 version_ = 0;

        po_network_type network_type_ = po_network_type::pnt_unknown;
        po_network_select netmask_select_ = po_network_select::pns_unknown;

        po_wifi_mode wifi_mode_ = po_wifi_mode::pwm_unknown;
        po_dhcp_mode dhcp_mode_ = po_dhcp_mode::pdm_unknown;
    };

    class po_tcp_cfg_write_req : public po_protocol_middle
    {
    public:
        po_tcp_cfg_write_req();
        virtual ~po_tcp_cfg_write_req();

        inline void set_status( h_uint32 s ) { this->status_ = s; }
        inline void set_pair_id( h_pair_id id ) { this->pair_id_ = id; }
        inline void set_des_mr_id( device_mr_id id ) { this->des_mr_id_ = id; }

        inline void set_static_ip( h_uint32 ip ) { this->static_ip_ = ip; }
        inline void set_static_netmask( h_uint32 nm ) { this->static_netmask_ = nm; }
        inline void set_static_gateway( h_uint32 gw ) { this->static_gateway_ = gw; }
        inline void set_static_dns( h_uint32 dns ) { this->static_dns_ = dns; }

        inline void set_server_domain( const char* d ) { this->domain_ = d; }
        inline void set_server_tcp_port( h_uint32 p ) { this->tcp_port_ = p; }
        inline void set_server_udp_port( h_uint32 p ) { this->udp_port_ = p; }

        inline void set_network_select( po_network_select s ) { this->netmask_select_ = s; }

    protected:
        virtual int serialize_in( unsigned char* buffer, size_t len, size_t& off ) override;
        virtual int deserialize_in( const unsigned char* buffer, size_t len, size_t& off ) override;

    private:
        h_uint32 status_ = 0;
        h_pair_id pair_id_ = 0;
        device_mr_id des_mr_id_ = 0;

        h_uint32 static_ip_  = 0;
        h_uint32 static_netmask_  = 0;
        h_uint32 static_gateway_ = 0;
        h_uint32 static_dns_ = 0;


        std::string domain_;
        h_uint32 tcp_port_  = 0;
        h_uint32 udp_port_ = 0;

        po_network_select netmask_select_ = po_network_select::pns_auto;
    };

    class po_tcp_cfg_write_rpd : public po_protocol_middle
    {
    public:
        po_tcp_cfg_write_rpd();
        virtual ~po_tcp_cfg_write_rpd();

        inline h_pair_id get_pair_id() const { return this->pair_id_; }
        inline device_mr_id get_des_mr_id() const { return this->des_mr_id_; }
        inline h_uint32 get_ecode() const { return this->ecode; }

    protected:
        virtual int serialize_in( unsigned char* buffer, size_t len, size_t& off ) override;
        virtual int deserialize_in( const unsigned char* buffer, size_t len, size_t& off ) override;

    private:
        device_mr_id des_mr_id_ = 0;
        h_pair_id pair_id_ = 0;
        h_uint32 ecode = 0;
    };

    class po_tcp_ctl_req : public po_protocol_middle
    {
    public:
        po_tcp_ctl_req();
        virtual ~po_tcp_ctl_req();

        inline void set_status( h_uint32 s ) { this->status = s; }
        inline void set_pair_id( h_pair_id id ) { this->pair_id_ = id; }

        void set_mr_reset( device_mr_id );
        void set_mr_reboot( device_mr_id );
        void set_mr_status_report_once(  );
        void set_wt_mr_status_report_interval( h_uint8 t );
        void set_rd_mr_status_report_interval();

        void set_mr_update_begin( device_mr_id mid, const mr_update_info& info  );
        // id: pkg id from 0.
        void set_mr_update_data( device_mr_id mid, h_uint32 id, const unsigned char* data, h_uint32 len );
        void set_mr_update_finish( device_mr_id mid );

        void set_rd_power_board_pre_status( device_mr_id id );
        void set_rd_power_board_cur_status( device_mr_id id );
        void set_rd_power_board_box_temperature( device_mr_id id );
        void set_rd_power_board_battery_voltage_capacity( device_mr_id id );
        void set_rd_power_board_camera_voltage_current( device_mr_id id );
        void set_rd_power_board_locate_voltage_current( device_mr_id id );
        void set_rd_power_board_router_voltage_current( device_mr_id id );
        void set_rd_power_board_switch_voltage_current( device_mr_id id );
        void set_rd_power_board_reload_voltage_current( device_mr_id id );
        void set_rd_power_board_auto_report( device_mr_id id );
        void set_wt_power_board_auto_report( device_mr_id id, bool open = true );
        void set_rd_power_board_report_interval( device_mr_id id );
        void set_wt_power_board_report_interval( device_mr_id id, unsigned char v = 30 ); // Second.
        void set_wt_power_boart_open_close_mic_cam( device_mr_id id, bool mo, bool co, bool po, bool bo );
        void set_wt_power_board_open( device_mr_id id );
        void set_wt_power_board_close( device_mr_id id );
        void set_wt_power_board_reboot( device_mr_id id );
        void set_wt_power_board_reset( device_mr_id id );
        void set_rd_power_board_shortcut_cmd( device_mr_id id );
        void set_rd_power_board_soft_version( device_mr_id id );
        void set_rd_power_board_hard_version( device_mr_id id );
    protected:
        virtual int serialize_in( unsigned char* buffer, size_t len, size_t& off ) override;
        virtual int deserialize_in( const unsigned char* buffer, size_t len, size_t& off ) override;

        void read_power_board( unsigned char addr, device_mr_id id );
        void write_power_board( unsigned char addr, h_uint8 value, device_mr_id id );

    private:
        h_uint32 status = 0;
        h_uint32 cmd = 0;
        h_pair_id pair_id_ = 0;

        unsigned char* data = NULL;
        size_t data_cap_ = 0;
        size_t data_len_ = 0;
    };

    class po_tcp_ctl_rpd : public po_protocol_middle
    {
    public:
        po_tcp_ctl_rpd();
        virtual ~po_tcp_ctl_rpd();

        /** For control command: MMR_CFG_CMD...  */
        inline h_uint32 get_cmd() const { return this->cmd_; }
        inline h_uint32 get_ecode() const { return this->ecode_; }
        inline h_pair_id get_pair_id() const { return this->pair_id_; }

        inline device_mr_id get_mr_id() const { return this->mr_id_; }


        inline const mr_status_array& get_mr_status_array() const { return this->status_array_; }
        inline h_uint8 get_mr_report_interval() const { return this->mr_report_interval_; }

        device_mr_id get_mr_update_begin() const { return this->mr_update_id_; }
        std::pair<device_mr_id, h_uint32> get_mr_update_data() const
            { return std::pair<device_mr_id, h_uint32>(this->mr_update_id_, this->mr_update_msg_id_); }
        device_mr_id get_mr_update_finish() const { return this->mr_update_id_; }

        /** For power board: POWER_BOARD_ADDR... */
        inline pkg_t get_pb_addr() const { return this->addr_; }
        inline int get_pb_ecode() const { return this->pb_ecode_; }
        inline const power_board_status& get_power_board_pre_status() const { return this->pre_status_; }
        inline const power_board_status& get_power_board_cur_status() const { return this->cur_status_; }
        inline const power_board_status_group& get_power_board_status_group() const { return this->status_gropu_; }
        inline const char* get_power_board_soft_version() const { return this->soft_version_.c_str(); }
        inline const char* get_power_board_hard_version() const { return this->hard_version_.c_str(); }

        inline bool get_pb_read_write_mode() const { return this->pb_rd_wt_mode_ == POWER_BOARD_CTL_RD; }

    protected:
        virtual int serialize_in( unsigned char* buffer, size_t len, size_t& off ) override;
        virtual int deserialize_in( const unsigned char* buffer, size_t len, size_t& off ) override;

        int deserialize_pb( const unsigned char* buffer, size_t len, size_t& off );
    private:
        int pb_ecode_ = 0;
        h_uint32 ecode_ = 0;
        h_uint32 cmd_ = 0;
        pkg_t addr_ = 0;

        h_pair_id pair_id_ = 0;

        h_uint8 mr_report_interval_ = 0;

        device_mr_id mr_id_ = 0;
        device_mr_id mr_update_id_ = 0;
        h_uint32 mr_update_msg_id_ = 0;

        h_uint8 pb_rd_wt_mode_;
        mr_status_array status_array_;
        power_board_status pre_status_;
        power_board_status cur_status_;
        power_board_status_group status_gropu_;

        std::string soft_version_;
        std::string hard_version_;

    };

    class po_udp_data_resp2 : public po_protocol_middle
    {
    public:
        po_udp_data_resp2();

        virtual ~po_udp_data_resp2();

        inline h_uint32 get_mr_count() const { return this->mr_cnt; }
        inline h_uint32 get_ispd_count() const { return this->ispd_cnt; }

        inline const ispd_date_time& get_date_time() const { return this->date_time; }

        inline const isdp_data_array& get_data_array() const { return this->data_array; }

    protected:
        virtual int serialize_in( unsigned char* buffer, size_t len, size_t& off ) override;
        virtual int deserialize_in( const unsigned char* buffer, size_t len, size_t& off ) override;

    private:

        h_uint32 mr_cnt;
        h_uint32 ispd_cnt;

        ispd_date_time date_time;

        isdp_data_array data_array;
    };

    class po_udp_mr_status : public po_protocol_middle
    {
    public:
        po_udp_mr_status();
        virtual ~po_udp_mr_status();

        inline const mr_status_array& get_status_array() const { return this->status_array; }

    protected:
        virtual int serialize_in( unsigned char* buffer, size_t len, size_t& off ) override;
        virtual int deserialize_in( const unsigned char* buffer, size_t len, size_t& off ) override;

    private:
        mr_status_array status_array;
    };

    class po_udp_power_board : public po_protocol_middle
    {
    public:
        po_udp_power_board();
        virtual ~po_udp_power_board();

        inline device_mr_id get_pb_mr_id() const { return this->id_; }
        inline const power_board_status& get_status() const { return this->status_; }
        inline const power_board_status_group& get_status_group() const { return this->group_; }

    protected:
        virtual int serialize_in( unsigned char* buffer, size_t len, size_t& off ) override;
        virtual int deserialize_in( const unsigned char* buffer, size_t len, size_t& off ) override;

    private:
        device_mr_id id_ = 0;
        power_board_status status_;
        power_board_status_group group_;
    };


}


#endif //HDTAS_PROTOCOL_DATA_PARSER_H

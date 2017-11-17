//
// Created by wanghaiyang on 12/6/16.
//

#include "protocol_data_parser.h"
#include "utility.h"
#include <string.h>

#define DEBUG_PRINT_RAW_DATA 0

#define DEBUG_PRINT_RAW_REGISTER 0
#define DEBUG_PRINT_RAW_MR_HEART_BEAT 0
#define DEBUG_PRINT_RAW_CONFIG_RD 1
#define DEBUG_PRINT_RAW_CONFIG_WT 1
#define DEBUG_PRINT_RAW_CONTROL 0

#define DEBUG_PRINT_RAW_ISPD_DATA 0
#define DEBUG_PRINT_RAW_MR_STATUS 0
#define DEBUG_PRINT_RAW_PB_STATUS 0

#define NUM_MAX_MRID_LEN 12

namespace hdtas
{
    static const unsigned int wCRCTalbeAbs[] = {
            0x0000, 0xCC01, 0xD801, 0x1400, 0xF001, 0x3C00,
            0x2800, 0xE401, 0xA001, 0x6C00, 0x7800, 0xB401,
            0x5000, 0x9C01, 0x8801, 0x4400,
    };

    static unsigned int CRC16_2( const unsigned char *pchMsg, size_t wDataLen)
    {
        unsigned int wCRC = 0xFFFF;
        for ( unsigned char i = 0; i < wDataLen; i++)
        {
            unsigned char chChar = *pchMsg++;
            wCRC = wCRCTalbeAbs[(chChar ^ wCRC) & 15] ^ (wCRC >> 4);
            wCRC = wCRCTalbeAbs[((chChar >> 4) ^ wCRC) & 15] ^ (wCRC >> 4);
        }
        return wCRC;
    }

    int po_package2::get_mmr_id_from_buffer( const unsigned char* b, size_t l,
                                             device_mr_id& id, size_t off  )
    {
        if ( l < off+sizeof(device_mr_id) )
            return -1;
        id = 0;
        utility::deserialize_int( b+off, id );
        return 0;
    }

    po_package2::po_package2( unsigned int h, unsigned int c ) :
            pkg_len(0),
            pkg_cnt(0),
            pkg_id(0),
            msg_id(0),
            mmr_id(0),
            head_flag_chars_(NULL),
            head_flag_len_(0),
            package_head_len_(h),
            data_buf_cap_(c),
            data_buf_len_(0),
            data_buf_(NULL)
    {

    }

    po_package2::~po_package2()
    {

    }

    int po_package2::initialize()
    {
        this->head_flag_len_ = DEFAULT_PO_PKG_HEAD_FLAG_LEN;
        this->head_flag_chars_ = new unsigned char[this->head_flag_len_];
        for ( unsigned int i = 0; i < this->head_flag_len_; ++i )
        {
            this->head_flag_chars_[i] = DEFAULT_PO_PKG_HEAD_FLAG_CHAR;
        }

        this->data_buf_ = new unsigned char[this->data_buf_cap_];
        this->data_buf_len_ = 0;
        return 0;
    }

    void po_package2::uninitialize()
    {
        delete [] this->head_flag_chars_;
        this->head_flag_chars_ = NULL;
        delete [] this->data_buf_;
        this->data_buf_ = NULL;
    }

    int po_package2::serialize( const unsigned char* buffer, size_t len, size_t& off )
    {
        off = 0;
        this->pkg_len = this->package_head_len_ + (h_uint32)len;

        memset( this->data_buf_, 0, this->pkg_len ); // Clear the buffer.

        memcpy( this->data_buf_+off, this->head_flag_chars_, this->head_flag_len_ );
        off += this->head_flag_len_;

        h_uint16 tempu16 = this->pkg_len;
        off += utility::serialize_int( tempu16, this->data_buf_+off );

        this->data_buf_[off++] = this->pkg_cnt;
        this->data_buf_[off++] = this->pkg_id;

        h_uint32 tempu32 = this->msg_id;
        off += utility::serialize_int( tempu32, this->data_buf_+off );

        tempu32 = this->mmr_id;
        off += utility::serialize_int( tempu32, this->data_buf_+off );

        memcpy( this->data_buf_+off, buffer, len );
        off += len;

        this->data_buf_len_ = this->pkg_len;

        return 0;
    }

    int po_package2::deserialize( const unsigned char* buffer, size_t len, size_t& off )
    {
        off = this->find_head_flag( buffer, len );
        if ( off != 0 )
        {
            this->strerr_ = "the package head flag offset at begin is wrong,";
            this->strerr_ += ", and try to find the new head flage at(";
            this->strerr_ += utility::uint64_2_str(off);
            this->strerr_ += ")";
            return -1;
        }

        if ( off+this->head_flag_len_ >= len )
            return 1;

        off += 4;

        h_uint16 tempu16 = 0;
        off += utility::deserialize_int( buffer+off, tempu16 );
        if ( 0 == tempu16 || tempu16 > this->data_buf_cap_-this->package_head_len_ )
        {
            // 如果解析出来的数据长度大于缓存的大小或者等于0，则丢弃整帧数据
            //off -= sizeof(tempu16);
            //off += this->find_head_flag( buffer+off, len-off );
            off = len;
            this->strerr_ = "the package logical length(";
            this->strerr_ += utility::uint64_2_str( tempu16 );
            if ( 0 == tempu16 )
            {
                this->strerr_ += ") is wrong ";
            }
            else
            {
                this->strerr_ += ") is larger than the MAX length(";
                this->strerr_ += utility::uint64_2_str( this->data_buf_cap_-
                                                        this->package_head_len_  );
                this->strerr_ += "), ";
            }
            this->strerr_ += "and try to find new head flag at(";
            this->strerr_ += utility::uint64_2_str( off );
            this->strerr_ += ")";
            return -1;
        }
        else
            this->pkg_len = tempu16;

        if ( this->pkg_len > len )
            return 1;

        this->pkg_cnt = buffer[off++];
        this->pkg_id = buffer[off++];

        h_uint32 tempu32 = 0;
        off += utility::deserialize_int( buffer+off, tempu32 );
        this->msg_id = tempu32;

        tempu32 = 0;
        off += utility::deserialize_int( buffer+off, tempu32 );
        this->mmr_id = tempu32;

        this->data_buf_len_ = this->pkg_len - this->package_head_len_;
        memcpy( this->data_buf_, buffer+off, this->data_buf_len_ );
        off += this->data_buf_len_;

        return 0;
    }

    size_t po_package2::find_head_flag( const unsigned char* buffer, size_t len )
    {
        size_t off = 0;
        bool bfind = false;
        while ( off+this->head_flag_len_ <= len )
        {
            if ( memcmp(buffer+off, this->head_flag_chars_, this->head_flag_len_) == 0 )
            {
                bfind = true;
                break;
            }
            else
                off += 1;
        }
        if ( !bfind )
            off = len;
        return off;
    }

    po_message2::po_message2( unsigned int l, unsigned int c ) :
            type(0),
            ack(0),
            version(0),
            reserve(0),
            mmr_id_(0),
            msg_id_(0),
            msg_head_len_(l),
            data_buf_cap_(c),
            data_buf_(NULL),
            msg_buf_cap_(c),
            msg_buf_len_(0),
            msg_buf_(NULL)
    {

    }

    po_message2::~po_message2()
    {
        this->uninitialize();
    }

    void po_message2::initialize()
    {
        this->data_buf_len_ = 0;
        this->data_buf_ = new unsigned char[this->data_buf_cap_];

        this->msg_buf_len_ = 0;
        this->msg_buf_ = new unsigned char[this->msg_buf_cap_];
    }

    void po_message2::uninitialize()
    {
        delete [] this->data_buf_;
        delete [] this->msg_buf_;
        this->data_buf_ = NULL;
        this->msg_buf_ = NULL;
    }

    int po_message2::serialize( bool head )
    {
		this->head_ = head;
		if (!head) return 0;
        size_t off = 0;
        return this->serialize_in( this->msg_buf_, this->msg_buf_cap_, off );
    }

    int po_message2::deserialize( const po_package_array2& items )
    {
        po_package_queue2 item_queue;
        for ( unsigned int i = 0; i < items.size(); ++i )
        {
            item_queue.push( items[i] );
        }

        this->msg_buf_len_ = 0;
        while ( !item_queue.empty() )
        {
            po_package2* pp = item_queue.top();
            this->mmr_id_ = pp->mmr_id;
            this->msg_id_ = pp->msg_id;
            memcpy( this->msg_buf_+this->msg_buf_len_, pp->data_buf_, pp->data_buf_len_ );
            this->msg_buf_len_ += pp->data_buf_len_;
        }

        size_t off = 0;
        return this->deserialize_in( this->msg_buf_, this->msg_buf_len_, off );
    }

    int po_message2::deserialize( const po_package2* item )
    {
        this->mmr_id_ = item->mmr_id;
        this->msg_id_ = item->msg_id;
        size_t off = 0;
        return this->deserialize_in( item->data_buf_, item->data_buf_len_, off );
    }

    int po_message2::serialize_in( unsigned char* buffer, size_t len, size_t& off )
    {
        off = 0;

        buffer[off++] = this->type;
        buffer[off++] = this->ack;
        buffer[off++] = this->version;
        buffer[off++] = this->reserve;

        memcpy( buffer+off, this->data_buf_, this->data_buf_len_ );
        off += this->data_buf_len_;
        this->msg_buf_len_ = off;
        return 0;
    }

    int po_message2::deserialize_in( const unsigned char* buffer, size_t len, size_t& off )
    {
        off = 0;

        this->type = buffer[off++];
        this->ack = buffer[off++];
        this->version = buffer[off++];
        this->reserve = buffer[off++];

        this->data_buf_len_ = len - this->msg_head_len_;
        memcpy( this->data_buf_, buffer+off, this->data_buf_len_ );

        off += this->data_buf_len_;
        return 0;
    }

#if DEBUG_PROTOCOL_DATA == 1
    static void debug_print_protocol_data( pkg_t type, const unsigned char* buf, size_t len )
    {
        std::string strinfo;
        switch ( type )
        {
            case PO_MSG_REG_REQ: strinfo += "MSG_REG_REQ: "; break;
            case PO_MSG_REG_RPD: strinfo += "MSG_REG_RPD: "; break;
            case PO_MSG_REG_CFM: strinfo += "MSG_REG_CFM: "; break;
            case PO_MSG_HB_REQ: strinfo += "MSG_HB_REQ: "; break;
            case PO_MSG_HB_RPD: strinfo += "MSG_HB_RPD: "; break;
            default: strinfo += "Unknown type: "; break;
        }
        char psztemp[16] = {0};
        for ( size_t i = 0; i < len; ++i )
        {
            snprintf( psztemp, 16, "%02x ", buf[i] );
            strinfo += psztemp;
        }

        std::string name = "debug";
        std::string title = "[protocol]";
        HDTAS_LOG( LOG_INFO, name, title, strinfo );
    }
#endif //DEBUG_PROTOCOL_DATA

    int po_protocol_middle::serialize( po_message2* msg )
    {
        size_t off = 0;
        int rtn = this->serialize_in( msg->data_buf_,
                                   msg->data_buf_cap_,
                                   off );
        if ( 0 == rtn )
        {
            msg->data_buf_len_ = off;
            msg->msg_buf_len_ = msg->msg_head_len_ + off;

#if DEBUG_PROTOCOL_DATA == 1
            debug_print_protocol_data( msg->msg_type, msg->data_buf_, msg->data_buf_len_ );
#endif //DEBUG_PROTOCOL_DATA
        }

        return 0;
    }

    int po_protocol_middle::deserialize( const po_message2* msg )
    {
#if DEBUG_PROTOCOL_DATA == 1
        debug_print_protocol_data( msg->msg_type, msg->data_buf_, msg->data_buf_len_ );
#endif //DEBUG_PROTOCOL_DATA

        this->mmr_id_ = msg->mmr_id_;
        this->msg_id_ = msg->msg_id_;

        size_t off = 0;
        int rtn = this->deserialize_in( msg->data_buf_,
                                     msg->data_buf_len_,
                                     off );
        return rtn;
    }

    po_tcp_reg_req2::po_tcp_reg_req2() :
            po_protocol_middle(PO_MSG_REG_REQ),
            status(0)
    {}

    po_tcp_reg_req2::~po_tcp_reg_req2()
    {}

    int po_tcp_reg_req2::serialize_in( unsigned char* buffer, size_t len, size_t& off )
    {
        if ( len < DEFAULT_TCP_REG_REG_LEN2 )
        {
            this->strerr = "the buffer(";
            this->strerr += utility::uint64_2_str(len);
            this->strerr += ") is smaller the tcp_hb_req2(";
            this->strerr += utility::uint32_2_str(DEFAULT_TCP_REG_REG_LEN2);
            this->strerr += ")";
            return -1;
        }

        off = 0;

        h_uint16 tempu16 = this->status;
        off += utility::serialize_int( tempu16, buffer+off );
        return 0;
    }

    int po_tcp_reg_req2::deserialize_in( const unsigned char* buffer, size_t len, size_t& off )
    {
        if ( len < DEFAULT_TCP_REG_REG_LEN2 )
        {
            this->strerr = "the buffer(";
            this->strerr += utility::uint64_2_str(len);
            this->strerr += ") is smaller the tcp_reg_cfm(";
            this->strerr += utility::uint32_2_str(DEFAULT_TCP_REG_RPD_LEN2);
            this->strerr += ")";
            return -1;
        }

        off = 0;

        h_uint16 tempu16 = 0;
        off += utility::deserialize_int( buffer+off, tempu16 );
        this->status = tempu16;

        return 0;
    }



    po_tcp_reg_rpd2::po_tcp_reg_rpd2() :
            po_protocol_middle(PO_MSG_REG_RPD),
            error(0)
    {}

    po_tcp_reg_rpd2::~po_tcp_reg_rpd2()
    {}

    int po_tcp_reg_rpd2::serialize_in( unsigned char* buffer, size_t len, size_t& off )
    {
        if ( len < DEFAULT_TCP_REG_RPD_LEN2 )
        {
            this->strerr = "the buffer(";
            this->strerr += utility::uint64_2_str(len);
            this->strerr += ") is smaller the tcp_reg_rpd2(";
            this->strerr += utility::uint32_2_str(DEFAULT_TCP_REG_RPD_LEN2);
            this->strerr += ")";
            return -1;
        }

        off = 0;

        h_uint16 tempu16 = this->error;
        off += utility::serialize_int( tempu16, buffer+off );

        buffer[off++] = (unsigned char)this->date_time.y;
        buffer[off++] = (unsigned char)this->date_time.m;
        buffer[off++] = (unsigned char)this->date_time.d;
        buffer[off++] = (unsigned char)this->date_time.h;
        buffer[off++] = (unsigned char)this->date_time.n;
        buffer[off++] = (unsigned char)this->date_time.s;

        tempu16 = this->date_time.ms;
        off += utility::serialize_int( tempu16, buffer+off );

        return 0;
    }

    int po_tcp_reg_rpd2::deserialize_in( const unsigned char* buffer, size_t len, size_t& off )
    {
        if ( len < DEFAULT_TCP_REG_RPD_LEN2 )
        {
            this->strerr = "the buffer(";
            this->strerr += utility::uint64_2_str(len);
            this->strerr += ") is smaller the tcp_reg_cfm(";
            this->strerr += utility::uint32_2_str(DEFAULT_TCP_REG_RPD_LEN2);
            this->strerr += ")";
            return -1;
        }
        off = 0;

        h_uint16 tempu16 = 0;
        off += utility::deserialize_int( buffer+off, tempu16 );
        this->error = tempu16;

        this->date_time.y = buffer[off++];
        this->date_time.m = buffer[off++];
        this->date_time.d = buffer[off++];
        this->date_time.h = buffer[off++];
        this->date_time.n = buffer[off++];
        this->date_time.s = buffer[off++];

        tempu16 = 0;
        off += utility::deserialize_int( buffer+off, tempu16 );
        this->date_time.ms = tempu16;

        return 0;
    }

    po_tcp_reg_cfm2::po_tcp_reg_cfm2() :
            po_protocol_middle(PO_MSG_REG_CFM),
            error(0)
    {
    }

    po_tcp_reg_cfm2::~po_tcp_reg_cfm2()
    {

    }

    int po_tcp_reg_cfm2::serialize_in( unsigned char* buffer, size_t len, size_t& off )
    {
        if ( len < DEFAULT_TCP_REG_CFM_LEN2 )
        {
            this->strerr = "the buffer(";
            this->strerr += utility::uint64_2_str(len);
            this->strerr += ") is smaller the tcp_reg_cfm2(";
            this->strerr += utility::uint32_2_str(DEFAULT_TCP_REG_CFM_LEN2);
            this->strerr += ")";
            return -1;
        }

        off = 0;

        h_uint16 tempu16 = this->error;
        off += utility::serialize_int( tempu16, buffer+off );

        return 0;
    }

    int po_tcp_reg_cfm2::deserialize_in( const unsigned char* buffer, size_t len, size_t& off )
    {
        if ( len < DEFAULT_TCP_REG_CFM_LEN2 )
        {
            this->strerr = "the buffer(";
            this->strerr += utility::uint64_2_str(len);
            this->strerr += ") is smaller the tcp_reg_cfm(";
            this->strerr += utility::uint32_2_str(DEFAULT_TCP_REG_CFM_LEN2);
            this->strerr += ")";
            return -1;
        }
        off = 0;

        h_uint16 tempu16 = 0;
        off += utility::deserialize_int( buffer+off, tempu16 );
        this->error = tempu16;

        return 0;
    }

    po_tcp_hb_req2::po_tcp_hb_req2() :
            po_protocol_middle(PO_MSG_HB_REQ),
            status(0)
    {
    }

    po_tcp_hb_req2::~po_tcp_hb_req2()
    {}

    int po_tcp_hb_req2::serialize_in( unsigned char* buffer, size_t len, size_t& off )
    {
        if ( len < DEFAULT_TCP_HB_REQ_LEN2 )
        {
            this->strerr = "the buffer(";
            this->strerr += utility::uint64_2_str(len);
            this->strerr += ") is smaller than the tcp_hb_req2(";
            this->strerr += utility::uint32_2_str(DEFAULT_TCP_HB_REQ_LEN2);
            this->strerr += ")";
            return -1;
        }

        off = 0;

        h_uint16 tempu16 = this->status;
        off += utility::serialize_int( tempu16, buffer+off );

        return 0;
    }

    int po_tcp_hb_req2::deserialize_in( const unsigned char* buffer, size_t len, size_t& off )
    {
#if DEBUG_PRINT_RAW_MR_HEART_BEAT
        printf( "Tcp heart beat require raw data(%lu): ", len );
        for ( size_t i = 0; i < len; ++i )
        {
            printf( " %02X", buffer[i] );
        }
        printf( "\n" );
#endif
        if ( len < DEFAULT_TCP_HB_REQ_LEN2 )
        {
            this->strerr = "the buffer(";
            this->strerr += utility::uint64_2_str(len);
            this->strerr += ") is smaller than the tcp_hb_req2(";
            this->strerr += utility::uint32_2_str(DEFAULT_TCP_HB_REQ_LEN2);
            this->strerr += ")";
            return -1;
        }
        off = 0;

        h_uint16 tempu16 = 0;
        off += utility::deserialize_int( buffer+off, tempu16 );
        this->status = tempu16;

        return 0;
    }

    po_tcp_hb_rpd2::po_tcp_hb_rpd2() :
            po_protocol_middle(PO_MSG_HB_RPD),
            error(0)
    {

    }

    po_tcp_hb_rpd2::~po_tcp_hb_rpd2()
    {

    }

    int po_tcp_hb_rpd2::serialize_in( unsigned char* buffer, size_t len, size_t& off )
    {
        if ( len < DEFAULT_TCP_HB_RPD_LEN2 )
        {
            this->strerr = "the buffer(";
            this->strerr += utility::uint64_2_str(len);
            this->strerr += ") is smaller than the tcp_hb_rpd2(";
            this->strerr += utility::uint32_2_str(DEFAULT_TCP_HB_RPD_LEN2);
            this->strerr += ")";
            return -1;
        }

        off = 0;

        h_uint16 tempu16 = this->error;
        off += utility::serialize_int( tempu16, buffer+off );

#if DEBUG_PRINT_RAW_MR_HEART_BEAT
        printf( "Tcp heart beat response raw data(%lu): ", off );
        for ( size_t i = 0; i < off; ++i )
        {
            printf( " %02X", buffer[i] );
        }
        printf( "\n" );
#endif
        return 0;
    }

    int po_tcp_hb_rpd2::deserialize_in( const unsigned char* buffer, size_t len, size_t& off )
    {
        if ( len < DEFAULT_TCP_HB_RPD_LEN2 )
        {
            this->strerr = "the buffer(";
            this->strerr += utility::uint64_2_str(len);
            this->strerr += ") is smaller the tcp_hb_rpd2(";
            this->strerr += utility::uint32_2_str(DEFAULT_TCP_HB_RPD_LEN2);
            this->strerr += ")";
            return -1;
        }
        off = 0;

        h_uint16 tempu16 = 0;
        off += utility::deserialize_int( buffer+off, tempu16 );
        this->error = tempu16;

        return 0;
    }

    po_tcp_cfg_read_req::po_tcp_cfg_read_req() :
            po_protocol_middle(PO_MSG_CFG_RD_REQ)
    {

    }

    po_tcp_cfg_read_req::~po_tcp_cfg_read_req()
    {

    }

    int po_tcp_cfg_read_req::serialize_in( unsigned char* buffer, size_t len, size_t& off )
    {
        if ( len < DEFAULT_TCP_CFG_RD_REQ_LEN )
        {
            this->strerr = utility::gt_snprintf( "the buffer(%lu) is smaller than the po_tcp_cfg_read_req(%u)",
                                                 len, DEFAULT_TCP_CFG_RD_REQ_LEN );
            return -1;
        }

        off = 0;

        h_uint16 tempu16 = this->status;
        off += utility::serialize_int( tempu16, buffer+off );
        h_uint32 tempu32 = this->pair_id_;
        off += utility::serialize_int( tempu32, buffer+off );

#if DEBUG_PRINT_RAW_CONFIG_RD || DEBUG_PRINT_RAW_DATA
        printf( "TCP read config require message id: %u\n", tempu32 );
#endif

        tempu32 = this->des_mr_id_;
        off += utility::serialize_int( tempu32, buffer+off );

#if DEBUG_PRINT_RAW_CONFIG_RD || DEBUG_PRINT_RAW_DATA
        printf( "Tcp read config require raw data(%lu): ", off );
        for ( size_t i = 0; i < off; ++i )
        {
            printf( " %02X", buffer[i] );
        }
        printf( "\n" );
#endif
        return 0;
    }

    int po_tcp_cfg_read_req::deserialize_in( const unsigned char* buffer, size_t len, size_t& off )
    {
        return -1;
    }

    po_tcp_cfg_read_rpd::po_tcp_cfg_read_rpd() :
            po_protocol_middle(PO_MSG_CFG_RD_RPD)
    {
    }

    po_tcp_cfg_read_rpd::~po_tcp_cfg_read_rpd()
    {

    }

    int po_tcp_cfg_read_rpd::serialize_in( unsigned char* buffer, size_t len, size_t& off )
    {
        return -1;
    }

    int po_tcp_cfg_read_rpd::deserialize_in( const unsigned char* buffer, size_t len, size_t& off )
    {
#if DEBUG_PRINT_RAW_CONFIG_RD
        printf( "Tcp read config response raw data(%lu): ", len );
        for ( size_t i = 0; i < len; ++i )
        {
            printf( " %02X", buffer[i] );
        }
        printf( "\n" );
#endif
         if ( len < DEFAULT_TCP_CFG_RD_RPD_LEN+1 )
        {
            this->strerr = "the buffer(";
            this->strerr += utility::uint64_2_str(len);
            this->strerr += ") is smaller than the po_tcp_cfg_read_rpd(";
            this->strerr += utility::uint32_2_str(DEFAULT_TCP_CFG_RD_RPD_LEN+1);
            this->strerr += ")";
            return -1;
        }
        off = 0;

        h_uint16 tempu16 = 0;
        h_uint32 tempu32 = 0;

        tempu16 = 0;
        off += utility::deserialize_int( buffer+off, tempu16 );
        this->ecode = tempu16;

        tempu32 = 0;
        off += utility::deserialize_int( buffer+off, tempu32 );
        this->pair_id_ = tempu32;

#if DEBUG_PRINT_RAW_CONFIG_RD || DEBUG_PRINT_RAW_DATA
        printf( "TCP read config response message id: %u\n", tempu32 );
#endif

        tempu32 = 0;
        off += utility::deserialize_int( buffer+off, tempu32 );
        this->des_mr_id_ = tempu32;

        tempu32 = 0;
        off += utility::deserialize_int( buffer+off, tempu32 );
        this->static_ip_ = tempu32;

        tempu32 = 0;
        off += utility::deserialize_int( buffer+off, tempu32 );
        this->static_netmask_ = tempu32;

        tempu32 = 0;
        off += utility::deserialize_int( buffer+off, tempu32 );
        this->static_gateway_ = tempu32;

        tempu32 = 0;
        off += utility::deserialize_int( buffer+off, tempu32 );
        this->static_dns_ = tempu32;

        this->mac_addr_.resize( 12 );
        unsigned char mac[6] = { buffer[off++], buffer[off++], buffer[off++],
                                 buffer[off++], buffer[off++], buffer[off++] };
        snprintf( &(this->mac_addr_[0]), 12, "%02X%02X%02X%02X%02X%02X",
                  mac[0], mac[1], mac[2], mac[3], mac[4], mac[5] );

        tempu32 = 0;
        off += utility::deserialize_int( buffer+off, tempu32 );
        this->dhcp_ip_ = tempu32;

        tempu32 = 0;
        off += utility::deserialize_int( buffer+off, tempu32 );
        this->dhcp_netmask_ = tempu32;

        tempu32 = 0;
        off += utility::deserialize_int( buffer+off, tempu32 );
        this->dhcp_gateway_ = tempu32;

        tempu32 = 0;
        off += utility::deserialize_int( buffer+off, tempu32 );
        this->dhcp_dns_ = tempu32;

        tempu32 = 0;
        off += utility::deserialize_int( buffer+off, tempu32 );
        this->tcp_port_ = tempu32;

        tempu32 = 0;
        off += utility::deserialize_int( buffer+off, tempu32 );
        this->udp_port_ = tempu32;

        tempu32 = 0;
        off += utility::deserialize_int( buffer+off, tempu32 );
        this->version_ = tempu32;

        off += 64;

        unsigned char tempc = buffer[off++];
        if ( 0 == tempc ) this->network_type_ = po_network_type::pnt_line;
        else if ( 1 == tempc ) this->network_type_  = po_network_type::pnt_wifi;
        else this->network_type_ = po_network_type::pnt_unknown;

        tempc = buffer[off++];
        if ( 0 == tempc ) this->netmask_select_ = po_network_select::pns_auto;
        else if ( 1 == tempc ) this->netmask_select_ = po_network_select::pns_line;
        else if ( 2 == tempc ) this->netmask_select_ = po_network_select::pns_wifi;
        else this->netmask_select_ = po_network_select::pns_unknown;

        tempc = buffer[off++];
        if ( 0 == tempc ) this->wifi_mode_ = po_wifi_mode::pwm_auto;
        else if ( 1 == tempc ) this->wifi_mode_ = po_wifi_mode::pwm_manual;
        else this->wifi_mode_ = po_wifi_mode::pwm_unknown;

        tempc = buffer[off++];
        if ( 0 == tempc ) this->dhcp_mode_ = po_dhcp_mode::pdm_close;
        else if ( 1 == tempc ) this->dhcp_mode_ = po_dhcp_mode::pdm_open;
        else this->dhcp_mode_ = po_dhcp_mode::pdm_open;


        unsigned int datasize = len - off;
        this->domain_.resize( datasize+1 );
        for ( unsigned int i = 0; i < datasize; ++i )
        {
            this->domain_[i] = buffer[off++];
        }
        return 0;
    }

    po_tcp_cfg_write_req::po_tcp_cfg_write_req() :
            po_protocol_middle(PO_MSG_CFG_WT_REQ)
    {
    }

    po_tcp_cfg_write_req::~po_tcp_cfg_write_req()
    {

    }

    int po_tcp_cfg_write_req::serialize_in( unsigned char* buffer, size_t len, size_t& off )
    {
        if ( len < DEFAULT_TCP_CFG_WT_REQ_LEN+this->domain_.size() )
        {
            this->strerr = utility::gt_snprintf( "the buffer(%lu) is smaller than the po_tcp_cfg_write_req(%lu)",
                                                 len, DEFAULT_TCP_CFG_WT_REQ_LEN+this->domain_.size());
            return -1;
        }

        off = 0;

        h_uint16 tempu16 = 0;
        h_uint32 tempu32 = 0;

        tempu16 = this->status_;
        off += utility::serialize_int( tempu16, buffer+off );

        tempu32 = this->pair_id_;
        off += utility::serialize_int( tempu32, buffer+off );

#if DEBUG_PRINT_RAW_CONFIG_WT || DEBUG_PRINT_RAW_DATA
        printf( "TCP write config require message id: %u\n", tempu32 );
#endif

        tempu32 = this->des_mr_id_;
        off += utility::serialize_int( tempu32, buffer+off );

        tempu32 = this->static_ip_;
        const unsigned char* tempbyte = (const unsigned char*)(&tempu32);
        for ( unsigned int i  = 0; i < sizeof(tempu32); ++i )
        {
            buffer[off++] = tempbyte[i];
        }

        tempu32 = this->static_netmask_;
        tempbyte = (const unsigned char*)(&tempu32);
        for ( unsigned int i  = 0; i < sizeof(tempu32); ++i )
        {
            buffer[off++] = tempbyte[i];
        }

        tempu32 = this->static_gateway_;
        tempbyte = (const unsigned char*)(&tempu32);
        for ( unsigned int i  = 0; i < sizeof(tempu32); ++i )
        {
            buffer[off++] = tempbyte[i];
        }

        tempu32 = this->static_dns_;
        tempbyte = (const unsigned char*)(&tempu32);
        for ( unsigned int i  = 0; i < sizeof(tempu32); ++i )
        {
            buffer[off++] = tempbyte[i];
        }

        tempu32 = this->tcp_port_;
        off += utility::serialize_int( tempu32, buffer+off );

        tempu32 = this->udp_port_;
        off += utility::serialize_int( tempu32, buffer+off );

        off += 64;

        if ( po_network_select::pns_auto == this->netmask_select_ )
            buffer[off++] = 0;
        else if ( po_network_select::pns_line == this->netmask_select_ )
            buffer[off++] = 1;
        else if ( po_network_select::pns_wifi == this->netmask_select_ )
            buffer[off++] = 2;
        else
            buffer[off++] = 0;

        for ( size_t i = 0; i < this->domain_.size(); ++i )
        {
            buffer[off++] = this->domain_[i];
        }

#if DEBUG_PRINT_RAW_CONFIG_WT
        printf( "Tcp write config require raw data(%lu): ", off );
        for ( size_t i = 0; i < off; ++i )
        {
            printf( " %02X", buffer[i] );
        }
        printf( "\n" );
#endif
        return 0;
    }

    int po_tcp_cfg_write_req::deserialize_in( const unsigned char* buffer, size_t len, size_t& off )
    {
        return -1;
    }

    po_tcp_cfg_write_rpd::po_tcp_cfg_write_rpd() :
            po_protocol_middle(PO_MSG_CFG_WT_RPD)
    {

    }

    po_tcp_cfg_write_rpd::~po_tcp_cfg_write_rpd()
    {

    }

    int po_tcp_cfg_write_rpd::serialize_in( unsigned char* buffer, size_t len, size_t& off )
    {
        return -1;
    }

    int po_tcp_cfg_write_rpd::deserialize_in( const unsigned char* buffer, size_t len, size_t& off )
    {
#if DEBUG_PRINT_RAW_CONFIG_WT || DEBUG_PRINT_RAW_DATA
        printf( "Tcp write config response raw data(%lu): ", len );
        for ( size_t i = 0; i < len; ++i )
        {
            printf( " %02X", buffer[i] );
        }
        printf( "\n" );
#endif
        if ( len < DEFAULT_TCP_CFG_WT_RPD_LEN )
        {
            this->strerr = utility::gt_snprintf( "the buffer(%lu is smaller than the po_tcp_cfg_write_rpd(%u)",
                                                 len, DEFAULT_TCP_CFG_WT_RPD_LEN );
            return -1;
        }
        off = 0;

        h_uint16 tempu16 = 0;
        off += utility::deserialize_int( buffer+off, tempu16 );
        this->ecode = tempu16;

        h_uint32 tempu32 = 0;
        off += utility::deserialize_int( buffer+off, tempu32 );
        this->pair_id_ = tempu32;

#if DEBUG_PRINT_RAW_CONFIG_WT || DEBUG_PRINT_RAW_DATA
        printf( "TCP write config response message id: %u\n", tempu32 );
#endif

        tempu32 = 0;
        off += utility::deserialize_int( buffer+off, tempu32 );
        this->des_mr_id_ = tempu32;
        return 0;
    }

    po_tcp_ctl_req::po_tcp_ctl_req() :
            po_protocol_middle( PO_MSG_CONTROL_REQ )
    {
        this->data_len_ = 0;
        this->data_cap_ = 512;
        this->data = new unsigned char[this->data_cap_];
    }

    po_tcp_ctl_req::~po_tcp_ctl_req()
    {
        delete [] this->data;
        this->data = nullptr;
    }

    void po_tcp_ctl_req::set_mr_reset( device_mr_id id )
    {
        this->cmd = MMR_CFG_CMD_RESET;
        this->data_len_ = sizeof(id);
        h_uint32 tempu32 = id;
        utility::serialize_int( tempu32, this->data );
    }

    void po_tcp_ctl_req::set_mr_reboot( device_mr_id id )
    {
        this->cmd = MMR_CFG_CMD_REBOOT;
        this->data_len_ = sizeof(id);
        h_uint32 tempu32 = id;
        utility::serialize_int( tempu32, this->data );

    }

    void po_tcp_ctl_req::set_mr_status_report_once( )
    {
        this->cmd = MMR_CFG_CMD_MR_STATUS_ONCE;
        this->data_len_ = 0;
    }

    void po_tcp_ctl_req::set_wt_mr_status_report_interval( h_uint8 t )
    {
        this->cmd = MMR_CFG_CMD_MR_WT_STATUS_INTERVAL;
        this->data_len_ = sizeof(t);
        this->data[0] = t;
    }

    void po_tcp_ctl_req::set_rd_mr_status_report_interval()
    {
        this->cmd = MMR_CFG_CMD_MR_RD_STATUS_INTERVAL;
        this->data_len_ = 0;
    }

    void po_tcp_ctl_req::set_mr_update_begin( device_mr_id mid, const mr_update_info& info  )
    {
        this->cmd = MMR_CFG_CMD_MR_UPDATE_BEGIN;


        h_uint32 tempu32 = mid;
        int off = utility::serialize_int( tempu32, this->data );

        memcpy( this->data+off, info.name, LEN_MAX_MR_UPDATE_FILE_NAME );
        off += LEN_MAX_MR_UPDATE_FILE_NAME;

        h_uint8 t = 0;
        switch ( info.type )
        {
            case mr_update_type::MUT_SOFT: t = 0; break;
        }

        this->data[off++] = t;

        tempu32 = info.length;
        off += utility::serialize_int( tempu32, this->data+off );

        tempu32 = info.hash;
        off += utility::serialize_int( tempu32,  this->data+off );
        this->data_len_ = off;
    }

    void po_tcp_ctl_req::set_mr_update_data( device_mr_id mid, h_uint32 id, const unsigned char* data, h_uint32 len )
    {
        this->cmd = MMR_CFG_CMD_MR_UPDATE_DATA;

        h_uint32 tempu32 = mid;
        int off = utility::serialize_int( tempu32, this->data );

        tempu32 = id;
        off += utility::serialize_int( tempu32, this->data+off );

        memcpy( this->data+off, data, len );
        this->data_len_ = off + len;
    }

    void po_tcp_ctl_req::set_mr_update_finish( device_mr_id mid )
    {
        this->cmd = MMR_CFG_CMD_MR_UPDATE_FINISH;
        h_uint32 tempu32 = mid;
        this->data_len_ = utility::serialize_int( tempu32, this->data );
    }

    void po_tcp_ctl_req::set_rd_power_board_pre_status( device_mr_id id )
    {
        this->read_power_board( POWER_BOARD_ADDR_RD_PRE_STATUS, id );
    }

    void po_tcp_ctl_req::set_rd_power_board_cur_status( device_mr_id id )
    {
        this->read_power_board( POWER_BOARD_ADDR_RD_CUR_STATUS, id );
    }

    void po_tcp_ctl_req::set_rd_power_board_box_temperature( device_mr_id id )
    {
        this->read_power_board( POWER_BOARD_ADDR_RD_BOX_TEMPERATURE, id );
    }

    void po_tcp_ctl_req::set_rd_power_board_battery_voltage_capacity( device_mr_id id )
    {
        this->read_power_board( POWER_BOARD_ADDR_RD_BATTERY_VOLTAGE_CAPACITY, id );
    }

    void po_tcp_ctl_req::set_rd_power_board_camera_voltage_current( device_mr_id id )
    {
        this->read_power_board( POWER_BOARD_ADDR_RD_CAMERA_VOLTAGE_CURRENT, id );
    }

    void po_tcp_ctl_req::set_rd_power_board_locate_voltage_current( device_mr_id id )
    {
        this->read_power_board( POWER_BOARD_ADDR_RD_LOCATE_VOLTAGE_CURRENT, id );
    }

    void po_tcp_ctl_req::set_rd_power_board_router_voltage_current( device_mr_id id )
    {
        this->read_power_board( POWER_BOARD_ADDR_RD_ROUTER_VOLTAGE_CURRENT, id );
    }

    void po_tcp_ctl_req::set_rd_power_board_switch_voltage_current( device_mr_id id )
    {
        this->read_power_board( POWER_BOARD_ADDR_RD_SWITCH_VOLTAGE_CURRENT, id );
    }

    void po_tcp_ctl_req::set_rd_power_board_reload_voltage_current( device_mr_id id )
    {
        this->read_power_board( POWER_BOARD_ADDR_RD_RELOAD_VOLTAGE_CURRENT, id );
    }

    void po_tcp_ctl_req::set_rd_power_board_auto_report( device_mr_id id )
    {
        this->read_power_board( POWER_BOARD_ADDR_RDWT_AUTO_REPORT, id );
    }

    void po_tcp_ctl_req::set_wt_power_board_auto_report( device_mr_id id, bool open )
    {
        unsigned char v = open ? POWER_BOARD_AUTO_REPORT_OPEN : POWER_BOARD_AUTO_REPORT_CLOSE;
        this->write_power_board( POWER_BOARD_ADDR_RDWT_AUTO_REPORT, v, id );
    }

    void po_tcp_ctl_req::set_rd_power_board_report_interval( device_mr_id id )
    {
        this->read_power_board( POWER_BOARD_ADDR_RDWT_REPORT_INTERVAL, id );
    }

    void po_tcp_ctl_req::set_wt_power_board_report_interval( device_mr_id id, unsigned char v )
    {
        this->write_power_board( POWER_BOARD_ADDR_RDWT_REPORT_INTERVAL, v, id );
    }

    void po_tcp_ctl_req::set_wt_power_boart_open_close_mic_cam( device_mr_id id, bool mo, bool co, bool po, bool bo )
    {
        h_uint8 value = 0;
        if ( mo ) value |= POWER_BOARD_STATUS_MICROPHONE;
        if ( co ) value |= POWER_BOARD_STATUS_CAMERA;
        if ( po ) value |= POWER_BOARD_STATUS_PRELOAD;
        if ( bo ) value |= POWER_BOARD_STATUS_BATTERY;
        this->write_power_board( POWER_BOARD_ADDR_WT_OPEN_CLOSE, value, id );
    }

    void po_tcp_ctl_req::set_wt_power_board_open( device_mr_id id )
    {
        this->write_power_board( POWER_BOARD_ADDR_WT_OPEN_CLOSE,
                                 POWER_BOARD_MICROPHONE_OPEN |
                                 POWER_BOARD_CAMERA_OPEN |
                                 POWER_BOARD_PRELOAD_OPEN, id );
    }

    void po_tcp_ctl_req::set_wt_power_board_close( device_mr_id id )
    {
        this->write_power_board( POWER_BOARD_ADDR_WT_OPEN_CLOSE,
                                 POWER_BOARD_MICROPHONE_CLOSE &
                                 POWER_BOARD_CAMERA_CLOSE &
                                 POWER_BOARD_PRELOAD_CLOSE, id );
    }

    void po_tcp_ctl_req::set_wt_power_board_reboot( device_mr_id id )
    {
        this->write_power_board( POWER_BOARD_ADDR_WT_REBOOT, 0X7F, id );
    }

    void po_tcp_ctl_req::set_wt_power_board_reset( device_mr_id id )
    {
        this->write_power_board( POWER_BOARD_ADDR_WT_RESET, 0X73, id );
    }

    void po_tcp_ctl_req::set_rd_power_board_shortcut_cmd( device_mr_id id )
    {
        this->read_power_board( POWER_BOARD_ADDR_RD_SHORTCUT_CMD, id );
    }

    void po_tcp_ctl_req::set_rd_power_board_soft_version( device_mr_id id )
    {
        this->read_power_board( POWER_BOARD_ADDR_RD_SOFT_VERSION, id );
    }

    void po_tcp_ctl_req::set_rd_power_board_hard_version( device_mr_id id )
    {
        this->read_power_board( POWER_BOARD_ADDR_RD_HARD_VERSION, id );
    }


    void po_tcp_ctl_req::read_power_board( unsigned char addr, device_mr_id id )
    {
        this->cmd = MMR_CFG_CMD_MR_POWER_BOARD;
        h_uint32 tempu32 = id;
        this->data_len_ = utility::serialize_int( tempu32, this->data );
        this->data[(this->data_len_)++] = POWER_BOARD_HEADER;
        this->data[(this->data_len_)++] = addr;
        this->data[(this->data_len_)++] = POWER_BOARD_CTL_RD;
        h_uint16 cs = CRC16_2( this->data+sizeof(tempu32), this->data_len_-sizeof(tempu32) );
        this->data_len_ += utility::serialize_int( cs, this->data+this->data_len_ );
        this->data[(this->data_len_)++] = POWER_BOARD_TAILER;
    }

    void po_tcp_ctl_req::write_power_board( unsigned char addr, h_uint8 value, device_mr_id id )
    {
        this->cmd = MMR_CFG_CMD_MR_POWER_BOARD;
        h_uint32 tempu32 = id;
        this->data_len_ = utility::serialize_int( tempu32, this->data );
        this->data[(this->data_len_)++] = POWER_BOARD_HEADER;
        this->data[(this->data_len_)++] = addr;
        this->data[(this->data_len_)++] = POWER_BOARD_CTL_WT;
        this->data[(this->data_len_)++] = sizeof(value);
        this->data[(this->data_len_)++] = value;
        h_uint16 cs = CRC16_2( this->data+sizeof(tempu32), this->data_len_-sizeof(tempu32) );
        this->data_len_ += utility::serialize_int( cs, this->data+this->data_len_ );
        this->data[(this->data_len_)++] = POWER_BOARD_TAILER;
    }

    int po_tcp_ctl_req::serialize_in( unsigned char* buffer, size_t len, size_t& off )
    {
        if ( len < DEFAULT_TCP_CFG_RD_REQ_LEN )
        {
            this->strerr = utility::gt_snprintf( "the buffer(%lu) is smaller than the po_tcp_cfg_read_req(%u)",
                                                 len, DEFAULT_TCP_CFG_RD_REQ_LEN );
            return -1;
        }

        off = 0;

        h_uint16 tempu16 = this->status;
        off += utility::serialize_int( tempu16, buffer+off );

        h_uint32 tempu32 = this->pair_id_;
        off += utility::serialize_int( tempu32, buffer+off );

#if DEBUG_PRINT_RAW_CONTROL || DEBUG_PRINT_RAW_DATA
        printf( "TCP control request message id: %u\n", tempu32 );
#endif

        tempu32 = this->cmd;
        off += utility::serialize_int( tempu32, buffer+off );

        memcpy( buffer+off, this->data, this->data_len_ );
        off += this->data_len_;

#if DEBUG_PRINT_RAW_CONTROL || DEBUG_PRINT_RAW_DATA
        char psztemp[64] = {0};
        struct tm ctm;
        time_t ctime = time(NULL);
        localtime_r( &ctime, &ctm );
        snprintf( psztemp, sizeof(psztemp), "%d-%d-%d %d:%d:%d",
                  ctm.tm_year, ctm.tm_mon, ctm.tm_mday, ctm.tm_hour, ctm.tm_min, ctm.tm_sec );
        printf( "%s TCP control request raw data(%lu):", psztemp, off );
        for ( size_t i = 0; i < off; ++i )
        {
            printf( " %02x", buffer[i] );
        }
        printf( "\n" );
#endif
        return 0;
    }

    int po_tcp_ctl_req::deserialize_in( const unsigned char* buffer, size_t len, size_t& off )
    {
        return -1;
    }

    po_tcp_ctl_rpd::po_tcp_ctl_rpd() :
            po_protocol_middle( PO_MSG_CONTROL_RPD )
    {}

    po_tcp_ctl_rpd::~po_tcp_ctl_rpd()
    {}

    int po_tcp_ctl_rpd::serialize_in( unsigned char* buffer, size_t len, size_t& off )
    {
        return -1;
    }

    int po_tcp_ctl_rpd::deserialize_in( const unsigned char* buffer, size_t len, size_t& off )
    {
#if DEBUG_PRINT_RAW_CONTROL || DEBUG_PRINT_RAW_DATA
        char psztemp[64] = {0};
        struct tm ctm;
        time_t ctime = time(NULL);
        localtime_r( &ctime, &ctm );
        snprintf( psztemp, sizeof(psztemp), "%d-%d-%d %d:%d:%d",
                  ctm.tm_year, ctm.tm_mon, ctm.tm_mday, ctm.tm_hour, ctm.tm_min, ctm.tm_sec );
        printf( "%s TCP control response raw data(%lu):", psztemp, len );
        for ( size_t i = 0; i < len; ++i )
        {
            printf( " %02x", buffer[i] );
        }
        printf( "\n" );
#endif

        if ( len < DEFAULT_TCP_CTL_RPD_STATIC_LEN )
        {
            this->strerr = utility::gt_snprintf( "the buffer(%lu) is smaller than the static po_tcp_cfg_read_req(%u)",
                                                 len, DEFAULT_TCP_CTL_RPD_STATIC_LEN );
            return -1;
        }

        off = 0;
        h_uint16 tempu16 = 0;
        off += utility::deserialize_int( buffer+off, tempu16 );
        this->ecode_ = tempu16;

        h_uint32 tempu32 = 0;
        off += utility::deserialize_int( buffer+off, tempu32 );
        this->pair_id_ = tempu32;

#if DEBUG_PRINT_RAW_CONTROL || DEBUG_PRINT_RAW_DATA
        printf( "TCP control response message id: %u\n", tempu32 );
#endif

        tempu32 = 0;
        off += utility::deserialize_int( buffer+off, tempu32 );
        this->cmd_ = tempu32;

        if ( 0 != this->ecode_ )
            return 0;

        switch ( this->cmd_ )
        {
            case MMR_CFG_CMD_RESET:
            {
                if ( len < DEFAULT_TCP_CTL_RESET_REQ_LEN )
                {
                    this->strerr = utility::gt_snprintf( "the buffer(%lu) is smaller than the reset po_tcp_ctl_rpd(%lu)",
                                                         len, DEFAULT_TCP_CTL_RESET_REQ_LEN );
                    return -1;
                }
                h_uint32  tempu32 = 0;
                off += utility::deserialize_int( buffer+off, tempu32 );
                this->mr_id_ = tempu32;
            }
            case MMR_CFG_CMD_REBOOT:
            {
                if ( len < DEFAULT_TCP_CTL_REBOOT_RPD_LEN )
                {
                    this->strerr = "the buffer(";
                    this->strerr += utility::uint64_2_str(len);
                    this->strerr += ") is smaller than the reboot po_tcp_ctl_rpd(";
                    this->strerr += utility::uint32_2_str(DEFAULT_TCP_CTL_REBOOT_RPD_LEN);
                    this->strerr += ")";
                    return -1;
                }
                h_uint32 tempu32 = 0;
                off += utility::deserialize_int( buffer+off, tempu32 );
                this->mr_id_ = tempu32;
                break;
            }
            case MMR_CFG_CMD_MR_STATUS_ONCE:
            {
                if ( len < DEFAULT_TCP_CTL_STATUS_STATIC_RPD_LEN )
                {
                    this->strerr = "the buffer(";
                    this->strerr += utility::uint64_2_str(len);
                    this->strerr += ") is smaller than the static status po_tcp_ctl_rpd(";
                    this->strerr += utility::uint32_2_str(DEFAULT_TCP_CTL_STATUS_STATIC_RPD_LEN);
                    this->strerr += ")";
                    return -1;
                }
                unsigned char ncount = buffer[off++];
                size_t length = DEFAULT_TCP_CTL_STATUS_STATIC_RPD_LEN + ncount*DEFAULT_TCP_CTL_STATUS_DYNAMIC_RPD_LEN;
                if ( len < length )
                {
                    this->strerr = "the buffer(";
                    this->strerr += utility::uint64_2_str(len);
                    this->strerr += ") is smaller than the total status po_tcp_ctl_rpd(";
                    this->strerr += utility::uint64_2_str(length);
                    this->strerr += ")";
                    return -1;
                }
                h_uint16 tempu16 = 0;
                h_uint32 tempu32 = 0;
                this->status_array_.clear();
                for ( unsigned int i = 0; i < ncount; ++i )
                {
                    off += utility::deserialize_int( buffer+off, tempu32 );
                    off += utility::deserialize_int( buffer+off, tempu16 );
                    this->status_array_.push_back( mr_status_stu(tempu32, tempu16) );
                }
                break;
            }
            case MMR_CFG_CMD_MR_WT_STATUS_INTERVAL:
            {
                if ( len < DEFAULT_TCP_CTL_MR_AUTO_REPORT_WT_RPD_LEN )
                {
                    this->strerr = "the buffer(";
                    this->strerr += utility::uint64_2_str(len);
                    this->strerr += ") is smaller than the mr auto report write po_tcp_ctl_rpd(";
                    this->strerr += utility::uint32_2_str(DEFAULT_TCP_CTL_MR_AUTO_REPORT_WT_RPD_LEN);
                    this->strerr += ")";
                    return -1;
                }
                this->mr_report_interval_ = buffer[off++];
                break;
            }
            case MMR_CFG_CMD_MR_RD_STATUS_INTERVAL:
            {
                if ( len < DEFAULT_TCP_CTL_MR_AUTO_REPORT_RD_RPD_LEN )
                {
                    this->strerr = "the buffer(";
                    this->strerr += utility::uint64_2_str(len);
                    this->strerr += ") is smaller than the mr auto report read po_tcp_ctl_rpd(";
                    this->strerr += utility::uint32_2_str(DEFAULT_TCP_CTL_MR_AUTO_REPORT_RD_RPD_LEN);
                    this->strerr += ")";
                    return -1;
                }
                this->mr_report_interval_ = buffer[off++];
                break;
            }
            case MMR_CFG_CMD_MR_UPDATE_BEGIN:
            {
                if ( len < DEFAULT_TCP_CTL_MR_UPDATE_START_RPD )
                {
                    this->strerr = utility::gt_snprintf( "the buffer(%lu) is smaller than "
                                                         "the mr update start response(%lu)",
                                                         len-10, DEFAULT_TCP_CTL_MR_UPDATE_START_RPD );
                    return -1;
                }
                h_uint32 tempu32 = 0;
                off += utility::deserialize_int( buffer+off, tempu32 );
                this->mr_update_id_ = tempu32;
                break;
            }
            case MMR_CFG_CMD_MR_UPDATE_DATA:
            {
                if ( len < DEFAULT_TCP_CTL_MR_UPDATE_DATA_RPD )
                {
                    this->strerr = utility::gt_snprintf( "the buffer(%lu) is smaller than "
                                                         "the mr update data response(%lu)",
                                                         len-10, DEFAULT_TCP_CTL_MR_UPDATE_DATA_RPD );
                    return -1;
                }
                h_uint32 tempu32 = 0;
                off += utility::deserialize_int( buffer+off, tempu32 );
                this->mr_update_id_ = tempu32;

                tempu32 = 0;
                off += utility::deserialize_int( buffer+off, tempu32 );
                this->mr_update_msg_id_ = tempu32;
                break;
            }
            case MMR_CFG_CMD_MR_UPDATE_FINISH:
            {
                if ( len < DEFAULT_TCP_CTL_MR_UPDATE_FINISH_RPD )
                {
                    this->strerr = utility::gt_snprintf( "the buffer(%lu) is samller than "
                                                         "the mr update data response(%lu)",
                                                         len-10, DEFAULT_TCP_CTL_MR_UPDATE_FINISH_RPD );
                    return -1;
                }
                h_uint32 tempu32 = 0;
                off += utility::deserialize_int( buffer+off, tempu32 );
                this->mr_update_id_ = tempu32;
                break;
            }
            case MMR_CFG_CMD_MR_POWER_BOARD:
            {
                size_t resoff = 0;
                int rtn = this->deserialize_pb( buffer+off, len-off, resoff );
                off += resoff;
                return rtn;
            }
            default:
            {
                this->strerr = "unknown control command(";
                this->strerr += utility::uint32_2_str( this->cmd_ );
                this->strerr += ")";
                off = len;
                break;
            }
        }
        return 0;
    }

    int po_tcp_ctl_rpd::deserialize_pb( const unsigned char* buffer, size_t len, size_t& off )
    {
        if ( len < DEFAULT_TCP_CTL_PB_STATIC_RPD_LEN )
        {
            this->strerr = "the buffer(";
            this->strerr += utility::uint64_2_str(len);
            this->strerr += ") is smaller than the power board static data(";
            this->strerr += utility::uint32_2_str(DEFAULT_TCP_CTL_PB_STATIC_RPD_LEN);
            this->strerr += ")";
            return -1;
        }
        h_uint32 tempu32 = 0;
        off = utility::deserialize_int( buffer, tempu32 );
        this->mr_id_ = tempu32;
        off++; // For: h_uint8 header = buffer[off++];

        h_uint16 tempu16 = 0;
        h_uint16 check_sum = CRC16_2( buffer+4, len-4-3 );
        utility::deserialize_int( buffer+len-3, tempu16 );
        if ( tempu16 != check_sum )
        {
            this->strerr = "power board data frame check sum is bad";
            return -1;
        }

        this->addr_ = buffer[off++];
        this->pb_rd_wt_mode_ = buffer[off++];
        unsigned char length = buffer[off++];
        if ( POWER_BOARD_CTL_WT == this->pb_rd_wt_mode_ )
        {
            unsigned char bsuc = buffer[off++]; // Success: 0X06, Failure: 0X15
            this->pb_ecode_ = (bsuc == POWER_BOARD_CTL_WT_SUCCESS ? H_STATUS_SUCCESS : H_STATUS_FAILURE);
        }
        else if ( POWER_BOARD_CTL_RD == this->pb_rd_wt_mode_ )
        {
            switch (  this->addr_ )
            {
                case POWER_BOARD_ADDR_RD_PRE_STATUS:
                case POWER_BOARD_ADDR_RD_CUR_STATUS:
                {
                    if ( length < DEFAULT_TCP_CTL_PB_STATUS_RPD_LEN )
                    {
                        this->strerr = "the buffer(";
                        this->strerr += utility::uint64_2_str(len);
                        this->strerr += ") is smaller than the power board status data(";
                        this->strerr += utility::uint32_2_str(DEFAULT_TCP_CTL_PB_STATUS_RPD_LEN);
                        this->strerr += ")";
                        return -1;
                    }
                    auto byte_2_struct = [](unsigned char f, unsigned char s, power_board_status &data) -> void
                    {
                        data.switch_ = ((f & POWER_BOARD_STATUS_SWITCH) == POWER_BOARD_STATUS_SWITCH) ? true : false;
                        data.router_ = ((f & POWER_BOARD_STATUS_ROUTER) == POWER_BOARD_STATUS_ROUTER) ? true : false;
                        data.locate_ = ((f & POWER_BOARD_STATUS_LOCATE) == POWER_BOARD_STATUS_LOCATE) ? true : false;
                        data.microphone_ = ((f & POWER_BOARD_STATUS_MICROPHONE) == POWER_BOARD_STATUS_MICROPHONE) ? true : false;
                        data.camera_ = ((f & POWER_BOARD_STATUS_CAMERA) == POWER_BOARD_STATUS_CAMERA) ? true : false;
                        data.preload_ = ((f & POWER_BOARD_STATUS_PRELOAD) == POWER_BOARD_STATUS_PRELOAD) ? true : false;

                        data.source_ = ((s & POWER_BOARD_STATUS_SOURCE) == POWER_BOARD_STATUS_SOURCE) ? true : false;
                        data.battery_charge_ =  ((s & POWER_BOARD_STATUS_BATTERY_CHARGE) == POWER_BOARD_STATUS_BATTERY_CHARGE) ? true : false;
                        data.battery_status_ = ((s & POWER_BOARD_STATUS_BATTERY_STATUS) == POWER_BOARD_STATUS_BATTERY_STATUS) ? true : false;
                        data.internet_ = ((s & POWER_BOARD_STATUS_INTERNET) == POWER_BOARD_STATUS_INTERNET) ? true : false;
                        data.battery_connect_ = ((s & POWER_BOARD_STATUS_BATTERY_CONNECT) == POWER_BOARD_STATUS_BATTERY_CONNECT) ? true : false;
                        data.battery_control_ = ((s & POWER_BOARD_STATUS_BATTERY_COLTROL) == POWER_BOARD_STATUS_BATTERY_COLTROL) ? true : false;
                    };


                    unsigned first = buffer[off++];
                    unsigned second = buffer[off++];
                    if (POWER_BOARD_ADDR_RD_PRE_STATUS == this->type_)
                        byte_2_struct(first, second, this->pre_status_);
                    else
                        byte_2_struct(first, second, this->cur_status_);
                    break;
                }
                case POWER_BOARD_ADDR_RD_BOX_TEMPERATURE:
                {
                    if ( length < DEFAULT_TCP_CTL_PB_TEMPERATURE_RPD_LEN )
                    {
                        this->strerr = "the buffer(";
                        this->strerr += utility::uint64_2_str(len);
                        this->strerr += ") is smaller than the power board temperature data(";
                        this->strerr += utility::uint32_2_str(DEFAULT_TCP_CTL_PB_TEMPERATURE_RPD_LEN);
                        this->strerr += ")";
                        return -1;
                    }
                    h_int16 temp16 = 0;
                    off += utility::deserialize_int( buffer+off, temp16 );
                    this->status_gropu_.temperature_ = temp16;
                    break;
                }
                case POWER_BOARD_ADDR_RD_BATTERY_VOLTAGE_CAPACITY:
                {
                    if ( length < DEFAULT_TCP_CTL_PB_BATTERY_RPD_LEN )
                    {
                        this->strerr = "the buffer(";
                        this->strerr += utility::uint64_2_str(len);
                        this->strerr += ") is smaller than the power board battery data(";
                        this->strerr += utility::uint32_2_str(DEFAULT_TCP_CTL_PB_BATTERY_RPD_LEN);
                        this->strerr += ")";
                        return -1;
                    }
                    h_uint16 tempu16 = 0;
                    off += utility::deserialize_int( buffer+off, tempu16 );
                    this->status_gropu_.battery_.first = tempu16;
                    this->status_gropu_.battery_.second = buffer[off++];
                    break;
                }
                case POWER_BOARD_ADDR_RD_CAMERA_VOLTAGE_CURRENT:
                {
                    if ( length < DEFAULT_TCP_CTL_PB_CAMERA_RPD_LEN )
                    {
                        this->strerr = "the buffer(";
                        this->strerr += utility::uint64_2_str(len);
                        this->strerr += ") is smaller than the power board camera data(";
                        this->strerr += utility::uint32_2_str(DEFAULT_TCP_CTL_PB_CAMERA_RPD_LEN);
                        this->strerr += ")";
                        return -1;
                    }
                    h_uint16 tempu16 = 0;
                    off += utility::deserialize_int( buffer+off, tempu16 );
                    this->status_gropu_.camera_.first = tempu16;
                    tempu16 = 0;
                    off += utility::deserialize_int( buffer+off, tempu16 );
                    this->status_gropu_.camera_.second = tempu16;
                    break;
                }
                case POWER_BOARD_ADDR_RD_LOCATE_VOLTAGE_CURRENT:
                {
                    if ( length < DEFAULT_TCP_CTL_PB_LOCATE_RPD_LEN )
                    {
                        this->strerr = "the buffer(";
                        this->strerr += utility::uint64_2_str(len);
                        this->strerr += ") is smaller than the power board locate data(";
                        this->strerr += utility::uint32_2_str(DEFAULT_TCP_CTL_PB_LOCATE_RPD_LEN);
                        this->strerr += ")";
                        return -1;
                    }
                    h_uint16 tempu16 = 0;
                    off += utility::deserialize_int( buffer+off, tempu16 );
                    this->status_gropu_.locate_.first = tempu16;
                    tempu16 = 0;
                    off += utility::deserialize_int( buffer+off, tempu16 );
                    this->status_gropu_.locate_.second = tempu16;
                    break;
                }
                case POWER_BOARD_ADDR_RD_ROUTER_VOLTAGE_CURRENT:
                {
                    if ( length < DEFAULT_TCP_CTL_PB_ROUTER_RPD_LEN )
                    {
                        this->strerr = "the buffer(";
                        this->strerr += utility::uint64_2_str(len);
                        this->strerr += ") is smaller than the power board router data(";
                        this->strerr += utility::uint32_2_str(DEFAULT_TCP_CTL_PB_ROUTER_RPD_LEN);
                        this->strerr += ")";
                        return -1;
                    }
                    h_uint16 tempu16 = 0;
                    off += utility::deserialize_int( buffer+off, tempu16 );
                    this->status_gropu_.router_.first = tempu16;
                    tempu16 = 0;
                    off += utility::deserialize_int( buffer+off, tempu16 );
                    this->status_gropu_.router_.second = tempu16;
                    break;
                }
                case POWER_BOARD_ADDR_RD_SWITCH_VOLTAGE_CURRENT:
                {
                    if ( length < DEFAULT_TCP_CTL_PB_SWITCH_RPD_LEN )
                    {
                        this->strerr = "the buffer(";
                        this->strerr += utility::uint64_2_str(len);
                        this->strerr += ") is smaller than the power board switch data(";
                        this->strerr += utility::uint32_2_str(DEFAULT_TCP_CTL_PB_SWITCH_RPD_LEN);
                        this->strerr += ")";
                        return -1;
                    }
                    h_uint16 tempu16 = 0;
                    off += utility::deserialize_int( buffer+off, tempu16 );
                    this->status_gropu_.switch_.first = tempu16;
                    tempu16 = 0;
                    off += utility::deserialize_int( buffer+off, tempu16 );
                    this->status_gropu_.switch_.second = tempu16;
                    break;
                }
                case POWER_BOARD_ADDR_RD_RELOAD_VOLTAGE_CURRENT:
                {
                    if ( length < DEFAULT_TCP_CTL_PB_PRELOAD_RPD_LEN )
                    {
                        this->strerr = "the buffer(";
                        this->strerr += utility::uint64_2_str(len);
                        this->strerr += ") is smaller than the power board preload data(";
                        this->strerr += utility::uint32_2_str(DEFAULT_TCP_CTL_PB_PRELOAD_RPD_LEN);
                        this->strerr += ")";
                        return -1;
                    }
                    h_uint16 tempu16 = 0;
                    off += utility::deserialize_int( buffer+off, tempu16 );
                    this->status_gropu_.preload_.first = tempu16;
                    tempu16 = 0;
                    off += utility::deserialize_int( buffer+off, tempu16 );
                    this->status_gropu_.preload_.second = tempu16;
                    break;
                }
                case POWER_BOARD_ADDR_RDWT_AUTO_REPORT:
                {
                    if ( length < DEFAULT_TCP_CTL_PB_AUTO_REPORT_RPD_LEN )
                    {
                        this->strerr = "the buffer(";
                        this->strerr += utility::uint64_2_str(len);
                        this->strerr += ") is smaller than the power board auto report data(";
                        this->strerr += utility::uint32_2_str(DEFAULT_TCP_CTL_PB_AUTO_REPORT_RPD_LEN);
                        this->strerr += ")";
                        return -1;
                    }
                    h_uint8 u8 = buffer[off++];
                    if ( POWER_BOARD_AUTO_REPORT_OPEN == u8 ) u8 = H_STATUS_OPEN;
                    else if ( POWER_BOARD_AUTO_REPORT_CLOSE == u8 ) u8 = H_STATUS_CLOSE;
                    else u8 = H_STATUS_UNKNOWN;
                    this->status_gropu_.auto_report_ = u8;
                    break;
                }
                case POWER_BOARD_ADDR_RDWT_REPORT_INTERVAL:
                {
                    if ( length < DEFAULT_TCP_CTL_PB_AUTO_REPORT_INTERVAL_RPD_LEN )
                    {
                        this->strerr = "the buffer(";
                        this->strerr += utility::uint64_2_str(len);
                        this->strerr += ") is smaller than the power board auto report interval data(";
                        this->strerr += utility::uint32_2_str(DEFAULT_TCP_CTL_PB_AUTO_REPORT_INTERVAL_RPD_LEN);
                        this->strerr += ")";
                        return -1;
                    }
                    this->status_gropu_.auto_report_interval_ = buffer[off++];
                    break;
                }
                case POWER_BOARD_ADDR_RD_SHORTCUT_CMD:
                {
                    if ( length < DEFAULT_TCP_CTL_PB_SHORTCUT_RPD_LEN )
                    {
                        this->strerr = "the buffer(";
                        this->strerr += utility::uint64_2_str(len);
                        this->strerr += ") is smaller than the power board shortcut data(";
                        this->strerr += utility::uint32_2_str(DEFAULT_TCP_CTL_PB_SHORTCUT_RPD_LEN);
                        this->strerr += ")";
                        return -1;
                    }
                    unsigned first = buffer[off++];
                    unsigned second = buffer[off++];
                    this->cur_status_.switch_ = ((first & POWER_BOARD_STATUS_SWITCH) == POWER_BOARD_STATUS_SWITCH) ? true : false;
                    this->cur_status_.router_ = ((first & POWER_BOARD_STATUS_ROUTER) == POWER_BOARD_STATUS_ROUTER) ? true : false;
                    this->cur_status_.locate_ = ((first & POWER_BOARD_STATUS_LOCATE) == POWER_BOARD_STATUS_LOCATE) ? true : false;
                    this->cur_status_.microphone_ = ((first & POWER_BOARD_STATUS_MICROPHONE) == POWER_BOARD_STATUS_MICROPHONE) ? true : false;
                    this->cur_status_.camera_ = ((first & POWER_BOARD_STATUS_CAMERA) == POWER_BOARD_STATUS_CAMERA) ? true : false;
                    this->cur_status_.preload_ = ((first & POWER_BOARD_STATUS_PRELOAD) == POWER_BOARD_STATUS_PRELOAD) ? true : false;

                    this->cur_status_.source_ = ((second & POWER_BOARD_STATUS_SOURCE) == POWER_BOARD_STATUS_SOURCE) ? true : false;
                    this->cur_status_.battery_charge_ =  ((second & POWER_BOARD_STATUS_BATTERY_CHARGE) == POWER_BOARD_STATUS_BATTERY_CHARGE) ? true : false;
                    this->cur_status_.battery_status_ = ((second & POWER_BOARD_STATUS_BATTERY_STATUS) == POWER_BOARD_STATUS_BATTERY_STATUS) ? true : false;
                    this->cur_status_.internet_ = ((second & POWER_BOARD_STATUS_INTERNET) == POWER_BOARD_STATUS_INTERNET) ? true : false;
                    this->cur_status_.battery_connect_ = ((second & POWER_BOARD_STATUS_BATTERY_CONNECT) == POWER_BOARD_STATUS_BATTERY_CONNECT) ? true : false;
                    this->cur_status_.battery_control_ = ((second & POWER_BOARD_STATUS_BATTERY_COLTROL) == POWER_BOARD_STATUS_BATTERY_COLTROL) ? true : false;

                    h_int16 temp16 = 0;
                    off += utility::deserialize_int( buffer+off, temp16 );
                    this->status_gropu_.temperature_ = temp16;
                    h_uint16 tempu16 = 0;
                    off += utility::deserialize_int( buffer+off, tempu16 );
                    this->status_gropu_.battery_.first = tempu16;
                    this->status_gropu_.battery_.second = buffer[off++];
                    tempu16 = 0;
                    off += utility::deserialize_int( buffer+off, tempu16 );
                    this->status_gropu_.camera_.first = tempu16;
                    tempu16 = 0;
                    off += utility::deserialize_int( buffer+off, tempu16 );
                    this->status_gropu_.camera_.second = tempu16;
                    tempu16 = 0;
                    off += utility::deserialize_int( buffer+off, tempu16 );
                    this->status_gropu_.locate_.first = tempu16;
                    tempu16 = 0;
                    off += utility::deserialize_int( buffer+off, tempu16 );
                    this->status_gropu_.locate_.second = tempu16;
                    tempu16 = 0;
                    off += utility::deserialize_int( buffer+off, tempu16 );
                    this->status_gropu_.router_.first = tempu16;
                    tempu16 = 0;
                    off += utility::deserialize_int( buffer+off, tempu16 );
                    this->status_gropu_.router_.second = tempu16;
                    tempu16 = 0;
                    off += utility::deserialize_int( buffer+off, tempu16 );
                    this->status_gropu_.switch_.first = tempu16;
                    tempu16 = 0;
                    off += utility::deserialize_int( buffer+off, tempu16 );
                    this->status_gropu_.switch_.second = tempu16;
                    tempu16 = 0;
                    off += utility::deserialize_int( buffer+off, tempu16 );
                    this->status_gropu_.preload_.first = tempu16;
                    tempu16 = 0;
                    off += utility::deserialize_int( buffer+off, tempu16 );
                    this->status_gropu_.preload_.second = tempu16;

                    h_uint8 u8 = buffer[off++];
                    if ( POWER_BOARD_AUTO_REPORT_OPEN == u8 ) u8 = H_STATUS_OPEN;
                    else if ( POWER_BOARD_AUTO_REPORT_CLOSE == u8 ) u8 =H_STATUS_CLOSE;
                    else u8 = H_STATUS_UNKNOWN;
                    this->status_gropu_.auto_report_ = u8;
                    this->status_gropu_.auto_report_interval_ = buffer[off++];
                    break;
                }
                case POWER_BOARD_ADDR_RD_SOFT_VERSION:
                {
                    if ( length < DEFAULT_TCP_CTL_PB_SOFT_VERSION_LEN )
                    {
                        this->strerr = "the buffer(";
                        this->strerr += utility::uint64_2_str(len);
                        this->strerr += ") is smaller than the power board soft version data(";
                        this->strerr += utility::uint32_2_str(DEFAULT_TCP_CTL_PB_SOFT_VERSION_LEN);
                        this->strerr += ")";
                        return -1;
                    }
                    unsigned char v1 = buffer[off++];
                    unsigned char v2 = buffer[off++];
                    unsigned char v3 = buffer[off++];
                    char psztemp[16] = {0};
                    SNPRINTF( psztemp, sizeof(psztemp), "%d.%d.%d", v1, v2, v3 );
                    this->soft_version_ = psztemp;
                    break;
                }
                case POWER_BOARD_ADDR_RD_HARD_VERSION:
                {

                    if ( length < DEFAULT_TCP_CTL_PB_HARD_VERSION_LEN )
                    {
                        this->strerr = "the buffer(";
                        this->strerr += utility::uint64_2_str(len);
                        this->strerr += ") is smaller than the power board hard version data(";
                        this->strerr += utility::uint32_2_str(DEFAULT_TCP_CTL_PB_HARD_VERSION_LEN);
                        this->strerr += ")";
                        return -1;
                    }
                    unsigned char v1 = buffer[off++];
                    unsigned char v2 = buffer[off++];
                    char psztemp[16] = {0};
                    SNPRINTF( psztemp, sizeof(psztemp), "%d.%d", v1, v2 );
                    this->hard_version_ = psztemp;
                    break;
                }
                default:
                {
                    this->strerr = "unknown power board address(";
                    this->strerr += utility::uint32_2_str( this->addr_ );
                    this->strerr += ")";
                    return -1;
                }
            }
        }
        else
        {
            this->strerr = "unknown power board control mode(";
            this->strerr += utility::uint32_2_str( this->pb_rd_wt_mode_ );
            this->strerr += ")";
            return -1;
        }
        off += 3;
        return 0;
    }

    po_udp_data_resp2::po_udp_data_resp2() :
            po_protocol_middle(PO_MSG_TAG_DATA_RESP),
            mr_cnt(0),
            ispd_cnt(0)
    {
    }

    po_udp_data_resp2::~po_udp_data_resp2()
    {}


    int po_udp_data_resp2::serialize_in( unsigned char* buffer, size_t len, size_t& off )
    {
        return -1;
    }

    int po_udp_data_resp2::deserialize_in( const unsigned char* buffer, size_t len, size_t& off )
    {
#if DEBUG_PRINT_RAW_ISPD_DATA || DEBUG_PRINT_RAW_DATA
        printf( "Udp ISPD raw data(%lu): ", len );
        for ( size_t i = 0; i < len; ++i )
        {
            printf( "%02X ", buffer[i] );
        }
        printf( "\n" );
#endif

        off = 0;
        if ( len < DEFAULT_UDP_TAG_DATE_RESP_STATIC_LEN )
        {
            this->strerr = "the buffer(";
            this->strerr += utility::uint64_2_str(len);
            this->strerr += ") is smaller than the static po_udp_data_resp(";
            this->strerr += utility::uint32_2_str(DEFAULT_UDP_TAG_DATE_RESP_STATIC_LEN);
            this->strerr += ")";
            return -1;
        }

        this->mr_cnt = buffer[off++];
        this->ispd_cnt = buffer[off++];

        size_t td_len = (DEFAULT_UDP_TAG_DATE_RESP_DYNAMIC_TD_LEN) * this->mr_cnt;
        size_t ispd_len = (td_len + DEFAULT_UDP_TAG_DATE_RESP_DYNAMIC_SENSOR_LEN) * this->ispd_cnt;
        ispd_len += DEFAULT_UDP_TAG_DATE_RESP_STATIC_LEN;
        if ( len < ispd_len && (ispd_len-len)%15 != 0 )
        {

            this->strerr = "the buffer(";
            this->strerr += utility::uint64_2_str(len);
            this->strerr += ") is smaller than the total po_udp_data_resp(";
            this->strerr += utility::uint64_2_str(ispd_len);
            this->strerr += ")";
            return -1;
        }

        this->date_time.y = buffer[off++];
        this->date_time.m = buffer[off++];
        this->date_time.d = buffer[off++];
        this->date_time.h = buffer[off++];
        this->date_time.n = buffer[off++];
        this->date_time.s = buffer[off++];

        h_uint16 tempu16 = 0;
        off += utility::deserialize_int( buffer+off, tempu16 );
        this->date_time.ms = tempu16;


        h_uint32 tempu32 = 0;
        h_uint32 tempuid = 0;
        this->data_array.clear();

        for ( unsigned int i = 0; i < this->ispd_cnt; ++i )
        {
            ispd_data_stu ids;

            tempu32 = 0;
            off += utility::deserialize_int( buffer+off, tempu32 );
            ids.id = tempu32;

            for ( unsigned int j = 0; j < this->mr_cnt; ++j )
            {
                tempuid = 0;
                off += utility::deserialize_int( buffer+off, tempuid );
                tempu32 = 0;
                off += utility::deserialize_int( buffer+off, tempu32 );
                ids.tds.push_back( mr_time_diff(tempuid, tempu32) );
            }

            size_t seni_len = buffer[off++];
            if ( 15 == seni_len )
            {
                h_int16 temp16 = 0;
                for ( unsigned int j = 0; j < 3; ++j )
                {
                    temp16 = 0;
                    off += utility::deserialize_int( buffer+off, temp16 );
                    if ( 0 == j ) ids.acceleration.x = temp16;
                    else if ( 1 == j ) ids.acceleration.y = temp16;
                    else ids.acceleration.z = temp16;
                }

                for ( unsigned int j = 0; j < 3; ++j )
                {
                    temp16 = 0;
                    off += utility::deserialize_int( buffer+off, temp16 );
                    if ( 0 == j ) ids.gyroscope.x = temp16;
                    else if ( 1 == j ) ids.gyroscope.y = temp16;
                    else ids.gyroscope.z = temp16;
                }

                tempu16 = 0;
                off += utility::deserialize_int( buffer+off, tempu16 );
                ids.heart_rate = tempu16;

                unsigned char tempu8 = buffer[off++];
                ids.power = tempu8 & 0X7F;
                ids.charge = (tempu8&0X80) == 0X80 ? 1 : 0;

            }
            else
            {
                ids.acceleration.x = 0;
                ids.acceleration.y = 0;
                ids.acceleration.z = 0;

                ids.gyroscope.x = 0;
                ids.gyroscope.y = 0;
                ids.gyroscope.z = 0;

                ids.heart_rate = 0;

                ids.power = 0;
                ids.charge = 0;
                //off += 15;

            }
            this->data_array.push_back( ids );
        }

        return 0;
    }


    po_udp_mr_status::po_udp_mr_status() :
            po_protocol_middle(PO_MSG_MR_STATUS_RESP)
    {}

    po_udp_mr_status::~po_udp_mr_status()
    {}

    int po_udp_mr_status::serialize_in( unsigned char* buffer, size_t len, size_t& off )
    {
        return -1;
    }

    int po_udp_mr_status::deserialize_in( const unsigned char* buffer, size_t len, size_t& off )
    {
        this->status_array.clear();

#if DEBUG_PRINT_RAW_MR_STATUS || DEBUG_PRINT_RAW_DATA
        printf( "Udp Mr status raw data(%lu): ", len );
        for ( size_t i = 0; i < len; ++i )
        {
            printf( "%02X ", buffer[i] );
        }
        printf( "\n" );
#endif

        off = 0;
        if ( len < DEFAULT_UDP_MR_STATUS_ESP_STATIC_LEN )
        {
            this->strerr = "the buffer(";
            this->strerr += utility::uint64_2_str(len);
            this->strerr += ") is smaller than the po_udp_mr_status(";
            this->strerr += utility::uint32_2_str(DEFAULT_UDP_MR_STATUS_ESP_STATIC_LEN);
            this->strerr += ")";
            return -1;
        }

        unsigned char ncount = buffer[off++];

        size_t total = (DEFAULT_UDP_MR_STATUS_ESP_DYNAMIC_LEN) * ncount +
                        DEFAULT_UDP_MR_STATUS_ESP_STATIC_LEN;
        if ( len < total )
        {
            this->strerr = "the buffer(";
            this->strerr += utility::uint64_2_str(len);
            this->strerr += ") is smaller than the po_udp_mr_status(";
            this->strerr += utility::uint64_2_str(total);
            this->strerr += ")";
            return -1;
        }

        h_uint16 tempu16 = 0;
        h_uint32 tempu32 = 0;
        for ( unsigned int i = 0; i < ncount; ++i )
        {
            off += utility::deserialize_int( buffer+off, tempu32 );
            off += utility::deserialize_int( buffer+off, tempu16 );
            this->status_array.push_back( mr_status_stu(tempu32, tempu16) );
        }
        return 0;
    }


    po_udp_power_board::po_udp_power_board() :
            po_protocol_middle(PO_MSG_POWER_BOARD_RESP)
    {}

    po_udp_power_board::~po_udp_power_board()
    {}

    int po_udp_power_board::serialize_in( unsigned char* buffer, size_t len, size_t& off )
    {
        return -1;
    }

    int po_udp_power_board::deserialize_in( const unsigned char* buffer, size_t len, size_t& off )
    {
#if DEBUG_PRINT_RAW_PB_STATUS || DEBUG_PRINT_RAW_DATA
        printf( "Udp Power board raw data(%lu): ", len );
        for ( size_t i = 0; i < len; ++i )
        {
            printf( "%02x ", buffer[i] );
        }
        printf( "\n" );
#endif

        static const unsigned char header = 0X84;
        static const unsigned char tailer = 0X16;

        if ( len < DEFAULT_UDP_MR_POWER_BOARD_LEN ||
             buffer[4] != header || buffer[len-1] != tailer )
        {
            this->strerr = utility::gt_snprintf( "power board data frame is bad(%lu)", len );
            return -1;
        }

        h_uint16 tempu16 = 0;
        h_uint16 check_sum = CRC16_2( buffer+4, len-4-3 );
        utility::deserialize_int( buffer+len-3, tempu16 );
        if ( tempu16 != check_sum )
        {
            this->strerr = "power board data frame check sum is bad";
            return -1;
        }

        off = 0;
        h_uint32 tempu32 = 0;
        off += utility::deserialize_int( buffer+off, tempu32 );
        this->id_ = tempu32;

        off += 4; // Header, CMD, CTL, Length.
        unsigned char temp = buffer[off++];
        this->status_.switch_ = (temp&POWER_BOARD_STATUS_SWITCH) == POWER_BOARD_STATUS_SWITCH ? true : false;
        this->status_.router_ = (temp&POWER_BOARD_STATUS_ROUTER) == POWER_BOARD_STATUS_ROUTER ? true : false;
        this->status_.locate_ = (temp&POWER_BOARD_STATUS_LOCATE) == POWER_BOARD_STATUS_LOCATE ? true : false;
        this->status_.microphone_ = (temp&POWER_BOARD_STATUS_MICROPHONE) == POWER_BOARD_STATUS_MICROPHONE ? true : false;
        this->status_.camera_ = (temp&POWER_BOARD_STATUS_CAMERA) == POWER_BOARD_STATUS_CAMERA ? true : false;
        this->status_.preload_ = (temp&POWER_BOARD_STATUS_PRELOAD) == POWER_BOARD_STATUS_PRELOAD ? true : false;

        temp = buffer[off++];
        this->status_.source_ = (temp&POWER_BOARD_STATUS_SOURCE) == POWER_BOARD_STATUS_SOURCE ? true : false;
        this->status_.battery_charge_ = (temp&POWER_BOARD_STATUS_BATTERY_CHARGE) == POWER_BOARD_STATUS_BATTERY_CHARGE ? true : false;
        this->status_.battery_status_ = (temp&POWER_BOARD_STATUS_BATTERY_STATUS) == POWER_BOARD_STATUS_BATTERY_STATUS ? true : false;
        this->status_.internet_ = (temp&POWER_BOARD_STATUS_INTERNET) == POWER_BOARD_STATUS_INTERNET ? true : false;
        this->status_.battery_connect_ = (temp&POWER_BOARD_STATUS_BATTERY_CONNECT) == POWER_BOARD_STATUS_BATTERY_CONNECT ? true : false;
        this->status_.battery_control_ = (temp&POWER_BOARD_STATUS_BATTERY_COLTROL) == POWER_BOARD_STATUS_BATTERY_COLTROL ? true : false;

        h_int16 temp16 = 0;
        off += utility::deserialize_int( buffer+off, temp16 );
        this->group_.temperature_ = temp16;
        tempu16 = 0;
        off += utility::deserialize_int( buffer+off, tempu16 );
        this->group_.battery_.first = tempu16;
        this->group_.battery_.second = buffer[off++];
        tempu16 = 0;
        off += utility::deserialize_int( buffer+off, tempu16 );
        this->group_.camera_.first = tempu16;
        tempu16 = 0;
        off += utility::deserialize_int( buffer+off, tempu16 );
        this->group_.camera_.second = tempu16;
        tempu16 = 0;
        off += utility::deserialize_int( buffer+off, tempu16 );
        this->group_.locate_.first = tempu16;
        tempu16 = 0;
        off += utility::deserialize_int( buffer+off, tempu16 );
        this->group_.locate_.second = tempu16;
        tempu16 = 0;
        off += utility::deserialize_int( buffer+off, tempu16 );
        this->group_.router_.first = tempu16;
        tempu16 = 0;
        off += utility::deserialize_int( buffer+off, tempu16 );
        this->group_.router_.second = tempu16;
        tempu16 = 0;
        off += utility::deserialize_int( buffer+off, tempu16 );
        this->group_.switch_.first = tempu16;
        tempu16 = 0;
        off += utility::deserialize_int( buffer+off, tempu16 );
        this->group_.switch_.second = tempu16;
        tempu16 = 0;
        off += utility::deserialize_int( buffer+off, tempu16 );
        this->group_.preload_.first = tempu16;
        tempu16 = 0;
        off += utility::deserialize_int( buffer+off, tempu16 );
        this->group_.preload_.second = tempu16;
        h_uint8 ar = buffer[off++];
        if ( POWER_BOARD_AUTO_REPORT_OPEN == ar )
            this->group_.auto_report_ = H_STATUS_OPEN;
        else if ( POWER_BOARD_AUTO_REPORT_CLOSE == ar )
            this->group_.auto_report_ = H_STATUS_CLOSE;
        else
            this->group_.auto_report_ = H_STATUS_UNKNOWN;

        this->group_.auto_report_interval_ = buffer[off++];
        off += 3;
        return 0;
    }

}

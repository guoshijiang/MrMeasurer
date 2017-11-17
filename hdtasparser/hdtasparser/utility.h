//
// Created by wanghaiyang on 9/20/16.
//

#ifndef HDTAS_UTILITY_H
#define HDTAS_UTILITY_H

#include "header.h"
#include "struct.h"
#include <vector>
#include <string>
#ifdef HDTAS_WINDOWS
#include <winsock2.h>
#else
#include <arpa/inet.h>
#include <bits/unique_ptr.h>

#endif

namespace hdtas
{
    class utility
    {
    public:
        enum IP_FAMILY
        {
            IP_UNKNOWN,
            IP_V4,
            IP_V6
        };

        enum SOCKET_TYPE
        {
            ST_UNKNOWN,
            ST_TCP,
            ST_UDP
        };
    public:
        typedef std::vector<std::string> ipaddresses;

        /*
         * Get the ip addresses of all network cards.
         * ipv4s: return ip addresses of V4.
         * ipv6s: return ip addresses of V6.
         * error: if failed, error will return the error message.
         * bool : true for success, false for failure.
         */
        static bool get_ipaddrs( ipaddresses& ipv4s, ipaddresses& ipv6s, std::string& error );
        static int get_ipv4( char* buffer, size_t cap );

        static bool is_ipv4_str( const char* ip );

        static std::string int32_2_str( h_int32 v );
        static std::string uint32_2_str( h_uint32 v );

        static std::string int64_2_str( h_int64 v );
        static std::string uint64_2_str( h_uint64 v );

        //template<typename ... Args>
        //inline static std::string snprintf( const std::string& f, Args ... args )
        //{
        //    size_t size = std::snprintf(nullptr, 0, f.c_str(), args ... ) + 1;
        //    std::unique_ptr<char[]> buffer( new char[size] );
        //    std::snprintf( buffer.get(), size, f.c_str(), args ... );
        //    return std::string( buffer.get(), buffer.get()+size-1 );
        //}

        static const char* gt_snprintf( const char* f, ... );

        static int get_host_port_from_socket( int fd, std::string& host, h_uint16& port );
        static int get_socket_address( const char* host, int port,
                                       utility::IP_FAMILY family,
                                       utility::SOCKET_TYPE type,
                                       struct sockaddr_storage& addr );

        /**
         * @brief Little end.
         * @param v
         * @param b
         * @return
         */
        template<typename T>
        static inline unsigned int serialize_int( T v, unsigned char* b )
        {
            unsigned char* t = (unsigned char*)&v;
            for ( unsigned int i = 0; i < sizeof(v); ++i )
            {
                b[i] = t[i];
            }
            return sizeof(v);
        }

        /**
         * @brief Little end.
         * @param b
         * @param v
         * @return
         */
        template<typename T>
        static inline unsigned int deserialize_int( const unsigned char* b, T& v )
        {
            v = 0;
            unsigned char* t = (unsigned char*)&v;
            for ( unsigned int i = 0; i < sizeof(v); ++i )
            {
                t[i] = b[i];
            }
            return sizeof(v);
        }

        template <typename T>
        static std::string int_2_hex( T v )
        {
            std::string n;
            n.resize( sizeof(v)*2, 0 );
            const unsigned char* b = (unsigned char*)&v;
            for ( unsigned int i = 0; i < sizeof(v); ++i )
            {
                snprintf( &n[i*2], 3, "%02X", b[i] );
            }
            return n;
        }


        template <typename T>
        static int hex_2_int( const std::string& n, T& v )
        {
            if ( n.size() != sizeof(v)*2 )
                return -1;
            unsigned char* b = (unsigned char*)&v;
            for ( unsigned int i = 0; i < sizeof(v); ++i )
            {
                char low = n[i*2];
                char high = n[i*2+1];

                if ( low >= 'a' && low <= 'f' )
                    low = low - 'a' + 10;
                else if ( low >= 'A' && low <= 'F' )
                    low = low - 'A' + 10;
                else
                    low = low - '0';
                if ( high >= 'a' && high <= 'f' )
                    high = high - 'a' + 10;
                else if ( high >= 'A' && high <= 'F' )
                    high = high - 'A' + 10;
                else
                    high = high - '0';
                b[i] = low*16+high;

            }
            return sizeof(v);
        }

        static std::string device_mr_id_2_uwb_id( device_mr_id id )
        {
            std::string temp( 13, '\0' );
            snprintf( &temp[0], 13, "BS%u", id );
            return temp;
        }

        static device_mr_id uwb_id_2_device_mr_id( const char* id )
        {
            device_mr_id dmid = 0;
            if ( int(strlen(id)) == sscanf(id, "BS%u", &dmid) )
                return 0;
            else
                return dmid;
        }

        static inline std::string net_to_pre( h_uint32 n )
        {
            const unsigned char* p = (const unsigned char*)(&n);
            char psztemp[16] = {0};
            snprintf( psztemp, 16, "%u.%u.%u.%u", p[3], p[2], p[1], p[0] );
            return std::string( psztemp );
        }

        static inline int pre_to_net( const std::string& p, h_uint32& n )
        {
            n = 0;
            unsigned char* ptn = (unsigned char*)(&n);
            if ( p.size() <  7 ) return -1;

            size_t findex = p.find( '.', 0 );
            if ( std::string::npos == findex ) return -1;
            std::string temp = p.substr( 0, findex );
            if ( temp.size() == 0 ) return -1;
            unsigned char value = atoi( temp.c_str() );
            if ( value > 255 ) return -1;
            ptn[3]  = value;

            size_t sindex = p.find( '.', findex+1 );
            if ( std::string::npos == sindex ) return -1;
            temp = p.substr( findex+1, sindex-findex-1 );
            if ( temp.size() == 0 ) return  -1;
            value = atoi( temp.c_str() );
            if ( value > 255 ) return -1;
            ptn[2]  = value;

            findex = p.find( '.', sindex+1 );
            if ( std::string::npos == findex ) return -1;
            temp = p.substr( sindex+1, findex-sindex-1 );
            if ( temp.size() == 0 ) return -1;
            value = atoi( temp.c_str() );
            if ( value > 255 ) return -1;
            ptn[1]  = value;

            if ( findex >= p.size() ) return -1;
            temp = p.substr( findex+1, p.size()-findex-1 );
            if ( temp.size() == 0 ) return -1;
            value = atoi( temp.c_str() );
            if ( value > 255 ) return -1;
            ptn[0]  = value;

            return 0;
        }

        /**
         * Get error buffer capacity.
         * @return Buffer capacity.
         */
        static size_t err_buf_cap();

        /**
         * Get error buffer(thread independent).
         * @return Buffer.
         */
        static char* err_buf();

        /**
         * Get system error from erron.
         * @return Error string or NULL.
         */
        static const char* err_str( int e = H_ERR_SYSTEM );

    };

    typedef struct host_port_info
    {
        std::string host;
        int port;
    } host_port_info;

    typedef std::vector<host_port_info> host_port_array;

    enum server_type
    {
        ST_UNKNOWN = 0,
        ST_ALONE = 1,
        ST_MASTER = 2,
        ST_SLAVER = 3
    };

    class configuration
    {
    public:
        configuration();

        ~configuration();

        int parse( const char* name );

        server_type get_server_type() const;

        unsigned int get_worker_num() const;

        const char* get_zk_info() const;

        const host_port_array& get_slaver_infos() const;

        const host_port_info& get_udp_info() const;

        const host_port_info& get_tcp_info() const;

        const host_port_info& get_control_info() const;

        const host_port_info& get_websocket_host() const;

        const redis_mq_info& get_rmq_info() const;

        const data_base_info& get_db_info() const;

        const host_port_array& get_mq_infos() const;

        const char* get_fs_info() const;
        const char* get_update_file_path() const;
        const char* get_log_path() const;

        bool be_daemon() const;

        const std::string& get_error() const;
    private:
        bool daemon_ = false;
        server_type type_;
        unsigned int number_ = 0;
        std::string zk_info_;
        std::string fs_info_;
        std::string strerr_;
        std::string log_path_;
        std::string update_path_;
        host_port_info udp_info_;
        host_port_info tcp_info_;
        host_port_info control_info_;
        host_port_info ws_info_;
        host_port_array slaver_infos_;
        host_port_array mq_infos_;
        redis_mq_info rmq_info_;
        data_base_info db_info_;
    };
}

#endif //HDTAS_UTILITY_H

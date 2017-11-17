//
// Created by wanghaiyang on 9/20/16.
//
#include "utility.h"

#include <cerrno>
#include <cstring>
#ifdef HDTAS_WINDOWS
#include <winsock2.h>
#include <ws2ipdef.h>
#include <Ws2tcpip.h>
#include <stdarg.h>
#else
#include <ifaddrs.h>
#include <netdb.h>
#include <stdarg.h>
#endif
namespace hdtas
{

#ifdef HDTAS_WINDOWS
static WCHAR * charToWchar(char *s){

   int w_nlen=MultiByteToWideChar(CP_ACP,0,s,-1,NULL,0);

  WCHAR *ret;

  ret=(WCHAR*) malloc(sizeof(WCHAR)*w_nlen);

  memset(ret,0,sizeof(ret));

  MultiByteToWideChar(CP_ACP,0,s,-1,ret,w_nlen);

  return ret;

}

static LPWSTR charToPWstr(char * s) {
    int w_nlen=MultiByteToWideChar(CP_ACP,0,s,-1,NULL,0);

   LPWSTR ret;

   ret=(LPWSTR) malloc(sizeof(WCHAR)*w_nlen);

   memset(ret,0,sizeof(ret));

   MultiByteToWideChar(CP_ACP,0,s,-1,ret,w_nlen);

   return ret;
}

static char* WCharToChar(WCHAR *s){

   int w_nlen=WideCharToMultiByte(CP_ACP,0,s,-1,NULL,0,NULL,false);

  char *ret=new char[w_nlen];

  memset(ret,0,w_nlen);

  WideCharToMultiByte(CP_ACP,0,s,-1,ret,w_nlen,NULL,false);

 return ret;

}

static char * PWstrToChar(PCWSTR s) {
    int w_nlen=WideCharToMultiByte(CP_ACP,0,s,-1,NULL,0,NULL,false);

   char *ret=new char[w_nlen];

   memset(ret,0,w_nlen);

   WideCharToMultiByte(CP_ACP,0,s,-1,ret,w_nlen,NULL,false);

  return ret;

}

#endif

#ifdef HDTAS_WINDOWS
    __declspec(thread) static char g_err_buf[HDTAS_ERR_BUF_CAP] = {0};
    __declspec(thread) static char g_thread_buffer[HDTAS_GLOBAL_THREAD_BUF_CAP] = {0};
    __declspec(thread) static bool g_standard_error = true;
#else
    static __thread char g_err_buf[HDTAS_ERR_BUF_CAP] = {0};
    static __thread char g_thread_buffer[HDTAS_GLOBAL_THREAD_BUF_CAP] = {0};
    static __thread bool g_standard_error = true;
#endif

    bool utility::get_ipaddrs( ipaddresses& ipv4s, ipaddresses& ipv6s, std::string& error )
    {
#ifdef HDTAS_WINDOWS
        return false;
#else
        char pszbuffer[BUFFER_SIZE_ERROR] = {0};
        // Clear all the buffers
        ipv4s.clear();
        ipv6s.clear();
        error.clear();

        struct ifaddrs* ipaddrs = NULL;
        if ( -1 == getifaddrs(&ipaddrs) )
        {
            char* prtn = strerror_r( errno, pszbuffer, BUFFER_SIZE_ERROR );
            error = prtn;
            return false;
        }

        struct ifaddrs* addr = ipaddrs;
        while ( addr != NULL )
        {
            if ( addr->ifa_addr->sa_family == AF_INET )
            {
                struct  sockaddr_in* addrin = reinterpret_cast<struct sockaddr_in*>( addr->ifa_addr );
                inet_ntop( AF_INET, &(addrin->sin_addr), pszbuffer, INET_ADDRSTRLEN );
                ipv4s.push_back( pszbuffer );
            }
            if ( addr->ifa_addr->sa_family == AF_INET6 )
            {
                struct sockaddr_in* addrin = reinterpret_cast<struct sockaddr_in*>( addr->ifa_addr );
                inet_ntop( AF_INET6, &(addrin->sin_addr), pszbuffer, INET6_ADDRSTRLEN );
                ipv6s.push_back( pszbuffer );
            }
            addr = addr->ifa_next;
        }
        if ( NULL != ipaddrs )
            freeifaddrs( ipaddrs );
        return true;
#endif
    }

    int utility::get_ipv4( char* buffer, size_t cap )
    {
#ifdef HDTAS_WINDOWS
        return 0;
#else
        struct ifaddrs* addrs = NULL;
        int tr = getifaddrs( &addrs );
        if ( 0 != tr )
            return H_ERR_SYSTEM;

        int rtn = H_ERR_NO_MATCH;
        struct ifaddrs* temp = addrs;
        while ( NULL != temp )
        {
            if ( temp->ifa_addr->sa_family == AF_INET )
            {
                if ( cap < LEN_MAX_IP_ADDRESS )
                {
                    rtn = H_ERR_BUFFER_INADEQUATE;
                    break;
                }

                if ( STRNCASECMP(temp->ifa_name, "e", 1) == 0 )
                {
                    struct sockaddr_in* addrin = (struct sockaddr_in*)(temp->ifa_addr);
                    inet_ntop( AF_INET, &addrin->sin_addr, buffer, cap );
                    rtn = H_SUCCESS;
                    break;
                }
            }

            temp = temp->ifa_next;
        }

        if ( NULL != addrs )
            freeifaddrs( addrs );
        return rtn;
#endif
    }

    bool utility::is_ipv4_str( const char* ip )
    {
        if ( NULL == ip )
            return false;
        unsigned int ip1 = 0, ip2 = 0, ip3 = 0, ip4 = 0;
        if ( sscanf(ip, "%u.%u.%u.%u", &ip1, &ip2, &ip3, &ip4) != 4  ||
             ip1 > 255 || ip2 > 255 || ip3 > 255 || ip4 > 255 )
            return false;
        else
            return true;
    }

    int utility::get_socket_address( const char* host, int port,
                                     utility::IP_FAMILY family,
                                     utility::SOCKET_TYPE type,
                                     struct sockaddr_storage& addr )
    {
        struct addrinfo hints;
        memset( &hints, 0, sizeof(hints) );
        if ( IP_FAMILY::IP_V4 == family )
            hints.ai_family = AF_INET;
        else if ( IP_FAMILY::IP_V6 == family )
            hints.ai_family = AF_INET6;
        else
        {
            g_standard_error = false;
            std::string str = "unknown ip family type";
            STRNCPY(g_err_buf, HDTAS_ERR_BUF_CAP, str.c_str(), str.size() );
            return -1;
        }

        if ( SOCKET_TYPE::ST_TCP == type )
            hints.ai_socktype = SOCK_STREAM;
        else if ( SOCKET_TYPE::ST_UDP == type )
            hints.ai_socktype = SOCK_DGRAM;
        else
        {
            g_standard_error = false;
            std::string str = "unknown socket type";
            STRNCPY(g_err_buf, HDTAS_ERR_BUF_CAP, str.c_str(), str.size() );
            return -1;
        }
#ifdef HDTAS_WINDOWS
        struct addrinfo* info = NULL;
        std::string strport = int32_2_str( port );
#else
        int rtn = getaddrinfo( host, strport.c_str(), NULL, &info );
#endif
        int rtn = 0;
        if ( 0 != rtn )
        {
            g_standard_error = false;

            const char* err = NULL;//WCharToChar( gai_strerror( rtn ) );
            STRNCPY(g_err_buf, HDTAS_ERR_BUF_CAP, err, STRLEN(err) );
            return -1;
        }

        bool bsuccess = false;
        struct addrinfo* temp = info;
        while ( NULL != temp )
        {
            if ( AF_INET == temp->ai_family )
            {
                bsuccess = true;
                struct sockaddr_in* target = (struct sockaddr_in*)(&addr);
                struct sockaddr_in* addrin = (struct sockaddr_in*)(temp->ai_addr);
                memcpy( target, addrin, sizeof(struct sockaddr_in) );
                break;
            }
            else if ( AF_INET6 == temp->ai_family )
            {
                bsuccess = true;
                struct sockaddr_in6* target = (struct sockaddr_in6*)(&addr);
                struct sockaddr_in6* addrin = (struct sockaddr_in6*)(temp->ai_addr);
                memcpy( target, addrin, sizeof(struct sockaddr_in6) );
                break;
            }
            else
                bsuccess = false;

            temp = temp->ai_next;
        }

        if ( !bsuccess )
        {
            g_standard_error = false;
            std::string str = "no matched address";
            STRNCPY(g_err_buf, HDTAS_ERR_BUF_CAP, str.c_str(), str.size() );
            return -1;
        }
        else
            return 0;
    }

    std::string utility::int32_2_str( h_int32 v )
    {
        char psztemp[16] = {0};
        SNPRINTF( psztemp, 16, "%d", v );
        return std::string(psztemp);
    }

    std::string utility::uint32_2_str( h_uint32 v )
    {
        char psztemp[16] = {0};
        SNPRINTF( psztemp, 16, "%u", v );
        return std::string(psztemp);
    }

    std::string utility::int64_2_str( h_int64 v )
    {
        char psztemp[64] = {0};
        SNPRINTF( psztemp, 64, "%ld", v );
        return std::string(psztemp);
    }

    std::string utility::uint64_2_str( h_uint64 v )
    {
        char psztemp[64] = {0};
        SNPRINTF( psztemp, 64, "%lu", v );
        return std::string(psztemp);
    }

    const char* utility::gt_snprintf( const char* f, ... )
    {
        va_list vl;
        va_start( vl, f );
        vsnprintf( g_thread_buffer, HDTAS_GLOBAL_THREAD_BUF_CAP, f, vl );
        va_end( vl );
        return g_thread_buffer;
    }

    int utility::get_host_port_from_socket( int fd, std::string& host, h_uint16& port )
    {
#ifdef HDTAS_WINDOWS
		SOCKADDR_STORAGE addr;
		addr.ss_family = 1;
		int len = sizeof(addr);
#else
		struct sockaddr_storage addr;
		socklen_t len = sizeof(addr);
        getsockname( fd, (struct sockaddr*)(&addr), &len );
#endif
		
        void* inaddr = nullptr;
        switch ( addr.ss_family )
        {
            case AF_INET:
            {
#ifdef HDTAS_WINDOWS
				PSOCKADDR_IN addrin = (PSOCKADDR_IN)(&addr);
#else
				struct sockaddr_in* addrin = (struct sockaddr_in*)(&addr);
                port = ntohs( addrin->sin_port );
#endif
                inaddr = &(addrin->sin_addr);
            }
                break;
            case AF_INET6:
            {
#ifdef HDTAS_WINDOWS
				PSOCKADDR_IN6 addrin = (PSOCKADDR_IN6)(&addr);
#else
				struct sockaddr_in6* addrin = (struct sockaddr_in6*)(&addr);
                port = ntohs( addrin->sin6_port );
#endif
                inaddr = &(addrin->sin6_addr);
            }
                break;
            default:
                return -1;
        }

		char addrbuf[128] = { 0 };

#ifdef HDTAS_WINDOWS
        const char* straddr = NULL;//PWstrToChar( InetNtop( addr.ss_family, inaddr,  charToWchar(addrbuf), sizeof(addrbuf) ) );
#else
		const char* straddr = inet_ntop( addr.ss_family, inaddr, addrbuf, sizeof(addrbuf) );
#endif

        if ( NULL != straddr )
        {
            host = straddr;
            return 0;
        }

        return -1;

    }



    size_t utility::err_buf_cap()
    {
        return HDTAS_ERR_BUF_CAP;
    }

    char* utility::err_buf()
    {
        return g_err_buf;
    }

    const char* utility::err_str( int e )
    {
        switch ( e )
        {
            case H_ERR_SYSTEM:
            {
#ifdef HDTAS_WINDOWS
                DWORD e = GetLastError();
		        FormatMessage( FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
                NULL, e, 0, /*charToPWstr(g_err_buf)*/NULL, HDTAS_ERR_BUF_CAP, NULL);
		        //return g_err_buf;
#else
                if ( !g_standard_error )
                {
                    g_standard_error = true;
                    return g_err_buf;
                }
                else
                    strerror_r(errno, g_err_buf, HDTAS_ERR_BUF_CAP);
#endif
                break;
            }
            case H_ERR_PARAMETER:
            {
                SNPRINTF( g_err_buf, HDTAS_ERR_BUF_CAP, "invalid parameter" );
                break;
            }
            case H_ERR_BUFFER_INADEQUATE:
            {
                SNPRINTF( g_err_buf, HDTAS_ERR_BUF_CAP, "buffer inadequate" );
                break;
            }
            case H_ERR_NO_MATCH:
            {
                SNPRINTF( g_err_buf, HDTAS_ERR_BUF_CAP, "no match item" );
                break;
            }
            default:
            {
                SNPRINTF( g_err_buf, HDTAS_ERR_BUF_CAP, "unknown error" );
                break;
            }
        }

        return g_err_buf;

    }

    bool parse_host_port_info( const std::string& info, host_port_array& array )
    {
        std::string temp;
        size_t findex = 0;
        size_t sindex = 0;
        host_port_info hp;

        array.clear();

        do
        {
            findex = info.find( ':', findex+1 );
            if ( std::string::npos == findex )
                goto ERROR_HANDLE;
            if ( 0!= sindex )
                sindex += 1;
            hp.host = info.substr( sindex, findex-sindex );

            sindex = info.find( ',', findex+1 );
            if ( std::string::npos == sindex )
                sindex = info.size();
            temp = info.substr( findex+1, sindex -findex-1 );
            hp.port = atoi( temp.c_str() );

            findex = sindex;
            array.push_back( hp );
        } while ( sindex < info.size() );


        return true;

        ERROR_HANDLE:
        array.clear();
        return false;
    }

    bool parser_mysql_info( const std::string& info, data_base_info& db_info )
    {
        size_t findex = info.find( ':', 0 ); // Get host.
        if ( std::string::npos == findex )
            return false;
        db_info.host = info.substr( 0, findex );

        size_t sindex = info.find( ':', findex+1 ); // Get port.
        if ( std::string::npos == sindex )
            return false;
        db_info.port = atoi( info.substr(findex+1, sindex-findex-1).c_str() );

        findex = info.find( ':', sindex+1 ); // Get user.
        if ( std::string::npos == findex )
            return false;
        db_info.user = info.substr( sindex+1, findex-sindex-1 );

        sindex = info.find( ':', findex+1 );
        if ( std::string::npos == sindex )
            return false;
        db_info.pwd = info.substr( findex+1, sindex-findex-1 );

        if ( sindex+1 >= info.size() )
            return false;
        db_info.db_name = info.substr( sindex+1, info.size()-sindex-1 );

        char psztemp[128] = {0};
        SNPRINTF( psztemp, 128, "tcp://%s:%d", db_info.host.c_str(), db_info.port );
        db_info.url = psztemp;

        return true;
    }

    bool parser_redis_info( const std::string& info, redis_mq_info& redis_info )
    {
        size_t findex = info.find( ':', 0 ); // Get host.
        if ( std::string::npos == findex )
            return false;
        redis_info.host = info.substr( 0, findex );

        size_t sindex = info.find( ':', findex+1 ); // Get port;
        if ( std::string::npos == sindex )
            return false;
        redis_info.port = atoi( info.substr(findex+1, sindex-findex-1).c_str() );

        findex = info.find( ':', sindex+1 ); // Get auth;
        if ( std::string::npos == findex )
            return false;
        redis_info.password = info.substr( sindex+1, findex-sindex-1 );
        if ( STRCASECMP("NULL", redis_info.password.c_str()) == 0 )
            redis_info.password = "";

        sindex = info.find( ':', findex+1 ); // Get subscribe channel.
        if ( std::string::npos == sindex )
            return false;
        redis_info.sub_channel = info.substr( findex+1, sindex-findex-1 );

        if ( sindex+1 >= info.size() )
            return false;
        redis_info.pub_channel = info.substr( sindex+1, info.size()-sindex-1 );
        return true;
    }

    configuration::configuration()
    {

    }

    configuration::~configuration()
    {

    }

    int configuration::parse( const char* name )
    {
        size_t buf_len = 256;
        char buffer[256] = {0};
#ifdef HDTAS_WINDOWS
		FILE* file = NULL;
		errno_t t = fopen_s( &file, name, "r");
		if (0 != t)
#else
		FILE* file = fopen( name, "r" );
		if (NULL == file)
#endif // HDTAS_WINDOWS
		{
			this->strerr_ = "file("; this->strerr_ += name;
			this->strerr_ += ") fopen() failed, ";
			this->strerr_ += utility::err_str();
			return -1;
		}

        
        if ( NULL == file )
        {
            this->strerr_ = "file("; this->strerr_ += name;
            this->strerr_ += ") fopen() failed, ";
			this->strerr_ += utility::err_str();
            return -1;
        }

        // Lambda func, get the equal mark value.
        auto get_value = [&] ( const char* data ) mutable -> std::string{
            std::string temp = data;
            size_t index = temp.find( '=' );
            if ( std::string::npos == index )
            {
                this->strerr_ = "can't find the mark(=) in data(";
                this->strerr_ += data; this->strerr_ += ")";
                return "";
            }
            std::string rtn = temp.substr( index+1 );
            if ( rtn.size() > 0 )
            {
                if ( rtn[0] == '"' )
                    rtn.erase(0, 1 );
                if ( rtn.size() > 0 && rtn[rtn.size()-1] == '\r' )
                    rtn.erase( rtn.size()-1, 1 );
                if ( rtn.size() > 0 && rtn[rtn.size()-1] == '\n' )
                    rtn.erase( rtn.size()-1, 1 );
                if ( rtn.size() > 0 && rtn[rtn.size()-1] == '"' )
                    rtn.erase( rtn.size()-1, 1 );
            }

            if ( rtn.size() == 0 )
            {
                this->strerr_ = "invalid data(";
                this->strerr_ += data; this->strerr_ += ")";
            }
            return rtn;
        };

        int rtn = 0;
        while( fgets(buffer, (int)buf_len, file) != NULL )
        {
            if ( strlen(buffer) < 3 )
                continue;
            if ( STRNCASECMP(buffer, "#", 1) == 0 )
                continue;
            rtn = -1;
            if ( STRNCASECMP(buffer, NAME_CFG_TYPE, sizeof(NAME_CFG_TYPE)-1) == 0 )
            {
                std::string data = get_value( buffer );
                if ( data.size() == 0 ) break;
                rtn = 0;
                if (STRCASECMP(data.c_str(), "a") == 0 ) this->type_ = server_type::ST_ALONE;
                else if ( STRCASECMP(data.c_str(), "s") == 0 ) this->type_ = server_type::ST_SLAVER;
                else if ( STRCASECMP(data.c_str(), "m") == 0 ) this->type_ = server_type::ST_MASTER;
                else
                {
                    rtn = -1;
                    this->type_ = server_type::ST_UNKNOWN;
                    break;
                }
            }
            else if ( STRNCASECMP(buffer, NAME_CFG_ZK_INFO, sizeof(NAME_CFG_ZK_INFO)-1) == 0 )
            {
                std::string data = get_value( buffer );
                if ( data.size() == 0 ) break;
                this->zk_info_ = data;
                rtn = 0;
            }
            else if ( STRNCASECMP(buffer, NAME_CFG_SS_INFO, sizeof(NAME_CFG_SS_INFO)-1) == 0 )
            {
                std::string data = get_value( buffer );
                if ( data.size() == 0 ) break;
                bool brtn = parse_host_port_info( data, this->slaver_infos_ );
                if ( !brtn || this->slaver_infos_.size() == 0 )
                {
                    this->strerr_ = "can't parse the data("; this->strerr_ += buffer;
                    this->strerr_ == ")"; break;
                }
                else
                    rtn = 0;
            }
            else if ( STRNCASECMP(buffer, NAME_CFG_WT_INFO, sizeof(NAME_CFG_WT_INFO)-1) == 0 )
            {
                std::string data = get_value( buffer );
                if ( data.size() == 0 ) break;
                this->number_ = atoi( data.c_str() );
                rtn = 0;
            }
            else if ( STRNCASECMP(buffer, NAME_CFG_TCP_INFO, sizeof(NAME_CFG_TCP_INFO)-1) == 0 )
            {
                std::string data = get_value( buffer );
                if ( data.size() == 0 ) break;
                host_port_array hpa;
                bool brtn = parse_host_port_info( data, hpa );
                if ( !brtn || hpa.size() == 0 )
                {
                    this->strerr_ = "can't parse the data("; this->strerr_ += buffer;
                    this->strerr_ == ")"; break;
                }
                else
                {
                    this->tcp_info_.host = hpa[0].host;
                    this->tcp_info_.port = hpa[0].port;
                    rtn = 0;
                }
            }
            else if ( STRNCASECMP(buffer, NAME_CFG_UCP_INFO, sizeof(NAME_CFG_UCP_INFO)-1) == 0 )
            {
                std::string data = get_value( buffer );
                if ( data.size() == 0 ) break;
                host_port_array hpa;
                bool brtn = parse_host_port_info( data, hpa );
                if ( !brtn || hpa.size() == 0 )
                {
                    this->strerr_ = "can't parse the data("; this->strerr_ += buffer;
                    this->strerr_ == ")"; break;
                }
                else
                {
                    this->udp_info_.host = hpa[0].host;
                    this->udp_info_.port = hpa[0].port;
                    rtn = 0;
                }
            }
            else if ( STRNCASECMP(buffer, NAME_CFG_CONTROL_INFO, sizeof(NAME_CFG_CONTROL_INFO)-1) == 0 )
            {
                std::string data = get_value( buffer );
                if ( data.size() == 0 ) break;
                host_port_array hpa;
                bool brtn = parse_host_port_info( data, hpa );
                if ( !brtn || hpa.size() == 0 )
                {
                    this->strerr_ = "can't parse the data("; this->strerr_ += buffer;
                    this->strerr_ == ")"; break;
                }
                else
                {
                    this->control_info_.host = hpa[0].host;
                    this->control_info_.port = hpa[0].port;
                    rtn = 0;
                }
            }
            else if ( STRNCASECMP(buffer, NAME_CFG_WS_INFO, sizeof(NAME_CFG_WS_INFO)-1) == 0 )
            {
				
                std::string data = get_value( buffer );
                if ( data.size() == 0 ) break;
                host_port_array hpa;
                bool brtn = parse_host_port_info( data, hpa );
                if ( !brtn || hpa.size() == 0 )
                {
                    this->strerr_ = "can't parse the data("; this->strerr_ += buffer;
                    this->strerr_ == ")"; break;
                }
                else
                {
                    this->ws_info_.host = hpa[0].host;
                    this->ws_info_.port = hpa[0].port;
                    rtn = 0;
                }
            }
            else if ( STRNCASECMP(buffer, NAME_CFG_RMQ_INFO, sizeof(NAME_CFG_RMQ_INFO)-1) == 0 )
            {
                std::string data = get_value( buffer );
                if ( data.size() == 0 ) break;
                bool brtn = parser_redis_info( data, this->rmq_info_ );
                if ( !brtn )
                {
                    this->strerr_ = "can't parse the data("; this->strerr_ += buffer;
                    this->strerr_ == ")"; break;
                }
                else
                    rtn = 0;
            }
            else if ( STRNCASECMP(buffer, NAME_CFG_DB_INFO, sizeof(NAME_CFG_DB_INFO)-1) == 0 )
            {
                std::string data = get_value( buffer );
                if ( data.size() == 0 ) break;
                bool brtn = parser_mysql_info( data, this->db_info_ );
                if ( !brtn )
                {
                    this->strerr_ = "can't parse the data("; this->strerr_ += buffer;
                    this->strerr_ == ")"; break;
                }
                else
                    rtn = 0;
            }
            else if ( STRNCASECMP(buffer, NAME_CFG_MQ_INFO, sizeof(NAME_CFG_MQ_INFO)-1) == 0 )
            {
                std::string data = get_value( buffer );
                if ( data.size() == 0 ) break;
                bool brtn = parse_host_port_info( data, this->mq_infos_ );
                if ( !brtn || this->mq_infos_.size() == 0 )
                {
                    this->strerr_ = "can't parse the data("; this->strerr_ += buffer;
                    this->strerr_ == ")"; break;
                }
                else
                    rtn = 0;
            }
            else if ( STRNCASECMP(buffer, NAME_CFG_FS_INFO, sizeof(NAME_CFG_FS_INFO)-1) == 0 )
            {
                std::string data = get_value( buffer );
                if ( data.size() == 0 ) break;
                this->fs_info_ = data;
                rtn = 0;
            }
            else if ( STRNCASECMP(buffer, NAME_CFG_UF_INFO, sizeof(NAME_CFG_UF_INFO)-1) == 0 )
            {
                std::string data = get_value( buffer );
                if ( data.size() == 0 ) break;
                this->update_path_ = data;
                rtn = 0;
            }
            else if ( STRNCASECMP(buffer, NAME_CFG_LOG_PATH, sizeof(NAME_CFG_LOG_PATH)-1) == 0 )
            {
                this->log_path_ = get_value( buffer );
                rtn = 0;
            }
            else if ( STRNCASECMP(buffer, NAME_CFG_DAEMON, sizeof(NAME_CFG_DAEMON)-1) == 0 )
            {
                std::string data = get_value( buffer );
                if ( STRCASECMP(data.c_str(), "true") == 0 )
                    this->daemon_ = true;
                else
                    this->daemon_ = false; 
                rtn = 0;
            }
            else
                continue;
        }
        return rtn;
    }

    server_type configuration::get_server_type() const
    {
        return this->type_;
    }

    unsigned int configuration::get_worker_num() const
    {
        return this->number_;
    }

    const char* configuration::get_zk_info() const
    {
        return this->zk_info_.c_str();
    }

    const host_port_array& configuration::get_slaver_infos() const
    {
        return this->slaver_infos_;
    }

    const host_port_info& configuration::get_udp_info() const
    {
        return this->udp_info_;
    }

    const host_port_info& configuration::get_tcp_info() const
    {
        return this->tcp_info_;
    }

    const host_port_info& configuration::get_control_info() const
    {
        return this->control_info_;
    }

    const host_port_info& configuration::get_websocket_host() const
    {
        return this->ws_info_;
    }

    const redis_mq_info& configuration::get_rmq_info() const
    {
        return this->rmq_info_;
    }

    const data_base_info& configuration::get_db_info() const
    {
        return this->db_info_;
    }

    const host_port_array& configuration::get_mq_infos() const
    {
        return this->mq_infos_;
    }

    const char* configuration::get_fs_info() const
    {
        return this->fs_info_.c_str();
    }

    const char* configuration::get_update_file_path() const
    {
        return this->update_path_.c_str();
    }

    const char* configuration::get_log_path() const
    {
        return this->log_path_.c_str();
    }

    bool configuration::be_daemon() const
    {
        return this->daemon_;
    }


    const std::string& configuration::get_error() const
    {
        return this->strerr_;
    }
}

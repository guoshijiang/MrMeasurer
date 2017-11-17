#ifndef COMMON_H
#define COMMON_H

#include <QByteArray>
#include <QVector>
#include <QDataStream>
#include <QSharedPointer>
#include <QtEndian>
#include <QBuffer>

namespace mmr_conmunication {

    static quint16 udp_port_send = 18392;

    enum MR_UDP_OPERATOR {
        read_var = 0x10010001,  //设备参数读取指令码
        write_var = 0x10010002, //设备参数写入指令码
        reset_var = 0x10010003, //恢复出厂默认参数指令码
        restart = 0x10010004,   //重启指令码
    };

    template <typename T>
    class udp_message {
    public:
        udp_message(const T & data, MR_UDP_OPERATOR op)
            : m_operator(op)
            , m_data(data)
        {
            setHeader();
            setLength();
        }

        udp_message(const char * data, size_t len, MR_UDP_OPERATOR op)
            : m_operator(op)
            , m_data(&data[8], len - 8)
        {
            setHeader();
            setLength();
        }

        QByteArray toBinary() {
            quint32 len = m_length + 4;
            char * tmp = new char[len];
            quint32 off = 0;

            qToLittleEndian(m_header, tmp + off);
            off += sizeof(m_header);

            qToLittleEndian(m_length, tmp + off);
            off += sizeof(m_length);

            qToLittleEndian(m_operator, tmp + off);
            off += sizeof(m_operator);

            QByteArray data_bin = m_data.toBinary();
            for (int i = 0; i != m_data.getLength(); i++) {
                tmp[off++] = data_bin.at(i);
            }

            QByteArray res(tmp, len);
            delete tmp;
            return res;
        }

        void setLength() {
            m_length = m_data.getLength() + sizeof(m_operator);
        }
        quint16 getLength() { return m_length; }

        void setHeader() { m_header = 0x7EAA; }  //设置帧头
        quint16 getHeader() { return m_header;}  //获取帧头

        void set_operator(quint16 value) { m_operator = value; }
        quint32 get_operator() { return m_operator; }

        T & get_data() { return m_data; }
        void set_data(const T & value) { m_data = value; }

    private:
        quint16 m_header;
        quint16 m_length;
        quint32 m_operator;
        T       m_data;
    };

    class udp_data {
    public:
        typedef QSharedPointer<udp_data> ptr;

        udp_data(const char * str, size_t len) {
            mr_id.resize(4);    //基站ID
            mac.resize(6);      //默认MAC地址
            domain.resize(256); //服务器域名或IP地址，以字符串保存
            manufacturer_defined_message.resize(64); //客户自定义信息

            build(str, len);
        }

        udp_data() {
            mr_id.resize(4);
            mac.resize(6);
            domain.resize(256);
            manufacturer_defined_message.resize(64);
        }

        udp_data(const udp_data & data) {
            mr_id.resize(4);
            mac.resize(6);
            domain.resize(256);
            manufacturer_defined_message.resize(64);

            this->err_code = data.err_code;  //错误代码，0无错误，其他：有错误
            this->rev = data.rev;            //版本号
            this->mr_id = data.mr_id;        //基站ID
            this->ip = data.ip;              //默认IP（静态）
            this->netmask = data.netmask;    //默认子网掩码（静态）
            this->gateway = data.gateway;    //默认网关（静态）
            this->dns = data.dns;            //默认DNS服务器（静态）
            this->mac = data.mac;            //默认MAC地址
            this->ip_d = data.ip_d;          //当前IP
            this->netmask_d = data.netmask_d;//当前子网掩码
            this->gateway_d = data.gateway_d;//当前网关
            this->dns_d = data.dns_d;        //当前DNS服务器
            this->domain = data.domain;      //服务器域名或IP地址，以字符串保存
            this->tcp_port = data.tcp_port;   //服务器TCP端口
            this->udp_port = data.udp_port;    //服务器UDP端口
            this->network_select = data.network_select;  //网络工作模式选择（0 自动，1有线，2无线)
            this->sbs_status_report_interval = data.sbs_status_report_interval;//从基站状态上报频率，单位秒
            this->manufacturer_defined_message = data.manufacturer_defined_message;//客户自定义信息
            this->dip_value = data.dip_value; //拨码开关的当前值
        }

        udp_data & operator =(const udp_data & data) {
            this->err_code = data.err_code;
            this->rev = data.rev;
            this->mr_id = data.mr_id;
            this->ip = data.ip;
            this->netmask = data.netmask;
            this->gateway = data.gateway;
            this->dns = data.dns;
            this->mac = data.mac;
            this->ip_d = data.ip_d;
            this->netmask_d = data.netmask_d;
            this->gateway_d = data.gateway_d;
            this->dns_d = data.dns_d;
            this->domain = data.domain;
            this->tcp_port = data.tcp_port;
            this->udp_port = data.udp_port;
            this->network_select = data.network_select;
            this->sbs_status_report_interval = data.sbs_status_report_interval;
            this->manufacturer_defined_message = data.manufacturer_defined_message;
            this->dip_value = data.dip_value;
            return *this;
        }

        void build(const char * str, size_t len) {
            //err_code    = str[0] | (str[1] << 8) | (str[2] << 16) | (str[3] << 24);
            err_code    = *(quint32*)(str);
            //rev         = str[4] | (str[5] << 8) | (str[6] << 16) | (str[7] << 24);
            rev         = *(quint32*)(str+4);
            for (int i = 0; i != mr_id.size(); i++) { // 4
                mr_id[i] = (quint8)str[8+i];
            }
            //ip          = str[12] | (str[13] << 8) | (str[14] << 16) | (str[15] << 24);
            ip          = *(quint32*)(str+12);
            //netmask     = str[16] | (str[17] << 8) | (str[18] << 16) | (str[19] << 24);
            netmask     = *(quint32*)(str+16);
            //gateway     = str[20] | (str[21] << 8) | (str[22] << 16) | (str[23] << 24);
            gateway     = *(quint32*)(str+20);
            //dns         = str[24] | (str[25] << 8) | (str[26] << 16) | (str[27] << 24);
            dns         = *(quint32*)(str+24);
            for (int i = 0; i != mac.size(); i++) { // 6
                mac[i] = (quint8)str[28+i];
            }
            //ip_d        = str[34] | (str[35] << 8) | (str[36] << 16) | (str[37] << 24);
            ip_d        = *(quint32*)(str+34);
            //netmask_d   = str[38] | (str[39] << 8) | (str[40] << 16) | (str[41] << 24);
            netmask_d   = *(quint32*)(str+38);
            //gateway_d   = str[42] | (str[43] << 8) | (str[44] << 16) | (str[45] << 24);
            gateway_d   = *(quint32*)(str+42);
            //dns_d       = str[46] | (str[47] << 8) | (str[48] << 16) | (str[49] << 24);
            dns_d       = *(quint32*)(str+46);
            for (int i = 0; i != domain.size(); i++ ) { // 256
                domain[i] = str[50+i];
            }
            //quint32 tmp = *(quint32*)(str+306);
            //quint32 tmp2 = qToLittleEndian(tmp);
            //tcp_port    = str[306] | (str[307] << 8) | (str[308] << 16) | (str[309] << 24);
            tcp_port    = *(quint32*)(str+306);
            //udp_port    = str[310] | (str[311] << 8) | (str[312] << 16) | (str[313] << 24);
            udp_port    = *(quint32*)(str+310);
            network_select                = (quint8)str[314];
            sbs_status_report_interval    = (quint8)str[315];
            for (int i = 0; i != manufacturer_defined_message.size(); i++) { // 64
                manufacturer_defined_message[i] = (quint8)str[316+i];
            }
            dip_value = (quint8)str[380];
        }

        QByteArray toBinary() {

            char tmp[381] = {0};
            int off = 0;

            qToLittleEndian(err_code, tmp+off);
            off += sizeof(err_code);

            qToLittleEndian(rev, tmp+off);
            off += sizeof(rev);

            for (int i = 0; i != mr_id.size(); i++) {
                tmp[off++] = mr_id[i];
            }

            qToLittleEndian(ip, tmp+off);
            off += sizeof(ip);

            qToLittleEndian(netmask, tmp+off);
            off += sizeof(netmask);

            qToLittleEndian(gateway, tmp+off);
            off += sizeof(gateway);

            qToLittleEndian(dns, tmp+off);
            off += sizeof(dns);

            for (int i = 0; i != mac.size(); i++) {
                tmp[off++] = mac[i];
            }

            qToLittleEndian(ip_d, tmp+off);
            off += sizeof(ip_d);

            qToLittleEndian(netmask_d, tmp+off);
            off += sizeof(netmask_d);

            qToLittleEndian(gateway_d, tmp+off);
            off += sizeof(gateway_d);

            qToLittleEndian(dns_d, tmp+off);
            off += sizeof(dns_d);

            for (int i = 0; i != domain.size(); i++) {
                tmp[off++] = domain[i];
            }

            qToLittleEndian(tcp_port, tmp+off);
            off += sizeof(tcp_port);

            qToLittleEndian(udp_port, tmp+off);
            off += sizeof(udp_port);

            qToLittleEndian(network_select, tmp+off);
            off += sizeof(network_select);

            qToLittleEndian(sbs_status_report_interval, tmp+off);
            off += sizeof(sbs_status_report_interval);

            for (int i = 0; i != manufacturer_defined_message.size(); i++) {
                tmp[off++] = manufacturer_defined_message[i];
            }

            qToLittleEndian(dip_value, tmp+off);
            off += sizeof(dip_value);

            QByteArray res(tmp, 381);
            return res;
        }

        quint16 getLength() {
            return  sizeof(err_code)
                    + sizeof(rev)
                    + mr_id.size()
                    + sizeof(ip)
                    + sizeof(netmask)
                    + sizeof(gateway)
                    + sizeof(dns)
                    + mac.size()
                    + sizeof(ip_d)
                    + sizeof(netmask_d)
                    + sizeof(gateway_d)
                    + sizeof(dns_d)
                    + domain.size()
                    + sizeof(tcp_port)
                    + sizeof(udp_port)
                    + sizeof(network_select)
                    + sizeof(sbs_status_report_interval)
                    + manufacturer_defined_message.size()
                    + sizeof(dip_value);
        }

        quint32 get_err_code() { return err_code; }
        void set_err_code(quint32 value) { err_code = value; }

        quint32 get_rev() { return rev; }
        void set_rev(quint32 value) { rev = value; }

        QVector<quint8> get_mr_id() { return mr_id; }
        void set_mr_id(QVector<quint8> value) { mr_id = value; }

        quint32 get_ip() { return ip; }
        void set_ip(quint32 value) { ip = value; }

        quint32 get_netmask() { return netmask; }
        void set_netmask(quint32 value) { netmask = value; }

        quint32 get_gateway() { return gateway; }
        void set_gateway(quint32 value) { gateway = value; }

        quint32 get_dns() { return dns; }
        void set_dns(quint32 value) { dns = value; }

        QVector<quint8> get_mac() { return mac; }
        void set_mac(QVector<quint8> value) { mac = value; }

        quint32 get_ip_d() { return ip_d; }
        void set_ip_d(quint32 value) { ip_d = value; }

        quint32 get_netmask_d() { return netmask_d; }
        void set_netmask_d(quint32 value) { netmask_d = value; }

        quint32 get_gateway_d() { return gateway_d; }
        void set_gateway_d(quint32 value) { gateway_d = value; }

        quint32 get_dns_d() { return dns_d; }
        void set_dns_d(quint32 value) { dns_d = value; }

        QVector<qint8> get_domain() { return domain; }
        void set_domain(QVector<qint8> value) { domain = value; }

        quint32 get_tcp_port() { return tcp_port; }
        void set_tcp_port(quint32 value) { tcp_port = value; }

        quint32 get_udp_port() { return udp_port; }
        void set_udp_port(quint32 value) { udp_port = value; }

        quint8 get_network_select() { return network_select; }
        void set_network_select(quint8 value) { network_select = value; }

        quint8 get_sbs_status_report_interval() { return sbs_status_report_interval; }
        void set_sbs_status_report_interval(quint8 value) { sbs_status_report_interval = value; }

        QVector<quint8> get_manufacturer_message() { return manufacturer_defined_message; }
        void set_manufacturer_message(QVector<quint8> value) { manufacturer_defined_message = value; }

        quint8 get_dip_value() { return dip_value; }
        void set_dip_value(quint8 value) { dip_value = value; }

    private:
        quint32 err_code;
        quint32 rev;
        QVector<quint8>  mr_id;// 4
        quint32 ip;
        quint32 netmask;
        quint32 gateway;
        quint32 dns;
        QVector<quint8>  mac;// 6
        quint32 ip_d;
        quint32 netmask_d;
        quint32 gateway_d;
        quint32 dns_d;
        QVector<qint8>    domain;// 256
        quint32 tcp_port;
        quint32 udp_port;
        quint8  network_select;
        quint8  sbs_status_report_interval;
        QVector<quint8>  manufacturer_defined_message; // 64
        quint8  dip_value;
    };

    class udp_data2 {
    public:
        udp_data2(const char * str, size_t len) {
            build(str, len);
        }

        udp_data2() : err_code(0) {}
        udp_data2(const udp_data2 & data) {
            this->err_code = data.err_code;
        }
        udp_data2 & operator =(const udp_data2 & data) {
            this->err_code = data.err_code;
        }

        void build(const char * str, size_t len) {
            //err_code = str[0] | (str[1] << 8) | (str[2] << 16) | (str[3] << 24);
            err_code = *(quint32*)(str);
        }

        QByteArray toBinary() {
            char tmp[4] = {0};
            qToLittleEndian(err_code, tmp);

            QByteArray res(tmp, 4);
            return res;
        }

        quint32 getLength() {
            return sizeof(err_code);
        }

        quint32 get_err_code() { return err_code; }
        void set_err_code(quint32 value) { err_code = value; }

    private:
        quint32 err_code;
    };

#if 0
    template<typename T>
    struct mr_udp_message {
        explicit mr_udp_message(const T & data, MR_UDP_OPERATOR op)
            : m_data(data)
            , m_operator(op)
        {
            m_header = 0x7EAA;
            setLength();
        }

        void setOperator(MR_UDP_OPERATOR op) {
            m_operator = op;
        }

        void setLength() {
            m_length = m_data.length() + 8;
        }

        QByteArray toBinary() {
            QByteArray res;
            QDataStream data(&res, QIODevice::WriteOnly);

            data << m_header << m_length << m_operator << m_data;

            return res;
        }


        QByteArray& serial(const mr_udp_message<T> & data);
        mr_udp_message<T>& deserial(const unsigned char * data, quint32 len);

        friend QDataStream &operator >>(QDataStream & in, mr_udp_message & data) {
            in >> data.m_header >> data.m_length >> data.m_operator >> data.m_data;
            return in;
        }

        friend QDataStream &operator <<(QDataStream & out, const mr_udp_message & data) {
            out << data.m_header << data.m_length << data.m_operator << data.m_data;
            return out;
        }


        quint16 m_header;
        quint16 m_length;
        quint32 m_operator;
        T       m_data;
    };

    struct mr_udp_data {
        explicit mr_udp_data() {
        }
        ~mr_udp_data() {}

        quint32 length() {
            return sizeof(err_code) + sizeof(rev) + mr_id.size() + sizeof(ip) + sizeof(netmask) + sizeof(gateway) + sizeof(dns) + sizeof(dns) + mac.size() + sizeof(ip_d) + sizeof(netmask_d) + sizeof(gateway_d) + sizeof(dns_d);
                + domain.size() + sizeof(tcp_port) + sizeof(udp_port) + sizeof(network_select) + sizeof(sbs_status_report_interval) + manufactureer_defined_message.size() + sizeof(dip_value);
        }

        QByteArray& serial(const mr_udp_data & data);
        mr_udp_data& deserial(const unsigned char * data, quint32 len);

        friend QDataStream &operator >>(QDataStream & in, mr_udp_data & data) {
            in >> data.err_code >> data.rev >> data.mr_id >> data.ip >> data.netmask >> data.gateway >> data.dns >> data.mac >> data.ip_d >> data.netmask_d >> data.gateway_d >> data.dns_d
               >> data.domain >> data.tcp_port >> data.udp_port >> data.network_select >> data.sbs_status_report_interval >> data.manufactureer_defined_message >> data.dip_value;
            return in;
        }



        friend QDataStream &operator <<(QDataStream & out, const mr_udp_data & data) {
            out << data.err_code << data.rev << data.mr_id << data.ip << data.netmask << data.gateway << data.dns << data.mac << data.ip_d << data.netmask_d << data.gateway_d << data.dns_d
                << data.domain << data.tcp_port << data.udp_port << data.network_select << data.sbs_status_report_interval << data.manufactureer_defined_message << data.dip_value;
            return out;
        }

        quint32 err_code;
        quint32 rev;
        QVector<quint8>  mr_id;// 4
        quint32 ip;
        quint32 netmask;
        quint32 gateway;
        quint32 dns;
        QVector<quint8>  mac;// 6
        quint32 ip_d;
        quint32 netmask_d;
        quint32 gateway_d;
        quint32 dns_d;
        QVector<qint8>    domain;// 256
        quint32 tcp_port;
        quint32 udp_port;
        quint8  network_select;
        quint8  sbs_status_report_interval;
        QVector<quint8>  manufactureer_defined_message; // 64
        quint8  dip_value;
    };

    struct mr_udp_data2 {
        mr_udp_data2() {}

        quint32 length() {return sizeof(error_code);}

        friend QDataStream &operator >>(QDataStream & in, mr_udp_data2 & data) {
            in >> data.error_code;
            return in;
        }

        friend QDataStream &operator <<(QDataStream & out, const mr_udp_data2 & data) {
            out << data.error_code;
            return out;
        }

        quint32 error_code;
    };
#endif
}

#endif // COMMON_H

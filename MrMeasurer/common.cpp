
#include "common.h"
#include <QDebug>
#include <QBuffer>

namespace mmr_conmunication {

#if 0
QByteArray& mr_udp_data::serial(const mr_udp_data & data)
{
    QByteArray res;
    QBuffer buffer(&res);
    buffer.open(QIODevice::WriteOnly);

    QDataStream out(&buffer);
    out << data;

    //out << *this;
    buffer.close();

    return res;
}

mr_udp_data& mr_udp_data::deserial(const unsigned char *data, quint32 len)
{
    QByteArray array(reinterpret_cast<const char*>(data), len);

    QBuffer buffer(&array);
    buffer.open(QIODevice::ReadOnly);

    QDataStream in(&buffer);
    mr_udp_data mr_data;
    in >> mr_data;

   // in >> *this;
    buffer.close();

    return mr_data;
}

template <typename T>
QByteArray& mr_udp_message<T>::serial(const mr_udp_message<T> & data)
{
    QByteArray res;
    QBuffer buffer(&res);
    buffer.open(QIODevice::WriteOnly);

    QDataStream out(&buffer);
    mr_udp_message<mr_udp_data> mr_msg;
    out << mr_msg;
    buffer.close();

    return res;
}

template<typename T>
mr_udp_message<T>& mr_udp_message<T>::deserial(const unsigned char *data, quint32 len)
{
    QByteArray array(data, len);
    QBuffer buffer(&array);

    buffer.open(QIODevice::ReadOnly);

    QDataStream in(&buffer);

    mr_udp_message<mr_udp_data> mr_msg;
    in >> mr_msg;
    buffer.close();

    return mr_msg;
}

#endif


}//

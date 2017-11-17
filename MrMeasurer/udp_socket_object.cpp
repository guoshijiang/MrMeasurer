#include "udp_socket_object.h"
#include "../hdtasparser/hdtasparser/hdtasparser.h"
#include "../hdtasparser/hdtasparser/utility.h"

#include <QThread>

udp_socket_object::udp_socket_object(QObject *parent) : QObject(parent)
{

}

udp_socket_object::~udp_socket_object()
{
    if (m_udp_socket_ptr->isOpen()) {
        m_udp_socket_ptr->close();
    }

    m_udp_socket_ptr->deleteLater();
    emit stop();
}

void udp_socket_object::setupThread(QThread *thread)
{
    connect(thread, SIGNAL(started()), this, SLOT(doWork()));

    moveToThread(thread);
}

void udp_socket_object::doWork()
{
    m_udp_socket_ptr = new QUdpSocket(this);
    if (!m_udp_socket_ptr) {
        qFatal("can not assign memory any more");
        return ;
    }

    if (!m_udp_socket_ptr->bind(QHostAddress::Any, udp_port, QUdpSocket::ShareAddress)) {
        qCritical("udp bind failed");
        return ;
    }

    qInfo() << "udp server starts listening port[" << udp_port << "]";

    connect(m_udp_socket_ptr, SIGNAL(readyRead()), this, SLOT(handleConnections()));
}

void udp_socket_object::handleConnections()
{
    QByteArray res;
    res.resize(m_udp_socket_ptr->pendingDatagramSize());
    QHostAddress client_ip;
    quint16      client_port;
    m_udp_socket_ptr->readDatagram(res.data(), res.size(), &client_ip, &client_port);

    //qDebug() << "udp connected [" << client_ip.toString() << ":" << client_port << "]"
    //         <<  ", message(" << res.size() << ")[" << res << "]";

    emit sendUdpMessage(res);
}

// mr_udp_handler
mr_udp_handler::mr_udp_handler(QObject *parent) : udp_socket_object(parent)
{

}

void mr_udp_handler::doWork()
{
    m_udp_socket_ptr = new QUdpSocket(this);
    if (!m_udp_socket_ptr) {
        qFatal("can not assign memory any more");
        return ;
    }

    if (!m_udp_socket_ptr->bind(QHostAddress::Any, 18393, QUdpSocket::ShareAddress)) {
        qCritical("udp bind failed");
        return ;
    }

    qInfo() << "mr udp server starts listening port[" << 18393 << "]";

    connect(m_udp_socket_ptr, SIGNAL(readyRead()), this, SLOT(handleConnections()));
}

// mr_udp_listener
mr_udp_listener::mr_udp_listener(QObject *parent)
{

}

void mr_udp_listener::doWork()
{
    m_udp_socket_ptr = new QUdpSocket(this);
    if (!m_udp_socket_ptr) {
        qFatal("can not assign memory any more");
        return ;
    }

    if (!m_udp_socket_ptr->bind(QHostAddress::Any, 8888, QUdpSocket::ShareAddress)) {
        qCritical("udp bind failed");
        return ;
    }

    qInfo() << "mr udp server starts listening port[" << 8888 << "]";

    connect(m_udp_socket_ptr, SIGNAL(readyRead()), this, SLOT(handleConnections()));
}

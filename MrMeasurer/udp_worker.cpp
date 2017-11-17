#include "udp_worker.h"

//
udp_worker::udp_worker(QObject *parent) : QObject(parent)
{
}


udp_worker::~udp_worker()
{
    qDebug() << "udp worker closed";
}

bool udp_worker::init()
{
    m_udp_thread_ptr = new udp_socket_thread(this);
    if (!m_udp_thread_ptr) {
        qFatal("can not assign memory any more");
        return false;
    }

    connect(m_udp_thread_ptr, SIGNAL(stop()), m_udp_thread_ptr, SLOT(stopThread()));
    connect(m_udp_thread_ptr, SIGNAL(sendUdpMessage(QByteArray)), this, SIGNAL(sendUdpMessage(QByteArray)));

    m_udp_thread_ptr->startThread();

    return true;
}

// mr_udp_worker
mr_udp_worker::mr_udp_worker(QObject *parent) : QObject(parent)
{

}

mr_udp_worker::~mr_udp_worker()
{

}

bool mr_udp_worker::init()
{
    m_mr_udp_thread_ptr = new mr_udp_thread(this);
    if (!m_mr_udp_thread_ptr) {
        qFatal("can not assign memory any more");
        return false;
    }

    connect(m_mr_udp_thread_ptr, SIGNAL(stop()), m_mr_udp_thread_ptr, SLOT(stopThread()));
    connect(m_mr_udp_thread_ptr, SIGNAL(sendUdpMessage(QByteArray)), this, SIGNAL(sendUdpMessage(QByteArray)));

    m_mr_udp_thread_ptr->startThread();

    return true;
}

// mr_listen_worker
mr_listen_worker::mr_listen_worker(QObject *parent) : QObject(parent)
{

}

mr_listen_worker::~mr_listen_worker()
{

}

bool mr_listen_worker::init()
{
    m_mr_udp_listen_ptr = new mr_listen_thread(this);
    if (!m_mr_udp_listen_ptr) {
        qFatal("can not assign memory any more");
        return false;
    }

    connect(m_mr_udp_listen_ptr, SIGNAL(stop()), m_mr_udp_listen_ptr, SLOT(stopThread()));
    connect(m_mr_udp_listen_ptr, SIGNAL(sendUdpMessage(QByteArray)), this, SIGNAL(sendUdpMessage(QByteArray)));

    m_mr_udp_listen_ptr->startThread();

    return true;
}

#include "udp_socket_thread.h"

udp_socket_thread::udp_socket_thread(QObject *parent)
    : QObject(parent)
    , m_thread_ptr(new QThread(this))
    , m_udp_obj_ptr(new udp_socket_object(0))
{
    connect(m_udp_obj_ptr, SIGNAL(stop()), this, SIGNAL(stop()));
    connect(m_udp_obj_ptr, SIGNAL(sendUdpMessage(QByteArray)), this, SIGNAL(sendUdpMessage(QByteArray)));
}

udp_socket_thread::~udp_socket_thread()
{
    if (m_thread_ptr->isRunning()) {
        stopThread();
    }
}

void udp_socket_thread::startThread()
{
    m_udp_obj_ptr->setupThread(m_thread_ptr);
    m_thread_ptr->start();
}

void udp_socket_thread::stopThread()
{
    if (m_thread_ptr->isRunning()) {
        m_thread_ptr->quit();
        m_thread_ptr->wait();
    }

    m_thread_ptr->deleteLater();
    m_udp_obj_ptr->deleteLater();
}


// mr_udp_thread
mr_udp_thread::mr_udp_thread(QObject *parent)
    : QObject(parent)
    , m_thread_ptr(new QThread(this))
    , m_mr_udp_obj_ptr(new mr_udp_handler(0))
{
    connect(m_mr_udp_obj_ptr, SIGNAL(stop()), this, SIGNAL(stop()));
    connect(m_mr_udp_obj_ptr, SIGNAL(sendUdpMessage(QByteArray)), this, SIGNAL(sendUdpMessage(QByteArray)));
}

mr_udp_thread::~mr_udp_thread()
{
    if (m_thread_ptr->isRunning()) {
        stopThread();
    }
}

void mr_udp_thread::startThread()
{
    m_mr_udp_obj_ptr->setupThread(m_thread_ptr);
    m_thread_ptr->start();
}

void mr_udp_thread::stopThread()
{
    if (m_thread_ptr->isRunning()) {
        m_thread_ptr->quit();
        m_thread_ptr->wait();
    }

    m_thread_ptr->deleteLater();
    m_mr_udp_obj_ptr->deleteLater();
}

// mr_listen_thread
mr_listen_thread::mr_listen_thread(QObject *parent)
    : QObject(parent)
    , m_thread_ptr(new QThread(this))
    , m_mr_listen_ptr(new mr_udp_listener(0))
{
    connect(m_mr_listen_ptr, SIGNAL(stop()), this, SIGNAL(stop()));
    connect(m_mr_listen_ptr, SIGNAL(sendUdpMessage(QByteArray)), this, SIGNAL(sendUdpMessage(QByteArray)));
}

mr_listen_thread::~mr_listen_thread()
{
    if (m_thread_ptr->isRunning()) {
        stopThread();
    }
}

void mr_listen_thread::startThread()
{
    m_mr_listen_ptr->setupThread(m_thread_ptr);
    m_thread_ptr->start();
}

void mr_listen_thread::stopThread()
{
    if (m_thread_ptr->isRunning()) {
        m_thread_ptr->quit();
        m_thread_ptr->wait();
    }

    m_thread_ptr->deleteLater();
    m_mr_listen_ptr->deleteLater();
}

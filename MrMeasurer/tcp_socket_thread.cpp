#include "tcp_socket_thread.h"

tcp_socket_thread::tcp_socket_thread(qintptr descriptor, QObject *parent)
    : QObject(parent)
    , m_thread_ptr(new QThread(this))
    //, m_tcp_socket_object_ptr(new tcp_socket_object(descriptor))
    , m_descriptor(descriptor)
{
}

tcp_socket_thread::~tcp_socket_thread()
{
    if (m_thread_ptr->isRunning()) {
        stopThread();
    }
}

void tcp_socket_thread::startThread()
{
    m_tcp_socket_object_ptr = new tcp_socket_object(m_descriptor);
    if (!m_tcp_socket_object_ptr) {
        qCritical("can not assign any memory");
    }

    connect(m_tcp_socket_object_ptr, SIGNAL(stop()), this, SIGNAL(stop()));
    connect(m_tcp_socket_object_ptr, SIGNAL(sendMessage(QByteArray)), this, SIGNAL(sendTcpMessage(QByteArray)));
    connect(this, SIGNAL(write(const char*,qint16)), m_tcp_socket_object_ptr, SLOT(onWrite(const char*,qint16)));

    m_tcp_socket_object_ptr->setupThread(m_thread_ptr);
    m_thread_ptr->start();
}

void tcp_socket_thread::stopThread()
{
    if (m_thread_ptr->isRunning()) {
        m_thread_ptr->quit();
        m_thread_ptr->wait();
    }
    m_thread_ptr->deleteLater();
    m_tcp_socket_object_ptr->deleteLater();
}

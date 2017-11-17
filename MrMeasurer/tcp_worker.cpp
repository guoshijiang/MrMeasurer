#include "tcp_worker.h"
#include "tcp_socket_thread.h"

tcp_worker::tcp_worker(QObject * parent) : QTcpServer(parent)
{ }

tcp_worker::~tcp_worker()
{
    qDebug() << "tcp worker closed";
    this->close();
}

bool tcp_worker::init()
{
    if (!this->listen(QHostAddress::Any, tcp_port)) {
        qCritical() << "tcp server failed to listen port[" << tcp_port << "]";
        return false;
    }

    qInfo() << "tcp server starts listening port[" << tcp_port << "]";
    return true;
}

void tcp_worker::incomingConnection(qintptr socketDescriptor)
{
    tcp_socket_thread * thread_ptr = new tcp_socket_thread(socketDescriptor);

    connect(thread_ptr, SIGNAL(stop()), thread_ptr, SLOT(stopThread()));
    connect(thread_ptr, SIGNAL(sendTcpMessage(QByteArray)), this, SIGNAL(sendTcpMessage(QByteArray)));
    connect(this, SIGNAL(write(const char*,qint16)), thread_ptr, SIGNAL(write(const char*,qint16)));

    // start single thread to handle
    thread_ptr->startThread();
}

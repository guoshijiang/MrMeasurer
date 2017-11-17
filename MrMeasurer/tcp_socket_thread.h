#ifndef TCP_SOCKET_THREAD_H
#define TCP_SOCKET_THREAD_H

#include "tcp_socket_object.h"

#include <QObject>
#include <QThread>
#include <QTcpSocket>

class tcp_socket_thread : public QObject
{
    Q_OBJECT
public:
    explicit tcp_socket_thread(qintptr descriptor, QObject *parent = 0);
    ~tcp_socket_thread();

    void startThread();

signals:
    void stop();
    void sendTcpMessage(const QByteArray & json);
    void write(const char * data, qint16 len);

public slots:
    void stopThread();

private:
    QThread *           m_thread_ptr;
    tcp_socket_object * m_tcp_socket_object_ptr;
    qintptr m_descriptor;
};

#endif // TCP_SOCKET_THREAD_H

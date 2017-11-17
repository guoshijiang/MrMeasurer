#ifndef UDP_SOCKET_THREAD_H
#define UDP_SOCKET_THREAD_H

#include "udp_socket_object.h"

#include <QObject>
#include <QThread>

// udp_socket_thread
class udp_socket_thread : public QObject
{
    Q_OBJECT

public:
    explicit udp_socket_thread(QObject * parent = 0);
    ~udp_socket_thread();

    void startThread();

signals:
    void stop();
    void sendUdpMessage(const QByteArray & json);

public slots:
    void stopThread();

private:
    QThread *           m_thread_ptr;
    udp_socket_object * m_udp_obj_ptr;
};

// mr_udp_thread
class mr_udp_thread : public QObject {
    Q_OBJECT
public:
    explicit mr_udp_thread(QObject * parent = 0);
    ~mr_udp_thread();

    void startThread();

signals:
    void stop();
    void sendUdpMessage(const QByteArray & json);

public slots:
    void stopThread();

private:
    QThread *           m_thread_ptr;
    mr_udp_handler *    m_mr_udp_obj_ptr;
};

// mr_listen_thread
class mr_listen_thread : public QObject {
    Q_OBJECT
public:
    explicit mr_listen_thread(QObject * parent = 0);
    ~mr_listen_thread();

    void startThread();

signals:
    void stop();
    void sendUdpMessage(const QByteArray & json);

public slots:
    void stopThread();

private:
    QThread *           m_thread_ptr;
    mr_udp_listener *   m_mr_listen_ptr;
};

#endif // UDP_SOCKET_THREAD_H

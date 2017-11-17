#ifndef UDP_WORKER_H
#define UDP_WORKER_H

#include "udp_socket_thread.h"

#include <QObject>

// udp_worker
class udp_worker : public QObject
{
    Q_OBJECT
public:
    explicit udp_worker(QObject *parent = 0);
    ~udp_worker();

    bool init();

signals:
    void sendUdpMessage(const QByteArray & json);

public slots:

private:
    udp_socket_thread * m_udp_thread_ptr;
};


// mr_udp_worker
class mr_udp_worker : public QObject {
    Q_OBJECT
public:
    explicit mr_udp_worker(QObject * parent = 0);
    ~mr_udp_worker();

    bool init();

signals:
    void sendUdpMessage(const QByteArray & json);
public slots:

private:
    mr_udp_thread * m_mr_udp_thread_ptr;
};

// mr_udp_listener
class mr_listen_worker : public QObject {
    Q_OBJECT
public:
    explicit mr_listen_worker(QObject * parent = 0);
    ~mr_listen_worker();

    bool init();
signals:
    void sendUdpMessage(const QByteArray & json);
public slots:

private:
    mr_listen_thread * m_mr_udp_listen_ptr;
};

#endif // UDP_WORKER_H

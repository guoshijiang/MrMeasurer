#ifndef UDP_SOCKET_OBJECT_H
#define UDP_SOCKET_OBJECT_H

#include <QObject>
#include <QUdpSocket>

class QThread;

const quint16 udp_port = 39527;

class udp_socket_object : public QObject
{
    Q_OBJECT
public:
    explicit udp_socket_object(QObject * parent = 0);
    virtual ~udp_socket_object();

    void setupThread(QThread * thread);
signals:
    void stop();
    void sendUdpMessage(const QByteArray & json);

public slots:
    void doWork();
    void handleConnections();

protected:
    QUdpSocket  * m_udp_socket_ptr;
};

class mr_udp_handler : public udp_socket_object {
    Q_OBJECT
public:
    explicit mr_udp_handler(QObject * parent = 0);

signals:

public slots:
    void doWork();

};

class mr_udp_listener : public udp_socket_object {
    Q_OBJECT
public:
    explicit mr_udp_listener(QObject * parent = 0);

public slots:
    void doWork();
};

#endif // UDP_SOCKET_OBJECT_H

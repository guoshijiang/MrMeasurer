#ifndef TCP_SOCKET_OBJECT_H
#define TCP_SOCKET_OBJECT_H

#include <QObject>
#include <QThread>
#include <QTcpSocket>
#include <QHostAddress>

class tcp_socket_object : public QObject
{
    Q_OBJECT
public:
    explicit tcp_socket_object(qintptr socketDescriptor, QObject *parent = 0);
    ~tcp_socket_object();

    void setupThread(QThread * thread);

signals:
    void stop();
    void error(QTcpSocket::SocketError socketError);

    void sendMessage(const QByteArray & json);

public slots:
    void doWork();

    void onDisconnected();

    void onReadyRead();
    void onWrite(const char * data, qint16 len);

private:
    qintptr      m_descriptor;
    QTcpSocket * m_tcp_socket_ptr;
};

#endif // TCP_SOCKET_OBJECT_H

#ifndef TCP_WORKER_H
#define TCP_WORKER_H

#include <QObject>
#include <QTcpServer>

static quint16 tcp_port = 39527;

class tcp_worker : public QTcpServer
{
    Q_OBJECT

public:
    explicit tcp_worker(QObject * parent = 0);
    ~tcp_worker();

    bool init();

protected:
    void incomingConnection(qintptr socketDescriptor) override;

signals:
    void sendTcpMessage(const QByteArray & json);
    void write(const char * data, qint16 len);
public slots:

};

#endif // TCP_WORKER_H

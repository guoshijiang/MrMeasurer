#include "tcp_socket_object.h"
#include "../hdtasparser/hdtasparser/hdtasparser.h"
#include "../hdtasparser/hdtasparser/utility.h"

tcp_socket_object::tcp_socket_object(qintptr socketDescriptor, QObject *parent)
    : QObject(parent)
    , m_descriptor(socketDescriptor)
    , m_tcp_socket_ptr(new QTcpSocket(this))
{

}

tcp_socket_object::~tcp_socket_object()
{
    emit stop();
    m_tcp_socket_ptr->close();
    delete m_tcp_socket_ptr;
    m_tcp_socket_ptr = nullptr;
}

void tcp_socket_object::setupThread(QThread *thread)
{
    connect(thread, SIGNAL(started()), this, SLOT(doWork()));
    connect(thread, SIGNAL(finished()), thread, SLOT(deleteLater()));
    connect(this, SIGNAL(stop()), thread, SLOT(quit()));

    moveToThread(thread);
}

void tcp_socket_object::doWork()
{
    if (!m_tcp_socket_ptr->setSocketDescriptor(m_descriptor)) {
        emit error(m_tcp_socket_ptr->error());
        return ;
    }

    connect(m_tcp_socket_ptr, SIGNAL(disconnected()), this, SLOT(onDisconnected()), Qt::DirectConnection);
    connect(m_tcp_socket_ptr, SIGNAL(readyRead()), this, SLOT(onReadyRead()), Qt::DirectConnection);
}

void tcp_socket_object::onDisconnected()
{
    qInfo() << "tcp disconnected [" << m_tcp_socket_ptr->peerAddress().toString()
            << ":" << m_tcp_socket_ptr->peerPort() << "]";

    m_tcp_socket_ptr->close();

    emit stop();
}

void tcp_socket_object::onReadyRead()
{
    QByteArray res;
    res.resize(m_tcp_socket_ptr->bytesAvailable());
    res = m_tcp_socket_ptr->readAll();

    //qDebug() << "tcp message: " << res;

    // send
    hdtas::HdtasPackage send_pkg;
    hdtas::HdtasMessage send_msg;
    std::pair<const unsigned char *, size_t> send_to;
    const char * response_pack = nullptr;
    int cnt = 0;

    // recv
    hdtas::HdtasPackage package;
    size_t length = 0;
    package.Unpack(reinterpret_cast<const unsigned char *>(res.constData()),
                   static_cast<size_t>(res.size()),
                   length);
    hdtas::HdtasMessage message;
    message.Deserialize(&package);

    hdtas::HDTAS_MSG_TYPE type = message.GetType();

    switch(message.GetType()) {
    case hdtas::HDTAS_MSG_TYPE::HCT_REG_REQ: {
        qDebug() << "reg request";

        hdtas::HdtasRegResponse reg_response;
        reg_response.SetMMrID(message.GetMMrID());
        reg_response.SetMsgID(message.GetMsgID());
        reg_response.SetEcode(0);
        hdtas::ispd_date_time date;
        reg_response.SetDateTime(date);

        reg_response.StartSerialize();
        cnt = reg_response.GetSerializeCount();
        while (cnt--) {
            reg_response.Serialize(&send_msg);
            send_msg.Serialize(&send_pkg);
            send_to = send_pkg.Pack();
            response_pack = reinterpret_cast<const char *>(send_to.first);

            m_tcp_socket_ptr->write(response_pack, send_to.second);
            m_tcp_socket_ptr->waitForBytesWritten();
        }
    } break;
    case hdtas::HDTAS_MSG_TYPE::HCT_REG_CFM: {
        qDebug() << "reg confirm";
    } break;
    case hdtas::HDTAS_MSG_TYPE::HCT_HB_REQ: {
        qDebug() << "hb request";

        hdtas::HdtasHeartBeatResponse hb_response;
        hb_response.SetMMrID(message.GetMMrID());
        hb_response.SetMsgID(message.GetMsgID());

        hb_response.StartSerialize();
        cnt = hb_response.GetSerializeCount();
        while (cnt--) {
            hb_response.Serialize(&send_msg);
            send_msg.Serialize(&send_pkg);
            send_to = send_pkg.Pack();
            response_pack = reinterpret_cast<const char *>(send_to.first);

            m_tcp_socket_ptr->write(response_pack, send_to.second);
            m_tcp_socket_ptr->waitForBytesWritten();
        }
    } break;
    case hdtas::HDTAS_MSG_TYPE::HMT_RD_CFG_REQ: {} break;
    case hdtas::HDTAS_MSG_TYPE::HMT_WT_CFG_RPN: {} break;
    case hdtas::HDTAS_MSG_TYPE::HMT_CTL_RPN: {
        qDebug() << "tcp ctl response";

        emit sendMessage(res);

    } break;
    case hdtas::HDTAS_MSG_TYPE::HMT_UNKNOWN: {
        qDebug() << "unknown type";
    } break;
    }

    //emit sendMessage(res);
}

void tcp_socket_object::onWrite(const char *data, qint16 len)
{
    if (!m_tcp_socket_ptr->isOpen()) {
        qWarning() << "tcp socket has not been initialized yet";
        m_tcp_socket_ptr->setSocketDescriptor(m_descriptor);
        //return;
    }

    m_tcp_socket_ptr->write(data, len);
    m_tcp_socket_ptr->waitForBytesWritten();
}

#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "../hdtasparser/hdtasparser/utility.h"

#include <QStandardItem>
#include <QElapsedTimer>
#include <QDate>
#include <QFont>
#include <QStringList>
#include <QNetworkInterface>
#include <QMessageBox>
#include <atomic>

#define  UPDATE_PB_INTERVAL     (1)
#define  TABLE_FONT             (22)
#define  TIMEOUT				(5)

#define MESSAGE_RED "<font size = 200 color = red ><strong>"
#define MESSAGE_GREEN "<font size = 200 color = blue ><strong>"
#define MESSAGE_END "</strong></font>"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    ui->splitter->setStretchFactor(0, 1);

    ui->lineEdit_mmr_id->setDisabled(true);
    ui->lineEdit_mr_id1->setDisabled(true);
    ui->lineEdit_mr_id2->setDisabled(true);
    ui->lineEdit_id_c->setDisabled(true);

    ui->pushButton_ok->setDisabled(true);
    ui->pushButton_start->setDisabled(true);
    //ui->pushButton->setDisabled(true);

    m_is_master = false;

    if (ui->radioButton->isChecked())
    {
        on_radioButton_clicked();
    }
    if (ui->radioButton_2->isChecked())
    {
        on_radioButton_2_clicked();
    }

    QRegExp reg3("^[B][S][0-9]*$");
    QValidator *validator3 = new QRegExpValidator(reg3, this->ui->lineEdit_id_c);
    ui->lineEdit_id_c->setValidator( validator3 );

    QValidator *validator2 = new QRegExpValidator(reg3, this->ui->lineEdit_mmr_id);
    ui->lineEdit_mmr_id->setValidator( validator2 );

    QValidator *validator1 = new QRegExpValidator(reg3, this->ui->lineEdit_mr_id1);
    ui->lineEdit_mr_id1->setValidator( validator1 );

    QValidator *validator0 = new QRegExpValidator(reg3, this->ui->lineEdit_mr_id2);
    ui->lineEdit_mr_id2->setValidator( validator0 );

    //add by guosj start
    QString new_ip;
    QList<QHostAddress> list = QNetworkInterface::allAddresses();
    foreach (QHostAddress address, list)
    {
        if (address.protocol() == QAbstractSocket::IPv4Protocol)
        {
            if (address.toString().contains("127.0.")) continue;
            new_ip = address.toString();
        }
    }
    ui->lineEdit_ip_c->setText(new_ip);
    //add by guosj end
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::start()
{
    this->showMaximized();
    m_has_ispd = false;
    m_has_mr = false;
    m_has_pb = false;

    //data
    //tcp
    m_tcp_worker_ptr = new tcp_worker(this);
    if (!m_tcp_worker_ptr)
    {
        qFatal("can not assign memory any more");
        return ;
    }
    connect(m_tcp_worker_ptr, SIGNAL(sendTcpMessage(QByteArray)), this, SLOT(onTcpMessage(QByteArray)));
    connect(this, SIGNAL(writeTcpMessage(const char*,qint16)), m_tcp_worker_ptr, SIGNAL(write(const char*,qint16)));

    if (!m_tcp_worker_ptr->init())
    {
        qCritical() << "tcp server init failed";
        return ;
    }
    qInfo() << "tcp server started";

    //udp
    m_udp_worker_ptr = new udp_worker(this);
    if (!m_udp_worker_ptr)
    {
        qFatal("can not assign memory any more");
        return ;
    }
    connect(m_udp_worker_ptr, SIGNAL(sendUdpMessage(QByteArray)), this, SLOT(onUdpMessage(QByteArray)));

    if (!m_udp_worker_ptr->init())
    {
        qCritical() << "udp server init failed";
        return ;
    }
    qInfo() << "udp server started";

    // mr_udp_worker
    m_mr_udp_worker_ptr = new mr_udp_worker(this);
    if (!m_mr_udp_worker_ptr)
    {
        qFatal("can not assign memory any more");
        return ;
    }
    connect(m_mr_udp_worker_ptr, SIGNAL(sendUdpMessage(QByteArray)), this, SLOT(onMrUdpMessage(QByteArray)));
    if (!m_mr_udp_worker_ptr->init())
    {
        qCritical() << "mr udp server init failed";
        return;
    }
    qInfo() << "mr udp server started";
#if 0
    // mr_listen_worker
    m_mr_listen_worker_ptr = new mr_listen_worker(this);
    if (!m_mr_listen_worker_ptr) {
        qFatal("can not assign memory any more");
        return ;
    }
    connect(m_mr_listen_worker_ptr, SIGNAL(sendUdpMessage(QByteArray)), this, SLOT(onMrUdpListenMessage(QByteArray)));
    if (!m_mr_listen_worker_ptr->init()) {
        qCritical() << "mr udp server init failed";
        return;
    }
    qInfo() << "mr udp listen server started";

#endif

    // model
    // udp ispd & udp pb & tcp mr
    m_model_ptr = new QStandardItemModel(this);
    if (!m_model_ptr)
    {
        qFatal("can not assign memory any more");
        return ;
    }
    // list view , display details of testing processes
    m_model_list_ptr = new QStandardItemModel(this);
    if (!m_model_list_ptr)
    {
        qFatal("can not assign memory any more");
        return ;
    }

    // view
    // tableView
    ui->tableView->verticalHeader()->hide();
    ui->tableView->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    ui->tableView->setModel(m_model_ptr);
    //listView
    ui->listView->setModel(m_model_list_ptr);
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    m_tcp_worker_ptr->deleteLater();
    qInfo() << "tcp server stopped";

    m_udp_worker_ptr->deleteLater();
    qInfo() << "udp server stopped";

    m_mr_udp_worker_ptr->deleteLater();
    qInfo() << "mr udp server stopped";

    event->accept();
}

void MainWindow::onTcpMessage(const QByteArray & res)
{
    hdtas::HdtasPackage package;
    size_t length = 0;
    package.Unpack(reinterpret_cast<const unsigned char *>(res.constData()),
                   static_cast<size_t>(res.size()),
                   length);
    hdtas::HdtasMessage message;
    message.Deserialize(&package);
    m_mmr_id = message.GetMMrID();
    switch (message.GetType())
    {
        case hdtas::HDTAS_MSG_TYPE::HMT_CTL_RPN:
        {
            hdtas::HdtasCtlMrPbResponse mr_pb_response;
            mr_pb_response.Deserialize(&message);
            switch (mr_pb_response.GetResponseType())
            {
                 //基站(mr)
                case hdtas::HDTAS_CTL_TYPE::HCT_MR_RD_AR_STU:
                {
                    qDebug() << "mr ar stu";
                } break;

                case hdtas::HDTAS_CTL_TYPE::HCT_MR_RD_AR_ITR:
                {
                    qDebug() << "read mr interval ";
                    //m_mr_ar = mr_pb_response.GetAutoReportInterval();
                    m_mr_interval.insert(mr_pb_response.GetMrID(), mr_pb_response.GetAutoReportInterval());
                } break;

                case hdtas::HDTAS_CTL_TYPE::HCT_MR_RD_STU_INFO:
                {
                    qDebug() << "mr stu";
                    hdtas::mr_status_array mr_array = mr_pb_response.GetStatusInfo();
                    m_mr_status_array = mr_array;
                    m_has_mr = true;
#if 0
                    m_model_ptr->setHorizontalHeaderLabels(QStringList()
                                                   << tr("终端编号")
                                                   << tr("终端状态"));
                     for (auto elem : mr_array)
                     {
                        QString id_str = QString(hdtas::utility::int_2_hex(elem.id).c_str());
                        QString status_str = QString::number(elem.status, 10);

                        QStandardItem * item_id = new QStandardItem(id_str);
                        QStandardItem * item_status = new QStandardItem(status_str);

                        m_model_ptr->appendRow(item_id);
                        m_model_ptr->setItem(m_model_ptr->indexFromItem(item_id).row(), 1, item_status);
                     }
                     //ui->tableView->setModel(m_model_ptr);
#endif
                } break;
                //电源控制板(pb)
                case hdtas::HDTAS_CTL_TYPE::HCT_PB_RD_AR_STU:
                {
                     qDebug() << "pb ar stu";
                } break;

                case hdtas::HDTAS_CTL_TYPE::HCT_PB_RD_AR_ITR:
                {
                    qDebug() << "read pb interval";
                    //m_pb_ar = mr_pb_response.GetAutoReportInterval2();
                    m_pb_interval.insert(mr_pb_response.GetMrID(), mr_pb_response.GetAutoReportInterval2());
                } break;

                case hdtas::HDTAS_CTL_TYPE::HCT_PB_RD_STU_INFO:
                {
                    qDebug() << "pb status";
                    hdtas::HdtasCtlMrPbResponse pb;
                    pb.Deserialize(&message);
                    device_mr_id id = pb.GetMrID();
                    hdtas::power_board_status pb_stu = pb.GetStatusInfo2();

                    QHash<CMPB_TYPE, bool> cmpb;
                    cmpb.insert(CMPB_TYPE::CAM, pb_stu.camera_);
                    cmpb.insert(CMPB_TYPE::MIC, pb_stu.microphone_);
                    cmpb.insert(CMPB_TYPE::PRE, pb_stu.preload_);
                    cmpb.insert(CMPB_TYPE::BAT, pb_stu.battery_control_);
                    m_cmpb_list.insert(id, cmpb);
                } break;

                case hdtas::HDTAS_CTL_TYPE::HCT_UNKNOWN:
                {
                    qDebug() << "tcp : unkown control type";
                } break;
            }
        }
    }
#if 0
    // test send tcp message
    QByteArray msg = "tcp server echo msg[";
    msg += json + "]";
    emit writeTcpMessage(msg.data(), msg.size());
#endif
}

void MainWindow::onUdpMessage(const QByteArray &res)
{
    hdtas::HdtasPackage package;
    size_t length = 0;
    package.Unpack(reinterpret_cast<const unsigned char *>(res.constData()),
                   static_cast<size_t>(res.size()),
                   length);
    hdtas::HdtasMessage message;
    message.Deserialize(&package);
    hdtas::HDTAS_MSG_TYPE type = message.GetType();
    switch (message.GetType())
    {
        case hdtas::HDTAS_MSG_TYPE::HMT_ISPD:
        {
            m_has_ispd = true;
#if 0
            m_model_ptr->setHorizontalHeaderLabels(QStringList()
                                               << tr("超级板编号")
                                               << tr("电量")
                                               << tr("充电状态")
                                               << tr("心率")
                                               << tr("加速度")
                                               << tr("陀螺仪")
                                               << tr("时间戳")
                                               << tr("时间差"));

            hdtas::HdtasIspdData ispd;
            ispd.Deserialize(&message);
            hdtas::isdp_data_array ispd_array = ispd.GetDataInfo();
            hdtas::ispd_date_time ispd_time = ispd.GetTimestamp();
            for (auto elem : ispd_array)
            {
                device_mr_id ispd_id = elem.id;
                qint32  ispd_power   = elem.power;
                qint32  ispd_charge  = elem.charge;
                qint32  ispd_hr      = elem.heart_rate;
                qint16  ispd_a_x = elem.acceleration.x;
                qint16  ispd_a_y = elem.acceleration.y;
                qint16  ispd_a_z = elem.acceleration.z;
                qint16  ispd_g_x = elem.gyroscope.x;
                qint16  ispd_g_y = elem.gyroscope.y;
                qint16  ispd_g_z = elem.gyroscope.z;
                hdtas::mr_time_diff_array ispd_tds = elem.tds;
                QString ispd_tds_str;
                for (auto index : ispd_tds)
                {
                    ispd_tds_str.append("[")
                            .append(QString::number(index.id, 16)).append(",")
                            .append(QString::number(index.td, 10)).append("];");
                }

                QString ispd_ac = QString("%1,%2,%3").arg(ispd_a_x).arg(ispd_a_y).arg(ispd_a_z);
                QString ispd_gy = QString("%1,%2,%3").arg(ispd_g_x).arg(ispd_g_y).arg(ispd_g_z);
                QString ispd_ts = QString("%1-%2-%3 %4:%5:%6:%7").arg(ispd_time.y).arg(ispd_time.m).arg(ispd_time.d).arg(ispd_time.h).arg(ispd_time.n).arg(ispd_time.s).arg(ispd_time.ms);

                QStandardItem * item_id     = new QStandardItem(QString(hdtas::utility::int_2_hex(ispd_id).c_str()));
                QStandardItem * item_power  = new QStandardItem(QString::number(ispd_power, 10));
                QStandardItem * item_charge = new QStandardItem(QString::number(ispd_charge, 10));
                QStandardItem * item_hr     = new QStandardItem(QString::number(ispd_hr, 10));
                QStandardItem * item_ac     = new QStandardItem(ispd_ac);
                QStandardItem * item_gy     = new QStandardItem(ispd_gy);
                QStandardItem * item_time   = new QStandardItem(ispd_ts);
                QStandardItem * item_tds    = new QStandardItem(ispd_tds_str);

                m_model_ptr->appendRow(item_id);
                m_model_ptr->setItem(m_model_ptr->indexFromItem(item_id).row(), 1, item_power);
                m_model_ptr->setItem(m_model_ptr->indexFromItem(item_id).row(), 2, item_charge);
                m_model_ptr->setItem(m_model_ptr->indexFromItem(item_id).row(), 3, item_hr);
                m_model_ptr->setItem(m_model_ptr->indexFromItem(item_id).row(), 4, item_ac);
                m_model_ptr->setItem(m_model_ptr->indexFromItem(item_id).row(), 5, item_gy);
                m_model_ptr->setItem(m_model_ptr->indexFromItem(item_id).row(), 6, item_time);
                m_model_ptr->setItem(m_model_ptr->indexFromItem(item_id).row(), 7, item_tds);
            }
#endif
        } break;

        case hdtas::HDTAS_MSG_TYPE::HMT_MR_STU:
        {
           m_has_mr = true;
        }break;

        case hdtas::HDTAS_MSG_TYPE::HMT_PB_STU:
        {
            m_has_pb = true;
#if 0
            m_model_ptr->setHorizontalHeaderLabels(QStringList()
                                           << tr("主控板编号")
                                           << tr("网络")
                                           << tr("主动上报")
                                           << tr("上报间隔")
                                           << tr("拾音器")
                                           << tr("摄像头")
                                           << tr("摄像头电压")
                                           << tr("摄像头电流")
                                           << tr("UWB板")
                                           << tr("UWB板电压")
                                           << tr("UWB板电流")
                                           << tr("交换机")
                                           << tr("交换机电压")
                                           << tr("交换机电流")
                                           << tr("路由器")
                                           << tr("路由器电压")
                                           << tr("路由器电流")
                                           << tr("预留负载")
                                           << tr("预留负载电压")
                                           << tr("预留负载电流")
                                           << tr("供电方式") // source
                                           << tr("电池状态") // batt_charge
                                           << tr("电池开关") // batt_conn
                                           << tr("电池控制") // batt_crtl
                                           << tr("电池容量") // batt_cap
                                           << tr("电池电压") // batt_volt
                                           << tr("是否满电")
                                           << tr("箱内温度"));
            hdtas::HdtasPbStatus pb;
            pb.Deserialize(&message);
            device_mr_id id = pb.GetMrID();
            hdtas::power_board_status pb_stu = pb.GetStatusInfo();
            hdtas::power_board_status_group pb_stu_grp = pb.GetStatusGroupInfo();

            QString pb_internet     = pb_stu.internet_ ? tr("开启") : tr("关闭");
            quint8 pb_auto_report   = pb_stu_grp.auto_report_;
            quint8 pb_auto_interval = pb_stu_grp.auto_report_interval_;
            QString pb_micro        = pb_stu.microphone_ ? tr("开启") : tr("关闭");
            QString pb_camera       = pb_stu.camera_ ? tr("开启") : tr("关闭");
            quint16 pb_camera_volt  = pb_stu_grp.camera_.first;
            quint16 pb_camera_cap   = pb_stu_grp.camera_.second;
            QString pb_locate       = pb_stu.locate_ ? tr("开启") : tr("关闭");
            quint16 pb_locate_volt  = pb_stu_grp.locate_.first;
            quint16 pb_locate_cap   = pb_stu_grp.locate_.second;
            QString pb_switch       = pb_stu.switch_ ? tr("开启") : tr("关闭");
            quint16 pb_switch_volt  = pb_stu_grp.switch_.first;
            quint16 pb_switch_cap   = pb_stu_grp.switch_.second;
            QString pb_router       = pb_stu.router_ ? tr("开启") : tr("关闭");
            quint16 pb_router_volt  = pb_stu_grp.router_.first;
            quint16 pb_router_cap   = pb_stu_grp.router_.second;
            QString pb_preload      = pb_stu.preload_ ? tr("开启") : tr("关闭");
            quint16 pb_preload_volt = pb_stu_grp.preload_.first;
            quint16 pb_preload_cap  = pb_stu_grp.preload_.second;
            QString pb_source       = pb_stu.source_ ? tr("电池") : tr("外接电源");
            QString pb_batt_cha     = pb_stu.battery_charge_ ? tr("充电") : tr("放电");
            QString pb_batt_conn    = pb_stu.battery_connect_ ? tr("电池已连接") : tr("电池断开连接");
            QString pb_batt_ctrl    = pb_stu.battery_control_ ? tr("强制") : tr("温度");
            quint16 pb_batt_volt    = pb_stu_grp.battery_.first;
            quint16 pb_batt_cap     = pb_stu_grp.battery_.second;
            QString pb_batt_stu     = pb_stu.battery_status_ ? tr("满电") : tr("非满电");
            qint16  pb_temperature  = pb_stu_grp.temperature_;

            QStandardItem * item_id = new QStandardItem(QString(hdtas::utility::int_2_hex(id).c_str()));
            QStandardItem * item_internet = new QStandardItem(pb_internet);
            QStandardItem * item_ar = new QStandardItem(QString::number(pb_auto_report, 10));
            QStandardItem * item_ar_inter = new QStandardItem(QString::number(pb_auto_interval, 10));
            QStandardItem * item_mic = new QStandardItem(pb_micro);
            QStandardItem * item_cam = new QStandardItem(pb_camera);
            QStandardItem * item_cam_v = new QStandardItem(QString::number(pb_camera_volt, 10));
            QStandardItem * item_cam_c = new QStandardItem(QString::number(pb_camera_cap, 10));
            QStandardItem * item_loc = new QStandardItem(pb_locate);
            QStandardItem * item_loc_v = new QStandardItem(QString::number(pb_locate_volt, 10));
            QStandardItem * item_loc_c = new QStandardItem(QString::number(pb_locate_cap, 10));
            QStandardItem * item_swi = new QStandardItem(pb_switch);
            QStandardItem * item_swi_v = new QStandardItem(QString::number(pb_switch_volt, 10));
            QStandardItem * item_swi_c = new QStandardItem(QString::number(pb_switch_cap, 10));
            QStandardItem * item_rou = new QStandardItem(pb_router);
            QStandardItem * item_rou_v = new QStandardItem(QString::number(pb_router_volt, 10));
            QStandardItem * item_rou_c = new QStandardItem(QString::number(pb_router_cap, 10));
            QStandardItem * item_pre = new QStandardItem(pb_preload);
            QStandardItem * item_pre_v = new QStandardItem(QString::number(pb_preload_volt, 10));
            QStandardItem * item_pre_c = new QStandardItem(QString::number(pb_preload_cap, 10));
            QStandardItem * item_source = new QStandardItem(pb_source);
            QStandardItem * item_batt_cha = new QStandardItem(pb_batt_cha);
            QStandardItem * item_batt_conn = new QStandardItem(pb_batt_conn);
            QStandardItem * item_batt_ctrl = new QStandardItem(pb_batt_ctrl);
            QStandardItem * item_batt_volt = new QStandardItem(pb_batt_volt);
            QStandardItem * item_batt_cap = new QStandardItem(pb_batt_cap);
            QStandardItem * item_batt_stu = new QStandardItem(pb_batt_stu);
            QStandardItem * item_temperature = new QStandardItem(QString::number(pb_temperature, 10));

            m_model_ptr->appendRow(item_id);
            m_model_ptr->setItem(m_model_ptr->indexFromItem(item_id).row(), 1, item_internet);
            m_model_ptr->setItem(m_model_ptr->indexFromItem(item_id).row(), 2, item_ar);
            m_model_ptr->setItem(m_model_ptr->indexFromItem(item_id).row(), 3, item_ar_inter);
            m_model_ptr->setItem(m_model_ptr->indexFromItem(item_id).row(), 4, item_mic);
            m_model_ptr->setItem(m_model_ptr->indexFromItem(item_id).row(), 5, item_cam);
            m_model_ptr->setItem(m_model_ptr->indexFromItem(item_id).row(), 6, item_cam_v);
            m_model_ptr->setItem(m_model_ptr->indexFromItem(item_id).row(), 7, item_cam_c);
            m_model_ptr->setItem(m_model_ptr->indexFromItem(item_id).row(), 8, item_loc);
            m_model_ptr->setItem(m_model_ptr->indexFromItem(item_id).row(), 9, item_loc_v);
            m_model_ptr->setItem(m_model_ptr->indexFromItem(item_id).row(), 10, item_loc_c);
            m_model_ptr->setItem(m_model_ptr->indexFromItem(item_id).row(), 11, item_swi);
            m_model_ptr->setItem(m_model_ptr->indexFromItem(item_id).row(), 12, item_swi_v);
            m_model_ptr->setItem(m_model_ptr->indexFromItem(item_id).row(), 13, item_swi_c);
            m_model_ptr->setItem(m_model_ptr->indexFromItem(item_id).row(), 14, item_rou);
            m_model_ptr->setItem(m_model_ptr->indexFromItem(item_id).row(), 15, item_rou_v);
            m_model_ptr->setItem(m_model_ptr->indexFromItem(item_id).row(), 16, item_rou_c);
            m_model_ptr->setItem(m_model_ptr->indexFromItem(item_id).row(), 17, item_pre);
            m_model_ptr->setItem(m_model_ptr->indexFromItem(item_id).row(), 18, item_pre_v);
            m_model_ptr->setItem(m_model_ptr->indexFromItem(item_id).row(), 19, item_pre_c);
            m_model_ptr->setItem(m_model_ptr->indexFromItem(item_id).row(), 20, item_source);
            m_model_ptr->setItem(m_model_ptr->indexFromItem(item_id).row(), 21, item_batt_cha);
            m_model_ptr->setItem(m_model_ptr->indexFromItem(item_id).row(), 22, item_batt_conn);
            m_model_ptr->setItem(m_model_ptr->indexFromItem(item_id).row(), 23, item_batt_ctrl);
            m_model_ptr->setItem(m_model_ptr->indexFromItem(item_id).row(), 24, item_batt_volt);
            m_model_ptr->setItem(m_model_ptr->indexFromItem(item_id).row(), 25, item_batt_cap);
            m_model_ptr->setItem(m_model_ptr->indexFromItem(item_id).row(), 26, item_batt_stu);
            m_model_ptr->setItem(m_model_ptr->indexFromItem(item_id).row(), 27, item_temperature);
#endif
       } break;
       case hdtas::HDTAS_MSG_TYPE::HMT_UNKNOWN:
       {
            qDebug() << "udp : unknown message type";
       } break;
    }
}

void MainWindow::onMrUdpMessage(const QByteArray &msg)
{
    QString recv_msg(msg);
    if (recv_msg.contains(QRegExp("[<,>]")))
    {
        QStringList id_ip_list = recv_msg.split(QRegExp("[<,>]"));
        bool ok;
        device_mr_id id = id_ip_list.at(1).toInt(&ok);
        if (!ok)
        {
            qCritical() << "fail to get id from id_ip map";
            return ;
        }
        QString ip = id_ip_list.at(2);
        if (ip.isEmpty())
        {
            qCritical() << "fail to get ip from id_ip map";
            return ;
        }
        m_id_ip_map.insert(id, ip);
        return ;
    }
}

void MainWindow::onMrUdpListenMessage(const QByteArray &msg)
{
    qDebug() << "recv listening msg = " << msg;
}

bool MainWindow::mr_restart(const QString & ip)
{
    mmr_conmunication::udp_data2 restart_data;
    restart_data.set_err_code(0);
    mmr_conmunication::udp_message<mmr_conmunication::udp_data2> restart_request(restart_data, mmr_conmunication::restart);
    QByteArray send_pkg = restart_request.toBinary();
    QUdpSocket udp_socket;
    udp_socket.bind(QHostAddress::Any, 8888, QUdpSocket::ShareAddress);

    QByteArray recv_msg;
    int len = -1;
    while ( len = udp_socket.writeDatagram(send_pkg.data(), send_pkg.size(), QHostAddress(ip), mmr_conmunication::udp_port_send)
           , (!udp_socket.waitForReadyRead(1000) || recv_msg.isEmpty()))
    {
        qDebug() << "have writen [" << len << "] bytes to [" << ip << ":" << mmr_conmunication::udp_port_send << "]";
        recv_msg.resize(udp_socket.pendingDatagramSize());
        udp_socket.readDatagram(recv_msg.data(), recv_msg.size());
    }
    udp_socket.close();

    mmr_conmunication::udp_message<mmr_conmunication::udp_data2> respond_msg(recv_msg.data(), recv_msg.size(), mmr_conmunication::read_var);
    quint32 err_code = respond_msg.get_data().get_err_code();
    if (0 == err_code)
    {
        return true;
    }
    else
    {
        return false;
    }
}

void MainWindow::get_mr_udp_data()
{
    mmr_conmunication::udp_data2 read_var_request;
    read_var_request.set_err_code(0);
    mmr_conmunication::udp_message<mmr_conmunication::udp_data2> read_var_request_pkg(read_var_request, mmr_conmunication::MR_UDP_OPERATOR::read_var);
    QByteArray send_pkg = read_var_request_pkg.toBinary();
    QUdpSocket udp_socket;
    udp_socket.bind(QHostAddress::Any, 8888, QUdpSocket::ShareAddress);
    udp_socket.writeDatagram(send_pkg, QHostAddress(m_id_ip_map[m_test_mmr_id]), mmr_conmunication::udp_port_send);

    QByteArray recv_msg;
    int len = -1;
    while (len = udp_socket.writeDatagram(send_pkg, QHostAddress(m_id_ip_map[m_test_mmr_id]), mmr_conmunication::udp_port_send)
           , (!udp_socket.waitForReadyRead(1000) || recv_msg.isEmpty()))
    {
        qDebug() << "have writen [" << len << "] bytes to [" << m_id_ip_map[m_test_mmr_id] << ":" << mmr_conmunication::udp_port_send << "]";
        recv_msg.resize(udp_socket.pendingDatagramSize());
        udp_socket.readDatagram(recv_msg.data(), recv_msg.size());
    }
    udp_socket.close();
    mmr_conmunication::udp_message<mmr_conmunication::udp_data> build_recv_msg(recv_msg.data(), recv_msg.size(), mmr_conmunication::MR_UDP_OPERATOR::read_var);
    m_mr_udp_data = build_recv_msg.get_data();
}

//测试函数
void MainWindow::on_pushButton_start_clicked()
{
    m_model_ptr->clear();
    m_model_list_ptr->clear();
    ui->pushButton_start->setDisabled(true);
    ui->statusBar->clearMessage();
    m_has_ispd = false;
    m_has_mr = false;
    m_has_pb = false;
    QString list_str;
    QStandardItem * item_list_str;
    //
    //获取终端的数量
    self_sleep(2);
    if (get_mr_list())
    {
        m_err_info = QString::fromLocal8Bit("已获取终端数量[") + QString::number(m_mr_status_array.size()) + QString::fromLocal8Bit("]");
        qInfo() << m_err_info;
        ui->statusBar->showMessage(m_err_info);

        list_str = m_err_info;
        item_list_str = new QStandardItem(list_str);
        m_model_list_ptr->appendRow(item_list_str);
    }
    else
    {
        m_err_info = QString::fromLocal8Bit("获取不到从终端的信息,请重试...");
        qCritical() << m_err_info;
        ui->statusBar->showMessage(m_err_info);
        ui->pushButton_start->setEnabled(true);
        list_str = m_err_info;
        item_list_str = new QStandardItem(list_str);
        m_model_list_ptr->appendRow(item_list_str);
        return;
    }

    //检查输入的ID是否正确
    if (m_mmr_id != m_test_mmr_id)
    {
        m_err_info = QString::fromLocal8Bit("请检查输入待测主终端ID是否正确");
        qDebug() << m_err_info;
        ui->statusBar->showMessage(m_err_info);
        QString str1 = MESSAGE_RED + m_err_info + MESSAGE_END;
        QMessageBox::warning(this, QString::fromLocal8Bit("警告"), str1);
        ui->pushButton_ok->setEnabled(true);
        return ;
    }

    //检查输入的ID是否正确
    if (m_mmr_id == m_test_mr_id1 || m_mmr_id == m_test_mr_id2)
    {
        m_err_info = QString::fromLocal8Bit("请检查输入待测从终端ID是否正确");
        qDebug() << m_err_info;
        ui->statusBar->showMessage(m_err_info);
        QString str1 = MESSAGE_RED + m_err_info + MESSAGE_END;
        QMessageBox::warning(this, QString::fromLocal8Bit("警告"), str1);
        ui->pushButton_ok->setEnabled(true);
        return ;
    }

    //更新主控板上报时间
    if (update_auto_report_inter(UPDATE_PB_INTERVAL))
    {
        m_err_info = QString::fromLocal8Bit("更新主控板上报时间成功");
        qInfo() << m_err_info;
    }
    else
    {
        m_err_info = QString::fromLocal8Bit("更新主控板上报时间失败");
        qCritical() << m_err_info;
    }

    list_str = m_err_info;
    item_list_str = new QStandardItem(list_str);
    m_model_list_ptr->appendRow(item_list_str);

    get_pb_interval(m_mmr_id);
    get_cmpb_status(m_mmr_id);

    m_model_ptr->setHorizontalHeaderLabels(QStringList()
                                           << QString::fromLocal8Bit("测试项目")
                                           << QString::fromLocal8Bit("测试结果"));
    //uwb板通信测试
    QStandardItem * item_uwb = new QStandardItem(QString::fromLocal8Bit("测试UWB板通信"));
    //m_model_list_ptr->appendRow(item_uwb);
    list_str = QString::fromLocal8Bit("开始测试UWB板通信");
    item_list_str = new QStandardItem(list_str);
    m_model_list_ptr->appendRow(item_list_str);
    qDebug() << list_str;

    item_uwb->setFont(QFont("Times", TABLE_FONT, QFont::Black));
    m_model_ptr->appendRow(item_uwb);

    ui->statusBar->showMessage(QString::fromLocal8Bit("正在测试..."));
    QStandardItem * item_uwb_res = new QStandardItem(QString::fromLocal8Bit("正在测试..."));
    m_model_ptr->setItem(m_model_ptr->indexFromItem(item_uwb).row(), 1, item_uwb_res);

    self_sleep(2);
    if (m_has_mr)
    {
        item_uwb_res = new QStandardItem(QString::fromLocal8Bit("测试通过"));
        item_uwb_res->setFont(QFont("Times", TABLE_FONT, QFont::Black));
        item_uwb_res->setForeground(QBrush(QColor(0, 255, 0)));
        m_model_ptr->setItem(m_model_ptr->indexFromItem(item_uwb).row(), 1, item_uwb_res);
        ui->statusBar->showMessage(QString::fromLocal8Bit("UWB板测试完成"), 3000);
        list_str = QString::fromLocal8Bit("可以通信的从终端数量为[") + QString::number(m_mr_status_array.size()) + QString::fromLocal8Bit("]台");
        item_list_str = new QStandardItem(list_str);
        m_model_list_ptr->appendRow(item_list_str);
    }
    else
    {
        item_uwb_res = new QStandardItem(QString::fromLocal8Bit("测试没有通过"));
        item_uwb_res->setFont(QFont("Times", TABLE_FONT, QFont::Black));
        item_uwb_res->setForeground(QBrush(QColor(255, 0, 0)));
        m_model_ptr->setItem(m_model_ptr->indexFromItem(item_uwb).row(), 1, item_uwb_res);
        ui->statusBar->showMessage(QString::fromLocal8Bit("UWB板通信测试没有通过"), 3000);
        list_str = QString::fromLocal8Bit("无法收到其他终端的UWB数据");
        item_list_str = new QStandardItem(list_str);
        m_model_list_ptr->appendRow(item_list_str);
    }

    //电源板测试
    QStandardItem * item_pb = new QStandardItem(QString::fromLocal8Bit("测试主控板通信"));
    //m_model_list_ptr->appendRow(item_pb);
    list_str = QString::fromLocal8Bit("开始测试主控板通信");
    item_list_str = new QStandardItem(list_str);
    m_model_list_ptr->appendRow(item_list_str);
    qDebug() << list_str;

    item_pb->setFont(QFont("Times", TABLE_FONT, QFont::Black));
    m_model_ptr->appendRow(item_pb);

    ui->statusBar->showMessage(QString::fromLocal8Bit("正在测试..."));
    QStandardItem * item_pb_res = new QStandardItem(QString::fromLocal8Bit("正在测试..."));
    m_model_ptr->setItem(m_model_ptr->indexFromItem(item_pb).row(), 1, item_pb_res);
    self_sleep(1.5);
    if (m_has_pb)
    {
        item_pb_res = new QStandardItem(QString::fromLocal8Bit("测试通过"));
        item_pb_res->setFont(QFont("Times", TABLE_FONT, QFont::Black));
        item_pb_res->setForeground(QBrush(QColor(0, 255, 0)));
        m_model_ptr->setItem(m_model_ptr->indexFromItem(item_pb).row(), 1, item_pb_res);
        ui->statusBar->showMessage(QString::fromLocal8Bit("主控板测试完成"), 3000);
        list_str = QString::fromLocal8Bit("主控板上报数据正常");
    }
    else
    {
        item_pb_res = new QStandardItem(QString::fromLocal8Bit("测试没有通过"));
        item_pb_res->setFont(QFont("Times", TABLE_FONT, QFont::Black));
        item_pb_res->setForeground(QBrush(QColor(255, 0, 0)));
        m_model_ptr->setItem(m_model_ptr->indexFromItem(item_pb).row(), 1, item_pb_res);
        m_model_list_ptr->appendRow(item_pb_res);
        ui->statusBar->showMessage(QString::fromLocal8Bit("主控板测试没有通过"), 3000);
        list_str = QString::fromLocal8Bit("无法收到主控板上报数据");
    }
    item_list_str = new QStandardItem(list_str);
    m_model_list_ptr->appendRow(item_list_str);

    //ispd超级板测试
    QStandardItem * item_ispd = new QStandardItem(QString::fromLocal8Bit("测试超级板通信"));
    //m_model_list_ptr->appendRow(item_ispd);
    list_str = QString::fromLocal8Bit("开始测试超级板通信");
    item_list_str = new QStandardItem(list_str);
    m_model_list_ptr->appendRow(item_list_str);
    qDebug() << list_str;

    item_ispd->setFont(QFont("Times", TABLE_FONT, QFont::Black));
    m_model_ptr->appendRow(item_ispd);

    ui->statusBar->showMessage(QString::fromLocal8Bit("正在测试..."));
    QStandardItem * item_ispd_res = new QStandardItem(QString::fromLocal8Bit("正在测试..."));
    m_model_ptr->setItem(m_model_ptr->indexFromItem(item_ispd).row(), 1, item_ispd_res);
    self_sleep(1.5);
    if (m_has_ispd)
    {
        item_ispd_res = new QStandardItem(QString::fromLocal8Bit("测试通过"));
        item_ispd_res->setFont(QFont("Times", TABLE_FONT, QFont::Black));
        item_ispd_res->setForeground(QBrush(QColor(0, 255, 0)));
        m_model_ptr->setItem(m_model_ptr->indexFromItem(item_ispd).row(), 1, item_ispd_res);
        ui->statusBar->showMessage(QString::fromLocal8Bit("主控板测试完成"), 3000);

        list_str = QString::fromLocal8Bit("成功收到超级板数据");
    }
    else
    {
        item_ispd_res = new QStandardItem(QString::fromLocal8Bit("测试没有通过"));
        item_ispd_res->setFont(QFont("Times", TABLE_FONT, QFont::Black));
        item_ispd_res->setForeground(QBrush(QColor(255, 0, 0)));    
        m_model_ptr->setItem(m_model_ptr->indexFromItem(item_ispd).row(), 1, item_ispd_res);
        ui->statusBar->showMessage(QString::fromLocal8Bit("主控板测试没有通过"), 3000);
        list_str = QString::fromLocal8Bit("无法收到超级板数据");
    }
    item_list_str = new QStandardItem(list_str);
    m_model_list_ptr->appendRow(item_list_str);

    //测试摄像头 not master
    QString err;
    int no_pass_cnt = 0;
    if (!m_is_master)
    {
        QStandardItem * item_cam = new QStandardItem(QString::fromLocal8Bit("测试摄像头"));
        //m_model_list_ptr->appendRow(item_cam);
        list_str = QString::fromLocal8Bit("开始测试待测从终端摄像头开关");
        item_list_str = new QStandardItem(list_str);
        m_model_list_ptr->appendRow(item_list_str);
        qDebug() << list_str;

        item_cam->setFont(QFont("Times", TABLE_FONT, QFont::Black));
        QStandardItem * item_cam_res = new QStandardItem(QString::fromLocal8Bit("正在测试..."));

        m_model_ptr->appendRow(item_cam);
        m_model_ptr->setItem(m_model_ptr->indexFromItem(item_cam).row(), 1, item_cam_res);
        ui->statusBar->showMessage(QString::fromLocal8Bit("正在测试..."));
        for (int test_cnt = 1; test_cnt <= 3; test_cnt++)
        {
            if (is_open(m_test_mr_id1, CMPB_TYPE::CAM))
            {
                // if it is open, then close it
                open_close(false, m_cmpb_list.value(m_test_mr_id1).value(MIC), m_cmpb_list.value(m_test_mr_id1).value(PRE), m_cmpb_list.value(m_test_mr_id1).value(BAT));
                //get_cmpb_status(m_test_mr_id1);
                if (!is_open(m_test_mr_id1, CMPB_TYPE::CAM))
                {
                    err = "No." + QString::number(test_cnt) + " close successfully";
                }
                else
                {
                    err = "No." + QString::number(test_cnt) + " close failed";
                    no_pass_cnt++;
                }
            }
            else
            {
                // if it is close, then open it
                open_close(true, m_cmpb_list.value(m_test_mr_id1).value(MIC), m_cmpb_list.value(m_test_mr_id1).value(PRE), m_cmpb_list.value(m_test_mr_id1).value(BAT));
                //get_cmpb_status(m_test_mr_id1);
                if (is_open(m_test_mr_id1, CMPB_TYPE::CAM))
                {
                    err = "No." + QString::number(test_cnt) + " open successfully";
                }
                else
                {
                    err = "No." + QString::number(test_cnt) + " open failed";
                    no_pass_cnt++;
                }
            }

            qInfo() << err;
            ui->statusBar->showMessage(err);

            list_str = err;
            item_list_str = new QStandardItem(list_str);
            m_model_list_ptr->appendRow(item_list_str);

        }
        if (no_pass_cnt > 2)
        {
            item_cam_res = new QStandardItem(QString::fromLocal8Bit("测试没有通过"));
            item_cam_res->setFont(QFont("Times", TABLE_FONT, QFont::Black));
            item_cam_res->setForeground(QBrush(QColor(255, 0, 0)));
        }
        else
        {
            item_cam_res = new QStandardItem(QString::fromLocal8Bit("测试通过"));
            item_cam_res->setFont(QFont("Times", TABLE_FONT, QFont::Black));
            item_cam_res->setForeground(QBrush(QColor(0, 255, 0)));
        }

        m_model_ptr->setItem(m_model_ptr->indexFromItem(item_cam).row(), 1, item_cam_res);
    }

    //测试麦克风, not master
    no_pass_cnt = 0;
    if (!m_is_master)
    {
        QStandardItem * item_mic = new QStandardItem(QString::fromLocal8Bit("测试拾音器"));
        //m_model_list_ptr->appendRow(item_mic);
        list_str = QString::fromLocal8Bit("开始测试待测从终端拾音器开关");
        item_list_str = new QStandardItem(list_str);
        m_model_list_ptr->appendRow(item_list_str);
        qDebug() << list_str;

        item_mic->setFont(QFont("Times", TABLE_FONT, QFont::Black));
        QStandardItem * item_mic_res = new QStandardItem(QString::fromLocal8Bit("正在测试..."));

        m_model_ptr->appendRow(item_mic);
        m_model_ptr->setItem(m_model_ptr->indexFromItem(item_mic).row(), 1, item_mic_res);
        ui->statusBar->showMessage(QString::fromLocal8Bit("正在测试..."));
        for (int test_cnt = 1; test_cnt <= 3; test_cnt++)
        {
            if (is_open(m_test_mr_id1, CMPB_TYPE::MIC))
            {
                // if it is open, then close it
                open_close(m_cmpb_list.value(m_test_mr_id1).value(CAM), false, m_cmpb_list.value(m_test_mr_id1).value(PRE), m_cmpb_list.value(m_test_mr_id1).value(BAT));
                get_cmpb_status(m_test_mr_id1);
                if (!is_open(m_test_mr_id1, CMPB_TYPE::MIC))
                {
                    err = "No." + QString::number(test_cnt) + " close successfully";
                }
                else
                {
                    err = "No." + QString::number(test_cnt) + " close failed";
                    no_pass_cnt++;
                }
            }
            else
            {
                // if it is close, then open it
                //open_close(true, false, false, false);
                open_close(m_cmpb_list.value(m_test_mr_id1).value(CAM), true, m_cmpb_list.value(m_test_mr_id1).value(PRE), m_cmpb_list.value(m_test_mr_id1).value(BAT));
                get_cmpb_status(m_test_mr_id1);
                if (is_open(m_test_mr_id1, CMPB_TYPE::MIC))
                {
                    err = "No." + QString::number(test_cnt) + " open successfully";
                }
                else
                {
                    err = "No." + QString::number(test_cnt) + " open failed";
                    no_pass_cnt++;
                }
            }
            qInfo() << err;
            ui->statusBar->showMessage(err);

            list_str = err;
            item_list_str = new QStandardItem(list_str);
            m_model_list_ptr->appendRow(item_list_str);
        }
        if (no_pass_cnt > 2)
        {
            item_mic_res = new QStandardItem(QString::fromLocal8Bit("测试没有通过"));
            item_mic_res->setFont(QFont("Times", TABLE_FONT, QFont::Black));
            item_mic_res->setForeground(QBrush(QColor(255, 0, 0)));
        }
        else
        {
            item_mic_res = new QStandardItem(QString::fromLocal8Bit("测试通过"));
            item_mic_res->setFont(QFont("Times", TABLE_FONT, QFont::Black));
            item_mic_res->setForeground(QBrush(QColor(0, 255, 0)));
        }

        m_model_ptr->setItem(m_model_ptr->indexFromItem(item_mic).row(), 1, item_mic_res);
    }

    //测试预留负载
    QStandardItem * item_pre = new QStandardItem(QString::fromLocal8Bit("测试预留负载"));
    //m_model_list_ptr->appendRow(item_pre);
    list_str = QString::fromLocal8Bit("开始测试预留负载开关");
    item_list_str = new QStandardItem(list_str);
    m_model_list_ptr->appendRow(item_list_str);
    qDebug() << list_str;

    item_pre->setFont(QFont("Times", TABLE_FONT, QFont::Black));
    QStandardItem * item_pre_res = new QStandardItem(QString::fromLocal8Bit("正在测试..."));

    m_model_ptr->appendRow(item_pre);
    m_model_ptr->setItem(m_model_ptr->indexFromItem(item_pre).row(), 1, item_pre_res);
    ui->statusBar->showMessage(QString::fromLocal8Bit("正在测试..."));
    no_pass_cnt = 0;
    if (!m_is_master)
    {
        for (int test_cnt = 1; test_cnt <= 3; test_cnt++)
        {
            if (is_open(m_test_mr_id1, CMPB_TYPE::PRE))
            {
                // if it is open, then close it
                open_close(m_cmpb_list.value(m_test_mr_id1).value(CAM), m_cmpb_list.value(m_test_mr_id1).value(MIC), false, m_cmpb_list.value(m_test_mr_id1).value(BAT));
                if (!is_open(m_test_mr_id1, CMPB_TYPE::PRE))
                {
                    err = "No." + QString::number(test_cnt) + " close successfully";
                }
                else
                {
                    err = "No." + QString::number(test_cnt) + " close failed";
                    no_pass_cnt++;
                }
            }
            else
            {
                // if it is close, then open it
                open_close(m_cmpb_list.value(m_test_mr_id1).value(CAM), m_cmpb_list.value(m_test_mr_id1).value(MIC), true, m_cmpb_list.value(m_test_mr_id1).value(BAT));
                if (is_open(m_test_mr_id1, CMPB_TYPE::PRE))
                {
                    err = "No." + QString::number(test_cnt) + " open successfully";
                }
                else
                {
                    err = "No." + QString::number(test_cnt) + " open failed";
                    no_pass_cnt++;
                }
            }
            qInfo() << err;
            ui->statusBar->showMessage(err);
            list_str = err;
            item_list_str = new QStandardItem(list_str);
            m_model_list_ptr->appendRow(item_list_str);

        } // for
        if (no_pass_cnt > 2)
        {
            item_pre_res = new QStandardItem(QString::fromLocal8Bit("测试没有通过"));
            item_pre_res->setFont(QFont("Times", TABLE_FONT, QFont::Black));
            item_pre_res->setForeground(QBrush(QColor(255, 0, 0)));
        }
        else
        {
            item_pre_res = new QStandardItem(QString::fromLocal8Bit("测试通过"));
            item_pre_res->setFont(QFont("Times", TABLE_FONT, QFont::Black));
            item_pre_res->setForeground(QBrush(QColor(0, 255, 0)));
        }
        m_model_ptr->setItem(m_model_ptr->indexFromItem(item_pre).row(), 1, item_pre_res);
    }
    else
    {
        for (int test_cnt = 1; test_cnt <= 3; test_cnt++)
        {
                if (is_open(m_mmr_id, CMPB_TYPE::BAT))
                {
                // if it is open, then close it
                open_close(m_cmpb_list.value(m_mmr_id).value(CAM), m_cmpb_list.value(m_mmr_id).value(MIC), false, m_cmpb_list.value(m_mmr_id).value(BAT));
                get_cmpb_status(m_mmr_id);
                if (!is_open(m_mmr_id, CMPB_TYPE::PRE))
                {
                    err = "No." + QString::number(test_cnt) + " close successfully";
                }
                else
                {
                    err = "No." + QString::number(test_cnt) + " close failed";
                    no_pass_cnt++;
                }
            }
            else
            {
                // if it is close, then open it
                open_close(m_cmpb_list.value(m_mmr_id).value(CAM), m_cmpb_list.value(m_mmr_id).value(MIC), true, m_cmpb_list.value(m_mmr_id).value(BAT));
                get_cmpb_status(m_mmr_id);
                if (is_open(m_mmr_id, CMPB_TYPE::PRE))
                {
                    err = "No." + QString::number(test_cnt) + " open successfully";
                }
                else
                {
                    err = "No." + QString::number(test_cnt) + " open failed";
                    no_pass_cnt++;
                }
            }
            qInfo() << err;
            ui->statusBar->showMessage(err);

            list_str = err;
            item_list_str = new QStandardItem(list_str);
            m_model_list_ptr->appendRow(item_list_str);
        }// for
        if (no_pass_cnt > 2)
        {
            item_pre_res = new QStandardItem(QString::fromLocal8Bit("测试没有通过"));
            item_pre_res->setFont(QFont("Times", TABLE_FONT, QFont::Black));
            item_pre_res->setForeground(QBrush(QColor(255, 0, 0)));
        }
        else
        {
            item_pre_res = new QStandardItem(QString::fromLocal8Bit("测试通过"));
            item_pre_res->setFont(QFont("Times", TABLE_FONT, QFont::Black));
            item_pre_res->setForeground(QBrush(QColor(0, 255, 0)));
        }
        m_model_ptr->setItem(m_model_ptr->indexFromItem(item_pre).row(), 1, item_pre_res);
    }// else, master

#if 0
    // battery
    QStandardItem * item_batt = new QStandardItem(tr("测试电池"));
    item_batt->setFont(QFont("Times", TABLE_FONT, QFont::Black));
    QStandardItem * item_batt_res = new QStandardItem(tr("正在测试..."));

    m_model_ptr->appendRow(item_batt);
    m_model_ptr->setItem(m_model_ptr->indexFromItem(item_batt).row(), 1, item_batt_res);
    ui->statusBar->showMessage(tr("正在测试..."));
    no_pass_cnt = 0;
    if (!m_is_master) {
        for (int test_cnt = 1; test_cnt <= 10; test_cnt++) {
            if (!get_cmpb_status(m_test_mr_id1)) {
                qDebug() << "failed get cmp list";
                return ;
            }
            if (is_open(m_test_mr_id1, CMPB_TYPE::BAT)) {
                // if it is open, then close it
                open_close(m_cmpb_list.value(m_test_mr_id1).value(CAM), m_cmpb_list.value(m_test_mr_id1).value(MIC), m_cmpb_list.value(m_test_mr_id1).value(PRE), false);
                get_cmpb_status(m_test_mr_id1);
                if (!is_open(m_test_mr_id1, CMPB_TYPE::BAT)) {
                    err = "No." + QString::number(test_cnt) + " close successfully";
                    qInfo() << err;
                    ui->statusBar->showMessage(err);
                } else{
                    err = "No." + QString::number(test_cnt) + " close failed";
                    qInfo() << err;
                    ui->statusBar->showMessage(err);
                    no_pass_cnt++;
                }
            } else {
                // if it is close, then open it
                open_close(m_cmpb_list.value(m_test_mr_id1).value(CAM), m_cmpb_list.value(m_test_mr_id1).value(MIC), m_cmpb_list.value(m_test_mr_id1).value(PRE), true);
                get_cmpb_status(m_test_mr_id1);
                if (is_open(m_test_mr_id1, CMPB_TYPE::BAT)) {
                    err = "No." + QString::number(test_cnt) + " open successfully";
                    qInfo() << err;
                    ui->statusBar->showMessage(err);
                } else{
                    err = "No." + QString::number(test_cnt) + " open failed";
                    qInfo() << err;
                    ui->statusBar->showMessage(err);
                    no_pass_cnt++;
                }
            }
        } // for
        if (no_pass_cnt > 2) {
            item_batt_res = new QStandardItem(tr("测试没有通过"));
            item_batt_res->setFont(QFont("Times", TABLE_FONT, QFont::Black));
            item_batt_res->setForeground(QBrush(QColor(255, 0, 0)));
        } else {
            item_batt_res = new QStandardItem(tr("测试通过"));
            item_batt_res->setFont(QFont("Times", TABLE_FONT, QFont::Black));
            item_batt_res->setForeground(QBrush(QColor(0, 255, 0)));
        }
        m_model_ptr->setItem(m_model_ptr->indexFromItem(item_batt).row(), 1, item_batt_res);
    } else {
        for (int test_cnt = 1; test_cnt <= 10; test_cnt++) {

            if (!get_cmpb_status(m_mmr_id)) {
                qDebug() << "failed get cmp list";
                return ;
            }
            if (is_open(m_mmr_id, CMPB_TYPE::BAT)) {
                // if it is open, then close it
                open_close(m_cmpb_list.value(m_mmr_id).value(CAM), m_cmpb_list.value(m_mmr_id).value(MIC), m_cmpb_list.value(m_mmr_id).value(PRE), false);
                get_cmpb_status(m_mmr_id);
                if (!is_open(m_mmr_id, CMPB_TYPE::BAT)) {
                    err = "No." + QString::number(test_cnt) + " close successfully";
                    qInfo() << err;
                    ui->statusBar->showMessage(err);
                } else{
                    err = "No." + QString::number(test_cnt) + " close failed";
                    qInfo() << err;
                    ui->statusBar->showMessage(err);
                    //item_cam_res = new QStandardItem(tr("测试没有通过"));
                    //m_model_ptr->setItem(m_model_ptr->indexFromItem(item_cam).row(), 1, item_cam_res);
                    //reset();
                    //return;
                    no_pass_cnt++;
                }
            } else {
                // if it is close, then open it
                open_close(m_cmpb_list.value(m_mmr_id).value(CAM), m_cmpb_list.value(m_mmr_id).value(MIC), m_cmpb_list.value(m_mmr_id).value(PRE), true);
                get_cmpb_status(m_mmr_id);
                if (is_open(m_mmr_id, CMPB_TYPE::BAT)) {
                    err = "No." + QString::number(test_cnt) + " open successfully";
                    qInfo() << err;
                    ui->statusBar->showMessage(err);
                } else{
                    err = "No." + QString::number(test_cnt) + " open failed";
                    qInfo() << err;
                    ui->statusBar->showMessage(err);
                    //item_cam_res = new QStandardItem(tr("测试没有通过"));
                    //m_model_ptr->setItem(m_model_ptr->indexFromItem(item_cam).row(), 1, item_cam_res);
                    //reset();
                    //return;
                    no_pass_cnt++;
                }
            }
        }// for
        if (no_pass_cnt > 2) {
            item_batt_res = new QStandardItem(tr("测试没有通过"));
            item_batt_res->setFont(QFont("Times", TABLE_FONT, QFont::Black));
            item_batt_res->setForeground(QBrush(QColor(255, 0, 0)));
        } else {
            item_batt_res = new QStandardItem(tr("测试通过"));
            item_batt_res->setFont(QFont("Times", TABLE_FONT, QFont::Black));
            item_batt_res->setForeground(QBrush(QColor(0, 255, 0)));
        }
        m_model_ptr->setItem(m_model_ptr->indexFromItem(item_batt).row(), 1, item_batt_res);
    }// else, master
#endif
    //modify by guosj
    reset();
    //ui->pushButton->setEnabled(true);
    //ui->pushButton_ok->setEnabled(true);
    ui->radioButton->setEnabled(true);
    ui->radioButton_2->setEnabled(true);
    ui->radioButton_line->setEnabled(true);
    ui->radioButton_wifi->setEnabled(true);
    m_err_info = QString::fromLocal8Bit("恢复出厂设置成功");
    qInfo() << m_err_info;
    list_str = m_err_info;
    item_list_str = new QStandardItem(list_str);
    m_model_list_ptr->appendRow(item_list_str);

    m_err_info = QString::fromLocal8Bit("测试结束");
    ui->pushButton_ok->setEnabled(true);
    ui->pushButton_start->setDisabled(true);
    ui->statusBar->showMessage(m_err_info);
    qCritical() << m_err_info;
    QString str1 = MESSAGE_RED + m_err_info + MESSAGE_END;
    QMessageBox::information(this,QString::fromLocal8Bit("信息"), str1);

    list_str = m_err_info;
    item_list_str = new QStandardItem(list_str);
    m_model_list_ptr->appendRow(item_list_str);

}

bool MainWindow::get_mr_list()
{
    hdtas::HdtasMessage message;
    hdtas::HdtasPackage package;
    std::pair<const unsigned char *, size_t> send_pkg;
    const char * data = nullptr;

    hdtas::HdtasCtlMrPbRequest mr_request;
    mr_request.SetMMrID(m_test_mmr_id);
    mr_request.SetMsgID(0);
    mr_request.ReadStatusInfo();
    mr_request.SetRequestID(100);
    mr_request.StartSerialize();
    int cnt = mr_request.GetSerializeCount();
    while (cnt--)
    {
        mr_request.Serialize(&message);
        message.Serialize(&package);
        send_pkg = package.Pack();
        data = reinterpret_cast<const char *>(send_pkg.first);
        emit writeTcpMessage(data, send_pkg.second);
    }

    self_sleep(3);
    // not lock here
    if (m_mr_status_array.empty())
    {
        return false;
    }
    else
    {
        qDebug() << "there are ["  << m_mr_status_array.size() << "] mr(s) available";
        return true;
    }
}

bool MainWindow::update_auto_report_inter(quint32 interval)
{
    hdtas::HdtasMessage message;
    hdtas::HdtasPackage package;
    std::pair<const unsigned char *, size_t> send_pkg;
    const char * data = nullptr;
    int cnt = 0;

    if (m_is_master)
    {
        hdtas::HdtasCtlMrPbRequest update_request;
        update_request.WriteAutoReportInterval(m_test_mmr_id, interval);
        update_request.SetRequestID(31);
        update_request.StartSerialize();
        cnt = update_request.GetSerializeCount();
        while (cnt--)
        {
            update_request.Serialize(&message);
            message.Serialize(&package);
            send_pkg = package.Pack();
            data = reinterpret_cast<const char *>(send_pkg.first);
            emit writeTcpMessage(data, send_pkg.second);
        }
        self_sleep(1);

        if (UPDATE_PB_INTERVAL == m_pb_interval.value(m_test_mmr_id))
        {
            return true;
        }
        else
        {
            return false;
        }
    }
    else
    {
        hdtas::HdtasCtlMrPbRequest update_request;
        update_request.WriteAutoReportInterval(m_test_mr_id1, interval);
        update_request.SetRequestID(31);
        update_request.StartSerialize();
        cnt = update_request.GetSerializeCount();
        while (cnt--)
        {
            update_request.Serialize(&message);
            message.Serialize(&package);
            send_pkg = package.Pack();
            data = reinterpret_cast<const char *>(send_pkg.first);
            emit writeTcpMessage(data, send_pkg.second);
        }
        self_sleep(1);

        if (UPDATE_PB_INTERVAL == m_pb_interval.value(m_test_mr_id1))
        {
            return true;
        }
        else
        {
            return false;
        }
    }
}

void MainWindow::open_close(bool cam, bool mic, bool pre, bool batt)
{
    hdtas::HdtasMessage message;
    hdtas::HdtasPackage package;
    std::pair<const unsigned char *, size_t> send_pkg;
    const char * data = nullptr;
    int cnt = 0;

    if (m_is_master)
    {
        // master
        hdtas::HdtasCtlMrPbRequest ctl_request;
        ctl_request.Open_Close(m_test_mmr_id, cam == true ? hdtas::HOCS_OPEN : hdtas::HOCS_CLOSE,
                               mic == true ? hdtas::HOCS_OPEN : hdtas::HOCS_CLOSE,
                               pre == true ? hdtas::HOCS_OPEN : hdtas::HOCS_CLOSE,
                               batt == true ? hdtas::HOCS_OPEN : hdtas::HOCS_CLOSE);
        ctl_request.SetRequestID(10);
        ctl_request.StartSerialize();
        cnt = ctl_request.GetSerializeCount();
        while (cnt--)
        {
            ctl_request.Serialize(&message);
            message.Serialize(&package);
            send_pkg = package.Pack();
            data = reinterpret_cast<const char *>(send_pkg.first);
            emit writeTcpMessage(data, send_pkg.second);
        }
        self_sleep(1);
    }
    else
    {
        // followers
        hdtas::HdtasCtlMrPbRequest ctl_request;
        ctl_request.Open_Close(m_test_mr_id1, cam == true ? hdtas::HOCS_OPEN : hdtas::HOCS_CLOSE,
                               mic == true ? hdtas::HOCS_OPEN : hdtas::HOCS_CLOSE,
                               pre == true ? hdtas::HOCS_OPEN : hdtas::HOCS_CLOSE,
                               batt == true ? hdtas::HOCS_OPEN : hdtas::HOCS_CLOSE);
        ctl_request.SetRequestID(11);
        ctl_request.StartSerialize();
        cnt = ctl_request.GetSerializeCount();
        while (cnt--)
        {
            ctl_request.Serialize(&message);
            message.Serialize(&package);
            send_pkg = package.Pack();
            data = reinterpret_cast<const char *>(send_pkg.first);
            emit writeTcpMessage(data, send_pkg.second);
        }
        self_sleep(1);
    }// else
}

bool MainWindow::is_open(device_mr_id id, CMPB_TYPE type)
{
    if (!get_cmpb_status(id))
    {
        qCritical() << QString("cannot get cmpb status of %1, as it is empty").arg(id);
        return false;
    }
    return m_cmpb_list.value(id).value(type);
}

bool MainWindow::get_cmpb_status(device_mr_id id)
{
    hdtas::HdtasMessage message;
    hdtas::HdtasPackage package;
    std::pair<const unsigned char *, size_t> send_pkg;
    const char * data = nullptr;
    int cnt = 0;

    hdtas::HdtasCtlMrPbRequest reset_request;
    reset_request.ReadStatusInfo(id);
    reset_request.SetRequestID(51);
    reset_request.StartSerialize();
    cnt = reset_request.GetSerializeCount();
    while (cnt--)
    {
        reset_request.Serialize(&message);
        message.Serialize(&package);
        send_pkg = package.Pack();
        data = reinterpret_cast<const char *>(send_pkg.first);
        emit writeTcpMessage(data, send_pkg.second);
    }
    self_sleep(1);
    if (m_cmpb_list.value(id).empty())
    {
        return false;
    }
    else
    {
        return true;
    }
}

void MainWindow::get_pb_interval(device_mr_id id)
{
    hdtas::HdtasMessage message;
    hdtas::HdtasPackage package;
    std::pair<const unsigned char *, size_t> send_pkg;
    const char * data = nullptr;
    int cnt = 0;

    hdtas::HdtasCtlMrPbRequest pb_inter_request;
    pb_inter_request.ReadAutoReportInterval(id);
    pb_inter_request.SetRequestID(20);
    pb_inter_request.StartSerialize();
    cnt = pb_inter_request.GetSerializeCount();
    while (cnt--) {
        pb_inter_request.Serialize(&message);
        message.Serialize(&package);
        send_pkg = package.Pack();
        data = reinterpret_cast<const char *>(send_pkg.first);
        emit writeTcpMessage(data, send_pkg.second);
    }
    self_sleep(1);

    if (!m_pb_interval.empty()) {
        qDebug() << m_pb_interval.value(id);
    } else {
        qDebug() << "m_pb_interval is empty";
    }

}

void MainWindow::get_mr_interval()
{

}

void MainWindow::reset()
{
    hdtas::HdtasMessage message;
    hdtas::HdtasPackage package;
    std::pair<const unsigned char *, size_t> send_pkg;
    const char * data = nullptr;
    int cnt = 0;
    if (m_is_master) {
        // master
        hdtas::HdtasCtlMrPbRequest reset_request;
        reset_request.Reset_MR(m_mmr_id); // change to reset later
        reset_request.SetRequestID(21);
        reset_request.StartSerialize();
        cnt = reset_request.GetSerializeCount();
        while (cnt--) {
            reset_request.Serialize(&message);
            message.Serialize(&package);
            send_pkg = package.Pack();
            data = reinterpret_cast<const char *>(send_pkg.first);
            emit writeTcpMessage(data, send_pkg.second);
        }
        self_sleep(1);
    } else {
        // followers
        hdtas::HdtasCtlMrPbRequest reset_request;
        reset_request.Reset_MR(m_test_mr_id1); // change to reset

        reset_request.SetRequestID(22);
        reset_request.StartSerialize();
        cnt = reset_request.GetSerializeCount();
        while (cnt--) {
            reset_request.Serialize(&message);
            message.Serialize(&package);
            send_pkg = package.Pack();
            data = reinterpret_cast<const char *>(send_pkg.first);
            emit writeTcpMessage(data, send_pkg.second);
        }// while
        self_sleep(1);
    }// else
}

#if 0
void MainWindow::setIP()
{
    SetIpDialog * setIpDialogPtr = new SetIpDialog(this);
    connect(setIpDialogPtr, SIGNAL(updateIP(QString)), this, SLOT(updateIP(QString)));
    setIpDialogPtr->show();
}
#endif

#if 0
void MainWindow::updateIP(const QString & ip)
{
    mmr_conmunication::udp_data2 read_var_request;
    read_var_request.set_err_code(0);
    mmr_conmunication::udp_message<mmr_conmunication::udp_data2> read_var_request_pkg(read_var_request, mmr_conmunication::MR_UDP_OPERATOR::read_var);
    QByteArray send_pkg = read_var_request_pkg.toBinary();
    QUdpSocket udp_socket;
    udp_socket.bind(QHostAddress::Any, 8888, QUdpSocket::ShareAddress);

    QByteArray recv_msg;
    int len = -1;
    QTime timer;
    timer.start();
    while (len = udp_socket.writeDatagram(send_pkg, QHostAddress(m_id_ip_map[m_test_mmr_id]), mmr_conmunication::udp_port_send)
           , (!udp_socket.waitForReadyRead(1000) || recv_msg.isEmpty())) {

        qDebug() << "have writen [" << len << "] bytes to [" << m_id_ip_map[m_test_mmr_id] << ":" << mmr_conmunication::udp_port_send << "]";

        if (timer.elapsed() > (TIMEOUT * 1000)) {
            m_err_info = QString::fromLocal8Bit("接收终端数据超时，请重新设置IP！");
            QString str1 = MESSAGE_RED + m_err_info + MESSAGE_END;
            QMessageBox::warning(this, QString::fromLocal8Bit("警告"), str1);

            qDebug() << m_err_info;
            ui->statusBar->showMessage(m_err_info);
            ui->pushButton_ok->setEnabled(true);
            return ;
        }

        recv_msg.resize(udp_socket.pendingDatagramSize());
        udp_socket.readDatagram(recv_msg.data(), recv_msg.size());
    }
    udp_socket.close();

    mmr_conmunication::udp_message<mmr_conmunication::udp_data> build_recv_msg(recv_msg.data(), recv_msg.size(), mmr_conmunication::MR_UDP_OPERATOR::read_var);
    m_mr_udp_data = build_recv_msg.get_data();

    mmr_conmunication::udp_data update_domain_request;
    update_domain_request.set_err_code(m_mr_udp_data.get_err_code());
    update_domain_request.set_rev(m_mr_udp_data.get_rev());
    update_domain_request.set_mr_id(m_mr_udp_data.get_mr_id());
    update_domain_request.set_ip(m_mr_udp_data.get_ip());
    update_domain_request.set_netmask(m_mr_udp_data.get_netmask());
    update_domain_request.set_gateway(m_mr_udp_data.get_gateway());
    update_domain_request.set_dns(m_mr_udp_data.get_dns());
    update_domain_request.set_mac(m_mr_udp_data.get_mac());
    update_domain_request.set_ip_d(m_mr_udp_data.get_ip_d());
    update_domain_request.set_netmask_d(m_mr_udp_data.get_netmask_d());
    update_domain_request.set_gateway_d(m_mr_udp_data.get_gateway_d());
    update_domain_request.set_dns_d(m_mr_udp_data.get_dns_d());

    QVector<qint8> new_domain;
    new_domain.resize(256);
    QByteArray raw_data = ip.toLatin1();
    for (int i = 0; i != raw_data.length(); i++ ) {
        new_domain[i] = raw_data.at(i);
    }
    update_domain_request.set_domain(new_domain);

    update_domain_request.set_tcp_port(m_mr_udp_data.get_tcp_port());
    update_domain_request.set_udp_port(m_mr_udp_data.get_udp_port());
    update_domain_request.set_network_select(m_mr_udp_data.get_network_select());
    update_domain_request.set_sbs_status_report_interval(m_mr_udp_data.get_sbs_status_report_interval());
    update_domain_request.set_manufacturer_message(m_mr_udp_data.get_manufacturer_message());
    update_domain_request.set_dip_value(m_mr_udp_data.get_dip_value());

    mmr_conmunication::udp_message<mmr_conmunication::udp_data> update_pkg(update_domain_request, mmr_conmunication::write_var);
    QByteArray send_pkg2 = update_pkg.toBinary();
    QUdpSocket udp_socket2;
    udp_socket2.bind(QHostAddress::Any, 8888, QUdpSocket::ShareAddress );

    QByteArray recv_res;
    len = -1;
    timer.restart();
    while (len = udp_socket2.writeDatagram(send_pkg2, QHostAddress(m_id_ip_map[m_test_mmr_id]), mmr_conmunication::udp_port_send)
           , (!udp_socket2.waitForReadyRead(1000) || recv_res.isEmpty())) {

        qDebug() << "have writen [" << len << "] bytes to [" << m_id_ip_map[m_test_mmr_id] << ":" << mmr_conmunication::udp_port_send << "]";

        if (timer.elapsed() > (TIMEOUT * 1000)) {
            m_err_info = QString::fromLocal8Bit("接收终端数据超时，请重新设置IP！");
            QString str1 = MESSAGE_RED + m_err_info + MESSAGE_END;
            QMessageBox::warning(this, QString::fromLocal8Bit("警告"), str1);

            qDebug() << m_err_info;
            ui->statusBar->showMessage(m_err_info);
            ui->pushButton_ok->setEnabled(true);
            return ;
        }

        recv_res.resize(udp_socket2.pendingDatagramSize());
        udp_socket2.readDatagram(recv_res.data(), recv_res.size());
    }
    udp_socket2.close();

    mmr_conmunication::udp_message<mmr_conmunication::udp_data2> reply_pkg(recv_res.data(), recv_res.size(), mmr_conmunication::MR_UDP_OPERATOR::read_var);
    mmr_conmunication::udp_data2 reply_data = reply_pkg.get_data();
    quint32 err_code = reply_data.get_err_code();
    if (0 == err_code) {
        m_err_info = QString::fromLocal8Bit("主终端更新IP成功");
    } else {
        m_err_info = QString::fromLocal8Bit("主终端更新IP不成功");
    }
    qInfo() << m_err_info;
    QMessageBox::information(this, QString::fromLocal8Bit("操作结果"), m_err_info);
    ui->statusBar->showMessage(m_err_info);

    if (mr_restart(m_id_ip_map[m_test_mmr_id])) {
        m_err_info = QString::fromLocal8Bit("主终端重启成功");
    } else {
        m_err_info = QString::fromLocal8Bit("主终端重启不成功");
    }
    qInfo() << m_err_info;
    QMessageBox::information(this, QString::fromLocal8Bit("操作结果"), m_err_info);
    ui->statusBar->showMessage(m_err_info);
}
#endif

void MainWindow::setCam()
{

    if (!m_is_master) {
        if (!is_open(m_test_mr_id1, CMPB_TYPE::CAM)) {
            // if it is close, then open it
            open_close(true, m_cmpb_list.value(m_test_mr_id1).value(MIC), m_cmpb_list.value(m_test_mr_id1).value(PRE), m_cmpb_list.value(m_test_mr_id1).value(BAT));
            if (is_open(m_test_mr_id1, CMPB_TYPE::CAM)) {
                m_err_info = QString::fromLocal8Bit("打开从终端摄像头成功");
            } else {
                m_err_info = QString::fromLocal8Bit("打开从终端摄像头失败");
            }
        } else {
            m_err_info = QString::fromLocal8Bit("从终端摄像头已打开");
        }
    } else {
        if (!is_open(m_test_mmr_id, CMPB_TYPE::CAM)) {
            // if it is close, then open it
            open_close(true, m_cmpb_list.value(m_test_mr_id1).value(MIC), m_cmpb_list.value(m_test_mr_id1).value(PRE), m_cmpb_list.value(m_test_mr_id1).value(BAT));
            if (is_open(m_test_mmr_id, CMPB_TYPE::CAM)) {
                m_err_info = QString::fromLocal8Bit("打开主终端摄像头成功");
            } else {
                m_err_info = QString::fromLocal8Bit("打开主终端摄像头失败");
            }          
        } else {
            m_err_info = QString::fromLocal8Bit("主终端摄像头已打开");
        }
    } // if else

    qInfo() << m_err_info;
    ui->statusBar->showMessage(m_err_info);
    QMessageBox::information(this, QString::fromLocal8Bit("操作结果"), m_err_info);
}

void MainWindow::setMic()
{
    if (!m_is_master) {
        if (!is_open(m_test_mr_id1, CMPB_TYPE::MIC)) {
            // if it is close, then open it
            open_close(m_cmpb_list.value(m_test_mr_id1).value(CAM), true, m_cmpb_list.value(m_test_mr_id1).value(PRE), m_cmpb_list.value(m_test_mr_id1).value(BAT));
            if (is_open(m_test_mr_id1, CMPB_TYPE::MIC)) {
                m_err_info = QString::fromLocal8Bit("打开从终端麦克风成功");
            } else {
                m_err_info = QString::fromLocal8Bit("打开从终端麦克风失败");
            }           
        } else {
            m_err_info = QString::fromLocal8Bit("从终端麦克风已打开");
        }
    } else {
        if (!is_open(m_test_mmr_id, CMPB_TYPE::MIC)) {
            // if it is close, then open it
            open_close(m_cmpb_list.value(m_test_mr_id1).value(CAM), true, m_cmpb_list.value(m_test_mr_id1).value(PRE), m_cmpb_list.value(m_test_mr_id1).value(BAT));
            if (is_open(m_test_mmr_id, CMPB_TYPE::MIC)) {
                m_err_info = QString::fromLocal8Bit("打开主终端麦克风成功");
            } else {
                m_err_info = QString::fromLocal8Bit("打开主终端麦克风失败");
            }           
        } else {
            m_err_info = QString::fromLocal8Bit("主终端麦克风已打开");
        }
    } // if else

    qInfo() << m_err_info;
    ui->statusBar->showMessage(m_err_info);
    QMessageBox::information(this, QString::fromLocal8Bit("操作结果"), m_err_info);
}

void MainWindow::self_sleep(double time)
{
    QElapsedTimer timer;
    timer.start();
    while (timer.elapsed() < 1000 * time) {
        QCoreApplication::processEvents();
    }
}

void MainWindow::on_radioButton_line_clicked()
{
    ui->lineEdit_id_c->clear();
    ui->pushButton_start->setDisabled(true);
    ui->pushButton_ok->setEnabled(true);
}

void MainWindow::on_radioButton_wifi_clicked()
{
    ui->lineEdit_id_c->clear();
    ui->pushButton_start->setDisabled(true);
    ui->pushButton_ok->setEnabled(true);
}

void MainWindow::on_radioButton_2_clicked()
{
    // master mr
    //m_id_ip_map.clear();

    m_is_master = true;
    ui->lineEdit_mmr_id->clear();
    ui->pushButton_start->setDisabled(true);
    ui->lineEdit_mmr_id->setDisabled(true);
    ui->lineEdit_mr_id1->setEnabled(true);
    ui->lineEdit_mr_id2->setEnabled(true);
    ui->lineEdit_id_c->clear();

    ui->pushButton_ok->setEnabled(true);
}

void MainWindow::on_radioButton_clicked()
{
    //follower mr
    //m_id_ip_map.clear();
    m_is_master = false;
    ui->lineEdit_mr_id1->clear();
    ui->pushButton_start->setDisabled(true);
    ui->lineEdit_mmr_id->setEnabled(true);
    ui->lineEdit_mr_id1->setDisabled(true);
    ui->lineEdit_mr_id2->setEnabled(true);
    ui->lineEdit_id_c->setEnabled(true);
    ui->lineEdit_id_c->clear();
    ui->pushButton_ok->setEnabled(true);
}

void MainWindow::on_pushButton_ok_clicked()
{
    ui->pushButton_start->setDisabled(true);
    m_model_list_ptr->clear();
    ui->pushButton_ok->setDisabled(true);
    m_model_ptr->clear();
    bool ok;
    QHashIterator<device_mr_id, QString> i(m_id_ip_map);
    //主终端
    if (m_is_master)
    {
        //set mr_id(s) and mmr_id
        m_test_mr_id1 = ui->lineEdit_mr_id1->text().mid(2).toInt(&ok);
        if (!ok)
        {
            m_err_info = QString::fromLocal8Bit("主终端测试过程中，获取基准测试1号终端ID失败,请重新输入！");
            QString str1 = MESSAGE_RED + m_err_info + MESSAGE_END;
            QMessageBox::warning(this, QString::fromLocal8Bit("警告"), str1);
            qCritical() << m_err_info;
            ui->statusBar->showMessage(m_err_info);
            ui->pushButton_ok->setEnabled(true);
            return;
        }

        m_test_mr_id2 = ui->lineEdit_mr_id2->text().mid(2).toInt(&ok);
        if (!ok)
        {
            m_err_info = QString::fromLocal8Bit("主终端测试过程中，获取基准测试2号终端ID失败，请重新输入！");
            qCritical() << m_err_info;
            QString str1 = MESSAGE_RED + m_err_info + MESSAGE_END;
            QMessageBox::warning(this, QString::fromLocal8Bit("警告"), str1);
            ui->statusBar->showMessage(m_err_info);
            ui->pushButton_ok->setEnabled(true);
            return;
        }

        while (i.hasNext())
        {
            i.next();
            if (i.key() != m_test_mr_id1 && i.key() != m_test_mr_id2)
            {
                m_test_mmr_id = i.key();
            }
        }
    }
    //从终端
    else
    {
        m_test_mmr_id = ui->lineEdit_mmr_id->text().mid(2).toInt(&ok);
        if (!ok)
        {
            m_err_info = QString::fromLocal8Bit("从终端测试过程中，获取主终端ID失败，请重新输入！");
            qCritical() << m_err_info;
            QString str1 = MESSAGE_RED + m_err_info + MESSAGE_END;
            QMessageBox::warning(this, QString::fromLocal8Bit("警告"), str1);
            ui->statusBar->showMessage(m_err_info);
            ui->pushButton_ok->setEnabled(true);
            return ;
        }
        m_test_mr_id2 = ui->lineEdit_mr_id2->text().mid(2).toInt(&ok);
        if (!ok)
        {
            m_err_info = QString::fromLocal8Bit("从终端测试过程中，获取基准从终端ID失败，请重新输入！");
            qCritical() << m_err_info;
            QString str1 = MESSAGE_RED + m_err_info + MESSAGE_END;
            QMessageBox::warning(this, QString::fromLocal8Bit("警告"), str1);

            ui->statusBar->showMessage(m_err_info);
            ui->pushButton_ok->setEnabled(true);
            return ;
        }

        while (i.hasNext())
        {
           i.next();
           if (i.key() != m_test_mmr_id && i.key() != m_test_mr_id2)
           {
              m_test_mr_id1 = i.key();
           }
        }

    }

    qDebug() << "{ mmr_id = " << m_test_mmr_id << ", ip = " << m_id_ip_map.value(m_test_mmr_id) << "  }";
    qDebug() << "{ mr_id1 = " << m_test_mr_id1 << ", ip = " << m_id_ip_map.value(m_test_mr_id1) << "  }";
    qDebug() << "{ mr_id2 = " << m_test_mr_id2 << ", ip = " << m_id_ip_map.value(m_test_mr_id2) << "  }";

    //add by guosj start
    device_mr_id new_id = ui->lineEdit_id_c->text().mid(2).toInt(&ok);
    if (!ok)
    {
        m_err_info = QString::fromLocal8Bit("获取待测终端ID失败");
        qCritical() << m_err_info;
        QString str1 = MESSAGE_RED + m_err_info + MESSAGE_END;
        QMessageBox::warning(this, QString::fromLocal8Bit("警告"), str1);
        //ui->pushButton_ok->setEnabled(true);
        return ;
    }

    if (m_is_master)
    {
        if (m_test_mmr_id == new_id)
        {
            //ui->pushButton_start->setEnabled(true);
        }
        else
        {
            QString str1 = MESSAGE_RED +(QString::fromLocal8Bit("输入的待测终端UWB板ID与待测终端UWB板ID不匹配")) + MESSAGE_END;
            QMessageBox::warning(this, QString::fromLocal8Bit("警告"), str1);
            ui->pushButton_ok->setEnabled(true);
            return ;
        }
    }
    else
    {
        if (m_test_mr_id1 == new_id)
        {
            //ui->pushButton_start->setEnabled(true);
        }
        else
        {
            QString str1 = MESSAGE_RED +(QString::fromLocal8Bit("输入的待测终端UWB板ID与待测终端UWB板ID不匹配")) + MESSAGE_END;
            QMessageBox::warning(this, QString::fromLocal8Bit("警告"), str1);
            ui->pushButton_ok->setEnabled(true);
            return ;
        }
    }
    //guosj end

    QTime timer;
    timer.restart();
    while (m_id_ip_map.value(m_test_mmr_id).isEmpty() ||
           m_id_ip_map.value(m_test_mr_id1).isEmpty() ||
           m_id_ip_map.value(m_test_mr_id2).isEmpty())
    {
        if (timer.elapsed() > (TIMEOUT * 1000))
        {
            if (m_id_ip_map.value(m_test_mmr_id).isEmpty())
            {
               m_err_info = QString::fromLocal8Bit("收取终端[") + QString::number(m_test_mmr_id) + QString::fromLocal8Bit("]广播数据失败，请手动重启该终端!");
            }
            else if (m_id_ip_map.value(m_test_mr_id1).isEmpty())
            {
               m_err_info = QString::fromLocal8Bit("收取终端[") + QString::number(m_test_mr_id1) + QString::fromLocal8Bit("]广播数据失败，请手动重启该终端!");
            }
            else if (m_id_ip_map.value(m_test_mr_id2).isEmpty())
            {
               m_err_info = QString::fromLocal8Bit("收取终端[") + QString::number(m_test_mr_id2) + QString::fromLocal8Bit("]广播数据失败，请手动重启该终端!");
            }
            QString str1 = MESSAGE_RED + m_err_info + MESSAGE_END;
            QMessageBox::warning(this, QString::fromLocal8Bit("警告"), str1);
            qDebug() << m_err_info;
            ui->statusBar->showMessage(m_err_info);
            ui->pushButton_ok->setEnabled(true);
            return ;
        }
    }

    //设置有线或者无线模式
    mmr_conmunication::udp_data data_tmp;
    mmr_conmunication::udp_data2 read_var_request;
    read_var_request.set_err_code(0);
    mmr_conmunication::udp_message<mmr_conmunication::udp_data2> send_read_pkg(read_var_request, mmr_conmunication::MR_UDP_OPERATOR::read_var);
    QByteArray send_read_request = send_read_pkg.toBinary();
    QByteArray send_res;

    QHash<device_mr_id, QString>::const_iterator index;
    for (index = m_id_ip_map.begin(); index != m_id_ip_map.end(); ++index)
    {
        //获取原始数据
        QUdpSocket udp_socket;
        udp_socket.bind(QHostAddress::Any, 8899, QUdpSocket::ShareAddress);
        QByteArray recv_msg;
        int len = -1;
        timer.restart();
        while (len = udp_socket.writeDatagram(send_read_request, QHostAddress(index.value()), mmr_conmunication::udp_port_send)
               , (!udp_socket.waitForReadyRead(1000) || recv_msg.isEmpty()))
        {
            qDebug() << "have writen [" << len << "] bytes to [" << index.value() << ":" << mmr_conmunication::udp_port_send << "]";
            if (timer.elapsed() > (TIMEOUT * 1000))
            {
                m_err_info = QString::fromLocal8Bit("接收终端数据超时，请重新初始化测试环境！");
                QString str1 = MESSAGE_RED + m_err_info + MESSAGE_END;
                QMessageBox::warning(this, QString::fromLocal8Bit("警告"), str1);
                qDebug() << m_err_info;
                ui->statusBar->showMessage(m_err_info);
                ui->pushButton_ok->setEnabled(true);
                return ;
            }
            recv_msg.resize(udp_socket.pendingDatagramSize());
            udp_socket.readDatagram(recv_msg.data(), recv_msg.size());
        }

        mmr_conmunication::udp_message<mmr_conmunication::udp_data> recv_data(recv_msg.data(), recv_msg.size(), mmr_conmunication::MR_UDP_OPERATOR::read_var);
        data_tmp = recv_data.get_data();



        //add by guosj start
        std::string ip = ui->lineEdit_ip_c->text().trimmed().toStdString();
        QVector<qint8> new_domain;
        new_domain.resize(256);
        for (int i = 0; i != ip.size(); i++)
        {
            new_domain[i]=(ip[i]);
        }
         new_domain[ip.size()] = ('\0');
        //guosj end

        //update to line model
        mmr_conmunication::udp_data update_data;
        update_data.set_err_code(data_tmp.get_err_code());
        update_data.set_rev(data_tmp.get_rev());
        update_data.set_mr_id(data_tmp.get_mr_id());
        update_data.set_ip(data_tmp.get_ip());
        update_data.set_netmask(data_tmp.get_netmask());
        update_data.set_gateway(data_tmp.get_gateway());
        update_data.set_dns(data_tmp.get_dns());
        update_data.set_mac(data_tmp.get_mac());
        update_data.set_ip_d(data_tmp.get_ip_d());
        update_data.set_netmask_d(data_tmp.get_netmask_d());
        update_data.set_gateway_d(data_tmp.get_gateway_d());
        update_data.set_dns_d(data_tmp.get_dns_d());
        update_data.set_domain(new_domain);
        update_data.set_tcp_port(data_tmp.get_tcp_port());
        update_data.set_udp_port(data_tmp.get_udp_port());
        if (ui->radioButton_wifi->isChecked())
        {
            update_data.set_network_select(2);
        }
        else
        {
            update_data.set_network_select(1);
        }
        update_data.set_sbs_status_report_interval(data_tmp.get_sbs_status_report_interval());
        update_data.set_manufacturer_message(data_tmp.get_manufacturer_message());
        update_data.set_dip_value(data_tmp.get_dip_value());
        mmr_conmunication::udp_message<mmr_conmunication::udp_data> send_updata_pkg(update_data, mmr_conmunication::MR_UDP_OPERATOR::write_var);
        send_res = send_updata_pkg.toBinary();

        recv_msg.clear();
        len = -1;
        timer.restart();
        while (len = udp_socket.writeDatagram(send_res, QHostAddress(index.value()), mmr_conmunication::udp_port_send)
               , (!udp_socket.waitForReadyRead(1000) || recv_msg.isEmpty()))
        {

            qDebug() << "have writen [" << len << "] bytes to [" << index.value() << ":" << mmr_conmunication::udp_port_send << "]";

            if (timer.elapsed() > (TIMEOUT * 1000))
            {
                m_err_info = QString::fromLocal8Bit("接收终端数据超时，请重新初始化测试环境！");
                QString str1 = MESSAGE_RED + m_err_info + MESSAGE_END;
                QMessageBox::warning(this, QString::fromLocal8Bit("警告"), str1);
                qDebug() << m_err_info;
                ui->statusBar->showMessage(m_err_info);
                ui->pushButton_ok->setEnabled(true);
                return ;
            }
            recv_msg.resize(udp_socket.pendingDatagramSize());
            udp_socket.readDatagram(recv_msg.data(), recv_msg.size());
        }

        //检查更新结果
        mmr_conmunication::udp_message<mmr_conmunication::udp_data2> resp_res(recv_msg.data(), recv_msg.size(), mmr_conmunication::MR_UDP_OPERATOR::read_var);
        quint32 err_code = resp_res.get_data().get_err_code();
        if (0 == err_code)
        {
            m_err_info = QString::fromLocal8Bit("成功更新网络模式[") + QString::number(index.key()) + QString::fromLocal8Bit("]");
        }
        else
        {
            m_err_info = QString::fromLocal8Bit("更新网络模式失败[") + QString::number(index.key()) + QString::fromLocal8Bit("]");
        }
        qDebug() << m_err_info;
        ui->statusBar->showMessage(m_err_info);
        udp_socket.close();

        //重启
        if (mr_restart(index.value()))
        {
           m_err_info = QString::fromLocal8Bit("[") + QString::number(index.key()) + "]" + QString::fromLocal8Bit("重启成功");
        }
        else
        {
           m_err_info = QString::fromLocal8Bit("[") + QString::number(index.key()) + "]" + QString::fromLocal8Bit("重启失败");
        }
        qDebug() << m_err_info;
        ui->statusBar->showMessage(m_err_info);
    }
    //添加一个定时器add by guosj
    m_timerid = startTimer(1000);
}

void MainWindow::timerEvent(QTimerEvent *ev)
{
    if(ev->timerId() == m_timerid)
    {
        static int a = 15;
        a--;
        if(a == 0)
        {
           a = 15;
           m_err_info = QString::fromLocal8Bit("初始化环境成功");
           qCritical() << m_err_info;
           QString str1 = MESSAGE_RED + m_err_info + MESSAGE_END;
           QMessageBox::information(this,QString::fromLocal8Bit("信息"), str1);
           ui->pushButton_start->setEnabled(true);
           killTimer(m_timerid);
        }
        m_err_info = QString::fromLocal8Bit("请在") + QString::number(a)+QString::fromLocal8Bit("秒后弹出初始化环境成功后点击开始测试...");
        ui->label_timeout->setText(m_err_info);
    }
}

void MainWindow::on_pushButton_clicked()
{
#if 0
    compare id
    ui->label_status->clear();
    ui->pushButton->setDisabled(true);
    bool ok;
    device_mr_id new_id = ui->lineEdit_id_c->text().mid(2).toInt(&ok);
    if (!ok)
    {
        m_err_info = QString::fromLocal8Bit("获取待测终端ID失败");
        qCritical() << m_err_info;
        ui->statusBar->showMessage(m_err_info);
        ui->label_status->setText(m_err_info);
        return ;
    }

    ui->label_status->clear();
    if (m_is_master)
    {
        if (m_test_mmr_id == new_id)
        {
            ui->label_status->setText(QString::fromLocal8Bit("比对通过"));
            ui->pushButton_start->setEnabled(true);
        }
        else
        {
            ui->label_status->setText(QString::fromLocal8Bit("比对不通过"));
        }
    }
    else
    {
        if (m_test_mr_id1 == new_id)
        {
            ui->label_status->setText(QString::fromLocal8Bit("比对通过"));
            ui->pushButton_start->setEnabled(true);
        }
        else
        {
            ui->label_status->setText(QString::fromLocal8Bit("比对不通过"));
        }
    }
#endif
}

void MainWindow::on_lineEdit_mmr_id_editingFinished()
{
    if (!ui->lineEdit_mmr_id->hasFocus())
    {
        return ;
    }
    if (ui->lineEdit_mmr_id->text().isEmpty()) return;
    if (ui->lineEdit_mmr_id->text().trimmed().size() != 12)
    {
        m_err_info = QString::fromLocal8Bit("输入基准测试主终端ID不正确");
        qCritical() << m_err_info;
        ui->statusBar->showMessage(m_err_info);
        QMessageBox::warning(this, QString::fromLocal8Bit("警告"), m_err_info);
        ui->lineEdit_mmr_id->clear();
        ui->lineEdit_mmr_id->setFocus();
        this->std_master_mr_id = ui->lineEdit_mmr_id->text().trimmed();
    }
}

void MainWindow::on_lineEdit_mr_id2_editingFinished()
{
    if (!ui->lineEdit_mr_id2->hasFocus())
    {
        return ;
    }
    if (ui->lineEdit_mr_id2->text().isEmpty()) return;
    if (ui->lineEdit_mr_id2->text().trimmed().size() != 12)
    {
        m_err_info = QString::fromLocal8Bit("输入基准测试从终端ID不正确");
        qCritical() << m_err_info;
        ui->statusBar->showMessage(m_err_info);
        QMessageBox::warning(this, QString::fromLocal8Bit("警告"), m_err_info);
        ui->lineEdit_mr_id2->clear();
        ui->lineEdit_mr_id2->setFocus();
    }
}

void MainWindow::on_lineEdit_mr_id1_editingFinished()
{
    if (!ui->lineEdit_mr_id1->hasFocus())
    {
        return ;
    }

    if (ui->lineEdit_mr_id1->text().isEmpty()) return ;

    if (ui->lineEdit_mr_id1->text().trimmed().size() != 12)
    {
        m_err_info = QString::fromLocal8Bit("输入基准测试从终端ID不正确");
        qCritical() << m_err_info;
        ui->statusBar->showMessage(m_err_info);
        QMessageBox::warning(this, QString::fromLocal8Bit("警告"), m_err_info);
        ui->lineEdit_mr_id1->clear();
        ui->lineEdit_mr_id1->setFocus();
    }
}

void MainWindow::on_lineEdit_id_editingFinished()
{
    if (!ui->lineEdit_id_c->hasFocus())
    {
        return ;
    }
    if (ui->lineEdit_id_c->text().isEmpty()) return;
    if (ui->lineEdit_id_c->text().trimmed().size() != 12)
    {
        m_err_info = QString::fromLocal8Bit("输入待测终端ID不正确");
        qCritical() << m_err_info;
        ui->statusBar->showMessage(m_err_info);
        QMessageBox::warning(this, QString::fromLocal8Bit("警告"), m_err_info);
        ui->lineEdit_id_c->clear();
        ui->lineEdit_id_c->setFocus();
    }
}

void MainWindow::on_actionsetIP_triggered()
{
}

void MainWindow::on_actionsetCam_triggered()
{
    setCam();
}

void MainWindow::on_actionsetMic_triggered()
{
    setMic();
}

void MainWindow::on_label_6_linkActivated(const QString &link)
{

}

void MainWindow::on_lineEdit_mmr_id_cursorPositionChanged(int arg1, int arg2)
{

}

void MainWindow::on_lineEdit_cursorPositionChanged(int arg1, int arg2)
{

}

void MainWindow::on_lineEdit_id_c_cursorPositionChanged(int arg1, int arg2)
{

}

void MainWindow::on_lineEdit_mmr_id_textChanged(const QString &arg1)
{
    if (this->std_master_mr_id != arg1)
    {
        this->ui->pushButton_ok->setEnabled(true);
        this->ui->pushButton_start->setEnabled(false);
    }
}

void MainWindow::on_lineEdit_mr_id2_textChanged(const QString &arg1)
{
    if (this->std_slaver_mmr_id2 != arg1)
    {
        this->ui->pushButton_ok->setEnabled(true);
        this->ui->pushButton_start->setEnabled(false);
    }
}

void MainWindow::on_lineEdit_mr_id1_textChanged(const QString &arg1)
{
    if (this->std_slaver_mmr_id1 != arg1)
    {
        this->ui->pushButton_ok->setEnabled(true);
        this->ui->pushButton_start->setEnabled(false);
    }
}

void MainWindow::on_lineEdit_id_c_textChanged(const QString &arg1)
{
    if(this->std_test_mmr_id != arg1)
    {
        this->ui->pushButton_ok->setEnabled(true);
        this->ui->pushButton_start->setEnabled(false);
    }
}

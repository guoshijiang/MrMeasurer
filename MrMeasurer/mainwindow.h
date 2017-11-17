#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "tcp_worker.h"
#include "udp_worker.h"

#include "../hdtasparser/hdtasparser/hdtasparser.h"
#include "common.h"

#include <QMainWindow>
#include <QCloseEvent>
#include <QStandardItemModel>
#include <QHash>
#include <atomic>
#include <QProgressDialog>
#include <QTimer>
#include <QAtomicInteger>

enum CMPB_TYPE {
    CAM,
    MIC,
    PRE,
    BAT
};

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

    void start();

protected:
    void closeEvent(QCloseEvent *event);

signals:
    void writeTcpMessage(const char * data, qint16 len);

public slots:

    void onTcpMessage(const QByteArray & msg);
    void onUdpMessage(const QByteArray & msg);
    void onMrUdpMessage(const QByteArray & msg);
    void onMrUdpListenMessage(const QByteArray & msg);

private slots:
    bool mr_restart(const QString & ip);
    void get_mr_udp_data();
    void on_pushButton_start_clicked();

    bool get_mr_list();
    bool update_auto_report_inter(quint32 interval);
    void open_close(bool cam, bool mic, bool pre, bool batt);
    bool is_open(device_mr_id id, CMPB_TYPE type);
    bool get_cmpb_status(device_mr_id id);

    void get_pb_interval(device_mr_id id);
    void get_mr_interval();

    void reset();
    //void timerEvent(QTimerEvent *ev);
    //void setIP();
    //void updateIP(const QString & ip);
    //void showTimelimit(int a);
    //void startProgress();
    void timerEvent(QTimerEvent *ev);
    void setCam();
    void setMic();

    void self_sleep(double time);

    void on_radioButton_line_clicked();
    void on_radioButton_wifi_clicked();
    void on_radioButton_2_clicked();
    void on_radioButton_clicked();
    void on_pushButton_ok_clicked();
    void on_pushButton_clicked();

    void on_actionsetIP_triggered();
    void on_actionsetCam_triggered();
    void on_actionsetMic_triggered();

    void on_lineEdit_id_editingFinished();
    void on_lineEdit_mmr_id_editingFinished();
    void on_lineEdit_mr_id2_editingFinished();
    void on_lineEdit_mr_id1_editingFinished();

    void on_label_6_linkActivated(const QString &link);

    void on_lineEdit_mmr_id_cursorPositionChanged(int arg1, int arg2);

    void on_lineEdit_cursorPositionChanged(int arg1, int arg2);

    void on_lineEdit_id_c_cursorPositionChanged(int arg1, int arg2);
    void on_lineEdit_mmr_id_textChanged(const QString &arg1);

    void on_lineEdit_mr_id2_textChanged(const QString &arg1);

    void on_lineEdit_mr_id1_textChanged(const QString &arg1);

    void on_lineEdit_id_c_textChanged(const QString &arg1);

private:
    Ui::MainWindow *ui;

    tcp_worker *    m_tcp_worker_ptr;
    udp_worker *    m_udp_worker_ptr;
    mr_udp_worker * m_mr_udp_worker_ptr;
    mr_listen_worker * m_mr_listen_worker_ptr;

    QStandardItemModel * m_model_ptr;
    QStandardItemModel * m_model_list_ptr;

    hdtas::mr_status_array  m_mr_status_array;
    device_mr_id            m_mmr_id;
    qint32                  m_pb_ar;

    bool m_has_ispd;  //mdf by guosj
    bool m_has_mr;    //mdf by guosj
    bool m_has_pb;    //mdf by guosj
    bool m_is_master;

    device_mr_id    m_test_mmr_id;
    device_mr_id    m_test_mr_id1;
    device_mr_id    m_test_mr_id2;

    //定时器部分
    int m_timerid;
    //QTimer m_timer;

    QString std_master_mr_id;
    QString std_slaver_mmr_id1;
    QString std_slaver_mmr_id2;
    QString std_test_mmr_id;

    QString m_err_info;
    QHash<device_mr_id, QHash<CMPB_TYPE, bool>> m_cmpb_list;
    QHash<device_mr_id, quint32>  m_pb_interval;
    QHash<device_mr_id, quint32>  m_mr_interval;
    QHash<device_mr_id, QString> m_id_ip_map;
    mmr_conmunication::udp_data m_mr_udp_data;
};

#endif // MAINWINDOW_H

#include "mainwindow.h"
#include <QApplication>
#include <QDir>
#include <QFile>
#include <QMutex>
#include <QDebug>
#include <QString>
#include <QDate>
#include <QDataStream>
#include <QTextStream>
#include <QtEndian>
#include<QTextCodec>

static void logMessageHandler(QtMsgType type, const QMessageLogContext & msgContext, const QString & msg) {
    QString txt;
    QDateTime date;
    txt.append(date.currentDateTime().toString()).append("[");
    switch (type) {
    case QtDebugMsg:
        txt.append("DEBUG").append("]");
        break;
    case QtInfoMsg:
        txt.append("INFO").append("]");
        break;
    case QtWarningMsg:
        txt.append("WARNING").append("]");
        break;
    case QtCriticalMsg:
        txt.append("CRITICAL").append("]");
        break;
    case QtFatalMsg:
        txt.append("FATAL").append("]");
        abort();
    }
    if (msgContext.file) txt.append(msgContext.file).append("]");
    if (msgContext.line) txt.append(QString::number(msgContext.line)).append("]");
    if (!msg.isEmpty()) txt.append(msg);

    QString logFileName = QDir::currentPath();
    int mid = logFileName.lastIndexOf("/");
    logFileName = logFileName.left(mid);

    logFileName.append("/log/");
    logFileName.append(QString::number(QDate::currentDate().year()))
               .append(QString("%1").arg(QDate::currentDate().month(), 2, 10, QChar('0')))
               .append(QString("%1").arg(QDate::currentDate().day(), 2, 10, QChar('0')))
               .append(QString(".log"));

    //logFileName = QDir::toNativeSeparators(logFileName);

    static QMutex mutex;
    QMutexLocker lock(&mutex);
    QFile logFile(logFileName);
    if (!logFile.open(QIODevice::Text | QIODevice::WriteOnly | QIODevice::Append)) {
        int err_no = logFile.error();
        QString err = logFile.errorString();
        qWarning() << err_no << ": "<< err;
    }
    QTextStream ts(&logFile);
    ts << txt << endl << flush;
    logFile.close();
}


int main(int argc, char *argv[])
{
    qInstallMessageHandler(logMessageHandler);
    QApplication a(argc, argv);
    MainWindow w;
    w.start();
    //w.show();

    return a.exec();
}

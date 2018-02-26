#ifndef MAINDIALOG_H
#define MAINDIALOG_H
#include <QtSql>
#include <QMainWindow>
#include <QtNetwork/QUdpSocket>
#include "dommodel.h"
#include <form_customview.h>
#include "form_viewcontainer.h"
#include "pcmpara.h"
#include <QTimer>
namespace Ui {
class MainDialog;
}

struct PcmFrameStruct{
    int pcmWordLen;
    int frameLen;
    int subFrameLen;
    int synWordLen;
    char* synWord;
    int synWordStation;
    int frameIDStattion;
    int frameIDLen;
    //FrameIDWay id副帧编码方式
};

class PcmAnalyThread : public QThread
{
    Q_OBJECT
public:
    PcmAnalyThread(QObject *parent);
    ~PcmAnalyThread();
     void ThreadInit(QObject *parent);
protected:
    void run();
public slots:
    void stopImmediately();
    void start();
private:
    QMutex m_stopLock;
    bool m_stopFalg;//退出
    bool m_suspendFalg;//暂停
//    QObject *m_parentPos;
    PcmFrameStruct m_pcmFrameStruct;
};

class ParaAnalyThread : public QThread
{
    Q_OBJECT
public:
    ParaAnalyThread(QObject *parent);
    ~ParaAnalyThread();
    void ThreadInit(QObject *parent);
public slots:
    void stopImmediately();
protected:
    void run();
private:
    QMutex m_stopLock;
    bool m_stopFalg;//退出
    PcmFrameStruct m_pcmFrameStruct;
};

class DataSaveThread : public QThread
{
    Q_OBJECT
public:
    DataSaveThread(QObject *parent);
    ~DataSaveThread();
    void ThreadInit(QObject *parent);
public slots:
    void stopImmediately();
protected:
    void run();
private:
    QMutex m_stopLock;
    bool m_stopFalg;//退出
    PcmFrameStruct m_pcmFrameStruct;
};

class MainDialog : public QMainWindow
{
    Q_OBJECT
public:
    explicit MainDialog(QWidget *parent = 0);
    ~MainDialog();
    QSqlDatabase m_sqlDb;
private slots:
    void onTableCurrowchange(const QModelIndex &current,const QModelIndex &previous);

    void on_actionact_test1_triggered();

    void on_actiontext2_triggered();

    void on_pushButton_acceptTask_clicked();

    void onReadData();//网络包接收响应

    void onPageIdChange(qlonglong curPageId);

    void onNomalCtrlUpdate();

    void onCurverCtrlUpdate();
private:
    bool connectDb();
    void startUdpRecive();
    Ui::MainDialog *ui;
    QString m_strDataBaseDriver;
    QString m_strDataBaseName;
    QString m_strHostName;
    QString m_strUser;
    QString m_strPassWord;
    QUdpSocket *m_udpSocket;
    quint16 m_udpPort;
    PcmAnalyThread *m_pcmAnalyThread;
    ParaAnalyThread *m_paraAnalyThread;
    DataSaveThread *m_dataSaveThread;
    QTimer *m_nomalCtrlUpdateTimer;
    QTimer *m_curverlCtrlUpdateTimer;
public:
    QList<Form_customView*> customViewList;//调用控件赋值的时候只需要调用该列表
    QList<Form_viewContainer*> viewContainerList;//清理时候只需要清理该列表
    PcmFrameStruct m_pcmFrameStruct;
    qlonglong m_ProjectId;
    qlonglong m_PlaneId;

//    qlonglong m_curPageId;
};


#endif // MAINDIALOG_H

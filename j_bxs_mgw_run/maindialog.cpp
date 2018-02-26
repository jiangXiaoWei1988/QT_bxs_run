#include "maindialog.h"
#include "ui_maindialog.h"
//#include <QApplication>
#include <QDesktopWidget>
#include <QPainter>
#include <QProxyStyle>
#include <QDebug>
#include <QFile>
#include <QFileDialog>
#include <QDateTime>
#include <QMessageBox>
#include <j_customcontrolled.h>
#include <QLabel>

//#include <qwt_plot_curve.h>
//#include <qwt_legend.h>
//#include <qwt_plot_canvas.h>
//#include <qwt_plot_grid.h>
//#include <qwt_plot_layout.h>
#include <QLayout>
#include <QLineEdit>
class CustomTabStyle : public QProxyStyle
{
public:
    QSize sizeFromContents(ContentsType type, const QStyleOption *option,
        const QSize &size, const QWidget *widget) const
    {
        QSize s = QProxyStyle::sizeFromContents(type, option, size, widget);
        if (type == QStyle::CT_TabBarTab) {
            s.transpose();
            s.rwidth() *= 2; // 设置每个tabBar中item的大小
            s.rheight() *= 2;
        }
        return s;
    }

    void drawControl(ControlElement element, const QStyleOption *option, QPainter *painter, const QWidget *widget) const
    {
        if (element == CE_TabBarTabLabel) {
            if (const QStyleOptionTab *tab = qstyleoption_cast<const QStyleOptionTab *>(option)) {
                QRect allRect = tab->rect;

                if((tab->state & QStyle::State_MouseOver) && (tab->state & QStyle::State_Enabled)){
                    painter->save();
                    painter->setPen(0x7d7d7d);
                    painter->setBrush(QBrush(0x7d7d7d));
                    painter->drawRect(allRect.adjusted(6, 6, -6, -6));
                    painter->restore();
                }
                if (tab->state & QStyle::State_Selected) {
                    painter->save();
                    painter->setPen(0x07961e);
                    painter->setBrush(QBrush(0x07961e));
//                    painter->setPen(0x89cfff);
//                    painter->setBrush(QBrush(0x89cfff));
                    painter->drawRect(allRect.adjusted(6, 6, -6, -6));
                    painter->restore();

                }


                QTextOption option;
                option.setAlignment(Qt::AlignCenter);

                if((tab->state & QStyle::State_Enabled))
                {
                    if (tab->state & QStyle::State_Selected) {
                        painter->setPen(0xf9a713);
                    }
                    else {
                        painter->setPen(0xffffff);
                    }
                }
                else {
                    painter->setPen(0x7d7d7d);
                }
                painter->drawText(allRect, tab->text, option);
                return;
            }
        }

        if (element == CE_TabBarTab) {
            QProxyStyle::drawControl(element, option, painter, widget);
        }
    }
};
    //缓冲池初始化
const int BUFFERPOOLSIZE = 4000;
QSemaphore g_freeByteForSave(BUFFERPOOLSIZE);
QSemaphore g_freeByteForAnaly(BUFFERPOOLSIZE);
QSemaphore g_usedByteForSave;
QSemaphore g_usedByteForAnaly;
char g_dataBufferPoolForRun[BUFFERPOOLSIZE];
char g_dataBUfferPoolForSave[BUFFERPOOLSIZE];
int g_iBufferPosForRun = 0;
int g_iBufferPosForSave = 0;

QSemaphore g_freeByteForSFrameData;
QSemaphore g_usedByteForSFrameData;
QList<char *>g_subFrameData;   //互斥

qlonglong g_curPageId;

QList<PcmPara*> g_pcmParaList;//参数列表
//int g_writePos = 0;
//int g_savePos = 0;
//int g_readPos = 0;
MainDialog::MainDialog(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainDialog)
{
    ui->setupUi(this);
    ui->tabWidget->tabBar()->setStyle(new CustomTabStyle);
//    for(int i = 0;i < ui->tabWidget->tabBar()->count();i ++)
//    {
//        ui->tabWidget->tabBar()->setTabEnabled(1,false);
//    }
    ui->tabWidget->tabBar()->setTabEnabled(1,false);
    /*读取配置信息
     * 验证config.xml文件是否存在，是否正确
     * 读取config.xml文件到
    */
    QDomDocument document;
    DomModel *xmlModel;
    QString configFilePath = QDir::currentPath() + "/config.xml";
    QFile file(configFilePath);
    if(file.open(QIODevice::ReadOnly))
    {
        if(document.setContent(&file))
        {
            xmlModel = new DomModel(document,DomModel::taskChoise,this);
            ui->treeView->setModel(xmlModel);
        }
        else {
             qDebug()<<"xml file error";
        }
    }
    else
    {
        qDebug()<<"file open fail";
    }
    document.clear();
    file.close();
    QObject::connect(ui->treeView->selectionModel(),SIGNAL(currentChanged(QModelIndex,QModelIndex)),this,SLOT(onTableCurrowchange(const QModelIndex &, const QModelIndex &)));
    //获取盘符
    foreach (QFileInfo fileInfo, QDir::drives()) {
        ui->comboBox_Dir->addItem(fileInfo.absolutePath());
    }
    if(ui->comboBox_Dir->count() > 1)
        ui->comboBox_Dir->setCurrentIndex(1);
    //读取ini文件
    QString setUpFilePath = QDir::currentPath() + "/setUp.ini";
    QFile setUpfile(setUpFilePath);
    if(setUpfile.open(QIODevice::ReadOnly))
    {
        QString strErrMsg;
        if(document.setContent(&setUpfile,&strErrMsg))
        {
            QDomNodeList connect = document.elementsByTagName("connect");
            if(connect.length() == 1)
            {
                if(connect.at(0).isElement())
                {
                    QDomElement conElement = connect.at(0).toElement();
                    m_strDataBaseDriver = conElement.firstChildElement("dataBaseDriver").text();
                    m_strDataBaseName = conElement.firstChildElement("dataBaseName").text();
                    m_strHostName = conElement.firstChildElement("hostName").text();
                    m_strUser = conElement.firstChildElement("user").text();
                    m_strPassWord = conElement.firstChildElement("passWord").text();
                    m_udpPort = (qint16)conElement.firstChildElement("udpPort").text().toInt();
                }
            }
            else
            {
                QMessageBox::critical(this,tr("error"),tr("connect.length() != 1"));
            }
        }
        else
        {
            QMessageBox::critical(this,tr("error"),tr("setup file error \n") + strErrMsg);
            for(int i = 0;i < ui->tabWidget->tabBar()->count();i ++)
                ui->tabWidget->tabBar()->setTabEnabled(i,false);
            ui->tab_setUp->setEnabled(false);
        }
    }
    else
    {
        QMessageBox::critical(this,tr("error"),tr("setup file open file"));
        for(int i = 0;i < ui->tabWidget->tabBar()->count();i ++)
            ui->tabWidget->tabBar()->setTabEnabled(i,false);
        ui->tab_setUp->setEnabled(false);
    }
    setUpfile.close();
    //连接数据库
    if(!connectDb())
    {
        for(int i = 0;i < ui->tabWidget->tabBar()->count();i ++)
            ui->tabWidget->tabBar()->setTabEnabled(i,false);
        ui->tab_setUp->setEnabled(false);
    }

    //初始化线程
    g_curPageId = -1;//初始化为-1 表示不可用
    m_pcmAnalyThread = new PcmAnalyThread(this);
    m_paraAnalyThread = new ParaAnalyThread(this);
    m_dataSaveThread = new DataSaveThread(this);
    //
    m_udpSocket = new QUdpSocket(this);
    //全屏
//    this->setGeometry(0,0,QApplication::desktop()->width(),QApplication::desktop()->height());
    ui->horizontalSlider->setVisible(false);
    //初始化控件更新的timer
    m_nomalCtrlUpdateTimer = new QTimer(this);
    connect(m_nomalCtrlUpdateTimer,&QTimer::timeout,this,&MainDialog::onNomalCtrlUpdate);

    m_curverlCtrlUpdateTimer = new QTimer(this);
    connect(m_curverlCtrlUpdateTimer,&QTimer::timeout,this,&MainDialog::onCurverCtrlUpdate);

    return;
}

MainDialog::~MainDialog()
{
    m_sqlDb.close();
    if(m_pcmAnalyThread->isRunning())
    {
        m_pcmAnalyThread->stopImmediately();

    }
    if(m_paraAnalyThread->isRunning())
    {
        m_paraAnalyThread->stopImmediately();
    }
    if(m_dataSaveThread->isRunning())
    {
        m_dataSaveThread->stopImmediately();
    }
    g_freeByteForSave.release(BUFFERPOOLSIZE);
    g_freeByteForAnaly.release(BUFFERPOOLSIZE);
    g_usedByteForAnaly.release(BUFFERPOOLSIZE);
    g_usedByteForSave.release(BUFFERPOOLSIZE);

    g_freeByteForSFrameData.release();
    g_usedByteForSFrameData.release();
    m_paraAnalyThread->wait();
    m_pcmAnalyThread->wait();
    m_dataSaveThread->wait();
    m_udpSocket->close();
    delete ui;
}

void MainDialog::onTableCurrowchange(const QModelIndex &current, const QModelIndex &previous)
{
    QString path;
    path += ui->comboBox_Dir->currentText();
    path += current.parent().data().toString() + "/";
    path += current.data().toString();
    path += QDateTime::currentDateTime().toString("yyyy-mm-dd hh:mm:ss") + ".data";
    ui->lineEdit_savePath->setText(path);
}

void MainDialog::on_actionact_test1_triggered()
{
//    QwtPlotCanvas *canvas = new QwtPlotCanvas();
//    canvas->setFocusIndicator( QwtPlotCanvas::CanvasFocusIndicator );
//    canvas->setFocusPolicy( Qt::StrongFocus );
//    canvas->setPalette( Qt::black );
//    ui->qwtPlot->setCanvas( canvas );
//    QwtPlotCurve *curve1 = new QwtPlotCurve("Curve_1");
//    QwtPlotCurve *curve2 = new QwtPlotCurve("Curve_2");

//    curve1->setStyle(QwtPlotCurve::CurveStyle::Lines);
////    curve1->setData();
////    curve2->setData();
//    curve1->attach(ui->qwtPlot);
//    curve2->attach(ui->qwtPlot);
//    ui->qwtPlot->replot();

    return;
//    ui->j_customControlLed1->setGeometry(20,20,50,250);
    QDomDocument domDocument;
//    domDocument.createAttribute("aa");
    domDocument.appendChild(domDocument.createElement("paraFormula"));
    qDebug()<<domDocument.firstChild().toElement().tagName();

//    qDebug()<<domDocument.toString();
//    qDebug()<<domDocument.firstChild();
    return;
    Form_customView *formTemp = new Form_customView(0,this);
    ui->tabWidget->insertTab(ui->tabWidget->tabBar()->count() - 1,formTemp,"testView");
    return;
    QDomDocument document("setUp");
    QString setUpFilePath = QDir::currentPath() + "/setUp.ini";
    QFile setUpfile(setUpFilePath);
    if(setUpfile.open(QIODevice::ReadOnly))
    {
        QString strErrMsg;
        if(document.setContent(&setUpfile,&strErrMsg))
        {
            QDomNodeList connect = document.elementsByTagName("connect");
            if(connect.length() == 1)
            {
                if(connect.at(0).isElement())
                {
                    QDomElement conElement = connect.at(0).toElement();
                    m_strDataBaseName = conElement.firstChildElement("dataBaseName").text();
                    m_strHostName = conElement.firstChildElement("hostName").text();
                    m_strUser = conElement.firstChildElement("user").text();
                    m_strPassWord = conElement.firstChildElement("passWord").text();
                    qDebug()<<"strDataBaseName"<<m_strDataBaseName;
                    qDebug()<<"strHostName"<<m_strHostName;
                    qDebug()<<"strUser"<<m_strUser;
                    qDebug()<<"strPassWord"<<m_strPassWord;
                }
            }
            else
            {
                //获取配置信息失败  报错
            }
        }
        else
        {
            QMessageBox::critical(this,tr("error"),tr("setup file error \n") + strErrMsg);
            for(int i = 0;i < ui->tabWidget->tabBar()->count();i ++)
                ui->tabWidget->tabBar()->setTabEnabled(i,false);
            ui->tab_setUp->setEnabled(false);
        }
    }
    else
    {
        QMessageBox::critical(this,tr("error"),tr("setup file open file"));
        for(int i = 0;i < ui->tabWidget->tabBar()->count();i ++)
            ui->tabWidget->tabBar()->setTabEnabled(i,false);
        ui->tab_setUp->setEnabled(false);
    }
    setUpfile.close();
    return;
    int iResult = ui->tabWidget->indexOf(ui->tab_setUp);
    if(iResult == -1)
    {
        iResult = ui->tabWidget->addTab(ui->tab_setUp,"setup");
    }
    ui->tab_setUp->hide();
//    QWidget *widge = new QWidget(this);
//    ui->tabWidget->addTab(widge,"111");
}

void MainDialog::on_actiontext2_triggered()
{
    m_pcmAnalyThread->stopImmediately();
    m_pcmAnalyThread->wait();
    return;

    int iResult = ui->tabWidget->indexOf(ui->tab_setUp);
    if(iResult != -1)
    {
        ui->tabWidget->removeTab(iResult);
    }
}

void MainDialog::on_pushButton_acceptTask_clicked()
{
    //判断当前节点是否为任务节点
    //读取节点信息初始化任务
    if(!ui->treeView->currentIndex().isValid())
    {
        QMessageBox::warning(this,tr("warning"),tr("please choise valid task"));
        return;
    }
    if(ui->checkBox_isSave->checkState() == Qt::Checked)
    {
        if(ui->lineEdit_savePath->text().isEmpty())
        {
            qDebug()<<"on_pushButton_acceptTask_clicked   ui->lineEdit_savePath->text().isEmpty()";
            return;
        }
    }
    //获取窗体信息
    QModelIndex interfaceModel = ui->treeView->currentIndex().child(3,0);
    for(int i = 0;interfaceModel.child(i,0).isValid();i ++)
    {
        QModelIndex viewNode = interfaceModel.child(i,0);
        if(!viewNode.child(0,0).isValid())//如果该类窗体没有子窗体则跳过
            continue;
        Form_viewContainer *viewContainer = new Form_viewContainer(&viewNode,this);
        viewContainerList.append(viewContainer);
        ui->tabWidget->insertTab(ui->tabWidget->tabBar()->count() - 1,viewContainer,viewNode.data().toString().left(viewNode.data().toString().length() - 2));        
        connect(viewContainer,&Form_viewContainer::pageIdChange, this,&MainDialog::onPageIdChange);
    }
    //按钮的使能
    ui->tabWidget->tabBar()->setTabEnabled(0,false);
    ui->tabWidget->tabBar()->setTabEnabled(1,true);
    ui->tabWidget->tabBar()->setTabEnabled(ui->tabWidget->tabBar()->count() - 1,false);
    //读取下级节点，构造frameStruct 获取 projectId planeId 初始化线程
    //projectId planeId
    DomItem* planeItem =  static_cast<DomItem*>(ui->treeView->currentIndex().internalPointer());
    m_PlaneId = planeItem->getId();
    DomItem *projectItem = static_cast<DomItem*> (ui->treeView->currentIndex().parent().internalPointer());
    m_ProjectId = projectItem->getId();
    QModelIndex frameStructModel = ui->treeView->currentIndex().child(1,0);
    DomItem * curModeItem = static_cast<DomItem *>(frameStructModel.internalPointer());
    if(curModeItem->node().isElement())
    {
       QDomElement qelement = curModeItem->node().toElement();
       QDomAttr attrPcmWordLen = qelement.attributeNode("pcmWordLen");
       QDomAttr attrFrameLen = qelement.attributeNode("frameLen");
       QDomAttr attrSubFrameLen = qelement.attributeNode("subFrameLen");
       QDomAttr attrSynWord = qelement.attributeNode("synWord");
       QDomAttr attrSynWordLen = qelement.attributeNode("synWordLen");
       QDomAttr attrSynWordStation = qelement.attributeNode("synWordStation");
       QDomAttr attrFrameIDStattion = qelement.attributeNode("frameIDStattion");
       QDomAttr attrFrameIDLen = qelement.attributeNode("frameIDLen");
   //    QDomAttr attrFrameIDWay = infoNode.attributeNode("FrameIDWay");
       m_pcmFrameStruct.pcmWordLen = attrPcmWordLen.value().toInt();
       m_pcmFrameStruct.frameLen = attrFrameLen.value().toInt();
       m_pcmFrameStruct.subFrameLen = attrSubFrameLen.value().toInt();
       m_pcmFrameStruct.synWordLen = attrSynWordLen.value().toInt();
       if(m_pcmFrameStruct.synWord != NULL)
           m_pcmFrameStruct.synWord = NULL;
       m_pcmFrameStruct.synWord = new char[m_pcmFrameStruct.synWordLen];
       //字符串转hex
       QString strHead = attrSynWord.value().toUtf8();
       for(int i = 0;i <m_pcmFrameStruct.synWordLen;i ++)
       {
           QString strTemp = strHead.left(2);
           strHead = strHead.right(strHead.length() - 2);
           m_pcmFrameStruct.synWord[i] = strTemp.toInt(0,16);
       }
//       memcpy(m_pcmFrameStruct.synWord,attrSynWord.value().data(),m_pcmFrameStruct.synWordLen);
       m_pcmFrameStruct.synWordStation = attrSynWordStation.value().toInt();
       m_pcmFrameStruct.frameIDLen = attrFrameIDLen.value().toInt();
       m_pcmFrameStruct.frameIDStattion = attrFrameIDStattion.value().toInt();
    }
    //delete申请的内存，根据配置信息重新申请
    g_subFrameData.clear();
    for(int i = 0;i < (m_pcmFrameStruct.subFrameLen * 8);i ++)
    {
        char * charPos = new char[m_pcmFrameStruct.frameLen];
        g_subFrameData.append(charPos);
    }
    //初始化参数列表
    g_pcmParaList.clear();
    //遍历controlSubList表，根据pageId controlId从customViewList获取控件指针    在根据paraId从ParaTable获取参数信息
    QSqlTableModel *sqlModelControlSubList = new QSqlTableModel(this,m_sqlDb);
    sqlModelControlSubList->setTable("controlSubList");
    sqlModelControlSubList->setFilter(QString("projectId = %1 and planeId = %2").arg(m_ProjectId).arg(m_PlaneId));
    sqlModelControlSubList->select();
    QSqlTableModel *sqlModelParaTable = new QSqlTableModel(this,m_sqlDb);
    sqlModelParaTable->setTable("ParaTable");
    sqlModelParaTable->setFilter(QString("projectId = %1 and planeId = %2").arg(m_ProjectId).arg(m_PlaneId));
    sqlModelParaTable->select();
    QSqlTableModel *sqlModelparaDisplay = new QSqlTableModel(this,m_sqlDb);
    sqlModelparaDisplay->setTable("paraDisplay");
    for(int i = 0;i < sqlModelControlSubList->rowCount();i ++)
    {
        QSqlRecord controlSubListRecord = sqlModelControlSubList->record(i);
        qlonglong qllParaId = controlSubListRecord.value("paraId").toLongLong();
        qlonglong qllCtrlId = controlSubListRecord.value("controlId").toLongLong();
        qlonglong qllPageId = controlSubListRecord.value("pageId").toLongLong();
        PcmPara *newPamPara = new PcmPara(this);
        g_pcmParaList.append(newPamPara);
        PcmPara::ParaStruct paraStruct;
        sqlModelParaTable->setFilter(QString("projectId = %1 and planeId = %2 and id = %3")
                                     .arg(m_ProjectId).arg(m_PlaneId).arg(qllParaId));
        if(sqlModelParaTable->rowCount() != 1)
        {
            qDebug()<<QString("projectId = %1 and planeId = %2 and paraId = %3")
                      .arg(m_ProjectId).arg(m_PlaneId).arg(qllParaId);
            qDebug()<<"sqlModelParaTable->rowCount() != 1" << sqlModelParaTable->rowCount();
            continue;
        }
        paraStruct.paraName = sqlModelParaTable->record(0).value("paraName").toString();
        paraStruct.subFrameIndex = sqlModelParaTable->record(0).value("subFrameId").toInt();
        paraStruct.sByte = sqlModelParaTable->record(0).value("sByte").toInt();
        paraStruct.sBit = sqlModelParaTable->record(0).value("sBit").toInt();
        paraStruct.eBit = sqlModelParaTable->record(0).value("eBit").toInt();
        paraStruct.isSyn = sqlModelParaTable->record(0).value("isSyn").toBool();
        paraStruct.sSubFrame = sqlModelParaTable->record(0).value("sSubFrame").toInt();
        paraStruct.eSubFrame = sqlModelParaTable->record(0).value("eSubFrame").toInt();
        paraStruct.isLittleEndian = sqlModelParaTable->record(0).value("isLittleEndian").toBool();
        paraStruct.paraFormula = sqlModelParaTable->record(0).value("formula").toString();
        //查询参数显示表
        sqlModelparaDisplay->setFilter(QString("projectId = %1 and planeId = %2 and paraId = %3").arg(m_ProjectId).arg(m_PlaneId).arg(qllParaId));       
        sqlModelparaDisplay->select();
        QList<PcmPara::ParaDisplayStruct> paraDisplayStruct;
        for(int i = 0;i < sqlModelparaDisplay->rowCount();i ++)
        {
            PcmPara::ParaDisplayStruct newParaDisplay;
            newParaDisplay.max = sqlModelparaDisplay->record(i).value("max").toDouble();
            newParaDisplay.min = sqlModelparaDisplay->record(i).value("min").toDouble();
            QString strColor = sqlModelparaDisplay->record(i).value("led").toString();
            newParaDisplay.text= sqlModelparaDisplay->record(i).value("displayText").toString();
            if(strColor == "silver")
            {
                newParaDisplay.ledColor = 0;
            }else if(strColor == "blue")
            {
                newParaDisplay.ledColor = 1;
            }
            else if(strColor == "green")
            {
                newParaDisplay.ledColor = 2;
            }
            else if(strColor == "orange")
            {
                newParaDisplay.ledColor = 3;
            }
            else if(strColor == "red")
            {
                newParaDisplay.ledColor = 4;
            }
            else if(strColor == "yellow")
            {
                newParaDisplay.ledColor = 5;
            }

            paraDisplayStruct.append(newParaDisplay);
        }
        //
        foreach (Form_customView *curForm, customViewList) {           
            if(curForm->getPageId() == qllPageId)
            {
                foreach (Form_customView::controlStruct curCtrl, curForm->m_ctrlList) {                
                    if(curCtrl.ctrlId == qllCtrlId)
                    {
                        //初始化参数
                        newPamPara->InitPara(curCtrl.handle,curCtrl.ctrlType,qllPageId,paraStruct,paraDisplayStruct);
                        break;
                    }
                }
                break;
            }
        }
    }
    g_freeByteForSFrameData.release(m_pcmFrameStruct.subFrameLen * 8);
    //启动线程
    m_pcmAnalyThread->ThreadInit(this);
    m_pcmAnalyThread->start();
    m_paraAnalyThread->ThreadInit(this);
    m_paraAnalyThread->start();
    if(ui->checkBox_isSave->checkState() == Qt::Checked)
    {
        m_dataSaveThread->ThreadInit(this);
        m_dataSaveThread->start();
    }
    //开启udp接收
    g_iBufferPosForRun = 0;
    g_iBufferPosForSave = 0;
    startUdpRecive();
    //开启控件更新timer
    m_nomalCtrlUpdateTimer->start(100);
    m_curverlCtrlUpdateTimer->start(1);
}

void MainDialog::onReadData()
{
    int iAnalyTimeoutTimes = 0;
    int iSaveTimeoutTimes = 0;
    do{
        int iDataGramSize = m_udpSocket->pendingDatagramSize();
        //将数据先存入本地缓存中
        QByteArray dataGram;
        dataGram.resize(iDataGramSize);
        m_udpSocket->readDatagram(dataGram.data(),iDataGramSize);        
        //实时存储数据的缓冲池
        if(ui->checkBox_isSave->checkState() == Qt::Checked)
        {
            if(g_freeByteForSave.tryAcquire(iDataGramSize,1))
            {
                if(iDataGramSize > BUFFERPOOLSIZE - g_iBufferPosForSave)
                {
                    memcpy(&g_dataBUfferPoolForSave[g_iBufferPosForSave],dataGram.data(),BUFFERPOOLSIZE - g_iBufferPosForSave);
                    memcpy(g_dataBUfferPoolForSave,&dataGram.data()[BUFFERPOOLSIZE - g_iBufferPosForSave],iDataGramSize - (BUFFERPOOLSIZE - g_iBufferPosForSave));
                    g_iBufferPosForSave = iDataGramSize - (BUFFERPOOLSIZE - g_iBufferPosForSave);
                }
                else
                {
                    memcpy(g_dataBUfferPoolForSave + g_iBufferPosForSave,dataGram.data(),iDataGramSize);
                    g_iBufferPosForSave += iDataGramSize;
                }
                g_usedByteForSave.release(iDataGramSize);
                iSaveTimeoutTimes = 0;
            }
            else {
                iSaveTimeoutTimes ++;
                if(m_dataSaveThread->isRunning() && iSaveTimeoutTimes > 3)
                    qDebug()<<"g_freeByteForSave.tryAcquire(iDataGramSize,1)" << iSaveTimeoutTimes;
            }
        }
        //实时处理数据的缓冲池
        if(g_freeByteForAnaly.tryAcquire(iDataGramSize,1)) //
        {
            if(iDataGramSize > BUFFERPOOLSIZE - g_iBufferPosForRun)
            {
                memcpy(&g_dataBufferPoolForRun[g_iBufferPosForRun],dataGram.data(),BUFFERPOOLSIZE - g_iBufferPosForRun);
                memcpy(g_dataBufferPoolForRun,&dataGram.data()[BUFFERPOOLSIZE - g_iBufferPosForRun],iDataGramSize - (BUFFERPOOLSIZE - g_iBufferPosForRun));
                g_iBufferPosForRun = iDataGramSize - (BUFFERPOOLSIZE - g_iBufferPosForRun);
            }
            else
            {
//                m_udpSocket->readDatagram(g_dataBufferPoolForRun + g_iBufferPosForRun,iDataGramSize);
                memcpy(g_dataBufferPoolForRun + g_iBufferPosForRun,dataGram.data(),iDataGramSize);
                g_iBufferPosForRun += iDataGramSize;
            }
            g_usedByteForAnaly.release(iDataGramSize);
            iAnalyTimeoutTimes = 0;
        }
        else{
            iAnalyTimeoutTimes ++;
            if(m_pcmAnalyThread->isRunning() && iAnalyTimeoutTimes > 3)
                qDebug()<<"g_freeByteForAnaly.tryAcquire(iDataGramSize,1)" << iAnalyTimeoutTimes ;
        }
    }while (m_udpSocket->hasPendingDatagrams());
}

void MainDialog::onPageIdChange(qlonglong curPageId)
{
    g_curPageId = curPageId;
}

void MainDialog::onNomalCtrlUpdate()
{
    foreach (PcmPara* pcmParaItem, g_pcmParaList)
    {
        if(pcmParaItem->IsNeedDisPlay(g_curPageId))
        {
            if(pcmParaItem->GetControlType() == Form_customView::controlType::curve
                    || pcmParaItem->GetControlType() == Form_customView::controlType::view)
            {
                    continue;
            }
            pcmParaItem->UpdateControl();
        }
    }
}

void MainDialog::onCurverCtrlUpdate()
{
    foreach (PcmPara* pcmParaItem, g_pcmParaList)
    {
        if(pcmParaItem->IsNeedDisPlay(g_curPageId))
        {
            if(pcmParaItem->GetControlType() == Form_customView::controlType::curve)
            {
                  pcmParaItem->UpdateControl();
            }
        }
    }
}

bool MainDialog::connectDb()
{
    m_sqlDb = QSqlDatabase::addDatabase(m_strDataBaseDriver,"m_sqlDb");
    m_sqlDb.setDatabaseName(m_strDataBaseName);
    m_sqlDb.setHostName(m_strHostName);
    m_sqlDb.setPort(-1);
    if(!m_sqlDb.open(m_strUser,m_strPassWord))
    {
        QSqlDatabase::removeDatabase("m_sqlDb");
        QMessageBox::warning(this,tr("Unable to open database"), tr("An error occurred while ""opening the connection: ") + m_sqlDb.lastError().text());
        return false;
    }
    return true;
}

void MainDialog::startUdpRecive()
{
//    g_writePos = 0;
//    g_savePos = 0;
//    g_freeByteForSave
//    (BUFFERPOOLSIZE)
//    g_freeByteForAnaly
//    g_usedByteForSave
//    g_usedByteForAnaly
    if(!m_udpSocket->bind(m_udpPort))
        qDebug()<<"绑定失败";
    connect(m_udpSocket,&QUdpSocket::readyRead,this,&MainDialog::onReadData);
}

PcmAnalyThread::PcmAnalyThread(QObject *parent) : QThread(parent)
{
    QMutexLocker locker(&m_stopLock);
    m_stopFalg = false;

}

PcmAnalyThread::~PcmAnalyThread()
{
    //    qDebug()<<"~PcmThread";
}

void PcmAnalyThread::ThreadInit(QObject *parent)
{
    MainDialog *parentPos = (MainDialog*)parent;
    this->m_pcmFrameStruct = parentPos->m_pcmFrameStruct;
}

void PcmAnalyThread::run()
{
    int iSourceBufferPos = 0;
    char *buffer; //根据源缓冲池数据指针将元缓冲池中的数据读取到当前缓冲池，避免边界问题
    int iBufferLen = m_pcmFrameStruct.frameLen * 2;
    buffer = new char[iBufferLen];
    int iBufferPos = 0;
    int iNeedData = iBufferLen;
    int iSubFrameDataWritPos = 0;
    char * cSynWord = new char[m_pcmFrameStruct.synWordLen];
    int iPcmWordLenByByte = m_pcmFrameStruct.pcmWordLen / 8; //pcm字长按字节算
    bool isTurn = true;

    if(isTurn) //转化大小端同步头模式
    {
        for(int i = 0;i < m_pcmFrameStruct.synWordLen / iPcmWordLenByByte;i ++)
        {
            for(int j = 0;j < iPcmWordLenByByte;j ++)
            {
                cSynWord[j + (i * iPcmWordLenByByte)] = m_pcmFrameStruct.synWord[iPcmWordLenByByte - j + (i * iPcmWordLenByByte) - 1];
//                cSynWord[iPcmWordLenByByte - j - 1] = m_pcmFrameStruct.synWord[j];
            }
        }
    }else {
        memcmp(cSynWord,m_pcmFrameStruct.synWord,m_pcmFrameStruct.synWordLen);
    }
    while (!m_stopFalg) {
        g_usedByteForAnaly.acquire(iNeedData);
        g_freeByteForSFrameData.acquire();
        iBufferPos = 0;
        //结束循环
        if(m_stopFalg)
        {
            QMutexLocker locker(&m_stopLock);
            return;
        }
        if(iBufferLen > BUFFERPOOLSIZE - iSourceBufferPos)
        {
            memcpy(buffer,g_dataBufferPoolForRun + iSourceBufferPos,BUFFERPOOLSIZE - iSourceBufferPos);
            memcpy(buffer + BUFFERPOOLSIZE - iSourceBufferPos,g_dataBufferPoolForRun,iBufferLen - (BUFFERPOOLSIZE - iSourceBufferPos));
        }
        else {
            memcpy(buffer,g_dataBufferPoolForRun + iSourceBufferPos,iBufferLen);
        }
        while (iBufferLen - iBufferPos >=
               (m_pcmFrameStruct.frameLen + m_pcmFrameStruct.synWordStation + m_pcmFrameStruct.synWordLen))
        {
            if(buffer[iBufferPos + m_pcmFrameStruct.synWordStation] == cSynWord[0])
            {
                bool flag = false;
                for(int i = 0;i < m_pcmFrameStruct.synWordLen;i ++)
                {
                    if(buffer[iBufferPos + m_pcmFrameStruct.synWordStation + i] != cSynWord[i]
                            || buffer[iBufferPos + m_pcmFrameStruct.synWordStation + m_pcmFrameStruct.frameLen + i] != cSynWord[i])
                    {
                        flag = true;
                        break;
                    }
                }
                if(flag)
                {
                    iBufferPos ++;
                    continue;
                }
                //验证完毕 g_dataBufferPool[iBufferPos]为正确帧
                memcpy(g_subFrameData[iSubFrameDataWritPos],buffer + iBufferPos,m_pcmFrameStruct.frameLen);
                iSubFrameDataWritPos ++;
                if(iSubFrameDataWritPos >= g_subFrameData.count())
                    iSubFrameDataWritPos = 0;
                iBufferPos += m_pcmFrameStruct.frameLen;

                //高低字节颠倒 因为数据不一定是pcm字的
                if(isTurn)
                {
                    char cTemp;
                    for(int i = 0;i < m_pcmFrameStruct.frameLen / iPcmWordLenByByte;i ++)
                    {
                        for(int j = 0;j < iPcmWordLenByByte / 2;j ++)
                        {
                            cTemp = g_subFrameData[iSubFrameDataWritPos][j + (i * iPcmWordLenByByte)];
                            g_subFrameData[iSubFrameDataWritPos][j + (i * iPcmWordLenByByte)] = g_subFrameData[iSubFrameDataWritPos][iPcmWordLenByByte - j + (i * iPcmWordLenByByte) - 1];
                            g_subFrameData[iSubFrameDataWritPos][iPcmWordLenByByte - j + (i * iPcmWordLenByByte) - 1] = cTemp;
                        }
                    }
                }

                g_usedByteForSFrameData.release();//成功后释放一个帧buffer
                break;
            }
            else {
                iBufferPos ++;
            }
        }
        g_freeByteForAnaly.release(iBufferPos);    //释放的数量应该是这次处理的数据量 就是iSourceBufferPos步进的数量iBufferPos
        iSourceBufferPos += iBufferPos;
        iNeedData = iBufferPos;
        if(iSourceBufferPos > BUFFERPOOLSIZE)
            iSourceBufferPos -= BUFFERPOOLSIZE;
    }

    //动态演示
//    while (true) {
//        for(int i = 0;i < m_customViewList.count(); i ++) {
//            msleep(1);
//            for (int j = 0;j < m_customViewList[i]->m_ctrlList.count(); j ++){
//                if(m_customViewList[i]->m_ctrlList[j].ctrlType == Form_customView::controlType::label)
//                    continue;
////                msleep(1);
//                usleep(10);
//                if(m_customViewList[i]->m_ctrlList[j].ctrlType == Form_customView::controlType::led)
//                {
//                    j_customControlLed *curLed = (j_customControlLed *)m_customViewList[i]->m_ctrlList[j].handle;
//                    if(curLed->getLedColor() == j_customControlLed::ledColor::yellow)
//                        curLed->setLedColor(j_customControlLed::ledColor::silver);
//                    else
//                        curLed->setLedColor((j_customControlLed::ledColor)(curLed->getLedColor() + 1));
//                }
//                if(m_customViewList[i]->m_ctrlList[j].ctrlType == Form_customView::controlType::LineEdit)
//                {
//                    QLineEdit *curEdit = (QLineEdit *)m_customViewList[i]->m_ctrlList[j].handle;
//                    double dCur = curEdit->text().toDouble();
//                    if(dCur > 9)
//                        dCur -= 8.9;
//                    else
//                        dCur += 0.6;
//                    curEdit->setText(QString::QString("%1").arg(dCur));
////                    curEdit->setText("111");
//                }
//            }
//        }
//        msleep(800);
//        //退出
//        QMutexLocker locker(&m_stopLock);
//        if(m_stopFalg)
//            return;
//    }
}

void PcmAnalyThread::start()
{
    QThread::start();
}

void PcmAnalyThread::stopImmediately()
{
    QMutexLocker locker(&m_stopLock);
    m_stopFalg = true;
}

ParaAnalyThread::ParaAnalyThread(QObject *parent)
{
    QMutexLocker locker(&m_stopLock);
    m_stopFalg = false;
    MainDialog *parentPos = (MainDialog*)parent;
    this->m_pcmFrameStruct = parentPos->m_pcmFrameStruct;
}

ParaAnalyThread::~ParaAnalyThread()
{

}

void ParaAnalyThread::ThreadInit(QObject *parent)
{
    MainDialog *parentPos = (MainDialog*)parent;
    this->m_pcmFrameStruct = parentPos->m_pcmFrameStruct;
}

void ParaAnalyThread::stopImmediately()
{
    QMutexLocker locker(&m_stopLock);
    m_stopFalg = true;
}

void ParaAnalyThread::run()
{
    int iSubFrameDataReadPos = 0;
    while (true) {

        g_usedByteForSFrameData.acquire();
        //结束循环
        if(m_stopFalg)
        {
            QMutexLocker locker(&m_stopLock);
            return;
        }
        //获取页面Id
//        q curPageId = g_curPageId;
        //获取子帧id
        uint uintFramIndex = 0;
        memcpy(&uintFramIndex,g_subFrameData[iSubFrameDataReadPos] + m_pcmFrameStruct.frameIDStattion,m_pcmFrameStruct.frameIDLen);
        foreach (PcmPara* pcmParaItem, g_pcmParaList) {
            if(pcmParaItem->IsNeedAnaly(g_curPageId,uintFramIndex))
            {
                QVariant varValue = pcmParaItem->AnalyPara(g_subFrameData[iSubFrameDataReadPos]);
            }
        }
        g_freeByteForSFrameData.release();
        iSubFrameDataReadPos ++;
        if(iSubFrameDataReadPos >= m_pcmFrameStruct.subFrameLen * 8)
            iSubFrameDataReadPos = 0;
    }
}

DataSaveThread::DataSaveThread(QObject *parent) : QThread(parent)
{
    QMutexLocker locker(&m_stopLock);
    m_stopFalg = false;
}

DataSaveThread::~DataSaveThread()
{

}

void DataSaveThread::ThreadInit(QObject *parent)
{
    MainDialog *parentPos = (MainDialog*)parent;
    this->m_pcmFrameStruct = parentPos->m_pcmFrameStruct;
}

void DataSaveThread::stopImmediately()
{
    QMutexLocker locker(&m_stopLock);
    m_stopFalg = true;
}

void DataSaveThread::run()
{
    //数据存储
    //实时存储数据
    QFile file("data.dat");
    file.open(QIODevice::WriteOnly|QIODevice::Truncate);
    int iSavePos = 0;
    while (true) {
        g_usedByteForSave.acquire(BUFFERPOOLSIZE / 2);
        if(m_stopFalg)
        {
            QMutexLocker locker(&m_stopLock);
            return;
        }
        //以二进制文件形式写入
        if(BUFFERPOOLSIZE / 2 > BUFFERPOOLSIZE - iSavePos)
        {
            file.write(&g_dataBUfferPoolForSave[iSavePos],BUFFERPOOLSIZE - iSavePos);
            file.write(g_dataBUfferPoolForSave,(BUFFERPOOLSIZE / 2) - (BUFFERPOOLSIZE - iSavePos));
            iSavePos = (BUFFERPOOLSIZE / 2) - (BUFFERPOOLSIZE - iSavePos);
        }
        else
        {
            file.write(&g_dataBUfferPoolForSave[iSavePos],BUFFERPOOLSIZE / 2);
            iSavePos += BUFFERPOOLSIZE / 2;
        }
        g_freeByteForSave.release(BUFFERPOOLSIZE / 2);
    }
    file.close();
}

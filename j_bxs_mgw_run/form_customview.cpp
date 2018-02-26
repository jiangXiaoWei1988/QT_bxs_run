#include "ui_form_customview.h"
#include "maindialog.h"
#include <QSql>
#include "form_customview.h"
#include "dommodel.h"
#include <QDebug>
#include <QLineEdit>
#include <QLabel>
#include "plot.h"
#include <qwt_plot.h>
#include "j_customcontrolled.h"
//#include <qwt_plot_canvas.h>
//#include "j_custom_control.h"
//#include "customControl/customledctrl/customledctrl.h"
Form_customView::Form_customView(QModelIndex *pageModel, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Form_customView)
{
    ui->setupUi(this);
    MainDialog *parentPos = (MainDialog *)parent->parent();
    DomItem *item = static_cast<DomItem*>(pageModel->internalPointer());
    int width = item->getAtt("rect_width").toInt();
    int height = item->getAtt("rect_height").toInt();
//    qDebug()<<item->getName()<<width<<height;
//    m_defaultRect = QRect::QRect(0,0,width,height);
    m_defaultRect.setX(0);
    m_defaultRect.setY(0);
    m_defaultRect.setWidth(width);
    m_defaultRect.setHeight(height);
    if(m_defaultRect.width() == 0)
        m_defaultRect.setWidth(800);
    if(m_defaultRect.height() == 0)
        m_defaultRect.setHeight(600);

    this->setGeometry(m_defaultRect);
    m_pageid = item->getId();

    if(parentPos->m_sqlDb.isOpen())
    {       
        QSqlTableModel *sqlTableModel = new QSqlTableModel(this,parentPos->m_sqlDb);
        sqlTableModel->setTable("controlList");     
        sqlTableModel->setFilter(QString("pageId = %1").arg(m_pageid));
        sqlTableModel->select();
        for(int i = 0;i < sqlTableModel->rowCount();i ++)
        {
            QSqlRecord curRecord = sqlTableModel->record(i);
            controlStruct ctrlStruct;
            ctrlStruct.ctrlId = curRecord.value("controlId").toLongLong();           
            ctrlStruct.ctrlType = (controlType)curRecord.value("type").toUInt();
            ctrlStruct.ctrlRect.setX(curRecord.value("xPosition").toInt());
            ctrlStruct.ctrlRect.setY(curRecord.value("yPosition").toInt());
            ctrlStruct.ctrlRect.setWidth(curRecord.value("width").toInt());
            ctrlStruct.ctrlRect.setHeight(curRecord.value("height").toInt());
            ctrlStruct.att = curRecord.value("att").toString();
            if(ctrlStruct.ctrlRect.bottomRight().x() > m_defaultRect.bottomRight().x()
                    || ctrlStruct.ctrlRect.bottomRight().y() > m_defaultRect.bottomRight().y())
            {
                 qDebug()<<QString("control id = %1 out of frame");
                 continue;
            }
            m_ctrlList.append(ctrlStruct);
        }

//        foreach (controlStruct ctrlStruct, m_ctrlList) {
        for(int i = 0;i < m_ctrlList.count();i ++){
            switch (m_ctrlList[i].ctrlType) {
            case controlType::label:
                newLabelCtrl(m_ctrlList[i]);
                break;
            case controlType::LineEdit:
                newLineEditCtrl(m_ctrlList[i]);
                break;
            case controlType::led:
                newLedCtrl(m_ctrlList[i]);
                break;
            case controlType::curve:
                newCurveCtrl(m_ctrlList[i]);
                break;
            case controlType::view:
                newLabelCtrl(m_ctrlList[i]);
                break;
            default:
                break;
            }
        }
    }
}

Form_customView::~Form_customView()
{
    m_ctrlList.clear();
    delete ui;
}

qlonglong Form_customView::getPageId()
{
    return m_pageid;
}

void Form_customView::newLineEditCtrl(Form_customView::controlStruct &curCtrlStruct)
{
//    QMap<QString,QString> attList = m_releaseAtt(curCtrlStruct.att);
    QLineEdit *newCtrl = new QLineEdit(this);   
    newCtrl->setGeometry(curCtrlStruct.ctrlRect);
    newCtrl->sizePolicy().setHorizontalPolicy(QSizePolicy::Expanding);
    newCtrl->sizePolicy().setVerticalPolicy(QSizePolicy::Expanding);
    curCtrlStruct.handle = newCtrl;
    newCtrl->show();
}

void Form_customView::newLabelCtrl(Form_customView::controlStruct &curCtrlStruct)
{
    //解析属性
    QMap<QString,QString> attList = m_releaseAtt(curCtrlStruct.att);
    QLabel *newCtrl = new QLabel(attList.value("text"),this);
    newCtrl->setGeometry(curCtrlStruct.ctrlRect);
    newCtrl->sizePolicy().setHorizontalPolicy(QSizePolicy::Expanding);
    newCtrl->sizePolicy().setVerticalPolicy(QSizePolicy::Expanding);
    curCtrlStruct.handle = newCtrl;
    newCtrl->show();
}

void Form_customView::newLedCtrl(Form_customView::controlStruct &curCtrlStruct)
{
//    //解析属性    
    QMap<QString,QString> attList = m_releaseAtt(curCtrlStruct.att);
    j_customControlLed *newCtrl = new j_customControlLed(this);
    newCtrl->setText(attList.value("text"));
    newCtrl->setGeometry(curCtrlStruct.ctrlRect);
    newCtrl->sizePolicy().setHorizontalPolicy(QSizePolicy::Expanding);
    newCtrl->sizePolicy().setVerticalPolicy(QSizePolicy::Expanding);
    newCtrl->setLedColor(j_customControlLed::ledColor::silver);
    curCtrlStruct.handle = newCtrl;
    newCtrl->show();
}

void Form_customView::newCurveCtrl(Form_customView::controlStruct &curCtrlStruct)
{
    MainDialog *parentPos = (MainDialog *)this->parent()->parent();
//    QMap<QString,QString> attList = m_releaseAtt(curCtrlStruct.att);
//    QwtPlot *newCtrl = new QwtPlot(this);
//    newCtrl->setGeometry(curCtrlStruct.ctrlRect);
//    newCtrl->sizePolicy().setHorizontalPolicy(QSizePolicy::Expanding);
//    newCtrl->sizePolicy().setVerticalPolicy(QSizePolicy::Expanding);
//    curCtrlStruct.handle = newCtrl;
//    newCtrl->show();
    QSqlTableModel *sqlTableModel_controlSubList = new QSqlTableModel(this,parentPos->m_sqlDb);
    sqlTableModel_controlSubList->setTable("controlSubList");
    sqlTableModel_controlSubList->setFilter(QString("controlId = %1").arg(curCtrlStruct.ctrlId));
    sqlTableModel_controlSubList->select();
    QList<QString> paraNameList;
    for(int i = 0;i < sqlTableModel_controlSubList->rowCount();i ++)
    {
        QSqlRecord curRecord = sqlTableModel_controlSubList->record(i);
        paraNameList.append(curRecord.value("paraName").toString());
    }

    QMap<QString,QString> attList = m_releaseAtt(curCtrlStruct.att);
    QTime timePolling = QTime::fromString(attList.value("timeWide"),Qt::TextDate);
    QTime minPolling(0,0,10);
    if(timePolling < minPolling)
    {
        qDebug()<<"timePolling < QTime(1000)";
        return;
    }
    double maxValue = attList.value("valueRangeE").toDouble();
    double minValue = attList.value("valueRangeS").toDouble();
    if(maxValue < minValue)
    {
        qDebug()<<"maxValue < minValue"<<minValue<<maxValue;
        return;
    }
    QWidget *w = new QWidget( this );
    w->setGeometry(curCtrlStruct.ctrlRect);
    w->sizePolicy().setHorizontalPolicy(QSizePolicy::Expanding);
    w->sizePolicy().setVerticalPolicy(QSizePolicy::Expanding);
    QHBoxLayout *hLayout = new QHBoxLayout( w );
    Plot *newCtrl = new Plot(minValue,maxValue,paraNameList,w,timePolling.second());
//    Settings ctrlSettings = newCtrl->getSettings();
//    ctrlSettings.curve.brush.setColor(Qt::GlobalColor::red);
//    newCtrl->setSettings(ctrlSettings);
    hLayout->addWidget( newCtrl );
    curCtrlStruct.handle = newCtrl;
}

QMap<QString, QString> Form_customView::m_releaseAtt(QString attStr)//之后要将他改为模拟Creating Custom Widgets for Qt Designer)中domXml()的实现方案
{
    QMap<QString,QString> attList;
    QString ctrlAtt = attStr;
    while (true) {
        QString curAtt;
        int attLen = ctrlAtt.indexOf('|');
        if(attLen == -1 && ctrlAtt.length() != 0)
        {
            curAtt = ctrlAtt.left(attLen);
            attLen = ctrlAtt.length();
            ctrlAtt.clear();
        }
        else if(ctrlAtt.length() != 0){
            curAtt = ctrlAtt.left(attLen);
            ctrlAtt = ctrlAtt.right(ctrlAtt.length() - attLen - 1);
        }
        if(attLen == -1)
            break;
        int indexOfequal = curAtt.indexOf('=');
        attList.insert(curAtt.left(indexOfequal).trimmed(),curAtt.right(curAtt.length() - indexOfequal - 1).trimmed());
    }
    return attList;
}

//void Form_customView::on_pushButton_clicked()
//{
//    qDebug()<<m_ctrlList.count();
//    QLabel *labPos = (QLabel *)m_ctrlList.at(0).handle;
//    qDebug()<<m_ctrlList.at(0).att;
//    labPos->setText("change");
//}

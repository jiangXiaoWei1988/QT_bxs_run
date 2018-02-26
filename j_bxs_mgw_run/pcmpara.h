#ifndef PCMPARA_H
#define PCMPARA_H

#include <QObject>
#include <form_customview.h>
#include <QVariant>
#include <j_customcontrolled.h>
#include <plot.h>
class PcmPara : public QObject
{
    Q_OBJECT
public:  
    struct ParaStruct
    {
        QString paraName;
        uint sByte;
        uint sBit;
        uint eBit;
        bool isSyn;
        uint sSubFrame;
        uint eSubFrame;
        uint subFrameIndex;
        bool isLittleEndian;
        QString paraFormula;
    };
    struct ParaDisplayStruct
    {
        double min;
        double max;
        QString text;
        int ledColor;
    };
public:
    explicit PcmPara(QObject *parent = nullptr);
    bool InitPara(QWidget *handle, Form_customView::controlType controlType, qlonglong pageId, ParaStruct paraStruct, QList<ParaDisplayStruct> paraDisplayList);
    QVariant AnalyPara(char * data);//解析数据到工程量，将工程量赋值到m_value;同时返回工程量   cahr*指向子帧开头
    bool IsNeedAnaly(qlonglong pageId, uint iFrameIndex);//根据参数id、页面id、子帧路序,判断当前参数是否需要显示 异步块另外考虑
    bool IsNeedDisPlay(qlonglong pageId);
    bool UpdateControl();//利用m_controlHandle 直接更新控件显示
    Form_customView::controlType GetControlType();
    QString getParaName();
signals:
public slots:
private:
    bool m_isInitOk;
    QWidget *m_controlHandle;
    QVariant m_value; //根据不同控件存储不同类型的数据
    Form_customView::controlType m_ctrlType;
    qlonglong m_pageId;
    ParaStruct  m_paraStruct;
    bool AnayFormula();//解析参数

    QList<ParaDisplayStruct> m_paraDisplayList;
    int m_dataUpdateTimes; //参数更新次数，在每次参数显示时归零

    int m_formulaType;//0,1,2   -1代表没有解算公式
    int m_symbolBit;
    //line
    double m_lineCoefficient;
    double m_lineConstant;
    //interval
    QList<double> m_intervalMax;
    QList<double> m_intervalMin;
    QList<double> m_coefficient;
    QList<double> m_constant;
    //    QList<int,int,QString,QString>displayList;
};

#endif // PCMPARA_H

//class PcmParaAnaly
//{
//    Q_OBJECT
//public:
//    PcmParaAnaly(QObject *parent);
//    void InitPara();
//    QVariant AnalyPara();
//private:
//    bool isInitOk;
//};

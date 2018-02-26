#include "pcmpara.h"
#include <QDebug>
#include <QLineEdit>
#include <QDomNode>
#include <QLabel>
#include <QLibrary>

//typedef bool (BOOL_HkxPlay_BufferData)(const char *strBufferData, long intLen,HWND hWnd);
//HINSTANCE g_hDll=NULL;
//BOOL_HkxPlay_BufferData *HkxPlay_BufferData = NULL;
//QLibrary *hkxLab;
PcmPara::PcmPara(QObject *parent) : QObject(parent)
{
    m_isInitOk = false;  
}

bool PcmPara::InitPara(QWidget *handle, Form_customView::controlType controlType, qlonglong pageId, PcmPara::ParaStruct paraStruct,QList<ParaDisplayStruct> paraDisplayList)
{
    m_controlHandle = handle;
    m_ctrlType = controlType;
    m_paraStruct = paraStruct;
    m_pageId = pageId;
    m_dataUpdateTimes = 0;
    m_paraDisplayList = paraDisplayList;
    qDebug()<<paraDisplayList.count();
    //初始化转换公式，因为其中要进行读取解析操作。为了避免每次计算都重复该工作，于是放到这里一次性初始化
    m_symbolBit = -1;
    m_formulaType = -1;
    if(!m_paraStruct.paraFormula.isEmpty())
    {
        QDomDocument domDocument;
        QString strErrMsg;
        if(!domDocument.setContent(m_paraStruct.paraFormula,&strErrMsg))
        {
            qDebug()<<strErrMsg;
        }
        if(domDocument.firstChild().toElement().tagName() != "paraFormula")
        {
            m_formulaType = -1;
            qDebug()<<"Formula error";
        }
        else {
            m_formulaType = domDocument.firstChild().toElement().attribute("formulaType").toInt();
            QDomNodeList symbolBitList = domDocument.firstChild().toElement().elementsByTagName("symbol");
            if(symbolBitList.count() == 1)
                m_symbolBit = symbolBitList.at(0).toElement().text().toInt();
            switch (m_formulaType) {
            case 0:
            {
                QDomNodeList domListA = domDocument.firstChild().toElement().elementsByTagName("line_coefficient");
                QDomNodeList domListB = domDocument.firstChild().toElement().elementsByTagName("line_constant");
                if(domListA.count() != 1 || domListB.count() != 1)
                {
                    m_formulaType = -1;
                    qDebug()<<"Formula error";
                }
                else {
                    m_lineCoefficient = domListA.at(0).toElement().text().toDouble();

                    m_lineConstant = domListB.at(0).toElement().text().toDouble();
                }
            }
                break;
            case 1:
            {
                QDomNodeList count = domDocument.firstChild().toElement().elementsByTagName("count");
                if(count.count() != 1)
                {
                    m_formulaType = -1;
                    qDebug()<<"Formula error";
                }
                int iPointCount = count.at(0).toElement().text().toInt();
                for(int i = 0;i < iPointCount - 1;i ++)
                {
                    QDomNodeList pointA1 = domDocument.firstChild().toElement().elementsByTagName(QString("colA_%1").arg(i));
                    QDomNodeList pointB1 = domDocument.firstChild().toElement().elementsByTagName(QString("colB_%1").arg(i));
                    QDomNodeList pointA2 = domDocument.firstChild().toElement().elementsByTagName(QString("colA_%1").arg(i + 1));
                    QDomNodeList pointB2 = domDocument.firstChild().toElement().elementsByTagName(QString("colB_%1").arg(i + 1));
                    if(pointA1.count() != 1 || pointB1.count() != 1 || pointA2.count() != 1 || pointB2.count() != 1)
                    {
                        m_formulaType = -1;
                        qDebug()<<"Formula error";
                    }
                    double dA1 = pointA1.at(0).toElement().text().toDouble();
                    double dB1 = pointB1.at(0).toElement().text().toDouble();
                    double dA2 = pointA2.at(0).toElement().text().toDouble();
                    double dB2 = pointB2.at(0).toElement().text().toDouble();
                    m_intervalMin.append(dA1);
                    m_intervalMax.append(dA2);
                    m_coefficient.append((dB1 - dB2) / (dA1-dA2));
                    m_constant.append(dB1 - ((dB1 - dB2) / (dA1-dA2) * dA1));
                }
            }
                break;
            case 2:
            {

            }
                break;
            default:
                break;
            }
        }
    }
    else {
        m_formulaType = -1;
    }
    //加载视频动态库
//    hkxLab = new QLibrary("E:/WorkSpace/CODE/jhxd_qt/build-j_bxs_mgw_run-qt_5_6_2_msvc2013_32_bit-Debug/debug/HkxPs1204.dll",this);
    return true;
}

QVariant PcmPara::AnalyPara(char *data)
{
    //根据参数类型（不是控件类型）区分不同参数的解析
    qlonglong value = 0;
    int iDataLen = (m_paraStruct.eBit - m_paraStruct.sBit) / 8;
    //添加当m_paraStruct.eBit 不是8的倍数的情况
    QByteArray qByteTemp;
    qByteTemp.resize(iDataLen);
    if(m_paraStruct.isLittleEndian)
    {
        for(int j = 0;j < iDataLen;j ++)
        {
            qByteTemp.data()[j] = data[iDataLen - j - 1 + m_paraStruct.sByte];
        }
    }
    else
    {
        for(int j = 0;j < iDataLen;j ++)
        {
            qByteTemp.data()[j] = data[j + m_paraStruct.sByte];
        }
    }

    if(iDataLen < 8) //小于8认为是一般参数
    {
        memcpy(&value,qByteTemp.data(),iDataLen);
        if(m_symbolBit != -1)
        {
            qlonglong qlTemp = 1;
            qlTemp <<= m_symbolBit;
//            qlTemp =~qlTemp;
//            qDebug()<<QString::number(qlTemp,16);
            if((qlTemp & value) != 0)
            {
//                qDebug()<<QString::number(value,16);
                value = value & ~qlTemp;
                value -= qlTemp;
//                qDebug()<<QString::number(value,16);
            }
//            if(value )
        }
        //此时得到数字量value
        // 以下根据公式，将工程量转化位数字量.如果没有公式则输出value
        switch (m_formulaType) {
        case 0:
        {
            m_value = value * m_lineCoefficient + m_lineConstant;
//            qDebug()<<this->getParaName()<<m_lineCoefficient<<m_lineConstant<<value;
//            int iTemp = 0xfefe;
//            m_value = (float)iTemp * m_lineCoefficient + m_lineConstant;
//            qDebug()<<this->getParaName()<<m_lineCoefficient<<m_lineConstant<<iTemp<<(float)iTemp;
        }
            break;
        case 1:
        {
            for(int i = 0;i < m_intervalMin.count();i ++)
            {
                if(value > m_intervalMax[i])
                    continue;
                if(value < m_intervalMin[i])
                    continue;
                m_value = value * m_coefficient[i] + m_constant[i];
            }
        }
            break;
        case 2:
        {

        }
            break;
        case -1:     //公式错误或者没有公式，原数据输出
        {
            m_value = value;
        }
            break;
        default:
            break;
        }
    }
    else {//视频参数
        m_value = qByteTemp;
    }
//    <paraFormula>
//    formulaType="lineFormula"
//     <line_coefficient>2</line_coefficient>
//     <line_constant>1</line_constant>
//    </paraFormula>
//    QString strFormula = m_paraStruct.paraFormula.left(':');
//    switch (strFormula) {
//    case "tab_lineFormula":
//        break;
//    default:
//        break;
//    }
    m_dataUpdateTimes ++;
    if(m_dataUpdateTimes >= 65535)
        m_dataUpdateTimes = 0;
    return m_value;
}

bool PcmPara::IsNeedAnaly(qlonglong pageId, uint iFrameIndex)
{
//    qDebug()<<m_ctrlType<<Form_customView::controlType::view;
    if(m_ctrlType != Form_customView::controlType::curve
            && m_ctrlType != Form_customView::controlType::view)
    {
        if(pageId != m_pageId)
            return false;
    }
    if(m_paraStruct.isSyn)
    {
        if(iFrameIndex <= m_paraStruct.sSubFrame || iFrameIndex >= m_paraStruct.eSubFrame)
            return false;
    }
    else
    {
        if(iFrameIndex != m_paraStruct.subFrameIndex)
            return false;
    }
    return true;
}

bool PcmPara::IsNeedDisPlay(qlonglong pageId)
{
    if(m_ctrlType == Form_customView::controlType::curve)
        return true;
    if(pageId != this->m_pageId)
        return false;
    return true;
}

bool PcmPara::UpdateControl()
{
    //先判断数据是否第一次更新。再判断数据上次更新后是否有再次刷新数据， 如果时简单显示控件（lineEdit led）再判断控件这次显示的值是否和上次一样
    if(m_dataUpdateTimes == -1)
    {
        qDebug()<<"data did't Refresh " << getParaName();
        return false;
    }
    if(m_dataUpdateTimes == 0)
    {
//        qDebug()<<"data did't Refresh after last ctrl Refresh " << getParaName();
        return false;
    }
    switch (this->m_ctrlType)
    {
    case Form_customView::controlType::LineEdit:
    {
        QLineEdit *ctrl = static_cast<QLineEdit*> (m_controlHandle);
        QString strTemp = QString();
        for(int i = 0;i < m_paraDisplayList.count();i ++)
        {
            if(m_value.toDouble() > m_paraDisplayList.at(i).max)
                continue;
            if(m_value.toDouble() <= m_paraDisplayList.at(i).min)
                continue;
            strTemp = m_paraDisplayList.at(i).text;
            break;
        }
        if(strTemp.isEmpty())
        {
            if(m_value.type() == QMetaType::Double)
            {
                if(ctrl->text() != QString::number(m_value.toDouble(),'f',3))
                    ctrl->setText(QString::number(m_value.toDouble(),'f',3));
            }
        }
        else {
            ctrl->setText(strTemp);
        }
    }
        break;
    case Form_customView::controlType::led:
    {
        j_customControlLed *ctrl = static_cast<j_customControlLed*>(m_controlHandle);
        for(int i = 0;i < m_paraDisplayList.count();i ++)
        {
            if(m_value.toDouble() > m_paraDisplayList.at(i).max)
                continue;
            if(m_value.toDouble() <= m_paraDisplayList.at(i).min)
                continue;
            ctrl->setLedColor((j_customControlLed::ledColor)m_paraDisplayList.at(i).ledColor);
        }
    }
        break;
    case Form_customView::controlType::view:
    {
        //        QLabel *ctrl = static_cast<QLabel*> (m_controlHandle);
        //        HINSTANCE hDll;
        //        BOOL_HkxPlay_BufferData *PlayBD;
        //        if(!hkxLab->isLoaded())
        //        {
        //            if(hkxLab->load())
        //            {
        //                qDebug()<<"load ok";
        //            }else {
        //                qDebug()<<"load fail";
        //                qDebug()<<hkxLab->errorString();
        //            }
        //        }
        //        if(!hkxLab->isLoaded())
        //            return false;
        //        if(HkxPlay_BufferData == NULL)
        //            HkxPlay_BufferData =(BOOL_HkxPlay_BufferData*)hkxLab->resolve("HkxPlay_BufferData");
        //        qDebug()<<HkxPlay_BufferData;
        //        if(HkxPlay_BufferData == NULL)
        //            return false;
        //        PlayBD = HkxPlay_BufferData;
        //        int iDataLen = (m_paraStruct.eBit - m_paraStruct.sBit) / 8;
        //        QByteArray value;
        //        memcpy(value.data(),m_value.data(),iDataLen);
        //        PlayBD(value.data(),value.length(),(HWND)ctrl);
    }
        break;
    case Form_customView::controlType::curve:
    {
        Plot *ctrl = static_cast<Plot*>(m_controlHandle);
//        if(m_paraStruct.paraName == "副翼位移")
//            qDebug()<<m_paraStruct.paraName<<m_value.toDouble();
        ctrl->upDateValue(m_value.toDouble(),m_paraStruct.paraName);
    }
        break;
    default:
        break;
    }
    m_dataUpdateTimes = 0;
    return true;
}

Form_customView::controlType PcmPara::GetControlType()
{
    return m_ctrlType;
}

QString PcmPara::getParaName()
{
    return this->m_paraStruct.paraName;
}

bool PcmPara::AnayFormula()
{
    return false;
}



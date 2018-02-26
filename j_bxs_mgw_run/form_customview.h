#ifndef FORM_CUSTOMVIEW_H
#define FORM_CUSTOMVIEW_H

#include <QWidget>
#include <QtUiPlugin/QDesignerExportWidget>

namespace Ui {
class Form_customView;
}

class Form_customView : public QWidget
{
   Q_OBJECT
public:
    enum controlType{
        label,
        LineEdit,
        led,
        curve,
        view,
        other
    };
    struct controlStruct{
        qlonglong ctrlId;
        controlType ctrlType;
        QRect ctrlRect;
        QString att;
        QWidget *handle;
    };
    explicit Form_customView(QModelIndex *pageModel, QWidget *parent = 0);
    ~Form_customView();
    QList<controlStruct> m_ctrlList;
    qlonglong getPageId();
//private slots:
//    void on_pushButton_clicked();
private:
    Ui::Form_customView *ui;
    qlonglong m_pageid;
    void newLineEditCtrl(controlStruct &curCtrlStruct);
    void newLabelCtrl(controlStruct &curCtrlStruct);
    void newLedCtrl(controlStruct &curCtrlStruct);
    void newCurveCtrl(controlStruct &curCtrlStruct);
    QMap<QString,QString> m_releaseAtt(QString attStr);
    QRect m_defaultRect;
};

#endif // FORM_CUSTOMVIEW_H

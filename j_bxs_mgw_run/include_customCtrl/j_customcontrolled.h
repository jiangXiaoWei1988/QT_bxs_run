#ifndef J_CUSTOMCONTROLLED_H
#define J_CUSTOMCONTROLLED_H
#include "ui_ledControl.h"
#include <QWidget>
#include <QLabel>
#include <QLayout>
#include <QHBoxLayout>
//namespace Ui {
//class ledControlFrom;
//}
class j_customControlLed : public QWidget
{
    Q_OBJECT
public: enum ledColor{
        silver,
        blue,
        green,
        orange,
        red,
        yellow
    };
public:
    j_customControlLed(QWidget *parent = 0);
    void setText(QString text);
    void setLedColor(ledColor colour);
    ledColor getLedColor();
private:
    Ui::ledControlFrom ui;
    ledColor m_curColor;
//    QLabel *picture;
//    QLabel *title;
//    QHBoxLayout *layout;

};

#endif

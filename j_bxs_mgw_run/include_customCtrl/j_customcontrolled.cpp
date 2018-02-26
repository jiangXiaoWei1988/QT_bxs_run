#include "j_customcontrolled.h"
#include <QDebug>

j_customControlLed::j_customControlLed(QWidget *parent) :
    QWidget(parent)
{
    ui.setupUi(this);
    QPixmap pixmap;
    pixmap = QPixmap(":/Resources/Led/silver.png");
    int iScale = pixmap.width() > pixmap.height() ?  pixmap.width() : pixmap.height();
    pixmap.scaled(iScale,iScale,Qt::KeepAspectRatio);
//    pixmap.scaled(iScale,iScale);
    ui.label->setPixmap(pixmap);
//    ui.label_2->rect().setWidth(ui.label_2->text().length() * 8);
//    QLabel *picture = new QLabel(this);
//    QHBoxLayout *layout = new QHBoxLayout();
//    QPixmap pixmap;
//    pixmap = QPixmap(":/Led/blue.png");
//    picture->setPixmap(pixmap);
//    layout->addWidget(picture);
//    this->setLayout(layout);


//    picture = new QLabel(this);
//    title = new QLabel(this);
//    QHBoxLayout *layout = new QHBoxLayout();
//    QPixmap pixmap = QPixmap(":/Resources/Led/blue.png");
//    picture->setPixmap(pixmap);
//    layout->addWidget(picture);
//    layout->addWidget(title);
//    this->setLayout(layout);
}

void j_customControlLed::setText(QString text)
{
    ui.label_2->setText(text);
    ui.label_2->rect().setWidth(text.length() * 8);
//    title->setText(text);
}

void j_customControlLed::setLedColor(j_customControlLed::ledColor colour)
{
    QPixmap pixmap;   
    switch (colour) {
    case ledColor::red:
    {
        pixmap = QPixmap(":/Resources/Led/red.png");
        m_curColor = red;
    }
        break;
    case ledColor::blue:
    {
        pixmap = QPixmap(":/Resources/Led/blue.png");
        m_curColor = blue;
    }
        break;
    case ledColor::green:
    {
        pixmap = QPixmap(":/Resources/Led/green.png");
        m_curColor = green;
    }
        break;
    case ledColor::orange:
    {
        pixmap = QPixmap(":/Resources/Led/orange.png");
        m_curColor = orange;
    }
        break;
    case ledColor::silver:
    {
        pixmap = QPixmap(":/Resources/Led/silver.png");
        m_curColor = silver;
    }
        break;
    case ledColor::yellow:
    {
        pixmap = QPixmap(":/Resources/Led/yellow.png");
        m_curColor = yellow;
    }
        break;
    default:
        break;
    }
    int iScale = pixmap.width() > pixmap.height() ?  pixmap.width() : pixmap.height();
    pixmap.scaled(iScale,iScale);
    ui.label->setPixmap(pixmap);
}

j_customControlLed::ledColor j_customControlLed::getLedColor()
{
    return m_curColor;
}





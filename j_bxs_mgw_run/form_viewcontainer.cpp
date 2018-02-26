#include "form_viewcontainer.h"
#include "ui_form_viewcontainer.h"
#include "dommodel.h"
#include <QScrollArea>
#include <QDebug>
#include "form_customview.h"
#include "maindialog.h"
#include <QProxyStyle>
#include <QPainter>
class CustomTabStyle_1 : public QProxyStyle
{
public:
    QSize sizeFromContents(ContentsType type, const QStyleOption *option,
        const QSize &size, const QWidget *widget) const
    {
        QSize s = QProxyStyle::sizeFromContents(type, option, size, widget);
        if (type == QStyle::CT_TabBarTab) {
//            s.transpose();
            s.rwidth() *= 1.5; // 设置每个tabBar中item的大小
            s.rheight() *= 1.2;
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
//                    painter->setPen(0x89cfff);   //137 207 255
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

Form_viewContainer::Form_viewContainer(QModelIndex *model, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Form_viewContainer)
{
    ui->setupUi(this);
    ui->tabWidget->tabBar()->setStyle(new CustomTabStyle_1);
    ui->tabWidget->installEventFilter(this);
    MainDialog *mainDlgPos = (MainDialog*)parent;
    DomItem *item = static_cast<DomItem*>(model->internalPointer());
//    qDebug()<<item<<item->getName()<<item->childCount()<<model->child(0,0).isValid();
    for(int i = 0;i < item->childCount();i ++)
    {
        QScrollArea *scrollare = new QScrollArea(this);
        ui->tabWidget->addTab(scrollare,item->child(i)->getName());
        Form_customView *customView = new Form_customView(&model->child(i,0),this);
        mainDlgPos->customViewList.append(customView);
        scrollare->setWidget(customView);
        m_pageIdList.append(customView->getPageId());
    }
}

Form_viewContainer::~Form_viewContainer()
{
    delete ui;
}

void Form_viewContainer::on_tabWidget_currentChanged(int index)
{
    //向主窗体发送信号，当前页面id（pageId）改变
    if(index > m_pageIdList.count() || m_pageIdList.count() <= 0)
        return ;
    emit pageIdChange(m_pageIdList.at(index));
}

bool Form_viewContainer::eventFilter(QObject *watched, QEvent *event)
{
    if(watched == ui->tabWidget)
    {
        if(event->type() == QEvent::Show)
        {
           emit ui->tabWidget->currentChanged(ui->tabWidget->currentIndex());
        }
    }
    return QWidget::eventFilter(watched,event);
}

//MyTableWidget::MyTableWidget(QWidget *parent):QTableWidget(parent)
//{

//}

//MyTableWidget::~MyTableWidget()
//{

//}

//void MyTableWidget::focusInEvent(QFocusEvent *e)
//{
//    emit MyTableWidgeGetFocus();
//}

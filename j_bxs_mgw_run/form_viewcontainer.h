#ifndef FORM_VIEWCONTAINER_H
#define FORM_VIEWCONTAINER_H

#include <QWidget>
#include <QTableWidget>
namespace Ui {
class Form_viewContainer;
}

class Form_viewContainer : public QWidget
{
    Q_OBJECT
public:
    explicit Form_viewContainer(QModelIndex *model, QWidget *parent = 0);
    ~Form_viewContainer();
private slots:
    void on_tabWidget_currentChanged(int index);
    bool eventFilter(QObject *watched, QEvent *event);
signals:
    void pageIdChange(qlonglong curPageId);
private:
    Ui::Form_viewContainer *ui;
    QList<qlonglong> m_pageIdList;
};

//class MyTableWidget : QTabWidget
//{
//    Q_OBJECT
//public:
//    MyTableWidget(QWidget *parent = Q_NULLPTR);
//    ~MyTableWidget();
//protected:
//    virtual void focusInEvent(QFocusEvent *e);
//signals:
//    void MyTableWidgeGetFocus();
//};

#endif // FORM_VIEWCONTAINER_H

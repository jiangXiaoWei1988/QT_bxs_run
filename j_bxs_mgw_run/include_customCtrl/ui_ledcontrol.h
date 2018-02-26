/********************************************************************************
** Form generated from reading UI file 'ledcontrol.ui'
**
** Created by: Qt User Interface Compiler version 5.6.3
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_LEDCONTROL_H
#define UI_LEDCONTROL_H

#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QLabel>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_ledControlFrom
{
public:
    QHBoxLayout *horizontalLayout;
    QLabel *label;
    QLabel *label_2;

    void setupUi(QWidget *ledControlFrom)
    {
        if (ledControlFrom->objectName().isEmpty())
            ledControlFrom->setObjectName(QStringLiteral("ledControlFrom"));
        ledControlFrom->resize(150, 30);
        horizontalLayout = new QHBoxLayout(ledControlFrom);
        horizontalLayout->setSpacing(1);
        horizontalLayout->setObjectName(QStringLiteral("horizontalLayout"));
        horizontalLayout->setSizeConstraint(QLayout::SetDefaultConstraint);
        horizontalLayout->setContentsMargins(2, 2, 2, 2);
        label = new QLabel(ledControlFrom);
        label->setObjectName(QStringLiteral("label"));
        QSizePolicy sizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
        sizePolicy.setHorizontalStretch(1);
        sizePolicy.setVerticalStretch(1);
        sizePolicy.setHeightForWidth(label->sizePolicy().hasHeightForWidth());
        label->setSizePolicy(sizePolicy);
        label->setMinimumSize(QSize(24, 24));
        label->setMaximumSize(QSize(100, 100));
        label->setLayoutDirection(Qt::LeftToRight);
        label->setAutoFillBackground(false);
        label->setFrameShape(QFrame::NoFrame);
        label->setTextFormat(Qt::PlainText);
        label->setScaledContents(true);
        label->setAlignment(Qt::AlignCenter);
        label->setWordWrap(true);
        label->setTextInteractionFlags(Qt::LinksAccessibleByMouse);

        horizontalLayout->addWidget(label, 0, Qt::AlignLeft|Qt::AlignVCenter);

        label_2 = new QLabel(ledControlFrom);
        label_2->setObjectName(QStringLiteral("label_2"));
        QSizePolicy sizePolicy1(QSizePolicy::Preferred, QSizePolicy::Preferred);
        sizePolicy1.setHorizontalStretch(0);
        sizePolicy1.setVerticalStretch(0);
        sizePolicy1.setHeightForWidth(label_2->sizePolicy().hasHeightForWidth());
        label_2->setSizePolicy(sizePolicy1);
        label_2->setMinimumSize(QSize(40, 20));
        QFont font;
        font.setFamily(QStringLiteral("Agency FB"));
        font.setPointSize(11);
        label_2->setFont(font);
        label_2->setFrameShape(QFrame::NoFrame);
        label_2->setTextFormat(Qt::AutoText);
        label_2->setScaledContents(true);
        label_2->setAlignment(Qt::AlignLeading|Qt::AlignLeft|Qt::AlignVCenter);
        label_2->setWordWrap(true);

        horizontalLayout->addWidget(label_2, 0, Qt::AlignVCenter);

        horizontalLayout->setStretch(0, 30);
        horizontalLayout->setStretch(1, 120);
#ifndef QT_NO_SHORTCUT
#endif // QT_NO_SHORTCUT

        retranslateUi(ledControlFrom);

        QMetaObject::connectSlotsByName(ledControlFrom);
    } // setupUi

    void retranslateUi(QWidget *ledControlFrom)
    {
        ledControlFrom->setWindowTitle(QApplication::translate("ledControlFrom", "Form", Q_NULLPTR));
        label->setText(QString());
        label_2->setText(QApplication::translate("ledControlFrom", "TextLabel", Q_NULLPTR));
    } // retranslateUi

};

namespace Ui {
    class ledControlFrom: public Ui_ledControlFrom {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_LEDCONTROL_H

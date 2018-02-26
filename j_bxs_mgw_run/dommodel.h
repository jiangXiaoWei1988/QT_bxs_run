#ifndef DOMMODEL_H
#define DOMMODEL_H
#include <QDomNode>
#include <QHash>
#include <QVariant>
#include <QAbstractItemModel>
#include <QDomDocument>
#include <QModelIndex>
#include <QListWidget>

class DomItem
{
public:
    enum NodeTpye{
        root,
        project,
        plane,
        frameConfig,
        jsjConfig,
        paraConfig,
        interfaceConfig,
        paraClass,
        interface_text,
        interface_state,
        interface_curve,
        interface_video,
        interface_text_child,
        interface_state_child,
        interface_curve_child,
        interface_video_child,
        other
    };
    DomItem(QDomNode &node, int row, DomItem *parent = 0);
    ~DomItem();
    DomItem *child(int i);
    DomItem *parent();
    QDomNode node() const;
    QString getAtt(const QString attName);
    int row();
    int childCount();
    NodeTpye getType();
    QString getName();
    qlonglong getId();
    bool insertChild(int position, DomItem *newNodePos);
    bool removeChild(DomItem *removeNodePos);
    bool change(const QVariant &value);
private:
    QDomNode m_domNode;
    QList<DomItem*> m_childItemList;
    DomItem *m_parentItemPos;
    int m_rowNumber;
};
class DomModel : public QAbstractItemModel
{
    Q_OBJECT
public:
    enum Atmosphere{
        nomal,
        paraChoise,
        taskChoise,
        other
    };
    static qlonglong getRandomId();
    DomModel(QDomDocument document,QDomNode &node,Atmosphere m_atmosphere = nomal ,QObject *parent = 0);
    DomModel(QDomDocument document,Atmosphere m_atmosphere = nomal ,QObject *parent = 0);
    ~DomModel();

    QVariant data(const QModelIndex &index, int role) const override;
    Qt::ItemFlags flags(const QModelIndex &index) const override;
    QModelIndex index(int row, int column,
                      const QModelIndex &parent = QModelIndex()) const override;
    QModelIndex parent(const QModelIndex &child) const override;
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    bool insertRows(int row, int count, const QModelIndex &parent) override;
    bool removeRows(int row, int count, const QModelIndex &parent) override;
    bool setData(const QModelIndex &index, const QVariant &value, int role) override;
private:
    QDomDocument m_domDocument;
    DomItem *m_rootItemPos;
    Atmosphere m_atmosphere;
};

#endif // DOMMODEL_H

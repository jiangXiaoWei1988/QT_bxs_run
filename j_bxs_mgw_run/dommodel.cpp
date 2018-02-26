#include "dommodel.h"
#include <QThread>
#include <QDateTime>
#include <QDebug>

//static QRect defultRect = QRect::QRect(0,0,1600,1020);
static QRect defultRect;
DomItem::DomItem(QDomNode &node, int row, DomItem *parent)
{
    m_domNode = node;
    m_rowNumber = row;
    m_parentItemPos = parent;    
    defultRect.setX(0);
    defultRect.setY(0);
    defultRect.setWidth(1600);
    defultRect.setHeight(1020);
}

DomItem::~DomItem()
{
    m_childItemList.clear();
}

DomItem *DomItem::child(int i)
{
    foreach (DomItem *curItem, m_childItemList) {
        if(curItem->m_rowNumber == i)
            return curItem;
    }
    if(i >= 0 && i < m_domNode.childNodes().count())
    {
        QDomNode childNode = m_domNode.childNodes().item(i);
        DomItem * childItem = new DomItem(childNode,i,this);
        m_childItemList.append(childItem);
        return childItem;
    }
    return 0;
}

DomItem *DomItem::parent()
{
    return m_parentItemPos;
}

QDomNode DomItem::node() const
{
    return m_domNode;
}

QString DomItem::getAtt(const QString attName)
{
    QString result = QString();
    if(this->node().isElement())
    {
        QDomElement element = this->node().toElement();
        if(element.hasAttribute(attName))
        {
            result = element.attribute(attName);
        }
    }
    return result;
}

int DomItem::row()
{
    return m_rowNumber;
}

int DomItem::childCount()
{
//    return m_childItemList.length();
    return m_domNode.childNodes().count();
}

DomItem::NodeTpye DomItem::getType()
{
    if(this->m_domNode.isElement())
    {
        QDomElement qelement = this->m_domNode.toElement();
        int iTemp = qelement.attribute("type").toInt();
        if(iTemp >= NodeTpye::root && iTemp <= NodeTpye::other)
        {
            return NodeTpye(iTemp);
        }
    }
    return NodeTpye::other;
}

QString DomItem::getName()
{
    if(this->m_domNode.isElement())
    {
        QDomElement qelement = this->m_domNode.toElement();
        return qelement.attribute("name");
    }
    return "error";
}

qlonglong DomItem::getId()
{
    if(this->m_domNode.isElement())
    {
        QDomElement qelement = this->m_domNode.toElement();
        return qelement.attribute("id").toLongLong();
    }
    else
        return -1;
}

bool DomItem::insertChild(int position, DomItem *newNodePos)
{
    if(position < 0)
        return false;
    foreach (DomItem *curItem,m_childItemList) {
        if(curItem->m_rowNumber >= position)
            curItem->m_rowNumber ++;
    }
    m_childItemList.insert(position,newNodePos);
    DomItem *beforeItem = this->child(position - 1);
    if(beforeItem == 0 && position == 0)
    {
        if(this->m_domNode.appendChild(newNodePos->m_domNode).isNull())
            return false;
    }else if(beforeItem != 0)
    {
        if(this->m_domNode.insertAfter(newNodePos->m_domNode,beforeItem->m_domNode).isNull())
            return false;
    }

    return true;
}

bool DomItem::removeChild(DomItem *removeNodePos)
{
    if(!removeNodePos)
        return false;
    foreach (DomItem *curItem,m_childItemList) {
        if(curItem->m_rowNumber >= removeNodePos->m_rowNumber)
            curItem->m_rowNumber --;
    }
    m_domNode.removeChild(removeNodePos->m_domNode);
    m_childItemList.removeOne(removeNodePos);
    return true;
}

bool DomItem::change(const QVariant &value)
{
    if(value.toString().isEmpty())
        return false;
    if(this->m_domNode.isElement())
    {
        QDomElement qelement = this->m_domNode.toElement();
        qelement.setAttribute("name",value.toString());
        return true;
    }
    return false;
}

//static qlonglong DomModel::getRandomId()
//{
//    QThread::msleep(1);//避免id 重复
//    qlonglong id = 0;
//    QDateTime curTime = QDateTime::currentDateTime();
//    id = curTime.toMSecsSinceEpoch();
//    return id;
//}

qlonglong DomModel::getRandomId()
{
    QThread::msleep(1);//避免id 重复
    qlonglong id = 0;
    QDateTime curTime = QDateTime::currentDateTime();
    id = curTime.toMSecsSinceEpoch();
    return id;
}

DomModel::DomModel(QDomDocument document, QDomNode &node, Atmosphere atmosphere, QObject *parent)
    : QAbstractItemModel(parent), m_domDocument(document)
{
    if(!node.isNull())
        m_rootItemPos = new DomItem(node,0);
    else
        m_rootItemPos = new DomItem(m_domDocument, 0);
    this->m_atmosphere = atmosphere;
}
DomModel::DomModel(QDomDocument document, Atmosphere atmosphere, QObject *parent)
    : QAbstractItemModel(parent), m_domDocument(document)
{
    m_rootItemPos = new DomItem(m_domDocument, 0);
    this->m_atmosphere = atmosphere;
}

DomModel::~DomModel()
{
    delete m_rootItemPos;
}

QVariant DomModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();
    DomItem *item = static_cast<DomItem*>(index.internalPointer());
    if (role == Qt::DisplayRole)
    {
        QString str = item->getName();
        if(!str.isNull())
        {
            return str;
        }
        return QVariant();
    }else if (role == Qt::DecorationRole) {
        DomItem::NodeTpye type = item->getType();
        switch (type) {
        case DomItem::NodeTpye::root:
        {
            //project
            return QIcon(":/Resources/icon/root.ico");
            break;
        }
        case DomItem::NodeTpye::project:
        {
            //project
            return QIcon(":/Resources/icon/projectConfig.ico");
            break;
        }
        case DomItem::NodeTpye::plane:
        {
            //plane
            return QIcon(":/Resources/icon/plane.ico");
            break;
        }
        case DomItem::NodeTpye::frameConfig:
        {
            //frameConfig
            return QIcon(":/Resources/icon/frameConfig.ico");
            break;
        }
        case DomItem::NodeTpye::jsjConfig:
        {
            //jsjConfig
            return QIcon(":/Resources/icon/jsjConfig.ico");
            break;
        }
        case DomItem::NodeTpye::paraConfig:
        {
            //paraConfig
            return QIcon(":/Resources/icon/paraConfig.ico");
            break;
        }
        case DomItem::NodeTpye::paraClass:
        {
            //paraClass
            return QIcon(":/Resources/icon/paraClass.ico");
            break;
        }
        case DomItem::NodeTpye::interfaceConfig:
        {
            //interfaceConfig
            return QIcon(":/Resources/icon/interfaceConfig.ico");
            break;
        }
        case DomItem::NodeTpye::interface_text:
        {
            //interface_text
            return QIcon(":/Resources/icon/text.ico");
            break;
        }
        case DomItem::NodeTpye::interface_state:
        {
            //interface_state
            return QIcon(":/Resources/icon/state.ico");
            break;
        }
        case DomItem::NodeTpye::interface_curve:
        {
            //interface_curve
            return QIcon(":/Resources/icon/curve.ico");
            break;
        }
        case DomItem::NodeTpye::interface_video:
        {
            //interface_video
            return QIcon(":/Resources/icon/video.ico");
            break;
        }
        case DomItem::NodeTpye::interface_video_child:
        {
            //interface_video
            return QIcon(":/Resources/icon/interFace_child.ico");
            break;
        }
        case DomItem::NodeTpye::interface_curve_child:
        {
            //interface_video
            return QIcon(":/Resources/icon/interFace_child.ico");
            break;
        }
        case DomItem::NodeTpye::interface_state_child:
        {
            //interface_video
            return QIcon(":/Resources/icon/interFace_child.ico");
            break;
        }
        case DomItem::NodeTpye::interface_text_child:
        {
            //interface_video
            return QIcon(":/Resources/icon/interFace_child.ico");
            break;
        }
        default:
            return QVariant();
            break;
        }
        return QVariant();
    }else if(role == Qt::UserRole){
        return item->getId();
    }
    else {
        return QVariant();
    }
}

Qt::ItemFlags DomModel::flags(const QModelIndex &index) const
{
    if (!index.isValid())
        return 0;
    Qt::ItemFlags flag = QAbstractItemModel::flags(index);
    DomItem *curItem = static_cast<DomItem*>(index.internalPointer());
    DomItem::NodeTpye parentType = curItem->getType();
    switch (parentType) {
    case DomItem::NodeTpye::root:
    case DomItem::NodeTpye::frameConfig:
    case DomItem::NodeTpye::jsjConfig:
    case DomItem::NodeTpye::paraConfig:
    case DomItem::NodeTpye::interfaceConfig:
    case DomItem::NodeTpye::interface_text:
    case DomItem::NodeTpye::interface_state:
    case DomItem::NodeTpye::interface_curve:
    case DomItem::NodeTpye::interface_video:
    {
        // can not change
        flag |= Qt::ItemFlag::ItemIsSelectable | Qt::ItemFlag::ItemIsUserCheckable | Qt::ItemFlag::ItemIsDragEnabled | Qt::ItemFlag::ItemIsEnabled |  Qt::ItemFlag::ItemIsDropEnabled;
        break;
    }
    default:
         flag |= Qt::ItemFlag::ItemIsSelectable | Qt::ItemFlag::ItemIsUserCheckable | Qt::ItemFlag::ItemIsDragEnabled | Qt::ItemFlag::ItemIsEnabled |  Qt::ItemFlag::ItemIsDropEnabled | Qt::ItemFlag::ItemIsEditable;
        break;
    }
    if(m_atmosphere != Atmosphere::nomal)
        flag &= ~Qt::ItemFlag::ItemIsEditable;
    if(m_atmosphere == Atmosphere::taskChoise)
        if(parentType != DomItem::NodeTpye::plane)
        {
            flag &= ~Qt::ItemFlag::ItemIsSelectable;
            flag &= ~Qt::ItemFlag::ItemIsUserCheckable;
            flag &= ~Qt::ItemFlag::ItemIsEnabled;
        }

    return flag;
}

QModelIndex DomModel::index(int row, int column, const QModelIndex &parent) const
{
    if (!hasIndex(row, column, parent))
        return QModelIndex();
    DomItem *parentItem;
    if (!parent.isValid())
        parentItem = m_rootItemPos;
    else
        parentItem = static_cast<DomItem*>(parent.internalPointer());
    DomItem *childItem = parentItem->child(row);
    if (childItem)
    {
        QDomNode node = childItem->node();
        if(node.nodeType() == QDomNode::ProcessingInstructionNode)
            return QModelIndex();
        if(node.nodeType() == QDomNode::AttributeNode)
            return QModelIndex();
        return createIndex(row, column, childItem);
    }
    else
        return QModelIndex();
}

QModelIndex DomModel::parent(const QModelIndex &child) const
{
    if (!child.isValid())
        return QModelIndex();
    DomItem *childItem = static_cast<DomItem*>(child.internalPointer());
    DomItem *parentItem = childItem->parent();
    if (!parentItem || parentItem == m_rootItemPos)
        return QModelIndex();
    return createIndex(parentItem->row(), 0, parentItem);
}

int DomModel::rowCount(const QModelIndex &parent) const
{
    DomItem *parentItem;
    if (!parent.isValid())
        parentItem = m_rootItemPos;
    else
        parentItem = static_cast<DomItem*>(parent.internalPointer());
    return parentItem->node().childNodes().count();
}

int DomModel::columnCount(const QModelIndex &parent) const
{
    return 1;
}

bool DomModel::insertRows(int row, int count, const QModelIndex &parent)
{
    DomItem *parentItem = static_cast<DomItem*>(parent.internalPointer());
    if(!parentItem)
    {
        return false;
    }
    bool result = false;
    //根据parentItem的类型判断是否可以添加子类节点。以及子类节点的类型
    beginInsertColumns(parent,row,row + count - 1);
    for(int i = 0;i < count ;i ++)
    {
        //judge parent type
        DomItem::NodeTpye parentType = parentItem->getType();
        switch (parentType) {
        case DomItem::NodeTpye::root:
        {
            //add a new project
            QDomElement newNode = m_domDocument.createElement("project");
            qint64 id = getRandomId();
            newNode.setAttribute("id",id);
            newNode.setAttribute("name",tr("new project"));
            newNode.setAttribute("type",DomItem::NodeTpye::project);
            newNode.setAttribute("describe",tr("input describe"));
            DomItem *newItem = new DomItem(newNode,row,parentItem);
            result = parentItem->insertChild(row,newItem);
            break;
        }
        case DomItem::NodeTpye::project:
        {
            //add a new plane
            QDomElement newNode_plane = m_domDocument.createElement("plane");
            qint64 id = getRandomId();
            newNode_plane.setAttribute("id",id);
            newNode_plane.setAttribute("name",tr("new plane"));
            newNode_plane.setAttribute("type",DomItem::NodeTpye::plane);
            DomItem *newItem_plane = new DomItem(newNode_plane,row,parentItem);
            result = parentItem->insertChild(row,newItem_plane);
            //add a new jsjConfig
            QDomElement newNode_jsjConfig = m_domDocument.createElement("jsjConfig");
            id = getRandomId();
            newNode_jsjConfig.setAttribute("id",id);
            newNode_jsjConfig.setAttribute("name",tr("jsjConfig"));
            newNode_jsjConfig.setAttribute("type",DomItem::NodeTpye::jsjConfig);
            newNode_jsjConfig.setAttribute("frequency",tr("input frequency"));
            newNode_jsjConfig.setAttribute("rate",tr("input rate"));
            DomItem *newItem_jsjConfig = new DomItem(newNode_jsjConfig,newItem_plane->childCount(),newItem_plane);
            result = newItem_plane->insertChild(newItem_plane->childCount(),newItem_jsjConfig);
            //add a new frameConfig
            QDomElement newNode_frameConfig = m_domDocument.createElement("frameConfig");
            id = getRandomId();
            newNode_frameConfig.setAttribute("id",id);
            newNode_frameConfig.setAttribute("name",tr("frameConfig"));
            newNode_frameConfig.setAttribute("type",DomItem::NodeTpye::frameConfig);
            newNode_frameConfig.setAttribute("pcmWordLen",tr("input pcmWordLen"));
            newNode_frameConfig.setAttribute("frameLen",tr("input frameLen"));
            newNode_frameConfig.setAttribute("subFrameLen",tr("input subFrameLen"));
            newNode_frameConfig.setAttribute("synWord",tr("input synWord"));
            newNode_frameConfig.setAttribute("synWordLen",tr("input synWordLen"));
            newNode_frameConfig.setAttribute("synWordStation",tr("input synWordStation"));
            newNode_frameConfig.setAttribute("frameIDStattion",tr("input frameIDStattion"));
            newNode_frameConfig.setAttribute("frameIDLen",tr("input frameIDLen"));
            newNode_frameConfig.setAttribute("FrameIDWay",tr("input FrameIDWay"));

            DomItem *newItem_frameConfig = new DomItem(newNode_frameConfig,newItem_plane->childCount(),newItem_plane);
            result = newItem_plane->insertChild(newItem_plane->childCount(),newItem_frameConfig);
            //add a new paraConfig
            QDomElement newNode_paraConfig = m_domDocument.createElement("paraConfig");
            id = getRandomId();
            newNode_paraConfig.setAttribute("id",id);
            newNode_paraConfig.setAttribute("name",tr("paraConfig"));
            newNode_paraConfig.setAttribute("type",DomItem::NodeTpye::paraConfig);
            DomItem *newItem_paraConfig = new DomItem(newNode_paraConfig,newItem_plane->childCount(),newItem_plane);
            result = newItem_plane->insertChild(newItem_plane->childCount(),newItem_paraConfig);

            //add a new interfaceConfig
            QDomElement newNode_interfaceConfig = m_domDocument.createElement("interfaceConfig");
            id = getRandomId();
            newNode_interfaceConfig.setAttribute("id",id);
            newNode_interfaceConfig.setAttribute("name",tr("interfaceConfig"));
            newNode_interfaceConfig.setAttribute("type",DomItem::NodeTpye::interfaceConfig);
            DomItem *newItem_interfaceConfig = new DomItem(newNode_interfaceConfig,newItem_plane->childCount(),newItem_plane);
            result = newItem_plane->insertChild(newItem_plane->childCount(),newItem_interfaceConfig);

            //add a new interface_text
            QDomElement newNode_interface_text_child = m_domDocument.createElement("interface_text");
            id = getRandomId();
            newNode_interface_text_child.setAttribute("id",id);
            newNode_interface_text_child.setAttribute("name",tr("interface_text"));
            newNode_interface_text_child.setAttribute("type",DomItem::NodeTpye::interface_text);
            DomItem *newItem_interface_text_child = new DomItem(newNode_interface_text_child,newItem_interfaceConfig->childCount(),newItem_interfaceConfig);
            result = newItem_interfaceConfig->insertChild(newItem_interfaceConfig->childCount(),newItem_interface_text_child);
            //add a new interface_state
            QDomElement newNode_interface_state_child = m_domDocument.createElement("interface_state");
            id = getRandomId();
            newNode_interface_state_child.setAttribute("id",id);
            newNode_interface_state_child.setAttribute("name",tr("interface_state"));
            newNode_interface_state_child.setAttribute("type",DomItem::NodeTpye::interface_state);
            DomItem *newItem_interface_state_child = new DomItem(newNode_interface_state_child,newItem_interfaceConfig->childCount(),newItem_interfaceConfig);
            result = newItem_interfaceConfig->insertChild(newItem_interfaceConfig->childCount(),newItem_interface_state_child);
            //add a new interface_curve
            QDomElement newNode_interface_curve_child = m_domDocument.createElement("interface_curve");
            id = getRandomId();
            newNode_interface_curve_child.setAttribute("id",id);
            newNode_interface_curve_child.setAttribute("name",tr("interface_curve"));
            newNode_interface_curve_child.setAttribute("type",DomItem::NodeTpye::interface_curve);
            DomItem *newItem_interface_curve_child = new DomItem(newNode_interface_curve_child,newItem_interfaceConfig->childCount(),newItem_interfaceConfig);
            result = newItem_interfaceConfig->insertChild(newItem_interfaceConfig->childCount(),newItem_interface_curve_child);
            //add a new interface_video
            QDomElement newNode_interface_video_child = m_domDocument.createElement("interface_video");
            id = getRandomId();
            newNode_interface_video_child.setAttribute("id",id);
            newNode_interface_video_child.setAttribute("name",tr("interface_video"));
            newNode_interface_video_child.setAttribute("type",DomItem::NodeTpye::interface_video);
            DomItem *newItem_interface_video_child = new DomItem(newNode_interface_video_child,newItem_interfaceConfig->childCount(),newItem_interfaceConfig);
            result = newItem_interfaceConfig->insertChild(newItem_interfaceConfig->childCount(),newItem_interface_video_child);
            break;
        }
        case DomItem::NodeTpye::plane:
        {
            //do nothing
            break;
        }
        case DomItem::NodeTpye::frameConfig:
        {
            //do nothing
            break;
        }
        case DomItem::NodeTpye::jsjConfig:
        {
            //do nothing
            break;
        }
        case DomItem::NodeTpye::paraConfig:
        {
            //add a new paraClass
            QDomElement newNode = m_domDocument.createElement("new_paraClass");
            qint64 id = getRandomId();
            newNode.setAttribute("id",id);
            newNode.setAttribute("name",tr("new new_paraClass"));
            newNode.setAttribute("type",DomItem::NodeTpye::paraClass);
            DomItem *newItem = new DomItem(newNode,row,parentItem);
            result = parentItem->insertChild(row,newItem);
            break;
        }
        case DomItem::NodeTpye::paraClass:
        {
            //add a new paraClass
            QDomElement newNode = m_domDocument.createElement("new_paraClass");
            qint64 id = getRandomId();
            newNode.setAttribute("id",id);
            newNode.setAttribute("name",tr("new new_paraClass"));
            newNode.setAttribute("type",DomItem::NodeTpye::paraClass);
            DomItem *newItem = new DomItem(newNode,row,parentItem);
            result = parentItem->insertChild(row,newItem);
            break;
        }
        case DomItem::NodeTpye::interfaceConfig:
        {
            //do nothing
            break;
        }
        case DomItem::NodeTpye::interface_text:
        {
            //add a new interface_text_child
            QDomElement newNode = m_domDocument.createElement("new_interface_text");
            qint64 id = getRandomId();
            newNode.setAttribute("id",id);
            newNode.setAttribute("name",tr("new new_interface_text"));
            newNode.setAttribute("type",DomItem::NodeTpye::interface_text_child);
            for(int i = 0;i < 4;i ++)
            {
                newNode.setAttribute(QString("gropName%1").arg(i),"");
            }
            newNode.setAttribute("rect_width",defultRect.width());
            newNode.setAttribute("rect_height",defultRect.height());
            DomItem *newItem = new DomItem(newNode,row,parentItem);
            result = parentItem->insertChild(row,newItem);
            break;
        }
        case DomItem::NodeTpye::interface_state:
        {
            //add a new interface_state_child
            QDomElement newNode = m_domDocument.createElement("new_interface_state");
            qint64 id = getRandomId();
            newNode.setAttribute("id",id);
            newNode.setAttribute("name",tr("new new_interface_state"));
            newNode.setAttribute("type",DomItem::NodeTpye::interface_state_child);
            newNode.setAttribute("rect_width",defultRect.width());
            newNode.setAttribute("rect_height",defultRect.height());
            DomItem *newItem = new DomItem(newNode,row,parentItem);
            result = parentItem->insertChild(row,newItem);
            break;
        }
        case DomItem::NodeTpye::interface_curve:
        {
            //add a new interface_curve_child
            QDomElement newNode = m_domDocument.createElement("new_interface_curve");
            qint64 id = getRandomId();
            newNode.setAttribute("id",id);
            newNode.setAttribute("name",tr("new new_interface_curve"));
            newNode.setAttribute("type",DomItem::NodeTpye::interface_curve_child);
            newNode.setAttribute("count",1);
            for(int i = 0;i < 4;i ++)
            {
                newNode.setAttribute(QString("x%1").arg(i),"");
                newNode.setAttribute(QString("y%1").arg(i),"");
            }
            newNode.setAttribute("rect_width",defultRect.width());
            newNode.setAttribute("rect_height",defultRect.height());
            DomItem *newItem = new DomItem(newNode,row,parentItem);
            result = parentItem->insertChild(row,newItem);
            break;
        }
        case DomItem::NodeTpye::interface_video:
        {
            //add a new interface_video_child
            QDomElement newNode = m_domDocument.createElement("new_interface_video");
            qint64 id = getRandomId();
            newNode.setAttribute("id",id);
            newNode.setAttribute("name",tr("new new_interface_video"));
            newNode.setAttribute("type",DomItem::NodeTpye::interface_video_child);
            newNode.setAttribute("count",2);
            for(int i = 0;i < 9;i ++)
            {
                newNode.setAttribute(QString("polling%1").arg(i),NULL);
            }
            newNode.setAttribute("rect_width",defultRect.width());
            newNode.setAttribute("rect_height",defultRect.height());
            DomItem *newItem = new DomItem(newNode,row,parentItem);
            result = parentItem->insertChild(row,newItem);
            break;
        }
        case DomItem::NodeTpye::interface_video_child:
        case DomItem::NodeTpye::interface_curve_child:
        case DomItem::NodeTpye::interface_state_child:
        case DomItem::NodeTpye::interface_text_child:
        {
            //do nothing
            break;
        }
        default:
            break;
        }
        //project

//        QDomElement newNode = domDocument.createElement("root");
//        newNode.setAttribute("type",DomItem::NodeTpye::root);
    }
    endInsertRows();
    return result;
}

bool DomModel::removeRows(int row, int count, const QModelIndex &parent)
{
    DomItem *parentItem = static_cast<DomItem*>(parent.internalPointer());
    bool success = true;
    beginRemoveRows(parent, row, row + count - 1);
    for(int i = 0;i < count;i ++)
    {
        if(!parentItem)
        {
            endRemoveRows();
            return false;
        }
        //judge parent type
        DomItem::NodeTpye parentType = parentItem->child(row)->getType();
        switch (parentType) {
        case DomItem::NodeTpye::root:
        case DomItem::NodeTpye::frameConfig:
        case DomItem::NodeTpye::jsjConfig:
        case DomItem::NodeTpye::paraConfig:
        case DomItem::NodeTpye::interfaceConfig:
        case DomItem::NodeTpye::interface_text:
        case DomItem::NodeTpye::interface_state:
        case DomItem::NodeTpye::interface_curve:
        case DomItem::NodeTpye::interface_video:
        {
            // can not remove
            continue;
            break;
        }
        default:
            break;
        }
//        beginInsertColumns(parent,row,row + count - 1);
        DomItem *removeNode = parentItem->child(row);
        success = parentItem->removeChild(removeNode);
    }
    endRemoveRows();
    return success;
}

bool DomModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if(index.isValid() && role == Qt::EditRole)
    {
        if(value.isNull())
            return false;
        DomItem *curItem = static_cast<DomItem*>(index.internalPointer());
        curItem->change(value);
        emit dataChanged(index,index);
        return true;
    }
    if(index.isValid() && role == Qt::UserRole)
    {
        //这时 value是QModelIndex 就修改当前节点的属性
    }
    return setData(index,value,role);
}

#include "j_customcontrolledplugin.h"
#include "j_customcontrolviewplugin.h"
#include "j_custom_control.h"

J_CUSTOM_CONTROL::J_CUSTOM_CONTROL(QObject *parent)
    : QObject(parent)
{
    m_widgets.append(new j_customControlLedPlugin(this));
    m_widgets.append(new j_customControlViewPlugin(this));

}

QList<QDesignerCustomWidgetInterface*> J_CUSTOM_CONTROL::customWidgets() const
{
    return m_widgets;
}

#if QT_VERSION < 0x050000
Q_EXPORT_PLUGIN2(j_custom_controlplugin, J_CUSTOM_CONTROL)
#endif // QT_VERSION < 0x050000

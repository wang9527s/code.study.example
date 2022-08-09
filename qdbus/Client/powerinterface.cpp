#include "powerinterface.h"

Q_DECLARE_METATYPE(BatteryPercentageMap)


PowerInterface::PowerInterface(QObject *parent)
    :    QDBusAbstractInterface("com.deepin.daemon.Power","/com/deepin/daemon/Power"
                                ,"com.deepin.daemon.Power"
                                ,QDBusConnection::sessionBus(),parent)
{
    qRegisterMetaType<BatteryPercentageMap>("BatteryPercentageMap");
    qDBusRegisterMetaType<BatteryPercentageMap>();

    QDBusConnection::sessionBus().connect(this->service(), this->path(), "org.freedesktop.DBus.Properties",
                                          "PropertiesChanged","sa{sv}as", this, SLOT(__propertyChanged__(QDBusMessage)));
}

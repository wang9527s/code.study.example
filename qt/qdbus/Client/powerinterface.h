#ifndef POWERINTERFACE_H
#define POWERINTERFACE_H

#include <QObject>
#include <QMap>

#include <QtDBus/QtDBus>
#include <QtDBus/QDBusInterface>
#include <QtDBus/QDBusMetaType>

typedef QMap<QString, double> BatteryPercentageMap;
class PowerInterface : public QDBusAbstractInterface
{
    Q_OBJECT

    Q_SLOT void __propertyChanged__(const QDBusMessage &msg)
    {
        QList<QVariant> arguments = msg.arguments();
        if (3 != arguments.count())
            return;
        QString interfaceName = msg.arguments().at(0).toString();
        if (interfaceName != "com.deepin.daemon.Power")
            return;
        QVariantMap changedProps = qdbus_cast<QVariantMap>(arguments.at(1).value<QDBusArgument>());
        foreach (const QString &prop, changedProps.keys()) {
            const QMetaObject *self = metaObject();
            for (int i = self->propertyOffset(); i < self->propertyCount(); ++i) {
                QMetaProperty p = self->property(i);
                if (p.name() == prop) {
                    Q_EMIT p.notifySignal().invoke(this);
                }
            }
        }
    }

public:
    PowerInterface(QObject *parent = 0);

    Q_PROPERTY(bool OnBattery READ onBattery NOTIFY OnBatteryChanged)
    inline bool onBattery() const
    {
        return qvariant_cast<bool>(property("OnBattery"));
    }

    Q_PROPERTY(BatteryPercentageMap BatteryPercentage READ batteryPercentage NOTIFY
                   BatteryPercentageChanged)
    inline BatteryPercentageMap batteryPercentage() const
    {
        return qvariant_cast<BatteryPercentageMap>(property("BatteryPercentage"));
    }

Q_SIGNALS: // SIGNALS
    void OnBatteryChanged();
    void BatteryPercentageChanged();
};

#endif // POWERINTERFACE_H

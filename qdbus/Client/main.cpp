#include "Client.h"
#include <QCoreApplication>
#include <QDBusConnection>
#include <QDBusError>
#include <QDebug>
#include <QTimer>

#include "powerinterface.h"

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    Client inter;

    PowerInterface * powInter = new PowerInterface(nullptr);
    auto checkLowPower=[=]{
        bool onBattery=powInter->onBattery();
        BatteryPercentageMap data=powInter->batteryPercentage();
        int batteryPercentage = uint(qMin(100.0, qMax(0.0, data.value("Display"))));
        bool lowPower= batteryPercentage<60 && onBattery;
        qDebug()<<"update power: onBattery "<<onBattery<<", batteryPercentage "
              <<batteryPercentage<< "isLowPower"<<lowPower;
    };

    QObject::connect(powInter,&PowerInterface::BatteryPercentageChanged,[=]{
        qDebug()<<"update: BatteryPercentageChanged";
        checkLowPower();
    });
    QObject::connect(powInter,&PowerInterface::OnBatteryChanged,[=]{
        qDebug()<<"update: OnBatteryChanged";
        checkLowPower();
    });

    return a.exec();
}

#include "NetworkInformationWidget.h"
#include <QGridLayout>
#include <QLabel>
#include <QHostAddress>
#include <QNetworkInterface>
#include <QSysInfo>

NetworkInformationWidget::NetworkInformationWidget(QWidget *parent) : QWidget(parent)
{
    QString computerInfo;
    computerInfo.append(tr("Network:\n"));
    foreach (const QHostAddress &address, QNetworkInterface::allAddresses())
    {
        if (address.protocol() == QAbstractSocket::IPv4Protocol)
            computerInfo.append(QString("\t %1 \n").arg(address.toString()));
    }

    computerInfo.append(tr("Kernel Type: %1 \n").arg(QSysInfo::kernelType()));
    computerInfo.append(tr("Kernel Varsion: %1 \n").arg(QSysInfo::kernelVersion()));
    //computerInfo.append(tr("Machine Host Name: %1 \n").arg(QSysInfo::machineHostName()));
    computerInfo.append(tr("Current CPU Architecture: %1 \n").arg(QSysInfo::currentCpuArchitecture()));

    auto lblComputerInfoText = new QLabel(computerInfo, this);

    auto mainGrid = new QGridLayout();
    this->setLayout(mainGrid);


    int rowIndex = 0;
    mainGrid->addWidget(lblComputerInfoText,                         rowIndex, 0, 1, 1);
}

#ifndef NETWORKADDRESSEDITOR_H
#define NETWORKADDRESSEDITOR_H

#include <QWidget>
#include <PreferenceAssociation.h>
#include "ApplicationSettings.h"

class NetworkAddressEditor : public QWidget
{
    Q_OBJECT
public:
    explicit NetworkAddressEditor(QWidget *parent, PreferenceAssociation *association,
                                  ApplicationPreferenceString *addressPref, ApplicationPreferenceInt *portPref);
    ~NetworkAddressEditor();
};

#endif // NETWORKADDRESSEDITOR_H

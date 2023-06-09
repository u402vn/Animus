#include "NetworkAddressEditor.h"
#include "Common/CommonWidgets.h"

NetworkAddressEditor::NetworkAddressEditor(QWidget *parent, PreferenceAssociation *association,
                                           ApplicationPreferenceString *addressPref, ApplicationPreferenceInt *portPref) : QWidget(parent)
{
    auto mainLayout = new QGridLayout(this);
    mainLayout->setMargin(0);

    int column = 1;
    if (addressPref != nullptr)
    {
        static QString ip4Range = "(([ 0]*)|( *[1-9] *)|( ?[1-9][0-9] ?)|(1[0-9][0-9])|([2][0-4][0-9])|(25[0-5]))";
        static QRegularExpression ip4Regex("^" + ip4Range + "\\." + ip4Range + "\\." + ip4Range + "\\." + ip4Range + "$");
        static QRegularExpressionValidator ip4Validator{ip4Regex};
        auto edtAddress = new QLineEdit(parent);
        edtAddress->setInputMask("000.000.000.000");
        edtAddress->setValidator(&ip4Validator);

        mainLayout->addWidget(edtAddress, 0, column++, 1, 1);
        association->addBinding(addressPref, edtAddress);
    }

    auto edtPort = CommonWidgetUtils::createPortEditor(this);
    mainLayout->addWidget(edtPort, 0, column++, 1, 1);
    association->addBinding(portPref, edtPort);
}

NetworkAddressEditor::~NetworkAddressEditor()
{

}

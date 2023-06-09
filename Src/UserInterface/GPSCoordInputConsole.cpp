#include "GPSCoordInputConsole.h"

#include <QGridLayout>
#include <QString>
#include <QStringList>
#include "Common/CommonWidgets.h"
#include "Common/CommonData.h"

void GPSCoordInputConsole::keyPressEvent(QKeyEvent *event)
{
    switch (event->key())
    {
    case Qt::Key_Return:
    case Qt::Key_Enter:
    case Qt::Key_Tab:
        if (_edtCoordLat->hasFocus())
        {
            event->accept();
            setEditFocus(_edtCoordLon);
        }
        else if (_edtCoordLon->hasFocus())
        {
            event->accept();
            emit onCoordEdited();
            emit onCoordEditingFinished();
            setEditFocus(_edtCoordLat);
        }
        break;
    default:
        QFrame::keyPressEvent(event);
    };
}

void GPSCoordInputConsole::setEditFocus(QLineEdit *edit)
{
    edit->setFocus();
    edit->setCursorPosition(0);
}

GPSCoordInputConsole::GPSCoordInputConsole(QWidget *parent) : QFrame(parent)
{
    _edtCoordLat = CommonWidgetUtils::createCoordEdit(this);
    connect(_edtCoordLat, &QLineEdit::textEdited, this, &GPSCoordInputConsole::onCoordTextEdited);
    _edtCoordLon = CommonWidgetUtils::createCoordEdit(this);
    connect(_edtCoordLon, &QLineEdit::textEdited, this, &GPSCoordInputConsole::onCoordTextEdited);

    auto coordLayout = new QGridLayout(this);
    coordLayout->setMargin(0);
    coordLayout->setSpacing(1);

    coordLayout->addWidget(_edtCoordLat, 0, 0, 1, 1);
    coordLayout->addWidget(_edtCoordLon, 0, 1, 1, 1);
}

GPSCoordInputConsole::~GPSCoordInputConsole()
{

}
/*
void parseCoordStr(const QString &coordinates)
{
    QRegularExpression re(
                "([SNsn][\\s]*)?((?:[\\+-]?[0-9]*[\\.,][0-9]+)|(?:[\\+-]?[0-9]+))(?:(?:[^ms\'′\"″,\\.\\dNEWnew]?)|(?:[^ms\'′\"″,\\.\\dNEWnew]+((?:[\\+-]?[0-9]*[\\.,][0-9]+)|(?:[\\+-]?[0-9]+))(?:(?:[^ds°\"″,\\.\\dNEWnew]?)|(?:[^ds°\"″,\\.\\dNEWnew]+((?:[\\+-]?[0-9]*[\\.,][0-9]+)|(?:[\\+-]?[0-9]+))[^dm°\'′,\\.\\dNEWnew]*))))"
                "([SNsn]?)[^\\dSNsnEWew]+([EWew][\\s]*)?((?:[\\+-]?[0-9]*[\\.,][0-9]+)|(?:[\\+-]?[0-9]+))(?:(?:[^ms\'′\"″,\\.\\dNEWnew]?)|(?:[^ms\'′\"″,\\.\\dNEWnew]+((?:[\\+-]?[0-9]*[\\.,][0-9]+)|(?:[\\+-]?[0-9]+))(?:(?:[^ds°\"″,\\.\\dNEWnew]?)|(?:[^ds°\"″,\\.\\dNEWnew]+((?:[\\+-]?[0-9]*[\\.,][0-9]+)|(?:[\\+-]?[0-9]+))[^dm°\'′,\\.\\dNEWnew]*))))([EWew]?)");

    QRegularExpressionMatch match = re.match(coordinates);
    if (match.hasMatch())
    {

    }

    auto match = re.match(coordinates);
    qDebug() << match.capturedTexts();

    QRegularExpressionMatchIterator i = re.globalMatch(coordinates);
    while (i.hasNext())
    {
        QRegularExpressionMatch match = i.next();
        if (match.hasMatch())
        {
            // qDebug() << match.captured(0);
            qDebug() << match.capturedTexts();
        }
    }
}
    */
void GPSCoordInputConsole::setCoordinatesString(const QString &coordinates)
{
    auto coordParts = coordinates.split(' ', QString::SkipEmptyParts);
    if (coordParts.count() == 2)
    {
        _edtCoordLat->setText(coordParts[0]);
        _edtCoordLon->setText(coordParts[1]);
    }
    else
    {
        _edtCoordLat->setText(QString("°\'\"%1").arg(WorldGPSCoord::postfixN()));
        _edtCoordLon->setText(QString("°\'\"%1").arg(WorldGPSCoord::postfixE()));
    }
}

const QString GPSCoordInputConsole::coordinatesString()
{
    QString lat = _edtCoordLat->displayText();
    QString lon = _edtCoordLon->displayText();

    QString result = QString("%1 %2").arg(lat).arg(lon);
    return result;
}

void GPSCoordInputConsole::onCoordTextEdited(const QString &text)
{
    Q_UNUSED(text);
    emit onCoordEdited();
}

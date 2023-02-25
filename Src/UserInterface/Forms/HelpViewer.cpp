#include "HelpViewer.h"
#include <QDesktopWidget>
#include <QDialogButtonBox>
#include <QVBoxLayout>
#include <QStyle>
#include <QFile>
#include <QTextCodec>
#include "Common/CommonWidgets.h"

HelpViewer::HelpViewer(QWidget *parent): QDialog(parent)
{
    this->setWindowTitle(tr("Help"));
    this->setModal(true);
    CommonWidgetUtils::updateWidgetGeometry(this, 800);
    //this->setAttribute(Qt::WA_DeleteOnClose, true);

    _textEdit = new QTextEdit(this);
    _textEdit->setReadOnly(true);
    _textEdit->setFocus();

    QDialogButtonBox * buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok, this);
    buttonBox->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    connect(buttonBox, &QDialogButtonBox::accepted, this, &HelpViewer::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &HelpViewer::reject);

    QVBoxLayout * mainLayout = CommonWidgetUtils::createVBoxLayout(this, 10);
    mainLayout->addWidget(_textEdit);
    mainLayout->addWidget(buttonBox);
}

bool HelpViewer::load(const QString &fileName)
{
    if (!QFile::exists(fileName))
        return false;
    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly))
        return false;
    QByteArray data = file.readAll();
    file.close();
    QTextCodec *codec = Qt::codecForHtml(data);
    QString str = codec->toUnicode(data);
    if (Qt::mightBeRichText(str))
    {
        _textEdit->setHtml(str);
    }
    else
    {
        str = QString::fromLocal8Bit(data);
        _textEdit->setPlainText(str);
    }

    return true;
}

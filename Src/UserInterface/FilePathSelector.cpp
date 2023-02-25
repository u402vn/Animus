#include "FilePathSelector.h"
#include <QGridLayout>
#include <QPushButton>
#include "Common/CommonWidgets.h"
#include "Common/CommonUtils.h"

FilePathSelector::FilePathSelector(QWidget *parent, const QString &selectorCaption, const QString &dialogCaption, const QString &filesFilter) : QWidget(parent)
{
    _dialogCaption = dialogCaption;
    _filesFilter = filesFilter;
    _useFolder = false;

    QGridLayout * layout = new QGridLayout(this);
    this->setLayout(layout);

    if (!selectorCaption.isEmpty())
    {
        _label = new QLabel(selectorCaption, this);
        _label->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed);
        setLabelWidth(150);
    }
    else
        _label = nullptr;

    _edit = new QLineEdit(this);
    _edit->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    _edit->setMinimumWidth(300);
    connect(_edit, &QLineEdit::textChanged, this, &FilePathSelector::onTextChanged);

    QPushButton * button = new QPushButton(this);
    button->setText(tr("..."));
    button->setCursor(Qt::ArrowCursor);
    button->setObjectName("FilePathSelectorButton");
    int frameWidth = _edit->style()->pixelMetric(QStyle::PM_DefaultFrameWidth);
    int fixedSize = _edit->sizeHint().height() - frameWidth * 2;
    button->setMinimumSize(fixedSize, fixedSize);
    button->setMaximumSize(fixedSize, fixedSize);

    connect(button, &QPushButton::released, this, &FilePathSelector::onButtonClicked);

    layout->setColumnStretch(0, 0);
    layout->setColumnStretch(1, 1);
    layout->setColumnStretch(2, 0);

    int col = 0;
    if (_label != nullptr)
        layout->addWidget(_label, 0, col++, 1, 1, Qt::AlignRight);
    layout->addWidget(_edit,  0, col++, 1, 1);
    layout->addWidget(button, 0, col++, 1, 1);
    layout->setRowStretch(0, 0);

    layout->setMargin(0);
    layout->setSpacing(0);
    //layout->setVerticalSpacing(0);
}

const QString FilePathSelector::getSelectedPath()
{
    return _edit->text().trimmed();
}

void FilePathSelector::setSelectedPath(const QString &value)
{
    _edit->setText(value);
}

void FilePathSelector::setUseFolder(bool value)
{
    _useFolder = value;
}

void FilePathSelector::setLabelWidth(int width)
{
    if (_label != nullptr)
    {
        _label->setMinimumWidth(width);
        _label->setMaximumWidth(width);
    }
}

void FilePathSelector::onButtonClicked()
{
    QString fileName = _useFolder ?
                CommonWidgetUtils::showOpenDirectoryDialog(_dialogCaption, _edit->text()) :
                CommonWidgetUtils::showOpenFileDialog(_dialogCaption, _edit->text(), _filesFilter);
    if (!fileName.isEmpty())
        _edit->setText(fileName);
}

void FilePathSelector::onTextChanged(const QString &text)
{
    Q_UNUSED(text);
    QString selectedFilePath = getSelectedPath();
    bool exist = (_useFolder && dirExists(selectedFilePath)) ||
            (!_useFolder && fileExists(selectedFilePath));
    _edit->setStyleSheet(exist ? "" : "color: red;");
}

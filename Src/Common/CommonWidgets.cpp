#include "CommonWidgets.h"
#include <QMessageBox>
#include <QFileDialog>
#include <QColorDialog>
#include <QPainter>
#include <QApplication>
#include <QDesktopWidget>
#include <QPropertyAnimation>

#define JOYSTICK_NUM_BUTTONS 14
#define JOYSTICK_NUM_AXES 5

//-------------------------------------- QDialEx --------------------------------------

QDialEx::QDialEx(QWidget *parent): QDial(parent)
{
    _setupCurrentValueInProgress = false;
    _indicatorValue = 0;
    _currentValue = 0;
    connect(this, &QDialEx::valueChanged, this, &QDialEx::processInternalValueChanged);
}

void QDialEx::setIndicatorValue(double value)
{
    if (_indicatorValue == value)
        return;
    _indicatorValue = value;
    update();
}

void QDialEx::processInternalValueChanged(int value)
{
    if (!_setupCurrentValueInProgress)
    {
        _currentValue = value;
        emit currentValueChanged(_currentValue);
    }
}

void QDialEx::setCurrentValue(double value)
{
    if (_currentValue == value)
        return;

    if (value < this->minimum())
        value = this->minimum();

    if (value > this->maximum())
        value = this->maximum();

    _setupCurrentValueInProgress = true;
    _currentValue = value;
    this->setValue(value);
    _setupCurrentValueInProgress = false;
    update();
    emit currentValueChanged(_currentValue);
}

double QDialEx::currentValue()
{
    return _currentValue;
}

void QDialEx::setCurrentValueForced(double value)
{
    if (value <= this->minimum())
        this->setMinimum(value - 1);
    if (value >= this->maximum())
        this->setMaximum(value + 1);
    setCurrentValue(value);
}

double QDialEx::dialMinimalStep()
{
    return 0.1;
}

void QDialEx::paintEvent(QPaintEvent *event)
{
    const int TEXT_VALUE_VERTICAL_INTERVAL = 14;
    QDial::paintEvent(event);

    QPainter painter(this);
    QFontMetrics fm(painter.font());
    int pixelsTextHigh = fm.height();
    QRect rect = QRect(0, 0, this->width(), this->height());
    painter.setPen(palette().color(QPalette::ButtonText));

    QRect valueRect;
    valueRect.setWidth(rect.width());
    valueRect.setHeight(pixelsTextHigh + 10);

    if (this->isEnabled())
    {
        valueRect.moveCenter(QPoint(rect.width() / 2, (rect.height() - pixelsTextHigh - TEXT_VALUE_VERTICAL_INTERVAL) / 2));
        painter.drawText(valueRect, Qt::AlignHCenter | Qt::AlignVCenter, QString::number(_currentValue, 'f', 1), nullptr);

        if (qAbs(_indicatorValue - _currentValue) >= dialMinimalStep())
            painter.setPen(Qt::red);

        valueRect.moveCenter(QPoint(rect.width() / 2, (rect.height() + pixelsTextHigh + TEXT_VALUE_VERTICAL_INTERVAL) / 2));
        painter.drawText(valueRect, Qt::AlignHCenter | Qt::AlignVCenter, QString::number(_indicatorValue, 'f', 1), nullptr);
    }
    else
    {
        painter.setPen(Qt::red);
        valueRect.moveCenter(rect.center());
        painter.drawText(valueRect, Qt::AlignHCenter | Qt::AlignVCenter, QString::number(_indicatorValue, 'f', 1), nullptr);
    }
}

//-------------------------------------- QButtonGroupExt --------------------------------------

QButtonGroupExt::QButtonGroupExt(QWidget *parent) : QButtonGroup(parent)
{
}

QButtonGroupExt::~QButtonGroupExt()
{

}

void QButtonGroupExt::setCheckedId(int buttonId)
{
    foreach (QAbstractButton *button, buttons())
        if (id(button) == buttonId)
        {
            button->setChecked(true);
            emit buttonClicked(buttonId);
            break;
        }
}

QRadioButton *QButtonGroupExt::appendButton(const QString &caption, const int id)
{
    auto button = new QRadioButton(caption, (QWidget*)this->parent());
    this->addButton(button, id);
    return button;
}

//-------------------------------------- QComboBoxExt --------------------------------------

void QComboBoxExt::wheelEvent(QWheelEvent *event)
{
    if (!_ignoreMouseWheel)
        QComboBox::wheelEvent(event);
}

QComboBoxExt::QComboBoxExt(QWidget *parent) : QComboBox(parent)
{
    _ignoreMouseWheel = true;
    setFocusPolicy(Qt::StrongFocus); //disable focus by mouse wheel
}

QComboBoxExt::QComboBoxExt(QWidget *parent, const QMap<int, QString> values) : QComboBoxExt(parent)
{
    auto i = values.begin();
    while (i != values.end())
    {
        this->addItem(i.value(), i.key());
        ++i;
    }
}

void QComboBoxExt::setCurrentData(const QVariant data)
{
    int index = this->findData(data);
    if (index >= 0)
        this->setCurrentIndex(index);
}

void QComboBoxExt::setIgnoreMouseWheel(bool value)
{
    _ignoreMouseWheel = value;
}

bool QComboBoxExt::ignoreMouseWheel()
{
    return _ignoreMouseWheel;
}

//-------------------------------------- QLabelEx --------------------------------------

void QLabelEx::mousePressEvent(QMouseEvent *event)
{
    if (event->buttons() == Qt::LeftButton)
        emit clicked();
}

QLabelEx::QLabelEx(QWidget *parent) : QLabel(parent)
{

}

//---------------------------------- SelectColorButton ----------------------------------

SelectColorButton::SelectColorButton(const QString &text, QWidget *parent) : QPushButton(text, parent)
{
    connect(this, &SelectColorButton::clicked, this, &SelectColorButton::changeColor);
}

void SelectColorButton::updateColor()
{
    double colorAsGray = (0.299 * _color.redF() + 0.587 *_color.greenF() + 0.114 * _color.blueF());
    QString textColor = colorAsGray > 0.5 ? "black" : "white";
    setStyleSheet("background-color: " + _color.name() + ";color: " + textColor);
}

void SelectColorButton::changeColor()
{
    QColor newColor = QColorDialog::getColor(_color, parentWidget());
    if (newColor.isValid() && (newColor.name() != _color.name()) )
        setColor(newColor);
}

void SelectColorButton::setColor(const QColor& newColor)
{
    _color = newColor;
    updateColor();
}

const QColor& SelectColorButton::getColor()
{
    return _color;
}

//-------------------------------------- Functions --------------------------------------

QVBoxLayout *CommonWidgetUtils::createVBoxLayout(QWidget *widget, int spacing)
{
    auto layout = new QVBoxLayout();
    widget->setLayout(layout);
    layout->setMargin(spacing);
    layout->setSpacing(spacing);
    return layout;
}

QAction *CommonWidgetUtils::createCheckableMenuSingleAction(const QString &caption, bool isChecked, QMenu *menu)
{
    auto parent = menu->parent();
    auto newAction = new QAction(caption, parent);
    newAction->setCheckable(true);
    newAction->setChecked(isChecked);
    menu->addAction(newAction);
    return newAction;
}

QAction *CommonWidgetUtils::createCheckableMenuGroupAction(const QString &caption, bool isChecked, QActionGroup *group, QMenu *menu, const QVariant &data)
{
    auto parent = menu->parent();
    auto newAction = new QAction(caption, parent);
    newAction->setCheckable(true);
    newAction->setActionGroup(group);
    newAction->setChecked(isChecked);
    newAction->setData(data);
    menu->addAction(newAction);
    return newAction;
}

QAction *CommonWidgetUtils::createMenuAction(const QString &caption, QMenu *menu)
{
    auto parent = menu->parent();
    auto newAction = new QAction(caption, parent);
    newAction->setCheckable(false);
    menu->addAction(newAction);
    return newAction;
}

QDialEx *CommonWidgetUtils::createDialEx(QWidget *parent, int size, int min, int max)
{
    //http://www.codeprogress.com/cpp/libraries/qt/showQtExample.php?index=561&key=QDialColoredText
    //http://dronecolony.com/2012/12/11/customized-qdial-with-qss-support/

    auto dial = new QDialEx(parent);
    dial->setRange(min, max);
    dial->setNotchesVisible(true);
    dial->setFixedSize(size, size);
    dial->setFocusPolicy(Qt::NoFocus);

    return dial;
}

QComboBoxExt *CommonWidgetUtils::createJButtonComboBox(QWidget *parent)
{
    auto comboBox = new QComboBoxExt(parent);
    comboBox->setIgnoreMouseWheel(true);
    comboBox->addItem(tr("None"), -1);
    for (int i = 0; i < JOYSTICK_NUM_BUTTONS; i++)
        comboBox->addItem(tr("Button %1").arg(i + 1), i);
    return comboBox;
}

QComboBoxExt *CommonWidgetUtils::createJAxisComboBox(QWidget *parent)
{
    auto comboBox = new QComboBoxExt(parent);
    comboBox->setIgnoreMouseWheel(true);
    comboBox->addItem(tr("None"), -1);
    for (int i = 0; i < JOYSTICK_NUM_AXES; i++)
        comboBox->addItem(tr("Axis %1").arg(i), i);
    return comboBox;
}


QFrame *CommonWidgetUtils::createSeparator(QWidget *parent)
{
    auto separator = new QFrame(parent);
    separator->setFrameShape(QFrame::HLine);
    separator->setFrameShadow(QFrame::Sunken);
    return separator;
}

QPushButton *CommonWidgetUtils::createButton(QWidget *parent, const QString &caption, const QString &toolTip, bool checkable, int width, int height, const QString &iconName)
{
    auto button = new QPushButton(caption, parent);
    button->setToolTip(toolTip);
    button->setCheckable(checkable);
    if ((width > 0) && (height > 0))
        button->setFixedSize(width, height);
    button->setFocusPolicy(Qt::NoFocus);

    if (!iconName.isEmpty())
    {
        QPixmap pixmap(iconName);
        QIcon icon(pixmap);
        button->setIcon(icon);
        button->setIconSize(pixmap.rect().size());
    }
    return button;
}

QSpinBox *CommonWidgetUtils::createRangeSpinbox(QWidget *parent, int minValue, int maxValue)
{
    auto spinBox = new QSpinBox(parent);
    spinBox->setRange(minValue, maxValue);
    spinBox->setAlignment(Qt::AlignRight);
    spinBox->setMaximumWidth(100);
    return spinBox;
}

QDoubleSpinBox *CommonWidgetUtils::createDoubleRangeSpinbox(QWidget *parent, double minValue, double maxValue, double step, int decimals)
{
    auto spinBox = new QDoubleSpinBox(parent);
    spinBox->setRange(minValue, maxValue);
    spinBox->setDecimals(decimals);
    spinBox->setSingleStep(step);
    spinBox->setAlignment(Qt::AlignRight);
    spinBox->setMaximumWidth(100);
    return spinBox;
}

void CommonWidgetUtils::drawText(QPainter &painter, QPoint pos, Qt::Alignment flags, const QString &text, bool shadowBackground)
{
    const qreal size = 32767.0;
    QPointF corner(pos.x(), pos.y() - size);
    if (flags & Qt::AlignHCenter)
        corner.rx() -= size/2.0;
    else if (flags & Qt::AlignRight)
        corner.rx() -= size;
    if (flags & Qt::AlignVCenter)
        corner.ry() += size/2.0;
    else if (flags & Qt::AlignTop)
        corner.ry() += size;
    else
        flags |= Qt::AlignBottom;
    QRectF rect {corner.x(), corner.y(), size, size};

    if (shadowBackground)
    {
        QFontMetrics fm = painter.fontMetrics();
        QRect boundingRect = fm.boundingRect(rect.toAlignedRect(), flags, text);
        QColor shadowColor(0, 0, 0, 128);
        painter.fillRect(boundingRect, shadowColor);
    }

    painter.drawText(rect, flags, text);
}

QGridLayout *CommonWidgetUtils::createGridLayoutForScrollArea(QScrollArea *scrollArea)
{
    auto scrollAreaWidget = new QWidget(scrollArea);
    scrollArea->setWidgetResizable(true);
    scrollArea->setWidget(scrollAreaWidget);
    scrollArea->setFrameShape(QFrame::NoFrame);

    QGridLayout *gridLayout = new QGridLayout();
    scrollAreaWidget->setLayout(gridLayout);

    return gridLayout;
}

bool CommonWidgetUtils::showConfirmDialog(const QString &text, bool defaultValue)
{
    QMessageBox::StandardButton defaultButton = defaultValue ? QMessageBox::Yes : QMessageBox::No;
    bool dialogResult = QMessageBox::Yes ==
            QMessageBox::question(nullptr,  qApp->applicationName(), text, QMessageBox::No | QMessageBox::Yes, defaultButton);
    return dialogResult;
}

const QString CommonWidgetUtils::showOpenFileDialog(const QString &caption, const QString &dir, const QString &filter)
{
    QString fileName = QFileDialog::getOpenFileName(nullptr, caption, dir, filter);
    return fileName;
}

const QString CommonWidgetUtils::showSaveFileDialog(const QString &caption,  const QString &dir, const QString &filter)
{
    QString fileName = QFileDialog::getSaveFileName(nullptr, caption, dir, filter);
    return fileName;
}

const QString CommonWidgetUtils::showOpenDirectoryDialog(const QString &caption, const QString &dir)
{
    QString dirName = QFileDialog::getExistingDirectory(nullptr, caption, dir);
    return dirName;
}

void CommonWidgetUtils::showInfoDialog(const QString &text)
{
    QMessageBox::information(nullptr,  qApp->applicationName(), text, QMessageBox::Ok);
}

void CommonWidgetUtils::showInfoDialogAutoclose(const QString &text, quint32 timeout)
{
    QMessageBox msgBox(QMessageBox::Information, qApp->applicationName(), text, QMessageBox::Ok, nullptr, Qt::Dialog);
    msgBox.setStandardButtons(QMessageBox::Ok);
    msgBox.button(QMessageBox::Ok)->animateClick(timeout);
    msgBox.exec();
}

QLineEdit *CommonWidgetUtils::makePortEditor(QWidget *parent)
{
    static QRegExp portRegex("^(?:[0-9]?[0-9]?[0-9]?[0-9]?[0-9])$");
    static QRegExpValidator portValidator{portRegex};

    auto editor = new QLineEdit(parent);
    editor->setValidator(&portValidator);
    editor->setMaximumWidth(100);
    return editor;
}

QRect CommonWidgetUtils::getDesktopAvailableGeometry()
{
    QRect result = qApp->desktop()->availableGeometry();
    return result;
}

QWidget *CommonWidgetUtils::createScrolledWidget(QWidget *parent)
{
    auto scrollArea = new QScrollArea(parent);
    auto scrolledWidget = new QWidget(scrollArea);
    scrollArea->setWidgetResizable(true);
    scrollArea->setWidget(scrolledWidget);
    scrollArea->setFrameShape(QFrame::NoFrame);
    return scrolledWidget;
}

QDialogButtonBox *CommonWidgetUtils::makeDialogButtonBox(QDialog *dialog, QDialogButtonBox::StandardButtons buttons)
{
    auto buttonBox = new QDialogButtonBox(buttons, dialog);
    buttonBox->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    connect(buttonBox, &QDialogButtonBox::accepted, dialog, &QDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, dialog, &QDialog::reject);
    return buttonBox;
}

void CommonWidgetUtils::updateWidgetGeometry(QWidget *widget, const int width, const int height)
{
    QRect desktopGeometry = CommonWidgetUtils::getDesktopAvailableGeometry();
    int widgetHeight = height > 0 ? height : desktopGeometry.height() * 2 / 3;
    QSize widgetRectSize(width, widgetHeight);
    QRect widgetRect = QStyle::alignedRect(Qt::LeftToRight, Qt::AlignCenter, widgetRectSize, desktopGeometry);
    widget->setGeometry(widgetRect);
    widget->setMinimumSize(widgetRect.size());
}

void CommonWidgetUtils::installEventFilterToApplication(QObject *object)
{
    qApp->installEventFilter(object);
}

//-------------------------------------- KeySequenceEditExt --------------------------------------

// https://web.archive.org/web/20130613105442/http://blog.qt.digia.com/blog/2007/06/06/lineedit-with-a-clear-button
void KeySequenceEditExt::resizeEvent(QResizeEvent *)
{
    QSize sz = _clearButton->size();
    int frameWidth = style()->pixelMetric(QStyle::PM_DefaultFrameWidth);
    int left = rect().right() - sz.width();
    int top = rect().top() + frameWidth;
    _clearButton->move(left, top);
}

KeySequenceEditExt::KeySequenceEditExt(QWidget *parent) :
    QKeySequenceEdit(parent)
{
    _clearButton = new QPushButton("X", this);
    _clearButton->setCursor(Qt::ArrowCursor);
    _clearButton->setObjectName("KeySequenceEditButton");

    connect(_clearButton, &QAbstractButton::clicked, this, &QKeySequenceEdit::clear);

    int frameWidth = style()->pixelMetric(QStyle::PM_DefaultFrameWidth);
    int fixedSize = this->sizeHint().height() - frameWidth * 2;
    _clearButton->setMinimumSize(fixedSize, fixedSize);
    _clearButton->setMaximumSize(fixedSize, fixedSize);
}

//-------------------------------------- CheckableMenu --------------------------------------

CheckableMenu::CheckableMenu(QWidget *parent) : QMenu (parent)
{
    _ignoreHide = false;
}

CheckableMenu::CheckableMenu(const QString &title, QWidget *parent) : QMenu (title, parent)
{
    _ignoreHide = false;
}

CheckableMenu::~CheckableMenu()
{
}

void CheckableMenu::mouseReleaseEvent(QMouseEvent *e)
{
    auto action = actionAt(e->pos());
    _ignoreHide = (action != nullptr) && action->isCheckable();
    QMenu::mouseReleaseEvent(e);
}

void CheckableMenu::setVisible(bool visible)
{
    if (_ignoreHide)
    {
        _ignoreHide = false;
        return;
    }
    QMenu::setVisible(visible);
}

//-------------------------------------- Spoiler --------------------------------------

Spoiler::Spoiler(const QString &title, QWidget *parent) : QWidget(parent)
{
    _toggleButton.setStyleSheet("QToolButton { border: none; font-weight: bold; }");
    _toggleButton.setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    _toggleButton.setArrowType(Qt::ArrowType::RightArrow);
    _toggleButton.setText(title);
    _toggleButton.setCheckable(true);
    _toggleButton.setChecked(false);

    _headerLine.setFrameShape(QFrame::HLine);
    _headerLine.setFrameShadow(QFrame::Sunken);
    _headerLine.setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Maximum);

    _contentArea.setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    // start out collapsed
    _contentArea.setMaximumHeight(0);
    _contentArea.setMinimumHeight(0);
    // let the entire widget grow and shrink with its content
    _toggleAnimation.addAnimation(new QPropertyAnimation(this, "minimumHeight"));
    _toggleAnimation.addAnimation(new QPropertyAnimation(this, "maximumHeight"));
    _toggleAnimation.addAnimation(new QPropertyAnimation(&_contentArea, "maximumHeight"));
    // don't waste space
    _mainLayout.setVerticalSpacing(0);
    _mainLayout.setContentsMargins(0, 0, 0, 0);
    int row = 0;
    _mainLayout.addWidget(&_toggleButton, row, 0, 1, 1, Qt::AlignLeft);
    _mainLayout.addWidget(&_headerLine, row++, 2, 1, 1);
    _mainLayout.addWidget(&_contentArea, row, 0, 1, 3);
    setLayout(&_mainLayout);
    QObject::connect(&_toggleButton, &QToolButton::clicked, [this](const bool checked)
    {
        if (checked)
        {
            auto contentLayout = _contentArea.layout();
            const auto collapsedHeight = sizeHint().height() - _contentArea.maximumHeight();
            auto contentHeight = contentLayout->sizeHint().height();
            for (int i = 0; i < _toggleAnimation.animationCount() - 1; ++i)
            {
                auto spoilerAnimation = static_cast<QPropertyAnimation *>(_toggleAnimation.animationAt(i));
                spoilerAnimation->setDuration(_animationDuration);
                spoilerAnimation->setStartValue(collapsedHeight);
                spoilerAnimation->setEndValue(collapsedHeight + contentHeight);
            }
            auto contentAnimation = static_cast<QPropertyAnimation *>(_toggleAnimation.animationAt(_toggleAnimation.animationCount() - 1));
            contentAnimation->setDuration(_animationDuration);
            contentAnimation->setStartValue(0);
            contentAnimation->setEndValue(contentHeight);
        }
        _toggleButton.setArrowType(checked ? Qt::ArrowType::DownArrow : Qt::ArrowType::RightArrow);
        _toggleAnimation.setDirection(checked ? QAbstractAnimation::Forward : QAbstractAnimation::Backward);
        _toggleAnimation.start();
    });
}

void Spoiler::setContentLayout(QLayout *contentLayout)
{
    delete _contentArea.layout();
    _contentArea.setLayout(contentLayout);
}

//-------------------------------------- SpoilerGrid --------------------------------------

SpoilerGrid::SpoilerGrid(const QString &title, QWidget *parent) : Spoiler(title, parent)
{
    setContentLayout(new QGridLayout());
}

QGridLayout *SpoilerGrid::gridLayout()
{
    return (QGridLayout *)_contentArea.layout();
}

//-------------------------------------- GridWidget --------------------------------------

void GridWidget::addWidget(QWidget *cellWidget, int row, int column, int rowSpan, int columnSpan, Qt::Alignment alignment)
{
    _mainLayout.addWidget(cellWidget, row, column, rowSpan, columnSpan, alignment);
}

GridWidget::GridWidget(QWidget *parent) : QWidget(parent)
{
    setLayout(&_mainLayout);
    _mainLayout.setColumnStretch(0, 1);
    _mainLayout.setColumnStretch(1, 0);
    _mainLayout.setColumnStretch(2, 0);
    _mainLayout.setColumnStretch(3, 0);
    _mainLayout.setColumnStretch(4, 0);
    _mainLayout.setColumnStretch(5, 1);
}

GridWidget::~GridWidget()
{

}

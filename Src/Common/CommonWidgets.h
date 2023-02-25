#ifndef COMMONWIDGETS_H
#define COMMONWIDGETS_H

#include <QWidget>
#include <QVBoxLayout>
#include <QDial>
#include <QFrame>
#include <QPushButton>
#include <QButtonGroup>
#include <QComboBox>
#include <QRadioButton>
#include <QKeySequenceEdit>
#include <QLabel>
#include <QToolButton>
#include <QLineEdit>
#include <QMouseEvent>
#include <QContextMenuEvent>
#include <QActionGroup>
#include <QAction>
#include <QMenu>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QDialogButtonBox>
#include <QDialog>
#include <QGridLayout>
#include <QScrollArea>
#include <QParallelAnimationGroup>

constexpr int DEFAULT_BUTTON_WIDTH = 300;
constexpr int DEFAULT_BUTTON_HEIGHT = 40;
constexpr int QUARTER_BUTTON_WIDTH =  69;//0.23 * DEFAULT_BUTTON_WIDTH;
static const QString NO_CAPTION = "";

class QDialEx final : public QDial
{
    Q_OBJECT
    double _indicatorValue;
    double _currentValue;
    bool _setupCurrentValueInProgress;
public:
    QDialEx(QWidget *parent);

    void setIndicatorValue(double value);
    void setCurrentValue(double value);
    double currentValue();
    void setCurrentValueForced(double value);

    static double dialMinimalStep();
protected:
    virtual void paintEvent(QPaintEvent *event);
private slots:
    void processInternalValueChanged(int value);
signals:
    void currentValueChanged(double currentValue);
};

class QButtonGroupExt final: public QButtonGroup
{
    Q_OBJECT
public:
    QButtonGroupExt(QWidget *parent);
    ~QButtonGroupExt();
    void setCheckedId(int buttonId);
    QRadioButton *appendButton(const QString &caption, const int id);
};

class QComboBoxExt final : public QComboBox
{
    Q_OBJECT
    bool _ignoreMouseWheel;
protected:
    void wheelEvent(QWheelEvent *event);
public:
    QComboBoxExt(QWidget *parent);
    QComboBoxExt(QWidget *parent, const QMap<int, QString> values);
    void setCurrentData(const QVariant data);
    void setIgnoreMouseWheel(bool value);
    bool ignoreMouseWheel();
};

class QLabelEx final : public QLabel
{
    Q_OBJECT
protected:
    void mousePressEvent(QMouseEvent *event);
public:
    QLabelEx(QWidget *parent);
signals:
    void clicked();
};

class SelectColorButton final : public QPushButton
{
    Q_OBJECT
public:
    SelectColorButton(const QString &text, QWidget *parent);
    void setColor(const QColor& newColor);
    const QColor& getColor();
public slots:
    void updateColor();
    void changeColor();
private:
    QColor _color;
};

class KeySequenceEditExt final : public QKeySequenceEdit
{
    Q_OBJECT
    QPushButton *_clearButton;
protected:
    void resizeEvent(QResizeEvent *);
public:
    KeySequenceEditExt(QWidget *parent);
};

class Spoiler : public QWidget
{
    Q_OBJECT
protected:
    QGridLayout _mainLayout;
    QToolButton _toggleButton;
    QFrame _headerLine;
    QParallelAnimationGroup _toggleAnimation;
    QScrollArea _contentArea;
    int _animationDuration{300};
public:
    explicit Spoiler(const QString &title, QWidget *parent);
    void setContentLayout(QLayout *contentLayout);
};

class SpoilerGrid : public Spoiler
{
    Q_OBJECT
    using Spoiler::setContentLayout;
public:
    explicit SpoilerGrid(const QString &title, QWidget *parent);
    QGridLayout *gridLayout();
};

class CheckableMenu : public QMenu
{
    Q_OBJECT
    bool _ignoreHide;
public:
    CheckableMenu(QWidget *parent);
    CheckableMenu(const QString &title, QWidget *parent);
    virtual ~CheckableMenu();
    virtual void mouseReleaseEvent (QMouseEvent *e);
    virtual void setVisible (bool visible);
};

class GridWidget : public QWidget
{
    Q_OBJECT
    QGridLayout _mainLayout;
public:
    void addWidget(QWidget *cellWidget, int row, int column, int rowSpan, int columnSpan, Qt::Alignment = 0);
    explicit GridWidget(QWidget *parent);
    ~GridWidget();
};

class CommonWidgetUtils : public QObject
{
    Q_OBJECT
public:
    static QVBoxLayout *createVBoxLayout(QWidget *widget, int spacing = 5);
    static QAction *createCheckableMenuSingleAction(const QString &caption, bool isChecked, QMenu *menu);
    static QAction *createCheckableMenuGroupAction(const QString &caption, bool isChecked, QActionGroup *group, QMenu *menu, const QVariant &var);
    static QAction *createMenuAction(const QString &caption, QMenu *menu);
    static QDialEx *createDialEx(QWidget *parent, int size, int min, int max);
    static QComboBoxExt *createJButtonComboBox(QWidget *parent);
    static QComboBoxExt *createJAxisComboBox(QWidget *parent);
    static QFrame *createSeparator(QWidget *parent);
    static QPushButton *createButton(QWidget *parent, const QString &caption, const QString &toolTip,
                                     bool checkable, int width, int height, const QString &iconName);
    static QSpinBox *createRangeSpinbox(QWidget *parent, int minValue, int maxValue);
    static QDoubleSpinBox *createDoubleRangeSpinbox(QWidget *parent, double minValue, double maxValue, double step, int decimals);
    static void drawText(QPainter &painter, QPoint pos, Qt::Alignment flags, const QString &text, bool shadowBackground);
    static QGridLayout *createGridLayoutForScrollArea(QScrollArea *scrollArea);
    static QWidget *createScrolledWidget(QWidget *parent);

    static void showInfoDialog(const QString &text);
    static bool showConfirmDialog(const QString &text, bool defaultValue);
    static const QString showOpenFileDialog(const QString &caption, const QString &dir, const QString &filter);
    static const QString showSaveFileDialog(const QString &caption, const QString &dir, const QString &filter);
    static const QString showOpenDirectoryDialog(const QString &caption, const QString &dir);
    static QLineEdit *makePortEditor(QWidget *parent);
    static QRect getDesktopAvailableGeometry();
    static QDialogButtonBox *makeDialogButtonBox(QDialog *dialog, QDialogButtonBox::StandardButtons buttons);
    static void updateWidgetGeometry(QWidget *widget, const int width, const int height = 0);
    static void installEventFilterToApplication(QObject *object);
};

#endif // COMMONWIDGETS_H

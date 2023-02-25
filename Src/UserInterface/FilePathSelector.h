#ifndef FILEPATHSELECTOR_H
#define FILEPATHSELECTOR_H

#include <QWidget>
#include <QLineEdit>
#include <QLabel>

class FilePathSelector final : public QWidget
{
    Q_OBJECT

    QString _dialogCaption;
    QString _filesFilter;
    bool _useFolder;

    QLabel * _label;
    QLineEdit * _edit;

public:
    explicit FilePathSelector(QWidget *parent, const QString &selectorCaption, const QString &dialogCaption, const QString &filesFilter);
    const QString getSelectedPath();
    void setSelectedPath(const QString &value);
    void setUseFolder(bool value);
    void setLabelWidth(int width);

private slots:
    void onButtonClicked();
    void onTextChanged(const QString & text);
};

#endif // FILEPATHSELECTOR_H

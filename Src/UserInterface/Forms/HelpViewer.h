#ifndef HELPVIEWER_H
#define HELPVIEWER_H

#include <QWidget>
#include <QDialog>
#include <QTextEdit>


//see text edit example

class HelpViewer final : public QDialog
{
    Q_OBJECT

    QTextEdit *_textEdit;
public:
    HelpViewer(QWidget *parent);

    bool load(const QString &fileName);
};

#endif // HELPVIEWER_H

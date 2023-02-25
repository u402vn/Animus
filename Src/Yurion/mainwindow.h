#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QtNetwork/QUdpSocket>
#include <QFile>

namespace Ui {
class MainWindow;
}

typedef void (*Inst) (unsigned char *);
typedef int (*Dec10)(
    const void *const pBufferIn,
    const unsigned int inBufferSizeBytes,
    void *const pBufferOut,
    const int black,
    const int YGain,
    const int CGain,
    const int Cif,
    const int time,
    void *const g_buff);

class MainWindow : public QMainWindow
{
    Q_OBJECT

    bool state = false;
    QUdpSocket sc;
    QFile *df, *nf;

    long bytes = 0;
    long packets = 0;

    QByteArray inBytes;
    QByteArray outBytes;

    Inst inst;
    Dec10 dec10;

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private slots:
    void readData();
    void on_pushButton_clicked();

private:
    Ui::MainWindow *ui;

    void SaveData(QByteArray& ba);
    void DecodeUrion(QByteArray& ba);
    void ParseData();
    uint16_t w16(QByteArray &ba, int idx);
};

#endif // MAINWINDOW_H

#include "mainwindow.h"
#include "ui_mainwindow.h"
//#include "mw11dec.h"
#include <QLibrary>
#include "opencv2/opencv.hpp"
#include "opencv/cv.h"
#include <QTime>

#define INPORT 44444
#define MW_DECODER_CONTEXT_SIZE_BYTES 5006288

using namespace cv;

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    inBytes.clear();

    ui->setupUi(this);
    ui->lineEdit->setInputMask("99999");
    //df = new QFile("D:\\Temp\\Yurion\\Rec16-49-48.977.bin");
    df = new QFile("D:\\Temp\\Yurion\\rawudpdata8.bin");
    nf = new QFile();
    //df = new QFile();

    connect(&sc, SIGNAL(readyRead()), this, SLOT(readData()));

    //DecodeUrion();

    //QByteArray bain;
    df->open(QIODevice::ReadOnly);

    long sz = df->size();
    inBytes.reserve(sz);
    inBytes = df->read(sz);
    df->close();

    QLibrary lib("mw11dec");
    inst = (Inst)(lib.resolve("_MWDec_Install"));
    dec10 = (Dec10)(lib.resolve("MwDecodeFrameV10Checked"));

    if (!inst || !dec10)
    {
        this->close();
    }
    ParseData();

}

MainWindow::~MainWindow()
{
    delete df;

    if(nf->isOpen())
        nf->close();
    delete nf;

    delete ui;
}

void MainWindow::readData()
{
    const int MAGIC_SIZE = 31980*2;

    QByteArray tmpBytes;
    while (sc.hasPendingDatagrams())
    {
        tmpBytes.reserve(sc.pendingDatagramSize());
        sc.readDatagram(tmpBytes.data(), tmpBytes.size());
        inBytes.reserve(inBytes.size()+tmpBytes.size());
        inBytes.append(tmpBytes);
        //SaveData(inBytes);
    }
    if(inBytes.size() > MAGIC_SIZE)
    {
        ParseData();
    }

}

void MainWindow::SaveData(QByteArray& ba)
{
    //qDebug() << ba.toHex();
    nf->write(ba);
    packets++;
    bytes += ba.size();

    ui->bytes->setText(QString("%1").arg(bytes));
    ui->packets->setText(QString("%1").arg(packets));
}

void MainWindow::DecodeUrion(QByteArray& ba)
{
    unsigned char *internal = new unsigned char [MW_DECODER_CONTEXT_SIZE_BYTES];
    static int nFrame = 1;

    /*
    QFile udo("d:\\Temporary\\Yurion\\mw10.data");
    udo.open(QIODevice::ReadOnly);
    long sz = udo.size();
    ba.reserve(sz);
    ba = udo.read(sz);
    udo.close();
    */

    long sz = ba.size();
    outBytes.reserve(704 * 576 * 4);

    inst(internal);
    //bool res = dec10(ba.data(), sz, outBytes.data(), 0, 0, 0, 0, 0, internal);
    bool res = dec10(ba.data(), sz, outBytes.data(), 0, 0, 0, 0, 0, internal);
    bool b;
    if(res)
    {
        //udo.write(baout);
        qDebug() << "Decode complete: Frame " << nFrame;

        QImage img((const uchar*)outBytes.data(),704,576,QImage::Format_ARGB32);
        img = img.mirrored();
        QString fName = "img.bmp";
        fName.insert(3, QString::number(nFrame));
        b = img.save(fName, "bmp", 100);
        nFrame++;
        qDebug() << b;
    }
    else
    {
        qDebug() << "Decode error";
    }

    //udo.close();
    delete [] internal;
}

void MainWindow::on_pushButton_clicked()
{
    quint16 inPort = INPORT;
    static int fileId = 0;

    if((state = !state))
    {
        ui->pushButton->setText("Disconnect");
        inPort = ui->lineEdit->text().toInt();
        //df->setFileName("Rec" + QString::number(fileId++) + ".bin");
        nf->setFileName("Rec" + QTime::currentTime().toString("hh-mm-ss.zzz") + ".bin");
        nf->open(QIODevice::WriteOnly);
        bool res = sc.bind(QHostAddress::Any, inPort);
        if(!res)
            qDebug() << "Not binded";
    }
    else
    {
        ui->pushButton->setText("Connect");
        sc.close();
        if(nf->isOpen())
            nf->close();
    }
}

void MainWindow::ParseData()
{
    int sCount = 0;
    int idx = -1;
    int idxEnd = -1;
    bool res = false;

    /*
    const unsigned char cmd[] = {//0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x57, 0x44,
                                 0x00, 0x18, 0x00, 0x00, 0x55, 0x55, 0x55, 0x55,
                                 0x01, 0x00, 0x00, 0x00, 0x03, 0x10, 0x00, 0x00,
                                 0x38, 0x41, 0xe9, 0x26, 0xb5, 0xc2, 0x00, 0x00,
                                 0x00, 0x00, 0x12, 0x03, 0xFF, 0xFF, 0xFF, 0xFF,
                                 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                                 0x57, 0x44, 0x33};
    QByteArray ba(reinterpret_cast<const char*>(cmd), 43);
    inBytes = ba;
    */

    const unsigned char preamble[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x57, 0x44};
    QByteArray prbyte(reinterpret_cast<const char*>(preamble), 8);
    const unsigned char fin[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
    QByteArray fnbyte(reinterpret_cast<const char*>(fin), 6);

    do
    {
        // !!!!
        outBytes.clear();

        idx = inBytes.indexOf(prbyte, idx+1);
        idxEnd = inBytes.indexOf(fnbyte, idxEnd+1);
        if(idxEnd < idx && idxEnd > 0)
        {
            QByteArray tmp = inBytes.left(idxEnd);
            DecodeUrion(tmp);
        }
        if(idx < 0)
        {
            // очистить массив кроме последних 8 байт
            inBytes.remove(0, inBytes.size() - 8);
            qDebug() << "Frame not found";
            return;
        }
        if(inBytes.size() < (idx+16))
        {
            qDebug() << "Frame damaged";
            return;
        }
        int frSize = w16(inBytes, idx+4*2) + (w16(inBytes, idx+5*2) << 16);
        if(inBytes.size() < (idx+frSize))
        {
            qDebug() << "Frame damaged";
            return;
        }

        if(idxEnd >= 5000000 && idxEnd < idx)
        {
            SaveData(outBytes);
        }

        inBytes.remove(0,idx);
        idx = 0;    // TODO: удалить idx из расчетов
        unsigned char c = inBytes.at(6);

        int idx8 = idx;
        //int lb = w16(inBytes, idx+8*2);
        //int hb = w16(inBytes, idx+9*2) << 16;
        //int frNum = lb + hb;
        int frNum = w16(inBytes, idx+8*2) + (w16(inBytes, idx+9*2) << 16);
        int word14audio = inBytes.at(idx+14*2);
        int word14video = inBytes.at(idx+14*2+1);
        int frPerSecond = w16(inBytes, idx+15*2);
        int vidShift = w16(inBytes, idx+17*2);

        int msec = inBytes[idx8+vidShift+0];
        int sec = inBytes[idx8+vidShift+1];
        int min = inBytes[idx8+vidShift+2];
        int hours = inBytes[idx8+vidShift+3];
        int day = inBytes[idx8+vidShift+4] + 1;
        int month = inBytes[idx8+vidShift+5] + 1;
        int year = w16(inBytes, idx8+vidShift+3*2);
        int nv4 = inBytes[idx8+vidShift+8];
        int wavelets = inBytes[idx8+vidShift+9];
        int nv5koef = w16(inBytes, idx8+vidShift+10);
        int nv6shift = w16(inBytes, idx8+vidShift+12);

        //int vStart = idx8 + vidShift + 114*2;
        //int vidSize = frSize - vidShift - 114*2;
        int vStart = frSize - 31980;
        int vidSize = 31980;

        outBytes.reserve(vidSize);
        outBytes = inBytes.mid(vStart, vidSize);

        DecodeUrion(outBytes);
    }
    while(idx >=0);
    //frNum++;
}

uint16_t MainWindow::w16(QByteArray &ba, int idx)
{
    uint16_t *val = reinterpret_cast<uint16_t*>(&ba.data()[idx]);
    return *val;
    //uint16_t lb = (*val & 0xff);
    //uint16_t hb = (*val & 0xff00);
    //return (lb << 8) | (hb >> 8);
}













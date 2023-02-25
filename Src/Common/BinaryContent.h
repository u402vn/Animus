#ifndef BINARYCONTENT_H
#define BINARYCONTENT_H

#include <QByteArray>

class BinaryContent
{
private:
    QByteArray _content;
public:
    void clear();
    int size() const;
    QString toHex() const;
    const QByteArray toByteArray() const;
    BinaryContent &appendHEX(const QByteArray &hexEncode);
    BinaryContent &appendChar(const quint8 value);
    BinaryContent &appendWord(const quint16 value);
    BinaryContent &appendDWord(const quint32 value);
    BinaryContent &appendFloatRev(const float value);
    BinaryContent &appendData(const char *data, quint32 size);
    BinaryContent &appendCharCRC();
    BinaryContent &appendCharCRC_CCITT();
    BinaryContent &prependChar(const quint8 value);
    BinaryContent &byteStaffing(const quint8 fromChar, const quint16 toWord);
};

#endif // BINARYCONTENT_H

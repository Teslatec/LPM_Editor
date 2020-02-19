#include "test_text_buffer.h"
#include "lpm_structs.h"

#include <QDebug>
#include <QSerialPort>

TestTextBuffer::TestTextBuffer(const QString & fileName, size_t maxSize)
    : f(fileName)
{
    isErr = true;
    if(f.open(QFile::ReadOnly))
    {
        a = f.readAll();
        a.remove(0, 2);
        if(a.size() <= (int)maxSize)
        {
            size_t appSize = maxSize - a.size();
            a.append(appSize, 0);
            isErr = false;
            //qDebug() << a.toHex();
        }
        f.close();
    }
}

TestTextBuffer::~TestTextBuffer()
{
    isErr = true;

//    if(f.open(QFile::WriteOnly))
//    {
//        int zeroPos = a.size()-2;
//        for( ; ; zeroPos -= 2)
//        {
//            if( (a[zeroPos] != (char)0) || (a[zeroPos+1] != (char)0) )
//            {
//                zeroPos += 2;
//                break;
//            }
//        }

//        QByteArray tmp = a;
//        tmp.remove(zeroPos, a.size() - zeroPos);

//        tmp.insert(0, (char)0xfe);
//        tmp.insert(0, (char)0xff);

//        //qDebug() << tmp.size() << tmp.toHex();

//        f.write(tmp);
//        f.close();
//        isErr = false;
//    }

//    QSerialPort port("COM33");
//    if(port.open(QSerialPort::WriteOnly))
//    {
//        int zeroPos = a.size()-2;
//        for( ; ; zeroPos -= 2)
//        {
//            if( (a[zeroPos] != (char)0) || (a[zeroPos+1] != (char)0) )
//            {
//                zeroPos += 2;
//                break;
//            }
//        }

//        QByteArray tmp = a;
//        tmp.remove(zeroPos, a.size() - zeroPos);

//        qDebug() << "port open";

//        port.setBaudRate(115200);
//        port.write(tmp);
//        port.waitForBytesWritten();
//        port.close();
//    }
}

void TestTextBuffer::buffer(LPM_Buf * buf)
{
    buf->data = (uint8_t*)a.data();
    buf->size = a.size();
}

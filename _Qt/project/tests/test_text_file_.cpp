#include "test_text_file.h"

#include <QFile>
#include <QDebug>

static void write(LPM_File * f, const LPM_Buf * buf, size_t offset);
static void read(LPM_File * f, LPM_Buf * buf, size_t offset);
static void clear(LPM_File * f);


static const LPM_FileFxns fxns =
{
    .write = &write,
    .read  = &read,
    .clear = &clear
};


static inline TestTextBuffer * _buf(LPM_File * f);

void TestTextFile_init(TestTextFile * obj, TestTextBuffer * buf)
{
    obj->base =
    {
        .fxns    = &fxns,
        .maxSize = 100,
        .error   = LPM_NO_ERROR,
    };
    obj->buf = buf;
}

void write(LPM_File * f, const LPM_Buf * buf, size_t offset)
{
    _buf(f)->seek(offset);
    _buf(f)->read(buf->data, buf->size);
}

void read(LPM_File * f, LPM_Buf * buf, size_t offset)
{
    _buf(f)->seek(offset);
    _buf(f)->write(buf->data, buf->size);
}

void clear(LPM_File * f) { (void)f; }

TestTextBuffer * _buf(LPM_File * f)
{
    return ((TestTextFile*)f)->buf;
}

TestTextBuffer::TestTextBuffer(const QString & fileName, size_t maxSize_)
    : f(fileName)
    , maxSize(maxSize_)
    , currPos(0)
{
    isErr = true;
    if(f.open(QFile::ReadOnly))
    {
        a = f.readAll();
        a.remove(0, 2);
        if(a.size() <= maxSize)
        {
            size_t appSize = maxSize - a.size();
            a.append(appSize, 0);
            isErr = false;
            qDebug() << a.toHex();
        }
        f.close();
    }
}

TestTextBuffer::~TestTextBuffer()
{
    sync();
}

void TestTextBuffer::read(uint8_t * buf, size_t size)
{
    memcpy(buf, dt()+currPos, size);
}

void TestTextBuffer::write(const uint8_t * buf, size_t size)
{
    memcpy(dt()+currPos, buf, size);
}

uint8_t * TestTextBuffer::data()
{
    return dt() + currPos;
}

void TestTextBuffer::seek(size_t pos)
{
    currPos = pos;
}

size_t TestTextBuffer::pos()
{
    return currPos;
}

size_t TestTextBuffer::size()
{
    return maxSize;
}

void TestTextBuffer::sync()
{
    isErr = true;
    if(f.open(QFile::WriteOnly))
    {
        int zeroPos = a.size()-2;
        for( ; ; zeroPos -= 2)
        {
            if( (a[zeroPos] != (char)0) || (a[zeroPos+1] != (char)0) )
            {
                zeroPos += 2;
                break;
            }
        }

        QByteArray tmp = a;
        tmp.remove(zeroPos, a.size() - zeroPos);

        tmp.insert(0, (char)0xfe);
        tmp.insert(0, (char)0xff);

        f.write(tmp);
        f.close();
        isErr = false;
    }
}

bool TestTextBuffer::errorOccured()
{
    return isErr;
}

uint8_t *TestTextBuffer::dt()
{
    return (uint8_t*)a.data();
}

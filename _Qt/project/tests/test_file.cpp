#include "test_file.h"

static const int MAX_SIZE = 16384 * 16;

static void write(LPM_File * f, const LPM_Buf * buf, size_t offset);
static void read(LPM_File * f, LPM_Buf * buf, size_t offset);
static void clear(LPM_File * f);

static const LPM_FileFxns fxns =
{
    &write,
    &read,
    &clear
};

void TestFile_init(TestFile * f, TestFileImpl * impl)
{
    f->base = { &fxns , MAX_SIZE, LPM_NO_ERROR };
    f->impl = impl;
}

void write(LPM_File * f, const LPM_Buf * buf, size_t offset)
{
    ((TestFile*)f)->impl->write(*buf, offset);
}

void read(LPM_File * f, LPM_Buf * buf, size_t offset)
{
    ((TestFile*)f)->impl->read(*buf, offset);
}

void clear(LPM_File * f)
{}

// ---------------------------------------------------------------

#include <QFile>

void TestFileImpl::load(const QString & fileName)
{
    arr.clear();
    QFile f(fileName);
    if(f.open(QFile::ReadOnly))
    {
        arr = f.readAll();
        if(arr.size() < MAX_SIZE)
            arr.append(MAX_SIZE - arr.size(), (char)0xFF);
        f.close();
    }
    else
    {
        arr.append(MAX_SIZE, (char)0xFF);
    }
}

void TestFileImpl::save(const QString & fileName)
{
    QFile f(fileName);
    if(f.open(QFile::WriteOnly))
    {
        if(arr.size() > MAX_SIZE)
            arr.truncate(MAX_SIZE);
        f.write(arr);
        f.close();
    }
}

void TestFileImpl::write(const LPM_Buf & buf, size_t offset)
{
    memcpy(arr.data()+offset, buf.data, buf.size);
}

void TestFileImpl::read(LPM_Buf & buf, size_t offset)
{
    memcpy(buf.data, arr.data()+offset, buf.size);
}

void TestFileImpl::clear()
{}

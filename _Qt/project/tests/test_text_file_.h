#ifndef TEST_TEXT_FILE_H
#define TEST_TEXT_FILE_H

#include "lpm_file.h"

#include <QFile>

class TestTextBuffer;

struct TestTextFile
{
    LPM_File base;
    TestTextBuffer * buf;
};

void TestTextFile_init(TestTextFile * obj, TestTextBuffer * buf);

inline LPM_File * TestTextFile_file(TestTextFile * obj)
{
    return &obj->base;
}

class TestTextBuffer
{
public:
    TestTextBuffer(const QString & fileName, size_t maxSize_);
    ~TestTextBuffer();

    void read(uint8_t * buf, size_t size);
    void write(const uint8_t * buf, size_t size);
    uint8_t * data();
    void seek(size_t pos);
    size_t pos();
    size_t size();
    void sync();
    bool errorOccured();

private:
    QFile f;
    QByteArray a;
    size_t maxSize;
    bool isErr;
    size_t currPos;

    uint8_t * dt();
};

#endif // TEST_TEXT_FILE_H

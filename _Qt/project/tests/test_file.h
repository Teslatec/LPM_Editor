#ifndef TEST_FILE_H
#define TEST_FILE_H


#include "lpm_file.h"

#include <QByteArray>

class TestFileImpl
{
public:
    void load(const QString & fileName);
    void save(const QString & fileName);

    void write(const LPM_Buf & buf, size_t offset);
    void read(LPM_Buf & buf, size_t offset);
    void clear();

private:
    QByteArray arr;
};

struct TestFile
{
    LPM_File base;
    TestFileImpl * impl;
};

void TestFile_init(TestFile * f, TestFileImpl * impl);

inline LPM_File * TestFile_base(TestFile * f)
{
    return &f->base;
}


#endif // TEST_FILE_H

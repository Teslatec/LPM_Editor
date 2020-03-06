#ifndef TEST_TEXT_BUFFER_H
#define TEST_TEXT_BUFFER_H

#include <QFile>

struct LPM_Buf;

class TestTextBuffer
{
public:
    TestTextBuffer(const QString & fileName, size_t maxSize, bool saveChanges_);
    ~TestTextBuffer();

    void buffer(LPM_Buf * buf);

private:
    QFile f;
    QByteArray a;
    bool isErr;
    bool saveChanges;
};

#endif // TEST_TEXT_BUFFER_H

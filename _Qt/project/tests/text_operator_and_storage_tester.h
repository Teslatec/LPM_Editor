#ifndef TEXT_OPERATOR_AND_STORAGE_TESTER_H
#define TEXT_OPERATOR_AND_STORAGE_TESTER_H

#include <stddef.h>

class QString;
struct Unicode_Buf;

class TextOperatorAndStorageTester
{
public:

    void exec(const QString & logFileName, size_t maxSize, bool fullLog);

    void testStorage(const QString & logFileName);

private:
    QString testStorageCalcEndOfText();
    QString testStorageCalcEndOfTextStep(int textSize, int bufSize);

    QString testStorageAppend();
    QString testStorageAppendStep(int textSize, int appendSize, int bufSize);

    QString testStorageInsert();
    QString testStorageInsertStep(int textSize, int insSize, int insPos, int bufSize);

    QString testStorageReplace();
    QString testStorageReplaceStep(int textSize, int rplSize, int rplPos, int bufSize);

    void writeLog(QString & logData, const QString & logFileName);
    void fillUnicodeBuf(QString & text, Unicode_Buf & buf);
    QString prepareToLog(const QString & text);
    QString yesOrNo(bool condition);
    QString createText(int textSize, int bufSize, bool numbers);
    void correctText(QString & text);
};

#endif // TEXT_OPERATOR_AND_STORAGE_TESTER_H

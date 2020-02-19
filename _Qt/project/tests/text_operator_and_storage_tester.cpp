#include "text_operator_and_storage_tester.h"

#include <QDebug>
#include <QFile>
#include <QTextStream>
#include <QTextCodec>
#include <QVector>

extern "C" {
#include "lpm_text_storage.h"
}

const QChar textNullChr = QChar::Null;
const QChar logNullChr  = 0x25A3;

void TextOperatorAndStorageTester::exec( const QString & logFileName,
                                         size_t maxSize,
                                         bool fullLog )
{
    if(maxSize > 8)
        fullLog = false;
    const int bufSize = maxSize;
    int passedCount = 0, totalCount = 0;
    auto makeTest = [this, bufSize, &passedCount, &totalCount](
            int endOfText, int rmawrPos, int rmLen, int insLen)
    {
        //printf("%d %d %d %d ", endOfText, rmawrPos, rmLen, insLen);
        QString log;
        bool written;
        int cpos;
        int crml;
        if(rmawrPos < endOfText)
        {
            cpos = rmawrPos;
            crml = ((endOfText-cpos)<rmLen) ? (endOfText-cpos) : rmLen;
        }
        else
        {
            cpos = endOfText;
            crml = 0;
        }

        if(insLen <= crml)
            written = true;
        else
            written = (insLen - crml) <= bufSize - endOfText;

        QString src = createText(endOfText, endOfText, true);
        QString srcToLog = prepareToLog(createText(endOfText, bufSize, true));
        log += srcToLog + "\t";

        if(written)
        {
            if(endOfText >= rmawrPos)
                src.replace(rmawrPos, rmLen, createText(insLen, insLen, false));
            else
                src.append(createText(insLen, insLen, false));
        }

        if(src.size() < bufSize)
            src.append(QString(bufSize - src.size(), logNullChr));
        else
            src.truncate(bufSize);
        log += src + "\t";

        QString txtSrc = createText(endOfText, bufSize, true);
        QString txtIns = createText(insLen, insLen, false);
        Unicode_Buf bfrSrc, bfrIns;

        fillUnicodeBuf(txtSrc, bfrSrc);
        fillUnicodeBuf(txtIns, bfrIns);
        LPM_TextStorage op;
        LPM_TextStorage_init(&op, &bfrSrc);
        LPM_SelectionCursor rmar = { .pos = (size_t)rmawrPos, .len = (size_t)rmLen };
        bool writtenOp = LPM_TextStorage_replace(&op, &rmar, &bfrIns);

        txtSrc = prepareToLog(txtSrc);
        correctText(txtSrc);

        bool passed = txtSrc == src;
        if(passed)
            passedCount++;
        totalCount++;

        log += prepareToLog(txtSrc) + "\t";
        log += QString("Пройдено: ") + yesOrNo(passed) + "\t";
        log += QString::number(endOfText) + "\t";
        log += QString::number(rmawrPos) + "\t";
        log += QString::number(rmLen) + "\t";
        log += QString::number(insLen) + "\t";
        log += QString("Выполнено: ") + yesOrNo(written) + "\t" + yesOrNo(writtenOp);
        return log;
    };

    QString log("--- Начало тестов ---\n");
    for(int endOfText = 0; endOfText < bufSize+1; endOfText++)
        for(int rmawrPos = 0; rmawrPos < bufSize+2; rmawrPos++)
            for(int rmLen = 0; rmLen < bufSize+2; rmLen++)
                for(int insLen = 0; insLen < bufSize+2; insLen++)
                    if(fullLog)
                        log += makeTest(endOfText, rmawrPos, rmLen, insLen) + "\n";
                    else
                        log  = makeTest(endOfText, rmawrPos, rmLen, insLen) + "\n";

    log += QString("--- Конец теста ---\n");
    log += QString("Всего тестов ") + QString::number(totalCount) +
            ", пройдено: " + QString::number(passedCount) + "\n";
    writeLog(log, logFileName);
}

void TextOperatorAndStorageTester::testStorage(const QString & logFileName)
{
    QString src = createText(5, 5, true);
    QString mod = src;
    mod.replace(5, 1, createText(2,2,false));
    QString log;
    log += testStorageCalcEndOfText();
    log += testStorageAppend();
    log += testStorageInsert();
    log += testStorageReplace();
    // ...
    writeLog(log, logFileName);
}

QString TextOperatorAndStorageTester::testStorageCalcEndOfText()
{
    QString log = QString("Тест определения конца текста:\n");
    log += testStorageCalcEndOfTextStep(0,  10) + "\n";
    log += testStorageCalcEndOfTextStep(1,  10) + "\n";
    log += testStorageCalcEndOfTextStep(9,  10) + "\n";
    log += testStorageCalcEndOfTextStep(10, 10) + "\n";
    log += "\n";
    return log;
}

QString TextOperatorAndStorageTester::testStorageCalcEndOfTextStep(int textSize, int bufSize)
{
    QString text(bufSize, textNullChr);
    for(int i = 0; i < textSize; i++)
        text[i] = QChar('0'+i);
    TextBuffer strg;
    Unicode_Buf bfr;
    fillUnicodeBuf(text, bfr);
    TextBuffer_init(&strg, &bfr);
    size_t calcedEndOfText = TextBuffer_endOfText(&strg);
    size_t actualEndOfText = textSize;
    return QString("Текст: ") + prepareToLog(text) +
            QString(" Конец текста: ") + QString::number(calcedEndOfText) +
            "(" + QString::number(actualEndOfText) + ")" +
            QString(" Пройдено: ") + yesOrNo(actualEndOfText == calcedEndOfText);
}

QString TextOperatorAndStorageTester::testStorageAppend()
{
    const int bufSize = 20;
    QString log = QString("Тест функции append:\n");
    log += testStorageAppendStep(0, 0,         bufSize) + "\n";
    log += testStorageAppendStep(0, 1,         bufSize) + "\n";
    log += testStorageAppendStep(0, bufSize/2, bufSize) + "\n";
    log += testStorageAppendStep(0, bufSize-1, bufSize) + "\n";
    log += testStorageAppendStep(0, bufSize,   bufSize) + "\n";
    log += testStorageAppendStep(1, 0,         bufSize) + "\n";
    log += testStorageAppendStep(1, 1,         bufSize) + "\n";
    log += testStorageAppendStep(1, bufSize/2, bufSize) + "\n";
    log += testStorageAppendStep(1, bufSize-2, bufSize) + "\n";
    log += testStorageAppendStep(1, bufSize-1, bufSize) + "\n";
    log += testStorageAppendStep(bufSize/2, 0,           bufSize) + "\n";
    log += testStorageAppendStep(bufSize/2, 1,           bufSize) + "\n";
    log += testStorageAppendStep(bufSize/2, bufSize/4,   bufSize) + "\n";
    log += testStorageAppendStep(bufSize/2, bufSize/2-1, bufSize) + "\n";
    log += testStorageAppendStep(bufSize/2, bufSize/2,   bufSize) + "\n";
    log += "\n";
    return log;
}

QString TextOperatorAndStorageTester::testStorageAppendStep(int textSize, int appendSize, int bufSize)
{
    QString srcText = createText(textSize, bufSize, true);
    QString apndText = createText(appendSize, appendSize, false);
    TextBuffer strg;
    Unicode_Buf srcBfr;
    Unicode_Buf apndBfr;
    fillUnicodeBuf(srcText, srcBfr);
    fillUnicodeBuf(apndText, apndBfr);
    TextBuffer_init(&strg, &srcBfr);
    QString log = QString("До: ") + prepareToLog(srcText) + ", " +
            QString::number(textSize) + " ";
    TextBuffer_append(&strg, &apndBfr);
    log += QString("После: ") + prepareToLog(srcText) + ", " +
            QString::number(TextBuffer_endOfText(&strg)) + " ";
    log += QString("Добавлено: ") + QString::number(appendSize);
    return log;
}

QString TextOperatorAndStorageTester::testStorageInsert()
{
    const int bufSize = 20;
    QString log = QString("Тест функции insert:\n");
    log += testStorageInsertStep(1, 0, 0, bufSize) + "\n";
    log += testStorageInsertStep(1, 1, 0, bufSize) + "\n";
    log += testStorageInsertStep(1, bufSize/2, 0, bufSize) + "\n";
    log += testStorageInsertStep(1, bufSize-2, 0, bufSize) + "\n";
    log += testStorageInsertStep(1, bufSize-1, 0, bufSize) + "\n";

    log += testStorageInsertStep(2, 0, 0, bufSize) + "\n";
    log += testStorageInsertStep(2, 0, 1, bufSize) + "\n";
    log += testStorageInsertStep(2, 1, 0, bufSize) + "\n";
    log += testStorageInsertStep(2, 1, 1, bufSize) + "\n";
    log += testStorageInsertStep(2, bufSize/2, 0, bufSize) + "\n";
    log += testStorageInsertStep(2, bufSize/2, 1, bufSize) + "\n";
    log += testStorageInsertStep(2, bufSize-3, 0, bufSize) + "\n";
    log += testStorageInsertStep(2, bufSize-3, 1, bufSize) + "\n";
    log += testStorageInsertStep(2, bufSize-2, 0, bufSize) + "\n";
    log += testStorageInsertStep(2, bufSize-2, 1, bufSize) + "\n";

    log += testStorageInsertStep(bufSize/2, 0,           0,           bufSize) + "\n";
    log += testStorageInsertStep(bufSize/2, 0,           bufSize/4,   bufSize) + "\n";
    log += testStorageInsertStep(bufSize/2, 0,           bufSize/2-1, bufSize) + "\n";
    log += testStorageInsertStep(bufSize/2, 1,           0,           bufSize) + "\n";
    log += testStorageInsertStep(bufSize/2, 1,           bufSize/4,   bufSize) + "\n";
    log += testStorageInsertStep(bufSize/2, 1,           bufSize/2-1, bufSize) + "\n";
    log += testStorageInsertStep(bufSize/2, bufSize/2-1, 0,           bufSize) + "\n";
    log += testStorageInsertStep(bufSize/2, bufSize/2-1, bufSize/4,   bufSize) + "\n";
    log += testStorageInsertStep(bufSize/2, bufSize/2-1, bufSize/2-1, bufSize) + "\n";
    log += testStorageInsertStep(bufSize/2, bufSize/2,   0,           bufSize) + "\n";
    log += testStorageInsertStep(bufSize/2, bufSize/2,   bufSize/4,   bufSize) + "\n";
    log += testStorageInsertStep(bufSize/2, bufSize/2,   bufSize/2-1, bufSize) + "\n";

    log += testStorageInsertStep(bufSize-1, 0, 0,         bufSize) + "\n";
    log += testStorageInsertStep(bufSize-1, 0, bufSize/2, bufSize) + "\n";
    log += testStorageInsertStep(bufSize-1, 0, bufSize-2, bufSize) + "\n";
    log += testStorageInsertStep(bufSize-1, 1, 0,         bufSize) + "\n";
    log += testStorageInsertStep(bufSize-1, 1, bufSize/2, bufSize) + "\n";
    log += testStorageInsertStep(bufSize-1, 1, bufSize-2, bufSize) + "\n";

    log += "\n";
    return log;
}

QString TextOperatorAndStorageTester::testStorageInsertStep(int textSize, int insSize, int insPos, int bufSize)
{
    QString srcTxt = createText(textSize, bufSize, true);
    QString insTxt = createText(insSize, insSize, false);
    TextBuffer strg;
    Unicode_Buf srcBfr;
    Unicode_Buf insBfr;
    fillUnicodeBuf(srcTxt, srcBfr);
    fillUnicodeBuf(insTxt, insBfr);
    TextBuffer_init(&strg, &srcBfr);
    QString log = QString("До: ") + prepareToLog(srcTxt) + ", " +
            QString::number(textSize) + " ";
    TextBuffer_insert(&strg, &insBfr, insPos);
    log += QString("После: ") + prepareToLog(srcTxt) + ", " +
            QString::number(TextBuffer_endOfText(&strg)) + " ";
    log += QString("Вставлено: ") + QString::number(insSize);
    return log;
}

QString TextOperatorAndStorageTester::testStorageReplace()
{
    const int bufSize = 20;
    QString log = QString("Тест функции replace:\n");

    log += testStorageReplaceStep(1, 0, 0, bufSize) + "\n";
    log += testStorageReplaceStep(1, 1, 0, bufSize) + "\n";

    log += testStorageReplaceStep(2, 0, 0, bufSize) + "\n";
    log += testStorageReplaceStep(2, 1, 0, bufSize) + "\n";
    log += testStorageReplaceStep(2, 2, 0, bufSize) + "\n";
    log += testStorageReplaceStep(2, 0, 1, bufSize) + "\n";
    log += testStorageReplaceStep(2, 1, 1, bufSize) + "\n";

    log += testStorageReplaceStep(bufSize/2, 0,         0, bufSize) + "\n";
    log += testStorageReplaceStep(bufSize/2, 1,         0, bufSize) + "\n";
    log += testStorageReplaceStep(bufSize/2, bufSize/4, 0, bufSize) + "\n";
    log += testStorageReplaceStep(bufSize/2, bufSize/2, 0, bufSize) + "\n";
    log += testStorageReplaceStep(bufSize/2, 0,           1, bufSize) + "\n";
    log += testStorageReplaceStep(bufSize/2, 1,           1, bufSize) + "\n";
    log += testStorageReplaceStep(bufSize/2, bufSize/4,   1, bufSize) + "\n";
    log += testStorageReplaceStep(bufSize/2, bufSize/2-1, 1, bufSize) + "\n";
    log += testStorageReplaceStep(bufSize/2, 0,           bufSize/4, bufSize) + "\n";
    log += testStorageReplaceStep(bufSize/2, 1,           bufSize/4, bufSize) + "\n";
    log += testStorageReplaceStep(bufSize/2, bufSize/4,   bufSize/4, bufSize) + "\n";
    log += testStorageReplaceStep(bufSize/2, 0,   bufSize/2-1, bufSize) + "\n";
    log += testStorageReplaceStep(bufSize/2, 1,   bufSize/2-1, bufSize) + "\n";

    log += "\n";
    return log;
}

QString TextOperatorAndStorageTester::testStorageReplaceStep(int textSize, int rplSize, int rplPos, int bufSize)
{
    QString srcTxt = createText(textSize, bufSize,  true);
    QString rplTxt = createText(rplSize, rplSize, false);
    TextBuffer strg;
    Unicode_Buf srcBfr;
    Unicode_Buf rplBfr;
    fillUnicodeBuf(srcTxt, srcBfr);
    fillUnicodeBuf(rplTxt, rplBfr);
    TextBuffer_init(&strg, &srcBfr);
    QString log = QString("До: ") + prepareToLog(srcTxt) + ", " +
            QString::number(textSize) + " ";
    TextBuffer_replace(&strg, &rplBfr, rplPos);
    log += QString("После: ") + prepareToLog(srcTxt) + ", " +
            QString::number(TextBuffer_endOfText(&strg)) + " ";
    log += QString("Заменено: ") + QString::number(rplSize);
    return log;
}

void TextOperatorAndStorageTester::writeLog( QString & logData,
                                             const QString & logFileName)
{
    QFile f(logFileName);
    if(f.open(QFile::WriteOnly))
    {
        QTextStream s(&f);
        s.setCodec("UTF-16LE");
        s << logData;
        f.close();
    }
}

void TextOperatorAndStorageTester::fillUnicodeBuf(QString & text, Unicode_Buf & buf)
{
    buf.data = reinterpret_cast<unicode_t*>(text.data());
    buf.size = static_cast<size_t>(text.size());
}

QString TextOperatorAndStorageTester::prepareToLog(const QString & text)
{
    QString result = text;
    for(auto & sym : result)
        if(sym == textNullChr)
            sym = logNullChr;
    return result;
}

QString TextOperatorAndStorageTester::yesOrNo(bool condition)
{
    return condition ? QString("Да") : QString("Нет");
}

QString TextOperatorAndStorageTester::createText(int textSize, int bufSize, bool numbers)
{
    QString text(bufSize, textNullChr);
    for(int i = 0; i < textSize; i++)
        text[i] = numbers ? QChar('0'+i) : QChar('a'+i);
    return text;
}

void TextOperatorAndStorageTester::correctText(QString & text)
{
    bool clrFlag = false;
    for(auto & chr : text)
        if(clrFlag)
            chr = logNullChr;
        else if(chr == logNullChr)
            clrFlag = true;
}


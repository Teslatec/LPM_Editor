#include "text_operator_and_storage_tester.h"

#include <QDebug>
#include <QFile>
#include <QTextStream>
#include <QTextCodec>
#include <QVector>

extern "C" {
#include "text_editor_text_operator.h"
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
        TextEditorTextOperator op;
        TextEditorTextOperator_init(&op, &bfrSrc);
        LPM_SelectionCursor rmar = { .pos = rmawrPos, .len = rmLen };
        bool writtenOp = TextEditorTextOperator_removeAndWrite(&op, &rmar, &bfrIns);

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
    TextEditorTextStorage strg;
    Unicode_Buf bfr;
    fillUnicodeBuf(text, bfr);
    TextEditorTextStorage_init(&strg, &bfr);
    size_t calcedEndOfText = TextEditorTextStorage_endOfText(&strg);
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
    TextEditorTextStorage strg;
    Unicode_Buf srcBfr;
    Unicode_Buf apndBfr;
    fillUnicodeBuf(srcText, srcBfr);
    fillUnicodeBuf(apndText, apndBfr);
    TextEditorTextStorage_init(&strg, &srcBfr);
    QString log = QString("До: ") + prepareToLog(srcText) + ", " +
            QString::number(textSize) + " ";
    TextEditorTextStorage_append(&strg, &apndBfr);
    log += QString("После: ") + prepareToLog(srcText) + ", " +
            QString::number(TextEditorTextStorage_endOfText(&strg)) + " ";
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
    TextEditorTextStorage strg;
    Unicode_Buf srcBfr;
    Unicode_Buf insBfr;
    fillUnicodeBuf(srcTxt, srcBfr);
    fillUnicodeBuf(insTxt, insBfr);
    TextEditorTextStorage_init(&strg, &srcBfr);
    QString log = QString("До: ") + prepareToLog(srcTxt) + ", " +
            QString::number(textSize) + " ";
    TextEditorTextStorage_insert(&strg, &insBfr, insPos);
    log += QString("После: ") + prepareToLog(srcTxt) + ", " +
            QString::number(TextEditorTextStorage_endOfText(&strg)) + " ";
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
    TextEditorTextStorage strg;
    Unicode_Buf srcBfr;
    Unicode_Buf rplBfr;
    fillUnicodeBuf(srcTxt, srcBfr);
    fillUnicodeBuf(rplTxt, rplBfr);
    TextEditorTextStorage_init(&strg, &srcBfr);
    QString log = QString("До: ") + prepareToLog(srcTxt) + ", " +
            QString::number(textSize) + " ";
    TextEditorTextStorage_replace(&strg, &rplBfr, rplPos);
    log += QString("После: ") + prepareToLog(srcTxt) + ", " +
            QString::number(TextEditorTextStorage_endOfText(&strg)) + " ";
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


//static void writeLog(const QString & logFileName, const QString & logData);
//static QString readSrcText(const QString & srcTextFileName, size_t maxSize);
//static QString testWrite( const QString & inputText,
//                          const QString & writeText,
//                          size_t rmawrPos, size_t rmLen );
//static void initObjectsToWrite( TextEditorTextOperator & op,
//                                Unicode_Buf & ttw,
//                                LPM_SelectionCursor & ra, size_t rmPos, size_t rmLen,
//                                QString & inputText,
//                                const QString & writeText );
//static size_t calcEndOfText(const QString & srcText);
//static QString toOultine(const QString & text);

//TextOperatorAndStorageTester::TextOperatorAndStorageTester()
//{}

//void TextOperatorAndStorageTester::exec( const QString & logFileName,
//                                         const QString & srcTextFileName,
//                                         size_t maxSize )
//{
//    auto srcText = readSrcText(srcTextFileName, maxSize);
//    size_t endOfText = calcEndOfText(srcText);
//    QString result = toOultine(srcText)  + QString(" : Исходный текст\n");
//    result += QString("--- ---\n");
//    result += testWrite(srcText, QString(""), 0,           0);
//    result += testWrite(srcText, QString(""), 1,           0);
//    result += testWrite(srcText, QString(""), endOfText-1, 0);
//    result += testWrite(srcText, QString(""), endOfText,   0);
//    result += testWrite(srcText, QString(""), endOfText+1, 0);
//    result += testWrite(srcText, QString(""), maxSize,     0);
//    result += testWrite(srcText, QString(""), maxSize+1,   0);
//    result += QString("--- ---\n");
//    result += testWrite(srcText, QString(1, '!'), 0, 0);
//    result += testWrite(srcText, QString(maxSize-endOfText-1, QChar('!')), 0, 0);
//    result += testWrite(srcText, QString(maxSize-endOfText,   QChar('!')), 0, 0);
//    result += testWrite(srcText, QString(maxSize-endOfText+1, QChar('!')), 0, 0);
//    result += testWrite(srcText, QString(endOfText-1, QChar('!')), 0, 0);
//    result += testWrite(srcText, QString(endOfText,   QChar('!')), 0, 0);
//    result += testWrite(srcText, QString(endOfText+1, QChar('!')), 0, 0);
//    result += testWrite(srcText, QString(maxSize-1, QChar('!')), 0, 0);
//    result += testWrite(srcText, QString(maxSize,   QChar('!')), 0, 0);
//    result += testWrite(srcText, QString(maxSize+1, QChar('!')), 0, 0);
//    result += testWrite(srcText, QString(1, '!'), endOfText, 0);
//    result += testWrite(srcText, QString(2, '!'), endOfText-1, 0);
//    result += testWrite(srcText, QString(2, '!'), endOfText, 0);
//    result += testWrite(srcText, QString(3, '!'), 2, 1);
//    result += testWrite(srcText, QString(3, '!'), endOfText-1, 1);
//    writeLog(logFileName, result);
//}


//QString readSrcText(const QString & srcTextFileName, size_t maxSize)
//{
//    QString result;
//    QFile f(srcTextFileName);
//    size_t textSize = 0;
//    if(f.open(QFile::ReadOnly))
//    {
//        auto arr = f.readAll();
//        arr.remove(0, 2);
//        textSize = arr.size() / 2;
//        result = QString::fromUtf16(reinterpret_cast<const unicode_t*>(arr.data()), textSize);
//        f.close();
//    }

//    if(textSize < maxSize)
//    {
//        QString ns(maxSize-textSize, QChar(QChar::Null));
//        result.append(ns);
//    }
//    else
//        result.truncate(maxSize);
//    return result;
//}


//void writeLog(const QString & logFileName, const QString & logData)
//{
//    QFile f(logFileName);
//    if(f.open(QFile::WriteOnly))
//    {
//        QTextStream s(&f);
//        s.setCodec("UTF-16LE");
//        s << QString("Начало теста"); endl(s);
//        s << logData; endl(s);
//        s << QString("Конец теста"); endl(s);
//        f.close();
//    }
//}

//QString testWrite( const QString & inputText,
//                   const QString & writeText,
//                   size_t rmawrPos, size_t rmLen )
//{
//    QString inputCopy = inputText;
//    TextEditorTextOperator op;
//    LPM_SelectionCursor ra;
//    Unicode_Buf ttw;

//    initObjectsToWrite(op, ttw, ra, rmawrPos, rmLen, inputCopy, writeText);
//    bool ok = TextEditorTextOperator_removeAndWrite(&op, &ra, &ttw);
//    QString result = toOultine(inputCopy);
//    result += QString(" : Позиция: ") + QString::number(rmawrPos);
//    result += QString("\tРазмер: ") + QString::number(rmLen);
//    result += QString("\tВыполнено: ") + (ok ? QString("да") : QString("нет"));
//    result += QString("\tТекст записи: ") + (writeText.size() == 0 ? QString("---") : writeText);
//    result += QString("\n");

//    return result;
//}

//void initObjectsToWrite( TextEditorTextOperator & op,
//                         Unicode_Buf & ttw,
//                         LPM_SelectionCursor & ra,
//                         size_t rmPos, size_t rmLen,
//                         QString & inputText,
//                         const QString & writeText )
//{
//    Unicode_Buf it =
//    {
//        it.data = reinterpret_cast<unicode_t*>(inputText.data()),
//        it.size = inputText.size()
//    };

//    TextEditorTextOperator_init(&op, &it);
//    ttw.data = const_cast<unicode_t*>(reinterpret_cast<const unicode_t*>(writeText.data()));
//    ttw.size = writeText.size();
//    ra.pos = rmPos;
//    ra.len = rmLen;
//}

//size_t calcEndOfText(const QString & srcText)
//{
//    TextEditorTextStorage ts;
//    Unicode_Buf tb =
//    {
//        .data = const_cast<unicode_t*>(reinterpret_cast<const unicode_t*>(srcText.data())),
//        .size = static_cast<size_t>(srcText.size())
//    };
//    TextEditorTextStorage_init(&ts, &tb);
//    return TextEditorTextStorage_endOfText(&ts);
//}

//QString toOultine(const QString & text)
//{
//    QString result = text;
//    for(auto & sym : result)
//        if(sym == QChar(QChar::Null))
//            sym = nullSym;
//    return result;
//}


//static void initUnicodeBuf(Unicode_Buf & buf, QString & text);
//static QByteArray readSrcText(const QString & srcTextFileName, size_t bufSize);
//static void initObjectsToWrite(TextEditorTextOperator & op,
//                         Unicode_Buf & buf,
//                         QByteArray & srcText );

//static QString testReadEndOfTextPosition(TextEditorTextOperator & op, size_t & endOfText);
//static QString testRead( const QString & testHeader,
//                         TextEditorTextOperator & op,
//                         size_t pos,
//                         size_t len );

//static QString testRemoveAndWrite(const QString & inputText,
//                                   size_t rmnwrPos,
//                                   size_t rmLen,
//                                   const QString & writeText );


//void initUnicodeBuf(Unicode_Buf & buf, QString & text)
//{
//    buf.data = reinterpret_cast<unicode_t*>(text.data()),
//    buf.size = text.size();
//}

//QByteArray readSrcText(const QString & srcTextFileName, size_t bufSize)
//{
//    bufSize *= 2;
//    QByteArray result;
//    QFile f(srcTextFileName);
//    if(f.open(QFile::ReadOnly))
//    {
//        result = f.readAll();
//        result.remove(0, 2);
//        if(result.size()*2 > bufSize)
//            result.truncate(bufSize);
//        else
//            result.append(bufSize - result.size(), '\0');
//        f.close();
//    }
//    else
//    {
//        result.append(bufSize, '\0');
//    }
//    return result;
//}


//void initObjectsToWrite( TextEditorTextOperator & op,
//                         Unicode_Buf & inputTextBuffer,
//                         Unicode_Buf & writeTextBuffer,
//                         QString& inputText,
//                         QByteArray  )
//{
//    buf->data = reinterpret_cast<unicode_t*>(srcText.data());
//    buf->size = srcText.size()/2;
//    TextEditorTextOperator_init(op, buf);
//}

//QString testReadEndOfTextPosition(TextEditorTextOperator & op, size_t & endOfText)
//{
//    endOfText = TextEditorTextStorage_endOfText(&op.textStorage);
//    return QString("\nЧтение позиции конца текста: ") +
//            QString::number(endOfText) + "\n";
//}

//QString testRead( const QString & testHeader,
//                 TextEditorTextOperator & op,
//                 size_t pos,
//                 size_t len )
//{
//    QString arr(len, QChar::LineSeparator);
//    Unicode_Buf buf =
//    {
//        .data = reinterpret_cast<unicode_t*>(arr.data()),
//        .size = len
//    };
//    TextEditorTextOperator_read(&op, pos, &buf);
//    return "\n" + testHeader + "\n" +
//            QString("Прочитано: ") + QString::number(buf.size) + QString(" cимволов\n") +
//            QString("Текст:\n") + arr + QString("\nКонец текста\n");
//}


//QString testRemoveAndWrite( const QString & inputText,
//                            size_t rmnwrPos, size_t rmLen,
//                            const QString & writeText )
//{
//    QString inputTextCopy = inputText;
//    Unicode_Buf inputTextBuf;
//    Unicode_Buf writeTextBuf;
//    TextEditorTextOperator op;

//    LPM_SelectionCursor sc = { .pos = rmnwrPos, .len = rmLen };
//    initObjectsToWrite(op, inputTextBuf, inputTextCopy);
//    initUnicodeBuf(writeTextBuf, writeText);
//    TextEditorTextOperator_removeAndWrite(&op, &sc, &writeTextBuf);

//    return QString("\nЗапись и удаление: ") +
//}


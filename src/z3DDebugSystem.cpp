/* This file is a part of Zavod3D engine project.
It's licensed unser the MIT license (see "License.txt" for details).*/

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdarg.h>
#include <io.h>

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include "z3DDebugSystem.h"

#ifdef _MSC_VER
#define va_copy(d, s) d = s
#endif

#ifndef Z3D_LOGFILENAME
#define Z3D_LOGFILENAME "z3DDebugLog.txt"
#endif // Z3D_LOGFILENAME

namespace z3D
{

// Флаг релизного режима
static bool s_releaseMode = false;

// Статус функции Log
static bool s_fLogOn = true;

// ---------------------------------------------------------------------------
// Файл лога
// ---------------------------------------------------------------------------
class LogFile
{
public:
    LogFile();
    ~LogFile();

    operator bool() {return f_ != 0;}
    FILE* f_;

};



LogFile::LogFile() :
    f_(0)
{
    static bool fLog = false;
    static const char* logFilename = Z3D_LOGFILENAME;
    if (!fLog)
    {
        // Попытка открыть файл лога для чтения
        if (access(logFilename, 0) == 0)
        {
            // Удаляем файл, если он существует
            remove(logFilename);
        }
        fLog = true;
    }
    // Пишем в лог
    //
    f_ = fopen(logFilename, "a");

}



LogFile::~LogFile()
{
    if (f_ != 0)
    {
        fclose(f_);
        f_ = 0;
    }
}


// ---------------------------------------------------------------------------
// Буфер для форматированной строки
// ---------------------------------------------------------------------------
class DebugFormattedBuf
{
public:
    DebugFormattedBuf();
    ~DebugFormattedBuf();

    // Освободить память, занимаемую буфером
    void Clear();

    // Увеличить размер буфера
    char* Resize(size_t n);

    // Добавить запись в буфер с использованием строки формата и списка аргументов
    void AppendFormatted(size_t from, const char* fmt, va_list ap);

    char* Buf() { return buf_; }
    size_t Size() { return bufSize_; }
    static DebugFormattedBuf& Instance() { return instance_; }

private:
    int Resize(size_t from, const char* fmt);

    char* buf_;         // буфер
    size_t bufSize_;    // размер буфера в байтах

    static DebugFormattedBuf instance_; // статический экземпляр класса
};

DebugFormattedBuf DebugFormattedBuf::instance_;



DebugFormattedBuf::DebugFormattedBuf() :
    buf_(0),
    bufSize_(0)
{
    Resize(2); // резервируем 2 байта для начала
}



DebugFormattedBuf::~DebugFormattedBuf()
{
    Clear();
}



void DebugFormattedBuf::Clear()
{
    if (buf_ != 0)
    {
        free (buf_);
        buf_ = 0;
        bufSize_ = 0;
    }
}



char* DebugFormattedBuf::Resize(size_t n)
{
    if (n <= bufSize_) return buf_;

    char* buf = (char*) malloc(n);
    if (buf != 0)
    {
        strncpy(buf, buf_, bufSize_);
        buf[bufSize_] = 0;
        Clear();
        buf_ = buf;
        bufSize_ = n;
        return buf_;
    }
    return 0;
}



int DebugFormattedBuf::Resize(size_t from, const char* fmt)
{
    int c = 0;
    int len = strlen(fmt);
    for (int i = 0; i < len; i++)
    {
        if (fmt[i] == '%') c++;
    }
    int reqSize = len + 1 + c * 260;
    Resize(from + reqSize);
    if (Size() < from + reqSize)
    {
        return -1;
    }
    return reqSize;
}


void DebugFormattedBuf::AppendFormatted(size_t from, const char* fmt, va_list ap)
{
    if (fmt == 0) return;
    if (strcmp(fmt, "") == 0) return;
    // вычисляем размер буфера под форматированную строку и резервируем его
    int reqSize = Resize(from, fmt);
    if (reqSize == -1) return;

    // форматируем буфер
    vsnprintf(buf_ + from, reqSize, fmt, ap);
}



// ---------------------------------------------------------------------------
// Реализация DebugTool
// ---------------------------------------------------------------------------

const bool DebugTool::kPopMsg = true;
const bool DebugTool::kPopNoMsg = false;

char DebugTool::typeStrHeader_[32];
char DebugTool::typeStrFooter_[32];
char DebugTool::detailsWord_[32];
char DebugTool::filenameLine_[270];



size_t DebugTool::Prepare(eMsgType msgType,
                       const char* exp,
                       const char* filename,
                       int iLine,
                       const char* fmt)
{
    if (msgType == kInfo)
    {
        sprintf(typeStrHeader_, "[INFO]:");
        typeStrFooter_[0] = 0;
    }
    else if (msgType == kTrace)
    {
        sprintf(typeStrHeader_, "[TRACE]:");
        sprintf(typeStrFooter_, "in file");
    }
    else if (msgType == kAssert)
    {
        sprintf(typeStrHeader_, "[ASSERT]:");
        if (s_releaseMode )
            typeStrFooter_[0] = 0;
        else
            sprintf(typeStrFooter_, "failed in file");
    }
    else if (msgType == kDebug)
    {
        sprintf(typeStrHeader_, "[DEBUG]:");
        if (s_releaseMode )
            typeStrFooter_[0] = 0;
        else
            sprintf(typeStrFooter_, "in file");
    }
    else if (msgType == kWarning)
    {
        sprintf(typeStrHeader_, "[WARN]:");
        if (s_releaseMode )
            typeStrFooter_[0] = 0;
        else
            sprintf(typeStrFooter_, "reported on file");
    }
    else if (msgType == kError)
    {
        sprintf(typeStrHeader_, "[ERROR]:");
        if (s_releaseMode )
            typeStrFooter_[0] = 0;
        else
            sprintf(typeStrFooter_, "found in file");
    }
    else if (msgType == kFatal)
    {
        sprintf(typeStrHeader_, "[FATAL]:");
        if (s_releaseMode )
            typeStrFooter_[0] = 0;
        else
            sprintf(typeStrFooter_, "found in file");
    }

    if (!s_releaseMode && filename != 0)
        sprintf(filenameLine_, "\"%s\"(%d)", filename, iLine);
    else
        strcpy (filenameLine_, "");


    if (fmt != 0 && strcmp(fmt, "") != 0)
        strcpy(detailsWord_, "Details: ");
    else
        strcpy(detailsWord_, "");

    size_t reqSize = 0;
    if (s_releaseMode)
    {
        reqSize = strlen (typeStrHeader_)
                + strlen("x")
                + 1
                + 16;
    }
    else
    {
        reqSize = strlen (typeStrHeader_)
                + strlen(exp)
                + strlen(typeStrFooter_)
                + strlen(filenameLine_)
                + strlen(detailsWord_)
                + strlen("x (x) x x. \nx\n")
                + 1
                + 16;
    }


    DebugFormattedBuf::Instance().Resize(reqSize);
    if (DebugFormattedBuf::Instance().Size() < reqSize)
    {
        return 0;
    }

    if (s_releaseMode)
    {
        sprintf(DebugFormattedBuf::Instance().Buf(),
            "%s",
            typeStrHeader_);
    }
    else
    {
        sprintf(DebugFormattedBuf::Instance().Buf(),
            "%s (%s) %s %s. \n%s\n",
            typeStrHeader_,
            exp,
            typeStrFooter_,
            filenameLine_,
            detailsWord_);
    }

    reqSize = strlen(DebugFormattedBuf::Instance().Buf());
    return reqSize;
}



void DebugTool::OutputMsg(eMsgType msgType,
                        const char* exp,
                        const char* filename,
                        int iLine,
                        bool fPopMsg,
                        const char* fmt, ...)
{
    size_t from = Prepare(msgType, exp, filename, iLine, fmt);

    va_list ap;
    va_start(ap, fmt);
    DebugFormattedBuf::Instance().AppendFormatted(from, fmt, ap);
    va_end(ap);


    Log("%s\n", DebugFormattedBuf::Instance().Buf());
    printf("%s\n", DebugFormattedBuf::Instance().Buf());


    if (fPopMsg)
    {
        char msgBoxName[32];
        DWORD msgBoxFlags = MB_TOPMOST | MB_OK;

        if (msgType == kInfo)
        {
            sprintf(msgBoxName, "INFO");
            msgBoxFlags |= MB_ICONINFORMATION;
        }
        else if (msgType == kTrace)
        {
            sprintf(msgBoxName, "TRACE");
            msgBoxFlags |= MB_ICONINFORMATION;
        }
        else if (msgType == kAssert)
        {
            sprintf(msgBoxName, "ASSERT:");

        }
        else if (msgType == kDebug)
        {
            sprintf(msgBoxName, "DEBUG");

        }
        else if (msgType == kWarning)
        {
            sprintf(msgBoxName, "WARN");
            msgBoxFlags |= MB_ICONINFORMATION;
        }
        else if (msgType == kError)
        {
            sprintf(msgBoxName, "ERROR");
            msgBoxFlags |= MB_ICONERROR;
        }
        else if (msgType == kFatal)
        {
            sprintf(msgBoxName, "FATAL");
            msgBoxFlags |= MB_ICONERROR;
        }

        MessageBoxA(0, DebugFormattedBuf::Instance().Buf(), msgBoxName, msgBoxFlags);
    }

    if (msgType == kFatal)
    {
        exit (EXIT_FAILURE);
    }
}



void DebugTool::SetReleaseMode()
{
    s_releaseMode = true;
}

void DebugTool::SetLogState(bool fOn)
{
    s_fLogOn = fOn;
}

void Log(const char* fmt, ...)
{
    if(s_fLogOn)
    {
        LogFile lf;
        if (lf)
        {
            va_list ap;
            va_start(ap, fmt);
            vfprintf(lf.f_, fmt, ap);
            va_end(ap);
        }
    }
}

} // end of z3D


#include "screenshots.h"

#include <QApplication>
#include <QFontDialog>
#include <QDateTime>
#include <Windows.h>
#include <QFileInfo>
#include <DbgHelp.h>
#pragma comment(lib, "dbghelp.lib")

LONG ApplicationCrashHandler(EXCEPTION_POINTERS *pException)
{
    QString str = QString("./dmp/mini-%1.dmp")
                      .arg(QDateTime::currentDateTime().toString("yyyy-MM-dd hh_mm_ss"));
    qInfo() << "slog start write to" << str;
    const QChar *path = str.unicode();
    HANDLE hFile = CreateFile((LPCWSTR)path, // L"mini.dmp",
                              GENERIC_READ | GENERIC_WRITE,
                              FILE_SHARE_WRITE,
                              NULL,
                              CREATE_ALWAYS,
                              FILE_ATTRIBUTE_NORMAL,
                              NULL);

    if (hFile == INVALID_HANDLE_VALUE) {
        qInfo() << "slog Create dump failed";
        return EXCEPTION_EXECUTE_HANDLER;
    }

    MINIDUMP_EXCEPTION_INFORMATION mdei;
    mdei.ThreadId = GetCurrentThreadId();
    mdei.ExceptionPointers = pException;
    mdei.ClientPointers = NULL;
    MINIDUMP_CALLBACK_INFORMATION mci;
    mci.CallbackRoutine = NULL;
    mci.CallbackParam = 0;

    // 数组越界的情况下 dump可能会生成失败
    MiniDumpWriteDump(
        GetCurrentProcess(), GetCurrentProcessId(), hFile, MiniDumpNormal, &mdei, NULL, &mci);

    CloseHandle(hFile);

    EXCEPTION_RECORD *record = pException->ExceptionRecord;
    qInfo() << "  errCode: " << QString::number(record->ExceptionCode, 16)
            << ", errAddr: " << QString::number((uint)record->ExceptionAddress, 16)
            << ", errFlag: " << QString::number(record->ExceptionFlags, 16)
            << ", errPara: " << QString::number(record->NumberParameters, 16);

    qInfo() << "slog creat dump " << (QFileInfo::exists(str) ? "success" : "failed");

    return EXCEPTION_EXECUTE_HANDLER;
}

int main(int argc, char *argv[])
{

    SetUnhandledExceptionFilter(ApplicationCrashHandler);

    QApplication a(argc, argv);

    ScreenShots w;
    w.showRaise();
    return a.exec();
}

// file: tlog.h

#ifndef TLOG_H
#define TLOG_H

#include <QCoreApplication>
#include <QTextStream>
#include <QtGui>
#include <QFile>
#include <QDir>

#include <tchar.h>
#include <stdio.h>
#include <stdarg.h>
#include <windows.h>

using namespace std;

// Имя лог файла
#define LOGNAME	    _T("log.txt")


// Коды сообщений
#define L_FATAL     (0)
#define L_ERROR     (1)
#define L_WARNING   (2)
#define L_MESSAGE   (3)
#define L_INFO      (4)
#define L_NOTICE    (5)
#define L_TRACE     (6)
#define L_DEBUG     (7)


class TLog
{
public:
	// Methods
	TLog();
	~TLog();

	void Write(int code, const QString message, ...);
	void Write(int code, const QString message, va_list args);

private:
	// Methods
	TLog(LPCTSTR lpszFileName);
	void DestroyCS();
	void WriteInternal(int code, const QString message, va_list args);

	CHAR *absPath(CHAR *fileName);

	// Attributes
	static TCHAR fname[MAX_PATH + 1];
	char szLevel [16];
	char szTime  [32];
	char szMessage [512];
	QTextCodec *codec;

	static CRITICAL_SECTION m_CS;
	static TLog *m_log;
	static long m_nRefCount;

	char argv1[FILENAME_MAX + 32];	// lenght of command line (file+path (260+32))
};

#endif //TLOG_H

// file: tlog.cpp

#include "tlog.h"

// Static members
TLog *TLog::m_log = NULL;
long  TLog::m_nRefCount = 0;
TCHAR TLog::fname[MAX_PATH + 1];
CRITICAL_SECTION TLog::m_CS;

// Публичный конструктор
TLog::TLog()
{
	// If there is no Logger instance, create it
	if (m_log == NULL)
		m_log = new TLog((LPCTSTR)LOGNAME);

	// Increase Logger reference counter
	if (m_log != NULL)
		m_nRefCount++;

	codec = QTextCodec::codecForName("UTF8");
	QTextCodec::setCodecForTr(codec);
	QTextCodec::setCodecForCStrings(codec);
	QTextCodec::setCodecForLocale(codec);
}

// Деструктор
TLog::~TLog()
{
	// If this Logger reference was last, destroy instance
	if (--m_nRefCount == 0)
	{
		if (m_log != NULL)
		{
			m_log->DestroyCS();
			delete m_log;
			m_log = NULL;
		}
	}
}

// Приватный конструктор
TLog::TLog (LPCTSTR lpszFileName)
{
	_tcsncpy(fname, lpszFileName, MAX_PATH);
	::InitializeCriticalSection(&m_CS);
}
// Удаление критической секции
void TLog::DestroyCS()
{
	::DeleteCriticalSection(&m_CS);
}

void TLog::Write(int code, const QString message, ...)
{
	if (m_log != NULL)
	{
		// Извлечение перечня агрументов
		va_list args;
		va_start(args, message);
		// Запись сообщения в файл
		m_log->WriteInternal(code, message, args);
		va_end(args);
	}
}

void TLog::Write(int code, const QString message, va_list args)
{
	if (m_log != NULL)
	// Запись сообщения в файл
	m_log->WriteInternal(code, message, args);
}

void TLog::WriteInternal(int code, const QString message, va_list args)
{
	::EnterCriticalSection(&m_CS);

	QString directoryPath = QFileInfo( QCoreApplication::applicationFilePath() ).absolutePath()  + "/logs";

	if (!(QDir(directoryPath).exists()==true))
	{
		QDir().mkdir(directoryPath);
	}

	switch(code){
	case 0:
		sprintf(szLevel, "FATAL:   ");
		break;
	case 1:
		sprintf(szLevel, "ERROR:   ");
		break;
	case 2:
		sprintf(szLevel, "WARNING: ");
		break;
	case 3:
		sprintf(szLevel, "MESSAGE: ");
		break;
	case 4:
		sprintf(szLevel, "INFO:    ");
		break;
	case 5:
		sprintf(szLevel, "NOTICE:  ");
		break;
	case 6:
		sprintf(szLevel, "TRACE:   ");
		break;
	case 7:
		sprintf(szLevel, "DEBUG:   ");
		break;
	default:
		sprintf(szLevel, "DEBUG:   ");
		break;
	}

	vsprintf(szMessage, message.toAscii(), args);

	QFile file(directoryPath + "/" + QDateTime::currentDateTime().toString("yyyy.MM.dd") + ".log");
	if(file.open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text))
	{
		QTextStream stream(&file);
		stream.setCodec ("CP1251");
		stream << QDateTime::currentDateTime().toString("yyyy.MM.dd | hh:mm:ss.zzz ") + szLevel + szMessage + "\n";
	}
	file.close();

	::LeaveCriticalSection(&m_CS);
}

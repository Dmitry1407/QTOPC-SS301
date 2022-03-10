#include <QtGui/QApplication>
#include <stdio.h>
#include <stdlib.h>

#include <objbase.h>
#include <unknwn.h>
#include <windows.h>
#include <initguid.h>

#include "forms/mainwindow.h"
#include "src/main.h"

#define INITGUID					// Initialize OLE constants
#define ECL_SID "QTOPC.SS301"		// identificator of OPC server

wchar_t wargv0[FILENAME_MAX + 32];	// lenght of command line (file+path (260+32))
char argv0[FILENAME_MAX + 32];		// lenght of command line (file+path (260+32))


int main(int argc, char *argv[])
{
	const char eClsidName [] = ECL_SID;
	const char eProgID [] = ECL_SID;

	char *cp;
	DWORD objid=::GetModuleFileNameW(NULL, wargv0, sizeof(wargv0));
	WideCharToMultiByte(CP_ACP, 0, wargv0, -1, argv0, 300, NULL, NULL);
	if(objid==0 || objid+50 > sizeof(argv0)) return 0;

	if (!QSystemTrayIcon::isSystemTrayAvailable())
	{
		 QMessageBox::critical(0, QObject::tr("Systray"),
							   QObject::tr("I couldn't detect any system tray "
										   "on this system."));
	 return 1;
	}
	QApplication::setQuitOnLastWindowClosed(false);

	QApplication app(argc, argv);

	// Проверка ключей, полученных из командной строки
	cp = argv[1];
	if(cp)
	{
		int finish = 1;				// flag of comlection
		if (strstr(cp, "/r"))		// Ключ регистрации сервера
		{
			if (loServerRegister(&GID_QT_OPC_Server, eProgID, eClsidName, argv0, 0))
			{
				QMessageBox::critical(0, QObject::tr(driverName), QObject::tr("Registration Failed!"));
				//m_log.Write(L_ERROR, "Ошибка регистрации сервера");
			}
			else
			{
				QMessageBox::information(0, QObject::tr(driverName), QObject::tr(driverName) + QObject::tr(" Server\nRegistration Ok!"));
				//m_log.Write(L_INFO, "Сервер зарегистрирован");
			}
		}
		else if (strstr(cp, "/u"))	// Ключ удаления регистрации сервера
		{
			if (loServerUnregister(&GID_QT_OPC_Server, eClsidName))
			{
				QMessageBox::critical(0, QObject::tr(driverName), QObject::tr("UnRegistration Failed!"));
				//m_log.Write(L_ERROR, "Ошибка удаления регистрации сервера");
			}
			else
			{
				QMessageBox::information(0, QObject::tr(driverName), QObject::tr(driverName) + QObject::tr(" Server\nUnregistered!"));
				//m_log.Write(L_INFO, "Регистрация сервера удалена");
			}
		}
		// Справка
		else if (strstr(cp, "/?"))
			QMessageBox::information(0, QObject::tr(driverName), QObject::trUtf8("Используйте:\n"
																					"Ключ /r для регистрации сервера.\n"
																					"Ключ /u для удаления регистрации.\n"
																					"Ключ /? для вызова данной справки."));
		else
		{
			//m_log.Write(L_INFO, "Неизвестный ключ запуска программы проигнорирован");
			finish = 0;		// nehren delat
		}
		if (finish)
		{
			return 0;
		}
	}

	MainWindow w;
	w.show();

	return app.exec();
}

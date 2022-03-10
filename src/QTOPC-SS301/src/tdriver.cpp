#include <QtGui>

#include "src/tdriver.h"

using namespace std;

loService *my_service;
static const loVendorInfo vendor = { 0, 1, 1, 0, "QTOPC-SS301 Server" };	// OPC vendor info (Major/Minor/Build/Reserv)
static void a_server_finished(VOID*, loService*, loClient*);				// OnServer finish his work
static int OPCstatus = OPC_STATUS_RUNNING;

// Ведение логов
TLog m_log;

WCHAR wargv1[FILENAME_MAX + 32];	// lenght of command line (file+path (260+32))
char  argv1[FILENAME_MAX + 32];		// lenght of command line (file+path (260+32))

// Теги
static CRITICAL_SECTION lk_values;          // защита ti[] от совместного доступа
static loTagId		ti[TAGS_NUM_MAX];		// идентификатор тега
static char        *tn[TAGS_NUM_MAX];       // имя тега
static loTagValue	tv[TAGS_NUM_MAX];		// значение тега

#include "src/opc_main.h"
#include "src/serv_main.h"

TDriver::TDriver()
{
	Tag  = new TagInfo [TAGS_IN_DEVICE];
	Tags = new TagInfo **[PORT_NUM_MAX];
	for (int i=0; i<PORT_NUM_MAX; i++)
	{
		Tags[i] = new TagInfo * [DEV_NUM_MAX];
		for (int j=0; j<DEV_NUM_MAX; j++)
		{
			Tags[i][j] = new TagInfo[TAGS_IN_DEVICE];
		}
	}

	Tag[0]  = (TagInfo){"KI",		"Коэффициент KI",							VT_R4,	52,	"",	"",	0};
	Tag[1]  = (TagInfo){"KU",		"Коэффициент KU",							VT_R4,	52,	"",	"",	0};
	Tag[2]  = (TagInfo){"Kpr",		"Коэффициент Kpr",							VT_I4,	24,	"",	"",	0};
	Tag[3]  = (TagInfo){"Ke",		"Коэффициент Ke",							VT_I4,	24,	"",	"",	0};
	Tag[4]  = (TagInfo){"AEp",		"Суммарная накопленная энергия",			VT_R4,	3,	"",	"",	0};
	Tag[5]  = (TagInfo){"AEo",		"Суммарная накопленная энергия",			VT_R4,	3,	"",	"",	0};
	Tag[6]  = (TagInfo){"REp",		"Суммарная накопленная энергия",			VT_R4,	3,	"",	"",	0};
	Tag[7]  = (TagInfo){"REo",		"Суммарная накопленная энергия",			VT_R4,	3,	"",	"",	0};
	Tag[8]  = (TagInfo){"AEp_1",	"Суммарная накопленная энергия пр. месяц",	VT_R4,	3,	"",	"",	0},
	Tag[9]  = (TagInfo){"AEo_1",	"Суммарная накопленная энергия пр. месяц",	VT_R4,	3,	"",	"",	0},
	Tag[10] = (TagInfo){"REp_1",	"Суммарная накопленная энергия пр. месяц",	VT_R4,	3,	"",	"",	0},
	Tag[11] = (TagInfo){"REo_1",	"Суммарная накопленная энергия пр. месяц",	VT_R4,	3,	"",	"",	0},
	Tag[12] = (TagInfo){"AEp3m",	"3-мин. срез энергии",						VT_R4,	5,	"",	"",	0};
	Tag[13] = (TagInfo){"AEo3m",	"3-мин. срез энергии",						VT_R4,	5,	"",	"",	0};
	Tag[14] = (TagInfo){"REp3m",	"3-мин. срез энергии",						VT_R4,	5,	"",	"",	0};
	Tag[15] = (TagInfo){"REo3m",	"3-мин. срез энергии",						VT_R4,	5,	"",	"",	0};
	Tag[16] = (TagInfo){"Pa",		"Мгновенная активная мощность фазы A",		VT_R4,	8,	"",	"",	0};
	Tag[17] = (TagInfo){"Pb",		"Мгновенная активная мощность фазы B",		VT_R4,	8,	"",	"",	0};
	Tag[18] = (TagInfo){"Pc",		"Мгновенная активная мощность фазы C",		VT_R4,	8,	"",	"",	0};
	Tag[19] = (TagInfo){"Qa",		"Мгновенная реактивная мощность фазы A",	VT_R4,	9,	"",	"",	0};
	Tag[20] = (TagInfo){"Qb",		"Мгновенная реактивная мощность фазы B",	VT_R4,	9,	"",	"",	0};
	Tag[21] = (TagInfo){"Qc",		"Мгновенная реактивная мощность фазы C",	VT_R4,	9,	"",	"",	0};
	Tag[22] = (TagInfo){"Ua",		"Напряжение фазы A",						VT_R4,	10,	"",	"",	0};
	Tag[23] = (TagInfo){"Ub",		"Напряжение фазы B",						VT_R4,	10,	"",	"",	0};
	Tag[24] = (TagInfo){"Uc",		"Напряжение фазы C",						VT_R4,	10,	"",	"",	0};
	Tag[25] = (TagInfo){"Ia",		"Ток фазы A",								VT_R4,	11,	"",	"",	0};
	Tag[26] = (TagInfo){"Ib",		"Ток фазы B",								VT_R4,	11,	"",	"",	0};
	Tag[27] = (TagInfo){"Ic",		"Ток фазы C",								VT_R4,	11,	"",	"",	0};
	Tag[28] = (TagInfo){"F",		"Частота сети",								VT_R4,	13,	"",	"",	0};
	Tag[29] = (TagInfo){"Serial",	"Серийный номер",							VT_BSTR,18,	"",	"",	0};
	Tag[30] = (TagInfo){"Connect",	"Состояние связи",							VT_BOOL,0,	"",	"",	0};

    tags_num = 0; tTotal = 0;
    InitializeCriticalSection(&lk_values);

	// Связывание слотов с сигналами потоков
	for (int i = 0; i<PORT_NUM_MAX; ++i)
	{
		QObject::connect(&poll[i], SIGNAL(pollDone(int)), this, SLOT(pDone(int)));
	}

	/*QObject::connect(&poll[0], SIGNAL(pollDone(int)), this, SLOT(pDone(int)));
	QObject::connect(&poll[1], SIGNAL(pollDone(int)), this, SLOT(pDone(int)));
	QObject::connect(&poll[2], SIGNAL(pollDone(int)), this, SLOT(pDone(int)));
	QObject::connect(&poll[3], SIGNAL(pollDone(int)), this, SLOT(pDone(int)));
	QObject::connect(&poll[4], SIGNAL(pollDone(int)), this, SLOT(pDone(int)));
	QObject::connect(&poll[5], SIGNAL(pollDone(int)), this, SLOT(pDone(int)));
	QObject::connect(&poll[6], SIGNAL(pollDone(int)), this, SLOT(pDone(int)));
	QObject::connect(&poll[7], SIGNAL(pollDone(int)), this, SLOT(pDone(int)));*/
}

TDriver::~TDriver()
{
	loSetState(my_service, 0, loOP_SHUTDOWN, OPC_STATUS_SUSPENDED, NULL);
	my_CF.serverRemove();

	INT ecode = loServiceDestroy(my_service);
	my_service = 0;
	DeleteCriticalSection (&lk_values);						// destroy CS

	for (int i=0; i<PORT_NUM_MAX; i++)
	{
		for (int j=0; j<DEV_NUM_MAX; j++)
		{
			delete []Tags[i][j];
		}
		delete []Tags[i];
	}
	delete []Tags;
	delete []Tag;
}


loTrid readTags(const loCaller *, unsigned  count, loTagPair taglist[],
				VARIANT   values[],	WORD      qualities[],	FILETIME  stamps[],
				HRESULT   errs[],	HRESULT  *master_err,	HRESULT  *master_qual,
				const VARTYPE vtype[],	LCID lcid)
{
    return loDR_STORED;
}


INT writeTags(const loCaller *ca, unsigned count, loTagPair taglist[],
				VARIANT values[], HRESULT error[], HRESULT *master, LCID lcid)
{
	return loDW_TOCACHE;
}


VOID activation_monitor(const loCaller *ca, INT count, loTagPair *til) {}


//=============================================================================
// Инициализация драйвера
//=============================================================================
UINT TDriver::initDriver()
{
	DWORD objid=::GetModuleFileNameW(NULL, wargv1, sizeof(wargv1));
	WideCharToMultiByte(CP_ACP, 0, wargv1, -1, argv1, 300, NULL, NULL);
	if(objid==0 || objid+50 > sizeof(argv1)) return 0;

	loDriver ld;		// structure of driver description
	LONG ecode;		// error code
	tTotal = TAGS_NUM_MAX;		// total tag quantity

	HRESULT hres;


	// Инициализация драйвера
	m_log.Write(L_INFO, "Запуск серсвиса драйвера...");
	if (my_service) {
		//m_log.Write(L_ERROR, "Драйвер уже инициализирован");
		return 0;
    }
    memset(&ld, 0, sizeof(ld));

	ld.ldRefreshRate = update;			// Время обновления кэша драйвера
	ld.ldRefreshRate_min = update/2;	// Минимальное ремя обновления кэша драйвера
	ld.ldWriteTags = writeTags;			// pointer to function write tag
	ld.ldReadTags = readTags;				// pointer to function read tag
	ld.ldSubscribe = activation_monitor;	// callback of tag activity
	ld.ldFlags = loDF_IGNCASE;			// ignore case
	ld.ldBranchSep = '/';						// hierarchial branch separator

	ecode = loServiceCreate(&my_service, &ld, tTotal);		//	creating loService
	if (ecode) return 1;									// error to create service
	m_log.Write(L_INFO, "Сервис драйвера запущен");


	// Инициализация библиотеки COM
	m_log.Write(L_INFO, "Инициализация библиотеки COM...");
	//if (FAILED(CoInitializeEx(NULL, COINIT_MULTITHREADED)))

	hres = CoInitializeEx(NULL, COINIT_MULTITHREADED);

	if (hres == RPC_E_CHANGED_MODE) {
		OleUninitialize();
		hres =  CoInitializeEx(0, COINIT_MULTITHREADED);
		//hres =  CoInitializeEx(0, COINIT_APARTMENTTHREADED);
	}

	if (FAILED(hres))
	{
		// Инициализация библиотеки COM
		m_log.Write(L_ERROR, "Ошибка инициализации библиотеки COM. Завершение работы...");
		return 0;
	}

	if (0)
	{	// open sockets and attempt connect to servers
		CoUninitialize();		// Closes the COM library on the current thread
		return 0;
	}

	initTags();

	m_log.Write(L_INFO, "Регистрация объекта COM...");
	if (FAILED(CoRegisterClassObject(GID_QT_OPC_Server, &my_CF,
					 CLSCTX_LOCAL_SERVER|CLSCTX_REMOTE_SERVER|CLSCTX_INPROC_SERVER,
					 REGCLS_MULTIPLEUSE, &objid)))
	{
		m_log.Write(L_ERROR, "Ошибка регистрации объекта COM. Завершение работы...");
		return 0;
	}

	m_log.Write(L_INFO, "Объект COM зарегистрирован");

	Sleep(1000);
	my_CF.Release();		// avoid locking by CoRegisterClassObject()

	if (OPCstatus!=OPC_STATUS_RUNNING)	// ???? maybe Status changed and OPC not currently running??
	{
		while(my_CF.in_use()) Sleep(1000);	// wait
		cleanup_all(objid);
		destroyDriver();
		return 0;
	}

	EnterCriticalSection(&lk_values);
	// Инициализация COM-портов =================================================
	for (int np=0; np<numPorts; ++np)
	{
		if (portType[np] == 0 && portEnable[np] == true)
		{
			// Установка таймаутов
			COMMTIMEOUTS timeouts;

			BYTE bDataBits = data_bits[np] & 0xFF;

			m_log.Write(L_INFO, "Открытие порта COM %d...", numPort[np]);
			//OutConsole(L_INFO, Rus("Открытие порта COM %d..."), numPort[np]);

			if (!poll[np].port->Open(numPort[np], (DWORD)baud[np], (CSerialPort::Parity) parity[np], bDataBits, (CSerialPort::StopBits) stop_bits[np], CSerialPort::NoFlowControl, FALSE))
			{
				m_log.Write(L_ERROR, "Ошибка открытия порта COM %d", numPort[np]);
				//OutConsole (L_ERROR, Rus("Ошибка открытия порта COM %d"), numPort[np]);
				for (int nd=0; nd<numDevices[np]; ++nd)
				{
					for (UINT nt=0; nt<TAGS_IN_DEVICE; ++nt){
							Tags[np][nd][nt].status = 0;
					}
				}
				continue;
			}

			timeouts.ReadIntervalTimeout = 64;	//MAXDWORD
			timeouts.ReadTotalTimeoutMultiplier = 1;
			timeouts.ReadTotalTimeoutConstant = 80;//rtimeout[np];
			timeouts.WriteTotalTimeoutMultiplier = 1;
			timeouts.WriteTotalTimeoutConstant = 25;//wtimeout[np];

			poll[np].port->SetTimeouts(timeouts);

			m_log.Write(L_INFO, "Порт COM %d открыт", numPort[np]);
			//OutConsole(L_INFO, Rus("Порт COM %d открыт"), numPort[np]);
		}
	}
	LeaveCriticalSection(&lk_values);

	// Запуск из клиента
	//if (init_tags()) return 1;

	//QMessageBox::information(0, QObject::tr("QTOPC"), QObject::trUtf8("Драйвер инициализирован!"));

	return 0;
}

UINT TDriver::destroyDriver()
{
	if (my_service)
	{
		loSetState(my_service, 0, loOP_SHUTDOWN, OPC_STATUS_SUSPENDED, NULL);
		//my_CF.serverRemove();

		INT ecode = loServiceDestroy(my_service);
		m_log.Write (L_INFO, "Работа сервиса завершена (%d)", ecode);
		//DeleteCriticalSection (&lk_values);						// destroy CS
		my_service = 0;
	}

	// Закрытие портов
	for (int np=0; np < numPorts; ++np)
	{
		poll[np].bRun = false;
		// Закрытие последоватльных портов
		if (portType[np] == 0 && portEnable[np] == true)
		{
			poll[np].port->Close();
			m_log.Write(L_INFO, "Порт COM %d закрыт", numPort[np]);
			//OutConsole(L_INFO, Rus("Порт COM %d закрыт"), numPort[np]);
		}
		// Закрытие TCP портов
		if (portType[np] == 1 && portEnable[np] == true)
		{
			closesocket(poll[np].soc);
			::WSACleanup();
			m_log.Write(L_INFO, "Порт %d закрыт", portIP[np]);
			//OutConsole(L_INFO, Rus("Порт %d закрыт"), numPort[np]);
		}
		poll[np].exit();
	}

	m_log.Write(L_INFO, "Сервис драйвера остановлен\n");
	//QMessageBox::information(0, QObject::tr("QTOPC"), QObject::trUtf8("Работа драйвера завершена!"));
	return	1;
}


//=============================================================================
// Инициализация тегов
//=============================================================================
INT TDriver::initTags()
{
	m_log.Write(L_INFO, "Инициализация тегов...");
	//OutConsole(L_INFO, Rus("Инициализация тегов..."));

	FILETIME ft;		//  64-bit value representing the number of 100-ns intervals since January 1,1601
	UINT rights=0;      // tag type (read/write)
	UINT ecode, numTag=0;
	tags_num = 0;
	WCHAR buf[DATALEN_MAX];

	GetSystemTimeAsFileTime(&ft);	// retrieves the current system date and time
	EnterCriticalSection(&lk_values);
	LCID lcid = MAKELCID(0x0409, SORT_DEFAULT); // This macro creates a locale identifier from a language identifier. Specifies how dates, times, and currencies are formatted

	// Проход по портам
	for (int np=0; np < numPorts; ++np)
	{
		// Проход по устройствам
		for (int nd=0; nd<numDevices[np]; ++nd)
		{
			if ((portEnable[np] == true) && (deviceEnable[np][nd] == true))
			{
				rights = OPC_READABLE;
				// Проход по тегам
				for (int nt=0; nt<TAGS_IN_DEVICE; ++nt)
				{
					tn[numTag] = new char[DATALEN_MAX];	// reserve memory for massive
					if(portType[np] == 0) sprintf (tn[numTag],"COM_%d/DEVICE_%0.2d/%s", numPort[np], numDevice[np][nd], Tag[nt].symbol);
					if(portType[np] == 1) sprintf (tn[numTag],"PORT_%d/DEVICE_%0.2d/%s", portIP[np], numDevice[np][nd], Tag[nt].symbol);
					VariantInit(&tv[numTag].tvValue);
					// function maps a character string to a wide-character (Unicode) string
					MultiByteToWideChar(CP_ACP, 0, tn[numTag], strlen(tn[numTag])+1,	buf, sizeof(buf)/sizeof(buf[0]));
					if (Tag[nt].type == VT_I4)
					{
						V_R4(&tv[numTag].tvValue) = 0;
						Tags[np][nd][nt].type = VT_I4;
						V_VT(&tv[numTag].tvValue) = VT_I4;
					}
					if (Tag[nt].type == VT_R4)
					{
						V_R4(&tv[numTag].tvValue) = 0.0;
						Tags[np][nd][nt].type = VT_R4;
						V_VT(&tv[numTag].tvValue) = VT_R4;
					}
					if (Tag[nt].type == VT_BOOL)
					{
						V_BOOL(&tv[numTag].tvValue) = false;
						Tags[np][nd][nt].type = VT_BOOL;
						V_VT(&tv[numTag].tvValue) = VT_BOOL;
					}
					if (Tag[nt].type == VT_BSTR)
					{
						V_BSTR(&tv[numTag].tvValue) = SysAllocString(L"");
						Tags[np][nd][nt].type = VT_BSTR;
						V_VT(&tv[numTag].tvValue) = VT_BSTR;
					}

					ecode = loAddRealTag_aW(my_service,				// actual service context
											&ti[numTag],			// returned TagId
											(loRealTag)(numTag+1),	// != 0 driver's key
											buf,					// tag name
											0,						// loTF_ Flags
											rights,					// OPC access rights
											&tv[numTag].tvValue,	// type and value for conversion checks
											0, 0);					// Analog EUtype: from 0 (unknown) to 0% (overload)
					tv[numTag].tvTi = ti[numTag];
					tv[numTag].tvState.tsTime = ft;
					tv[numTag].tvState.tsError = S_OK;
					tv[numTag].tvState.tsQuality = OPC_QUALITY_NOT_CONNECTED;

	//				printf ("loAddRealTag(%s) = %u (t=%d)\n", Rus(tn[numTag]), ti[numTag], Tags[numTag].type);
	//				m_log.Write(L_DEBUG, "Тег [%d][%d][%d] добавлен", np, nd, nt);

					sprintf (Tags[np][nd][nt].name,"%s",tn[numTag]);
					poll[np].device[nd].tags[nt]=numTag;
					numTag++; tags_num++; poll[np].device[nd].channels++;
				}
			}
		}
	}
    LeaveCriticalSection(&lk_values);

	for (UINT i=0; i<numTag; ++i) delete tn[i];

	if(ecode)
	{
		m_log.Write(L_ERROR, "Ошибка инициализации тегов err=%d", ecode);
		return -1;
	}

	m_log.Write(L_INFO, "Теги инициализированы");
	//OutConsole(L_INFO, Rus("Теги инициализированы"));

	return 0;
}


//=============================================================================
// Запуск опроса устройств
//=============================================================================
void TDriver::poll_device()
{
	for (int np=0; np<numPorts; ++np)
	{
		if (portEnable[np])
		{
			for (int nd=0; nd<numDevices[np]; ++nd)
			{
				poll[np].deviceEnable[nd] = deviceEnable[np][nd];
				poll[np].numDevice[nd] = numDevice[np][nd];
				poll[np].timeCorrectEnable[nd] = timeCorrectEnable[np][nd];
				poll[np].timeShiftMax[nd] = timeShiftMax[np][nd];
			}
			poll[np].name = "Thread " + QString::number(np);
			poll[np].period = period[np];
			poll[np].np = np;

			if (portType[np] == 0) poll[np].numPort = numPort[np];
			if (portType[np] == 1) poll[np].numPort = portIP[np];

			poll[np].numDevices = numDevices[np];
			poll[np].portType = portType[np];
			strcpy (poll[np].ipAddress, addressIP[np].toAscii());
			poll[np].timeoutTCP = timeoutTCP[np];

			poll[np].bRun = true;

			poll[np].start();
		}
	}
}


//=============================================================================
// Обновление кеша OPC драйвера
//=============================================================================
void TDriver::pDone(int np)
{
	FILETIME ft, lft;
	SYSTEMTIME st;
	VARTYPE tvVt;
	int poll_err=0, ci;

	EnterCriticalSection(&lk_values);
	GetSystemTimeAsFileTime (&ft);
	FileTimeToLocalFileTime(&ft, &lft);
	FileTimeToSystemTime(&lft, &st);

	for (int nd=0; nd<numDevices[np]; ++nd)
	{
		if ((portEnable[np] == true) && (deviceEnable[np][nd] == true))
		{
			for (int nt=0; nt<TAGS_IN_DEVICE; ++nt)
			{
				Tags[np][nd][nt].status = poll[np].device[nd].tagStatus[nt];
				strcpy(Tags[np][nd][nt].value, poll[np].device[nd].tagValue[nt]);
				sprintf (Tags[np][nd][nt].timestamp, "%04d.%02d.%02d  %02d:%02d:%02d.%03d",
						 st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond, st.wMilliseconds);

				ci = poll[np].device[nd].tags[nt];

				tvVt = tv[ci].tvValue.vt;
				VariantClear(&tv[ci].tvValue);

				if (Tags[np][nd][nt].status == 0) strcpy(Tags[np][nd][nt].value, "0") ;

				if (Tags[np][nd][nt].type == VT_R4)
				{
					char *stopstring;
					V_R4(&tv[ci].tvValue) = (FLOAT) strtod(Tags[np][nd][nt].value, &stopstring);
				}

				if (Tags[np][nd][nt].type == VT_I4)
				{
					char *stopstring;
					V_I4(&tv[ci].tvValue) = (INT) strtod(Tags[np][nd][nt].value, &stopstring);
				}

				if (Tags[np][nd][nt].type == VT_BOOL)
				{
					char *stopstring;
					V_BOOL(&tv[ci].tvValue) = (BOOL) strtod(Tags[np][nd][nt].value, &stopstring);
				}

				if (Tags[np][nd][nt].type == VT_BSTR)
				{
					WCHAR buf[64];

					MultiByteToWideChar (CP_ACP,                    // ANSI code page
										0,  // flags
										(LPCSTR)Tags[np][nd][nt].value,     // points to the character string to be converted
										strlen(Tags[np][nd][nt].value)+1,   // size in bytes of the string pointed to
										buf,                        // Points to a buffer that receives the translated string
										sizeof(buf)/sizeof(buf[0]));// function maps a character string to a wide-character (Unicode) string

					V_BSTR(&tv[ci].tvValue) = SysAllocString(buf);
				}

				V_VT(&tv[ci].tvValue) = tvVt;
				if (Tags[np][nd][nt].status == 0) tv[ci].tvState.tsQuality = OPC_QUALITY_UNCERTAIN;
				else tv[ci].tvState.tsQuality = OPC_QUALITY_GOOD;
				tv[ci].tvState.tsTime = ft;
			}
		}

		poll_err = 0;
	}

	GetSystemTimeAsFileTime (&ft);
	loCacheUpdate(my_service, tags_num, tv, &ft);

//	m_log.Write(L_INFO, "Тегов обновлено: %d", tags_num);

	LeaveCriticalSection(&lk_values);

//	if (!my_CF.in_use()) driverRun = false;

	emit updateTableWidget();
}



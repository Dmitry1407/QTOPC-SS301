//#include <QDebug>
#include <QMutex>
#include <windows.h>

CRITICAL_SECTION lk_values;
#define TIMEOUT_TCP 2000; // msec

#include "tpoll.h"

tpoll::tpoll()
{
	port = new CSerialPort;
	device = new TDevice[DEV_NUM_MAX];

	// Параметры TCP/IP
	soc = INVALID_SOCKET;
	strcpy (ipAddress, "255.255.255.255");
}


tpoll::~tpoll()
{
	delete []device;
}


void tpoll::run()
{
	//qDebug() << this->name;
	//FILETIME ft;
	DWORD start, cycle;
	//UINT tags_per_dev=sizeof(Tag)/sizeof(TagInfo);
	int rs, rtc, iResult=1, poll_err=0, conn_err=0, tryRead=0;

	QMutex mutex;

	for (int nd=0; nd<numDevices; ++nd)
	{\
		backCorrectTime[nd] = QTime::currentTime();
	}

	while (bRun)
	{
		start = GetTickCount();

		// Открытие порта
		if (portType == 1)
		{
			mutex.lock();
			iResult = TCPconnect();
			mutex.unlock();
		}

		for (int nd=0; nd<numDevices; ++nd)
		{
			if (deviceEnable[nd])
			{
				//qDebug() << "Response Port: " << numPort << ", Device: " << numDevice[nd];
				//m_log.Write(L_INFO, "Опрос Port: %d, Device: %d", numPort, numDevice[nd]);

				if ((portType == 1 && iResult > 0) || portType == 0)
				{
					// Корректировка времени
					if (timeCorrectEnable[nd])
					{
						rtc = timeCorrect(nd);
						rs = rtc;
					}

					// Чтение коэффициентов KI KU
					if (bRun)
					{
						//qDebug() << "Read KI KU";
						rs = this->readParam (nd, 0, 52, 0);
						if (rs == -2) {
							++tryRead;
							if (tryRead > 1) goto NextDevice;
						}
						if (rs == -1) {
								conn_err=1;
						}
						if (rs == 0){
							m_log.Write(L_ERROR, "Port: %d, Device: %d. Ошибка чтения коэффициентов KI, KU", numPort, numDevice[nd]);
						}
						if (rs <= 0){
							device[nd].tagStatus[0] = 0;
							device[nd].tagStatus[1] = 0;
						}
					}

					// Чтение коэффициентов Kpr Ke
					if (bRun)
					{
						//qDebug() << "Read Kpr Ke";
						rs = this->readParam (nd, 2, 24, 0);
						if (rs == -2) {
							++tryRead;
							if (tryRead > 1) goto NextDevice;
						}
						if (rs == -1) {
								conn_err=1;
						}
						if (rs == 0){
							m_log.Write(L_ERROR, "Port: %d, Device: %d. Ошибка чтения коэффициентов Kpr, Ke", numPort, numDevice[nd]);
							//OutConsole(L_ERROR, Rus("Порт %d, Устройство %d. Ошибка чтения коэффициентов Kpr, Ke"), numPort[np], device[np][nd].numDevice);
						}
						if (rs <= 0){
							device[nd].tagStatus[2] = 0;
							device[nd].tagStatus[3] = 0;
						}
					}

					// Чтение суммарной накопленной энергии
					if (bRun)
					{
						//qDebug() << "Read SUM ENG";
						rs = this->readParam (nd, 4, 3, 0);
						if (rs == -2) {
							++tryRead;
							if (tryRead > 1) goto NextDevice;
						}
						if (rs == -1) {
								conn_err=1;
						}
						if (rs == 0){
							m_log.Write(L_ERROR, "Port: %d, Device: %d. Ошибка чтения приращения энергии за месяц", numPort, numDevice[nd]);
							//OutConsole(L_ERROR, Rus("Порт %d, Устройство %d. Ошибка чтения приращения энергии за месяц"), numPort[np], device[np][nd].numDevice);
						}
						if (rs <= 0){
							device[nd].tagStatus[4] = 0;
							device[nd].tagStatus[5] = 0;
							device[nd].tagStatus[6] = 0;
							device[nd].tagStatus[7] = 0;
						}
					}

					// Чтение приращения энергии за предыдущий месяц
					if (bRun)
					{
						rs = this->readParam (nd, 8, 3, -1);
						if (rs == -2) {
							++tryRead;
							if (tryRead > 1) goto NextDevice;
						}
						if (rs == -1) {
								conn_err=1;
						}
						if (rs == 0){
							m_log.Write(L_ERROR, "Port: %d, Device: %d. Ошибка чтения приращения энергии за пред. месяц", numPort, numDevice[nd]);
							//OutConsole(L_ERROR, Rus("Порт %d, Устройство %d. Ошибка чтения приращения энергии за пред. месяц"), numPort[np], device[np][nd].numDevice);
						}
						if (rs <= 0){
							device[nd].tagStatus[8] = 0;
							device[nd].tagStatus[9] = 0;
							device[nd].tagStatus[10] = 0;
							device[nd].tagStatus[11] = 0;
						}
					}

					// Чтение 3мин. мощности
					if (bRun)
					{
						//qDebug() << "Read 3MIN POWER";
						rs = this->readParam (nd, 12, 5, 0);
						if (rs == -2) {
							++tryRead;
							if (tryRead > 1) goto NextDevice;
						}
						if (rs == -1) {
								conn_err=1;
						}
						if (rs == 0){
							m_log.Write(L_ERROR, "Port: %d, Device: %d. Ошибка чтения 3мин. мощности", numPort, numDevice[nd]);
							//OutConsole(L_ERROR, Rus("Порт %d, Устройство %d. Ошибка чтения 3мин. мощности"), numPort[np], device[np][nd].numDevice);
						}
						if (rs <= 0){
							device[nd].tagStatus[12] = 0;
							device[nd].tagStatus[13] = 0;
							device[nd].tagStatus[14] = 0;
							device[nd].tagStatus[15] = 0;
						}
					}

					// Чтение мгновенной активной мощности
					if (bRun)
					{
						//qDebug() << "Read CURR ACT POWER";
						rs = this->readParam (nd, 16, 8, 0);
						if (rs == -2) {
							++tryRead;
							if (tryRead > 1) goto NextDevice;
						}
						if (rs == -1) {
								conn_err=1;
						}
						if (rs == 0){
							m_log.Write(L_ERROR, "Port: %d, Device: %d. Ошибка чтения мгновенной активной мощности", numPort, numDevice[nd]);
							//OutConsole(L_ERROR, Rus("Порт %d, Устройство %d. Ошибка чтения мгновенной активной мощности"), numPort[np], device[np][nd].numDevice);
						}
						if (rs <= 0){
							device[nd].tagStatus[16] = 0;
							device[nd].tagStatus[17] = 0;
							device[nd].tagStatus[18] = 0;
						}
					}

					// Чтение мгновенной реактивной мощности
					if (bRun)
					{
						//qDebug() << "Read CURR REACT POWER";
						rs = this->readParam (nd, 19, 9, 0);
						if (rs == -2) {
							++tryRead;
							if (tryRead > 1) goto NextDevice;
						}
						if (rs == -1) {
								conn_err=1;
						}
						if (rs == 0){
							m_log.Write(L_ERROR, "Port: %d, Device: %d. Ошибка чтения мгновенной реактивной мощности", numPort, numDevice[nd]);
							//OutConsole(L_ERROR, Rus("Порт %d, Устройство %d. Ошибка чтения мгновенной реактивной мощности"), numPort[np], device[np][nd].numDevice);
						}
						if (rs <= 0){
							device[nd].tagStatus[19] = 0;
							device[nd].tagStatus[20] = 0;
							device[nd].tagStatus[21] = 0;
						}
					}

					// Чтение показаний напряжения
					if (bRun)
					{
						//qDebug() << "Read U";
						rs = this->readParam (nd, 22, 10, 0);
						if (rs == -2) {
							++tryRead;
							if (tryRead > 1) goto NextDevice;
						}
						if (rs == -1) {
								conn_err=1;
						}
						if (rs == 0){
							m_log.Write(L_ERROR, "Port: %d, Device: %d. Ошибка чтения показаний напряжения", numPort, numDevice[nd]);
							//OutConsole(L_ERROR, Rus("Порт %d, Устройство %d. Ошибка чтения показаний напряжения"), numPort[np], device[np][nd].numDevice);
						}
						if (rs <= 0){
							device[nd].tagStatus[22] = 0;
							device[nd].tagStatus[23] = 0;
							device[nd].tagStatus[24] = 0;
						}
					}

					// Чтение показаний тока
					if (bRun)
					{
						//qDebug() << "Read I";
						rs = this->readParam (nd, 25, 11, 0);
						if (rs == -2) {
							++tryRead;
							if (tryRead > 1) goto NextDevice;
						}
						if (rs == -1) {
								conn_err=1;
						}
						if (rs == 0){
							m_log.Write(L_ERROR, "Port: %d, Device: %d. Ошибка чтения показаний тока", numPort, numDevice[nd]);
							//OutConsole(L_ERROR, Rus("Порт %d, Устройство %d. Ошибка чтения показаний тока"), numPort[np], device[np][nd].numDevice);
						}
						if (rs <= 0){
							device[nd].tagStatus[25] = 0;
							device[nd].tagStatus[26] = 0;
							device[nd].tagStatus[27] = 0;
						}
					}

					// Чтение частоты сети
					if (bRun)
					{
						//qDebug() << "Read F";
						rs = this->readParam (nd, 28, 13, 0);
						if (rs == -2) {
							++tryRead;
							if (tryRead > 1) goto NextDevice;
						}
						if (rs == -1) {
								conn_err=1;
						}
						if (rs == 0){
							m_log.Write(L_ERROR, "Port: %d, Device: %d. Ошибка чтения частоты сети", numPort, numDevice[nd]);
							//OutConsole(L_ERROR, Rus("Порт %d, Устройство %d. Ошибка чтения частоты сети"), numPort[np], device[np][nd].numDevice);
						}
						if (rs <= 0){
							device[nd].tagStatus[28] = 0;
						}
					}

					// Чтение серийного номера
					if (bRun)
					{
						rs = this->readParam (nd, 29, 18, 0);
						if (rs == -2) {
							++tryRead;
							if (tryRead > 1) goto NextDevice;
						}
						if (rs == -1) {
								conn_err=1;
						}
						if (rs == 0){
							m_log.Write(L_ERROR, "Port: %d, Device: %d. Ошибка чтения заводского номера", numPort, numDevice[nd]);
							//OutConsole(L_ERROR, Rus("Порт %d, Устройство %d. Ошибка чтения заводского номера"), numPort[np], device[np][nd].numDevice);
						}
						if (rs <= 0){
							device[nd].tagStatus[29] = 0;
						}
					}
				}

				else conn_err = 1;

				if (conn_err == 1)
				{
					conn_err = 1;
	NextDevice:		for(UINT i=0; i<(TAGS_IN_DEVICE-1); ++i)
					{
						device[nd].tagStatus[i] = 0;
					}
					poll_err=1;
				}

				sprintf(device[nd].tagValue[30], "%d", 1-poll_err);
				device[nd].tagStatus[30] = 1;

				// При ошибках подключения переходим к опросу следующего устройства
				if (conn_err == 1 || tryRead > 1)
				{
					if(portType == 1 && tryRead > 1)
					{
						m_log.Write(L_ERROR, "Port: %d, Device: %d. Устройство не отвечает на запросы. Переподключение", numPort, numDevice[nd]);
					}
					if(portType == 1 && tryRead < 1)
					{
						m_log.Write(L_ERROR, "Port: %d. Переподключение", numPort);
					}

					if(portType == 1)
					{
						mutex.lock();
						TCPclose();
						::WSACleanup();
						mutex.unlock();

						iResult = TCPconnect();
					}

					poll_err = 0;
					tryRead  = 0;
					conn_err = 0;

					continue;
				}

				poll_err = 0;
				tryRead  = 0;
			}
		}

		// GetSystemTimeAsFileTime (&ft);

		// Закрытие порта
		if (portType == 1)
		{
			mutex.lock();
			TCPclose();
			mutex.unlock();
		}

		emit pollDone(np);

		cycle = GetTickCount();

		//m_log.Write(L_INFO, "Port: %d. Опрос завершен. Время опроса %d мс\n", numPort, cycle-start);
		//OutConsole(L_INFO, Rus("Опрос завершен. Время опроса %d мс\n"), cycle-start);

		if ((cycle-start) < (period))
			Sleep((period)-(cycle-start));
    }
}


//=============================================================================
// Чтение параметров
//=============================================================================
int tpoll::readParam (int nd, UINT tag_num, uint8_t param_num, int param_shift)
{
	int   rs, iParam1, iParam2;
	float  param1, param2, param3, param4;
	BYTE   data[200];

	// Посылка запроса
	if (param_num == 3)
		rs = this->sendPackage (numDevice[nd], 4, param_num, param_shift, 0);
	else
		rs = this->sendPackage (numDevice[nd], 3, param_num, param_shift, 0);
	// Обработка ошибки посылки пакета
	if (rs <= 0)
	{
		return rs;
	}

	// Чтение ответа
	if (rs >  0)  rs = this->readPackage(numDevice[nd], data, param_num);
	// Обработка ошибки приема пакета
	if (rs <= 0)
	{
		return rs;
	}

	switch (param_num)
	{
		case 24:
			iParam1 = (data[7]<<24 | data[6]<<16 | data[5]<<8 | data[4]);
			device[nd].Kpr = iParam1;
			iParam2 = (data[9]<<8 | data[8]);
			device[nd].Ke  = iParam2;
			sprintf (device[nd].tagValue[tag_num], "%d", iParam1);
			sprintf (device[nd].tagValue[tag_num+1], "%d", iParam2);
			device[nd].tagStatus[tag_num] = 1;
			device[nd].tagStatus[tag_num+1] = 1;
			break;

		case 52:
			BTF.intValue = (data[7]<<24 | data[6]<<16 | data[5]<<8 | data[4]);
			param1 = BTF.floatValue;
			device[nd].KI = param1;
			BTF.intValue = (data[11]<<24 | data[10]<<16 | data[9]<<8 | data[8]);
			param2 = BTF.floatValue;
			device[nd].KU = param2;
			sprintf (device[nd].tagValue[tag_num], "%f", param1);
			sprintf (device[nd].tagValue[tag_num+1], "%f", param2);
			device[nd].tagStatus[tag_num] = 1;
			device[nd].tagStatus[tag_num+1] = 1;
			break;
	}

	switch (param_num)
	{
		case 3:
			BTF.intValue = (data[7]<<24 | data[6]<<16 | data[5]<<8 | data[4]);
			param1 = BTF.floatValue;
			BTF.intValue = (data[11]<<24 | data[10]<<16 | data[9]<<8 | data[8]);
			param2 = BTF.floatValue;
			BTF.intValue = (data[15]<<24 | data[14]<<16 | data[13]<<8 | data[12]);
			param3 = BTF.floatValue;
			BTF.intValue = (data[19]<<24 | data[18]<<16 | data[17]<<8 | data[16]);
			param4 = BTF.floatValue;
			sprintf (device[nd].tagValue[tag_num], "%f", param1);
			sprintf (device[nd].tagValue[tag_num+1], "%f", param2);
			sprintf (device[nd].tagValue[tag_num+2], "%f", param3);
			sprintf (device[nd].tagValue[tag_num+3], "%f", param4);
			device[nd].tagStatus[tag_num] = 1;
			device[nd].tagStatus[tag_num+1] = 1;
			device[nd].tagStatus[tag_num+2] = 1;
			device[nd].tagStatus[tag_num+3] = 1;
			break;

		case 5:
			BTF.intValue = (data[7]<<24 | data[6]<<16 | data[5]<<8 | data[4]);
			if (device[nd].Kpr > 0) param1 = BTF.floatValue * device[nd].KU * device[nd].KI / 1000;
				else param1 = 0;
			BTF.intValue = (data[11]<<24 | data[10]<<16 | data[9]<<8 | data[8]);
			if (device[nd].Kpr > 0) param2 = BTF.floatValue * device[nd].KU * device[nd].KI / 1000;
				else param2 = 0;
			BTF.intValue = (data[15]<<24 | data[14]<<16 | data[13]<<8 | data[12]);
			if (device[nd].Kpr > 0) param3 = BTF.floatValue * device[nd].KU * device[nd].KI / 1000;
				else param3 = 0;
			BTF.intValue = (data[19]<<24 | data[18]<<16 | data[17]<<8 | data[16]);
			if (device[nd].Kpr > 0) param4 = BTF.floatValue * device[nd].KU * device[nd].KI / 1000;
				else param4 = 0;
			sprintf (device[nd].tagValue[tag_num], "%f", param1);
			sprintf (device[nd].tagValue[tag_num+1], "%f", param2);
			sprintf (device[nd].tagValue[tag_num+2], "%f", param3);
			sprintf (device[nd].tagValue[tag_num+3], "%f", param4);
			device[nd].tagStatus[tag_num] = 1;
			device[nd].tagStatus[tag_num+1] = 1;
			device[nd].tagStatus[tag_num+2] = 1;
			device[nd].tagStatus[tag_num+3] = 1;
			break;

		case 8:
		case 9:
			BTF.intValue = (data[11]<<24 | data[10]<<16 | data[9]<<8 | data[8]);
			param1 = BTF.floatValue * device[nd].KU * device[nd].KI / 1000;
			BTF.intValue = (data[15]<<24 | data[14]<<16 | data[13]<<8 | data[12]);
			param2 = BTF.floatValue * device[nd].KU * device[nd].KI / 1000;
			BTF.intValue = (data[19]<<24 | data[18]<<16 | data[17]<<8 | data[16]);
			param3 = BTF.floatValue * device[nd].KU * device[nd].KI / 1000;
			sprintf (device[nd].tagValue[tag_num], "%f", param1);
			sprintf (device[nd].tagValue[tag_num+1], "%f", param2);
			sprintf (device[nd].tagValue[tag_num+2], "%f", param3);
			device[nd].tagStatus[tag_num] = 1;
			device[nd].tagStatus[tag_num+1] = 1;
			device[nd].tagStatus[tag_num+2] = 1;
			break;

		case 10:
			BTF.intValue = (data[7]<<24 | data[6]<<16 | data[5]<<8 | data[4]);
			param1 = BTF.floatValue * device[nd].KU;
			BTF.intValue = (data[11]<<24 | data[10]<<16 | data[9]<<8 | data[8]);
			param2 = BTF.floatValue * device[nd].KU;
			BTF.intValue = (data[15]<<24 | data[14]<<16 | data[13]<<8 | data[12]);
			param3 = BTF.floatValue * device[nd].KU;
			sprintf (device[nd].tagValue[tag_num], "%f", param1);
			sprintf (device[nd].tagValue[tag_num+1], "%f", param2);
			sprintf (device[nd].tagValue[tag_num+2], "%f", param3);
			device[nd].tagStatus[tag_num] = 1;
			device[nd].tagStatus[tag_num+1] = 1;
			device[nd].tagStatus[tag_num+2] = 1;
			break;

		case 11:
			BTF.intValue = (data[7]<<24 | data[6]<<16 | data[5]<<8 | data[4]);
			param1 = BTF.floatValue * device[nd].KI;
			BTF.intValue = (data[11]<<24 | data[10]<<16 | data[9]<<8 | data[8]);
			param2 = BTF.floatValue * device[nd].KI;
			BTF.intValue = (data[15]<<24 | data[14]<<16 | data[13]<<8 | data[12]);
			param3 = BTF.floatValue * device[nd].KI;
			sprintf (device[nd].tagValue[tag_num],"%f", param1);
			sprintf (device[nd].tagValue[tag_num+1],"%f", param2);
			sprintf (device[nd].tagValue[tag_num+2],"%f", param3);
			device[nd].tagStatus[tag_num] = 1;
			device[nd].tagStatus[tag_num+1] = 1;
			device[nd].tagStatus[tag_num+2] = 1;
			break;

		case 13:
			BTF.intValue = (data[7]<<24 | data[6]<<16 | data[5]<<8 | data[4]);
			param1 = BTF.floatValue;
			sprintf (device[nd].tagValue[tag_num], "%f", param1);
			device[nd].tagStatus[tag_num] = 1;
			break;

		case 18:
			sprintf  (device[nd].tagValue[tag_num],"%c%c%c%c%c%c%c%c%c%c",
								data[4], data[5], data[6], data[7], data[8], data[9], data[10], data[11], data[12], data[13]);
			device[nd].tagStatus[tag_num] = 1;
			break;
	}
	return 1;
}


//=============================================================================
// Посылка пакета
//=============================================================================
BOOL tpoll::sendPackage (int nd, int fun, int kop, int shft, int corr)
{
	uint16_t	CRC=0;          //(* CRC checksum *)
	UINT		nbytes = 0;     //(* number of bytes in send packet *)
	uint8_t		data[100];      //(* send sequence *)

	// Формирование посылки
	data[0] = nd;                  // Номер устройства
	data[1] = fun;					// Функция
	data[2] = kop;					// Код параметра
	data[3] = shft;					// Смещение

	if (kop==5) data[3] = -1;

	data[4] = 0x0;					// Тариф
	data[5] = corr;					// Уточнение

	// Контрольная сумма
	CRC = GetCRC(data, 6);
	data[6] = (CRC & 0x00FF);
	data[7] = ((CRC>>8) & 0x00FF);

	if (portType==0)
	{
		DWORD dwErrors=CE_FRAME|CE_IOE|CE_TXFULL|CE_RXPARITY|CE_RXOVER|CE_OVERRUN|CE_MODE|CE_BREAK;
		//port[this->idPort].ClearWriteBuffer();
		this->port->Purge(PURGE_TXABORT|PURGE_RXABORT|PURGE_TXCLEAR|PURGE_RXCLEAR);
		this->port->ClearError(dwErrors);
		nbytes = this->port->Write(data, 8);
	}
	else nbytes = send( soc, (const char*)data, 8, 0 );

	//m_log.Write(L_TRACE, "Port: %d, Device: %d  WR: %02X %02X %02X %02X %02X %02X %02X %02X", numPort, dev, data[0],data[1],data[2],data[3],data[4],data[5],data[6],data[7]);
	//	OutConsole (L_TRACE, "WR: %02X %02X %02X %02X %02X %02X %02X %02X", data[0],data[1],data[2],data[3],data[4],data[5],data[6],data[7]);

	if((portType > 0) && ((nbytes == SOCKET_ERROR) || (nbytes == 0))){
		m_log.Write(L_ERROR, "Ошибка Port: %d, Device: %d. Ошибка отправки данных в порт", this->numPort, nd);
		//OutConsole (L_ERROR, Rus("Ошибка отправки данных в порт %d"), this->numPort);
		return -1;
	}
	return nbytes;
}


//=============================================================================
// Чтение пакета
//=============================================================================
int  tpoll::readPackage (int nd, uint8_t* dat, uint8_t param_num)
{
	uint16_t   CRC=0;			//(* CRC checksum *)
	int        nbytes = 0;		//(* number of bytes in recieve packet *)
	uint8_t    data[100];		//(* recieve sequence *)
	int iResult;
	fd_set	read_s;
	timeval	time_out;

	time_out.tv_sec = 0;
	//time_out.tv_usec =  1000 * TIMEOUT_TCP;	// msec -> usec.
	time_out.tv_usec =  1000 * timeoutTCP;	// msec -> usec.

	Sleep (10);

	if (portType==0)
	{
		nbytes = this->port->Read(data, 100);
	}
	else
	{
		FD_ZERO (&read_s);	 // Обнуляем множество
		FD_SET  (soc, &read_s); // Заносим в него наш сокет

		if ((iResult = select (0, &read_s, NULL, NULL, &time_out)) == SOCKET_ERROR)
		{
			m_log.Write(L_ERROR, "Ошибка Port: %d, Device: %d. Ошибка чтения порта", this->numPort, nd);
			//OutConsole (L_ERROR, Rus("Ошибка чтения порт %d"), this->numPort);
			return -1;
		}

		if (iResult==0)
		{
			m_log.Write(L_ERROR, "Ошибка Port: %d, Device: %d. Истекло время ожидания ответа для порта", this->numPort, nd);
			//OutConsole (L_ERROR, Rus("Истекло время ожидания ответа для порта %d"), this->numPort);
			return -2;
		}

		if ((iResult>0) && (FD_ISSET (soc, &read_s)) )
		{
			// Данные готовы к чтению...
			nbytes = recv( soc, (char *)data, 100, 0 );
			if (nbytes == SOCKET_ERROR)
			{
				return -1;
			}
		}
	}

	if (nbytes==0) return -1;

	if (nbytes==6 && param_num!=32)
	{
		CRC = GetCRC(data, nbytes-2);
		if ((CRC != (data[nbytes-2] | data[nbytes-1]<<8)) || data[3] > 0) nbytes=0;
		m_log.Write(L_ERROR, "Параметр %d не поддерживается устройством", param_num);
		//OutConsole(L_ERROR, Rus("Параметр %d не поддерживается устройством"), param_num);
		return nbytes;
	}

	if ((nbytes>5) & (data[3] > 0))
	{
		m_log.Write(L_TRACE, "Ошибка запроса %d", data[3]);
		//OutConsole (L_TRACE, Rus("Ошибка запроса %d"), data[3]);
		return 0;
	}

	switch (param_num)
	{
		case 3:
		case 5:
		case 8:
		case 9:
//			m_log.Write(L_TRACE, "Port: %d, Device: %d  RD: %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X",
//														numPort, numDevice[nd], data[0],data[1],data[2],data[3],data[4],data[5],data[6],data[7],data[8],data[9],data[10],data[11],data[12],data[13],data[14],data[15],data[16],data[17],data[18],data[19],data[20],data[21]);
//			OutConsole (L_TRACE, "Port: %d, Device: %d  RD: %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X",
//														data[0],data[1],data[2],data[3],data[4],data[5],data[6],data[7],data[8],data[9],data[10],data[11],data[12],data[13],data[14],data[15],data[16],data[17],data[18],data[19],data[20],data[21]);
			break;

		case 10:
		case 11:
//			m_log.Write(L_TRACE, "Port: %d, Device: %d  RD: %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X",
//														numPort, numDevice[nd], data[0],data[1],data[2],data[3],data[4],data[5],data[6],data[7],data[8],data[9],data[10],data[11],data[12],data[13],data[14],data[15],data[16],data[17]);
//			OutConsole(L_TRACE, Rus("RD: %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X"),
//														data[0],data[1],data[2],data[3],data[4],data[5],data[6],data[7],data[8],data[9],data[10],data[11],data[12],data[13],data[14],data[15],data[16],data[17]);
			break;

		case 13:
//			m_log.Write(L_TRACE, "Port: %d, Device: %d  RD: %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X",
//														numPort, numDevice[nd], data[0],data[1],data[2],data[3],data[4],data[5],data[6],data[7],data[8],data[9]);
//			OutConsole (L_TRACE, "RD: %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X",
//														data[0],data[1],data[2],data[3],data[4],data[5],data[6],data[7],data[8],data[9]);
			break;

		case 18:
//			m_log.Write(L_TRACE, "Port: %d, Device: %d  RD: %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X",
//														numPort, numDevice[nd], data[0],data[1],data[2],data[3],data[4],data[5],data[6],data[7],data[8],data[9],data[10],data[11],data[12],data[13],data[14],data[15]);
//			OutConsole (L_TRACE, "RD: %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X",
//														data[0],data[1],data[2],data[3],data[4],data[5],data[6],data[7],data[8],data[9],data[10],data[11],data[12],data[13],data[14],data[15]);
			break;

		case 24:
//			m_log.Write(L_TRACE, "Port: %d, Device: %d  RD: %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X",
//														numPort, numDevice[nd], data[0],data[1],data[2],data[3],data[4],data[5],data[6],data[7],data[8],data[9],data[10],data[11],data[12],data[13]);
//			OutConsole (L_TRACE, "RD: %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X",
//														data[0],data[1],data[2],data[3],data[4],data[5],data[6],data[7],data[8],data[9],data[10],data[11],data[12],data[13]);
			break;

		case 52:
//			m_log.Write(L_TRACE, "Port: %d, Device: %d  RD: %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X",
//														numPort, numDevice[nd], data[0],data[1],data[2],data[3],data[4],data[5],data[6],data[7],data[8],data[9],data[10],data[11],data[12],data[13],data[14],data[15],data[16],data[17],data[18],data[19],data[20],data[21],data[22],data[23]);
//			OutConsole (L_TRACE, "RD: %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X",
//														data[0],data[1],data[2],data[3],data[4],data[5],data[6],data[7],data[8],data[9],data[10],data[11],data[12],data[13],data[14],data[15],data[16],data[17],data[18],data[19],data[20],data[21],data[22],data[23]);
			break;
	}

	CRC = GetCRC(data, nbytes-2);
	if (CRC != (data[nbytes-2] | data[nbytes-1]<<8))
	{
		m_log.Write(L_ERROR, "Port: %d, Device: %d. Чтение параметра %d: BAD CRC! Read:[%02X %02X] Calc:[%02X %02X]", numPort, numDevice[nd], param_num, data[nbytes-2], data[nbytes-1], (CRC & 0x00FF), ((CRC>>8) & 0x00FF));
		//OutConsole (L_ERROR, Rus("Порт %d, Устройство %d. Чтение параметра %d: BAD CRC! Read:[%02X %02X] Calc:[%02X %02X]"),this->numPort, this->numDevice, param_num, data[nbytes-2], data[nbytes-1], (CRC & 0x00FF), ((CRC>>8) & 0x00FF));
		this->port->ClearReadBuffer();
		return 0;
	}

	//	m_log.Write(L_TRACE, "Port: %d, Device: %d  CRC OK: [%02X %02X]", numPort, numDevice[nd], (CRC & 0x00FF), ((CRC>>8) & 0x00FF));
	//	OutConsole (L_TRACE, "CRC OK: [%02X %02X]", (CRC & 0x00FF), ((CRC>>8) & 0x00FF));

	memcpy (dat, data, nbytes);

	return nbytes;
}


//=============================================================================
// Работа с TCP портами
//=============================================================================
int tpoll::TCPconnect ()
{
	int iResult = 0;

	//m_log.Write(L_INFO, "Подключение к порту %d...", numPort);
	//OutConsole(L_INFO, Rus("Подключение к порту %d..."), numPort);

	iResult = ::WSAStartup(MAKEWORD(2,2), &wsaData);

	if (iResult != 0) {
		m_log.Write(L_ERROR, "Port: %d. Ошибка WSAStartup: %d", numPort, iResult);
		//OutConsole (L_ERROR, Rus("Ошибка WSAStartup: %d"), iResult);
		return 0;
	}

	//m_log.Write(L_INFO, "Gethostbyname...");
	hostEnt = ::gethostbyname((const char*)ipAddress);

	if(!hostEnt)
	{
		m_log.Write(L_ERROR, "Port: %d. Ошибка gethostbyname", numPort);
		//OutConsole (L_ERROR, Rus("Ошибка gethostbyname"));
		::WSACleanup();
		return 0;
	}

	//m_log.Write(L_INFO, "Socket...");
	soc = ::socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (soc == INVALID_SOCKET)
	{
		m_log.Write(L_ERROR, "Port: %d. Ошибка открытия сокета: %d", numPort, WSAGetLastError());
		//OutConsole (L_ERROR, Rus("Ошибка открытия сокета: %d"), WSAGetLastError());
		::WSACleanup();
		return 0;
	}

	server.sin_family = PF_INET;
	server.sin_port = ::htons(numPort);
	server.sin_addr.s_addr = ::inet_addr((const char*)ipAddress);

	// Connect to server.
	//m_log.Write(L_INFO, "Connect...");
	iResult = ::connect( soc, (struct sockaddr*) &server, sizeof(server));
	if (iResult == SOCKET_ERROR)
	{
		m_log.Write(L_ERROR, "Port: %d. Ошибка подключения к серверу: %d", numPort, WSAGetLastError());
		//OutConsole (L_ERROR, Rus("Ошибка подключения к серверу: %d"), WSAGetLastError());
		closesocket(soc);
		soc = INVALID_SOCKET;

		Sleep(timeoutTCP);
		return 0;
	}
	//m_log.Write(L_INFO, "Подключение к порту %d выполнено", numPort);
	return 1;
}


int tpoll::TCPclose()
{
	int iResult;

	//m_log.Write(L_INFO, "Отключение от порта %d...", numPort);
	//OutConsole (L_INFO, Rus("Отключение от порта %d\n"), numPort);

	iResult = shutdown(soc, SD_RECEIVE);

	closesocket(soc);
	soc = INVALID_SOCKET;

	//m_log.Write(L_INFO, "Отключение от порта %d выполнено", numPort);
	return 1;
}


//=============================================================================
// Вычисление контрольной суммы
//=============================================================================
uint16_t tpoll::GetCRC(uint8_t* Package, uint8_t len)
{
	uint16_t idx;
	uint8_t CRChi = 0xFF;
	uint8_t CRClo = 0xFF;

	while(len--)
	{
		idx = (CRChi ^ *Package++) & 0xFF;
		CRChi = CRClo ^ tblCRChi[idx];
		CRClo = tblCRClo[idx];
	}
	return ((CRChi << 8) | CRClo);
}


//=============================================================================
// Изменение формата времени
//=============================================================================
VOID tpoll::timetToFileTime( time_t t, LPFILETIME pft )
{
	LONGLONG ll = Int32x32To64(t, 10000000) + 116444736000000000;
	pft->dwLowDateTime = (DWORD) ll;
	pft->dwHighDateTime =  (ULONG)(ll >>32);
}


//=============================================================================
// Корректировка времени
//=============================================================================
int tpoll::timeCorrect( int nd )
{
	QMutex mutex;
	int  rs, adjust_time = 4;
	int  deviceTimeInSeconds, localTimeInSeconds, correctTimeInSeconds, timeShift, additive;
	BYTE dataRead[100];
	BYTE dataSend[100];
	BYTE oldTime[3];
	BYTE newTime[3];
	uint16_t CRC=0;          //(* CRC checksum *)
	UINT     nbytes = 0;     //(* number of bytes in send packet *)

	//mutex.lock();

	// Определение системного времени
	QTime correctTime = QTime::currentTime();

	if ((correctTime.hour() >= adjust_time)
		&& (backCorrectTime[nd].hour() < adjust_time))
	{
		m_log.Write(L_INFO, "Port: %d, Device: %d. Запуск корректировки времени...", this->numPort, numDevice[nd]);

		backCorrectTime[nd] = correctTime;
		// Чтение времени устройства
		// Посылка запроса
		rs = this->sendPackage(numDevice[nd], 4, 32, 0, 0);
		// Обработка ошибки посылки пакета
		if (rs <= 0)
		{
			m_log.Write(L_ERROR, "Port: %d, Device: %d. Ошибка чтения текущего времени", this->numPort, numDevice[nd]);
			return -1;
		}
		// Чтение ответа
		if (rs >  0)  rs = this->readPackage(numDevice[nd], dataRead, 32);
		// Обработка ошибки приема пакета
		if (rs <= 0)
		{
			m_log.Write(L_ERROR, "Port: %d, Device: %d. Ошибка чтения текущего времени", this->numPort, numDevice[nd]);
			return -2;
		}

		oldTime[0] = dataRead[4];
		oldTime[1] = dataRead[5];
		oldTime[2] = dataRead[6];

		// Перевод времени в секунды
		deviceTimeInSeconds = oldTime[0] + (oldTime[1] * 60) + (oldTime[2] * 3600);
		localTimeInSeconds = correctTime.second() + (correctTime.minute() * 60) + (correctTime.hour() * 3600);

		// Поправка на время опроса устройства
		QTime addTime = QTime::currentTime();
		additive = addTime.second() + (addTime.minute() * 60) + (addTime.hour() * 3600) - localTimeInSeconds;

		// Часы устройства спешат
		if ((deviceTimeInSeconds - localTimeInSeconds) >= 5)
		{
			if (deviceTimeInSeconds < localTimeInSeconds + timeShiftMax[nd] )
			{
				timeShift = deviceTimeInSeconds - localTimeInSeconds;
			}
			else
			{
				timeShift = timeShiftMax[nd];
			}
			correctTimeInSeconds = deviceTimeInSeconds - timeShift + additive;
		}

		// Часы устройства отстают
		if ((deviceTimeInSeconds - localTimeInSeconds) <= -5)
		{
			if (deviceTimeInSeconds + timeShiftMax[nd] > localTimeInSeconds )
			{
				timeShift = localTimeInSeconds - deviceTimeInSeconds;
			}
			else
			{
				timeShift = timeShiftMax[nd];
			}
			correctTimeInSeconds = deviceTimeInSeconds + timeShift + additive;
		}

		// Часы устройства показывают допустимое время
		if ((deviceTimeInSeconds - localTimeInSeconds) > -5 && (deviceTimeInSeconds - localTimeInSeconds) < 5)
		{
			m_log.Write(L_INFO, "Port: %d, Device: %d. Время на устройстве не нуждается в корректировке",
						this->numPort, numDevice[nd]);

			return 0;
		}

		newTime[0] = correctTimeInSeconds % 60;
		newTime[1] = (correctTimeInSeconds % 3600) / 60;
		newTime[2] = correctTimeInSeconds / 3600;

		// Установка времени устройства
		// Посылка запроса
		// Формирование посылки
		dataSend[0] = numDevice[nd];		// Номер устройства
		dataSend[1] = 16;					// Функция
		dataSend[2] = 32;					// Код параметра
		dataSend[3] = 0;					// Уточнение

		// Корректировка времени устройства
		dataSend[4] = newTime[0];			// Секунды
		dataSend[5] = newTime[1];			// Минуты
		dataSend[6] = newTime[2];			// Часы
		dataSend[7] = dataRead[7];			// Дни
		dataSend[8] = dataRead[8];			// Месяцы
		dataSend[9] = dataRead[9];			// Годы

		// Контрольная сумма
		CRC = GetCRC(dataSend, 10);
		dataSend[10] = (CRC & 0x00FF);
		dataSend[11] = ((CRC>>8) & 0x00FF);

		if (portType == 0)
		{
			DWORD dwErrors=CE_FRAME|CE_IOE|CE_TXFULL|CE_RXPARITY|CE_RXOVER|CE_OVERRUN|CE_MODE|CE_BREAK;
			this->port->Purge(PURGE_TXABORT|PURGE_RXABORT|PURGE_TXCLEAR|PURGE_RXCLEAR);
			this->port->ClearError(dwErrors);
			nbytes = this->port->Write(dataSend, 12);
		}
		if (portType == 1)
		{
			nbytes = send( soc, (const char*)dataSend, 12, 0 );
		}

		if((portType == 1) && ((nbytes == SOCKET_ERROR) || (nbytes == 0)))
		{
			m_log.Write(L_ERROR, "Ошибка Port: %d, Device: %d. Ошибка отправки данных в порт", this->numPort, numDevice[nd]);
			nbytes = -1;
		}

		// Обработка ошибки посылки пакета
		if (nbytes <= 0)
		{
			m_log.Write(L_ERROR, "Port: %d, Device: %d. Ошибка записи откорректированного времени", this->numPort, numDevice[nd]);
			return -11;
		}

		// Чтение ответа
		if (nbytes >  0)  rs = this->readPackage(numDevice[nd], dataRead, 32);
		// Обработка ошибки приема пакета
		if (rs <= 0)
		{
			m_log.Write(L_ERROR, "Port: %d, Device: %d. Ошибка записи откорректированного времени", this->numPort, numDevice[nd]);
			return -12;
		}

		m_log.Write(L_INFO, "Port: %d, Device: %d. Проведена корректировка времени. Было: %02d:%02d:%02d. Установлено: %02d:%02d:%02d.",
					this->numPort, numDevice[nd], oldTime[2], oldTime[1], oldTime[0], newTime[2], newTime[1], newTime[0]);

		//mutex.unlock();
		return 0;
	}
	backCorrectTime[nd] = correctTime;
	return 0;
}

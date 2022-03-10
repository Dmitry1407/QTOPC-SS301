#include <windows.h>
#include <objbase.h>
#include <unknwn.h>
#include <iostream>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#include <QtGui>
#include <QObject>
#include <QAxWidget>
#include <QAxObject>

// 3rd Includes
#include "3rd/opcda/opcda.h"
#include "3rd/lightopc/lightopc.h"
#include "3rd/serialport/serialport.h"

// Local Includes
#include "src/main.h"
#include "src/tlog.h"
#include "src/tdevice.h"
#include "src/tpoll.h"

#ifndef TDRIVER_H
#define TDRIVER_


class TDriver : public QObject
{
	Q_OBJECT
public:
	TDriver();
	~TDriver();

	UINT initDriver();
	UINT destroyDriver();
	INT  initTags();
	VOID poll_device();

public:
	tpoll poll[PORT_NUM_MAX];

	QTextCodec *codec;

	// Параметры OPC
	UINT tags_num;                              // количество тегов
	UINT tTotal;                                // общее количество тегов

	BOOL driverRun;

	// Параметры драйвера
	int numPorts;

	int timeout;
	int delay;
	int update;

	// Общие параметры портов
	BOOL portEnable[PORT_NUM_MAX];
	int portType[PORT_NUM_MAX];
	int numDevices[PORT_NUM_MAX];

	// Параметры COM портов
	QString portName[PORT_NUM_MAX];
	int numPort[PORT_NUM_MAX];

	int baudID[PORT_NUM_MAX];
	int data_bitsID[PORT_NUM_MAX];
	int stop_bitsID[PORT_NUM_MAX];
	int parityID[PORT_NUM_MAX];
	int period[PORT_NUM_MAX];

	int baud[PORT_NUM_MAX];
	int data_bits[PORT_NUM_MAX];
	int stop_bits[PORT_NUM_MAX];
	int parity[PORT_NUM_MAX];

	// Параметры TCP портов
	QString addressIP[PORT_NUM_MAX];
	int portIP[PORT_NUM_MAX];
	int timeoutTCP[PORT_NUM_MAX];

	// Параметры устройств
	BOOL deviceEnable[PORT_NUM_MAX][DEV_NUM_MAX];
	QString deviceName[PORT_NUM_MAX][DEV_NUM_MAX];
	int numDevice[PORT_NUM_MAX][DEV_NUM_MAX];

	// Корректировка времени
	BOOL timeCorrectEnable[PORT_NUM_MAX][DEV_NUM_MAX];
	int  timeShiftMax[PORT_NUM_MAX][DEV_NUM_MAX];


	struct TagInfo
	{
		char    symbol[16];
		char    name[200];
		UINT	type;		// VT_I2 VT_I4 VT_R4 VT_R8 VT_BSTR VT_BOOL
		UINT	getCmd;
		char    value[200];
		char    timestamp[32];
		SHORT   status;		// текущий статус, (0-нет связи, 1-нормально, 2-:)
	};

	TagInfo *Tag;
	TagInfo ***Tags;
	//TagInfo Tag[TAGS_IN_DEVICE];
	//TagInfo Tags[PORT_NUM_MAX][DEV_NUM_MAX][TAGS_IN_DEVICE];

	union ByteToFloat {
		uint32_t intValue;
		float  floatValue;
	} BTF;

private slots:
	void pDone(int);

signals:
	void updateTableWidget();

protected:
};


#endif // TDRIVER_H

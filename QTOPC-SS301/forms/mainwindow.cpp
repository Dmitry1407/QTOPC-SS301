#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "./3rd/serialport/serialport.h"
#include "./3rd/lightopc/lightopc.h"
#include "./3rd/opcda/opcda.h"


MainWindow::MainWindow(QWidget *parent) :
	QMainWindow(parent), ui(new Ui::MainWindow)
{
	driver = new TDriver();

	bPage1=bPage2=bPage3=0;
	curTreeIndex1=curTreeIndex2=curTreeIndex3=-1;
	ui->setupUi(this);

	// Создание экшенов
	createActions();

	// Создание тулбара
	createToolBar();

	// Создание treeWidget
	readConfig("");
	createTreeWidget();
	setWindowTitle(tr(driverName) + tr(" - ") + QFileInfo(fileName).fileName());

	// Создание tableWidget
	createTableWidget();

	ui->tabWidget->setCurrentIndex(0);

	selectDriver();

	// Создание иконки тулбара
	createTrayIcon();
	setIcon();
	trayIcon->show();
	bVisible = true;

	curPort = 0;
	curDevice = 0;
	driverRun = false;
	driver->driverRun = false;

	updateTableWidget();

	// Связывание слотов с сигналами потоков
	QObject::connect(driver, SIGNAL(updateTableWidget()), this, SLOT(updateTableWidget()));

	// Запуск OPC
	runDriver();
}


MainWindow::~MainWindow()
{
	delete ui;
}


void MainWindow::createActions()
{
	newAction = new QAction(tr("&New"), this);
	newAction->setIcon(QIcon(":/icons/file/file_new.png"));
	newAction->setStatusTip(tr("New"));
	connect(newAction, SIGNAL(triggered()), this, SLOT(newFile()));

	openAction = new QAction(tr("&Open"), this);
	openAction->setIcon(QIcon(":/icons/file/file_open.png"));
	openAction->setStatusTip(tr("Open"));
	connect(openAction, SIGNAL(triggered()), this, SLOT(openFile()));

	saveAction = new QAction(tr("&Save"), this);
	saveAction->setIcon(QIcon(":/icons/file/file_save.png"));
	saveAction->setStatusTip(tr("Save"));
	connect(saveAction, SIGNAL(triggered()), this, SLOT(saveFile()));

	saveAsAction = new QAction(tr("&Save as"), this);
	saveAsAction->setIcon(QIcon(":/icons/file/file_save_as.png"));
	saveAsAction->setStatusTip(tr("Save"));
	connect(saveAsAction, SIGNAL(triggered()), this, SLOT(saveFileAs()));

	addAction = new QAction(tr("&Add"), this);
	addAction->setIcon(QIcon(":/icons/edit/plus.png"));
	addAction->setStatusTip(tr("Add"));
	connect(addAction, SIGNAL(triggered()), this, SLOT(addItem()));

	removeAction = new QAction(tr("&Remove"), this);
	removeAction->setIcon(QIcon(":/icons/edit/minus.png"));
	removeAction->setStatusTip(tr("Remove"));
	connect(removeAction, SIGNAL(triggered()), this, SLOT(removeItem()));

	runAction = new QAction(tr("&Run"), this);
	runAction->setIcon(QIcon(":/icons/plc/run.png"));
	runAction->setStatusTip(tr("Run"));
	connect(runAction, SIGNAL(triggered()), this, SLOT(runDriver()));

	stopAction = new QAction(tr("&Stop"), this);
	stopAction->setIcon(QIcon(":/icons/plc/stop.png"));
	stopAction->setStatusTip(tr("Stop"));
	connect(stopAction, SIGNAL(triggered()), this, SLOT(stopDriver()));

	restoreAction = new QAction(tr("&Restore"), this);
	restoreAction->setIcon(QIcon(":/icons/plc/run.png"));
	connect(restoreAction, SIGNAL(triggered()), this, SLOT(showNormal()));

	quitAction = new QAction(tr("&Quit"), this);
	quitAction->setIcon(QIcon(":/icons/file/file_quit.png"));
	connect(quitAction, SIGNAL(triggered()), qApp, SLOT(quit()));
}


void MainWindow::createToolBar()
{
	mainToolBar = addToolBar(tr("Main"));
	mainToolBar->setMovable(false);

	mainToolBar->addAction(newAction);
	mainToolBar->addAction(openAction);
	mainToolBar->addAction(saveAction);
	saveAction->setEnabled(false);
	mainToolBar->addAction(saveAsAction);
	mainToolBar->addSeparator();
	mainToolBar->addAction(addAction);
	addAction->setEnabled(false);
	mainToolBar->addAction(removeAction);
	removeAction->setEnabled(false);
	mainToolBar->addSeparator();

	QWidget* spacer = new QWidget();
	spacer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	//mainToolBar is a pointer to an existing toolbar
	mainToolBar->addWidget(spacer);

	mainToolBar->addSeparator();
	mainToolBar->addAction(stopAction);
	mainToolBar->addAction(runAction);
	stopAction->setEnabled(false);
	//stopAction->setVisible(false);
}


void MainWindow::addItem()
{
	ui->statusBar->showMessage(tr("addItem"));

	// добавление порта
	if ((curTreeIndex2 < 0) && (curTreeIndex3 < 0) && (driver->numPorts < PORT_NUM_MAX))
	{
		driver->portType[driver->numPorts] = 0;
		driver->numPort[driver->numPorts] = driver->numPorts + 1;
		driver->portEnable[driver->numPorts] = 1;
		driver->portName[driver->numPorts] = "port_" + QString::number(driver->numPorts+1);

		driver->addressIP[driver->numPorts] = "127.0.0.1";
		driver->portIP[driver->numPorts] = 4001 + driver->numPorts;
		driver->timeoutTCP[driver->numPorts] = 2000;

		driver->numDevices[driver->numPorts] = 0;
		driver->baudID[driver->numPorts] = 3;
		driver->baud[driver->numPorts] = ui->cbBaud->itemText(driver->baudID[driver->numPorts]).toInt();
		driver->data_bitsID[driver->numPorts] = 2;
		driver->data_bits[driver->numPorts] = ui->cbDataBits->itemText(driver->data_bitsID[driver->numPorts]).toInt();
		driver->stop_bitsID[driver->numPorts] = 0;
		driver->stop_bits[driver->numPorts] = ui->cbStopBits->itemText(driver->stop_bitsID[driver->numPorts]).toInt();
		driver->parityID[driver->numPorts] = 0;
		driver->parity[driver->numPorts] = 2;
		driver->period[driver->numPorts] = 1000;

		++driver->numPorts;
		insertItem  (currentItem, "port_" + QString::number(driver->numPorts));
	}

	// добавление устройства
	if ((curTreeIndex2 >= 0) && (curTreeIndex3 < 0) && (driver->numDevices[curPort] < DEV_NUM_MAX))
	{
		driver->numDevice[curPort][driver->numDevices[curPort]] = driver->numDevices[curPort]+1;
		driver->deviceEnable[curPort][driver->numDevices[curPort]] = 1;
		driver->deviceName[curPort][driver->numDevices[curPort]] = "device_" + QString::number(driver->numDevices[curPort]+1);
		\
		driver->timeCorrectEnable[curPort][driver->numDevices[curPort]] = false;
		driver->timeShiftMax[curPort][driver->numDevices[curPort]] = 30;

		++driver->numDevices[curPort];
		insertItem  (currentItem, "device_" + QString::number(driver->numDevices[curPort]));
	}
	saveAction->setEnabled(true);
}


void MainWindow::removeItem()
{
	ui->statusBar->showMessage(tr("removeItem"));

	// удаление порта
	if ((curTreeIndex2 >= 0) && (curTreeIndex3 < 0) && curPort < (PORT_NUM_MAX-1))
	{
		for (int np = curPort; np < (driver->numPorts-1); ++np)
		{
			driver->portType[np] = driver->portType[np+1];
			driver->numPort[np] = driver->numPort[np+1];
			driver->portEnable[np] = driver->portEnable[np+1];
			driver->portName[np] = driver->portName[np+1];

			driver->addressIP[np] = driver->addressIP[np+1];
			driver->portIP[np] = driver->portIP[np+1];
			driver->timeoutTCP[np] = driver->timeoutTCP[np+1];

			driver->numDevices[np] = driver->numDevices[np+1];
			driver->baudID[np] = driver->baudID[np+1];
			driver->baud[np] = driver->baud[np+1];
			driver->data_bitsID[np] = driver->data_bitsID[np+1];
			driver->data_bits[np] = driver->data_bits[np+1];
			driver->stop_bitsID[np] = driver->stop_bitsID[np+1];
			driver->stop_bits[np] = driver->stop_bits[np+1];
			driver->parityID[np] = driver->parityID[np+1];
			driver->parity[np] = driver->parity[np+1];
			driver->period[np] = driver->period[np+1];

			for (int nd = 0; nd < driver->numDevices[np]; ++nd)
			{
				driver->numDevice[np][nd] = driver->numDevice[np+1][nd];
				driver->deviceEnable[np][nd] = driver->deviceEnable[np+1][nd];
				driver->deviceName[np][nd] = driver->deviceName[np+1][nd];
				driver->timeCorrectEnable[np][nd] = driver->timeCorrectEnable[np+1][nd];
				driver->timeShiftMax[np][nd] = driver->timeShiftMax[np+1][nd];
			}
		}
		--driver->numPorts;
		//selectPort(0);

		ui->statusBar->showMessage("Port " + QString::number(curPort));

		ui->cbPortEnable->setChecked(driver->portEnable[curPort]);
		ui->editPortName->setText(driver->portName[curPort]);

		if (driver->portType[curPort] == 0)
		{
			ui->rbCOM->setChecked(true);
			on_rbCOM_clicked();
		}
		else
		{
			ui->rbTCP->setChecked(true);
			on_rbTCP_clicked();
		}

		ui->editPollPeriod->setText(QString::number(driver->period[curPort]));
		ui->editNumPort->setText(QString::number(driver->numPort[curPort]));
		ui->cbBaud->setCurrentIndex(driver->baudID[curPort]);
		ui->cbDataBits->setCurrentIndex(driver->data_bitsID[curPort]);
		ui->cbParity->setCurrentIndex(driver->parityID[curPort]);
		ui->cbStopBits->setCurrentIndex(driver->stop_bitsID[curPort]);

		ui->editAddressIP->setText(driver->addressIP[curPort]);
		ui->editPortIP->setText(QString::number(driver->portIP[curPort]));
		ui->editTimeoutTCP->setText(QString::number(driver->timeoutTCP[curPort]));
	}

	if ((curTreeIndex2 >= 0) && (curTreeIndex3 < 0) && curPort >= (PORT_NUM_MAX-1))
	{
		--driver->numPorts;
		//selectPort(0);

		ui->statusBar->showMessage("Port " + QString::number(curPort));

		ui->cbPortEnable->setChecked(driver->portEnable[curPort]);
		ui->editPortName->setText(driver->portName[curPort]);

		if (driver->portType[curPort] == 0)
		{
			ui->rbCOM->setChecked(true);
			on_rbCOM_clicked();
		}
		else
		{
			ui->rbTCP->setChecked(true);
			on_rbTCP_clicked();
		}

		ui->editPollPeriod->setText(QString::number(driver->period[curPort]));
		ui->editNumPort->setText(QString::number(driver->numPort[curPort]));
		ui->cbBaud->setCurrentIndex(driver->baudID[curPort]);
		ui->cbDataBits->setCurrentIndex(driver->data_bitsID[curPort]);
		ui->cbParity->setCurrentIndex(driver->parityID[curPort]);
		ui->cbStopBits->setCurrentIndex(driver->stop_bitsID[curPort]);

		ui->editAddressIP->setText(driver->addressIP[curPort]);
		ui->editPortIP->setText(QString::number(driver->portIP[curPort]));
		ui->editTimeoutTCP->setText(QString::number(driver->timeoutTCP[curPort]));
	}

	// удаление устройства
	if ((curTreeIndex2 >= 0) && (curTreeIndex3 >= 0) && curDevice < (DEV_NUM_MAX-1))
	{
		for (int nd = curDevice; nd < (driver->numDevices[curPort]-1); ++nd)
		{
			driver->numDevice[curPort][nd] = driver->numDevice[curPort][nd+1];
			driver->deviceEnable[curPort][nd] = driver->deviceEnable[curPort][nd+1];
			driver->deviceName[curPort][nd] = driver->deviceName[curPort][nd+1];
			driver->timeCorrectEnable[curPort][nd] = driver->timeCorrectEnable[curPort][nd+1];
			driver->timeShiftMax[curPort][nd] = driver->timeShiftMax[curPort][nd+1];
		}
		--driver->numDevices[curPort];
		//selectDevice(0, 0);

		ui->cbDeviceEnable->setChecked(driver->deviceEnable[curPort][curDevice]);
		ui->editDeviceName->setText(driver->deviceName[curPort][curDevice]);
		ui->editNumDevice->setText(QString::number(driver->numDevice[curPort][curDevice]));
		ui->cbTimeCorrectEnable->setChecked(driver->timeCorrectEnable[curPort][curDevice]);
		ui->editTimeShiftMax->setText(QString::number(driver->timeShiftMax[curPort][curDevice]));
	}

	if ((curTreeIndex2 >= 0) && (curTreeIndex3 >= 0) && curDevice >= (DEV_NUM_MAX-1))
	{
		--driver->numDevices[curPort];
		//selectDevice(0, 0);

		ui->cbDeviceEnable->setChecked(driver->deviceEnable[curPort][curDevice]);
		ui->editDeviceName->setText(driver->deviceName[curPort][curDevice]);
		ui->editNumDevice->setText(QString::number(driver->numDevice[curPort][curDevice]));
		ui->cbTimeCorrectEnable->setChecked(driver->timeCorrectEnable[curPort][curDevice]);
		ui->editTimeShiftMax->setText(QString::number(driver->timeShiftMax[curPort][curDevice]));
	}

	if (currentItem)
	{
		deleteItem (currentItem);
		//currentItem = NULL;
	}
}


void MainWindow::newFile()
{
	for (int np = 0; np < driver->numPorts; ++np)
	{
		for (int nd = 0; nd < driver->numDevices[np]; ++nd)
		{
			driver->numDevice[np][nd] = 0;
			driver->deviceEnable[np][nd] = false;
			driver->deviceName[np][nd] = "";
			driver->timeCorrectEnable[np][nd] = false;
			driver->timeShiftMax[np][nd] = 30;
		}

		driver->portType[np] = 0;
		driver->numPort[np] = 0;
		driver->portEnable[np] = 0;
		driver->portName[np] = "";

		driver->addressIP[np] = "";
		driver->portIP[np] = 0;
		driver->timeoutTCP[np] = 0;

		driver->numDevices[np] = 0;
		driver->baudID[np] = 0;
		driver->baud[np] = 0;
		driver->data_bitsID[np] = 0;
		driver->data_bits[np] = 0;
		driver->stop_bitsID[np] = 0;
		driver->stop_bits[np] = 0;
		driver->parityID[np] = 0;
		driver->period[np] = 0;
	}
	driver->numPorts = 0;

	createTreeWidget();
	setWindowTitle(tr(driverName) + tr(" - ?????"));
	fileName = "";

	saveAction->setEnabled(false);
}


void MainWindow::openFile()
{
	fileName = QFileDialog::getOpenFileName();
	ui->statusBar->showMessage(fileName);

	readConfig(fileName);
	createTreeWidget();

	setWindowTitle(tr(driverName) + tr(" - ") + QFileInfo(fileName).fileName());
	saveAction->setEnabled(true);
	selectDriver();
}


void MainWindow::saveFile()
{
	if (fileName == "")
		saveFileAs();
	else
		saveConfig(fileName);
}


void MainWindow::saveFileAs()
{
	fileName = QFileDialog::getSaveFileName();
	saveConfig(fileName);
	saveAction->setEnabled(true);
}


void MainWindow::runDriver()
{
	readUI();

	//stopAction->setVisible(true);
	runAction->setEnabled(false);
	stopAction->setEnabled(true);	

	newAction->setEnabled(false);
	openAction->setEnabled(false);
	saveAction->setEnabled(false);
	saveAsAction->setEnabled(false);
	addAction->setEnabled(false);
	removeAction->setEnabled(false);

	ui->statusBar->showMessage(tr("RUN"));

	ui->tab_1->setEnabled(false);

	driver->initDriver();

	driverRun = true;
	driver->driverRun = true;

	driver->poll_device();
}


void MainWindow::stopDriver()
{
	runAction->setEnabled(true);
	stopAction->setEnabled(false);

	newAction->setEnabled(true);
	openAction->setEnabled(true);
	saveAction->setEnabled(true);
	saveAsAction->setEnabled(true);
	addAction->setEnabled(true);
	removeAction->setEnabled(true);

	ui->statusBar->showMessage(tr("STOP"));

	ui->tab_1->setEnabled(true);

	driver->destroyDriver();

	driverRun = false;
	driver->driverRun = false;

	updateTableWidget();
}


void MainWindow::readConfig(QString fileName)
{
	QSettings startup(QFileInfo( QCoreApplication::applicationFilePath() ).absolutePath() + "/startup.ini", QSettings::IniFormat);
	configName = QFileInfo( QCoreApplication::applicationFilePath() ).absolutePath() + "/" + startup.value("startup/config", "").toString();

	if (fileName == "")
		fileName = configName;

	this->fileName = fileName;

	QSettings config(fileName, QSettings::IniFormat);
	ui->statusBar->showMessage(config.value("driver/update", "").toString());

	driver->update = config.value("driver/update", "").toInt();
	driver->numPorts = config.value("driver/num_ports", "").toInt();

	for (int np = 0; np < driver->numPorts; ++np)
	{
		driver->portType[np] = config.value("port_" + QString::number(np) + "/type", "").toBool();
		driver->numPort[np] = config.value("port_" + QString::number(np) + "/port", "").toInt();
		driver->portEnable[np] = config.value("port_" + QString::number(np) + "/port_enable", "").toBool();
		driver->portName[np] = config.value("port_" + QString::number(np) + "/port_name", "").toString();

		driver->addressIP[np] = config.value("port_" + QString::number(np) + "/ip_address", "").toString();
		driver->portIP[np] = config.value("port_" + QString::number(np) + "/ip_port", "").toInt();
		driver->timeoutTCP[np] = config.value("port_" + QString::number(np) + "/ip_timeout", "").toInt();

		driver->numDevices[np] = config.value("port_" + QString::number(np) + "/num_devices", "").toInt();
		driver->baudID[np] = config.value("port_" + QString::number(np) + "/baud", "").toInt();
		driver->baud[np] = ui->cbBaud->itemText(driver->baudID[np]).toInt();
		driver->data_bitsID[np] = config.value("port_" + QString::number(np) + "/data_bits", "").toInt();
		driver->data_bits[np] = ui->cbDataBits->itemText(driver->data_bitsID[np]).toInt();
		driver->stop_bitsID[np] = config.value("port_" + QString::number(np) + "/stop_bits", "").toInt();
		driver->stop_bits[np] = ui->cbStopBits->itemText(driver->stop_bitsID[np]).toInt();
		driver->parityID[np] = config.value("port_" + QString::number(np) + "/parity", "").toInt();
		if (driver->parityID[np] == 0)  driver->parity[np] = 2;
		if (driver->parityID[np] == 1)  driver->parity[np] = 0;
		if (driver->parityID[np] == 2)  driver->parity[np] = 3;
		driver->period[np] = config.value("port_" + QString::number(np) + "/period", "").toInt();

		for (int nd = 0; nd < driver->numDevices[np]; ++nd)
		{
			driver->numDevice[np][nd] = config.value("port_" + QString::number(np) + "/device_" + QString::number(nd), "").toInt();
			driver->deviceEnable[np][nd] = config.value("port_" + QString::number(np) + "/device_" + QString::number(nd) + "_enable", "").toBool();
			driver->deviceName[np][nd] = config.value("port_" + QString::number(np) + "/device_" + QString::number(nd) + "_name", "").toString();
			driver->timeCorrectEnable[np][nd] = config.value("port_" + QString::number(np) + "/device_" + QString::number(nd) + "_tc_enable", "").toBool();
			driver->timeShiftMax[np][nd] = config.value("port_" + QString::number(np) + "/device_" + QString::number(nd) + "_time_shift", "").toInt();;
		}
	}
}


void MainWindow::saveConfig(QString fileName)
{
	QSettings config(fileName, QSettings::IniFormat);
	config.clear();

	config.setValue("driver/update", ui->editDriverUpdate->text());
	config.setValue("driver/num_ports", driver->numPorts);

	readUI();

	for (int np = 0; np < driver->numPorts; ++np)
	{
		config.setValue("port_" + QString::number(np) + "/port_enable", driver->portEnable[np]);
		config.setValue("port_" + QString::number(np) + "/port_name", driver->portName[np]);
		config.setValue("port_" + QString::number(np) + "/type", driver->portType[np]);
		config.setValue("port_" + QString::number(np) + "/ip_address", driver->addressIP[np]);
		config.setValue("port_" + QString::number(np) + "/ip_port", driver->portIP[np]);
		config.setValue("port_" + QString::number(np) + "/ip_timeout", driver->timeoutTCP[np]);
		config.setValue("port_" + QString::number(np) + "/port", driver->numPort[np]);
		config.setValue("port_" + QString::number(np) + "/baud", driver->baudID[np]);
		config.setValue("port_" + QString::number(np) + "/data_bits", driver->data_bitsID[np]);
		config.setValue("port_" + QString::number(np) + "/stop_bits", driver->stop_bitsID[np]);
		config.setValue("port_" + QString::number(np) + "/parity", driver->parityID[np]);
		config.setValue("port_" + QString::number(np) + "/period", driver->period[np]);
		config.setValue("port_" + QString::number(np) + "/num_devices", driver->numDevices[np]);

		for (int nd = 0; nd < driver->numDevices[np]; ++nd)
		{
			config.setValue("port_" + QString::number(np) + "/device_" + QString::number(nd), driver->numDevice[np][nd]);
			config.setValue("port_" + QString::number(np) + "/device_" + QString::number(nd) + "_enable", driver->deviceEnable[np][nd]);
			config.setValue("port_" + QString::number(np) + "/device_" + QString::number(nd) + "_name", driver->deviceName[np][nd]);
			config.setValue("port_" + QString::number(np) + "/device_" + QString::number(nd) + "_tc_enable", driver->timeCorrectEnable[np][nd]);
			config.setValue("port_" + QString::number(np) + "/device_" + QString::number(nd) + "_time_shift", driver->timeShiftMax[np][nd]);
		}
	}
	setWindowTitle(tr(driverName) + tr(" - ") + QFileInfo(fileName).fileName());
	ui->statusBar->showMessage(trUtf8("Файл ") + QFileInfo(fileName).fileName() + trUtf8(" сохранен."));
}

// Работа с треем
void MainWindow::createTrayIcon()
{
	trayIconMenu = new QMenu(this);
	trayIconMenu->addAction(restoreAction);
	trayIconMenu->addSeparator();
	trayIconMenu->addAction(quitAction);

	trayIcon = new QSystemTrayIcon(this);
	trayIcon->setContextMenu(trayIconMenu);
	trayIcon->setIcon(QIcon(":/icons/file/toolbox_16.png"));
	trayIcon->setToolTip(driverName);

	connect(trayIcon, SIGNAL(activated(QSystemTrayIcon::ActivationReason)),
			this, SLOT(iconActivated(QSystemTrayIcon::ActivationReason)));
}


void MainWindow::setIcon()
{
	QIcon icon = QIcon(":/icons/file/toolbox_16.png");
	trayIcon->setIcon(icon);
	setWindowIcon(icon);
}


void MainWindow::setVisible(bool visible)
{
	//minimizeAction->setEnabled(visible);
	restoreAction->setEnabled(isMaximized() || !visible);
	QMainWindow::setVisible(visible);
}


void MainWindow::closeEvent(QCloseEvent *event)
{
	//if (trayIcon->isVisible())
	if (driverRun)
	{
		hide();
		event->ignore();
		bVisible = false;
	}
}


void MainWindow::iconActivated(QSystemTrayIcon::ActivationReason reason)
{
	switch (reason)
	{
	case QSystemTrayIcon::Trigger:
		break;
	case QSystemTrayIcon::DoubleClick:
		if (bVisible)
		{
			hide();
			bVisible = false;
		}
		else
		{
			showNormal();
			bVisible = true;
		}
		break;
	default:
		break;
	}
}


// Работа с TreeWidget
void MainWindow::createTreeWidget()
{
	ui->treeWidget->clear();

	QTreeWidgetItem* ptwgItemDriver = new QTreeWidgetItem(ui->treeWidget);
	ptwgItemDriver->setText(0, driverName);

	QTreeWidgetItem* ptwgItemPort = 0;

	for (int np = 0; np < driver->numPorts; ++np)
	{
		ptwgItemPort = new QTreeWidgetItem(ptwgItemDriver);
		ptwgItemPort->setText(0, driver->portName[np]);

		QTreeWidgetItem* ptwgItemDevice = 0;

		for (int nd = 0; nd < driver->numDevices[np]; ++nd)
		{
			ptwgItemDevice = new QTreeWidgetItem(ptwgItemPort);
			ptwgItemDevice->setText(0, driver->deviceName[np][nd]);
		}
	}
}


void MainWindow::updateTreeWidget()
{
}


// Работа с TableWidget
void MainWindow::createTableWidget()
{
	QTableWidgetItem *newItem = new QTableWidgetItem( "" );
	int countRow;

	ui->tagView->clearContents();

	ui->tagView->clear();
	ui->tagView->setRowCount (0);
	ui->tagView->setColumnCount(6);
	ui->tagView->setSortingEnabled (false);

	QStringList columns;
	columns << "Symbol" << "Name" << "TagID" << "Value" << "Quality" << "TimeStamp";
	ui->tagView->setHorizontalHeaderLabels(columns);

	// Определение числа тегов
	countRow = 0;

	for (int np=0; np<driver->numPorts; ++np)
	{
		for (int nd=0; nd<driver->numDevices[np]; ++nd)
		{
			countRow += TAGS_IN_DEVICE;
		}
	}

	for (int rw=0; rw<countRow; ++rw)
	{
		ui->tagView->insertRow(rw);
		for (int column=0; column<6; ++column)
		{
			newItem = new QTableWidgetItem( "" );
			ui->tagView->setItem( rw, column, newItem );
		}
	}
	//ui->tagView->resizeRowsToContents();
	//ui->tagView->resizeColumnsToContents();
}


void MainWindow::updateTableWidget()
{
	QTableWidgetItem *newItem = new QTableWidgetItem( "" );
	int countRow=0, countRowBack;
	int firstTag=0;

	//Завершение работы при отключении всех клиентов
	if ((driver->driverRun == false) && (driverRun == true)) stopDriver();

	// Определение числа тегов
	if (bPage1)
	{
		for (int np=0; np<driver->numPorts; ++np)
		{
			for (int nd=0; nd<driver->numDevices[np]; ++nd)
			{
				countRow += TAGS_IN_DEVICE;
			}
		}
	}
	if (bPage2)
	{
		for (int nd=0; nd<driver->numDevices[curPort]; ++nd)
		{
			countRow += TAGS_IN_DEVICE;
		}
	}
	if (bPage3)
	{
		countRow += TAGS_IN_DEVICE;
	}

	countRowBack = ui->tagView->rowCount ();

	// Удаление лишних строк
	if (countRowBack > countRow)
	{
		for (int rw=countRowBack; rw>=countRow; --rw)
		{
			ui->tagView->removeRow(rw);
		}
	}

	// Добавление новых строк
	if (countRowBack < countRow)
	{
		for (int rw=countRowBack; rw<countRow; ++rw)
		{
			ui->tagView->insertRow(rw);
			for (int column=0; column<6; ++column)
			{
				newItem = new QTableWidgetItem( "" );
				ui->tagView->setItem( rw, column, newItem );
			}
		}
	}

	// Определение позиции первого тега
	for (int np=0; np<(curPort+1); ++np)
	{
		for (int nd=0; nd<driver->numDevices[np]; ++nd)
		{
			if((np==curPort) && (nd==curDevice)) break;
			firstTag += TAGS_IN_DEVICE;
		}
	}

	// Заполнение таблицы
	if (driverRun)
	{
		int rw=0;
		if (bPage1)
		{
			for (int np=0; np<driver->numPorts; ++np)
			{
				for (int nd=0; nd<driver->numDevices[np]; ++nd)
				{
					for (int nt=0; nt<TAGS_IN_DEVICE; ++nt, ++rw)
					{
						if ((driver->portEnable[np] == true) && (driver->deviceEnable[np][nd] == true))
						{
							ui->tagView->item(rw, 0)->setText( driver->Tag[rw % TAGS_IN_DEVICE].symbol );
							ui->tagView->item(rw, 1)->setText( driver->Tag[rw % TAGS_IN_DEVICE].name );
							ui->tagView->item(rw, 2)->setText( driver->Tags[np][nd][nt].name );
							ui->tagView->item(rw, 3)->setText( driver->Tags[np][nd][nt].value );

							switch (driver->Tags[np][nd][nt].status)
							{
								case 0:
									ui->tagView->item(rw, 4)->setText("UNCERTAIN");
									break;
								case 1:
									ui->tagView->item(rw, 4)->setText("GOOD");
									break;
								case 2:

									break;
							}
							ui->tagView->item(rw, 5)->setText( driver->Tags[np][nd][nt].timestamp );
						}
						else
						{
							ui->tagView->item(rw, 0)->setText( driver->Tag[rw % TAGS_IN_DEVICE].symbol );
							ui->tagView->item(rw, 1)->setText( driver->Tag[rw % TAGS_IN_DEVICE].name );
							ui->tagView->item(rw, 2)->setText("-");
							ui->tagView->item(rw, 3)->setText("-");
							ui->tagView->item(rw, 4)->setText("Disable");
							ui->tagView->item(rw, 5)->setText("-");
						}
					}
				}
			}
		}
		if (bPage2)
		{
			for (int nd=0; nd<driver->numDevices[curPort]; ++nd)
			{
				for (int nt=0; nt<TAGS_IN_DEVICE; ++nt, ++rw)
				{
					if ((driver->portEnable[curPort] == true) && (driver->deviceEnable[curPort][nd] == true))
					{
						ui->tagView->item(rw, 0)->setText( driver->Tag[rw % TAGS_IN_DEVICE].symbol );
						ui->tagView->item(rw, 1)->setText( driver->Tag[rw % TAGS_IN_DEVICE].name );
						ui->tagView->item(rw, 2)->setText( driver->Tags[curPort][nd][nt].name );
						ui->tagView->item(rw, 3)->setText( driver->Tags[curPort][nd][nt].value );

						switch (driver->Tags[curPort][nd][nt].status)
						{
							case 0:
								ui->tagView->item(rw, 4)->setText("UNCERTAIN");
								break;
							case 1:
								ui->tagView->item(rw, 4)->setText("GOOD");
								break;
							case 2:

								break;
						}
						ui->tagView->item(rw, 5)->setText( driver->Tags[curPort][nd][nt].timestamp );
					}
					else
					{
						ui->tagView->item(rw, 0)->setText( driver->Tag[rw % TAGS_IN_DEVICE].symbol );
						ui->tagView->item(rw, 1)->setText( driver->Tag[rw % TAGS_IN_DEVICE].name );
						ui->tagView->item(rw, 2)->setText("-");
						ui->tagView->item(rw, 3)->setText("-");
						ui->tagView->item(rw, 4)->setText("Disable");
						ui->tagView->item(rw, 5)->setText("-");
					}
				}
			}
		}
		if (bPage3)
		{
			for (int nt=0; nt<TAGS_IN_DEVICE; ++nt, ++rw)
			{
				if ((driver->portEnable[curPort] == true) && (driver->deviceEnable[curPort][curDevice] == true))
				{
					ui->tagView->item(rw, 0)->setText( driver->Tag[rw % TAGS_IN_DEVICE].symbol );
					ui->tagView->item(rw, 1)->setText( driver->Tag[rw % TAGS_IN_DEVICE].name );
					ui->tagView->item(rw, 2)->setText( driver->Tags[curPort][curDevice][nt].name );
					ui->tagView->item(rw, 3)->setText( driver->Tags[curPort][curDevice][nt].value );

					switch (driver->Tags[curPort][curDevice][nt].status)
					{
						case 0:
							ui->tagView->item(rw, 4)->setText("UNCERTAIN");
							break;
						case 1:
							ui->tagView->item(rw, 4)->setText("GOOD");
							break;
						case 2:

							break;
					}
					ui->tagView->item(rw, 5)->setText( driver->Tags[curPort][curDevice][nt].timestamp );
				}
				else
				{
					ui->tagView->item(rw, 0)->setText( driver->Tag[rw % TAGS_IN_DEVICE].symbol );
					ui->tagView->item(rw, 1)->setText( driver->Tag[rw % TAGS_IN_DEVICE].name );
					ui->tagView->item(rw, 2)->setText("-");
					ui->tagView->item(rw, 3)->setText("-");
					ui->tagView->item(rw, 4)->setText("Disable");
					ui->tagView->item(rw, 5)->setText("-");
				}
			}
		}
	}
	else
	{
		for (int rw=0; rw<countRow; ++rw)
		{
			ui->tagView->item(rw, 0)->setText( driver->Tag[rw % TAGS_IN_DEVICE].symbol );
			ui->tagView->item(rw, 1)->setText( driver->Tag[rw % TAGS_IN_DEVICE].name );
			ui->tagView->item(rw, 2)->setText("-");
			ui->tagView->item(rw, 3)->setText("-");
			ui->tagView->item(rw, 4)->setText("-");
			ui->tagView->item(rw, 5)->setText("-");
		}
	}
	//ui->logView->addItem(trUtf8("*** адын-адын-адын!!!"));
}


// Обработка радиобатонов
void MainWindow::on_rbCOM_clicked()
{
	ui->editNumPort->setEnabled(true);
	ui->cbBaud->setEnabled(true);
	ui->cbDataBits->setEnabled(true);
	ui->cbParity->setEnabled(true);
	ui->cbStopBits->setEnabled(true);

	ui->editAddressIP->setEnabled(false);
	ui->editPortIP->setEnabled(false);
	ui->editTimeoutTCP->setEnabled(false);
}


void MainWindow::on_rbTCP_clicked()
{
	ui->editNumPort->setEnabled(false);
	ui->cbBaud->setEnabled(false);
	ui->cbDataBits->setEnabled(false);
	ui->cbParity->setEnabled(false);
	ui->cbStopBits->setEnabled(false);

	ui->editAddressIP->setEnabled(true);
	ui->editPortIP->setEnabled(true);
	ui->editTimeoutTCP->setEnabled(true);
}


void MainWindow::on_treeWidget_clicked(const QModelIndex &index)
{
	QModelIndex pIndex = index.parent();
	QModelIndex ppIndex = pIndex.parent();

	backTreeIndex1 = curTreeIndex1;
	backTreeIndex2 = curTreeIndex2;
	backTreeIndex3 = curTreeIndex3;

	curTreeIndex1 = index.row();
	curTreeIndex2 = pIndex.row();
	curTreeIndex3 = ppIndex.row();

	if ((backTreeIndex3 <0) && (backTreeIndex2>=0)) backItem->setText(0, ui->editPortName->text());
	if ((backTreeIndex3>=0) && (backTreeIndex2>=0)) backItem->setText(0, ui->editDeviceName->text());

	if ((curTreeIndex3 <0) && (curTreeIndex2 <0)) selectDriver();
	if ((curTreeIndex3 <0) && (curTreeIndex2>=0)) selectPort(index.row());
	if ((curTreeIndex3>=0) && (curTreeIndex2>=0)) selectDevice(pIndex.row(), index.row());

	updateTableWidget();
}


void MainWindow::on_treeWidget_itemClicked(QTreeWidgetItem *item, int column)
{
	backItem = currentItem;
	currentItem = item;
	currentColumn = column;
}


void MainWindow::selectDriver()
{
	if(bPage2)
	{
		changePage2(curPort);
	}

	if(bPage3)
	{
		changePage3(curPort, curDevice);
	}

	ui->stackedWidget->setCurrentIndex(0);
	ui->editDriverUpdate->setText(QString::number(driver->update));

	if (!driverRun)
	{
		addAction->setEnabled(true);
		removeAction->setEnabled(false);
	}

	bPage1 = true;
	bPage2 = bPage3 = false;
	curPort=curDevice=0;
}


void MainWindow::selectPort(int np)
{
	if(bPage1)
	{
		changePage1();
	}

	if(bPage2)
	{
		changePage2(curPort);
	}

	if(bPage3)
	{
		changePage3(curPort, curDevice);
	}

	ui->stackedWidget->setCurrentIndex(1);
	ui->cbPortEnable->setChecked(driver->portEnable[np]);
	ui->editPortName->setText(driver->portName[np]);

	if (driver->portType[np] == 0)
	{
		ui->rbCOM->setChecked(true);
		on_rbCOM_clicked();
	}
	else
	{
		ui->rbTCP->setChecked(true);
		on_rbTCP_clicked();
	}

	ui->editPollPeriod->setText(QString::number(driver->period[np]));
	ui->editNumPort->setText(QString::number(driver->numPort[np]));
	ui->cbBaud->setCurrentIndex(driver->baudID[np]);
	ui->cbDataBits->setCurrentIndex(driver->data_bitsID[np]);
	ui->cbParity->setCurrentIndex(driver->parityID[np]);
	ui->cbStopBits->setCurrentIndex(driver->stop_bitsID[np]);

	ui->editAddressIP->setText(driver->addressIP[np]);
	ui->editPortIP->setText(QString::number(driver->portIP[np]));
	ui->editTimeoutTCP->setText(QString::number(driver->timeoutTCP[np]));

	if (!driverRun)
	{
		addAction->setEnabled(true);
		removeAction->setEnabled(true);
	}

	bPage2 = true;
	bPage1 = bPage3 = false;

	curPort=curDevice=0;
	curPort = np;
}


void MainWindow::selectDevice(int np, int nd)
{
	if(bPage1)
	{
		changePage1();
	}

	if(bPage2)
	{
		changePage2(curPort);
	}

	if(bPage3)
	{
		changePage3(curPort, curDevice);
	}

	ui->stackedWidget->setCurrentIndex(2);
	ui->cbDeviceEnable->setChecked(driver->deviceEnable[np][nd]);
	ui->editDeviceName->setText(driver->deviceName[np][nd]);
	ui->editNumDevice->setText(QString::number(driver->numDevice[np][nd]));
	ui->cbTimeCorrectEnable->setChecked(driver->timeCorrectEnable[np][nd]);
	ui->editTimeShiftMax->setText(QString::number(driver->timeShiftMax[np][nd]));

	if (!driverRun)
	{
		addAction->setEnabled(false);
		removeAction->setEnabled(true);
	}

	bPage3 = true;
	bPage1 = bPage2 = false;

	curPort = np;
	curDevice = nd;
}


void MainWindow::changePage1()
{
	driver->update = ui->editDriverUpdate->text().toInt();
}


void MainWindow::changePage2(int np)
{
	driver->portEnable[np] = ui->cbPortEnable->isChecked();
	driver->portName[np] = ui->editPortName->text();
	driver->period[np] = ui->editPollPeriod->text().toInt();
	driver->numPort[np] = ui->editNumPort->text().toInt();
	driver->addressIP[np] = ui->editAddressIP->text();
	driver->portIP[np] = ui->editPortIP->text().toInt();
	driver->timeoutTCP[np] = ui->editTimeoutTCP->text().toInt();

	driver->baudID[np] = ui->cbBaud->currentIndex();
	driver->baud[np] = ui->cbBaud->currentText().toInt();

	driver->data_bitsID[np] = ui->cbDataBits->currentIndex();
	driver->data_bits[np] = ui->cbDataBits->currentText().toInt();

	driver->stop_bitsID[np] = ui->cbStopBits->currentIndex();
	driver->stop_bits[np] = ui->cbStopBits->currentText().toInt();

	driver->portType[np] = ui->rbTCP->isChecked();
}


void MainWindow::changePage3(int np, int nd)
{
	driver->deviceEnable[np][nd] = ui->cbDeviceEnable->isChecked();
	driver->deviceName[np][nd] = ui->editDeviceName->text();
	driver->numDevice[np][nd] = ui->editNumDevice->text().toInt();
	driver->timeCorrectEnable[np][nd] = ui->cbTimeCorrectEnable->isChecked();
	driver->timeShiftMax[np][nd] = ui->editTimeShiftMax->text().toInt();
}


void MainWindow::deleteItem (QTreeWidgetItem *curItem)
{
	int iIndex;
	QTreeWidgetItem *parent = curItem->parent();

	if (parent)
	{
		iIndex = parent->indexOfChild(ui->treeWidget->currentItem());
		delete parent->takeChild(iIndex);

		ui->treeWidget->show();

		currentItem = ui->treeWidget->currentItem();

		QModelIndex index = ui->treeWidget->currentIndex();
		QModelIndex pIndex = index.parent();
		QModelIndex ppIndex = pIndex.parent();

		curTreeIndex1 = index.row();
		curTreeIndex2 = pIndex.row();
		curTreeIndex3 = ppIndex.row();

		if ((curTreeIndex3 <0) && (curTreeIndex2 <0)) selectDriver();
		if ((curTreeIndex3 <0) && (curTreeIndex2>=0)) selectPort(index.row());
		if ((curTreeIndex3>=0) && (curTreeIndex2>=0)) selectDevice(pIndex.row(), index.row());

		updateTableWidget();
	}
}


void MainWindow::insertItem (QTreeWidgetItem *parent, QString text)
{
	QTreeWidgetItem *newItem = new QTreeWidgetItem(parent);
	newItem->setText(0, text);
	if (parent->isExpanded()==false) parent->setExpanded(true);
}


void MainWindow::on_cbPortEnable_clicked()
{
	driver->portEnable[curPort] = ui->cbPortEnable->isChecked();
}


void MainWindow::on_cbDeviceEnable_clicked()
{
	driver->deviceEnable[curPort][curDevice] = ui->cbDeviceEnable->isChecked();
}


void MainWindow::on_cbTimeCorrectEnable_clicked()
{
	driver->timeCorrectEnable[curPort][curDevice] = ui->cbTimeCorrectEnable->isChecked();
}

void MainWindow::readUI()
{
	if(bPage1)
	{
		driver->update = ui->editDriverUpdate->text().toInt();
	}
	if(bPage2)
	{
		driver->portEnable[curPort] = ui->cbPortEnable->isChecked();
		driver->portName[curPort] = ui->editPortName->text();
		driver->period[curPort] = ui->editPollPeriod->text().toInt();
		driver->numPort[curPort] = ui->editNumPort->text().toInt();
		driver->addressIP[curPort] = ui->editAddressIP->text();
		driver->portIP[curPort] = ui->editPortIP->text().toInt();
		driver->timeoutTCP[curPort] = ui->editTimeoutTCP->text().toInt();
		driver->baudID[curPort] = ui->cbBaud->currentIndex();
		driver->baud[curPort] = ui->cbBaud->currentText().toInt();
		driver->data_bitsID[curPort] = ui->cbDataBits->currentIndex();
		driver->data_bits[curPort] = ui->cbDataBits->currentText().toInt();
		driver->stop_bitsID[curPort] = ui->cbStopBits->currentIndex();
		driver->stop_bits[curPort] = ui->cbStopBits->currentText().toInt();
		driver->portType[curPort] = ui->rbTCP->isChecked();
	}
	if(bPage3)
	{
		driver->deviceEnable[curPort][curDevice] = ui->cbDeviceEnable->isChecked();
		driver->deviceName[curPort][curDevice] = ui->editDeviceName->text();
		driver->numDevice[curPort][curDevice] = ui->editNumDevice->text().toInt();
		driver->timeCorrectEnable[curPort][curDevice] = ui->cbTimeCorrectEnable->isChecked();
		driver->timeShiftMax[curPort][curDevice] = ui->editTimeShiftMax->text().toInt();
	}
}

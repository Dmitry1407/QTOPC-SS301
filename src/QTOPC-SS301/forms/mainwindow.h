#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QtGui>
//#include <QXmlStreamReader>
//#include <QXmlStreamWriter>
#include <QMainWindow>
#include <QtGui/QMainWindow>
#include <QtCore/QCoreApplication>
#include <QTableWidgetItem>
#include <QFileDialog>
#include <QSettings>
#include <QClipboard>
#include <QtCore/QtGlobal>
#include <QtGui/QClipboard>

#include <QSystemTrayIcon>

#include "src/tdriver.h"

QT_BEGIN_NAMESPACE
class QSignalMapper;
class QTreeWidget;
class QListWidget;
QT_END_NAMESPACE

namespace Ui
{
    class MainWindow;
}

class MainWindow : public QMainWindow
{
	Q_OBJECT

public:
	explicit MainWindow(QWidget *parent = 0);
	void setVisible(bool visible);
	~MainWindow();

	TDriver *driver;
	//TDriver driver;

protected:
    void closeEvent(QCloseEvent *event);

private slots:
	void newFile();
	void openFile();
	void saveFile();
	void saveFileAs();
	void addItem();
	void removeItem();
	void runDriver();
	void stopDriver();

	void setIcon();
	void iconActivated(QSystemTrayIcon::ActivationReason reason);

	void on_treeWidget_clicked(const QModelIndex &index);
	void on_treeWidget_itemClicked(QTreeWidgetItem *, int);
	void selectDriver();
	void selectPort(int np);
	void selectDevice(int np, int nd);

	void on_rbCOM_clicked();
	void on_rbTCP_clicked();

	void updateTableWidget();
	void on_cbPortEnable_clicked();
	void on_cbDeviceEnable_clicked();

	void on_cbTimeCorrectEnable_clicked();

private:
	void createActions();
	void createToolBar();
	void createTrayIcon();

	void createTreeWidget();
	void updateTreeWidget();
	void deleteItem (QTreeWidgetItem *currentItem);	//удаление элемента из QTreeWidget
	void insertItem (QTreeWidgetItem *, QString);	//добавление элемента в QTreeWidget

	void createTableWidget();
	//void updateTableWidget();

	void readConfig(QString);
	void saveConfig(QString);

	void changePage1();
	void changePage2(int);
	void changePage3(int, int);
	void readUI();

	Ui::MainWindow *ui;

	QAction *newAction;
	QAction *openAction;
	QAction *saveAction;
	QAction *saveAsAction;
	QAction *addAction;
	QAction *removeAction;
	QAction *runAction;
	QAction *stopAction;

	QToolBar *mainToolBar;

	QTreeWidget *TreeWindows;
	QListWidget *PropertyEditor;

	QSignalMapper *windowMapper;
	QTreeWidgetItem *currentItem;	// текущий элемент, запоминается при клике в QTreeWidget
	QTreeWidgetItem *backItem;		// предыдущий элемент, запоминается при клике в QTreeWidget
	int currentColumn;				// номер столбца, на самом деле будет = 0, т.к. у нас 1 столбец

	QString fileName;
	QString configName;

	int curPort;
	int curDevice;
	int curTreeIndex1, curTreeIndex2, curTreeIndex3;
	int backTreeIndex1, backTreeIndex2, backTreeIndex3;

	bool bPage1, bPage2, bPage3;

	bool driverRun;
	bool bVisible;

	// организация работы в трее
	QAction *minimizeAction;
	QAction *restoreAction;
	QAction *quitAction;

	QSystemTrayIcon *trayIcon;
	QMenu *trayIconMenu;
};

#endif // MAINWINDOW_H

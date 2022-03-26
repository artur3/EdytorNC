/***************************************************************************
 *   Copyright (C) 2006-2022 by Artur Kozioł                               *
 *   artkoz78@gmail.com                                                    *
 *                                                                         *
 *   This file is part of EdytorNC.                                        *
 *                                                                         *
 *   EdytorNC is free software; you can redistribute it and/or modify      *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/



#include "spfsmainwindow.h"
#include "ui_SPFSMainWindow.h"
#include <QDebug>


SPFSMainWindow::SPFSMainWindow(QWidget *parent) : QMainWindow(parent), ui(new Ui::SPFSMainWindow)
{
    setAttribute(Qt::WA_DeleteOnClose);
    ui->setupUi(this);

    setWindowModality(Qt::NonModal);

    setWindowIcon(QIcon(":/images/serial.svg"));

    setWindowTitle(tr("EdytorNC - Serial port file server"));

    windowMapper = new QSignalMapper(this);
    connect(windowMapper, SIGNAL(mapped(QWidget *)), this, SLOT(setActiveSubWindow(QWidget *)));
    connect(ui->mdiArea, SIGNAL(subWindowActivated(QMdiSubWindow *)), this, SLOT(activeWindowChanged(QMdiSubWindow *)));

    closable = false;
    ui->mdiArea->setViewMode(QMdiArea::TabbedView);
    QTabBar* tab = ui->mdiArea->findChild<QTabBar*>();
    if(tab)
    {
        //connect(tab, SIGNAL(tabCloseRequested(int)), this, SLOT(closeTab(int)));
        //tab->setTabsClosable(true);
        // The tabs might be very wide
        tab->setExpanding(false);
    };

    createActions();

    createTrayIcon();


    loadSettings();

    if(startMinimizedAction->isChecked())
        QTimer::singleShot(500, this, SLOT(hide()));
    else
        showNormal();

    changeActiveWindow();
}

//**************************************************************************************************
//
//**************************************************************************************************

SPFSMainWindow::~SPFSMainWindow()
{
    delete ui;
}

//**************************************************************************************************
//
//**************************************************************************************************

void SPFSMainWindow::closeEvent(QCloseEvent *event)
{
    if(closable)
    {
        event->accept();
    }
    else
    {
        if(trayIcon->isVisible())
        {
            QMessageBox::information(this, tr("EdytorNC - Serial port file server"),
                                     tr("The program will keep running in the "
                                        "system tray. To terminate the program, "
                                        "choose <b>Quit</b> in the context menu "
                                        "of the system tray entry."));
            hide();
            event->ignore();
        };
    };
}

//**************************************************************************************************
//
//**************************************************************************************************

void SPFSMainWindow::saveSettings()
{
    QStringList activeConfigs;

    foreach(QMdiSubWindow *window, ui->mdiArea->subWindowList())
    {
        SerialTransmissionDialog *mdiChild = qobject_cast<SerialTransmissionDialog *>(window->widget());
        if(mdiChild > NULL)
            activeConfigs.append(mdiChild->configName());
    };

    QSettings settings("EdytorNC", "EdytorNC");
    settings.beginGroup("SPFSMainWindow");
    settings.setValue("StartMinimized", startMinimizedAction->isChecked());
    settings.remove("ActiveConfigs");
    activeConfigs.removeDuplicates();
    activeConfigs.removeAll("");
    settings.setValue("ActiveConfigs", activeConfigs);
    settings.endGroup();
}

//**************************************************************************************************
//
//**************************************************************************************************

void SPFSMainWindow::loadSettings()
{
    QSettings settings("EdytorNC", "EdytorNC");
    settings.beginGroup("SPFSMainWindow");

    startMinimizedAction->setChecked(settings.value("StartMinimized", false).toBool());

    QStringList activeConfigs = settings.value("ActiveConfigs", (QStringList() << "")).toStringList();
    activeConfigs.removeDuplicates();
    activeConfigs.removeAll("");
    settings.endGroup();

    QStringList list = activeConfigs;
    QStringList::Iterator it = list.begin();
    while(it != list.end())
    {
        SerialTransmissionDialog *spServer = createSerialPortServer(*it);
        spServer->showMaximized(); //show();
        ++it;
    };
}

//**************************************************************************************************
//
//**************************************************************************************************

SerialTransmissionDialog *SPFSMainWindow::createSerialPortServer(QString __config)
{
    SerialTransmissionDialog *spServer = new SerialTransmissionDialog(this, Qt::SubWindow, true);  // Qt::Tool | Qt::CustomizeWindowHint | Qt::WindowTitleHint
    spServer->setAttribute(Qt::WA_DeleteOnClose);
    ui->mdiArea->addSubWindow(spServer);

    spServer->startFileServer(__config);

    return spServer;
}

//**************************************************************************************************
//
//**************************************************************************************************

void SPFSMainWindow::startSerialPortServer()
{
    QString _config = configBox->currentText();

    SerialTransmissionDialog *_existing = findMdiChild(_config);
    if(_existing)
        _existing->setFocus();
    else
    {
        SerialTransmissionDialog *spServer = createSerialPortServer(_config);
        //spServer->startFileServer(_config, true);
        spServer->show();
    };
}

//**************************************************************************************************
//
//**************************************************************************************************

void SPFSMainWindow::stopSerialPortServer()
{
    QString _config = configBox->currentText();

    SerialTransmissionDialog *_existing = findMdiChild(_config);
    if(_existing > NULL)
    {
        _existing->close();
        _existing->parentWidget()->close();
    };

    changeActiveWindow();
}

//**************************************************************************************************
//
//**************************************************************************************************

void SPFSMainWindow::createActions()
{
    configPortAct = new QAction(QIcon(":/images/serialconfig.png"), tr("Serial port configuration"), this);
    //configPortAct->setShortcut(tr("F3"));
    configPortAct->setToolTip(tr("Serial port configuration"));
    connect(configPortAct, SIGNAL(triggered()), this, SLOT(serialConfig()));

    startServerAct = new QAction(QIcon(":/images/connect_creating.png"), tr("&Start"), this);
    //receiveAct->setShortcut(tr("Shift+F3"));
    startServerAct->setToolTip(tr("Start"));
    connect(startServerAct, SIGNAL(triggered()), this, SLOT(startSerialPortServer()));

    stopServerAct = new QAction(QIcon(":/images/stop.png"), tr("S&top"), this);
    //sendAct->setShortcut(tr("F3"));
    stopServerAct->setToolTip(tr("Stop"));
    connect(stopServerAct, SIGNAL(triggered()), this, SLOT(stopSerialPortServer()));


    resetServerAct = new QAction(QIcon(":/images/reset.svg"), tr("&Reset"), this);
    resetServerAct->setToolTip(tr("Reset serial port"));
    connect(resetServerAct, SIGNAL(triggered()), this, SLOT(doPortReset()));


    closeServerAct = new QAction(QIcon(":/images/window-close.svg"), tr("&Minimize to tray"), this);
    closeServerAct->setToolTip(tr("Minimize to system tray"));
    connect(closeServerAct, SIGNAL(triggered()), this, SLOT(hide()));

    browseSaveFolderAct = new QAction(QIcon(":/images/browse.png"), tr("&Browse save folder"), this);
    browseSaveFolderAct->setToolTip(tr("Browse save folder"));
    connect(browseSaveFolderAct, SIGNAL(triggered()), this, SLOT(browseSaveFolder()));

    showNewFilesAct = new QAction(QIcon(":/images/download.svg"), tr("&Show saved files"), this);
    showNewFilesAct->setToolTip(tr("Show saved files"));
    connect(showNewFilesAct, SIGNAL(triggered()), this, SLOT(showNewFiles()));

    aboutAction = new QAction(tr("&About"), this);
    aboutAction->setToolTip(tr("Show the application's About box"));
    connect(aboutAction, SIGNAL(triggered()), this, SLOT(about()));

    aboutQtAct = new QAction(tr("About &Qt"), this);
    aboutQtAct->setToolTip(tr("Show the Qt library's About box"));
    connect(aboutQtAct, SIGNAL(triggered()), qApp, SLOT(aboutQt()));

    minimizeAction = new QAction(tr("Mi&nimize"), this);
    connect(minimizeAction, SIGNAL(triggered()), this, SLOT(hide()));

    maximizeAction = new QAction(tr("Ma&ximize"), this);
    connect(maximizeAction, SIGNAL(triggered()), this, SLOT(showMaximized()));

    restoreAction = new QAction(tr("&Restore"), this);
    connect(restoreAction, SIGNAL(triggered()), this, SLOT(showNormal()));

    quitAction = new QAction(QIcon(":/images/exit.svg"), tr("&Quit"), this);
    connect(quitAction, SIGNAL(triggered()), this, SLOT(quitApp()));

    startMinimizedAction = new QAction(tr("Start minimized"), this);
    startMinimizedAction->setCheckable(true);
    startMinimizedAction->setChecked(false);

    configBox = new QComboBox();
    configBox->setSizeAdjustPolicy(QComboBox::AdjustToContents);
    configBox->setDuplicatesEnabled(false);

    connect(configBox, SIGNAL(currentIndexChanged(int)), SLOT(changeActiveWindow()));

    ui->toolBar->addWidget(configBox);
    ui->toolBar->addAction(configPortAct);
    ui->toolBar->addSeparator();
    ui->toolBar->addAction(startServerAct);
    ui->toolBar->addAction(stopServerAct);

    ui->toolBar->addSeparator();
    ui->toolBar->addAction(resetServerAct);

    fileToolBar = new QToolBar(tr("FileToolBar"));
    addToolBar(Qt::TopToolBarArea, fileToolBar);
    fileToolBar->setObjectName("FileToolBar");


    fileToolBar->addAction(browseSaveFolderAct);
    fileToolBar->addAction(showNewFilesAct);
    fileToolBar->addSeparator();
    fileToolBar->addAction(closeServerAct);

    ui->menu_File->addAction(startMinimizedAction);
    ui->menu_File->addSeparator();
//    ui->menu_File->addAction(browseSaveFolderAct);
//    ui->menu_File->addAction(showNewFilesAct);
//    ui->menu_File->addSeparator();
    ui->menu_File->addAction(closeServerAct);
    ui->menu_File->addSeparator();
    ui->menu_File->addAction(quitAction);

    ui->menu_Help->addAction(aboutAction);


    loadSerialConfignames();
    configBox->adjustSize();
    //updateCurrentSerialConfig();
}

//**************************************************************************************************
//
//**************************************************************************************************

void SPFSMainWindow::serialConfig()
{
    SerialPortConfigDialog *serialConfigDialog = new SerialPortConfigDialog(this, configBox->currentText());

    if(serialConfigDialog->exec() == QDialog::Accepted)
    {
        loadSerialConfignames();
        configBox->adjustSize();
    };
}

//**************************************************************************************************
//
//**************************************************************************************************

void SPFSMainWindow::loadSerialConfignames()
{
    int id;
    QStringList list;
    QString item;

    QSettings settings("EdytorNC", "EdytorNC");
    settings.beginGroup("SerialPortConfigs");

    configBox->clear();
    list = settings.value("SettingsList", QStringList(tr("Default"))).toStringList();
    list.sort();
    configBox->addItems(list);
    item = settings.value("CurrentSerialPortSettings", tr("Default")).toString();
    id = configBox->findText(item);
    if(id >= 0)
        configBox->setCurrentIndex(id);

    settings.endGroup();
}

//**************************************************************************************************
//
//**************************************************************************************************

//void SPFSMainWindow::updateCurrentSerialConfig()
//{
//    int id;
//    QDir dir;

//    bool hasMdiChild = (activeMdiChild() != NULL);
//    if(hasMdiChild && (serialToolBar > NULL))
//    {
//        dir.setPath(activeMdiChild()->filePath());
//        dir.setFilter(QDir::Files | QDir::Hidden | QDir::NoSymLinks);
//        dir.setSorting(QDir::Name);
//        dir.setNameFilters(QStringList("*.ini"));

//        QFileInfoList list = dir.entryInfoList();

//        if(!list.isEmpty())
//        {
//            QFileInfo name = list.at(0);
//            id = configBox->findText(name.baseName());
//            if(id >= 0)
//                configBox->setCurrentIndex(id);
//        };
//    };
//}

//**************************************************************************************************
//
//**************************************************************************************************

void SPFSMainWindow::tileSubWindowsVertycally()
{
//    if(ui->mdiArea->subWindowList().isEmpty())
//        return;

//    QPoint position(0, 0);
//    foreach(QMdiSubWindow *window, ui->mdiArea->subWindowList())
//    {
//        QRect rect(0, 0, ui->mdiArea->width(),
//                   ui->mdiArea->height() / ui->mdiArea->subWindowList().count());
//        window->setGeometry(rect);
//        window->move(position);
//        position.setY(position.y() + window->height());
//    }
}

//**************************************************************************************************
//
//**************************************************************************************************

SerialTransmissionDialog *SPFSMainWindow::findMdiChild(const QString __config)
{
    foreach(QMdiSubWindow *window, ui->mdiArea->subWindowList())
    {
        SerialTransmissionDialog *mdiChild = qobject_cast<SerialTransmissionDialog *>(window->widget());
        if(mdiChild > NULL)
            if(mdiChild->configName() == __config)
                return mdiChild;
    };
    return NULL;
}

//**************************************************************************************************
//
//**************************************************************************************************

void SPFSMainWindow::setActiveSubWindow(QWidget *window)
{
    if(!window)
        return;
    ui->mdiArea->setActiveSubWindow(qobject_cast<QMdiSubWindow *>(window));
}

//**************************************************************************************************
//
//**************************************************************************************************

SerialTransmissionDialog *SPFSMainWindow::activeMdiChild()
{
    if(QMdiSubWindow *activeSubWindow = ui->mdiArea->activeSubWindow())
    {
        return qobject_cast<SerialTransmissionDialog *>(activeSubWindow->widget());
    }
    else
    {
        SerialTransmissionDialog *mdiChild = findMdiChild(configBox->currentText());

        if(mdiChild > NULL)
        {
           return mdiChild;
        };
    };

    return NULL;
}

//**************************************************************************************************
//
//**************************************************************************************************

void SPFSMainWindow::activeWindowChanged(QMdiSubWindow *window)
{
    if(!window)
        return;

    SerialTransmissionDialog *mdiChild = qobject_cast<SerialTransmissionDialog *>(window->widget()); //activeMdiChild();

    if(mdiChild > NULL)
    {
        mdiChild->setFocus();

        if(configBox)
        {
            int id = configBox->findText(mdiChild->configName());
            if(id >= 0)
                configBox->setCurrentIndex(id);
        };

        configPortAct->setEnabled(false);
        startServerAct->setEnabled(false);
        resetServerAct->setEnabled(true);
        stopServerAct->setEnabled(true);
        browseSaveFolderAct->setEnabled(true);
        showNewFilesAct->setEnabled(true);
    }
    else
    {
        configPortAct->setEnabled(true);
        startServerAct->setEnabled(true);
        resetServerAct->setEnabled(false);
        stopServerAct->setEnabled(false);
        browseSaveFolderAct->setEnabled(false);
        showNewFilesAct->setEnabled(false);
    };

}

//**************************************************************************************************
//
//**************************************************************************************************

void SPFSMainWindow::changeActiveWindow()
{
    SerialTransmissionDialog *mdiChild = findMdiChild(configBox->currentText());

    if(mdiChild > NULL)
    {
        mdiChild->setFocus();
        configPortAct->setEnabled(false);
        startServerAct->setEnabled(false);
        resetServerAct->setEnabled(true);
        stopServerAct->setEnabled(true);
        browseSaveFolderAct->setEnabled(true);
        showNewFilesAct->setEnabled(true);
    }
    else
    {
        configPortAct->setEnabled(true);
        startServerAct->setEnabled(true);
        resetServerAct->setEnabled(false);
        stopServerAct->setEnabled(false);
        browseSaveFolderAct->setEnabled(false);
        showNewFilesAct->setEnabled(false);
    };
}

//**************************************************************************************************
//
//**************************************************************************************************

void SPFSMainWindow::closeTab(int i)
{
    QTabBar* tab = ui->mdiArea->findChild<QTabBar*>();
    if(tab > NULL)
    {
        tab->removeTab(i);
    };
}

//**************************************************************************************************
//
//**************************************************************************************************

void SPFSMainWindow::doPortReset()
{
    SerialTransmissionDialog *mdiChild = activeMdiChild();
    if(mdiChild > NULL)
        mdiChild->portReset();
}

//**************************************************************************************************
//
//**************************************************************************************************

void SPFSMainWindow::showNewFiles()
{
    ui->mdiArea->setFocus();
    SerialTransmissionDialog *mdiChild = activeMdiChild();

    if(mdiChild > NULL)
    {
        if((!mdiChild->savePath().isEmpty() && !mdiChild->readPaths().isEmpty()))
        {
            QSettings settings("EdytorNC", "EdytorNC");
            QStringList extensions = settings.value("Extensions", "").toStringList();
            extensions.removeDuplicates();
            extensions.sort();

            FileChecker *fileCheck = new FileChecker(this);
            fileCheck->setData(mdiChild->savePath(), mdiChild->readPaths(), extensions);
            fileCheck->findFiles();
            fileCheck->exec();
        };
    };

}

//**************************************************************************************************
//
//**************************************************************************************************

void SPFSMainWindow::browseSaveFolder()
{
    SerialTransmissionDialog *mdiChild = activeMdiChild();
    if(mdiChild > NULL)
        QDesktopServices::openUrl(QUrl(QString("file:///%1").arg(mdiChild->savePath()), QUrl::TolerantMode));
}

//**************************************************************************************************
//
//**************************************************************************************************

void SPFSMainWindow::createTrayIcon()
{
    trayIconMenu = new QMenu(this);
    trayIconMenu->addAction(minimizeAction);
    trayIconMenu->addAction(maximizeAction);
    trayIconMenu->addAction(restoreAction);
    trayIconMenu->addSeparator();
    trayIconMenu->addAction(aboutAction);
    trayIconMenu->addSeparator();
    trayIconMenu->addAction(quitAction);

    trayIcon = new QSystemTrayIcon(this);
    trayIcon->setContextMenu(trayIconMenu);
    connect(trayIcon, SIGNAL(activated(QSystemTrayIcon::ActivationReason)), this, SLOT(iconActivated(QSystemTrayIcon::ActivationReason)));

    trayIcon->setIcon(QIcon(":/images/serial.svg"));
    trayIcon->show();
}

//**************************************************************************************************
//
//**************************************************************************************************

void SPFSMainWindow::iconActivated(QSystemTrayIcon::ActivationReason reason)
{
    switch(reason)
    {
        case QSystemTrayIcon::Trigger:
            if(isVisible())
                hide();
            else
                showNormal();
            break;
        case QSystemTrayIcon::DoubleClick:
            break;
        case QSystemTrayIcon::MiddleClick:
            break;
        default:
            ;
    };
}

//**************************************************************************************************
//
//**************************************************************************************************

void SPFSMainWindow::quitApp()
{
    saveSettings();

    foreach(const QMdiSubWindow *window, ui->mdiArea->subWindowList(QMdiArea::StackingOrder))
    {
        SerialTransmissionDialog *mdiChild = qobject_cast<SerialTransmissionDialog *>(window->widget());
        if(mdiChild != NULL)
        {
            mdiChild->close();
            mdiChild->parentWidget()->close();
        }
    };

    closable = true;
    close();
}

//**************************************************************************************************
//
//**************************************************************************************************

void SPFSMainWindow::about()
{
    QMessageBox::about(this, tr("About EdytorNC - Serial port file server"),
                       tr("The <b>EdytorNC</b> is text editor for CNC programmers.") +
//                       tr("<P>Version: same as EdytorNC") +
                       tr("<P>Version: %1.%2.%3").arg(VERSION_MAJOR).arg(VERSION_MINOR).arg(VERSION_BUILD) +
                       tr("<P>Copyright (C) 1998 - 2022 by <a href=\"mailto:artkoz78@gmail.com\">Artur Kozioł</a>") +
                       tr("<P>") +
                       tr("<P>EdytorNC contains pieces of code from other Open Source projects.") +
                       tr("<P>") +
                       tr("<P><i>EdytorNC - Serial port file server is free software; you can redistribute it and/or modify"
                              "it under the terms of the GNU General Public License  as published by"
                              "the Free Software Foundation; either version 2 of the License, or"
                              "(at your option) any later version.</i>") +
                       tr("<P><i>The program is provided AS IS with NO WARRANTY OF ANY KIND,"
                              "INCLUDING THE WARRANTY OF DESIGN,"
                              "MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.</i>"));
}

//**************************************************************************************************
//
//**************************************************************************************************

void SPFSMainWindow::messReceived(const QString &text)
{
    if(text == "Show")
        showNormal();

//    emit needToShow();
}

//**************************************************************************************************
//
//**************************************************************************************************



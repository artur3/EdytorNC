/***************************************************************************
*   MIT License
*
*   Orginal author:
*   QFtpServer Copyright (c) 2020 Alexander Almaleh
*   https://github.com/sashoalm/QFtpServer
*
*   Permission is hereby granted, free of charge, to any person obtaining a copy
*   of this software and associated documentation files (the "Software"), to deal
*   in the Software without restriction, including without limitation the rights
*   to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
*   copies of the Software, and to permit persons to whom the Software is
*   furnished to do so, subject to the following conditions:
*
*   The above copyright notice and this permission notice shall be included in all
*   copies or substantial portions of the Software.
*
*   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
*   IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
*   FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
*   AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
*   LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
*   OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
*   SOFTWARE.
****************************************************************************/

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



#include "ftpserverwindow.h"
#include "ui_ftpserverwindow.h"
#include "ftpserver.h"


#include <QCoreApplication>
#include <QSettings>
#include <QFileDialog>

static FTPserverWindow *mainWindow = 0;

static void myMessageOutput(QtMsgType type, const QMessageLogContext &/*context*/, const QString &msg)
{
    mainWindow->appendText(type, msg);

}

//**************************************************************************************************
//
//**************************************************************************************************

FTPserverWindow::FTPserverWindow(QWidget *parent): QMainWindow(parent), ui(new Ui::FTPserverWindow)
{
    setAttribute(Qt::WA_DeleteOnClose);
    ui->setupUi(this);

    setWindowModality(Qt::NonModal);

    mainWindow = this;
    qInstallMessageHandler(myMessageOutput);

    // Set window icon.
    setWindowIcon(QIcon(":/images/ftp.svg"));

    ui->textEdit->setReadOnly(true);

    createActions();

    createTrayIcon();

    closable = false;

    server = 0;
    startServer();




    if(startMinimizedAction->isChecked())
        QTimer::singleShot(500, this, SLOT(hide()));
    else
        showNormal();
}

//**************************************************************************************************
//
//**************************************************************************************************

FTPserverWindow::~FTPserverWindow()
{
    saveSettings();
    delete ui;
}

//**************************************************************************************************
//
//**************************************************************************************************

void FTPserverWindow::showExpanded()
{
    show();
}

//**************************************************************************************************
//
//**************************************************************************************************

void FTPserverWindow::loadSettings()
{
    // UNIX-derived systems such as Linux and Android don't allow access to
    // port 21 for non-root programs, so we will use port 2121 instead.
    QString defaultPort;
#if defined(Q_OS_UNIX)
    defaultPort = "2121";
#else
    defaultPort = "21";
#endif


    QSettings settings("EdytorNC", "EdytorNC");

    settings.beginGroup("FTPConfigs");

    lineEditPort = settings.value("Port", defaultPort).toString();
    lineEditUserName = settings.value("Username", "EDYTORNC").toString();
    lineEditPassword = settings.value("Password", "CNC2022").toString();
    lineEditRootPath = settings.value("RootPath", QDir::rootPath()).toString();
    checkBoxAnonymous = settings.value("Anonymous", false).toBool();
    checkBoxReadOnly = settings.value("Readonly", false).toBool();
    checkBoxOnlyOneIpAllowed = settings.value("OneIp", true).toBool();



//    ui->lineEditPort->setText(settings.value("FTPserver/port", defaultPort).toString());
//    ui->lineEditUserName->setText(settings.value("FTPserver/username", "admin").toString());
//    ui->lineEditPassword->setText(settings.value("FTPserver/password", "qt").toString());
//    ui->lineEditRootPath->setText(settings.value("FTPserver/rootpath", QDir::rootPath()).toString());
//    ui->checkBoxAnonymous->setChecked(settings.value("FTPserver/anonymous", false).toBool());
//    ui->checkBoxReadOnly->setChecked(settings.value("FTPserver/readonly", false).toBool());
//    ui->checkBoxOnlyOneIpAllowed->setChecked(settings.value("FTPserver/oneip", true).toBool());

}

//**************************************************************************************************
//
//**************************************************************************************************
void FTPserverWindow::saveSettings()
{
//    QSettings settings("EdytorNC", "EdytorNC");
//    settings.setValue("FTPserver/port", ui->lineEditPort->text());
//    settings.setValue("FTPserver/username", ui->lineEditUserName->text());
//    settings.setValue("FTPserver/password", ui->lineEditPassword->text());
//    settings.setValue("FTPserver/rootpath", ui->lineEditRootPath->text());
//    settings.setValue("FTPserver/anonymous", ui->checkBoxAnonymous->isChecked());
//    settings.setValue("FTPserver/readonly", ui->checkBoxReadOnly->isChecked());
//    settings.setValue("FTPserver/oneip", ui->checkBoxOnlyOneIpAllowed->isChecked());
}

//**************************************************************************************************
//
//**************************************************************************************************

void FTPserverWindow::startServer()
{
    QString userName;
    QString password;

    loadSettings();

    if (!checkBoxAnonymous)
    {
        userName = lineEditUserName;
        password = lineEditPassword;
    }

    delete server;

    server = new FtpServer(this, lineEditRootPath, lineEditPort.toInt(), userName,
                           password, checkBoxReadOnly, checkBoxOnlyOneIpAllowed);

    connect(server, SIGNAL(newPeerIp(QString)), SLOT(onPeerIpChanged(QString)));

    if (server->isListening())
    {
        ui->statusBar->showMessage(tr("Listening at: ") + FtpServer::lanIp());
        closable = false;
    }
    else
    {
        ui->statusBar->showMessage(tr("Not listening."));
        closable = true;
    }
}

//**************************************************************************************************
//
//**************************************************************************************************

void FTPserverWindow::pushButtonRestartServer_clicked()
{
    startServer();
}

//**************************************************************************************************
//
//**************************************************************************************************

void FTPserverWindow::toolButtonBrowse_clicked()
{
//    QString rootPath;

//    rootPath = QFileDialog::getExistingDirectory(this, QString(), ui->lineEditRootPath->text());

//    if (rootPath.isEmpty())
//    {
//        return;
//    }

//    ui->lineEditRootPath->setText(rootPath);
}

//**************************************************************************************************
//
//**************************************************************************************************

void FTPserverWindow::onPeerIpChanged(const QString &peerIp)
{
    ui->statusBar->showMessage(tr("Connected to: ") + peerIp);

    qDebug() << "+++++ Connected to: " << peerIp;
}

//**************************************************************************************************
//
//**************************************************************************************************

void FTPserverWindow::pushButtonExit_clicked()
{
    hide();
}

//**************************************************************************************************
//
//**************************************************************************************************

void FTPserverWindow::appendText(QtMsgType type, const QString &text)
{
    switch (type) {
    case QtDebugMsg:
        if(text.contains("*****"))
            ui->textEdit->setTextColor(Qt::blue);
        else
            ui->textEdit->setTextColor(Qt::darkGreen);

        if(text.contains("+++++"))
            ui->textEdit->setTextColor(Qt::darkCyan);

        if(text.contains("reply"))
            ui->textEdit->setTextColor(Qt::darkRed);

        if(text.contains("command"))
            ui->textEdit->setTextColor(Qt::red);

        break;
    case QtWarningMsg:
        ui->textEdit->setTextColor(Qt::gray);
        break;
    default:
        ui->textEdit->setTextColor(Qt::black);
        break;
    }


    if(ui->textEdit->document()->blockCount() > 2000)  // too many lines may not be good
    {
        ui->textEdit->clear();
    };


    QString dateTime = QDate::currentDate().toString("yyyy-MM-dd") + " " + QTime::currentTime().toString("HH:mm:ss").remove(QRegularExpression(" \\w+"));

    ui->textEdit->append(dateTime + "  " + text);
    ui->textEdit->moveCursor(QTextCursor::End);
    ui->textEdit->ensureCursorVisible();
}

//**************************************************************************************************
//
//**************************************************************************************************

void FTPserverWindow::createActions()
{
    closeServerAct = new QAction(QIcon(":/images/window-close.svg"), tr("&Minimize to tray"), this);
    closeServerAct->setToolTip(tr("Minimize to system tray"));
    connect(closeServerAct, SIGNAL(triggered()), this, SLOT(hide()));

    aboutAction = new QAction(tr("&About"), this);
    aboutAction->setToolTip(tr("Show the application's About box"));
    connect(aboutAction, SIGNAL(triggered()), this, SLOT(about()));

    minimizeAction = new QAction(tr("Mi&nimize"), this);
    connect(minimizeAction, SIGNAL(triggered()), this, SLOT(hide()));

    maximizeAction = new QAction(tr("Ma&ximize"), this);
    connect(maximizeAction, SIGNAL(triggered()), this, SLOT(showMaximized()));

    restoreAction = new QAction(tr("&Restore"), this);
    connect(restoreAction, SIGNAL(triggered()), this, SLOT(showNormal()));

    quitAction = new QAction(QIcon(":/images/exit.svg"), tr("&Quit"), this);
    connect(quitAction, SIGNAL(triggered()), this, SLOT(quitApp()));

    resetServerAct = new QAction(QIcon(":/images/reset.svg"), tr("&Reset"), this);
    resetServerAct->setToolTip(tr("Restart server"));
    connect(resetServerAct, SIGNAL(triggered()), this, SLOT(pushButtonRestartServer_clicked()));

    configPortAct = new QAction(QIcon(":/images/serialconfig.png"), tr("Serial port configuration"), this);
    configPortAct->setToolTip(tr("Serial port configuration"));
    connect(configPortAct, SIGNAL(triggered()), this, SLOT(configPushButton_clicked()));


    browseSaveFolderAct = new QAction(QIcon(":/images/browse.png"), tr("&Browse save folder"), this);
    browseSaveFolderAct->setToolTip(tr("Browse save folder"));
    connect(browseSaveFolderAct, SIGNAL(triggered()), this, SLOT(browseSaveFolder()));

    showNewFilesAct = new QAction(QIcon(":/images/download.svg"), tr("&Show saved files"), this);
    showNewFilesAct->setToolTip(tr("Show saved files"));
    connect(showNewFilesAct, SIGNAL(triggered()), this, SLOT(showNewFiles()));



    connect(ui->clearToolButton, SIGNAL(clicked(bool)), this, SLOT(clearToolButton_clicked()));

    startMinimizedAction = new QAction(tr("Start minimized"), this);
    startMinimizedAction->setCheckable(true);
    startMinimizedAction->setChecked(false);


    configBox = new QComboBox();
    configBox->setSizeAdjustPolicy(QComboBox::AdjustToContents);
    configBox->setDuplicatesEnabled(false);

    connect(configBox, SIGNAL(currentIndexChanged(int)), SLOT(changeActiveWindow()));


    //ui->toolBar->setStyleSheet("QToolBar{spacing:25px;}");


    ui->toolBar->addAction(configPortAct);
    ui->toolBar->addSeparator();
    ui->toolBar->addAction(resetServerAct);


    fileToolBar = new QToolBar(tr("FileToolBar"));
    addToolBar(Qt::TopToolBarArea, fileToolBar);
    fileToolBar->setObjectName("FileToolBar");

    fileToolBar->addWidget(configBox);
    fileToolBar->addAction(browseSaveFolderAct);
    fileToolBar->addAction(showNewFilesAct);
    fileToolBar->addSeparator();

    fileToolBar->addAction(closeServerAct);



    ui->menu_File->addAction(startMinimizedAction);
    ui->menu_File->addSeparator();

    ui->menu_File->addAction(closeServerAct);
    ui->menu_File->addSeparator();
    ui->menu_File->addAction(quitAction);

    ui->menu_Help->addAction(aboutAction);

    loadSerialConfignames();
    configBox->adjustSize();

}

//**************************************************************************************************
//
//**************************************************************************************************

void FTPserverWindow::createTrayIcon()
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

    trayIcon->setIcon(QIcon(":/images/ftp.svg"));
    trayIcon->show();
}

//**************************************************************************************************
//
//**************************************************************************************************

void FTPserverWindow::iconActivated(QSystemTrayIcon::ActivationReason reason)
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

void FTPserverWindow::quitApp()
{
    saveSettings();

    closable = true;

    close();
}

//**************************************************************************************************
//
//**************************************************************************************************

void FTPserverWindow::about()
{
    QMessageBox::about(this, tr("About EdytorNC - FTP file server"),
                       tr("The <b>EdytorNC</b> is text editor for CNC programmers.") +
//                       tr("<P>Version: same as EdytorNC") +
                       tr("<P>Version: %1.%2.%3").arg(VERSION_MAJOR).arg(VERSION_MINOR).arg(VERSION_BUILD) +
                       tr("<P>Copyright (C) 1998 - 2022 by <a href=\"mailto:artkoz78@gmail.com\">Artur Kozioł</a>") +
                       tr("<P>QFtpServer Copyright (c) 2020 by <a href=\"https://github.com/sashoalm/QFtpServer\">Alexander Almaleh</a>") +
                       tr("<P>") +
                       //tr("<a href='https://www.flaticon.com/' title='Some icons'>Some icons created by inkubators - flaticon.com</a>") +
                       tr("<P>") +
                       tr("<P>EdytorNC contains pieces of code from other Open Source projects.") +
                       tr("<P>") +
                       tr("<P><i>EdytorNC - FTP file server is free software; you can redistribute it and/or modify"
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


void FTPserverWindow::configPushButton_clicked()
{
    FtpConfigDialog *dlg = new FtpConfigDialog;
    dlg->setAttribute( Qt::WA_DeleteOnClose, true );
    dlg->setModal(true);

    if(dlg->exec() == QDialog::Accepted)
    {
        loadSerialConfignames();
        configBox->adjustSize();
    };

}

//**************************************************************************************************
//
//**************************************************************************************************

void FTPserverWindow::closeEvent(QCloseEvent *event)
{
    if(closable)
    {
        event->accept();
    }
    else
    {
        if(trayIcon->isVisible())
        {
            QMessageBox::information(this, tr("EdytorNC - FTP file server"),
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


void FTPserverWindow::clearToolButton_clicked()
{
   ui->textEdit->clear();
}

//**************************************************************************************************
//
//**************************************************************************************************

void FTPserverWindow::messReceived(const QString &text)
{
    if(text == "Show")
        showNormal();

    emit needToShow();
}

//**************************************************************************************************
//
//**************************************************************************************************

void FTPserverWindow::showNewFiles()
{
    QStringList list = getPathFromConfig();

    if(list.size() > 1)
    {
        if((!list[0].isEmpty() && !list[1].isEmpty()) && (list[0] != list[1]))
        {
            QSettings settings("EdytorNC", "EdytorNC");
            QStringList extensions = settings.value("Extensions", "").toStringList();
            extensions.removeDuplicates();
            extensions.sort();

            FileChecker *fileCheck = new FileChecker(this);
            fileCheck->setData(QString(list[1]), QStringList(list[0]), extensions);
            fileCheck->findFiles();
            fileCheck->exec();
        }
        else
        {
            ui->statusBar->showMessage(tr("Paths not configured"), 3000);
        };
    };

}

//**************************************************************************************************
//
//**************************************************************************************************

void FTPserverWindow::browseSaveFolder()
{
    QStringList list = getPathFromConfig();

    if(!list[1].isEmpty())
    {
//        qDebug() << "Deb: " << list;
        QDesktopServices::openUrl(QUrl(QString("file:///%1").arg(list[1]), QUrl::TolerantMode));
    }
}

//**************************************************************************************************
//
//**************************************************************************************************

void FTPserverWindow::loadSerialConfignames()
{
    int id;
    QStringList list;
    QString item;

    QSettings settings("EdytorNC", "EdytorNC");
    settings.beginGroup("FTPConfigs");

    configBox->clear();
    list = settings.value("SettingsList", QStringList(tr("Default"))).toStringList();
    list.sort();
    configBox->addItems(list);
    item = settings.value("CurrentFtpServerSettings", tr("Default")).toString();
    id = configBox->findText(item);
    if(id >= 0)
        configBox->setCurrentIndex(id);

    settings.endGroup();
}

//**************************************************************************************************
//
//**************************************************************************************************

QStringList FTPserverWindow::getPathFromConfig()
{
    QStringList list;
    QString rootPath, dir1;

    QSettings settings("EdytorNC", "EdytorNC");
    settings.beginGroup("FTPConfigs");

    rootPath = settings.value("RootPath", QDir::rootPath()).toString();

    settings.beginGroup(configBox->currentText());

    dir1 = QDir(rootPath).absoluteFilePath(settings.value("RelativePath", "").toString());

    if(QDir(dir1).exists())
        list.append(dir1);
    else
       list.append("");

    dir1 = QDir(dir1).absoluteFilePath(settings.value("SaveDir", "").toString());

    if(QDir(dir1).exists())
        list.append(dir1);
    else
       list.append("");

    settings.endGroup();
    settings.endGroup();

//    qDebug() << "Paths: " << list;


    return list;
}





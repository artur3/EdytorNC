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


#include "mainwindow.h"
#include "ui_mainwindow.h"



MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), ui(new Ui::MainWindow)
{
    setAttribute(Qt::WA_DeleteOnClose);
    ui->setupUi(this);

    setWindowModality(Qt::NonModal);

    setWindowIcon(QIcon(":/images/programmanager.svg"));


    createActions();
    createTrayIcon();

    connect(ui->fileTableWidget, SIGNAL(cellDoubleClicked(int,int)), SLOT(tableCellDoubleClicked(int, int)));

    findFiles();

    showMaximized();
}

//**************************************************************************************************
//
//**************************************************************************************************

MainWindow::~MainWindow()
{
    delete ui;
}

//**************************************************************************************************
//
//**************************************************************************************************

void MainWindow::createTrayIcon()
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

    trayIcon->setIcon(QIcon(":/images/programmanager.svg"));
    trayIcon->show();
}

//**************************************************************************************************
//
//**************************************************************************************************

void MainWindow::iconActivated(QSystemTrayIcon::ActivationReason reason)
{
    switch(reason)
    {
        case QSystemTrayIcon::Trigger:
            if(isVisible())
                hide();
            else
                showMaximized();
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

void MainWindow::messReceived(const QString &text)
{
    if(text == "Show")
        showMaximized();

    emit needToShow();
}

//**************************************************************************************************
//
//**************************************************************************************************

void MainWindow::about()
{
    QMessageBox::about(this, tr("About EdytorNC - CNC Program Manager"),
                       tr("The <b>EdytorNC</b> is text editor for CNC programmers.") +
//                       tr("<P>Version: same as EdytorNC") +
                       tr("<P>Version: %1.%2.%3").arg(VERSION_MAJOR).arg(VERSION_MINOR).arg(VERSION_BUILD) +
                       tr("<P>Copyright (C) 1998 - 2022 by <a href=\"mailto:artkoz78@gmail.com\">Artur Kozioł</a>") +
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

void MainWindow::createActions()
{
    closeServerAct = new QAction(QIcon(":/images/window-close.svg"), tr("&Minimize to tray"), this);
    closeServerAct->setToolTip(tr("Minimize to system tray"));
    connect(closeServerAct, SIGNAL(triggered()), this, SLOT(hide()));

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

    reloadAct = new QAction(QIcon(":/images/reset.svg"), tr("&Refresh"), this);
    reloadAct->setToolTip(tr("Refresh program list"));
    connect(reloadAct, SIGNAL(triggered()), this, SLOT(findFiles()));

    configPortAct = new QAction(QIcon(":/images/serialconfig.png"), tr("Serial port configuration"), this);
    configPortAct->setToolTip(tr("Serial port configuration"));
    connect(configPortAct, SIGNAL(triggered()), this, SLOT(configPushButton_clicked()));


    browseSaveFolderAct = new QAction(QIcon(":/images/browse.png"), tr("&Browse save folder"), this);
    browseSaveFolderAct->setToolTip(tr("Browse save folder"));
    connect(browseSaveFolderAct, SIGNAL(triggered()), this, SLOT(browseSaveFolder()));

    showNewFilesAct = new QAction(QIcon(":/images/download.svg"), tr("&Show saved files"), this);
    showNewFilesAct->setToolTip(tr("Show saved files"));
    connect(showNewFilesAct, SIGNAL(triggered()), this, SLOT(showNewFiles()));



//    connect(ui->clearToolButton, SIGNAL(clicked(bool)), this, SLOT(clearToolButton_clicked()));

    startMinimizedAction = new QAction(tr("Start minimized"), this);
    startMinimizedAction->setCheckable(true);
    startMinimizedAction->setChecked(false);


    configBox = new QComboBox();
    configBox->setSizeAdjustPolicy(QComboBox::AdjustToContents);
    configBox->setDuplicatesEnabled(false);



    //ui->toolBar->setStyleSheet("QToolBar{spacing:25px;}");


    ui->mainToolBar->addAction(configPortAct);
    ui->mainToolBar->addSeparator();
    ui->mainToolBar->addAction(reloadAct);


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
    ui->menu_Help->addAction(aboutQtAct);

    loadSerialConfignames();


    QFont font = configBox->font();
    font.setPointSize(font.pointSize() + 2);
    configBox->setFont(font);

    configBox->adjustSize();

    connect(configBox, SIGNAL(currentIndexChanged(int)), SLOT(findFiles()));

}

//**************************************************************************************************
//
//**************************************************************************************************

void MainWindow::quitApp()
{
//    saveSettings();

//    closable = true;

    close();
}

//**************************************************************************************************
//
//**************************************************************************************************

void MainWindow::findFiles()
{
    getPathFromConfig();


    if(!mainPath.isEmpty())
    {
        findFiles(mainPath, extensions);
    }
    else
    {
        ui->statusBar->showMessage(tr("Paths not configured"), 3000);
    };

}

//**************************************************************************************************
//
//**************************************************************************************************

void MainWindow::findFiles(const QString startDir, QStringList fileFilter)
{
    int pos;
    QRegExp exp;
    qint64 size;
    QString line;
    QStringList files, comments;
    QStringList labels;

    reloadAct->setEnabled(false);
    configBox->setEnabled(false);
    QApplication::setOverrideCursor(Qt::BusyCursor);


    ui->fileTableWidget->clear();
    ui->fileTableWidget->model()->removeRows(0, ui->fileTableWidget->rowCount());

    labels << tr("File Name") << tr("Info 1") << tr("Info 2") << tr("Info 3") << tr("Status") << tr("Size") << tr("Modification date");
    ui->fileTableWidget->setColumnCount(labels.size());
    ui->fileTableWidget->setHorizontalHeaderLabels(labels);


    exp.setCaseSensitivity(Qt::CaseInsensitive);
    exp.setPattern("\\([^\\n\\r]*\\)|;[^\\n\\r]*");

    pos = 0;

    qDebug() << "****" << fileFilter;

    QDir directory = QDir(startDir);

    files = directory.entryList(fileFilter, QDir::Files | QDir::NoSymLinks | QDir::Readable);


//    QProgressDialog progress("Scanning files...", "Abort", 0, files.size(), this);
//    progress.setWindowModality(Qt::WindowModal);

    ui->fileTableWidget->setSortingEnabled(false);

    ui->fileTableWidget->setUpdatesEnabled(false);


    for(int i = 0; i < files.size(); ++i)
    {
//        progress.setValue(i);
//        qApp->processEvents();

//        if(progress.wasCanceled())
//            break;

        QFile file(directory.absoluteFilePath(files[i]));

        if(file.open(QIODevice::ReadOnly | QIODevice::Text))
        {
            QTextStream in(&file);
            size = file.size();
            comments.clear();

            for(int j = 0; j <= 10; ++j)
            {
                line = in.readLine();

                pos = 0;
                if((pos = line.indexOf(exp, pos)) >= 0)
                {
                    QString t = line.mid(pos, exp.matchedLength());
                    pos += exp.matchedLength();
                    if(!t.contains(";$"))
                    {
                        t.remove('(');
                        t.remove(')');
                        t.remove(';');
                    };

//                    if(t.length() > 127)
//                        t.remove(127, t.length());

                    comments.append(t);
                };

                if(comments.size() == 3)
                    break;

            };

            while(comments.size() < 3)  // ensure taht always are trhee items on the list
                comments.append("");



            QTableWidgetItem *fileNameItem = new QTableWidgetItem(files[i]);
            fileNameItem->setTextAlignment(Qt::AlignLeft | Qt::AlignVCenter);
            fileNameItem->setFlags(fileNameItem->flags() &  ~Qt::ItemIsEditable);

            QTableWidgetItem *infoNameItem1 = new QTableWidgetItem(comments[0]);
            infoNameItem1->setTextAlignment(Qt::AlignLeft | Qt::AlignVCenter);

            QTableWidgetItem *infoNameItem2 = new QTableWidgetItem(comments[1]);
            infoNameItem2->setTextAlignment(Qt::AlignLeft | Qt::AlignVCenter);

            QTableWidgetItem *infoNameItem3 = new QTableWidgetItem(comments[2]);
            infoNameItem3->setTextAlignment(Qt::AlignLeft | Qt::AlignVCenter);

            QTableWidgetItem *sizeItem = new QTableWidgetItem(tr("%1 KB").arg(int((size + 1023) / 1024)));
            sizeItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
            sizeItem->setFlags(sizeItem->flags() &  ~Qt::ItemIsEditable);

            QTableWidgetItem *dateItem = new QTableWidgetItem();
            dateItem->setTextAlignment(Qt::AlignLeft | Qt::AlignVCenter);
            dateItem->setData(Qt::DisplayRole, QFileInfo(file).lastModified());
            dateItem->setFlags(dateItem->flags() &  ~Qt::ItemIsEditable);


            int row = ui->fileTableWidget->rowCount();
            ui->fileTableWidget->insertRow(row);
            ui->fileTableWidget->setItem(row, 0, fileNameItem);
            ui->fileTableWidget->setItem(row, 1, infoNameItem1);
            ui->fileTableWidget->setItem(row, 2, infoNameItem2);
            ui->fileTableWidget->setItem(row, 3, infoNameItem3);
            ui->fileTableWidget->setItem(row, 5, sizeItem);
            ui->fileTableWidget->setItem(row, 6, dateItem);

            file.close();

        };
    };

//    progress.setValue(files.size());

    ui->fileTableWidget->setSortingEnabled(true);

    ui->fileTableWidget->resizeRowsToContents();
    ui->fileTableWidget->resizeColumnsToContents();
    ui->fileTableWidget->setUpdatesEnabled(true);

    QApplication::restoreOverrideCursor();
    reloadAct->setEnabled(true);
    configBox->setEnabled(true);
}

//**************************************************************************************************
//
//**************************************************************************************************

void MainWindow::showNewFiles()
{
//    getPathFromConfig();


    if((!mainPath.isEmpty() && !downloadPath.isEmpty()) && (mainPath != downloadPath))
    {
        FileChecker *fileCheck = new FileChecker(this);
        fileCheck->setData(downloadPath, QStringList(mainPath), extensions);
        fileCheck->findFiles();
        fileCheck->exec();
    }
    else
    {
        ui->statusBar->showMessage(tr("Paths not configured"), 3000);
    };


}

//**************************************************************************************************
//
//**************************************************************************************************

void MainWindow::browseSaveFolder()
{
//    getPathFromConfig();

    if(!downloadPath.isEmpty())
    {
//        qDebug() << "Deb: " << list;
        QDesktopServices::openUrl(QUrl(QString("file:///%1").arg(downloadPath), QUrl::TolerantMode));
    }
}

//**************************************************************************************************
//
//**************************************************************************************************

void MainWindow::loadSerialConfignames()
{
    int id;
    QStringList list;
    QString item;

    QSettings settings("EdytorNC", "EdytorNC");

    extensions = settings.value("Extensions", "*.nc").toStringList();
    extensions.removeDuplicates();
    extensions.sort();

    settings.beginGroup("FTPConfigs");

    configBox->clear();
    list = settings.value("SettingsList", QStringList(tr("Default"))).toStringList();
    list.sort();
    configBox->addItems(list);




    settings.endGroup();

    settings.beginGroup("ProgramManager");

    item = settings.value("CurrentSettings", tr("Default")).toString();
    id = configBox->findText(item);
    if(id >= 0)
        configBox->setCurrentIndex(id);

    settings.endGroup();
}

//**************************************************************************************************
//
//**************************************************************************************************

void MainWindow::getPathFromConfig()
{
    QString rootPath, dir1;

    QSettings settings("EdytorNC", "EdytorNC");
    settings.beginGroup("FTPConfigs");

    rootPath = settings.value("RootPath", QDir::rootPath()).toString();

    settings.beginGroup(configBox->currentText());

    dir1 = QDir(rootPath).absoluteFilePath(settings.value("RelativePath", "").toString());

    if(QDir(dir1).exists())
        mainPath = dir1;
    else
       mainPath = "";

    dir1 = QDir(dir1).absoluteFilePath(settings.value("SaveDir", "").toString());

    if(QDir(dir1).exists())
        downloadPath = dir1;
    else
       downloadPath = "";

    settings.endGroup();
    settings.endGroup();

//    qDebug() << "Paths: " << list;


}

//**************************************************************************************************
//
//**************************************************************************************************

void MainWindow::tableCellDoubleClicked(int row, int col)
{
    QString files, info1;
    QTableWidgetItem *item;
    int startRow, endRow;


    qDebug() << "Double clicked: " << row << col;

    if(col == 0)
    {

        QString path1 = mainPath;
        if(!path1.endsWith("/"))
            path1 = path1 + "/";


        item = ui->fileTableWidget->item(row, 1);
        if(item)
        {
            info1 = item->text();
        };

        startRow = row - 1;
        while(true)
        {
            if(startRow < 0)
                break;

            item = ui->fileTableWidget->item(startRow, 1);
            if(item)
            {
                if((info1 == item->text()))
                {
                    startRow--;
                }
                else
                    break;
            }
            else
                break;
        };
        startRow++;

        endRow = row + 1;
        while(true)
        {
            if(endRow > ui->fileTableWidget->rowCount())
                break;

            item = ui->fileTableWidget->item(endRow, 1);
            if(item)
            {
                if((info1 == item->text()))
                {
                    endRow++;
                }
                else
                    break;
            }
            else
                break;
        };
        endRow--;

        qDebug() << "Start, end: " << startRow << endRow;

        for(int i = startRow; i <= endRow; ++i)
        {
            item = ui->fileTableWidget->item(i, 0);
            if(item)
            {
                if(QFile::exists(path1 + item->text()))
                {
                    files.append(path1 + item->text() + " ");
                };

            };

            qDebug() << "Start, end, i: " << startRow << endRow << i;
        }

        qDebug() << "Files: " << files;

        if(!files.isEmpty())
            openInEditor(files);
    };
}

//**************************************************************************************************
//
//**************************************************************************************************

void MainWindow::openInEditor(QString files)
{

    QString fileName, path;

    path = QApplication::applicationDirPath() + "/";
    path = QDir::toNativeSeparators(path);

#ifdef Q_OS_LINUX
    fileName = path + "EdytorNC";
#endif

#ifdef Q_OS_WIN32
    fileName = "EdytorNC.exe";
#endif

    edytorProc = findChild<QProcess *>("EdytorNCApp");

    if(!edytorProc)
    {
        edytorProc = new QProcess(this);
        edytorProc->setObjectName("EdytorNCApp");
        edytorProc->setWorkingDirectory(path);
        edytorProc->startDetached(fileName + " " + files);

    }
    else
        if(edytorProc->pid() == 0)
        {
            edytorProc->setWorkingDirectory(path);
            edytorProc->startDetached(fileName  + " " + files);
        };

}

/***************************************************************************
 *   Copyright (C) 2006-2024 by Artur Kozioł                               *
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
#include "pmconfigdialog.h"
#include "copyfiledialog.h"


#include <QSqlError>
#include <QSqlQuery>


#define   ALL_FILES_TEXT    "** ALL **"


MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), ui(new Ui::MainWindow)
{
    setAttribute(Qt::WA_DeleteOnClose);
    ui->setupUi(this);

    setWindowModality(Qt::NonModal);

    setWindowIcon(QIcon(":/images/programmanager.svg"));

    createActions();
    createTrayIcon();

    if(!createConnection())
    {
        statusBar()->showMessage(tr("Failed to connect to the database"), 0);
        closable = true;
    }
    else
    {
        initDatabase();
        closable = false;
    };

    if((refreshTime1.toString("HH:mm") == "00:00") && (refreshTime2.toString("HH:mm") == "00:00"))
        closable = true;


    listViewModel = new QStringListModel(this);

    fileWatcher = new QFileSystemWatcher(this);

    tableViewModel = new QSqlTableModel(this);

    ui->tableView->setModel(tableViewModel);

    tableViewModel->setEditStrategy(QSqlTableModel::OnManualSubmit);

    ui->tableView->horizontalHeader()->setMinimumSectionSize(150);


    connect(ui->tableView, SIGNAL(doubleClicked(QModelIndex)), SLOT(tableCellDoubleClicked(QModelIndex)));
    connect(ui->tableView, SIGNAL(clicked(QModelIndex)), SLOT(tableViewItemClicked(QModelIndex)));

    connect(ui->filterLineEdit, SIGNAL(textChanged(QString)), SLOT(applyFilter()));

    connect(ui->listView, SIGNAL(clicked(QModelIndex)), SLOT(listViewItemClicked(QModelIndex)));


    loadConfignames();
    changeTable();

    showMaximized();

//    ui->mainToolBar->installEventFilter(this);
    ui->listView->installEventFilter(this);
    ui->tableView->installEventFilter(this);

    refreshTimer = new QTimer(this);
    refreshTimer->setInterval(1000);
    connect(refreshTimer, SIGNAL(timeout()), SLOT(refreshTimerTimout()));
    refreshTimer->start(1000);


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

    trayIconMenu->addAction(scanAllFilesAct);
    trayIconMenu->addSeparator();
    trayIconMenu->addAction(minimizeAction);
    trayIconMenu->addAction(maximizeAction);
    trayIconMenu->addAction(restoreAction);
    trayIconMenu->addSeparator();
    trayIconMenu->addAction(aboutAction);
    trayIconMenu->addSeparator();
    trayIconMenu->addAction(quitAction);

    trayIcon = new QSystemTrayIcon(this);
    trayIcon->setContextMenu(trayIconMenu);
    trayIcon->setToolTip(tr("EdytorNC - Program Manager"));

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
                       tr("<P>Copyright (C) 1998 - 2024 by <a href=\"mailto:artkoz78@gmail.com\">Artur Kozioł</a>") +
                       tr("<P>") +
                       //tr("<a href='https://www.flaticon.com/' title='Some icons'>Some icons created by inkubators - flaticon.com</a>") +
                       tr("<P>") +
                       tr("<P>EdytorNC contains pieces of code from other Open Source projects.") +
                       tr("<P>") +
                       tr("<P><i>EdytorNC - CNC Program Manager is free software; you can redistribute it and/or modify"
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
    closeServerAct->setStatusTip(tr("Minimize to system tray"));
    connect(closeServerAct, SIGNAL(triggered()), this, SLOT(hide()));

    aboutAction = new QAction(tr("&About"), this);
    aboutAction->setStatusTip(tr("Show the application's About box"));
    connect(aboutAction, SIGNAL(triggered()), this, SLOT(about()));

    aboutQtAct = new QAction(tr("About &Qt"), this);
    aboutQtAct->setStatusTip(tr("Show the Qt library's About box"));
    connect(aboutQtAct, SIGNAL(triggered()), qApp, SLOT(aboutQt()));

    minimizeAction = new QAction(tr("Mi&nimize"), this);
    connect(minimizeAction, SIGNAL(triggered()), this, SLOT(hide()));

    maximizeAction = new QAction(tr("Ma&ximize"), this);
    connect(maximizeAction, SIGNAL(triggered()), this, SLOT(showMaximized()));

    restoreAction = new QAction(tr("&Restore"), this);
    connect(restoreAction, SIGNAL(triggered()), this, SLOT(showNormal()));

    quitAction = new QAction(QIcon(":/images/exit.svg"), tr("&Quit"), this);
    connect(quitAction, SIGNAL(triggered()), this, SLOT(quitApp()));

    reloadAct = new QAction(QIcon(":/images/refresh.png"), tr("&Refresh"), this);
    reloadAct->setStatusTip(tr("Refresh program list for current machine"));
    connect(reloadAct, SIGNAL(triggered()), this, SLOT(findFiles()));

    scanAllFilesAct = new QAction(QIcon(":/images/refresh.png"), tr("&Rescan all"), this);
    scanAllFilesAct->setStatusTip(tr("Rescan program list for all machines"));
    connect(scanAllFilesAct, SIGNAL(triggered()), this, SLOT(rescanFiles()));

    configPortAct = new QAction(QIcon(":/images/serialconfig.png"), tr("Program Manager configuration"), this);
    configPortAct->setStatusTip(tr("Program Manager configuration"));
    connect(configPortAct, SIGNAL(triggered()), this, SLOT(configPushButton_clicked()));


    browseSaveFolderAct = new QAction(QIcon(":/images/browse.png"), tr("&Browse save folder"), this);
    browseSaveFolderAct->setStatusTip(tr("Browse save folder"));
    connect(browseSaveFolderAct, SIGNAL(triggered()), this, SLOT(browseSaveFolder()));

    showNewFilesAct = new QAction(QIcon(":/images/download.svg"), tr("Show saved files"), this);
    showNewFilesAct->setStatusTip(tr("Show files received from the machine"));
    connect(showNewFilesAct, SIGNAL(triggered()), this, SLOT(showNewFiles()));
    connect(showNewFilesAct, SIGNAL(hovered()), this, SLOT(showNewFilesActHovered()));

    copyFilesAct = new QAction(QIcon(":/images/edit-copy.svg"), tr("&Copy files"), this);
    copyFilesAct->setStatusTip(tr("Copy files"));
    connect(copyFilesAct, SIGNAL(triggered()), this, SLOT(copyFiles()));


//    connect(ui->clearToolButton, SIGNAL(clicked(bool)), this, SLOT(clearToolButton_clicked()));

    startMinimizedAction = new QAction(tr("Start minimized"), this);
    startMinimizedAction->setCheckable(true);
    startMinimizedAction->setChecked(false);


    //ui->toolBar->setStyleSheet("QToolBar{spacing:25px;}");

    ui->mainToolBar->setStyleSheet("QToolBar{spacing:20px;}");

    ui->mainToolBar->addAction(configPortAct);
    ui->mainToolBar->addSeparator();
//    ui->mainToolBar->addSeparator();




//    fileToolBar = new QToolBar(tr("FileToolBar"));
//    addToolBar(Qt::TopToolBarArea, fileToolBar);
//    fileToolBar->setObjectName("FileToolBar");
//    fileToolBar->setStyleSheet("QToolBar{spacing:10px;}");

    ui->mainToolBar->addAction(reloadAct);
    ui->mainToolBar->addSeparator();
    ui->mainToolBar->addAction(browseSaveFolderAct);
    ui->mainToolBar->addSeparator();
    ui->mainToolBar->addAction(showNewFilesAct);
    ui->mainToolBar->addSeparator();
    ui->mainToolBar->addAction(copyFilesAct);
    ui->mainToolBar->addSeparator();


//    windowToolBar = new QToolBar(tr("WindowToolBar"));
//    windowToolBar->setStyleSheet("QToolBar{spacing:10px;}");
//    windowToolBar->setObjectName("WindowToolBar");
//    addToolBar(Qt::TopToolBarArea, windowToolBar);

    ui->mainToolBar->addAction(closeServerAct);




    ui->menu_File->addAction(scanAllFilesAct);
    ui->menu_File->addSeparator();

    ui->menu_File->addAction(startMinimizedAction);
    ui->menu_File->addSeparator();

    ui->menu_File->addAction(closeServerAct);
    ui->menu_File->addSeparator();
    ui->menu_File->addAction(quitAction);

    ui->menu_Help->addAction(aboutAction);
    ui->menu_Help->addAction(aboutQtAct);




//    QFont font = configBox->font();
//    font.setPointSize(font.pointSize() + 2);
//    configBox->setFont(font);


//    configBox->adjustSize();



//    connect(configBox, SIGNAL(currentIndexChanged(int)), SLOT(changeTable()));


}

//**************************************************************************************************
//
//**************************************************************************************************

void MainWindow::quitApp()
{
//    saveSettings();

    closable = true;

    db.close();

    close();
}

//**************************************************************************************************
//
//**************************************************************************************************

void MainWindow::on_lineFind_textEdited(const QString &arg1)
{
//    proxy_model->setFilterCaseSensitivity(Qt::CaseInsensitive);
//    proxy_model->setFilterRegExp(arg1);
//    proxy_model->setFilterKeyColumn(-1);




//    ui->tableView->resizeColumnsToContents();


//    proxy_model->setSourceModel(model);
//    ui->tableView->setModel(proxy_model);
//    QSortFilterProxyModel proxy;
//    proxy.setSourceModel(ui->tableView->model());


//    QModelIndex index=proxy.mapToSource(proxy.index(0,0));
//    if(index.isValid())
//    {
//        ui->tableView->selectionModel()->select(index,QItemSelectionModel::Select | QItemSelectionModel::Rows);
//        ui->tableView->scrollTo(index,QAbstractItemView::EnsureVisible);
//    }


}

//**************************************************************************************************
//
//**************************************************************************************************

void MainWindow::applyFilter()
{ 
    if(currentMachineName == tr(ALL_FILES_TEXT))
    {
        if(ui->filterLineEdit->text().isEmpty())
            tableViewModel->setFilter("");
        else
            tableViewModel->setFilter(QString("infoName1 LIKE '%%2%' OR infoName2 LIKE '%%2%' OR infoName3 LIKE '%%2%'").arg(ui->filterLineEdit->text()));
    }
    else
    {
        if(ui->filterLineEdit->text().isEmpty())
            tableViewModel->setFilter(QString("machine='%1'").arg(currentMachineName));
        else
            tableViewModel->setFilter(QString("machine='%1' AND (infoName1 LIKE '%%2%' OR infoName2 LIKE '%%2%' OR infoName3 LIKE '%%2%')").arg(currentMachineName, ui->filterLineEdit->text()));
    };
}

//**************************************************************************************************
//
//**************************************************************************************************

void MainWindow::changeTable()
{
    getDataFromConfig();

    //    qDebug() << "**" << currentTableName << mainPath;

    tableViewModel->setTable("programs");

    applyFilter();

    tableViewModel->select();

    tableViewModel->setHeaderData(0, Qt::Horizontal, QObject::tr("ID"));
    tableViewModel->setHeaderData(1, Qt::Horizontal, QObject::tr("Machine"));
    tableViewModel->setHeaderData(2, Qt::Horizontal, QObject::tr("File Name"));
    tableViewModel->setHeaderData(3, Qt::Horizontal, QObject::tr("Info 1") );
    tableViewModel->setHeaderData(4, Qt::Horizontal, QObject::tr("Info 2"));
    tableViewModel->setHeaderData(5, Qt::Horizontal, QObject::tr("Info 3"));
    tableViewModel->setHeaderData(6, Qt::Horizontal, QObject::tr("Size") );
    tableViewModel->setHeaderData(7, Qt::Horizontal, QObject::tr("Modification date"));


    ui->tableView->hideColumn(0);

    if(!(currentMachineName == tr(ALL_FILES_TEXT)))
        ui->tableView->hideColumn(1);

    ui->tableView->setVisible(false);
    ui->tableView->resizeColumnsToContents();
    ui->tableView->setVisible(true);


}

//**************************************************************************************************
//
//**************************************************************************************************

void MainWindow::findFiles()
{
    //getDataFromConfig();

    if(!currentMachineName.isEmpty() && (currentMachineName != tr(ALL_FILES_TEXT)))
    {

        //findFiles(mainPath, extensions);

        //updateFileTable(currentMachineName, mainPath, extensions);

        rescanFiles(currentMachineName, false);

        changeTable();

    };

}

//**************************************************************************************************
//
//**************************************************************************************************

void MainWindow::updateFileTable(const QString machine, const QString startDir, QStringList fileFilter)
{
//    int pos;
//    QRegExp exp;
//    qint64 size;
//    QString line;
    QStringList fileList, comments, prevComments;
    QStringList labels;
    //    QString machineName;
    //    bool alternate = false;

    if(!tryOpenDb())
    {
        qDebug() << "updateFileTable:" << "DB is not open";

        return;
    };


    reloadAct->setEnabled(false);
    ui->listView->setEnabled(false);
    QApplication::setOverrideCursor(Qt::BusyCursor);



//    exp.setCaseSensitivity(Qt::CaseInsensitive);
//    exp.setPattern("\\([^\\n\\r]*\\)|;[^\\n\\r]*");

//    pos = 0;

//    QDir directory = QDir(startDir);

    //    fileFilter.replaceInStrings(";", " ");

//    fileList = directory.entryList(fileFilter, QDir::Files | QDir::NoSymLinks | QDir::Readable);

//    qDebug() << "updateFileTable: " << "PATH: " << directory.path() << directory.nameFilters() <<  fileList.size();


    //    QProgressDialog progress("Scanning files...", "Abort", 0, files.size(), this);
    //    progress.setWindowModality(Qt::WindowModal);

//    qDebug() << "updateFileTable:" << machine << startDir << fileFilter <<  fileList.size();

    //    QSqlQuery query;



    QDirIterator it(startDir, fileFilter, QDir::Files | QDir::NoSymLinks | QDir::Readable);

    while(it.hasNext())
    {
        updateFileInDb(machine, it.next());

//        qDebug() << it.next();
    };



//    for(int i = 0; i < fileList.size(); ++i)
//    {
//        qDebug() << "updateFileTable:" << fileList[i].toLatin1();

//        updateFileInDb(machine, directory.absoluteFilePath(fileList[i]));

//        QFile file(directory.absoluteFilePath(fileList[i]));

//        if(file.open(QIODevice::ReadOnly | QIODevice::Text))
//        {
//            QTextStream in(&file);
//            size = file.size();
//            comments.clear();

//            for(int j = 0; j <= 10; ++j)
//            {
//                line = in.readLine();

//                pos = 0;
//                if((pos = line.indexOf(exp, pos)) >= 0)
//                {
//                    QString t = line.mid(pos, exp.matchedLength());
//                    pos += exp.matchedLength();

//                    if(!t.contains(";$"))
//                    {
//                        t.remove('(');
//                        t.remove(')');
//                        t.remove(';');
//                        comments.append(t);
//                    };
//                };

//                if(comments.size() == 3)
//                    break;

//            };

//            file.close();

//            while(comments.size() < 3)  // ensure taht always are trhee items on the list
//                comments.append("");


//            bool alreadyExist = false;

//            query.prepare("SELECT * FROM programs WHERE machine=:machine AND fileName=:fileName");
//            query.bindValue(":machine", machine);
//            query.bindValue(":fileName", fileList[i]);

//            if(!query.exec())
//            {
//                // select failed
//            }

//            alreadyExist = query.next();

//            if(alreadyExist)
//            {
//                query.prepare(QString("UPDATE programs SET infoName1=:infoName1, infoName2=:infoName2, infoName3=:infoName3, size=:size, date=:date WHERE machine=:machine AND fileName=:fileName"));
//                query.bindValue(":machine", machine);
//                query.bindValue(":fileName", fileList[i]);
//                query.bindValue(":infoName1", comments[0]);
//                query.bindValue(":infoName2", comments[1]);
//                query.bindValue(":infoName3", comments[2]);
//                query.bindValue(":date", QFileInfo(file).lastModified().toString("yyyy-MM-dd HH:mm:ss"));
//                query.bindValue(":size", tr("%1 KB").arg(int((size + 1023) / 1024)));
//                query.exec();


//                //            qDebug() << "UPDATE: " << query.lastError().text();
//                //            qDebug() << query.lastQuery();

//                qDebug() << "updateFileTable:" << "UPDATE";

//            }
//            else
//            {
//                query.prepare(QString("INSERT INTO programs (machine, fileName, infoName1, infoName2, infoName3, size, date) VALUES (:machine, :fileName, :infoName1, :infoName2, :infoName3, :size, :date)"));
//                query.bindValue(":machine", machine);
//                query.bindValue(":fileName", fileList[i]);
//                query.bindValue(":infoName1", comments[0]);
//                query.bindValue(":infoName2", comments[1]);
//                query.bindValue(":infoName3", comments[2]);
//                query.bindValue(":date", QFileInfo(file).lastModified().toString("yyyy-MM-dd HH:mm:ss"));
//                query.bindValue(":size", tr("%1 KB").arg(int((size + 1023) / 1024)));
//                query.exec();


//                //                qDebug() << "INSERT: " << query.lastError().text();
//                //                qDebug() << query.lastQuery();

//                qDebug() << "updateFileTable:" << "INSERT";
//            };


//        };

//    };

    QApplication::restoreOverrideCursor();
    reloadAct->setEnabled(true);
    ui->listView->setEnabled(true);
}


//**************************************************************************************************
//
//**************************************************************************************************

void MainWindow::findFiles(const QString startDir, QStringList fileFilter)
{
//    int pos;
//    QRegExp exp;
//    qint64 size;
//    QString line;
//    QStringList files, comments, prevComments;
//    QStringList labels;
//    bool alternate = false;


//    reloadAct->setEnabled(false);
//    configBox->setEnabled(false);
//    QApplication::setOverrideCursor(Qt::BusyCursor);


//    ui->fileTableWidget->clear();
//    ui->fileTableWidget->model()->removeRows(0, ui->fileTableWidget->rowCount());

//    labels << tr("File Name") << tr("Info 1") << tr("Info 2") << tr("Info 3") << tr("Size") << tr("Modification date");
//    ui->fileTableWidget->setColumnCount(labels.size());
//    ui->fileTableWidget->setHorizontalHeaderLabels(labels);


//    exp.setCaseSensitivity(Qt::CaseInsensitive);
//    exp.setPattern("\\([^\\n\\r]*\\)|;[^\\n\\r]*");

//    pos = 0;

//    qDebug() << "****" << fileFilter;

//    QDir directory = QDir(startDir);

//    files = directory.entryList(fileFilter, QDir::Files | QDir::NoSymLinks | QDir::Readable);


////    QProgressDialog progress("Scanning files...", "Abort", 0, files.size(), this);
////    progress.setWindowModality(Qt::WindowModal);

//    ui->fileTableWidget->setSortingEnabled(false);

//    ui->fileTableWidget->setUpdatesEnabled(false);


//    for(int i = 0; i < files.size(); ++i)
//    {
////        progress.setValue(i);
////        qApp->processEvents();

////        if(progress.wasCanceled())
////            break;

//        QFile file(directory.absoluteFilePath(files[i]));

//        if(file.open(QIODevice::ReadOnly | QIODevice::Text))
//        {
//            QTextStream in(&file);
//            size = file.size();
//            comments.clear();

//            for(int j = 0; j <= 10; ++j)
//            {
//                line = in.readLine();

//                pos = 0;
//                if((pos = line.indexOf(exp, pos)) >= 0)
//                {
//                    QString t = line.mid(pos, exp.matchedLength());
//                    pos += exp.matchedLength();
//                    if(!t.contains(";$"))
//                    {
//                        t.remove('(');
//                        t.remove(')');
//                        t.remove(';');
//                    };

////                    if(t.length() > 127)
////                        t.remove(127, t.length());

//                    comments.append(t);
//                };

//                if(comments.size() == 3)
//                    break;

//            };

//            file.close();

//            while(comments.size() < 3)  // ensure taht always are trhee items on the list
//                comments.append("");



//            QTableWidgetItem *fileNameItem = new QTableWidgetItem(files[i]);
//            fileNameItem->setTextAlignment(Qt::AlignLeft | Qt::AlignVCenter);
//            fileNameItem->setFlags(fileNameItem->flags() &  ~Qt::ItemIsEditable);

//            QTableWidgetItem *infoNameItem1 = new QTableWidgetItem(comments[0]);
//            infoNameItem1->setTextAlignment(Qt::AlignLeft | Qt::AlignVCenter);

//            QTableWidgetItem *infoNameItem2 = new QTableWidgetItem(comments[1]);
//            infoNameItem2->setTextAlignment(Qt::AlignLeft | Qt::AlignVCenter);

//            QTableWidgetItem *infoNameItem3 = new QTableWidgetItem(comments[2]);
//            infoNameItem3->setTextAlignment(Qt::AlignLeft | Qt::AlignVCenter);

//            QTableWidgetItem *sizeItem = new QTableWidgetItem(tr("%1 KB").arg(int((size + 1023) / 1024)));
//            sizeItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
//            sizeItem->setFlags(sizeItem->flags() &  ~Qt::ItemIsEditable);

//            QTableWidgetItem *dateItem = new QTableWidgetItem();
//            dateItem->setTextAlignment(Qt::AlignLeft | Qt::AlignVCenter);
//            dateItem->setData(Qt::DisplayRole, QFileInfo(file).lastModified());
//            dateItem->setFlags(dateItem->flags() &  ~Qt::ItemIsEditable);

//            int row = ui->fileTableWidget->rowCount();
//            ui->fileTableWidget->insertRow(row);
//            ui->fileTableWidget->setItem(row, 0, fileNameItem);
//            ui->fileTableWidget->setItem(row, 1, infoNameItem1);
//            ui->fileTableWidget->setItem(row, 2, infoNameItem2);
//            ui->fileTableWidget->setItem(row, 3, infoNameItem3);
//            ui->fileTableWidget->setItem(row, 4, sizeItem);
//            ui->fileTableWidget->setItem(row, 5, dateItem);


//            if(!comments.isEmpty() && !prevComments.isEmpty())
//            {
//                if(comments[0] == prevComments[0])
//                {
//                    QColor bColor;

//                    if(alternate)
//                        bColor = QColor(255, 255, 170);
//                    else
//                        bColor = QColor(247, 247, 157);

//                    for(int k = 0 ; k < ui->fileTableWidget->columnCount(); k++)
//                    {
//                        QTableWidgetItem *item = ui->fileTableWidget->item(row, k);
//                        if(item)
//                            item->setBackground(bColor);
//                    };

//                    if(row > 0)
//                        row--;

//                    for(int k = 0 ; k < ui->fileTableWidget->columnCount(); k++)
//                    {
//                        QTableWidgetItem *item = ui->fileTableWidget->item(row, k);
//                        if(item)
//                            item->setBackground(bColor);
//                    };


//                }
//                else
//                {
////                    fileNameItem->setBackgroundColor(Qt::gray);

//                    alternate = !alternate;
//                };
//            };

//            prevComments = comments;

//        };
//    };

////    progress.setValue(files.size());

//    ui->fileTableWidget->setSortingEnabled(true);

//    ui->fileTableWidget->resizeRowsToContents();
//    ui->fileTableWidget->resizeColumnsToContents();
//    ui->fileTableWidget->setUpdatesEnabled(true);

//    QApplication::restoreOverrideCursor();
//    reloadAct->setEnabled(true);
//    configBox->setEnabled(true);
}

//**************************************************************************************************
//
//**************************************************************************************************

void MainWindow::showNewFiles()
{
    getDataFromConfig();

    if((!mainPath.isEmpty() && !savePath.isEmpty()) && (mainPath != savePath))
    {
        FileChecker *fileCheck = new FileChecker(this);
        fileCheck->setData(savePath, QStringList(mainPath), extensions);
        fileCheck->findFiles();
        fileCheck->exec();
    }
    else
    {
        ui->statusBar->showMessage(tr("Paths not configured"), 5000);
    };


}

//**************************************************************************************************
//
//**************************************************************************************************

void MainWindow::browseSaveFolder()
{
//    getPathFromConfig();

    if(!savePath.isEmpty())
    {
//        qDebug() << "Deb: " << list;
        QDesktopServices::openUrl(QUrl(QString("file:///%1").arg(savePath), QUrl::TolerantMode));
    }
}

//**************************************************************************************************
//
//**************************************************************************************************

void MainWindow::listViewItemClicked (QModelIndex index)
{
   currentMachineName = index.data().toString();

//   if(currentMachineName == tr(ALL_FILES_TEXT))
//       currentMachineName = "";

   changeTable();

}

//**************************************************************************************************
//
//**************************************************************************************************

void MainWindow::loadConfignames()
{
    QStringList list;

    list = getMachineListFromDb();

//    disconnect(fileWatcher, SIGNAL(fileChanged(QString)));

    fileWatcher->removePaths(fileWatcher->directories());  //remove existing paths

//    fileWatcher.clear();
    if(!closable)
    {
        for(const auto& i : qAsConst(list))
        {
            QString path = getMachineMainPathFromDb(i);

            if(!path.isEmpty())
            {
                bool success = fileWatcher->addPath(path);

                qDebug() << "fileWatcher file add: " << success;
            };
        };

        connect(fileWatcher, SIGNAL(directoryChanged(QString)), SLOT(fileChangedSlot(QString)));
        connect(fileWatcher, SIGNAL(fileChanged(QString)), SLOT(fileChangedSlot(QString)));

        qDebug() << "fileWatcher: " << fileWatcher->directories();

    };

    list << tr(ALL_FILES_TEXT);
    list.sort();


    listViewModel->setStringList(list);
    ui->listView->setModel(listViewModel);
    ui->listView->adjustSize();
}

//**************************************************************************************************
//
//**************************************************************************************************

QString MainWindow::getMachineMainPathFromConfig(const QString machineName)
{
    QString rootPath, path;

    QSettings settings(QSettings::IniFormat, QSettings::UserScope, "EdytorNC", "EdytorNC");
    settings.beginGroup("PMConfigs");

    rootPath = settings.value("RootPath", QDir::rootPath()).toString();


    settings.beginGroup(machineName);

    path = settings.value("RelativePath", "").toString();

    if(path.isEmpty())
        return "";

    path = QDir(rootPath).absoluteFilePath(path);

    settings.endGroup();
    settings.endGroup();


    if(QDir(path).exists())
        return path;
    else
       return "";


//    qDebug() << "getMachineMainPathFromConfig: " << path;


}

//**************************************************************************************************
//
//**************************************************************************************************

QString MainWindow::getMachineSavePathFromConfig(const QString machineName)
{
    QString rootPath, path;

    QSettings settings(QSettings::IniFormat, QSettings::UserScope, "EdytorNC", "EdytorNC");
    settings.beginGroup("PMConfigs");

    rootPath = settings.value("RootPath", QDir::rootPath()).toString();


    settings.beginGroup(machineName);

    path = QDir(rootPath).absoluteFilePath(settings.value("RelativePath", "").toString());
    path = QDir(path).absoluteFilePath(settings.value("SaveDir", "").toString());

    settings.endGroup();
    settings.endGroup();

    qDebug() << "getMachineSavePathFromConfig: " << path;

    if(QDir(path).exists())
        return path;
    else
       return "";
}


//**************************************************************************************************
//
//**************************************************************************************************

QStringList MainWindow::getFileExtensionsFromConfig(const QString machineName)
{
    QStringList ext;
    QString t_ext;

    QSettings settings(QSettings::IniFormat, QSettings::UserScope, "EdytorNC", "EdytorNC");
    settings.beginGroup("PMConfigs");
    settings.beginGroup(machineName);

    t_ext = settings.value("ListExt", "*.nc").toString();

    ext.append(t_ext.split(";", QString::SkipEmptyParts));

    ext.removeDuplicates();
    ext.sort();

    settings.endGroup();
    settings.endGroup();


    qDebug() << "getFileExtensionsFromConfig: " << ext << ext.size();


    return ext;

}

//**************************************************************************************************
//
//**************************************************************************************************

QString MainWindow::getMachineMainPathFromDb(const QString machineName)
{
    QString rootPath, path;

    if(tryOpenDb())
    {
        QSqlQuery query;

        query.prepare(QString("SELECT relativepath FROM config WHERE machine=:machine"));
        query.bindValue(":machine", machineName);
        query.exec();

//        qDebug() << "getMachineMainPathFromDb: " << "SELECT: " << query.lastError().text();

        while(query.next())
        {
            path = query.value(0).toString();
        };
    };

    if(path.isEmpty())
        return "";

    QSettings settings(QSettings::IniFormat, QSettings::UserScope, "EdytorNC", "EdytorNC");
    settings.beginGroup("PMConfigs");

    rootPath = settings.value("RootPath", QDir::rootPath()).toString();

    settings.endGroup();

    path = QDir(rootPath).absoluteFilePath(path);

    qDebug() << "getMachineMainPathFromDb: " << path;

    if(QDir(path).exists())
        return path;
    else
        return "";
}

//**************************************************************************************************
//
//**************************************************************************************************

QString MainWindow::getMachineSavePathFromDb(const QString machineName)
{
    QString rootPath, r_path, s_path;

//    QSettings settings(QSettings::IniFormat, QSettings::UserScope, "EdytorNC", "EdytorNC");
//    settings.beginGroup("PMConfigs");

//    rootPath = settings.value("RootPath", QDir::rootPath()).toString();

//    settings.endGroup();


    if(tryOpenDb())
    {

        r_path = getMachineMainPathFromDb(machineName);

        QSqlQuery query;

        query.prepare("SELECT savepath FROM config WHERE machine=:machine");
        query.bindValue(":machine", machineName);
        query.exec();

//        qDebug() << "getMachineSavePathFromDb: " << "SELECT: " << query.lastError().text();

        while (query.next())
        {
            s_path = query.value(0).toString();
        };
    };

    if(s_path.isEmpty())
        return "";


    s_path = QDir(r_path).absoluteFilePath(s_path);

    qDebug() << "getMachineSavePathFromDb: " << s_path;

    if(QDir(s_path).exists())
        return s_path;
    else
        return "";
}

//**************************************************************************************************
//
//**************************************************************************************************

QStringList MainWindow::getMachineFileExtFromDb(const QString machineName)
{
    QStringList ext;
    QString t_ext;

    if(tryOpenDb())
    {
        QSqlQuery query;

        query.prepare("SELECT extensions FROM config WHERE machine=:machine");
        query.bindValue(":machine", machineName);
        query.exec();

        while (query.next())
        {
            t_ext = query.value(0).toString();
        };
    };

    if(t_ext.isEmpty())
        t_ext = "*.nc";

    ext.append(t_ext.split(";", QString::SkipEmptyParts));

    ext.removeDuplicates();
    ext.sort();

    qDebug() << "getMachineFileExtFromDb: " << ext << ext.size();


    return ext;
}

//**************************************************************************************************
//
//**************************************************************************************************

QStringList MainWindow::getMachineListFromDb()
{
    QStringList list;

    if(tryOpenDb())
    {
        QSqlQuery query;

        query.prepare("SELECT machine FROM config");
        query.exec();

        while (query.next())
        {
            list.append(query.value(0).toString());
        };
        list.removeDuplicates();
        list.sort(Qt::CaseInsensitive);

        qDebug() << "getMachineListFromDb: " << "SELECT: " << list;
        qDebug() << "getMachineListFromDb: " << "SELECT: " << query.lastError().text();

    };

    return list;

}

//**************************************************************************************************
//
//**************************************************************************************************

void MainWindow::getDataFromConfig()
{

    extensions = getMachineFileExtFromDb(currentMachineName);
    mainPath = getMachineMainPathFromDb(currentMachineName);
    savePath = getMachineSavePathFromDb(currentMachineName);

    qDebug() << "getDataFromConfig: " << mainPath << savePath << extensions;

//    QString rootPath, dir1;

//    QSettings settings(QSettings::IniFormat, QSettings::UserScope, "EdytorNC", "EdytorNC");
//    settings.beginGroup("PMConfigs");

//    rootPath = settings.value("RootPath", QDir::rootPath()).toString();

//    settings.beginGroup(currentMachineName);

//    QString t_ext = settings.value("ListExt", "*.nc").toString();

//    extensions.append(t_ext.split(";", QString::SkipEmptyParts));
//    extensions.removeDuplicates();
//    extensions.sort();

//    dir1 = QDir(rootPath).absoluteFilePath(settings.value("RelativePath", "").toString());

//    if(QDir(dir1).exists())
//        mainPath = dir1;
//    else
//       mainPath = "";

//    dir1 = QDir(dir1).absoluteFilePath(settings.value("SaveDir", "").toString());

//    if(QDir(dir1).exists())
//        downloadPath = dir1;
//    else
//       downloadPath = "";

//    settings.endGroup();
//    settings.endGroup();



}

//**************************************************************************************************
//
//**************************************************************************************************

void MainWindow::tableCellDoubleClicked(const QModelIndex &index)
{
    QString files;

    qDebug() << "Double clicked: " << index.row() << index.column();


    files = fileNamesFromIndex(index);
    if(!files.isEmpty())
        openInEditor(files);

}

//**************************************************************************************************
//
//**************************************************************************************************

QString MainWindow::fileNamesFromIndex(const QModelIndex &index)
{
    QString files, info1, tx;
    int startRow, endRow;

    files = "";


    qDebug() << "fileNamesFromIndex: " << index.row() << index.column();

    if(index.column() > 1)
    {
        QString path1 = mainPath;
        if(!path1.endsWith("/"))
            path1 = path1 + "/";


        info1 = ui->tableView->model()->data(ui->tableView->model()->index(index.row(),3)).toString().simplified();

        startRow = index.row() - 1;
        while(true)
        {
            if(startRow < 0)
                break;

            tx = ui->tableView->model()->data(ui->tableView->model()->index(startRow, 3)).toString().simplified();
            if((info1 == tx))
            {
                startRow--;
            }
            else
                break;

        };
        startRow++;

        endRow = index.row() + 1;
        while(true)
        {
            if(endRow > ui->tableView->model()->rowCount())
                break;


            tx = ui->tableView->model()->data(ui->tableView->model()->index(endRow, 3)).toString();
            if((info1 == tx))
            {
                endRow++;
            }
            else
                break;

        };
        endRow--;

//        qDebug() << "Start, end: " << startRow << endRow;

        for(int i = startRow; i <= endRow; ++i)
        {

            tx = ui->tableView->model()->data(ui->tableView->model()->index(i, 2)).toString();
            if(QFile::exists(path1 + tx))
            {
                files.append("\"" + path1 + tx + "\"" + " ");
            };
        };
    };

    qDebug() << "Filesforindex: " << files;

    return files;
}

//**************************************************************************************************
//
//**************************************************************************************************

//void MainWindow::tableCellDoubleClicked(int row, int col)
//{
//    QString files, info1;
//    QTableWidgetItem *item;
//    int startRow, endRow;


//    qDebug() << "Double clicked: " << row << col;

//    if(col == 0)
//    {

//        QString path1 = mainPath;
//        if(!path1.endsWith("/"))
//            path1 = path1 + "/";


//        item = ui->fileTableWidget->item(row, 1);
//        if(item)
//        {
//            info1 = item->text();
//        };

//        startRow = row - 1;
//        while(true)
//        {
//            if(startRow < 0)
//                break;

//            item = ui->fileTableWidget->item(startRow, 1);
//            if(item)
//            {
//                if((info1 == item->text()))
//                {
//                    startRow--;
//                }
//                else
//                    break;
//            }
//            else
//                break;
//        };
//        startRow++;

//        endRow = row + 1;
//        while(true)
//        {
//            if(endRow > ui->fileTableWidget->rowCount())
//                break;

//            item = ui->fileTableWidget->item(endRow, 1);
//            if(item)
//            {
//                if((info1 == item->text()))
//                {
//                    endRow++;
//                }
//                else
//                    break;
//            }
//            else
//                break;
//        };
//        endRow--;

//        qDebug() << "Start, end: " << startRow << endRow;

//        for(int i = startRow; i <= endRow; ++i)
//        {
//            item = ui->fileTableWidget->item(i, 0);
//            if(item)
//            {
//                if(QFile::exists(path1 + item->text()))
//                {
//                    files.append(path1 + item->text() + " ");
//                };

//            };

//            qDebug() << "Start, end, i: " << startRow << endRow << i;
//        }

//        qDebug() << "Files: " << files;

//        if(!files.isEmpty())
//            openInEditor(files);
//    };
//}

//**************************************************************************************************
//
//**************************************************************************************************

void MainWindow::openInEditor(QString files)
{

    QString exeName, path;

    path = QApplication::applicationDirPath() + "/";
    path = QDir::toNativeSeparators(path);

#ifdef Q_OS_LINUX
    fileName = path + "EdytorNC";
#endif

#ifdef Q_OS_WIN32
    exeName = "EdytorNC.exe";
#endif

    edytorProc = findChild<QProcess *>("EdytorNCApp");

    if(!edytorProc)
    {
        edytorProc = new QProcess(this);
        edytorProc->setObjectName("EdytorNCApp");
        edytorProc->setWorkingDirectory(path);
        edytorProc->startDetached(exeName + " " + files);

    }
    else
        if(edytorProc->processId() == 0)
        {
            edytorProc->setWorkingDirectory(path);
            edytorProc->startDetached(exeName  + " " + files);
        };

}

//**************************************************************************************************
//
//**************************************************************************************************

bool MainWindow::createConnection()
{
    bool dbType;
    QString dbHost;
    QString dbUser;
    QString dbPass;
    uint dbPort;

    QSettings settings(QSettings::IniFormat, QSettings::UserScope, "EdytorNC", "EdytorNC");

    settings.beginGroup("PMConfigs");

    refreshTime1 = settings.value("RefreshTime1", tr("00:00")).toTime();
    refreshTime2 = settings.value("RefreshTime2", tr("00:00")).toTime();



    dbType = settings.value("DBType", true).toBool();
    dbHost = settings.value("DBHost", tr("127.0.0.1")).toString();
    dbUser = settings.value("DBUser", tr("edytornc")).toString();
    dbPass = settings.value("DBPass", tr("edytornc")).toString();
    dbPort = settings.value("DBPort", 3306).toUInt();

    settings.endGroup();

    if(dbType)
    {
        db = QSqlDatabase::addDatabase("QSQLITE");
        db.setDatabaseName("ProgramManager.db");
    }
    else
    {
        db = QSqlDatabase::addDatabase("QMYSQL");
        db.setHostName(dbHost);
        db.setPort(dbPort);
        //        db.setDatabaseName("ProgramManager");
        db.setUserName(dbUser);
        db.setPassword(dbPass);

        if(db.open())
        {
            QSqlQuery query(db);
            // Create your database if it does not exist already
            query.exec("CREATE DATABASE IF NOT EXISTS ProgramManager");

            qDebug() << "createConnection: " << db.lastError().text();
        }
        else
        {
            QMessageBox::critical(nullptr, QObject::tr("Cannot open database"),
                                  QObject::tr("Unable to establish a database connection.\nError: %1").arg(db.lastError().text()), QMessageBox::Ok);
            return false;
        };

        db.close();
        db.setDatabaseName("ProgramManager");

    };

    qDebug() << "createConnection: " << db.driverName();

    return tryOpenDb();

}
//**************************************************************************************************
//
//**************************************************************************************************

bool MainWindow::tryOpenDb(bool silent)
{
    if(db.isOpen())
    {

        QSqlQuery query(db);
        if(query.exec("SELECT 1"))
            return true;

    };

    db.close();
    if(!db.open())
    {
        qDebug() << "isDbOpen: " << db.driverName() << db.lastError().text();

        if(!silent)
            QMessageBox::critical(nullptr, QObject::tr("Cannot open database"),
                                  QObject::tr("Unable to establish a database connection.\nError: %1").arg(db.lastError().text()), QMessageBox::Ok);
        return false;
    };


    return true;
}

//**************************************************************************************************
//
//**************************************************************************************************

void MainWindow::initDatabase()
{
    if (!tryOpenDb())
    {
        return;
    };


    QSqlQuery query(db);
    if(db.driverName() == "QSQLITE")
    {
        query.exec(QString("CREATE TABLE IF NOT EXISTS programs (id INT PRIMARY KEY, machine varchar(128), fileName varchar(20), "
                                       "infoName1 varchar(128), infoName2 varchar(128), infoName3 varchar(128), size varchar(64), date varchar(64))"));

        qDebug() << "initDatabase: " << "CREATE TABLE: " << query.lastError().text();


        query.exec(QString("CREATE TABLE IF NOT EXISTS config (id INT PRIMARY KEY, machine varchar(128), rootpath varchar(128), "
                           "relativepath varchar(64), savepath varchar(64), extensions varchar(64))"));

        qDebug() << "initDatabase: " << "CREATE TABLE: " << query.lastError().text();
    }
    else
    {
        query.exec(QString("CREATE TABLE IF NOT EXISTS programs (id INT NOT NULL AUTO_INCREMENT, machine varchar(128), fileName varchar(20), "
                                       "infoName1 varchar(128), infoName2 varchar(128), infoName3 varchar(128), size varchar(64), date varchar(64), PRIMARY KEY (id))"));

        qDebug() << "initDatabase: " << "CREATE TABLE: " << query.lastError().text();


        query.exec(QString("CREATE TABLE IF NOT EXISTS config (id INT NOT NULL AUTO_INCREMENT, machine varchar(128), rootpath varchar(128), "
                                       "relativepath varchar(64), savepath varchar(64), extensions varchar(64), PRIMARY KEY (id))"));

        qDebug() << "initDatabase: " << "CREATE TABLE: " << query.lastError().text();
    };

}

//**************************************************************************************************
//
//**************************************************************************************************

void MainWindow::configPushButton_clicked()
{
    PmConfigDialog *dlg = new PmConfigDialog(this, &db);
    dlg->setAttribute( Qt::WA_DeleteOnClose, true );
    dlg->setModal(true);

    if(dlg->exec() == QDialog::Accepted)
    {
        loadConfignames();
        ui->listView->adjustSize();
    };

}

//**************************************************************************************************
//
//**************************************************************************************************

void MainWindow::copyFiles()
{
    QString tx;

    if(!ui->tableView->selectionModel()->hasSelection())
        return;

    tx = fileNamesFromIndex(ui->tableView->currentIndex());


    copyFileDialog *dlg = new copyFileDialog;
    dlg->setAttribute( Qt::WA_DeleteOnClose, true );
    dlg->setModal(true);
    dlg->setDatabase(&db);
    dlg->setSource(currentMachineName, tx);


    if(dlg->exec() == QDialog::Accepted)
    {
//        loadSerialConfignames();
//        ui->listView->adjustSize();
    };

}

//**************************************************************************************************
//
//**************************************************************************************************

QStringList MainWindow::getMachineListFromConfig()
{
    QStringList list;

    QSettings settings(QSettings::IniFormat, QSettings::UserScope, "EdytorNC", "EdytorNC");

    settings.beginGroup("PMConfigs");

    list = settings.value("SettingsList", QStringList(tr(""))).toStringList();

    settings.endGroup();

    list.removeDuplicates();
    list.sort();

    qDebug() << "getMachineListFromConfig: " << list;

    return list;
}

//**************************************************************************************************
//
//**************************************************************************************************

void MainWindow::rescanFiles(QString machine, bool deleteOldRecords)
{
    QStringList list;
    QString rootPath;
    QMessageBox msgBox;


    QSettings settings(QSettings::IniFormat, QSettings::UserScope, "EdytorNC", "EdytorNC");
    settings.beginGroup("PMConfigs");

    rootPath = settings.value("RootPath", QDir::rootPath()).toString();

    settings.endGroup();


    if((!QDir(rootPath).exists()) || QDir(rootPath).entryList(QDir::AllDirs | QDir::NoDotAndDotDot | QDir::Readable).isEmpty())
    {
        qDebug() << "rescanFiles" << "Root path not accessible?";

        return;
    };


    scanAllFilesAct->setEnabled(false);
    QApplication::setOverrideCursor(Qt::BusyCursor);

    QSqlQuery query(db);

    if(machine.isEmpty())  //  rescan all if machine is empty
    {
        list = getMachineListFromDb();

//        query.exec(QString("DELETE FROM programs;"));

//        qDebug() << "rescanFiles" << "DELETE FROM ALL " << query.lastError().text();
    }
    else
    {
        list.append(machine);

//        query.exec(QString("DELETE FROM programs WHERE machine='%1'").arg(machine));

//        qDebug() << "rescanFiles" << "DELETE FROM: " << query.lastError().text();

    };

    msgBox.setText(tr("Scanning files in %1. Please wait...").arg(""));
    msgBox.setIcon(QMessageBox::Information);
    msgBox.setStandardButtons(QMessageBox::NoButton);
    msgBox.show();


    for(const auto& i : qAsConst(list))
    {
        msgBox.setText(tr("Scanning files in %1. Please wait...").arg(i));
        QApplication::processEvents();

        QString path = getMachineMainPathFromDb(i);
        QStringList ext = getMachineFileExtFromDb(i);

        if(QDir(path).exists())
        {
            if(deleteOldRecords)
            {
                query.exec(QString("DELETE FROM programs WHERE machine='%1'").arg(i));

                qDebug() << "rescanFiles:" << "DELETE FROM: " << i << query.lastError().text();
            };

            updateFileTable(i, path, ext);

        };

        qDebug() << "rescanFiles: " << i << path << ext;
    };

    msgBox.close();


    scanAllFilesAct->setEnabled(true);
    QApplication::restoreOverrideCursor();

}

//**************************************************************************************************
//
//**************************************************************************************************

bool MainWindow::eventFilter(QObject *object, QEvent *event)
{
    QString name, text;
    QDir dir;

//    qDebug() << object->objectName() << " Event: " << event->type();

    QHelpEvent *helpEvent = static_cast<QHelpEvent *>(event);

    if((event->type() == QEvent::ToolTip))
    {
        name = "";

        if(object->objectName() == "listView")
        {
            dir.setFilter(QDir::Files);

            QModelIndex idx = ui->listView->indexAt(ui->listView->mapFromGlobal(helpEvent->globalPos()));
            name = idx.data().toString();
        };

//        qDebug() << name;

        if(!name.isEmpty())
        {
            dir.setPath(getMachineSavePathFromDb(name));
            dir.setFilter(QDir::Files);
            dir.setNameFilters(extensions);

            uint noOfFiles = dir.count();

            qDebug() << name << "noOfFiles: " << noOfFiles << "Dir: " << dir.path();

            if(noOfFiles > 0)
                text = tr("%1:  %2 files received awaiting review").arg(name).arg(noOfFiles);
            else
                text = name;

            QToolTip::showText(helpEvent->globalPos(), text, this, QRect());

            return true;
        }
        else
        {
            QToolTip::hideText();
            event->ignore();

        };
    };

//    if((event->type() == QEvent::KeyRelease) && (object->objectName() == "tableView"))
//    {
////        qDebug() << "helpEvent" << helpEvent;

//        QKeyEvent *k = (QKeyEvent*) event;

//        if((k->key() == Qt::Key_Up) || (k->key() == Qt::Key_Down))
//        {
//            tableViewItemClicked(ui->tableView->currentIndex());
//        };

//    };

//    if((event->type() == QEvent::Enter) && (object->objectName() == "tableView"))
//    {
//        qDebug() << "helpEvent" << helpEvent;

////        tableViewItemClicked(ui->tableView->indexAt(ui->tableView->mapFromGlobal(helpEvent->globalPos())));
//        tableViewItemClicked(ui->tableView->currentIndex());

//    };

    return QObject::eventFilter(object, event);
}

//**************************************************************************************************
//
//**************************************************************************************************

void MainWindow::showNewFilesActHovered()
{
    QString name, text;
    QDir dir;

    name = currentMachineName;

    uint noOfFiles = 0;

    if(!name.isEmpty())
    {
        dir.setPath(getMachineSavePathFromDb(name));
        dir.setFilter(QDir::Files);
        dir.setNameFilters(extensions);
        noOfFiles = dir.count();

        qDebug() << "showNewFilesActHovered: " << name << noOfFiles << dir.path();

    };

//    qDebug() << name << "noOfFiles: " << noOfFiles << "Dir: " << dir.path();


    if(noOfFiles > 0)
        text = tr("%1:  %2 files received awaiting review").arg(name).arg(noOfFiles);
    else
        text = name + " " + tr("Show saved files");

    showNewFilesAct->setToolTip(text);

}

//**************************************************************************************************
//
//**************************************************************************************************

void MainWindow::closeEvent(QCloseEvent *event)
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

void MainWindow::tableViewItemClicked(QModelIndex index)
{

    QString info1, tx;
    int startRow, endRow;


//    qDebug() << "Clicked: " << index.row() << index.column();

    if(index.column()  > 0)
    {

        QString path1 = mainPath;
        if(!path1.endsWith("/"))
            path1 = path1 + "/";


        info1 = ui->tableView->model()->data(ui->tableView->model()->index(index.row(), 3)).toString().simplified();

        startRow = index.row() - 1;
        while(true)
        {
            if(startRow < 0)
                break;

            tx = ui->tableView->model()->data(ui->tableView->model()->index(startRow, 3)).toString().simplified();
            if((info1 == tx))
            {
                startRow--;
            }
            else
                break;

        };
        startRow++;

        endRow = index.row() + 1;
        while(true)
        {
            if(endRow > ui->tableView->model()->rowCount())
                break;


            tx = ui->tableView->model()->data(ui->tableView->model()->index(endRow, 3)).toString();
            if((info1 == tx))
            {
                endRow++;
            }
            else
                break;

        };
        endRow--;

        qDebug() << "Start, end: " << startRow << endRow;

        ui->tableView->setUpdatesEnabled(false);
        ui->tableView->blockSignals(true);

//        tx = ui->tableView->model()->data(ui->tableView->model()->index(startRow, 2)).toString();

        QItemSelection sel = ui->tableView->selectionModel()->selection();

        sel.select(ui->tableView->model()->index(startRow, 0), ui->tableView->model()->index(endRow, ui->tableView->model()->columnCount() - 1));

        ui->tableView->selectionModel()->select(sel, QItemSelectionModel::Select | QItemSelectionModel::Rows);

//        qDebug() << "QItemSelection" << sel;

        ui->tableView->blockSignals(false);
        ui->tableView->setUpdatesEnabled(true);
    };

//    qDebug() << "tableViewItemClicked: " << files;

}

//**************************************************************************************************
//
//**************************************************************************************************

void MainWindow::refreshTimerTimout()
{
    if(refreshTime1.toString("HH:mm") != "00:00")
        if(QTime().currentTime().toString("HH:mm:ss") == refreshTime1.toString("HH:mm:01"))
            rescanFiles();

    if(refreshTime2.toString("HH:mm") != "00:00")
        if(QTime().currentTime().toString("HH:mm:ss") == refreshTime2.toString("HH:mm:01"))
            rescanFiles();

//    qDebug() << "refreshTimerTimout: " << "files" << QTime().currentTime().toString("HH:mm:00");
}

//**************************************************************************************************
//
//**************************************************************************************************

void MainWindow::updateFileInDb(QString machine, QString fileName)
{
    int pos;
    QRegExp exp;
    qint64 size;
    QString line;
    QStringList fileList, comments, prevComments;
    QStringList labels;

    exp.setCaseSensitivity(Qt::CaseInsensitive);
    exp.setPattern("\\([^\\n\\r]*\\)|;[^\\n\\r]*");


    bool alreadyExist = false;

    QSqlQuery query;
    QFile file(fileName);

    query.prepare("SELECT date FROM programs WHERE machine=:machine AND fileName=:fileName");
    query.bindValue(":machine", machine);
    query.bindValue(":fileName", QFileInfo(file.fileName()).fileName());

    if(!query.exec())
    {
        // select failed
    }

    while(query.next())
    {
        QString date = query.value(0).toString();

        if(date == QFileInfo(file).lastModified().toString("yyyy-MM-dd HH:mm:ss"))
        {

//            qDebug() << "updateFileInDb: " << "Same date time, return " << QFileInfo(file.fileName()).fileName();

            return;
        }

        alreadyExist = true;
    };


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
                    comments.append(t);
                };
            };

            if(comments.size() == 3)
                break;

        };

        file.close();

        while(comments.size() < 3)  // ensure taht always are trhee items on the list
            comments.append("");


        if(alreadyExist)
        {
            query.prepare(QString("UPDATE programs SET infoName1=:infoName1, infoName2=:infoName2, infoName3=:infoName3, size=:size, date=:date WHERE machine=:machine AND fileName=:fileName"));
            query.bindValue(":machine", machine);
            query.bindValue(":fileName", QFileInfo(file.fileName()).fileName());
            query.bindValue(":infoName1", comments[0]);
            query.bindValue(":infoName2", comments[1]);
            query.bindValue(":infoName3", comments[2]);
            query.bindValue(":date", QFileInfo(file).lastModified().toString("yyyy-MM-dd HH:mm:ss"));
            query.bindValue(":size", tr("%1 KB").arg(int((size + 1023) / 1024)));
            query.exec();


            //            qDebug() << "UPDATE: " << query.lastError().text();
            //            qDebug() << query.lastQuery();

//            qDebug() << "updateFileInDb:" << "UPDATE" << QFileInfo(file.fileName()).fileName();
        }
        else
        {
            query.prepare(QString("INSERT INTO programs (machine, fileName, infoName1, infoName2, infoName3, size, date) VALUES (:machine, :fileName, :infoName1, :infoName2, :infoName3, :size, :date)"));
            query.bindValue(":machine", machine);
            query.bindValue(":fileName", QFileInfo(file.fileName()).fileName());
            query.bindValue(":infoName1", comments[0]);
            query.bindValue(":infoName2", comments[1]);
            query.bindValue(":infoName3", comments[2]);
            query.bindValue(":date", QFileInfo(file).lastModified().toString("yyyy-MM-dd HH:mm:ss"));
            query.bindValue(":size", tr("%1 KB").arg(int((size + 1023) / 1024)));
            query.exec();


//                            qDebug() << "INSERT: " << query.lastError().text();
//                            qDebug() << query.lastQuery();

//            qDebug() << "updateFileInDb:" << "INSERT" << QFileInfo(file.fileName()).fileName();
        };


    };

}

//**************************************************************************************************
//
//**************************************************************************************************

void MainWindow::fileChangedSlot(const QString &path)
{
    QDir dir;

    QString machine = getMachineNameFromPath(path);

    qDebug() << "fileChangedSlot: " << "path " << path;


    dir.setPath(path);
    dir.setFilter(QDir::Files | QDir::NoSymLinks | QDir::Readable | QDir::Modified);
    dir.setNameFilters(getMachineFileExtFromDb(machine));
    dir.setSorting(QDir::Time | QDir::IgnoreCase);


    QStringList fileList = dir.entryList();

    for(int i = 0; i < qMin(fileList.size(), 1000); ++i)
    {
         QString filePath = QFileInfo(dir, fileList[i]).filePath();

         updateFileInDb(machine, filePath);

         qDebug() << "fileChangedSlot: " << "New file " << filePath << i;
    };
}

//**************************************************************************************************
//
//**************************************************************************************************

QString MainWindow::getMachineNameFromPath(const QString &path)
{

    QString rootPath, relativePath, machine;

    if(tryOpenDb())
    {

        QSettings settings(QSettings::IniFormat, QSettings::UserScope, "EdytorNC", "EdytorNC");
        settings.beginGroup("PMConfigs");

        rootPath = settings.value("RootPath", QDir::rootPath()).toString();

        settings.endGroup();

        relativePath = QDir(rootPath).relativeFilePath(path);

//        relativePath = QFileInfo(relativePath).path();


        qDebug() << "getMachineNameFromPath: " << relativePath << path << rootPath;


        QSqlQuery query;

        query.prepare(QString("SELECT machine FROM config WHERE relativepath=:relativepath"));
        query.bindValue(":relativepath", relativePath);
        query.exec();

//        qDebug() << "getMachineMainPathFromDb: " << "SELECT: " << query.lastError().text();

        while(query.next())
        {
            machine = query.value(0).toString();
        };
    };


    qDebug() << "getMachineNameFromPath: " << machine;

    return machine;

}
//**************************************************************************************************
//
//**************************************************************************************************















//QSqlQuery query;

//if(!db.tables().contains(tableName.toLatin1()))
//{
//    qDebug() << query.exec(QString("create table %1 (fileName varchar(20) primary key, "
//                                   "infoName1 varchar(128), infoName2 varchar(128), infoName3 varchar(128), size varchar(64), date varchar(64))").arg(tableName));
//    qDebug() << query.lastError();
//};
//     // qDebug() << query.exec(QString("INSERT INTO %1 VALUES(%2, %3, %4)").arg(configBox->currentText(), files[i], comments[0], comments[1]));



//query.prepare(QString("UPDATE %1 SET infoName1=:infoName1, infoName2=:infoName2, infoName3=:infoName3, size=:size, date=:date WHERE fileName=:fileName").arg(tableName));
//query.bindValue(":fileName", files[i]);
//query.bindValue(":infoName1", comments[0]);
//query.bindValue(":infoName2", comments[1]);
//query.bindValue(":infoName3", comments[2]);
//query.bindValue(":date", QFileInfo(file).lastModified().toString("yyyy-MM-dd HH:mm:ss"));
//query.bindValue(":size", tr("%1 KB").arg(int((size + 1023) / 1024)));
//query.exec();


////            qDebug() << query.lastError();
////            qDebug() << query.lastQuery();


//if (query.numRowsAffected() == 0)
//{
//    query.prepare(QString("INSERT INTO %1 (fileName, infoName1, infoName2, infoName3, size, date) VALUES (:fileName, :infoName1, :infoName2, :infoName3, :size, :date)").arg(tableName));
//    query.bindValue(":fileName", files[i]);
//    query.bindValue(":infoName1", comments[0]);
//    query.bindValue(":infoName2", comments[1]);
//    query.bindValue(":infoName3", comments[2]);
//    query.bindValue(":date", QFileInfo(file).lastModified().toString("yyyy-MM-dd HH:mm:ss"));
//    query.bindValue(":size", tr("%1 KB").arg(int((size + 1023) / 1024)));
//    query.exec();


////                qDebug() << query.lastError();
////                qDebug() << query.lastQuery();
//};









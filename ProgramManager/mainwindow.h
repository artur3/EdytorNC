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


#ifndef MAINWINDOW_H
#define MAINWINDOW_H


#include <QtWidgets>
#include <QSettings>
#include <QFileDialog>
#include <QMainWindow>
#include <QSqlTableModel>
#include <QTableView>

#include "../FileChecker/filechecker.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

public slots:
    void messReceived(const QString &text = "");

protected:
        bool eventFilter(QObject *object, QEvent *event);
        void closeEvent(QCloseEvent *event);


private slots:
    void iconActivated(QSystemTrayIcon::ActivationReason reason);
    void createTrayIcon();
    void about();
    void createActions();
    void quitApp();

    void findFiles();
    void applyFilter();

    void showNewFiles();
    void browseSaveFolder();
    void tableCellDoubleClicked(const QModelIndex &index);
    void tableViewItemClicked(QModelIndex index);

    void changeTable();
    void listViewItemClicked(QModelIndex index);

    void configPushButton_clicked();
    void copyFiles();
    void rescanFiles(QString machine = "", bool deleteOldRecords = true);

    void fileChangedSlot(const QString &path);
    void showNewFilesActHovered();
    void refreshTimerTimout();
signals:
    void needToShow();


private:
    Ui::MainWindow *ui;

    QSystemTrayIcon *trayIcon;
    QMenu *trayIconMenu;

    QAction *closeServerAct;
    QAction *minimizeAction;
    QAction *maximizeAction;
    QAction *restoreAction;
    QAction *quitAction;
    QAction *aboutAction;
    QAction *aboutQtAct;
    QAction *startMinimizedAction;
    QAction *reloadAct;
    QAction *stopServerAct;
    QAction *configPortAct;
    QAction *browseSaveFolderAct;
    QAction *showNewFilesAct;
    QAction *copyFilesAct;
    QAction *scanAllFilesAct;
    //QComboBox *configBox;
    QToolBar *fileToolBar;
    QToolBar *windowToolBar;

    QProcess *edytorProc;

    QStringList extensions;
    QString mainPath;
    QString savePath;

    QSqlDatabase db;
    QSqlTableModel *tableViewModel;

    QString currentMachineName;

    QPointer<QFileSystemWatcher> fileWatcher;
    QStringListModel *listViewModel;


    bool closable;

    QTimer *refreshTimer;
    QTime refreshTime1;
    QTime refreshTime2;


    void updateFileTable(const QString machine, const QString startDir, QStringList fileFilter);
    void findFiles(const QString startDir, QStringList fileFilter);
    void loadConfignames();
    void getDataFromConfig();
    void openInEditor(QString files);
    bool createConnection();
    QString fileNamesFromIndex(const QModelIndex &index);
    QString getMachineMainPathFromConfig(const QString machineName);

    void on_lineFind_textEdited(const QString &arg1);





    QStringList getMachineListFromConfig();
    QStringList getFileExtensionsFromConfig(const QString machineName);
    QString getMachineSavePathFromConfig(const QString machineName);
    void initDatabase();
    QString getMachineMainPathFromDb(const QString machineName);
    QString getMachineSavePathFromDb(const QString machineName);
    QStringList getMachineFileExtFromDb(const QString machineName);
    QStringList getMachineListFromDb();
    bool tryOpenDb(bool silent = false);
    void updateFileInDb(QString machine, QString fileName);
    QString getMachineNameFromPath(const QString &path);
};

#endif // MAINWINDOW_H

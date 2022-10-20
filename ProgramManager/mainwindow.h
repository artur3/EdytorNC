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


#ifndef MAINWINDOW_H
#define MAINWINDOW_H


#include <QtWidgets>
#include <QSettings>
#include <QFileDialog>
#include <QMainWindow>

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

private slots:
    void iconActivated(QSystemTrayIcon::ActivationReason reason);
    void createTrayIcon();
    void about();
    void createActions();
    void quitApp();

    void findFiles();

    void showNewFiles();
    void browseSaveFolder();
    void tableCellDoubleClicked(int row, int col);


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
    QComboBox *configBox;
    QToolBar *fileToolBar;

    QProcess *edytorProc;

    QStringList extensions;
    QString mainPath;
    QString downloadPath;

    void findFiles(const QString startDir, QStringList fileFilter);
    void loadSerialConfignames();
    void getPathFromConfig();
    void openInEditor(QString files);

};

#endif // MAINWINDOW_H

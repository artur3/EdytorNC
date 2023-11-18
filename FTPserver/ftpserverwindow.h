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
 *   Copyright (C) 2022-2024 by Artur Kozioł                               *
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


#ifndef FTPSERVERWINDOW_H
#define FTPSERVERWINDOW_H

#include <QMainWindow>
#include "ftpconfigdialog.h"
#include "../FileChecker/filechecker.h"
//#include "../KDiff3App/kdiff3app.h"

namespace Ui {
    class FTPserverWindow;
}

class FtpServer;
class FileChecker;
//class KDiff3App;

class FTPserverWindow : public QMainWindow
{
    Q_OBJECT
public:
    explicit FTPserverWindow(QWidget *parent = 0);
    virtual ~FTPserverWindow();
    void showExpanded();
    void appendText(QtMsgType type, const QString &text);

public slots:
    void messReceived(const QString &text = "");

protected:
    void closeEvent(QCloseEvent *event);


private slots:
    void pushButtonRestartServer_clicked();
    void toolButtonBrowse_clicked();
    void onPeerIpChanged(const QString &peerIp);
    void pushButtonExit_clicked();
    void iconActivated(QSystemTrayIcon::ActivationReason reason);
    void quitApp();
    void about();
    void clearToolButton_clicked();
    void configPushButton_clicked();
    void showNewFiles();
    void browseSaveFolder();

signals:
    void needToShow();

private:
    Ui::FTPserverWindow *ui;

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
    QAction *resetServerAct;
    QAction *stopServerAct;
    QAction *configPortAct;
    QAction *browseSaveFolderAct;
    QAction *showNewFilesAct;
    QComboBox *configBox;
    QToolBar *fileToolBar;

    bool closable;


    QString lineEditPort;
    QString lineEditUserName;
    QString lineEditPassword;
    QString lineEditRootPath;
    bool checkBoxAnonymous;
    bool checkBoxReadOnly;
    bool checkBoxOnlyOneIpAllowed;



    // This is the FTP server object.
    FtpServer *server;

    // Load the stored settings.
    void loadSettings();

    // Save the current settings.
    void saveSettings();

    // Restart the FTP server.
    void startServer();
    void createActions();
    void createTrayIcon();
    void loadSerialConfignames();
    QStringList getPathFromConfig();


};

#endif // FTPSERVERWINDOW_H

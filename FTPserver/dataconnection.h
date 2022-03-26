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
 *   Copyright (C) 2022-2022 by Artur Kozioł                               *
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

#ifndef PASSIVEDATACONNECTION_H
#define PASSIVEDATACONNECTION_H

class SslServer;
class QSslSocket;
class FtpCommand;

#include <QObject>
#include <QPointer>

// This class encapsulates the logic of the creation of both passive and active
// data connections. In short - there can be only one data connection per
// control connection. If a new one is requested while there is one being used
// currently, the operation using it is aborted. A new data connection starts
// when a PASV or PORT command is issued. Then the user must specify a command
// that uses the data connection, such as LIST or RETR/STOR. If we have
// encryption enabled, the data connection is also first encrypted. Once we
// have the command, and are connected to the client, and the connection has
// been encrypted, we run the command.

class DataConnection : public QObject
{
    Q_OBJECT
public:
    explicit DataConnection(QObject *parent = 0);

    // Connects to a host. Any existing data connections
    // or commands are aborted.
    void scheduleConnectToHost(const QString &hostName, int port, bool encrypt);

    // Starts listening for new data connections. Any existing data connections
    // or commands are aborted.
    int listen(bool encrypt);

    // Sets the ftp command. This function can be called only once after each
    // call of listen().
    bool setFtpCommand(FtpCommand *command);

    // Returns the currently running ftpCommand, if it is already running, but
    // not yet finished. Otherwise returns 0.
    FtpCommand *ftpCommand();

signals:

private slots:
    void newConnection();
    void encrypted();
    void connected();

private:
    void startFtpCommand();
    SslServer *server;
    QSslSocket *socket;
    QPointer<FtpCommand> command;
    bool isSocketReady;
    bool isWaitingForFtpCommand;
    bool encrypt;

    // Used for the active data connection (PORT command).
    bool isActiveConnection;
    QString hostName;
    int port;
};

#endif // PASSIVEDATACONNECTION_H

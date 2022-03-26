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

#ifndef FTPSERVER_H
#define FTPSERVER_H

#include <QObject>
#include <QSet>

class SslServer;

// The ftp server. Listens on a port, and starts a new control connection each
// time it gets connected.

class FtpServer : public QObject
{
    Q_OBJECT
public:
    explicit FtpServer(QObject *parent, const QString &rootPath, int port = 21,
                       const QString &userName = QString(), const QString &password = QString(),
                       bool readOnly = false, bool onlyOneIpAllowed = false);

    // Whether or not the server is listening for incoming connections. If it
    // is not currently listening then there was an error - probably no
    // internet connection is available, or the port address might require root
    // priviledges (on Linux).
    bool isListening();

    // Get the LAN IP of the host, e.g. "192.168.1.10".
    static QString lanIp();

signals:
    // A connection from a new IP has been established. This signal is emitted
    // when the FTP server is connected by a new IP. The new IP will then be
    // stored and will not cause this FTP server instance to emit this signal
    // any more.
    void newPeerIp(const QString &ip);

private slots:
    // Called by the SSL server when we have received a new connection.
    void startNewControlConnection();

private:
    // If both username and password are empty, it means anonymous mode - any
    // username and password combination will be accepted.
    QString userName;
    QString password;

    // The root path is the virtual root directory of the FTP. It works like
    // chroot (see http://en.wikipedia.org/wiki/Chroot). The FTP server will
    // not access files or folders that are not in this folder or any of its
    // subfolders.
    QString rootPath;

    // The SSL server listen for incoming connections.
    SslServer *server;

    // All the IPs that this FTP server object has encountered in its lifetime.
    // See the signal newPeerIp.
    QSet<QString> encounteredIps;

    // Whether or not the server is in read-only mode. In read-only mode the
    // server will not create, modify or delete any files or directories.
    bool readOnly;

    // Causes the server to remember the first IP that connects to it, and then
    // refuse connections from any other IP. This makes sense because a mobile
    // phone is unlikely to be used from 2 places at once.
    bool onlyOneIpAllowed;
};

#endif // FTPSERVER_H

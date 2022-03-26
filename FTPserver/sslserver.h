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


#ifndef SSLSERVER_H
#define SSLSERVER_H

#include <QTcpServer>

class QSslSocket;

// A simple SSL server. Returns a QSslSocket instead of a QTcpSocket, but
// started in unencrypted mode, so it can be used as a regular QTcpServer, as
// well. The QSslSockets are loaded with a default certificate coming from the
// resource files.

class SslServer : public QTcpServer
{
    Q_OBJECT
public:
    explicit SslServer(QObject *parent);

    // Sets the local certificate and private key for the socket, so
    // startServerEncryption() can be used. We get the local certificate and
    // private key from the application's resources.
    static void setLocalCertificateAndPrivateKey(QSslSocket *socket);

signals:

private:
    // Workaround so we support both Qt 5 and Qt 4.
    // QTcpServer::incomingConnection(int handle) has been changed to
    // QTcpServer::incomingConnection(qintptr handle) this 2nd one is sneaky as it
    // compiles properly but no connections appear to arrive since the compiler
    // doesn’t consider int and qintptr the same.
#if QT_VERSION >= 0x050000
    typedef qintptr PortableSocketDescriptorType;
#else
    typedef int PortableSocketDescriptorType;
#endif

    void incomingConnection(PortableSocketDescriptorType socketDescriptor);
};

#endif // SSLSERVER_H

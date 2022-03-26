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

#include "ftpretrcommand.h"
#include <QFile>
#include <QSslSocket>

FtpRetrCommand::FtpRetrCommand(QObject *parent, const QString &fileName, FtpSettings &settings, qint64 seekTo) :
    FtpCommand(parent)
{
    this->seekTo = seekTo;
    file = 0;
    this->settings = settings;

    QString fName = fileName;
    QFileInfo fileInfo;
    QString ext;

    if(fName.count('.') > 1)  //check for and remove second extension (if added automatically by machine)
    {
        ext = fileInfo.suffix();
        ext.prepend('.');
        fName.remove(ext);
    };

    qDebug() << "***** 452 FtpRetrCommand: " << fileName << fName;

    fileInfo.setFile(fName);

    if(!fileInfo.exists()) // if file not exist try to apply mask on filename
    {
        qDebug() << QString("***** File name (RAW): %1").arg(fName) << this->settings.fileNameProcessing;

        QString fileNameMask = settings.uploadFileNameMask;

        if(!fileNameMask.isEmpty())
        {
            QString tx = "";
            int i = 0;

            while(i < fileNameMask.length())
            {
                if(fileNameMask[i].isLetterOrNumber() || fileNameMask[i] == '.' || fileNameMask[i] == '_')
                {
                    tx.append(fileNameMask[i]);
                }
                else
                {
                    if(fileNameMask[i] == '$')
                    {
                        i++;

                        QString num;

                        while(i < fileNameMask.length())
                        {
                            if(fileNameMask[i].isDigit())
                                num.append(fileNameMask[i]);
                            else
                            {
                                i--;
                                break;
                            };
                            i++;
                        };

                        int j = num.toInt();

                        if(j > 0)
                            j--;

                        if(j < fName.length())
                            tx.append(fName[j]);
                    };
                };
                i++;
            };

            fName = tx;
            fileInfo.setFile(fName);
        };

        qDebug() << QString("***** File name (mask: %3): %1").arg(fName).arg(settings.fileNameMask);

    };

    if(!fileInfo.exists()) // if file still not exist try to change extension
    {
        QString tmpName = fName;
        QStringList extensions = settings.searchExt.split(';', QString::SkipEmptyParts, Qt::CaseInsensitive);

        for(int i = 0; i < extensions.count(); ++i)
        {
            ext = extensions[i];
            ext.remove("*");
            tmpName.remove('.' + fileInfo.suffix());
            tmpName.append(ext);
            fileInfo.setFile(tmpName);

            qDebug() << "***** 454 FtpRetrCommand: " << fName << tmpName << extensions << ext;

            if(fileInfo.exists())
            {
                fName = tmpName;
                break;
            };
        };
    };

    this->fileName = fName;

    qDebug() << "***** 456 FtpRetrCommand: " << this->fileName;
}

FtpRetrCommand::~FtpRetrCommand()
{
    if (started)
    {
        if (file && file->isOpen() && file->atEnd())
        {
            emit reply("226 Closing data connection.");
        }
        else
        {
            emit reply("550 Requested action not taken; file unavailable.");
        }
    }
}

void FtpRetrCommand::startImplementation()
{
    file = new QFile(fileName, this);

    if (!file->open(QIODevice::ReadOnly))
    {
        deleteLater();
        return;
    }

    emit reply("150 File status okay; about to open data connection.");

    if (seekTo)
    {
        file->seek(seekTo);
    }

    // For encryted SSL sockets, we need to use the encryptedBytesWritten()
    // signal, see the QSslSocket documentation to for reasons why.
    if (socket->isEncrypted()) {
        connect(socket, SIGNAL(encryptedBytesWritten(qint64)), this, SLOT(refillSocketBuffer(qint64)));
    } else {
        connect(socket, SIGNAL(bytesWritten(qint64)), this, SLOT(refillSocketBuffer(qint64)));
    }

    refillSocketBuffer(128*1024);
}

void FtpRetrCommand::refillSocketBuffer(qint64 bytes)
{
    if (!file->atEnd()) {
        socket->write(file->read(bytes));
    } else {
        socket->disconnectFromHost();
    }
}

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

#include "ftplistcommand.h"

#include <QFile>
#include <QDir>
#include <QDateTime>
#include <QTimer>
#include <QSslSocket>

FtpListCommand::FtpListCommand(QObject *parent, const QString &listDirectory, FtpSettings &settings, bool nameListOnly) :
    FtpCommand(parent)
{
    this->listDirectory = listDirectory;
    this->nameListOnly = nameListOnly;
    this->settings = settings;
}

FtpListCommand::~FtpListCommand()
{
    if (started) {
        emit reply("226 Closing data connection.");
    }
}

void FtpListCommand::startImplementation()
{
    QFileInfo info(listDirectory);

    if (!info.isReadable()) {
        emit reply("425 File or directory is not readable or doesn't exist.");
        socket->disconnectFromHost();
        return;
    }

    emit reply("150 File status okay; about to open data connection.");

    index = 0;
    list = new QFileInfoList;
    if (!info.isDir())
    {
        *list = (QFileInfoList() << info);
    }
    else
    {
        QStringList nameFilters = settings.listExt.split(';', QString::SkipEmptyParts, Qt::CaseInsensitive);

        QDir::Filters filter =(settings.listDirs ? QDir::Files | QDir::AllDirs : QDir::Files | QDir::Readable | QDir::NoDotAndDotDot);


         qDebug() << "187 Filters: " << nameFilters << filter;

        *list = QDir(listDirectory).entryInfoList(nameFilters, filter, QDir::Name | QDir::DirsFirst | QDir::IgnoreCase);
    }

    // Start the timer
    timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(listNextBatch()));
    timer->start(0);
}

QString padded(QString s, int n)
{
    while (s.size() < n) {
        s = ' ' + s;
    }
    return s;
}

QString FtpListCommand::fileListingString(const QFileInfo &fi)
{
    // This is how the returned list looks. It is like what is returned by
    // 'ls -l':
    // drwxr-xr-x    9 ftp      ftp          4096 Nov 17  2009 pub

    QString line;
    if (!nameListOnly) {
        // Directory/symlink/file.
        if (fi.isSymLink()) {
            line += 'l';
        } else if (fi.isDir()) {
            line += 'd';
        } else {
            line += '-';
        }

        // Permissions.
        QFile::Permissions p = fi.permissions();
        line += (p & QFile::ReadOwner) ? 'r' : '-';
        line += (p & QFile::WriteOwner) ? 'w' : '-';
        line += (p & QFile::ExeOwner) ? 'x' : '-';
        line += (p & QFile::ReadGroup) ? 'r' : '-';
        line += (p & QFile::WriteGroup) ? 'w' : '-';
        line += (p & QFile::ExeGroup) ? 'x' : '-';
        line += (p & QFile::ReadOther) ? 'r' : '-';
        line += (p & QFile::WriteOther) ? 'w' : '-';
        line += (p & QFile::ExeOther) ? 'x' : '-';

        // Owner/group.
        QString owner = fi.owner();
        if (owner.isEmpty()) {
            owner = "unknown";
        }
        QString group = fi.group();
        if (group.isEmpty()) {
            group = "unknown";
        }
        line += ' ' + padded(owner, 10) + ' ' + padded(group, 10);

        // File size.
        line += ' ' + padded(QString::number(fi.size()), 14);

        // Last modified - note we **must** use english locale, otherwise FTP clients won't understand us.
        QLocale locale(QLocale::English);
        QDateTime lm = fi.lastModified();
        if (lm.date().year() != QDate::currentDate().year()) {
            line += ' ' + locale.toString(lm.date(), "MMM dd  yyyy");
        } else {
            line += ' ' + locale.toString(lm.date(), "MMM dd") + ' ' + locale.toString(lm.time(), "hh:mm");
        }
    }
    line += ' ' + fi.fileName();
    line += "\r\n";
    return line;
}

void FtpListCommand::listNextBatch()
{
    // List next 10 items.
    int stop = qMin(index + 10, list->size());
    while (index < stop) {
        QString line = fileListingString(list->at(index));
        socket->write(line.toUtf8());
        index++;
    }

    // If all files have been listed, then finish.
    if (list->size() == stop) {
        delete list;
        timer->stop();
        socket->disconnectFromHost();
    }
}

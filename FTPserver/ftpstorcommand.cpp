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

#include "ftpstorcommand.h"
#include <QFile>
#include <QSslSocket>

#include <QDebug>


#include "../EdytorNC/commoninc.h"



FtpStorCommand::FtpStorCommand(QObject *parent, const QString &fileName, FtpSettings &settings, bool appendMode, qint64 seekTo) :
    FtpCommand(parent)
{
    this->fileName = fileName;
    this->appendMode = appendMode;
    file = 0;
    this->seekTo = seekTo;
    success = false;
    this->settings = settings;
}

//**************************************************************************************************
//
//**************************************************************************************************

FtpStorCommand::~FtpStorCommand()
{
    if (started)
    {
        if (success)
        {

            file->flush();
            file->close();

            QFile *tmpFile = new QFile(tmpFileName, this);
            QString nName;

            if(settings.fileNameProcessing > 0)
            {
                if(tmpFile->open(QIODevice::ReadWrite))
                {
                    qDebug() << "***** -1 File open error ";

                    QString tx;
                    tx.append(tmpFile->read(256)); // Should be enough to detect program name   //readAll());

                    qDebug() << "***** 0 Settings: " << this->settings.configName;

                    nName = guessFileName(&tx);

                    qDebug() << "***** 1 Detected name: " << nName;
                };
            };

            qDebug() << "***** 2 Filename: " << fileName;

            QDir path;
            if(settings.saveDir.isEmpty())
                path.setPath(settings.rootPath + '/' + settings.relativePath );
            else
                path.setPath(settings.rootPath + '/' + settings.relativePath + '/' + settings.saveDir);


            if(nName.isEmpty())
            {
                QFileInfo fi(fileName);

                if(path.exists())
                    fileName = path.absolutePath() + "/" + fi.fileName();
            }
            else
            {
                if(path.exists())
                {
                    fileName = path.absolutePath() + "/" + nName;
                }
                else
                    success = false;
            };

            fileName = QDir::cleanPath(fileName);

            qDebug() << "***** 3 Save filename and path: " << fileName;

            QFileInfo fileInfo;
            fileInfo.setFile(fileName);

            if(fileInfo.isFile() || !fileInfo.exists())
            {
                QFile dstFile;
                dstFile.setFileName(fileName);

                if(dstFile.exists() && this->settings.renameIfExists)
                {
                    QString backupName = fileInfo.path() + '/' + fileInfo.baseName();
                    backupName.append(".bak");
                    backupName = QDir::cleanPath(backupName);

                    QFile::remove(backupName);

                    if(dstFile.rename(backupName))
                    {
                        // write to log file
                        qDebug() << QString("***** OK: Renaming file: %1 to %2.").arg(fileName).arg(backupName);
                    }
                    else
                    {
                        // write error to log file
                        qDebug() << QString("***** ERROR: When renaming file: %1 . %2").arg(fileName).arg(dstFile.errorString());
                    };
                }
                else
                {
                    QFile::remove(fileName);
                };

                if(tmpFile->rename(fileName))
                {
                    // write to log file
                    qDebug() << QString("***** OK: Succesfully saved file: %1 .").arg(fileName);

                    success = true;
                }
                else
                {
                    // write error to log file
                    qDebug() << QString("***** ERROR: When saving file: %1 . %2").arg(fileName).arg(file->errorString());

                    success = false;
                };
            };
        };

        if(success)
        {
            emit reply("226 Closing data connection.");
        }
        else
        {
            emit reply("451 Requested action aborted: local error in processing.");
        }
    }
}

//**************************************************************************************************
//
//**************************************************************************************************

void FtpStorCommand::startImplementation()
{


    tmpFileName = fileName;
    tmpFileName.replace('.', '_');
    tmpFileName.append(".tmp");


    file = new QFile(tmpFileName, this);

    if (!file->open(appendMode ? QIODevice::Append : QIODevice::WriteOnly))
    {
        deleteLater();
        return;
    }

    success = true;

    emit reply("150 File status okay; about to open data connection.");

    if (seekTo)
    {
        file->seek(seekTo);
    }

    connect(socket, SIGNAL(readyRead()), this, SLOT(acceptNextBlock()));
}

//**************************************************************************************************
//
//**************************************************************************************************

void FtpStorCommand::acceptNextBlock()
{
    const QByteArray &bytes = socket->readAll();
    int bytesWritten = file->write(bytes);

    if (bytesWritten != bytes.size())
    {
        emit reply("451 Requested action aborted. Could not write data to file.");
        deleteLater();
    }
    else
    {

    };
}

//**************************************************************************************************
// Detect program name
//**************************************************************************************************

QString FtpStorCommand::guessFileName(QString *text)
{
    QString fileName, extension, name1, ext1, name2, ext2;
    int pos;
    QRegExp expression;

    if(text->isEmpty())
        return "";

    forever // Detect program name like: O0032, %_N_PR25475002_MPF, $O0004.MIN%...
    {
        expression.setPattern(FILENAME_SINU840);
        pos = text->indexOf(expression);
        if(pos >= 0)
        {
            name1 = text->mid(pos, expression.matchedLength());
            name1.remove("%_N_");
            //name1.remove(QRegExp("_(MPF|SPF|TEA|COM|PLC|DEF|INI)"));

            expression.setPattern("_(MPF|SPF|TEA|COM|PLC|DEF|INI)");
            pos = name1.indexOf(expression);
            if(pos >= 0)
            {
                ext1 = name1.mid(pos, expression.matchedLength());
                name1.remove(ext1);
                ext1.remove(" ");
                ext1.replace('_', '.');
            }
            else
                ext1.clear();

            qDebug() << "3" << name1 << ext1;
            break;
        };

        expression.setPattern(FILENAME_OSP);
        pos = text->indexOf(expression);
        if(pos >= 0)
        {
            name1 = text->mid(pos, expression.matchedLength());
            name1.remove("$");

            expression.setPattern("\\.(MIN|SSB|SDF|TOP|LIB|SUB|MSB)[%]{0,1}");
            pos = name1.indexOf(expression);
            if(pos >= 0)
            {
                ext1 = name1.mid(pos, expression.matchedLength());
                qDebug() << "10.1" << name1 << ext1 << pos;
                name1.remove(ext1);
                ext1.remove(" ");
                ext1.remove("%");
            }
            else
                ext1.clear();

            name1.remove(QRegExp("[%]{0,1}"));
            //name1.remove(QRegExp(".(MIN|SSB|SDF|TOP|LIB|SUB|MSB)[%]{0,1}"));
            qDebug() << "10" << name1 << ext1;
            break;
        };

        expression.setPattern(FILENAME_SINU);
        pos = text->indexOf(expression);
        if(pos >= 0)
        {
            name1 = text->mid(pos, expression.matchedLength());
            //name1.remove(QRegExp("%(MPF|SPF|TEA)[\\s]{0,3}"));

            expression.setPattern("%(MPF|SPF|TEA)[\\s]{0,3}");
            pos = name1.indexOf(expression);
            if(pos >= 0)
            {
                ext1 = name1.mid(pos, expression.matchedLength());
                name1.remove(ext1);
                ext1.remove(" ");
                ext1.remove("%");
                ext1.prepend('.');
            }
            else
                ext1.clear();


            //name1.append(ext);
            qDebug() << "11" << name1 << ext1;
            break;
        };

        expression.setPattern(FILENAME_PHIL);
        pos = text->indexOf(expression);
        if(pos >= 0)
        {
            name1 = text->mid(pos, expression.matchedLength());
            name1.remove(QRegExp("%PM[\\s]{1,}[N]{1,1}"));
            ext1.clear();
            qDebug() << "12" << name1 << ext1;
            break;
        };

        expression.setPattern(FILENAME_FANUC);
        pos = text->indexOf(expression);
        if(pos >= 0)
        {
            name1 = text->mid(pos, expression.matchedLength());
            name1.replace(':', 'O');
            name1.remove("<");
            name1.remove(">");

            ext1.clear();
            pos = name1.lastIndexOf('.');
            if(pos >= 0)
            {
                ext1 = name1.mid(pos, name1.length());
                name1.remove(ext1);
            };

            qDebug() << "13" << name1 << ext1;

            break;
        };

        expression.setPattern(FILENAME_HEID1);
        pos = text->indexOf(expression);
        if(pos >= 0)
        {
            name1 = text->mid(pos, expression.matchedLength());
            name1.remove("%");
            name1.remove(QRegExp("\\s"));
            ext1.clear();

            qDebug() << "14" << name1 << ext1;

            break;
        };

        expression.setPattern(FILENAME_HEID2);
        pos = text->indexOf(expression);
        if(pos >= 0)
        {
            name1 = text->mid(pos, expression.matchedLength());
            name1.remove(QRegExp("(BEGIN)(\\sPGM\\s)"));
            name1.remove(QRegExp("(\\sMM|\\sINCH)"));
            ext1.clear();

            break;
        };

        expression.setPattern(FILENAME_FADAL);
        pos = text->indexOf(expression);
        if(pos >= 0)
        {
            name1 = text->mid(pos, expression.matchedLength());
            name1.remove("N1");

            ext1.clear();
            qDebug() << "13" << name1 << ext1;
            break;
        };

        name1 = "";
        ext1 = "";
        break;
    };


    name1.remove(".");
    name1.remove(",");
    name1 = name1.simplified();
    name1 = name1.trimmed();
    ext1 = ext1.simplified();
    ext1 = ext1.trimmed();

    if(this->settings.removeLetters)
    {
        QString tmpName = name1;
        tmpName.remove(QRegExp("[a-zA-Z-.]{1,}"));
        tmpName = tmpName.simplified();
        tmpName = tmpName.trimmed();
        if(!tmpName.isEmpty()) // use name with letters if new name is empty
            name1 = tmpName;
    };

    qDebug() << "15" << name1 << ext1;


    forever // detect program name by user selected expression
    {
        expression.setPattern(this->settings.fileNameExpAs);
        //expression.setPattern("\\((O|:){0,1}[0-9]{4,4}\\.(NC|A|B){1,1}\\)");
        pos = text->indexOf(expression);
        if(pos >= 2)
        {
            name2 = text->mid(pos, expression.matchedLength());
            name2.remove(";");
            name2.remove("(");
            name2.remove(")");
            name2.remove("[");
            name2.remove("]");
            name2.remove("<");
            name2.remove(">");

            pos = name2.lastIndexOf('.');
            if(pos >= 0)
            {
                ext2 = name2.mid(pos, expression.matchedLength());
                name2.remove(expression);
                ext2.remove(" ");
            }
            else
                ext2.clear();


            break;
        };

        name2 = "";
        ext2 = "";
        break;

    };

    name2.remove("*");
    name2.remove(",");
    name2 = name2.simplified();
    name2 = name2.trimmed();
    ext2 = ext2.simplified();
    ext2 = ext2.trimmed();

    if(!ext2.isEmpty())
        name2.remove(ext2);

    if(settings.removeLetters)
    {
        QString tmpName = name2;
        tmpName.remove(QRegExp("[a-zA-Z-.]{1,}"));
        tmpName = tmpName.simplified();
        tmpName = tmpName.trimmed();
        if(!tmpName.isEmpty()) // use name with letters if new name is empty
            name2 = tmpName;
    };

    if(this->settings.fileNameLowerCase)
    {
        name1 = name1.toLower();
        ext1 = ext1.toLower();

        name2 = name2.toLower();
        ext2 = ext2.toLower();
    }
    else
    {
        name1 = name1.toUpper();
        ext1 = ext1.toUpper();

        name2 = name2.toUpper();
        ext2 = ext2.toUpper();
    };

    qDebug() << "16" << name2 << ext2;


    if(settings.fileNameProcessing == 1)
    {
        if(name1.isEmpty())
        {
            fileName = name2;
            extension = ext2;
        }
        else
        {
            fileName = name1;
            extension = ext1;
        };
    }
    else
        if(name2.isEmpty())
        {
            fileName = name1;
            extension = ext1;
        }
        else
        {
            fileName = name2;
            extension = ext2;
        };


    if(fileName.isEmpty())
    {
        QString dateTime = QDate::currentDate().toString("yyyy-MM-dd") + "_" + QTime::currentTime().toString("HH_mm_ss").remove(QRegularExpression(" \\w+"));
        fileName = dateTime + settings.saveExt;

        qDebug() << QString("***** WARNING: Could not find program name. Using date time code.");

        qDebug() << "18" << fileName;

        return fileName;
    }
    else
    {
        if(this->settings.useAsExt && (!extension.isEmpty()))
            fileName = fileName + extension;
        else
        {
            if(this->settings.appendExt)
                fileName = fileName + extension.replace('.', '_') + settings.saveExt;
            else
                fileName = fileName + settings.saveExt;
        };

        qDebug() << QString("***** File name (RAW): %1").arg(fileName) << this->settings.fileNameProcessing;

        QString fileNameMask = settings.fileNameMask;

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

                        //                    qDebug() << "File Server 0015.2" << num << j;

                        if(j < fileName.length())
                            tx.append(fileName[j]);

                    };
                };

                i++;

                //            qDebug() << "File Server 0015.5" << i;
            };

            fileName = tx;
        };


        qDebug() << QString("***** File name (mask: %3): %1").arg(fileName).arg(settings.fileNameMask);
    };

    return fileName;
}

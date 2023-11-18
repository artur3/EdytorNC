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


#ifndef FTPCONFIGDIALOG_H
#define FTPCONFIGDIALOG_H


#include <QtWidgets>

#include "ui_ftpconfigdialog.h"



struct FtpSettings
{
    QString configName;
    QString rootPath;
    QString relativePath;

    bool removeLetters;

    bool fileNameLowerCase;

    int fileNameProcessing;
    bool appendExt;
    bool useAsExt;
    bool splitPrograms;

    QString saveExt;

    QString saveDir;

    bool createLogFile;
    bool listDirs;
    bool renameIfExists;

    bool logData;

    QString listExt;

    QString searchExt;

//    QString searchPath2;
//    QString searchExt2;
//    QString searchPath3;
//    QString searchExt3;
    QString fileNameExpFs;
    QString fileNameExpAs;
//    QString eobChar;
    QString fileNameExpSaveFile;
    QString fileNameMask;
    QString uploadFileNameMask;
//    QString removeFromRecieved;
};

class FtpConfigDialog : public QDialog, private Ui::FtpConfigDialog
{
   Q_OBJECT

   public:
     FtpConfigDialog(QWidget *parent = 0, Qt::WindowFlags f = Qt::Dialog);
     ~FtpConfigDialog();

   public slots:


   protected:

   signals :



   private slots:
     void browseButtonClicked();
     void saveButtonClicked();
     void loadSettings();
     void changeSettings();
     void closeButtonClicked();
     void deleteButtonClicked();
     void saveCloseButtonClicked();
     void flowCtlGroupReleased();
     void toolButtonBrowseClicked();
     void browse2ButtonClicked();
     void browse3ButtonClicked();
     void browse4ButtonClicked();
     void portNameComboBoxIndexChanged(QString name);
//     void autoSaveCheckBoxChanged(int state);
     void appendExtCheckBoxChanged(int state);
     void useAsExtCheckBoxChanged(int state);
     void fileServerCheckBoxChanged(int state);
     void leaveAsIsCheckBoxChanged(int state);
     void addButtonClicked();
     void removeButtonClicked();
     void addMaskButtonClicked();
     void removeMaskButtonClicked();
     void addEobButtonClicked();
     void deleteEobButtonClicked();
     void addEobCharButtonClicked();
     void deleteEobCharButtonClicked();
     void eobComboBoxEditTextChanged(const QString text);
     void addFileNameButtonClicked();
     void removeFileNameButtonClicked();
     void helpButtonClicked();
     void readPath1Changed(const QString text);
     void readPath2Changed(const QString text);
     void readPath3Changed(const QString text);
     void refreshRelativePath();
     void refreshSavePath();


private:
     QString browseForDir(const QString dir, QString windowTitle);

     QString configName;

};


#endif

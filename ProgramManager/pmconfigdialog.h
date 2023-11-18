/***************************************************************************
 *   Copyright (C) 2006-2024 by Artur KozioĹ‚                               *
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


#ifndef PMCONFIGDIALOG_H
#define PMCONFIGDIALOG_H


#include <QtWidgets>

#include "ui_pmconfigdialog.h"

#include <QSqlError>
#include <QSqlQuery>

//struct FtpSettings
//{
//    QString configName;
//    QString rootPath;
//    QString relativePath;

//    bool removeLetters;

//    bool fileNameLowerCase;

//    int fileNameProcessing;
//    bool appendExt;
//    bool useAsExt;
//    bool splitPrograms;

//    QString saveExt;

//    QString saveDir;

//    bool createLogFile;
//    bool listDirs;
//    bool renameIfExists;

//    bool logData;

//    QString listExt;

//    QString searchExt;

////    QString searchPath2;
////    QString searchExt2;
////    QString searchPath3;
////    QString searchExt3;
//    QString fileNameExpFs;
//    QString fileNameExpAs;
////    QString eobChar;
//    QString fileNameExpSaveFile;
//    QString fileNameMask;
//    QString uploadFileNameMask;
////    QString removeFromRecieved;
//};

class PmConfigDialog : public QDialog, private Ui::PmConfigDialog
{
   Q_OBJECT

   public:
     PmConfigDialog(QWidget *parent = 0, QSqlDatabase *database = 0, Qt::WindowFlags f = Qt::Dialog);
     ~PmConfigDialog();

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
     void refreshRelativePath();
     void refreshSavePath();
     void browse4ButtonClicked();
     void portNameComboBoxIndexChanged(QString name);
//     void autoSaveCheckBoxChanged(int state);
     void appendExtCheckBoxChanged(int state);
     void useAsExtCheckBoxChanged(int state);
     void fileServerCheckBoxChanged(int state);
     void sqliteCheckBoxChanged(int state);
     void saveConfigToDb();
     void readMachinePathFromDb();
     void readMachinesFromDb();
     void deleteMachineFromDb();
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



private:
     QString browseForDir(const QString dir, QString windowTitle);

     QString configName;

     QSqlDatabase *db;


};


#endif

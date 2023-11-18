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
      

   
#include "pmconfigdialog.h"




PmConfigDialog::PmConfigDialog(QWidget *parent, QSqlDatabase *database, Qt::WindowFlags f) : QDialog(parent, f)
{
   setupUi(this);
   setAttribute(Qt::WA_DeleteOnClose);
   setWindowTitle(tr("Program Manager configuration"));
   setModal(true);

   configName = "";

   db = database;

   connect(toolButtonBrowse, SIGNAL(clicked()), SLOT(toolButtonBrowseClicked()));

   connect(saveButton, SIGNAL(clicked()), SLOT(saveButtonClicked()));
   connect(saveCloseButton, SIGNAL(clicked()), SLOT(saveCloseButtonClicked()));
   connect(deleteButton, SIGNAL(clicked()), SLOT(deleteButtonClicked()));
   connect(closeButton, SIGNAL(clicked()), SLOT(closeButtonClicked()));
   connect(helpButton, SIGNAL(clicked()), SLOT(helpButtonClicked()));

   loadSettings();


//   connect(configNameBox, SIGNAL(currentIndexChanged(int)), SLOT(changeSettings()));
   connect(configNameBox, SIGNAL(currentIndexChanged(int)), SLOT(readMachinePathFromDb()));

   connect(rootPathLineEdit, SIGNAL(textChanged(QString)), SLOT(refreshRelativePath()));
   connect(relativePathComboBox, SIGNAL(currentIndexChanged(int)), SLOT(refreshSavePath()));
   connect(sqliteCheckBox, SIGNAL(stateChanged(int)), SLOT(sqliteCheckBoxChanged(int)));

   if(db == 0)
       tabWidget->setCurrentIndex(2);
   else
       tabWidget->setCurrentIndex(0);


   QRegExp rx("((\\*\\.){1,1}(\\w){1,3}){1,1}((;){1,1}(\\*\\.){1,1}(\\w){1,3}){0,9}", Qt::CaseInsensitive, QRegExp::RegExp2);
   QValidator *listExtLineEditValid = new QRegExpValidator(rx, this);
   listExtLineEdit->setValidator(listExtLineEditValid);

   setResult(QDialog::Rejected);
}

//**************************************************************************************************
//
//**************************************************************************************************

PmConfigDialog::~PmConfigDialog()
{

}

//**************************************************************************************************
//
//**************************************************************************************************

void PmConfigDialog::flowCtlGroupReleased()
{


}

//**************************************************************************************************
//
//**************************************************************************************************

void PmConfigDialog::browseButtonClicked()
{


}

//**************************************************************************************************
//
//**************************************************************************************************

void PmConfigDialog::saveButtonClicked()
{
    QStringList list;
//    QString item, curItem;

    saveConfigToDb();

    QSettings settings(QSettings::IniFormat, QSettings::UserScope, "EdytorNC", "EdytorNC");

    settings.beginGroup("PMConfigs");

//    curItem = configNameBox->currentText();


    settings.setValue("RootPath", rootPathLineEdit->text());

    settings.setValue("RefreshTime1", timeEdit_1->time().toString());
    settings.setValue("RefreshTime2", timeEdit_2->time().toString());

    settings.setValue("DBType", sqliteCheckBox->isChecked());
    settings.setValue("DBHost", hostLineEdit->text());
    settings.setValue("DBUser", userLineEdit->text());
    settings.setValue("DBPass", passwordLineEdit->text());
    settings.setValue("DBPort", portSpinBox->text());


//    list.clear();
//    list.prepend(configNameBox->currentText());
//    for(int i = 0; i <= configNameBox->count(); i++)
//    {
//       item = configNameBox->itemText(i);
//       if(!item.isEmpty())
//         if(!list.contains(item))
//           list.prepend(item);
//    };

//    while(list.size() > 64)
//    {
//       list.removeLast();
//    };
//    list.sort();

//    settings.setValue("SettingsList", list);
//    settings.setValue("CurrentPMSettings", configNameBox->currentText());

//    settings.beginGroup(configNameBox->currentText());

//    settings.setValue("ListExt", listExtLineEdit->text());
//    settings.setValue("RelativePath", relativePathComboBox->currentText());
//    settings.setValue("SaveDir", saveDirComboBox->currentText());

//    settings.endGroup();

//    QStringList eList;

//    settings.endGroup();
////    settings.sync();

//    configNameBox->blockSignals(true);
//    configNameBox->clear();
//    configNameBox->addItems(list);
//    configNameBox->blockSignals(false);
//    int id = configNameBox->findText(curItem);
//    if(id >= 0)
//       configNameBox->setCurrentIndex(id);

}

//**************************************************************************************************
//
//**************************************************************************************************

void PmConfigDialog::changeSettings()
{
    QString tx;
    QStringList list;

    readMachinePathFromDb();
    return;



//    QSettings settings(QSettings::IniFormat, QSettings::UserScope, "EdytorNC", "EdytorNC");

//    settings.beginGroup("PMConfigs");

//    configName = configNameBox->currentText();

//    settings.beginGroup(configName);

//    listExtLineEdit->setText(settings.value("ListExt", "*.nc").toString());


//    relativePathComboBox->clear();
//    tx = settings.value("RelativePath", "").toString();
//    relativePathComboBox->addItem(tx);
//    relativePathComboBox->setCurrentIndex(relativePathComboBox->findText(tx));
//    refreshRelativePath();

//    saveDirComboBox->clear();
//    tx = settings.value("SaveDir", "").toString();
//    saveDirComboBox->addItem(tx);
//    saveDirComboBox->setCurrentIndex(saveDirComboBox->findText(tx));
//    refreshSavePath();


//    settings.endGroup();
//    settings.endGroup();
}

//**************************************************************************************************
//
//**************************************************************************************************

void PmConfigDialog::loadSettings()
{
    int id;
    QStringList list;

    QSettings settings(QSettings::IniFormat, QSettings::UserScope, "EdytorNC", "EdytorNC");

    settings.beginGroup("PMConfigs");

    rootPathLineEdit->setText(settings.value("RootPath", QDir::rootPath()).toString());

    timeEdit_1->setTime(settings.value("RefreshTime1", tr("00:00")).toTime());
    timeEdit_2->setTime(settings.value("RefreshTime2", tr("00:00")).toTime());

    bool ty = settings.value("DBType", true).toBool();
    sqliteCheckBox->setChecked(ty);
    mariadbCheckBox->setChecked(!ty);

    hostLineEdit->setText(settings.value("DBHost", tr("127.0.0.1")).toString());
    userLineEdit->setText(settings.value("DBUser", tr("edytornc")).toString());
    passwordLineEdit->setText(settings.value("DBPass", tr("edytornc")).toString());
    portSpinBox->setValue(settings.value("DBPort", 3306).toUInt());



//    configNameBox->clear();
//    list = settings.value("SettingsList", tr("Default")).toStringList();
//    list.sort();
//    configNameBox->addItems(list);
//    configName = settings.value("CurrentPMSettings", tr("Default")).toString();
//    id = configNameBox->findText(configName);
//    configNameBox->setCurrentIndex(id);
//    settings.endGroup();

    readMachinesFromDb();

    changeSettings();
}

//**************************************************************************************************
//
//**************************************************************************************************

void PmConfigDialog::portNameComboBoxIndexChanged(QString name)
{

}


//**************************************************************************************************
//
//**************************************************************************************************

void PmConfigDialog::deleteButtonClicked()
{
//    if(configNameBox->currentText() == tr("Default"))
//        return;

//    QSettings settings(QSettings::IniFormat, QSettings::UserScope, "EdytorNC", "EdytorNC");

//    settings.beginGroup("PMConfigs");
//    settings.remove(configNameBox->currentText());
//    settings.endGroup();

    deleteMachineFromDb();

    int id = configNameBox->findText(configNameBox->currentText());
    configNameBox->removeItem(id);
}

//**************************************************************************************************
//
//**************************************************************************************************

void PmConfigDialog::closeButtonClicked()
{
//   QSettings settings(QSettings::IniFormat, QSettings::UserScope, "EdytorNC", "EdytorNC");
//   settings.beginGroup("PMConfigs");
//   settings.setValue("CurrentPMSettings", configNameBox->currentText());
//   settings.endGroup();

   close();
}

//**************************************************************************************************
//
//**************************************************************************************************

void PmConfigDialog::saveCloseButtonClicked()
{
   saveButtonClicked();
   closeButtonClicked();
   accept();
}

//**************************************************************************************************
//
//**************************************************************************************************

QString PmConfigDialog::browseForDir(QString dir, QString windowTitle)
{
    QDir directory(dir);

    if((!directory.exists()) || (!directory.isReadable()) || (dir.isEmpty()))
        directory.setPath(QDir::homePath());

    dir = QFileDialog::getExistingDirectory(
                this,
                windowTitle,
                directory.canonicalPath(),
                QFileDialog::ShowDirsOnly);

    if(!dir.isEmpty())
    {
        directory.setPath(dir);
    };

    return directory.canonicalPath();
}

//**************************************************************************************************
//
//**************************************************************************************************

void PmConfigDialog::toolButtonBrowseClicked()
{
    QString rootPath;

    rootPath = QFileDialog::getExistingDirectory(this, QString(), rootPathLineEdit->text());

    if (rootPath.isEmpty())
    {
        return;
    }

    rootPathLineEdit->setText(rootPath);
}

//**************************************************************************************************
//
//**************************************************************************************************

void PmConfigDialog::browse2ButtonClicked()
{
    //relativePathLineEdit->setText(browseForDir(relativePathLineEdit->text(), tr("Select path")));
}

//**************************************************************************************************
//
//**************************************************************************************************

void PmConfigDialog::refreshRelativePath()
{

    QStringList list;
    QString tx;
    QDir dir;
    QPalette palette;


    relativePathComboBox->blockSignals(true);
    relativePathComboBox->setDuplicatesEnabled(false);

    dir.setPath(rootPathLineEdit->text());

    tx = relativePathComboBox->currentText();
    relativePathComboBox->clear();

    for(const QFileInfo & finfo: dir.entryInfoList(QDir::AllDirs | QDir::Writable | QDir::Readable | QDir::NoDotAndDotDot | QDir::NoSymLinks, QDir::Name | QDir::IgnoreCase))
    {
        relativePathComboBox->addItem(dir.relativeFilePath(finfo.absoluteFilePath()));
    };

    relativePathComboBox->addItem(tx);
    relativePathComboBox->setCurrentIndex(relativePathComboBox->findText(tx));

    relativePathComboBox->blockSignals(false);

}

//**************************************************************************************************
//
//**************************************************************************************************

void PmConfigDialog::refreshSavePath()
{
    QStringList list;
    QString tx;
    QDir dir;
    QPalette palette;


    saveDirComboBox->blockSignals(true);
    saveDirComboBox->setDuplicatesEnabled(false);


    dir.setPath(rootPathLineEdit->text() + "/" + relativePathComboBox->currentText());

    tx = saveDirComboBox->currentText();
    saveDirComboBox->clear();

    for(const QFileInfo & finfo: dir.entryInfoList(QDir::AllDirs | QDir::Writable | QDir::Readable | QDir::NoDotAndDotDot | QDir::NoSymLinks, QDir::Name | QDir::IgnoreCase))
    {
        saveDirComboBox->addItem(dir.relativeFilePath(finfo.absoluteFilePath()));
    };

    saveDirComboBox->addItem(tx);
    saveDirComboBox->setCurrentIndex(saveDirComboBox->findText(tx));

    saveDirComboBox->blockSignals(false);



//    dir.setPath(rootPathLineEdit->text() + "/" + relativePathComboBox->currentText()+ "/" + saveDirComboBox->currentText());

//    saveDirComboBox->setAutoFillBackground(true);
//    palette = saveDirComboBox->palette();
//    //palette.setColor(QPalette( Qt::blue ));

//    if(dir.exists())
//        saveDirComboBox->setPalette(QPalette());
//    else
//        saveDirComboBox->setPalette(QPalette( Qt::blue ));

//    qDebug() << "Exist: " << dir.exists();
}

//**************************************************************************************************
//
//**************************************************************************************************

void PmConfigDialog::sqliteCheckBoxChanged(int state)
{   

    hostLineEdit->setEnabled(state == Qt::Unchecked);
    userLineEdit->setEnabled(state == Qt::Unchecked);
    passwordLineEdit->setEnabled(state == Qt::Unchecked);
    portSpinBox->setEnabled(state == Qt::Unchecked);



//    fileNameMaskComboBox->setEnabled(state == Qt::Unchecked);
//    addFileNameButton->setEnabled(state == Qt::Unchecked);
//    removeFileNameButton->setEnabled(state == Qt::Unchecked);
//    removeMaskButton->setEnabled(state == Qt::Unchecked);
//    addMaskButton->setEnabled(state == Qt::Unchecked);
//    fileNameLowerCaseCheckBox->setEnabled(state == Qt::Unchecked);
//    removeLettersCheckBox->setEnabled(state == Qt::Unchecked);
//    fileNameExpASComboBox->setEnabled(state == Qt::Unchecked);
//    useAsExtCheckBox->setEnabled(state == Qt::Unchecked);
//    appendExtCheckBox->setEnabled(state == Qt::Unchecked);
//    saveExtComboBox->setEnabled(state == Qt::Unchecked);
}

//**************************************************************************************************
//
//**************************************************************************************************

void PmConfigDialog::browse4ButtonClicked()
{
//    searchPath3LineEdit->setText(browseForDir(searchPath3LineEdit->text(), tr("Select search path")));
}

//**************************************************************************************************
//
//**************************************************************************************************

void PmConfigDialog::appendExtCheckBoxChanged(int state)
{
//    if(state == Qt::Checked)
//        useAsExtCheckBox->setChecked(false);
}

//**************************************************************************************************
//
//**************************************************************************************************

void PmConfigDialog::useAsExtCheckBoxChanged(int state)
{
//    if(state == Qt::Checked)
//        appendExtCheckBox->setChecked(false);

}

//**************************************************************************************************
//
//**************************************************************************************************

void PmConfigDialog::fileServerCheckBoxChanged(int state)
{
   //relativePathLineEdit->setEnabled(state == Qt::Checked);
//   searchPath2LineEdit->setEnabled(state == Qt::Checked);
//   searchPath3LineEdit->setEnabled(state == Qt::Checked);
//   searchExt1ComboBox->setEnabled(state == Qt::Checked);
//   searchExt2ComboBox->setEnabled(state == Qt::Checked);
//   searchExt3ComboBox->setEnabled(state == Qt::Checked);

//   relativePathBrowsePushButton->setEnabled(state == Qt::Checked);

//   browse3PushButton->setEnabled(state == Qt::Checked);
//   browse4PushButton->setEnabled(state == Qt::Checked);
//   fileNameExpFSComboBox->setEnabled(state == Qt::Checked);
//   fileNameMaskComboBox->setEnabled(state == Qt::Checked);
//   removeButton->setEnabled(state == Qt::Checked);
//   addButton->setEnabled(state == Qt::Checked);
//   removeMaskButton->setEnabled(state == Qt::Checked);
//   addMaskButton->setEnabled(state == Qt::Checked);
//   callerProgNameLineEdit->setEnabled(state == Qt::Checked);
//   fileNameLowerCaseCheckBox->setEnabled(state == Qt::Checked);
//   flowCtlGroupReleased();
}

//**************************************************************************************************
//
//**************************************************************************************************

void PmConfigDialog::saveConfigToDb()
{
    if(db == 0)
        return;

    if(db->isOpen())
    {
        QSqlQuery query;

        QString machine = configNameBox->currentText();
        bool alreadyExist = false;

        query.prepare("SELECT * FROM config WHERE machine=:machine");
        query.bindValue(":machine", machine);

        if(!query.exec())
        {
            // select failed
        }

        alreadyExist = query.next();

        if(alreadyExist)
        {
            query.prepare("UPDATE config SET machine=:machine, rootpath=:rootpath, relativepath=:relativepath, savepath=:savepath, extensions=:extensions WHERE machine=:machine");
            query.bindValue(":machine", machine);
            query.bindValue(":rootpath", rootPathLineEdit->text());
            query.bindValue(":relativepath", relativePathComboBox->currentText());
            query.bindValue(":savepath", saveDirComboBox->currentText());
            query.bindValue(":extensions", listExtLineEdit->text());
            query.exec();


            qDebug() << "saveConfigToDb: " << "UPDATE: " << machine << query.lastError().text();
            qDebug() << query.lastQuery();
        }
        else
        {
            query.prepare("INSERT INTO config (machine, rootpath, relativepath, savepath, extensions) VALUES (:machine, :rootpath, :relativepath, :savepath, :extensions)");
            query.bindValue(":machine", machine);
            query.bindValue(":rootpath", rootPathLineEdit->text());
            query.bindValue(":relativepath", relativePathComboBox->currentText());
            query.bindValue(":savepath", saveDirComboBox->currentText());
            query.bindValue(":extensions", listExtLineEdit->text());
            query.exec();


            qDebug() << "saveConfigToDb: " << "INSERT: " << machine << query.lastError().text();
            qDebug() << query.lastQuery();
        };


        readMachinesFromDb();

    };

}

//**************************************************************************************************
//
//**************************************************************************************************

void PmConfigDialog::readMachinePathFromDb()
{
    QString tx;


    if(db == 0)
        return;

    if(db->isOpen())
    {
        QSqlQuery query;

        query.prepare(QString("SELECT relativepath FROM config WHERE machine=:machine"));
        query.bindValue(":machine", configNameBox->currentText());
        query.exec();

        while (query.next())
        {
            tx = query.value(0).toString();

            relativePathComboBox->clear();
            relativePathComboBox->addItem(tx);
            relativePathComboBox->setCurrentIndex(relativePathComboBox->findText(tx));
            refreshRelativePath();

            qDebug() << "readConfigFromDb: " << "SELECT: " << relativePathComboBox->currentText();

        };


        query.prepare(QString("SELECT savepath FROM config WHERE machine=:machine"));
        query.bindValue(":machine", configNameBox->currentText());
        query.exec();

        while (query.next())
        {
            tx = query.value(0).toString();

            saveDirComboBox->clear();
            saveDirComboBox->addItem(tx);
            saveDirComboBox->setCurrentIndex(saveDirComboBox->findText(tx));
            refreshRelativePath();

            qDebug() << "readConfigFromDb: " << "SELECT: " << saveDirComboBox->currentText();

        };


        query.prepare(QString("SELECT extensions FROM config WHERE machine=:machine"));
        query.bindValue(":machine", configNameBox->currentText());
        query.exec();

        while (query.next())
        {
            tx = query.value(0).toString();

            if(tx.isEmpty())
                tx = "*.nc";

            listExtLineEdit->setText(tx);

            qDebug() << "readConfigFromDb: " << "SELECT: " << listExtLineEdit->text();

        };


        qDebug() << "readConfigFromDb: " << "SELECT: " << query.lastError().text();

    };

}

//**************************************************************************************************
//
//**************************************************************************************************

void PmConfigDialog::readMachinesFromDb()
{
    QStringList list;

    if(db == 0)
        return;

    if(db->isOpen())
    {
        QSqlQuery query;
        QString machine = configNameBox->currentText();

        query.prepare(QString("SELECT machine FROM config"));
        query.exec();

        while (query.next())
        {
            list.append(query.value(0).toString());
        };
        list.removeDuplicates();
        list.sort(Qt::CaseInsensitive);

        configNameBox->blockSignals(true);
        configNameBox->clear();
        configNameBox->addItems(list);
        configNameBox->blockSignals(false);


        int id = configNameBox->findText(machine);
        if(id >= 0)
           configNameBox->setCurrentIndex(id);


        qDebug() << "readMachinesFromDb: " << "SELECT: " << query.lastError().text();
    };

}

//**************************************************************************************************
//
//**************************************************************************************************

void PmConfigDialog::deleteMachineFromDb()
{
    if(db == 0)
        return;

    if(db->isOpen())
    {
        QSqlQuery query;

        query.prepare(QString("DELETE FROM config WHERE machine=:machine"));
        query.bindValue(":machine", configNameBox->currentText());
        query.exec();

    };
}


//**************************************************************************************************
//
//**************************************************************************************************

void PmConfigDialog::addEobButtonClicked()
{
//    QString text = endOfProgComboBox->currentText();
//    if(!text.isEmpty())
//        endOfProgComboBox->insertItem(0, text);
}

//**************************************************************************************************
//
//**************************************************************************************************

void PmConfigDialog::deleteEobButtonClicked()
{
//    endOfProgComboBox->removeItem(endOfProgComboBox->currentIndex());
}

//**************************************************************************************************
//
//**************************************************************************************************

void PmConfigDialog::addEobCharButtonClicked()
{
//    QString text = eobComboBox->currentText();
//    if(!text.isEmpty())
//        eobComboBox->insertItem(0, text);
}

//**************************************************************************************************
//
//**************************************************************************************************

void PmConfigDialog::deleteEobCharButtonClicked()
{
//    eobComboBox->removeItem(eobComboBox->currentIndex());
}

//**************************************************************************************************
//
//**************************************************************************************************

void PmConfigDialog::eobComboBoxEditTextChanged(const QString text)
{
//    eobComboBox->setCurrentText(text.toUpper());
}

//**************************************************************************************************
//
//**************************************************************************************************

void PmConfigDialog::addFileNameButtonClicked()
{
//    QString text = fileNameExpASComboBox->currentText();
//    if(!text.isEmpty())
//        fileNameExpASComboBox->insertItem(0, text);
}

//**************************************************************************************************
//
//**************************************************************************************************

void PmConfigDialog::removeFileNameButtonClicked()
{
//    fileNameExpASComboBox->removeItem(fileNameExpASComboBox->currentIndex());
}

//**************************************************************************************************
//
//**************************************************************************************************

void PmConfigDialog::helpButtonClicked()
{
    QDesktopServices::openUrl(QUrl(QString("file:///%1/EdytorNC_SerialTransmission_Help.html").arg(QCoreApplication::applicationDirPath()), QUrl::TolerantMode));
//    SerialPortCfgHelpDialog helpDialog;
//    helpDialog.exec();
}

//**************************************************************************************************
//
//**************************************************************************************************

void PmConfigDialog::readPath1Changed(const QString text)
{
//    QPalette palette;

//    if(text.isEmpty())
//        return;

//    palette.setColor(relativePathLineEdit->foregroundRole(), Qt::red);

//    QDir dir;
//    dir.setPath(text);
//    if(dir.exists())
//      relativePathLineEdit->setPalette(QPalette());
//    else
//      relativePathLineEdit->setPalette(palette);
}

//**************************************************************************************************
//
//**************************************************************************************************

void PmConfigDialog::readPath2Changed(const QString text)
{
//    QPalette palette;

//    if(text.isEmpty())
//        return;

//    palette.setColor(searchPath2LineEdit->foregroundRole(), Qt::red);

//    QDir dir;
//    dir.setPath(text);
//    if(dir.exists())
//      searchPath2LineEdit->setPalette(QPalette());
//    else
//      searchPath2LineEdit->setPalette(palette);
}

//**************************************************************************************************
//
//**************************************************************************************************

void PmConfigDialog::readPath3Changed(const QString text)
{
//    QPalette palette;

//    if(text.isEmpty())
//        return;
//    palette.setColor(searchPath3LineEdit->foregroundRole(), Qt::red);

//    QDir dir;
//    dir.setPath(text);
//    if(dir.exists())
//      searchPath3LineEdit->setPalette(QPalette());
//    else
//      searchPath3LineEdit->setPalette(palette);
}


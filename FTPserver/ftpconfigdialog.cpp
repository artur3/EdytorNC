/***************************************************************************
 *   Copyright (C) 2006-2022 by Artur Kozioł                               *
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
      

   
#include "ftpconfigdialog.h" 



static const char blankString[] = QT_TRANSLATE_NOOP("SettingsDialog", "N/A");


FtpConfigDialog::FtpConfigDialog(QWidget *parent, Qt::WindowFlags f) : QDialog(parent, f)
{
   setupUi(this);
   setAttribute(Qt::WA_DeleteOnClose);
   setWindowTitle(tr("FTP server configuration"));
   setModal(true);

   configName = "";

   connect(toolButtonBrowse, SIGNAL(clicked()), SLOT(toolButtonBrowseClicked()));

   connect(saveButton, SIGNAL(clicked()), SLOT(saveButtonClicked()));
   connect(saveCloseButton, SIGNAL(clicked()), SLOT(saveCloseButtonClicked()));
   connect(deleteButton, SIGNAL(clicked()), SLOT(deleteButtonClicked()));
   connect(closeButton, SIGNAL(clicked()), SLOT(closeButtonClicked()));
   connect(helpButton, SIGNAL(clicked()), SLOT(helpButtonClicked()));
   connect(addMaskButton, SIGNAL(clicked()), SLOT(addMaskButtonClicked()));
   connect(removeMaskButton, SIGNAL(clicked()), SLOT(removeMaskButtonClicked()));
   connect(addFileNameButton, SIGNAL(clicked()), SLOT(addFileNameButtonClicked()));
   connect(removeFileNameButton, SIGNAL(clicked()), SLOT(removeFileNameButtonClicked()));
   connect(appendExtCheckBox, SIGNAL(stateChanged(int)), SLOT(appendExtCheckBoxChanged(int)));
   connect(useAsExtCheckBox, SIGNAL(stateChanged(int)), SLOT(useAsExtCheckBoxChanged(int)));

   connect(leaveAsIsCheckBox, SIGNAL(stateChanged(int)), SLOT(leaveAsIsCheckBoxChanged(int)));

   loadSettings();
   flowCtlGroupReleased();

   connect(configNameBox, SIGNAL(currentIndexChanged(int)), SLOT(changeSettings()));


   tabWidget->setCurrentIndex(1);

   QRegExp rx("((\\*\\.){1,1}(\\w){1,3}){1,1}((;){1,1}(\\*\\.){1,1}(\\w){1,3}){0,9}", Qt::CaseInsensitive, QRegExp::RegExp2);
   QValidator *listExtLineEditValid = new QRegExpValidator(rx, this);
   listExtLineEdit->setValidator(listExtLineEditValid);
   searchExtLineEdit->setValidator(listExtLineEditValid);

   lineEditPort->setValidator(new QIntValidator(1, 65535, this));

   setResult(QDialog::Rejected);
}

//**************************************************************************************************
//
//**************************************************************************************************

FtpConfigDialog::~FtpConfigDialog()
{

}

//**************************************************************************************************
//
//**************************************************************************************************

void FtpConfigDialog::flowCtlGroupReleased()
{


}

//**************************************************************************************************
//
//**************************************************************************************************

void FtpConfigDialog::browseButtonClicked()
{


}

//**************************************************************************************************
//
//**************************************************************************************************

void FtpConfigDialog::saveButtonClicked()
{
    QStringList list;
    QString item, curItem;

    QSettings settings("EdytorNC", "EdytorNC");

    settings.beginGroup("FTPConfigs");

    curItem = configNameBox->currentText();

    settings.setValue("Port", lineEditPort->text());
    settings.setValue("Username", lineEditUserName->text());
    settings.setValue("Password", lineEditPassword->text());
    settings.setValue("RootPath", lineEditRootPath->text());
    settings.setValue("Anonymous", checkBoxAnonymous->isChecked());
    settings.setValue("Readonly", checkBoxReadOnly->isChecked());
    settings.setValue("OneIp", checkBoxOnlyOneIpAllowed->isChecked());

    settings.setValue("AllowDelete", allowDeleteCheckBox->isChecked());
    settings.setValue("AllowMkDir", allowMkDirCheckBox->isChecked());
    settings.setValue("AllowDirChange", allowDirChangeCheckBox->isChecked());


    list.clear();
    list.prepend(configNameBox->currentText());
    for(int i = 0; i <= configNameBox->count(); i++)
    {
       item = configNameBox->itemText(i);
       if(!item.isEmpty())
         if(!list.contains(item))
           list.prepend(item);
    };

    while(list.size() > 64)
    {
       list.removeLast();
    };
    list.sort();
    settings.setValue("SettingsList", list);
    settings.setValue("CurrentFtpServerSettings", configNameBox->currentText());

    settings.beginGroup(configNameBox->currentText());
    settings.setValue("ListExt", listExtLineEdit->text());
    settings.setValue("SearchExt", searchExtLineEdit->text());
    settings.setValue("ListDirs", listDirsCheckBox->isChecked());
    settings.setValue("RelativePath", relativePathComboBox->currentText());

    settings.setValue("DataToLogFile", logDataCheckBox->isChecked());

    settings.setValue("CreateLogFile", logFileCheckBox->isChecked());
    settings.setValue("CreateBackup", renameCheckBox->isChecked());
    settings.setValue("RemoveLetters", removeLettersCheckBox->isChecked());

    int state;
    if(leaveAsIsCheckBox->isChecked())
        state = 0;
    if(detectFormFileNameCheckBox->isChecked())
        state = 1;
    if(userExpCheckBox->isChecked())
        state = 2;

    settings.setValue("FileNameProcessing", state);

    settings.setValue("AppendExtension", appendExtCheckBox->isChecked());
    settings.setValue("UseAsExtension", useAsExtCheckBox->isChecked());
    settings.setValue("SplitPrograms", splitProgramsCheckBox->isChecked());
    settings.setValue("SaveDir", saveDirComboBox->currentText());
    settings.setValue("SaveExt", saveExtComboBox->currentText());

    settings.setValue("FileNameMaskSelected", fileNameMaskComboBox->currentText());
    settings.setValue("UploadFileNameMaskSelected", uploadFileNameMaskComboBox->currentText());

    settings.setValue("FileNameExpASSelected", fileNameExpASComboBox->currentText());
    settings.setValue("FileNameLowerCase", fileNameLowerCaseCheckBox->isChecked());

    settings.endGroup();

    QStringList eList;
    eList.clear();
    eList.prepend(fileNameMaskComboBox->currentText());
    for(int i = 0; i <= fileNameMaskComboBox->count(); i++)
    {
       item = fileNameMaskComboBox->itemText(i);
       if(!item.isEmpty())
         if(!eList.contains(item))
           eList.prepend(item);
    };
    eList.removeDuplicates();
    eList.sort();
    settings.setValue("FileNameMask", eList);

    eList.clear();
    eList.prepend(uploadFileNameMaskComboBox->currentText());
    for(int i = 0; i <= uploadFileNameMaskComboBox->count(); i++)
    {
       item = uploadFileNameMaskComboBox->itemText(i);
       if(!item.isEmpty())
         if(!eList.contains(item))
           eList.prepend(item);
    };
    eList.removeDuplicates();
    eList.sort();
    settings.setValue("UploadFileNameMask", eList);

    eList.clear();
    eList.prepend(fileNameExpASComboBox->currentText());
    for(int i = 0; i <= fileNameExpASComboBox->count(); i++)
    {
       item = fileNameExpASComboBox->itemText(i);
       if(!item.isEmpty())
         if(!eList.contains(item))
           eList.prepend(item);
    };
    eList.removeDuplicates();
    eList.sort();
    settings.setValue("FileNameExpAS", eList);

    settings.endGroup();
//    settings.sync();

    configNameBox->blockSignals(true);
    configNameBox->clear();
    configNameBox->addItems(list);
    configNameBox->blockSignals(false);
    int id = configNameBox->findText(curItem);
    if(id >= 0)
       configNameBox->setCurrentIndex(id);

}

//**************************************************************************************************
//
//**************************************************************************************************

void FtpConfigDialog::changeSettings()
{

    // UNIX-derived systems such as Linux and Android don't allow access to
    // port 21 for non-root programs, so we will use port 2121 instead.
    QString defaultPort;
#if defined(Q_OS_UNIX)
    defaultPort = "2121";
#else
    defaultPort = "21";
#endif


    int id;
    QStringList list;

    QSettings settings("EdytorNC", "EdytorNC");

    QStringList extensions = settings.value("Extensions", (QStringList() << "*.nc" << "*.cnc")).toStringList();

    extensions.removeDuplicates();
    extensions.replaceInStrings("*", "");
    saveExtComboBox->clear();
    saveExtComboBox->insertItems(0, extensions);


    settings.beginGroup("FTPConfigs");

    lineEditPort->setText(settings.value("Port", defaultPort).toString());
    lineEditUserName->setText(settings.value("Username", "EDYTORNC").toString());
    lineEditPassword->setText(settings.value("Password", "CNC2022").toString());
    lineEditRootPath->setText(settings.value("RootPath", QDir::rootPath()).toString());
    checkBoxAnonymous->setChecked(settings.value("Anonymous", false).toBool());
    checkBoxReadOnly->setChecked(settings.value("Readonly", false).toBool());
    checkBoxOnlyOneIpAllowed->setChecked(settings.value("OneIp", true).toBool());

    allowDeleteCheckBox->setChecked(settings.value("AllowDelete", false).toBool());
    allowMkDirCheckBox->setChecked(settings.value("AllowMkDir", false).toBool());
    allowDirChangeCheckBox->setChecked(settings.value("AllowDirChange", false).toBool());


    list.clear();
    list.append(settings.value("FileNameExpFS", (QStringList() << "\\([0-9]{4,4}\\)" << "\\([0-9]{4,4}\\.NC\\)" << "\\((O|:){0,1}[0-9]{4,4}\\.(NC|CNC|AGC){1,1}\\)")).toStringList());
    list.removeDuplicates();

    list.clear();
    list.append(settings.value("FileNameMask", (QStringList() << "" << "O$1$2$3$4.A" << "$1$2$3$4.$6" << "$1$2$3$4$5$6$7$8")).toStringList());
    list.removeDuplicates();
    fileNameMaskComboBox->clear();
    fileNameMaskComboBox->insertItems(0, list);

    list.clear();
    list.append(settings.value("UploadFileNameMask", (QStringList() << "" << "O$1$2$3$4.A" << "$1$2$3$4.$6" << "$1$2$3$4$5$6$7$8")).toStringList());
    list.removeDuplicates();
    uploadFileNameMaskComboBox->clear();
    uploadFileNameMaskComboBox->insertItems(0, list);

    list.clear();
    list.append(settings.value("FileNameExpAS", (QStringList() << "\\([0-9]{4,4}\\)" << "\\([0-9]{4,4}\\.NC\\)" << "\\((O|:){0,1}[0-9]{4,4}\\.(NC|CNC|AGC){1,1}\\)")).toStringList());
    list.removeDuplicates();
    fileNameExpASComboBox->clear();
    fileNameExpASComboBox->insertItems(0, list);


    QDir dir;
    dir.setPath(lineEditRootPath->text());
    relativePathComboBox->clear();
    for(const QFileInfo & finfo: dir.entryInfoList(QDir::AllDirs | QDir::Writable | QDir::Readable | QDir::NoDotAndDotDot | QDir::NoSymLinks, QDir::Name | QDir::IgnoreCase))
    {
        relativePathComboBox->addItem(dir.relativeFilePath(finfo.absoluteFilePath()));
    };

    configName = configNameBox->currentText();
    settings.beginGroup(configName);

    listDirsCheckBox->setChecked(settings.value("ListDirs", true).toBool());
    listExtLineEdit->setText(settings.value("ListExt", "").toString());
    searchExtLineEdit->setText(settings.value("SearchExt", "").toString());

    logFileCheckBox->setChecked(settings.value("CreateLogFile", true).toBool());
    logDataCheckBox->setChecked(settings.value("DataToLogFile", false).toBool());

    renameCheckBox->setChecked(settings.value("CreateBackup", true).toBool());
    removeLettersCheckBox->setChecked(settings.value("RemoveLetters", true).toBool());

    int state = settings.value("FileNameProcessing", 0).toInt();
    switch (state) {
    case 0:
        leaveAsIsCheckBox->setChecked(true);
        detectFormFileNameCheckBox->setChecked(false);
        userExpCheckBox->setChecked(false);
        break;
    case 1:
        leaveAsIsCheckBox->setChecked(false);
        detectFormFileNameCheckBox->setChecked(true);
        userExpCheckBox->setChecked(false);
        break;
    case 2:
        leaveAsIsCheckBox->setChecked(false);
        detectFormFileNameCheckBox->setChecked(false);
        userExpCheckBox->setChecked(true);
        break;
    default:
        break;
    };

    appendExtCheckBox->setChecked(settings.value("AppendExtension", false).toBool());
    useAsExtCheckBox->setChecked(settings.value("UseAsExtension", false).toBool());
    splitProgramsCheckBox->setChecked(settings.value("SplitPrograms", true).toBool());

    id = saveExtComboBox->findText(settings.value("SaveExt", ".nc").toString());
    if(id >= 0)
       saveExtComboBox->setCurrentIndex(id);

    fileNameLowerCaseCheckBox->setChecked(settings.value("FileNameLowerCase", true).toBool());

    id = fileNameMaskComboBox->findText(settings.value("FileNameMaskSelected", "").toString());
    if(id >= 0)
       fileNameMaskComboBox->setCurrentIndex(id);

    id = uploadFileNameMaskComboBox->findText(settings.value("UploadFileNameMaskSelected", "").toString());
    if(id >= 0)
       uploadFileNameMaskComboBox->setCurrentIndex(id);

    id = fileNameExpASComboBox->findText(settings.value("FileNameExpASSelected", "").toString());
    if(id >= 0)
       fileNameExpASComboBox->setCurrentIndex(id);


    id = relativePathComboBox->findText(settings.value("RelativePath", "").toString());
    if(id >= 0)
       relativePathComboBox->setCurrentIndex(id);

    dir.setPath(lineEditRootPath->text() + "/" + relativePathComboBox->currentText());
    saveDirComboBox->clear();
    for(const QFileInfo & finfo: dir.entryInfoList(QDir::AllDirs | QDir::Writable | QDir::Readable | QDir::NoDotAndDotDot | QDir::NoSymLinks, QDir::Name | QDir::IgnoreCase))
    {
        saveDirComboBox->addItem(dir.relativeFilePath(finfo.absoluteFilePath()));
    };
    saveDirComboBox->addItem("");
    id = saveDirComboBox->findText(settings.value("SaveDir", "").toString());
    if(id >= 0)
        saveDirComboBox->setCurrentIndex(id);


    settings.endGroup();
    settings.endGroup();
}

//**************************************************************************************************
//
//**************************************************************************************************

void FtpConfigDialog::loadSettings()
{
    int id;
    QStringList list;

    QSettings settings("EdytorNC", "EdytorNC");

    settings.beginGroup("FTPConfigs");



    configNameBox->clear();
    list = settings.value("SettingsList", tr("Default")).toStringList();
    list.sort();
    configNameBox->addItems(list);
    configName = settings.value("CurrentFtpServerSettings", tr("Default")).toString();
    id = configNameBox->findText(configName);
    configNameBox->setCurrentIndex(id);
    settings.endGroup();

    changeSettings();
}

//**************************************************************************************************
//
//**************************************************************************************************

void FtpConfigDialog::portNameComboBoxIndexChanged(QString name)
{

}


//**************************************************************************************************
//
//**************************************************************************************************

void FtpConfigDialog::deleteButtonClicked()
{
    if(configNameBox->currentText() == tr("Default"))
        return;

    QSettings settings("EdytorNC", "EdytorNC");

    settings.beginGroup("FTPConfigs");
    settings.remove(configNameBox->currentText());
    settings.endGroup();

    int id = configNameBox->findText(configNameBox->currentText());
    configNameBox->removeItem(id);
}

//**************************************************************************************************
//
//**************************************************************************************************

void FtpConfigDialog::closeButtonClicked()
{
   QSettings settings("EdytorNC", "EdytorNC");
   settings.beginGroup("FTPConfigs");
   settings.setValue("CurrentSerialPortSettings", configNameBox->currentText());
   settings.endGroup();

   close();
}

//**************************************************************************************************
//
//**************************************************************************************************

void FtpConfigDialog::saveCloseButtonClicked()
{
   saveButtonClicked();
   closeButtonClicked();
   accept();
}

//**************************************************************************************************
//
//**************************************************************************************************

QString FtpConfigDialog::browseForDir(QString dir, QString windowTitle)
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

void FtpConfigDialog::toolButtonBrowseClicked()
{
    QString rootPath;

    rootPath = QFileDialog::getExistingDirectory(this, QString(), lineEditRootPath->text());

    if (rootPath.isEmpty())
    {
        return;
    }

    lineEditRootPath->setText(rootPath);
}

//**************************************************************************************************
//
//**************************************************************************************************

void FtpConfigDialog::browse2ButtonClicked()
{
    //relativePathLineEdit->setText(browseForDir(relativePathLineEdit->text(), tr("Select path")));
}

//**************************************************************************************************
//
//**************************************************************************************************

void FtpConfigDialog::browse3ButtonClicked()
{

}

//**************************************************************************************************
//
//**************************************************************************************************

void FtpConfigDialog::leaveAsIsCheckBoxChanged(int state)
{   
    fileNameMaskComboBox->setEnabled(state == Qt::Unchecked);
    addFileNameButton->setEnabled(state == Qt::Unchecked);
    removeFileNameButton->setEnabled(state == Qt::Unchecked);
    removeMaskButton->setEnabled(state == Qt::Unchecked);
    addMaskButton->setEnabled(state == Qt::Unchecked);
    fileNameLowerCaseCheckBox->setEnabled(state == Qt::Unchecked);
    removeLettersCheckBox->setEnabled(state == Qt::Unchecked);
    fileNameExpASComboBox->setEnabled(state == Qt::Unchecked);
    useAsExtCheckBox->setEnabled(state == Qt::Unchecked);
    appendExtCheckBox->setEnabled(state == Qt::Unchecked);
    saveExtComboBox->setEnabled(state == Qt::Unchecked);
}

//**************************************************************************************************
//
//**************************************************************************************************

void FtpConfigDialog::browse4ButtonClicked()
{
//    searchPath3LineEdit->setText(browseForDir(searchPath3LineEdit->text(), tr("Select search path")));
}

//**************************************************************************************************
//
//**************************************************************************************************

void FtpConfigDialog::appendExtCheckBoxChanged(int state)
{
    if(state == Qt::Checked)
        useAsExtCheckBox->setChecked(false);
}

//**************************************************************************************************
//
//**************************************************************************************************

void FtpConfigDialog::useAsExtCheckBoxChanged(int state)
{
    if(state == Qt::Checked)
        appendExtCheckBox->setChecked(false);

}

//**************************************************************************************************
//
//**************************************************************************************************

void FtpConfigDialog::fileServerCheckBoxChanged(int state)
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

void FtpConfigDialog::addButtonClicked()
{
//    QString text = fileNameExpFSComboBox->currentText();
//    if(!text.isEmpty())
//        fileNameExpFSComboBox->insertItem(0, text);
}

//**************************************************************************************************
//
//**************************************************************************************************

void FtpConfigDialog::removeButtonClicked()
{
//    fileNameExpFSComboBox->removeItem(fileNameExpFSComboBox->currentIndex());
}

//**************************************************************************************************
//
//**************************************************************************************************

void FtpConfigDialog::addMaskButtonClicked()
{
    QString text = fileNameMaskComboBox->currentText();
    if(!text.isEmpty())
        fileNameMaskComboBox->insertItem(0, text);
}

//**************************************************************************************************
//
//**************************************************************************************************

void FtpConfigDialog::removeMaskButtonClicked()
{
    fileNameMaskComboBox->removeItem(fileNameMaskComboBox->currentIndex());
}


//**************************************************************************************************
//
//**************************************************************************************************

void FtpConfigDialog::addEobButtonClicked()
{
//    QString text = endOfProgComboBox->currentText();
//    if(!text.isEmpty())
//        endOfProgComboBox->insertItem(0, text);
}

//**************************************************************************************************
//
//**************************************************************************************************

void FtpConfigDialog::deleteEobButtonClicked()
{
//    endOfProgComboBox->removeItem(endOfProgComboBox->currentIndex());
}

//**************************************************************************************************
//
//**************************************************************************************************

void FtpConfigDialog::addEobCharButtonClicked()
{
//    QString text = eobComboBox->currentText();
//    if(!text.isEmpty())
//        eobComboBox->insertItem(0, text);
}

//**************************************************************************************************
//
//**************************************************************************************************

void FtpConfigDialog::deleteEobCharButtonClicked()
{
//    eobComboBox->removeItem(eobComboBox->currentIndex());
}

//**************************************************************************************************
//
//**************************************************************************************************

void FtpConfigDialog::eobComboBoxEditTextChanged(const QString text)
{
//    eobComboBox->setCurrentText(text.toUpper());
}

//**************************************************************************************************
//
//**************************************************************************************************

void FtpConfigDialog::addFileNameButtonClicked()
{
    QString text = fileNameExpASComboBox->currentText();
    if(!text.isEmpty())
        fileNameExpASComboBox->insertItem(0, text);
}

//**************************************************************************************************
//
//**************************************************************************************************

void FtpConfigDialog::removeFileNameButtonClicked()
{
    fileNameExpASComboBox->removeItem(fileNameExpASComboBox->currentIndex());
}

//**************************************************************************************************
//
//**************************************************************************************************

void FtpConfigDialog::helpButtonClicked()
{
    QDesktopServices::openUrl(QUrl(QString("file:///%1/EdytorNC_SerialTransmission_Help.html").arg(QCoreApplication::applicationDirPath()), QUrl::TolerantMode));
//    SerialPortCfgHelpDialog helpDialog;
//    helpDialog.exec();
}

//**************************************************************************************************
//
//**************************************************************************************************

void FtpConfigDialog::readPath1Changed(const QString text)
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

void FtpConfigDialog::readPath2Changed(const QString text)
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

void FtpConfigDialog::readPath3Changed(const QString text)
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


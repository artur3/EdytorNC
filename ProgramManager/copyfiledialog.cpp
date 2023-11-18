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


#include "copyfiledialog.h"
#include "ui_copyfiledialog.h"

#include <QDebug>
//#include <qsettings.h>
#include <QFileInfo>
#include <QLineEdit>
#include <QSpacerItem>
#include <QSqlError>
#include <QSqlQuery>
#include <QDir>
#include <QToolTip>
#include <QFile>
#include <QPushButton>






#define ROWOFSSET 3


copyFileDialog::copyFileDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::copyFileDialog)
{
    ui->setupUi(this);

    connect(ui->buttonBox, SIGNAL(accepted()), SLOT(copyFiles()));

    connect(ui->copyToComboBox, SIGNAL(currentIndexChanged(QString)), SLOT(findLastFileNumber()));

}

//**************************************************************************************************
//
//**************************************************************************************************

copyFileDialog::~copyFileDialog()
{
    delete ui;
}

//**************************************************************************************************
//
//**************************************************************************************************

void copyFileDialog::loadConfignames()
{
//    QStringList list;

//    QSettings settings(QSettings::IniFormat, QSettings::UserScope, "EdytorNC", "EdytorNC");

//    settings.beginGroup("PMConfigs");

//    list = settings.value("SettingsList", QStringList(tr(""))).toStringList();


//    list.removeDuplicates();
//    list.sort();

////    qDebug() << "list: " << list;

//    ui->machineComboBox->clear();
//    ui->machineComboBox->addItems(list);

//    settings.endGroup();

}

//**************************************************************************************************
//
//**************************************************************************************************

void copyFileDialog::setDatabase(QSqlDatabase *database)
{ 
    db = database;
}


//**************************************************************************************************
//
//**************************************************************************************************

void copyFileDialog::setSource(QString mch, QString fileList)
{
    ui->copyFromComboBox->addItem(mch);
    ui->copyFromComboBox->setCurrentIndex(0);

    sourceFiles = fileList.split(" ", QString::SkipEmptyParts, Qt::CaseInsensitive);

    uint i = 0;

    for(const auto& it : sourceFiles)
    {
        QString fileName = QFileInfo(QString(it).remove("\"")).fileName();

        QLabel *sLabel = new QLabel(this);
        sLabel->setObjectName(QString("sLabel_%1").arg(i));
        sLabel->setText(fileName);


        QLabel *tLabel = new QLabel(this);
        tLabel->setText(tr("Copy file: "));

        ui->srcGridLayout->addWidget(tLabel, i + ROWOFSSET, 0);
        ui->srcGridLayout->addWidget(sLabel, i + ROWOFSSET, 1);



        QLineEdit *dLineEdit = new QLineEdit(this);
        dLineEdit->setObjectName(QString("dLineEdit_%1").arg(i));
        dLineEdit->setPlaceholderText(fileName);

        connect(dLineEdit, SIGNAL(textChanged(QString)), SLOT(dstLineEditChanged()));


        QLabel *dTLabel = new QLabel(this);
        dTLabel->setText(tr("to: ").arg(i));

        ui->dstGridLayout->addWidget(dTLabel, i + ROWOFSSET, 0);
        ui->dstGridLayout->addWidget(dLineEdit, i + ROWOFSSET, 1);


        int start = 0;
        int len = 0;
        int num;

        num = isFileNameNumeric(fileName, start, len);

        qDebug() << "setSource: " << mch << fileName << num << start << len;


        readMachinesFromDb();
        findLastFileNumber();

        i++;
    };


//    ui->machineComboBox->setCurrentIndex(ui->machineComboBox->findText(c));

}

//**************************************************************************************************
//
//**************************************************************************************************

void copyFileDialog::findLastFileNumber()
{
    int start = 0;
    int len = 0;
    int num, prevNum = 0, startNum = 0, maxNum;
    QString nextName;
    QSqlQuery query;


    for(int i = 0; i < 8; i++)
    {

        start = 0;
        len = 0;

        QLabel *srcLabel = findChild<QLabel *>(QString("sLabel_%1").arg(i));
        QLineEdit *dsLineEdit = findChild<QLineEdit *>(QString("dLineEdit_%1").arg(i));

        if((srcLabel == 0) || (dsLineEdit == 0))
            break ;


        num = isFileNameNumeric(srcLabel->text(), start, len);

        maxNum = num;
        startNum = 1;  // startNum = num;

        if(db->isOpen() && (num > 0))
        {

            do
            {
                maxNum = maxNum + 100;
                prevNum = num;

                query.prepare(QString("SELECT MAX(cast(SUBSTR(fileName, %1, %2) as integer)) FROM programs WHERE machine=:machine AND cast(SUBSTR(fileName, %1, %2) as integer) BETWEEN %3 AND %4").arg(start + 1).arg(len).arg(startNum).arg(maxNum));
                query.bindValue(":machine", ui->copyToComboBox->currentText());
                query.exec();

                while (query.next())
                {
                    num = query.value(0).toString().toInt();
                };

                startNum = maxNum;



                qDebug() << "findLastFileNumber: " << "SELECT: " << num;
                qDebug() << "findLastFileNumber: " << "SELECT error: " << query.lastError().text();
                qDebug() << "findLastFileNumber: " << "SELECT query: " << query.lastQuery();

            }while(num > 0);

            prevNum++;
            prevNum = prevNum + i;
            nextName = QString::number(prevNum);

            while(nextName.length() < len)
                nextName.prepend('0');

            nextName = srcLabel->text().replace(start, len, nextName);

            dsLineEdit->setText(nextName);


        };

        qDebug() << "setSource: " << ui->copyToComboBox->currentText() << srcLabel->text() << num << prevNum << maxNum;


    };



}

//**************************************************************************************************
//
//**************************************************************************************************

QString copyFileDialog::readDstPathFromDb()
{
    QString relativePath, rootPath;


    if(db == 0)
        return "";

    if(db->isOpen())
    {
        QSqlQuery query;

        query.prepare(QString("SELECT rootpath FROM config WHERE machine=:machine"));
        query.bindValue(":machine", ui->copyToComboBox->currentText());
        query.exec();

        while (query.next())
        {
            rootPath = query.value(0).toString();
        };


        query.prepare(QString("SELECT relativepath FROM config WHERE machine=:machine"));
        query.bindValue(":machine", ui->copyToComboBox->currentText());
        query.exec();

        while (query.next())
        {
            relativePath = query.value(0).toString();
        };

        return QDir(rootPath + '/' + relativePath).canonicalPath();

        qDebug() << "readDstPathFromDb: " << "rootPath " << relativePath;
        qDebug() << "readDstPathFromDb: " << "SELECT: " << query.lastError().text();

    };

    return "";

}

//**************************************************************************************************
//
//**************************************************************************************************

QString copyFileDialog::readSrcPathFromDb()
{
    QString relativePath, rootPath;


    if(db == 0)
        return "";

    if(db->isOpen())
    {
        QSqlQuery query;

        query.prepare(QString("SELECT rootpath FROM config WHERE machine=:machine"));
        query.bindValue(":machine", ui->copyFromComboBox->currentText());
        query.exec();

        while (query.next())
        {
            rootPath = query.value(0).toString();
        };


        query.prepare(QString("SELECT relativepath FROM config WHERE machine=:machine"));
        query.bindValue(":machine", ui->copyFromComboBox->currentText());
        query.exec();

        while (query.next())
        {
            relativePath = query.value(0).toString();
        };

        return QDir(rootPath + '/' + relativePath).canonicalPath();

        qDebug() << "readSrcPathFromDb: " << "rootPath " << relativePath;
        qDebug() << "readSrcPathFromDb: " << "SELECT: " << query.lastError().text();

    };

    return "";

}

//**************************************************************************************************
//
//**************************************************************************************************

void copyFileDialog::readMachinesFromDb()
{
    QStringList list;

    if(db == 0)
        return;

    if(db->isOpen())
    {
        QSqlQuery query;
        QString machine = ui->copyFromComboBox->currentText();

        query.prepare(QString("SELECT machine FROM config"));
        query.exec();

        while (query.next())
        {
            list.append(query.value(0).toString());
        };
        list.removeDuplicates();
        list.sort(Qt::CaseInsensitive);

        ui->copyToComboBox->blockSignals(true);
        ui->copyToComboBox->clear();
        ui->copyToComboBox->addItems(list);
        ui->copyToComboBox->blockSignals(false);


        int id = ui->copyToComboBox->findText(machine);
        if(id >= 0)
           ui->copyToComboBox->setCurrentIndex(id);


        qDebug() << "readMachinesFromDb: " << "SELECT: " << query.lastError().text();
    };

}

//**************************************************************************************************
//
//**************************************************************************************************

int copyFileDialog::isFileNameNumeric(QString fileName, int &numStart, int &numLen)
{
    QString tx;
    QRegExp exp;

    exp.setPattern("[0-9]{4,10}");

    int pos = fileName.indexOf(exp, 0);

    if(pos >= 0)
    {
        numStart = pos;
        numLen = exp.matchedLength();
        tx = fileName.mid(pos, exp.matchedLength());
    };


    qDebug() << "isFileNameNumeric: " << fileName << tx;

    if(!tx.isEmpty())
        return tx.toInt();
    else
        return -1;
}

//**************************************************************************************************
//
//**************************************************************************************************

void copyFileDialog::dstLineEditChanged()
{
    QFileInfo file;
    QPalette paletteRed;

    QString path = readDstPathFromDb();


    QLineEdit *dsLineEdit = qobject_cast<QLineEdit *>(sender());

    if(dsLineEdit == 0)
        return ;

    paletteRed.setColor(dsLineEdit->backgroundRole(), QColor(255, 138, 101));

    file.setFile(QDir(path), dsLineEdit->text());


    QPoint point = dsLineEdit->mapToGlobal(QPoint(5, 5));

    if(file.exists())
    {
        dsLineEdit->setPalette(paletteRed);
        dsLineEdit->setToolTip(tr("File already exist"));
        QToolTip::showText(point, tr("File already exist"));

        ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(false);

    }
    else
    {
        dsLineEdit->setPalette(QPalette());
        QToolTip::hideText();
        dsLineEdit->setToolTip(tr(""));

        ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(true);
    };


    qDebug() << "dstLineEditChanged: " << file.fileName() << file.filePath();


}

//**************************************************************************************************
//
//**************************************************************************************************

void copyFileDialog::copyFiles()
{
    QString srcPath = readSrcPathFromDb();
    QString dstPath = readDstPathFromDb();

    QFile file;

    for(int i = 0; i < 8; i++)
    {

        QLabel *srcLabel = findChild<QLabel *>(QString("sLabel_%1").arg(i));
        QLineEdit *dsLineEdit = findChild<QLineEdit *>(QString("dLineEdit_%1").arg(i));

        if((srcLabel == 0) || (dsLineEdit == 0))
            break ;

//        if(QFileInfo(QDir(dstPath), dsLineEdit->text()).exists())
//        {
//           setResult(QDialog::Rejected);
//        };

        file.setFileName(QFileInfo(QDir(srcPath), srcLabel->text()).absoluteFilePath());
        file.copy(QFileInfo(QDir(dstPath), dsLineEdit->text()).absoluteFilePath());



        qDebug() << "copyFiles: " << file.fileName() << file.errorString();
       };
}

//**************************************************************************************************
//
//**************************************************************************************************


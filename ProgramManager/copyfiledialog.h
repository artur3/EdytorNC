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


#ifndef COPYFILEDIALOG_H
#define COPYFILEDIALOG_H

#include <QDialog>
#include <QSqlDatabase>


namespace Ui {
class copyFileDialog;
}

class copyFileDialog : public QDialog
{
    Q_OBJECT

public:
    explicit copyFileDialog(QWidget *parent = nullptr);
    ~copyFileDialog();

    void setSource(QString mch, QString fileList);
    void setDatabase(QSqlDatabase *database = 0);

private slots:
      void findLastFileNumber();

      void dstLineEditChanged();
      void copyFiles();
private:
    Ui::copyFileDialog *ui;



    QStringList sourceFiles;
    QSqlDatabase *db;

    void readMachinesFromDb();
    QString readDstPathFromDb();
    int isFileNameNumeric(QString fileName, int &numStart, int &numLen);

    void loadConfignames();

    QString readSrcPathFromDb();
};

#endif // COPYFILEDIALOG_H

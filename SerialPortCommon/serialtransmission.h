/***************************************************************************
 *   Copyright (C) 2006-2019 by Artur Kozioł                               *
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

#ifndef SERIALTRANSMISSION_H
#define SERIALTRANSMISSION_H

#include <QtWidgets>
//#include <QtSerialPort/QSerialPort>

#include "serialportconfigdialog.h"
#include "../SerialPortCommon/ui_transmissiondialog.h"





/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
=========================================================================================

=========================================================================================
+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/

class TransmissionDialog : public QDialog, private Ui::TransmissionDialog
{
   Q_OBJECT

   public:
     TransmissionDialog(QWidget *parent = 0, Qt::WindowFlags f = Qt::Dialog);
     ~TransmissionDialog();

   public slots:


   protected:

   signals :



   private slots:
     void closeButtonClicked();
     void clearButtonClicked();
     void connectButtonToggled(bool tg);
     void configButtonClicked();
     void changeSettings();
     void updateLeds();
     void setRtsButtonClicked();
     void setDtrButtonClicked();
     void sendTextEditChanged();
     void lineDelaySlot();
     void setXonButtonClicked();
     void setXoffButtonClicked();
     void textEditScroll(int pos);
     void hexTextEditScroll(int pos);



   private:
     void loadSerialConfignames();
     void showError(int error);
     void sendText(QString tx);


     bool stop;
     QString portName;
     QString sendAtEnd;
     QString sendAtBegining;

     QSerialPort *comPort;
     QTimer *timer;
     long int count;
     double lineDelay;
     bool readyCont;
     SerialPortSettings portSettings;

};



#endif // SERIALTRANSMISSION_H

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


#ifndef SERIALTRANSMISSIONDIALOG_H
#define SERIALTRANSMISSIONDIALOG_H


#include <QtWidgets>
#include <QtSerialPort/QSerialPort>

#include "serialtransmission.h"
#include "../EdytorNC/commoninc.h"

#include "../SerialPortCommon/ui_serialtransmissiondialog.h"

class SerialTransmissionDialog : public QDialog, private Ui::SerialTransmissionDialog
{
    Q_OBJECT

public:
    SerialTransmissionDialog(QWidget *parent = 0, Qt::WindowFlags f = Qt::Dialog, bool mode = false);
    ~SerialTransmissionDialog();
    QStringList receiveData(QString configName);
    void sendData(QString dataToSend, QString configName);
    void startFileServer(QString configName);
    QString configName();
    QString savePath();
    QStringList readPaths();

public slots:
    bool wasCanceled();
    void portReset();


protected:
    void closeEvent(QCloseEvent *event);

signals :



private slots:
    void cancelButtonClicked();
    void updateStatus();
    void sendStartDelayTimeout();
    void stopButtonClicked();
    void autoCloseTimerTimeout();
    void showSerialPortError(QSerialPort::SerialPortError error);
    void serialPortBytesWritten(qint64 bytes);
    void serialPortReadyRead();
    void serialPortRequestToSend(bool set);
    void lineDelaySlot();
    void fileServerProcessData();
    void fileServerBytesWritten(qint64 bytes);
    void sendTimeoutTimerTimeout();
    void receiveTimeoutTimerTimeout();
    void reset(bool re);
    void reconnectTimerTimeout();

private:
    void setRange(int min, int max);
    void setValue(int val);
    void setLabelText(const QString text, bool multiline = false, bool writeToLog = false);
    void loadConfig(QString configName);
    QStringList guessFileName(QString *text);
    QString saveDataToFile(QString *text);
    void writeLog(QString msg, QString timeStamp = "");
    QStringList splitFile(QString *text);
    QStringList processReceivedData();
    void prepareDataBeforeSending(QString *data);
    void procesSpecialCharacters(QString *text, QString *fileData);
    void resetTransmission(bool portRestart = false);


    QSerialPort serialPort;
    bool canceled;
    SerialPortSettings portSettings;
    QByteArray serialPortReadBuffer;
    QStringList serialPortWriteBuffer;
    qint64 bytesWritten;
    qint64 noOfBytes;
    QStringList::iterator writeBufferIterator;
    bool xoffReceived;
    bool prevXoffReceived;
    unsigned int autoCloseCountner;
    unsigned int sendStartDelayCountner;
    //int autoCloseCountnerReloadValue;
//    int fileServerDataTimeoutCountner;
//    int fileServerDataTimeoutCountnerReloadValue;
    unsigned int sendTimeoutCountner;
    //int sendTimeoutCountnerReloadValue;
    unsigned int receiveTimeoutCountner;
    //int receiveTimeoutCountnerReloadValue;
    bool stop;
    bool serverMode;
    bool sending;
    QTimer *sendStartDelayTimer;
    QTimer *updateStatusTimer;
    QTimer *autoCloseTimer;
    //QTimer *fileServerDataTimeoutTimer;
    QTimer *sendTimeoutTimer;
    QTimer *receiveTimeoutTimer;
    QTimer *reconnectTimer;

    uint8_t EIAISOCodes[2][128] =
    {
     // { 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f, 0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f, 0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28, 0x29, 0x2a, 0x2b, 0x2c, 0x2d, 0x2e, 0x2f, 0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x3a, 0x3b, 0x3c, 0x3d, 0x3e, 0x3f, 0x40, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48, 0x49, 0x4a, 0x4b, 0x4c, 0x4d, 0x4e, 0x4f, 0x50, 0x51, 0x52, 0x53, 0x54, 0x55, 0x56, 0x57, 0x58, 0x59, 0x5a, 0x5b, 0x5c, 0x5d, 0x5e, 0x5f, 0x60, 0x61, 0x62, 0x63, 0x64, 0x65, 0x66, 0x67, 0x68, 0x69, 0x6a, 0x6b, 0x6c, 0x6d, 0x6e, 0x6f, 0x70, 0x71, 0x72, 0x73, 0x74, 0x75, 0x76, 0x77, 0x78, 0x79, 0x7a, 0x7b, 0x7c, 0x7d, 0x7e, 0x7f } // ASCII
        { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x2a, 0x33, 0x10, 0x00, 0x00, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x10, 0x00, 0x00, 0x00, 0x80, 0x5b, 0x0e, 0x00, 0x7c, 0x71, 0x00, 0x70, 0x3b, 0x40, 0x6b, 0x31, 0x20, 0x01, 0x02, 0x13, 0x04, 0x15, 0x16, 0x07, 0x08, 0x19, 0x10, 0x00, 0x00, 0x0b, 0x00, 0x00, 0x6d, 0x61, 0x62, 0x73, 0x64, 0x75, 0x76, 0x67, 0x68, 0x79, 0x51, 0x52, 0x43, 0x54, 0x45, 0x46, 0x57, 0x58, 0x49, 0x32, 0x23, 0x34, 0x00, 0x26, 0x37, 0x38, 0x29, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }, // EIA
        { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x88, 0x09, 0x0a, 0x00, 0x00, 0x8d, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xa0, 0x00, 0x00, 0x00, 0x24, 0xa5, 0xa6, 0x00, 0x28, 0xa9, 0x00, 0x2b, 0x00, 0x2d, 0x2e, 0xaf, 0x30, 0xb1, 0xb2, 0x33, 0xb4, 0x35, 0x36, 0xb7, 0xb8, 0x39, 0x3a, 0x00, 0x00, 0xbd, 0x00, 0x00, 0xc0, 0x41, 0x42, 0xc3, 0x44, 0xc5, 0xc6, 0x47, 0x48, 0xc9, 0xca, 0x4b, 0xcc, 0x4d, 0x4e, 0xcf, 0x50, 0xd1, 0xd2, 0x53, 0xd4, 0x55, 0x00, 0xd7, 0xd8, 0x59, 0x5a, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 } // ISO
    };

};

#endif // SERIALTRANSMISSIONDIALOG_H

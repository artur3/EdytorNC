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



#include "serialtransmissiondialog.h"
#include <QDebug>


SerialTransmissionDialog::SerialTransmissionDialog(QWidget *parent, Qt::WindowFlags f, bool mode) : QDialog(parent, f)
{
   setupUi(this);


   serverMode = mode;
   canceled = false;
   sending = false;
   xoffReceived = false;
   prevXoffReceived = false;
   autoCloseCountner = 15;
   sendStartDelayCountner = 0;
   sendTimeoutCountner = 2;
   receiveTimeoutCountner = 2;

   sendTimeoutTimer = new QTimer(this);
   sendTimeoutTimer->setInterval(1000);
   sendTimeoutTimer->stop();
   connect(sendTimeoutTimer, SIGNAL(timeout()), this, SLOT(sendTimeoutTimerTimeout()));

   receiveTimeoutTimer = new QTimer(this);
   receiveTimeoutTimer->setInterval(1000);
   receiveTimeoutTimer->stop();
   connect(receiveTimeoutTimer, SIGNAL(timeout()), this, SLOT(receiveTimeoutTimerTimeout()));

   sendStartDelayTimer = new QTimer(this);
   sendStartDelayTimer->setInterval(1000);
   sendStartDelayTimer->stop();
   connect(sendStartDelayTimer, SIGNAL(timeout()), this, SLOT(sendStartDelayTimeout()));

   reconnectTimer = new QTimer(this);
   reconnectTimer->setInterval(60 * 1000);
   reconnectTimer->stop();
   connect(reconnectTimer, SIGNAL(timeout()), this, SLOT(reconnectTimerTimeout()));


   if(serverMode)
   {
       setModal(false);
       setAttribute(Qt::WA_DeleteOnClose);
       cancelButton->hide();
       bottomLine->hide();
       setWindowIcon(QIcon(":/images/spserver.png"));

       plainTextEdit->setCenterOnScroll(false);
   }
   else
   {
       setWindowTitle(tr("Serial transmission"));
       setModal(true);
       connect(cancelButton, SIGNAL(clicked()), SLOT(cancelButtonClicked()));
       delete(plainTextEdit);
       adjustSize();

   };

   connect(this, SIGNAL(rejected()), SLOT(cancelButtonClicked()));

   updateStatusTimer = new QTimer(this);
   connect(updateStatusTimer, SIGNAL(timeout()), this, SLOT(updateStatus()));

   autoCloseTimer = new QTimer(this);
   autoCloseTimer->setInterval(1000);
   autoCloseTimer->stop();
   connect(autoCloseTimer, SIGNAL(timeout()), this, SLOT(autoCloseTimerTimeout()));

   updateStatusTimer->start(80);

}

//**************************************************************************************************
//
//**************************************************************************************************

SerialTransmissionDialog::~SerialTransmissionDialog()
{

}

//**************************************************************************************************
//
//**************************************************************************************************

void SerialTransmissionDialog::closeEvent(QCloseEvent *event)
{
   updateStatusTimer->stop();
   autoCloseTimer->stop();
   canceled = true;

   if(serialPort.isOpen())
   {
       //serialPort.clearError();
       serialPort.close();
   };
   event->accept();
}

//**************************************************************************************************
//
//**************************************************************************************************

void SerialTransmissionDialog::autoCloseTimerTimeout()
{
    if(autoCloseCountner == 0)
    {
        autoCloseTimer->stop();
        return;
    };

    if(autoCloseCountner <= 1)
    {
        autoCloseCountner = 0;
        cancelButtonClicked();
    }
    else
    {
        if(!xoffReceived)
            autoCloseCountner--;
    };

    if(autoCloseCountner <= (portSettings.autoCloseTimeout - 2))
    {
        cancelButton->setText(tr("Auto &closing in %1s").arg(autoCloseCountner));
    }
    else
       cancelButton->setText(tr("&Close"));
}

//**************************************************************************************************
//
//**************************************************************************************************

void SerialTransmissionDialog::cancelButtonClicked()
{
   updateStatusTimer->stop();
   autoCloseTimer->stop();
   canceled = true;
   close();
}

//**************************************************************************************************
//
//**************************************************************************************************

bool SerialTransmissionDialog::wasCanceled()
{
   return canceled;
}

//**************************************************************************************************
//
//**************************************************************************************************

void SerialTransmissionDialog::setLabelText(const QString text, bool multiline, bool writeToLog)
{
    QString timeStamp = QDate::currentDate().toString(Qt::DefaultLocaleShortDate) + " " + QTime::currentTime().toString(Qt::DefaultLocaleLongDate).remove(QRegularExpression(" \\w+"));

    if(multiline && serverMode)
    {
        if(plainTextEdit)
        {
            if(plainTextEdit->blockCount() > 2000)  // too many lines may not be good
            {
                plainTextEdit->clear();
            };

            plainTextEdit->moveCursor(QTextCursor::End);
            plainTextEdit->insertPlainText("\n" + timeStamp + "  " + text);
            plainTextEdit->ensureCursorVisible();
        };
    }
    else
    {
        statusLabel->setText(text);
    };

    if(writeToLog)
        writeLog(text, timeStamp);
}

//**************************************************************************************************
//
//**************************************************************************************************

void SerialTransmissionDialog::setValue(int val)
{
   progressBar->setValue(val);
}

//**************************************************************************************************
//
//**************************************************************************************************

void SerialTransmissionDialog::setRange(int min, int max)
{
   if(max == 0)
   {
      progressBar->hide();
      //cancelButton->setText(tr("&Close"));
      //cancelButton->setIcon(QIcon(":/images/window-close.png"));
      //adjustSize();
   }
   else
   {
      progressBar->setRange(min, max);
      progressBar->show();
   };

   progressBar->setValue(0);
}

//**************************************************************************************************
//
//**************************************************************************************************

void SerialTransmissionDialog::updateStatus()
{
   updateStatusTimer->stop();

   if(xoffReceived)
        xoffLabel->setText("<html><head/><body><p><span style=\" font-weight:600;color:#ff0000;\">XOFF</span></p></body></html>");
   else
       xoffLabel->setText("<html><head/><body><p><span style=\" font-weight:600;color:#005500;\">XON</span></p></body></html>");


   if(sending)
   {
       if(serialPortWriteBuffer.isEmpty())
       {
           modeLabel->setText(" ");
           modeLabel->setToolTip(tr("Waiting for data..."));
       }
       else
       {
           modeLabel->setPixmap(QPixmap(":/images/send.png"));
           modeLabel->setToolTip(tr("Sending a file..."));
       };
   }
   else
   {
       if(serialPortReadBuffer.isEmpty())
       {
           modeLabel->setText(" ");
           modeLabel->setToolTip(tr("Waiting for data..."));
       }
       else
       {
           modeLabel->setPixmap(QPixmap(":/images/receive.png"));
           modeLabel->setToolTip(tr("Receiving a file..."));
       };
   };


   connected1Label->setText(tr("<html><head/><body><p>Port: <span style=\" font-weight:600;\">%1</span> is</p></body></html>").arg(portSettings.portName));
   if(serialPort.isOpen())
   {
       connectedLabel->setText(tr("<html><head/><body><p><span style=\" font-weight:600; color:#009500;\">OPEN</span></p></body></html>"));

       QSerialPort::PinoutSignals status = serialPort.pinoutSignals();

       ctsLabel->setEnabled(status & QSerialPort::ClearToSendSignal);
       dsrLabel->setEnabled(status & QSerialPort::DataSetReadySignal);
       dcdLabel->setEnabled(status & QSerialPort::DataCarrierDetectSignal);
       rtsLabel->setEnabled(status & QSerialPort::RequestToSendSignal);
       dtrLabel->setEnabled(status & QSerialPort::DataTerminalReadySignal);

       xoff1Label->setEnabled(!xoffReceived);

       if(sendStartDelayCountner > 0)
           setLabelText(tr("Sending starts in %1s").arg(sendStartDelayCountner));
       else
           if(xoffReceived) //  || !(status & QSerialPort::ClearToSendSignal)
           {

           };

       if((xoffReceived != prevXoffReceived))  // catch change of xoffReceived
       {
           qDebug() << "CHANGED xoffReceived" << xoffReceived << prevXoffReceived;

           prevXoffReceived = xoffReceived;
           if(xoffReceived)
           {
               if(sending)
               {
                   if((portSettings.waitForCts || (portSettings.FlowControl == QSerialPort::HardwareControl)) && !(status & QSerialPort::ClearToSendSignal))
                   {
                       setLabelText(tr("Waiting for CTS signal..."));
                   }
                   else
                       if(portSettings.waitForXon)
                       {
                           setLabelText(tr("Waiting for XON character..."));
                       }
                       else
                           if((portSettings.Xoff && portSettings.Xon) && (portSettings.FlowControl == QSerialPort::SoftwareControl))
                               setLabelText(tr("Waiting for XON character..."));
               };

           }
           else
           {
               if(writeBufferIterator != serialPortWriteBuffer.end()) // try to restart trasmission
               {
                   serialPortBytesWritten(0);
               };
               setLabelText("");
           };

       };

       if(portSettings.waitForCts || (portSettings.FlowControl == QSerialPort::HardwareControl))
       {
           if(!(status & QSerialPort::ClearToSendSignal))
               xoffReceived = true;
           else
               xoffReceived = false;
       };
   }
   else
   {
       ctsLabel->setEnabled(false);
       dsrLabel->setEnabled(false);
       dcdLabel->setEnabled(false);
       rtsLabel->setEnabled(false);
       dtrLabel->setEnabled(false);
       xoff1Label->setEnabled(xoffReceived);

       connectedLabel->setText(tr("<html><head/><body><p><span style=\" font-weight:600; color:#ff0000;\">CLOSED</span></p></body></html>"));
   };

   updateStatusTimer->start();
}

//**************************************************************************************************
//
//**************************************************************************************************

void SerialTransmissionDialog::sendStartDelayTimeout()
{
    if(sendStartDelayCountner > 0)
        sendStartDelayCountner--;
    else
    {
        sendStartDelayTimer->stop();
        xoffReceived = false;
        //prevXoffReceived = false;
    };
}

//**************************************************************************************************
//
//**************************************************************************************************

void SerialTransmissionDialog::stopButtonClicked()
{
    stop = true;
    qApp->processEvents();
}

//**************************************************************************************************
//
//**************************************************************************************************

void SerialTransmissionDialog::showSerialPortError(QSerialPort::SerialPortError error)
{
    QString text;
    QMessageBox msgBox;

    qDebug() << "Serial port error: " << error << " " << serialPort.errorString();

    switch(error)
    {
        case QSerialPort::NoError                   : text = tr("No Error has occured");
            //statusBar()->showMessage(text);
            return;
        case QSerialPort::DeviceNotFoundError       : text = tr("An error occurred while attempting to open an non-existing serial port.");
            stop = true;
            break;
        case QSerialPort::PermissionError           : text = tr("An error occurred while attempting to open an already opened serial port by another process or a user not having enough permission and credentials to open.");
            stop = true;
            break;
        case QSerialPort::OpenError                 : text = tr("An error occurred while attempting to open an already opened serial port.");
            stop = true;
            break;
        case QSerialPort::NotOpenError              : text = tr("Serial port is not opened.");
            stop = true;
            break;
        case QSerialPort::ParityError               : text = tr("Parity error detected by the hardware while reading data");
            stop = true;
            break;
        case QSerialPort::FramingError              : text = tr("Framing error detected by the hardware while reading data");
            stop = true;
            break;
        case QSerialPort::BreakConditionError       : text = tr("Break condition detected by the hardware on the input line");
            stop = true;
            break;
        case QSerialPort::WriteError                : text = tr("An I/O error occurred while writing the data");
            break;
        case QSerialPort::ReadError                 : text = tr("An I/O error occurred while reading the data");
            stop = true;
            break;
        case QSerialPort::ResourceError             : text = tr("An I/O error occurred when a resource becomes unavailable, e.g. when the device is unexpectedly removed from the system");
            stop = true;
            break;
        case QSerialPort::UnsupportedOperationError : text = tr("The requested serial port operation is not supported or prohibited by the running operating system.");
            stop = true;
            break;
        case QSerialPort::TimeoutError              : text = tr("A timeout error occurred.");
           stop = true;
           break;
        case QSerialPort::UnknownError              : text = tr("An unidentified error occurred");
           stop = true;
           break;
        default                                     : text = tr("An unidentified error occurred");
    };

    if(serverMode)
    {
        setLabelText(tr("ERROR:\t Serial port error %1: \"%2\".").arg(error).arg(serialPort.errorString()), serverMode, true);
        serialPort.clearError();
    }
    else
    {
        writeLog(tr("ERROR:\t Serial port error %1: \"%2\".").arg(error).arg(serialPort.errorString()), "");
        msgBox.setWindowTitle(tr("EdytorNC - serial transmission"));
        msgBox.setText(text);
        msgBox.setIcon(QMessageBox::Critical);
        msgBox.exec();
        canceled = true;
        stop = true;
        close();
    };
}

//**************************************************************************************************
//
//**************************************************************************************************

void SerialTransmissionDialog::serialPortBytesWritten(qint64 bytes)
{
    if(noOfBytes == 0)
        return;

    if(bytes > 0)
    {
        bytesWritten += bytes;
    };

    if(serverMode)
    {
        sendTimeoutCountner = portSettings.sendTimeout;
        if(!sendTimeoutTimer->isActive())
            sendTimeoutTimer->start();
    };

    setValue(bytesWritten);
    setLabelText(tr("Sending byte %1 of %2").arg(bytesWritten).arg(noOfBytes));

    qDebug() << "Bytes written" << bytesWritten << " of " << noOfBytes;

    if(stop)
        return;

    if(writeBufferIterator == serialPortWriteBuffer.end()) // all data sent
    {
        if(serverMode)
        {
            sendTimeoutCountner = 0;
            sendTimeoutTimer->stop();
            setLabelText(tr("OK:\t Sending completed."), serverMode, true);
            setLabelText(tr("Wainting for data..."));
            setRange(0, 0);
        }
        else
        {
            autoCloseCountner = portSettings.autoCloseTimeout;
            if(!autoCloseTimer->isActive())
                autoCloseTimer->start();

            stop = true;
        };

        sending = false;
        return;
    };

    if((portSettings.lineDelay > 0) && (bytes > 0))
    {
        QTimer::singleShot(int(portSettings.lineDelay * 1000), this, SLOT(lineDelaySlot()));
        return;
    };

    if(!xoffReceived)
    {
        QByteArray buff;
        buff.clear();
        buff.append(*writeBufferIterator);
        serialPort.write(buff, buff.size());
        writeBufferIterator++;

        qDebug() << "*** ***";

    };

    qDebug() << "*** xoffReceived ***" << xoffReceived << " Stop" << stop;
}

//**************************************************************************************************
//
//**************************************************************************************************

//void SerialTransmissionDialog::serialPortReadyRead()
//{
//    qint64 lineLength;
//    QByteArray buff;

//    char buf[1024];
//    while((lineLength = serialPort.readLine(buf, sizeof(buf))))
//    {

//        buff = buf;
//        int xoffPos = buff.lastIndexOf(portSettings.Xoff);
//        int xonPos = buff.lastIndexOf(portSettings.Xon);

//        if(xoffPos >= 0) //only XOFF received
//        {
//            xoffReceived = true;
//            setLabelText(tr("XOFF received..."));
//        };

//        if(xonPos >= 0)  //only XON received
//        {
//            xoffReceived = false;
//            sendStartDelay = 0;
//            setLabelText(tr("XON received..."));
//        };

//        if(xoffPos > xonPos) //both XOFF/XON received but XOFF last
//        {
//            xoffReceived = true;
//            setLabelText(tr("XOFF received..."));
//        };

//        serialPortReadBuffer.append(buf);




//        setLabelText(tr("Receiving byte %1").arg(serialPortReadBuffer.size() - 1));

//        //qDebug() << "Data read" << buff << "xoffReceived" << xoffReceived;

//        if(QString(buff).contains(endOfProgChar) && (!endOfProgChar.isEmpty()))
//        {
//            qDebug() << "endOfProgChar" << endOfProgChar;
//            qDebug() << "Procces data" << processReceivedData();
//        };

//    };
//    autoCloseCountner = autoCloseCountnerReloadValue;
//    if(!autoCloseTimer->isActive())
//        autoCloseTimer->start();

//}

//**************************************************************************************************
//
//**************************************************************************************************

void SerialTransmissionDialog::serialPortReadyRead()
{
    QByteArray buff(serialPort.readAll());

    // software flow control at application level
    if(((portSettings.Xoff > 0) && (portSettings.Xon > 0)) && sending)  // disabled when Xon or Xoff set to 0
    {
        int xoffPos = buff.lastIndexOf(portSettings.Xoff);
        int xonPos = buff.lastIndexOf(portSettings.Xon);

        if(xoffPos >= 0) //only XOFF received
        {
            xoffReceived = true;
            setLabelText(tr("XOFF received..."));
        };

        if(xonPos >= 0)  //only XON received
        {
            xoffReceived = false;
            sendStartDelayCountner = 1;
            setLabelText(tr("XON received..."));
        };

        if(xoffPos > xonPos) //both XOFF/XON received but XOFF last
        {
            xoffReceived = true;
            setLabelText(tr("XOFF received..."));
        };
    }
    else
        xoffReceived = false;

    if(portSettings.logData)
    {
       setLabelText(tr("Received data\n") + buff, true, true);
    };

    if(sending)
        return;

    serialPortReadBuffer.append(buff);

    setLabelText(tr("Receiving byte: %1").arg(serialPortReadBuffer.size() - 1));


    receiveTimeoutCountner = portSettings.receiveTimeout;

    if(!portSettings.endOfProgChar.isEmpty())
    {
        QRegExp exp(portSettings.endOfProgChar);

        if(QString(serialPortReadBuffer).lastIndexOf(exp) >= 0)
        {
            setLabelText(tr("Program received"));
            receiveTimeoutCountner = 1;
        };
    };

    if(!receiveTimeoutTimer->isActive())
        receiveTimeoutTimer->start();
}

//**************************************************************************************************
//
//**************************************************************************************************

void SerialTransmissionDialog::serialPortRequestToSend(bool set)
{
    qDebug() << "Request To Send - RTS " << set;
}

//**************************************************************************************************
//
//**************************************************************************************************

void SerialTransmissionDialog::sendData(QString dataToSend, QString configName)
{
    //QTimer *sendStartDelayTimer = NULL;

    if(dataToSend.isEmpty())
        return;

    if(configName.isEmpty())
        return;

    loadConfig(configName);

    resetTransmission(true);

    prepareDataBeforeSending(&dataToSend);


    if(stop)
        return;

    sending = true;

    if(portSettings.waitForCts)
    {
        xoffReceived = true;
        prevXoffReceived = false;
    };

    if(portSettings.waitForXon)
    {
        if((portSettings.Xoff > 0) && (portSettings.Xon > 0))
        {
            xoffReceived = true;
            prevXoffReceived = false;
        };
    };

    if((portSettings.sendStartDelayReloadValue > 0)) // && (portSettings.FlowControl == QSerialPort::SoftwareControl)
    {
        xoffReceived = true;
        prevXoffReceived = false;
        sendStartDelayCountner = portSettings.sendStartDelayReloadValue;
        sendStartDelayTimer->start();
    };

    qDebug() << "4569" << "xoffReceived" << xoffReceived;

    writeBufferIterator = serialPortWriteBuffer.begin();
    serialPortBytesWritten(0);  // start
    exec();
    sending = false;
}

//**************************************************************************************************
//
//**************************************************************************************************

void SerialTransmissionDialog::prepareDataBeforeSending(QString *data)
{
    if(data->isEmpty())
        return;

    // prepare data to send
    if(portSettings.removeBefore && (data->count('%') > 1))
    {
        data->remove(0, data->indexOf("%"));
    };

    // ensure that data contains only CRLF line endings
    QRegExp exp;
    exp.setPattern("[\\n\\r]{1,}");
    int i = data->indexOf(exp); // detecting EOB combination used
    if(i >= 0)
    {
        if(exp.matchedLength() > 5)  // too long EOB, try to find next one
            i = data->indexOf(exp, i + exp.matchedLength());

        QString tx = data->mid(i, exp.matchedLength());
        data->replace(tx, "\r\n");
    };

    if(!portSettings.sendAtBegining.isEmpty())
    {
        procesSpecialCharacters(&portSettings.sendAtBegining, data);
        data->prepend(portSettings.sendAtBegining);
    };

    if(!portSettings.sendAtEnd.isEmpty())
    {
        procesSpecialCharacters(&portSettings.sendAtEnd, data);
        data->append(portSettings.sendAtEnd);
    };

    serialPortWriteBuffer = data->split("\n", QString::SkipEmptyParts); // \n is not appended to a string only \r are left
    serialPortWriteBuffer.replaceInStrings("\r", portSettings.eobChar); // insert line endings. \r is replaced with choosen line ending

    noOfBytes = serialPortWriteBuffer.join("").length();  // get new size
    setRange(0, noOfBytes);
}

//**************************************************************************************************
//
//**************************************************************************************************

void SerialTransmissionDialog::procesSpecialCharacters(QString *text, QString *fileData)
{
    if(text->isEmpty())
        return;

    bool ok;
    QString fileExt = "";
    QString fileExtA = "";
    QString fileName = "";

    if((!fileData->isEmpty()) || (!fileData->isNull()))
    {
        QStringList list = (guessFileName(fileData));

        fileName = list.at(0);
        fileExt = list.at(1);
        if(!fileExt.isEmpty())
        {
            fileExt.prepend('.');
            fileName.remove(fileExt);
        };

        int pos = fileName.lastIndexOf('_');
        if(pos >= 0)
        {
            fileExtA =  fileName.mid(pos, fileName.length());
            fileName.remove(fileExtA);
        };
    };


    // HEX codes
    QRegExp exp("0x[0-9a-fA-F]{1,2}");
    int pos = 0;
    while((pos = text->indexOf(exp, pos)) >= 0)
    {
        QString fTx = text->mid(pos, exp.matchedLength());
        char chr = fTx.toInt(&ok, 16);
        if(ok)
            text->replace(pos, exp.matchedLength(), QString(chr));
    };
    text->remove(" ");


    if(text->contains("LF"))
        text->replace("LF", "\r\n");

    if(text->contains("CR"))
        text->replace("CR", "\r\n");

    if(text->contains("TAB"))
        text->replace("TAB", "\t");

    if(text->contains("SP"))
        text->replace("SP", " ");

    if(text->contains("FA"))  // File Extension Appended at end of filename (after _ char.)
    {
        text->replace("FA", fileExtA);
    };

    if(text->contains("FN"))  // FileName
    {
        text->replace("FN", fileName);
    };

    if(text->contains("FE"))  // File Extension
    {
        text->replace("FE", fileExt);
    };

}
//**************************************************************************************************
//
//**************************************************************************************************

void SerialTransmissionDialog::loadConfig(QString configName)
{
    QString port;
    bool ok;


    portSettings.configName = configName;
    setObjectName(portSettings.configName);
    stop = true;
    QSettings settings(QSettings::IniFormat, QSettings::UserScope, "EdytorNC", "EdytorNC");
    settings.beginGroup("SerialPortConfigs");

#ifdef Q_OS_WIN
    port = "COM1";
#else
    port = "/dev/ttyUSB0";
#endif

    settings.beginGroup(configName);

    portSettings.portName = settings.value("PortName", port).toString();
    portSettings.BaudRate = (QSerialPort::BaudRate) settings.value("BaudRate", QSerialPort::Baud9600).toInt();
    portSettings.DataBits = (QSerialPort::DataBits) settings.value("DataBits", QSerialPort::Data8).toInt();
    portSettings.StopBits = (QSerialPort::StopBits) settings.value("StopBits", QSerialPort::TwoStop).toInt();
    portSettings.Parity = (QSerialPort::Parity) settings.value("Parity", QSerialPort::NoParity).toInt();
    portSettings.FlowControl = (QSerialPort::FlowControl) settings.value("FlowControl", QSerialPort::HardwareControl).toInt();
    portSettings.lineDelay = settings.value("LineDelay", 0).toDouble();
    portSettings.Xon = settings.value("Xon", "17").toString().toInt(&ok, 0);
    portSettings.Xoff = settings.value("Xoff", "19").toString().toInt(&ok, 0);
    portSettings.sendAtEnd = settings.value("SendAtEnd", "").toString();
    portSettings.sendAtBegining = settings.value("SendAtBegining", "").toString();
    portSettings.deleteControlChars = settings.value("DeleteControlChars", true).toBool();
    portSettings.removeEmptyLines = settings.value("RemoveEmptyLines", true).toBool();
    portSettings.removeBefore = settings.value("RemoveBefore", false).toBool();
    portSettings.removeSpaceEOB = settings.value("RemoveSpaceEOB", false).toBool();
    portSettings.autoCloseTimeout = settings.value("AutoCloseTime", 15).toInt();
    portSettings.sendStartDelayReloadValue = settings.value("SendingStartDelay", 0).toInt();
    portSettings.createLogFile = settings.value("CreateLogFile", true).toBool();
    portSettings.eobChar = settings.value("EobChar", "CRLF").toString();
    portSettings.sendTimeout = settings.value("SendTimeoutTime", 3).toInt();
    portSettings.receiveTimeout = settings.value("ReceiveTimeoutTime", 5).toInt();
    portSettings.removeFromRecieved = settings.value("RemoveFromReceived", "").toString();

    portSettings.autoSave = settings.value("AutoSave", false).toBool();
    portSettings.endOfProgChar = settings.value("EndOfProgExpSelected", "").toString();
    portSettings.renameIfExists = settings.value("CreateBackup", true).toBool();
    portSettings.removeLetters = settings.value("RemoveLetters", true).toBool();
    portSettings.guessFileNameByProgName = settings.value("DetectFormFileName", true).toBool();
    portSettings.appendExt = settings.value("AppendExtension", false).toBool();
    portSettings.useAsExt = settings.value("UseAsExtension", false).toBool();
    portSettings.splitPrograms = settings.value("SplitPrograms", true).toBool();
    portSettings.savePath = settings.value("SavePath", "").toString();
    portSettings.saveExt = settings.value("SaveExt", ".nc").toString();

    portSettings.logData = settings.value("DataToLogFile", false).toBool();
    portSettings.waitForCts = settings.value("WaitForCTS", false).toBool();
    portSettings.waitForXon = settings.value("WaitForXON", false).toBool();
    portSettings.sendXon = settings.value("SendXONAtStart", false).toBool();

    portSettings.fileServer = settings.value("FileServer", false).toBool();
    portSettings.callerProgName = settings.value("CallerProg", "O5555").toString();
    portSettings.reconnectTime = settings.value("ReconnectTimeoutTime", 60).toInt();
    portSettings.searchPath1 = settings.value("SearchPath1", "").toString();
    portSettings.searchExt1 = settings.value("SearchExt1", ".nc").toString();
    portSettings.searchPath2 = settings.value("SearchPath2", "").toString();
    portSettings.searchExt2 = settings.value("SearchExt2", ".nc").toString();
    portSettings.searchPath3 = settings.value("SearchPath3", "").toString();
    portSettings.searchExt3 = settings.value("SearchExt3", ".nc").toString();

    portSettings.fileNameLowerCase = settings.value("FileNameLowerCase", true).toBool();
    portSettings.fileNameExpFs = settings.value("FileNameExpFSSelected", "").toString();
    portSettings.fileNameMask = settings.value("FileNameMaskSelected", "").toString();
    portSettings.fileNameExpAs = settings.value("FileNameExpASSelected", "").toString();


    settings.endGroup();
    settings.endGroup();


    if(portSettings.eobChar.contains("LF"))
        portSettings.eobChar.replace("LF", "\n");

    if(portSettings.eobChar.contains("CR"))
        portSettings.eobChar.replace("CR", "\r");

    //portLabel->setText(portSettings.portName);
    //settingsNameLabel->setText(portSettings.configName);

}

//**************************************************************************************************
//
//**************************************************************************************************

void SerialTransmissionDialog::lineDelaySlot()
{
    serialPortBytesWritten(0);
}

//**************************************************************************************************
//
//**************************************************************************************************

QStringList SerialTransmissionDialog::receiveData(QString configName)
{
    QStringList outputList;

    outputList.clear();
    if(configName.isEmpty())
        return outputList;

    loadConfig(configName);

    resetTransmission(true);

    if(stop)
        return QStringList("");

    sendStartDelayCountner = 0;  // not needed when receiving

    setRange(0, 0); // do not display progress bar


    setLabelText(tr("Receiving byte %1").arg(0));

    if(serialPort.isOpen())
        if(portSettings.sendXon && (!sending))
            serialPort.write(&portSettings.Xon, 1);

    exec();

    return processReceivedData();
}

//**************************************************************************************************
//
//**************************************************************************************************

QStringList SerialTransmissionDialog::processReceivedData()
{
    QString readData;
    QStringList outputList;
    qint64 j;
    qint64 i;
    QRegExp exp;

    outputList.clear();
    if(serialPortReadBuffer.isEmpty())
        return outputList;

    i = serialPortReadBuffer.size();
    for(j = 0; j < i; j++)
    {
        if(portSettings.deleteControlChars)
            if(((serialPortReadBuffer.at(j) <= 0x1F) || (serialPortReadBuffer.at(j) >= 0x7F))) // is control character (below 0x1F and above 0x7F)
                if((serialPortReadBuffer.at(j) != 0x0A) && (serialPortReadBuffer.at(j) != 0x0D)) // but not LF or CR
                    continue; //skip this character

        readData.append(serialPortReadBuffer.at(j));
    };

    exp.setCaseSensitivity(Qt::CaseInsensitive);
    if(!portSettings.removeFromRecieved.isEmpty())
    {
        exp.setPattern(portSettings.removeFromRecieved);
        readData.remove(exp);
    };

    exp.setPattern("[\\n\\r]{1,}");
    i = readData.indexOf(exp); // detecting what EOB combination is used
    if(i >= 0)
    {
        QString tx = readData.mid(i, exp.matchedLength());
        readData.replace(tx, "\r\n");
    };

    if(portSettings.removeSpaceEOB) //removes white space at end of line added by Fanuc
    {
        readData.replace(" \r\n", "\r\n");
    };

    if(portSettings.removeEmptyLines)
    {
        QRegExp exp("(\\r\\n){2,}");
        int i = 0;
        while(i >= 0)
        {
            i = readData.indexOf(exp, 0);
            if(i >= 0)
            {
                readData.replace(exp, "\r\n");
            };
        };
    };

    if(!portSettings.autoSave)
    {
        outputList.append(readData);
        return outputList;
    }
    else
        outputList.append("#FILE_LIST#");

    if(portSettings.splitPrograms) // check for more than one program in received data
    {
        QStringList progList = splitFile(&readData);

        if(progList.size() > 1)
        {
            QString tmpData;
            QStringList::const_iterator it = progList.constBegin();
            while(it != progList.constEnd())
            {
                tmpData.clear();
                tmpData.append(*it);
                readData.remove(tmpData);
                outputList.append(saveDataToFile(&tmpData));
                it++;

            };

            qDebug() << "LOST DATA" << readData;
        }
        else
            outputList.append(saveDataToFile(&readData));
    }
    else
        outputList.append(saveDataToFile(&readData));

    return outputList;
}

//**************************************************************************************************
// Detect program name
//**************************************************************************************************

QStringList SerialTransmissionDialog::guessFileName(QString *text)
{
    QString fileName, extension, name1, ext1, name2, ext2;
    int pos;
    QRegExp expression;
    QStringList list;

    list.clear();
    if(text->isEmpty())
        return list;

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
                    ext1.replace('_', '.');;
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

        if(portSettings.removeLetters)
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
            expression.setPattern(portSettings.fileNameExpAs);
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


    if(portSettings.removeLetters)
    {
        QString tmpName = name2;
        tmpName.remove(QRegExp("[a-zA-Z-.]{1,}"));
        tmpName = tmpName.simplified();
        tmpName = tmpName.trimmed();
        if(!tmpName.isEmpty()) // use name with letters if new name is empty
            name2 = tmpName;
    };

    if(portSettings.fileNameLowerCase)
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

    if(name1 == portSettings.callerProgName)
    {
        fileName = name1;
        extension = ext1;
    }
    else
    {
        if(portSettings.guessFileNameByProgName)
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
    };


    qDebug() << "17.1" << fileName << extension << portSettings.guessFileNameByProgName;

    list.append(fileName);
    list.append(extension);


    // prepare full path
    if(fileName.isEmpty())
    {
        QString dateTime = QDate::currentDate().toString("yyyy-MM-dd") + "_" + QTime::currentTime().toString("HH_mm_ss").remove(QRegularExpression(" \\w+"));
        fileName = portSettings.savePath + "/" + dateTime + portSettings.saveExt;
        setLabelText(tr("WARNING:\t Could not find program name. Using date time code."), serverMode);

        qDebug() << "18" << fileName;
    }
    else
    {
        if(portSettings.useAsExt && (!extension.isEmpty()))
            fileName = portSettings.savePath + "/" + fileName + extension;
        else
        {
            if(portSettings.appendExt)
                fileName = portSettings.savePath + "/" + fileName + extension.replace('.', '_') + portSettings.saveExt;
            else
                fileName = portSettings.savePath + "/" + fileName + portSettings.saveExt;
        };
    };

    list.append(fileName); // full path

    qDebug() << "17.5" << fileName << portSettings.appendExt << portSettings.useAsExt;

    return list;
}

//**************************************************************************************************
//  Save received program to a file. Return filename and empty program text if succes or leave program text unchanged
//**************************************************************************************************

QString SerialTransmissionDialog::saveDataToFile(QString *text)
{
    QFile file;
    QString fileName;

    if(!portSettings.autoSave)
        return "";

    if(text->isNull() || text->isEmpty())
        return "";

    if(text->length() < 15)
    {
        setLabelText(tr("ERROR:\t Received file too small (less than 15 characters)."), serverMode, true);
        return "";
    };

    QStringList list = guessFileName(text);

    if(list.size() < 3) // always size should be 3
        return "";

    fileName = list.at(2);

    file.setFileName(fileName);

    if(file.exists() && portSettings.renameIfExists)
    {
//        QString oldName = fileName;
//        oldName.replace(QRegExp("\\.[a-zA-Z0-9]{1,3}"), ".bak");
//        QFile::remove(oldName);

        QFileInfo fileInfo;
        fileInfo.setFile(fileName);
        QString backupName = fileInfo.path() + '/' + fileInfo.baseName();
        backupName.append(".bak");
        backupName = QDir::cleanPath(backupName);

        QFile::remove(backupName);

        if(file.rename(fileName, backupName))
        {
            // write to log file
            setLabelText(tr("OK:\t Renaming file: \"%1\" to \"%2\".").arg(fileName).arg(backupName), serverMode, true);
        }
        else
        {
            // write error to log file
            setLabelText(tr("ERROR:\t Renaming file: \"%1\". %2").arg(fileName).arg(file.errorString()), serverMode, true);
        };
    }; 

    if(file.open(QIODevice::WriteOnly))
    {
        // save received data to file
        QTextStream out(&file);
        out << *text;
        file.flush();
        file.close();

        text->clear();


        // write to log file
        setLabelText(tr("OK:\t Saving file: \"%1\".").arg(fileName), serverMode, true);
    }
    else
    {
        // write error to log file
        setLabelText(tr("ERROR:\t Saving file: \"%1\". %2").arg(fileName).arg(file.errorString()), serverMode, true);
    };

    return fileName;

}

//**************************************************************************************************
//  Write to log file
//**************************************************************************************************

void SerialTransmissionDialog::writeLog(QString msg, QString timeStamp)
{
    QByteArray text;
    QFile logFile;

    // log file
    if((!portSettings.savePath.isEmpty()) && portSettings.createLogFile)
    {
        logFile.setFileName(portSettings.savePath + "/" + portSettings.configName + "_serial_log.txt");

        QIODevice::OpenMode flags = QIODevice::WriteOnly;

        if((QFileInfo(logFile).size()) > 1048576)  // limit size of log file to 1MB
            flags |= QIODevice::Truncate;
        else
            flags |= QIODevice::Append;

        if(logFile.open(flags))
        {
            if((flags & QIODevice::Truncate))
                writeLog(tr("Log file truncated, size > 1MB."));

            if(timeStamp.isEmpty())
            {
                timeStamp = QDate::currentDate().toString(Qt::DefaultLocaleShortDate) + " " + QTime::currentTime().toString(Qt::DefaultLocaleLongDate).remove(QRegularExpression(" \\w+"));
            };

            text.append(timeStamp);
            text.append("  ");
            logFile.write(text, text.length());

            text.clear();
            text.append(msg + "\r\n");
            logFile.write(text, text.length());

            logFile.close();
        }
        else
        {
            qDebug() << "Cannot open log file";
        };

    };
}

//**************************************************************************************************
// Split file
//**************************************************************************************************

QStringList SerialTransmissionDialog::splitFile(QString *text)
{
    int progBegin, progEnd;
    QStringList progs, exp;
    QList<int> progBegins;
    int index;
    QString tx;
    QRegExp expression;

    progs.clear();
    if(text->isNull() || text->isEmpty())
        return progs;


    exp << FILENAME_SINU840
        << FILENAME_OSP
        << FILENAME_FANUC
        << FILENAME_SINU
        << FILENAME_HEID1
        << FILENAME_HEID2
        << FILENAME_PHIL
        << FILENAME_FADAL;


    // detect CNC control type
    foreach(const QString expTx, exp)
    {
        expression.setPattern(expTx);
        if(text->contains(expression))
        {
            exp.clear();
            exp.append(expTx);
            break;
        };
    };

    //  prepare program list
    index = 0;
    foreach(const QString expTx, exp)
    {
        expression.setPattern(expTx);
        do
        {
            index = text->indexOf(expression, index);
            if(index >= 0)
            {
                progBegins.append(index);
                index += expression.matchedLength();
            }
            else
                index = 0;

        }while(index > 0);
    };


//    if(!endOfProgChar.isEmpty())
//    {
//        index = 0;
//        expression.setPattern(endOfProgChar);
//        do
//        {
//            index = text->indexOf(expression, index);
//            if(index >= 0)
//            {
//                index += expression.matchedLength();
//                progBegins.append(index);
//            }
//            else
//                index = 0;

//        }while(index > 0);
//    };

    std::sort(progBegins.begin(), progBegins.end());


    // split file  TODO: data can be lost if filename detection fails also some garbage are left
    QList<int>::const_iterator it = progBegins.constBegin();
    while(it != progBegins.constEnd())
    {
        progBegin = *it;
        it++;
        if(it != progBegins.constEnd())
            progEnd = *it;
        else
            progEnd = text->size();

        tx = text->mid(progBegin, progEnd - progBegin);
        if(!tx.isEmpty())
        {
            progs.append(tx);
        };
    };

    return progs;
}

//**************************************************************************************************
//
//**************************************************************************************************

void SerialTransmissionDialog::startFileServer(QString configName)
{
    stop = true;

    if(configName.isEmpty())
        return;

    loadConfig(configName);

    if(!portSettings.fileServer)
    {
        QMessageBox::information(this, tr("Serial transmission - File server"),
                                 tr("Can't start.\n"
                                    "File server option is not enabled in serial port settings"));
        return;
    };


    resetTransmission(true);

    if(serialPort.isOpen())
        if(portSettings.sendXon && (!sending))
            serialPort.write(&portSettings.Xon, 1);

    setWindowTitle(tr("%1").arg(configName)); //"Serial transmission - File server -> %1"
    setRange(0, 0);
    setLabelText(tr("Waiting for data..."));
}

//**************************************************************************************************
//
//**************************************************************************************************

void SerialTransmissionDialog::fileServerProcessData()
{
    QByteArray buff;
    QString fileName, ext, path, tmpBuff;
    QFileInfo fileInfo;


    setLabelText(tr("Received data..."));


    QStringList progList = processReceivedData();
    serialPortReadBuffer.clear();


    if(!progList.isEmpty())
    {
        QStringList::const_iterator itp = progList.constBegin();

        if((*itp) == "#FILE_LIST#")
        {
            qDebug() << "File Server 0012" << *itp;

            itp++;
            while(itp != progList.constEnd())
            {
                fileInfo.setFile(*itp);
                fileName = fileInfo.baseName();

                qDebug() << "File Server 0013" << *itp << fileName << portSettings.callerProgName;

                if(fileName == portSettings.callerProgName)
                {
                    setLabelText(tr("INFO:\t Received \"Caller\" program: \"%1\".").arg(fileInfo.fileName()), serverMode, true);

                    QFile file(*itp);
                    if(file.open(QIODevice::ReadOnly))
                    {
                        qDebug() << "File Server 0014" << *itp;
                        buff = file.readAll();
                        file.close();
                    };

                    fileName.clear();
                    ext.clear();
                    tmpBuff.clear();
                    tmpBuff.append(buff);
                    QRegExp exp(portSettings.fileNameExpFs);
                    int pos = tmpBuff.indexOf(exp, Qt::CaseInsensitive);

                    qDebug() << "File Server 0015" << buff << pos;

                    if(pos >= 1)
                    {
                        fileName.append(tmpBuff.mid(pos, exp.matchedLength()));
                        fileName = fileName.simplified();
                        fileName.remove(" ");

                        setLabelText(tr("INFO:\t Requested file name (RAW): \"%1\"").arg(fileName), serverMode, true);

                        QString fileNameMask = portSettings.fileNameMask;

                        if(fileNameMask.isEmpty())
                        {
                            fileName.remove(';');
                            fileName.remove('(');
                            fileName.remove(')');
                            fileName.remove('[');
                            fileName.remove(']');
                            fileName.remove('*');
                            fileName.remove(',');
                        }
                        else
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

                                        qDebug() << "File Server 0015.2" << num << j;

                                        if(j < fileName.length())
                                            tx.append(fileName[j]);

                                    };
                                };

                                i++;

                                qDebug() << "File Server 0015.5" << i;
                            };

                            fileName = tx;
                        };


                        fileInfo.setFile(fileName);
                        ext = fileInfo.suffix();
                        if(!ext.isEmpty())
                        {
                            ext.prepend('.');
                            fileName.remove(ext);
                        };

                        if(portSettings.fileNameLowerCase)
                        {
                            fileName = fileName.toLower();
                            ext = ext.toLower();
                        }
                        else
                        {
                            fileName = fileName.toUpper();
                            ext = ext.toUpper();
                        };


                        setLabelText(tr("INFO:\t Requested file name (mask: %3): \"%1%2\"").arg(fileName).arg(ext).arg(portSettings.fileNameMask), serverMode, true);
                    };

                    qDebug() << "File Server 1" << fileName << ext;

                    if(!fileName.isEmpty())
                    {
                        path = portSettings.searchPath1 + "/" + fileName + (ext.isEmpty() ? portSettings.searchExt1 : ext);
                        fileInfo.setFile(path);
                        if(!fileInfo.exists())
                        {
                            if(!portSettings.searchPath1.isEmpty())
                                setLabelText(tr("ERROR:\t Can't find file in path 1: \"%1\".").arg(path), serverMode, true);

                            path = portSettings.searchPath2 + "/" + fileName + (ext.isEmpty() ? portSettings.searchExt2 : ext);
                            fileInfo.setFile(path);
                            if(!fileInfo.exists())
                            {
                                if(!portSettings.searchPath2.isEmpty())
                                    setLabelText(tr("ERROR:\t Can't find file in path 2: \"%1\".").arg(path), serverMode, true);

                                path = portSettings.searchPath3 + "/" + fileName + (ext.isEmpty() ? portSettings.searchExt3 : ext);
                                fileInfo.setFile(path);
                                if(!fileInfo.exists())
                                {
                                    if(!portSettings.searchPath3.isEmpty())
                                        setLabelText(tr("ERROR:\t Can't find file in path 3: \"%1\".").arg(path), serverMode, true);

                                    path.clear();
                                };
                            };
                        };

                        qDebug() << "File Server 0016" << path << fileName << ext;

                        if(!path.isEmpty())
                        {
                            file.setFileName(path);
                            if(file.open(QIODevice::ReadOnly))
                            {
                                qDebug() << "File Server 2" << path;


                                buff = file.readAll();
                                file.close();

                                resetTransmission();

                                tmpBuff.clear();
                                tmpBuff.append(buff);


//                                if(!portSettings.guessFileNameByProgName && !portSettings.fileNameExpAs.isEmpty())
//                                {
//                                    QRegExp exp;
//                                    exp.setPattern(portSettings.fileNameExpAs);
//                                    int i = data->indexOf(exp);
//                                    if(i < 0)
//                                    {

//                                    };


//                                };




                                prepareDataBeforeSending(&tmpBuff);
                                setRange(0, noOfBytes);
                                sending = true;
                                writeBufferIterator = serialPortWriteBuffer.begin();

                                if(portSettings.waitForCts)
                                {
                                    xoffReceived = true;
                                    prevXoffReceived = false;
                                };

                                if(portSettings.waitForXon)
                                {
                                    if((portSettings.Xoff > 0) && (portSettings.Xon > 0))
                                    {
                                        xoffReceived = true;
                                        prevXoffReceived = false;
                                    };
                                };

                                if((portSettings.sendStartDelayReloadValue > 0)) // && (portSettings.FlowControl == QSerialPort::SoftwareControl)
                                {
                                    xoffReceived = true;
                                    prevXoffReceived = false;
                                    sendStartDelayCountner = portSettings.sendStartDelayReloadValue;
                                    sendStartDelayTimer->start();
                                };

                                fileServerBytesWritten(0);  // start

                                setLabelText(tr("OK:\t Sending a file: \"%1\".").arg(path), serverMode, true);
                            }
                            else
                            {
                                setLabelText(tr("ERROR:\t Can't send file: \"%1\". %2").arg(path).arg(file.errorString()), serverMode, true);
                            };
                        };
                    }
                    else
                    {
                        setLabelText(tr("ERROR:\t Can't find program name to send in \"Caller\" program.").arg(path), serverMode, true);
                    };

                    break;
                }
                itp++;
            };
        };
    };
    setLabelText(tr("Waiting for data..."));
}

//**************************************************************************************************
//
//**************************************************************************************************

void SerialTransmissionDialog::fileServerBytesWritten(qint64 bytes)
{
    if(bytes > 0)
        bytesWritten += bytes;

    setValue(bytesWritten);
    setLabelText(tr("Sending byte %1 of %2").arg(bytesWritten).arg(noOfBytes));

    //qDebug() << "Bytes written" << bytesWritten << " of " << noOfBytes;

    if(stop)
        return;

    if(writeBufferIterator == serialPortWriteBuffer.end())
    {
        //stop = true;
        sending = false;
        setLabelText(tr("OK:\t Sending a file completed."), serverMode, true);
        setLabelText(tr("Waiting for data..."));
        setRange(0, 0);
        return;
    };

    if((portSettings.lineDelay > 0) && (bytes > 0))
    {
        QTimer::singleShot(int(portSettings.lineDelay * 1000), this, SLOT(lineDelaySlot()));
        return;
    };

    if(!xoffReceived)
    {
        QByteArray buff;
        buff.clear();
        buff.append(*writeBufferIterator);

        serialPort.write(buff, buff.size());
        writeBufferIterator++;
    };

    //qDebug() << "xoffReceived" << xoffReceived << " Stop" << stop;
}

//**************************************************************************************************
//
//**************************************************************************************************

void SerialTransmissionDialog::sendTimeoutTimerTimeout()
{
    if(sendTimeoutCountner == 0)
    {
        setLabelText(tr("ERROR:\t Sending timedout. Reseting."), serverMode, true);
        sendTimeoutTimer->stop();
        reset(false);
        return;
    };
}

//**************************************************************************************************
//
//**************************************************************************************************

void SerialTransmissionDialog::receiveTimeoutTimerTimeout()
{

    qDebug() << "receiveTimeoutTimerTimeout: " << receiveTimeoutCountner;

    if(serverMode)
    {
        if(receiveTimeoutCountner == 0)
            timeoutLabel->setText("");
        else
            timeoutLabel->setText(tr("Receive timeout in: %1s").arg(receiveTimeoutCountner));
    };

    if(receiveTimeoutCountner == 0)
    {
        receiveTimeoutTimer->stop();
        return;
    };

    if(receiveTimeoutCountner <= 1)
    {
        receiveTimeoutCountner = 0;
        if(serverMode)
        {
            fileServerProcessData();
        }
        else
        {
            autoCloseCountner = portSettings.autoCloseTimeout;
            if(!autoCloseTimer->isActive())
                autoCloseTimer->start();
        };
    }
    else
    {
        if(!xoffReceived || portSettings.FlowControl == QSerialPort::HardwareControl)
            receiveTimeoutCountner--;
    };
}

//**************************************************************************************************
//
//**************************************************************************************************

void SerialTransmissionDialog::resetTransmission(bool portRestart)
{
    reconnectTimer->stop();

    autoCloseTimer->stop();
    serialPort.clearError();
    bytesWritten = 0;
    xoffReceived = false;
    prevXoffReceived = false;
    sending = false;
    stop = false;
    serialPortReadBuffer.clear();
    serialPortWriteBuffer.clear();
    writeBufferIterator = serialPortWriteBuffer.end(); // we don't send anything yet
    noOfBytes = 0;

    if(portRestart)
    {
        disconnect(&serialPort, 0, 0, 0);

        if(serialPort.isOpen())
        {
            // try to clear the serial port, clear() reports error on windows
            serialPort.setFlowControl(QSerialPort::NoFlowControl);
            serialPort.flush();
            serialPort.readAll();
            serialPort.setFlowControl(portSettings.FlowControl);
            serialPort.close();
            serialPort.clearError();
        };

        qApp->processEvents();

        connect(&serialPort, SIGNAL(error(QSerialPort::SerialPortError)), this, SLOT(showSerialPortError(QSerialPort::SerialPortError)));
        connect(&serialPort, SIGNAL(bytesWritten(qint64)), this, SLOT(serialPortBytesWritten(qint64)));
        connect(&serialPort, SIGNAL(readyRead()), this, SLOT(serialPortReadyRead()));
        if(portSettings.FlowControl == QSerialPort::HardwareControl)
            connect(&serialPort, SIGNAL(requestToSendChanged(bool)), this, SLOT(serialPortRequestToSend(bool)));

        qApp->processEvents();

        serialPort.clearError();
        serialPort.setPortName(portSettings.portName);
        serialPort.setBaudRate(portSettings.BaudRate, QSerialPort::Output);
        serialPort.setBaudRate(portSettings.BaudRate, QSerialPort::Input);
        serialPort.setDataBits(portSettings.DataBits);
        serialPort.setParity(portSettings.Parity);
        serialPort.setStopBits(portSettings.StopBits);
        serialPort.setFlowControl(portSettings.FlowControl);

        if(serialPort.open(QIODevice::ReadWrite))
        {
            stop = false;
        }
        else
        {
            stop = true;
        };

    };

    if(serverMode)
    {
        if(portSettings.reconnectTime < 300)
        {
            reconnectTimer->setInterval(portSettings.reconnectTime * 1000);
            reconnectTimer->start();
        };
    };
}

//**************************************************************************************************
//
//**************************************************************************************************

void SerialTransmissionDialog::reset(bool re)
{
    Q_UNUSED(re);

    resetTransmission(true);
}

//**************************************************************************************************
//
//**************************************************************************************************

QString SerialTransmissionDialog::configName()
{
    return portSettings.configName;
}

//**************************************************************************************************
//
//**************************************************************************************************

void SerialTransmissionDialog::portReset()
{
    reset(true);
}

//**************************************************************************************************
//
//**************************************************************************************************

void SerialTransmissionDialog::reconnectTimerTimeout()
{
    if(!serialPort.isOpen())
        reset(true);
}

//**************************************************************************************************
//
//**************************************************************************************************

QString SerialTransmissionDialog::savePath()
{
    return portSettings.savePath;
}

//**************************************************************************************************
//
//**************************************************************************************************

QStringList SerialTransmissionDialog::readPaths()
{
    return QStringList() << portSettings.searchPath1 << portSettings.searchPath2 << portSettings.searchPath3;
}

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


#include "../QtSingleApplication/qtsingleapplication.h"
#include "spfsmainwindow.h"
#include <QDebug>



#define LOCALE_PATH         "/usr/share/edytornc/lang/"




int main(int argc, char *argv[])
{   
   Q_INIT_RESOURCE(application);
   QtSingleApplication app("SPFSMainWindow", argc, argv);

   if(app.isRunning())
       return !app.sendMessage("Show");

   QTranslator qtTranslator; // Try to load Qt translations from QLibraryInfo::TranslationsPath if this fails looks for translations in app dir.
   if(!qtTranslator.load("qt_" + QLocale::system().name(), QLibraryInfo::location(QLibraryInfo::TranslationsPath)))
      qtTranslator.load("qt_" + QLocale::system().name(), app.applicationDirPath());
   app.installTranslator(&qtTranslator);

   QTranslator myappTranslator; // Try to load EdytorNC translations from LOCALE_PATH if this fails looks for translations in app dir.
   if(!myappTranslator.load("edytornc_" + QLocale::system().name(), LOCALE_PATH))
      myappTranslator.load("edytornc_" + QLocale::system().name(), app.applicationDirPath());
   app.installTranslator(&myappTranslator);

   SPFSMainWindow *mw = new SPFSMainWindow();

   mw->show();

   app.setActivationWindow(mw, false);

   QObject::connect(&app, SIGNAL(messageReceived(const QString&)), mw, SLOT(messReceived(const QString&)));

//   QObject::connect(mw, SIGNAL(needToShow()), &app, SLOT(activateWindow()));

   return app.exec();

}


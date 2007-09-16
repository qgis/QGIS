/***************************************************************************
     main.cpp
     --------------------------------------
    Date                 : Sun Sep 16 12:06:57 AKDT 2007
    Copyright            : (C) 2007 by Gary E. Sherman
    Email                : sherman at mrcc dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifdef HAVE_CONFIG_H
#include <qgsconfig.h>
#endif

#include <cstdio>
#include <cstdlib>
#include "plugingui.h"

#include <QApplication>
#include <QTranslator>
#include <QString>
#include <QTextCodec>
#include "qgsapplication.h"

int main(int argc, char *argv[])
{
  QApplication a(argc, argv);

  /* Load translationfile */
  QTranslator tor(0);
  tor.load(QString("qgis_") + QTextCodec::locale(), QgsApplication::pkgDataPath() + "/i18n");
  a.installTranslator(&tor);
  
  QgsGridMakerPluginGui *myPluginGui=new QgsGridMakerPluginGui();
  a.setMainWidget(myPluginGui);
  myPluginGui->show();

  return a.exec();

  return EXIT_SUCCESS;
}

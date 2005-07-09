/***************************************************************************
                          qgscontexthelp.cpp
                    Display context help for a dialog
                             -------------------
    begin                : 2005-06-19
    copyright            : (C) 2005 by Gary E.Sherman
    email                : sherman at mrcc.com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
/* $Id$ */
#include <iostream>
#include <qstring.h>
#include <qdir.h>
#include <qprocess.h>
#include <qapplication.h>
#include "qgscontexthelp.h"
#include <cassert>
QgsContextHelp::QgsContextHelp(int contextId)
{
  run(contextId);
}
void QgsContextHelp::run(int contextId)
{
  // Assume minimum Qt 3.2 version and use the API to get the path
  // path to the help viewer
      QString helpPath = qApp->applicationDirPath(); 
#ifdef Q_OS_MACX
      helpPath += "/bin/qgis_help.app/Contents/MacOS";
#endif
      helpPath += "/qgis_help";
#ifdef QGISDEBUG
      std::cout << "Help path is " << helpPath.local8Bit() << std::endl; 
#endif
      QProcess *proc = new QProcess();
      proc->addArgument(helpPath);
      QString id;
      proc->addArgument(id.setNum(contextId));
#ifdef QGISDEBUG
      std::cout << "Starting help process with context " << contextId << std::endl; 
#endif
      proc->start();
}
QgsContextHelp::~QgsContextHelp()
{
}

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
QgsContextHelp::QgsContextHelp(QString &_contextId)
{
  QString contextId = _contextId;
  run(contextId);
}
void QgsContextHelp::run(QString contextId)
{
  // Assume minimum Qt 3.2 version and use the API to get the path
  // path to the help viewer
      QString helpPath = qApp->applicationDirPath() + "/qgis_help";
      std::cout << "Help path is " << helpPath << std::endl; 
      QProcess *proc = new QProcess();
      proc->addArgument(helpPath);
      proc->addArgument(contextId);
      std::cout << "Starting help process with context " << contextId << std::endl; 
      proc->start();
}
QgsContextHelp::~QgsContextHelp()
{
}

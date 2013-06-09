/***************************************************************************
     main.cpp
     --------------------------------------
    Date                 : Sun Sep 16 12:12:52 AKDT 2007
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
#include <qapplication.h>
#include "qgsspit.h"

int main( int argc, char ** argv )
{
  QApplication a( argc, argv );
  // Set up the QSettings environment must be done after qapp is created
  QCoreApplication::setOrganizationName( "QuantumGIS" );
  QCoreApplication::setOrganizationDomain( "qgis.org" );
  QCoreApplication::setApplicationName( "QGIS2" );

  QgsSpit w( 0, Qt::Window );
  w.show();

  a.connect( &a, SIGNAL( lastWindowClosed() ), &a, SLOT( quit() ) );

  return a.exec();
}

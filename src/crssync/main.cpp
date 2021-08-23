/***************************************************************************
                            crssync.cpp
                            sync srs.db with proj
                            -------------------
   begin                : May 2011
   copyright            : (C) 2011 by Juergen E. Fischer, norBIT GmbH
   email                : jef at norbit dot de
***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "qgsapplication.h"
#include "qgscoordinatereferencesystem.h"
#include "qgsconfig.h"

#include <QRegExp>
#include <QTemporaryDir>

#include <iostream>
#include <limits>

#include <cpl_error.h>

void CPL_STDCALL showError( CPLErr errClass, int errNo, const char *msg )
{
  Q_UNUSED( errClass )
  const QRegExp re( "EPSG PCS/GCS code \\d+ not found in EPSG support files.  Is this a valid\nEPSG coordinate system?" );
  if ( errNo != 6 && !re.exactMatch( msg ) )
  {
    std::cerr << msg;
  }
}

int main( int argc, char **argv )
{
  const QCoreApplication app( argc, argv );

  const QStringList args = QCoreApplication::arguments();

  bool verbose = false;

  for ( const QString &arg : args )
  {
    if ( arg == QLatin1String( "--verbose" ) )
      verbose = true;
  }

  const QTemporaryDir stemp;
  QSettings::setDefaultFormat( QSettings::IniFormat );
  QSettings::setPath( QSettings::IniFormat, QSettings::UserScope, stemp.path() );

  const QTemporaryDir temp;
  QgsApplication::init( temp.path() );

  if ( !QgsApplication::isRunningFromBuildDir() )
  {
    char *prefixPath = getenv( "QGIS_PREFIX_PATH" );
    QgsApplication::setPrefixPath( prefixPath ? prefixPath : CMAKE_INSTALL_PREFIX, TRUE );
  }

  if ( verbose )
    std::cout << "Synchronizing CRS database with GDAL/PROJ definitions." << std::endl;

  CPLPushErrorHandler( showError );

  const int res = QgsCoordinateReferenceSystem::syncDatabase();

  CPLPopErrorHandler();

  if ( res == 0 && verbose )
  {
    std::cout << "No CRS updates were necessary." << std::endl;
  }
  else if ( res > 0 && verbose )
  {
    std::cout << res << " CRSs updated." << std::endl;
  }
  else if ( res == std::numeric_limits<int>::min() )
  {
    std::cout << "CRSs synchronization not possible." << std::endl;
  }
  else if ( res < 0 )
  {
    std::cout << -res << " CRSs could not be updated." << std::endl;
  }

  QgsApplication::exitQgis();

  return 0;
}

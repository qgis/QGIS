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

#include <iostream>
#include <limits>

#include <cpl_error.h>

void CPL_STDCALL showError( CPLErr errClass, int errNo, const char *msg )
{
  Q_UNUSED( errClass );
  QRegExp re( "EPSG PCS/GCS code \\d+ not found in EPSG support files.  Is this a valid\nEPSG coordinate system?" );
  if ( errNo != 6 && !re.exactMatch( msg ) )
  {
    std::cerr << msg;
  }
}

int main( int argc, char ** argv )
{
  QgsApplication a( argc, argv, false );

  if ( !QgsApplication::isRunningFromBuildDir() )
  {
    char* prefixPath = getenv( "QGIS_PREFIX_PATH" );
    QgsApplication::setPrefixPath( prefixPath ? prefixPath : CMAKE_INSTALL_PREFIX, TRUE );
  }

  std::cout << "Synchronizing CRS database with GDAL/PROJ definitions." << std::endl;

  CPLPushErrorHandler( showError );

  int res = QgsCoordinateReferenceSystem::syncDb();

  CPLPopErrorHandler();

  if ( res == 0 )
  {
    std::cout << "No CRS updates were necessary." << std::endl;
  }
  else if ( res > 0 )
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

  exit( 0 );
}

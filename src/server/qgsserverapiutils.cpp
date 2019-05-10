/***************************************************************************
                          qgsserverapiutils.cpp

  Class defining utilities for QGIS server APIs.
  -------------------
  begin                : 2019-04-16
  copyright            : (C) 2019 by Alessandro Pasotti
  email                : elpaso at itopen dot it
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsserverapiutils.h"
#include "qgsrectangle.h"
#include "qgscoordinatereferencesystem.h"

#include <QUrl>

QgsRectangle QgsServerApiUtils::parseBbox( const QString &bbox )
{
  // TODO
  return QgsRectangle();
}

QgsCoordinateReferenceSystem QgsServerApiUtils::parseCrs( const QString &bboxCrs )
{
  QgsCoordinateReferenceSystem crs;
  // We get this:
  // http://www.opengis.net/def/crs/OGC/1.3/CRS84
  // We want this:
  // "urn:ogc:def:crs:<auth>:[<version>]:<code>"
  const auto parts { QUrl( bboxCrs ).path().split( '/' ) };
  if ( parts.count() == 5 )
  {
    crs.fromOgcWmsCrs( QStringLiteral( "urn:ogc:def:crs:<auth>:[<version>]:<code>" ).arg( parts[2], parts[3], parts[4] ) );
  }
  return crs;
}

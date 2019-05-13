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
  const auto parts { bbox.split( ',', QString::SplitBehavior::SkipEmptyParts ) };
  // Note: Z is ignored
  auto ok { true };
  if ( parts.count() >= 4 )
  {
    auto toDouble = [ & ]( const int i ) -> double
    {
      if ( ! ok )
        return 0;
      return parts[i].toDouble( &ok );
    };
    const auto rect { QgsRectangle( toDouble( 0 ), toDouble( 1 ), toDouble( 2 ), toDouble( 3 ) ) };
    if ( ok )
    {
      return rect;
    }
  }
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
  if ( parts.count() == 6 )
  {
    return crs.fromOgcWmsCrs( QStringLiteral( "urn:ogc:def:crs:%1:%2:%3" ).arg( parts[3], parts[4], parts[5] ) );
  }
  else
  {
    return crs;
  }
}

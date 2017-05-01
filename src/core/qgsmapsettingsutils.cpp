/***************************************************************************
                         qgsmapsettingsutils.cpp
                             -------------------
    begin                : May 2017
    copyright            : (C) 2017 by Mathieu Pellerin
    email                : nirvn dot asia at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsmapsettings.h"
#include "qgsmapsettingsutils.h"

#include <QString>

QString QgsMapSettingsUtils::worldFileContent( const QgsMapSettings &mapSettings )
{
  double xOrigin = mapSettings.visiblePolygon().at( 0 ).x() + ( mapSettings.mapUnitsPerPixel() / 2 );
  double yOrigin = mapSettings.visiblePolygon().at( 0 ).y() - ( mapSettings.mapUnitsPerPixel() / 2 );

  QString content;
  // Pixel XDim
  content += qgsDoubleToString( mapSettings.mapUnitsPerPixel() ) + "\r\n";
  // Rotation on y axis
  content += QString( "%1\r\n" ).arg( mapSettings.rotation() );
  // Rotation on x axis
  content += QString( "%1\r\n" ).arg( mapSettings.rotation() );
  // Pixel YDim - almost always negative
  // See https://en.wikipedia.org/wiki/World_file#cite_ref-3
  content += '-' + qgsDoubleToString( mapSettings.mapUnitsPerPixel() ) + "\r\n";
  // Origin X (center of top left cell)
  content += qgsDoubleToString( xOrigin ) + "\r\n";
  // Origin Y (center of top left cell)
  content += qgsDoubleToString( yOrigin ) + "\r\n";

  return content;
}

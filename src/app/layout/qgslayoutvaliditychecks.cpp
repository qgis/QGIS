/***************************************************************************
                              qgslayoutvaliditychecks.cpp
                              ---------------------------
    begin                : November 2018
    copyright            : (C) 2018 Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************/
/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgslayoutvaliditychecks.h"
#include "qgsvaliditycheckcontext.h"
#include "qgslayoutitemmap.h"
#include "qgslayout.h"

QList<QgsValidityCheckResult> QgsLayoutMapCrsValidityCheck::runCheck( const QgsValidityCheckContext *context, QgsFeedback * ) const
{
  QList<QgsValidityCheckResult> results;
  const QgsLayoutValidityCheckContext *layoutContext = dynamic_cast< const QgsLayoutValidityCheckContext * >( context );
  if ( !layoutContext )
    return results;

  QList< QgsLayoutItemMap * > mapItems;
  layoutContext->layout->layoutItems( mapItems );
  for ( QgsLayoutItemMap *map : qgis::as_const( mapItems ) )
  {
    if ( map->crs().authid() == QStringLiteral( "EPSG:3857" ) )
    {
      QgsValidityCheckResult res;
      res.type = QgsValidityCheckResult::Warning;
      res.title = tr( "Map projection is misleading" );
      res.detailedDescription = tr( "The projection for the map item %1 is set to <i>Web Mercator (EPSG:3857)</i> which misrepresents areas and shapes. Consider using an appropriate local projection instead." ).arg( map->displayName() );
      results.append( res );
    }
  }

  return results;
}

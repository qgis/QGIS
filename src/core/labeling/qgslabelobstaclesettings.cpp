/***************************************************************************
  qgslabelobstaclesettings.cpp
  ----------------------------
  Date                 : December 2019
  Copyright            : (C) 2019 by Nyall Dawson
  Email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgslabelobstaclesettings.h"
#include "qgspropertycollection.h"
#include "qgsexpressioncontext.h"
#include "qgspallabeling.h"
#include "qgsvariantutils.h"

void QgsLabelObstacleSettings::setObstacleGeometry( const QgsGeometry &obstacleGeom )
{
  mObstacleGeometry = obstacleGeom;
}

QgsGeometry QgsLabelObstacleSettings::obstacleGeometry() const
{
  return mObstacleGeometry;
}

void QgsLabelObstacleSettings::updateDataDefinedProperties( const QgsPropertyCollection &properties, QgsExpressionContext &context )
{
  if ( properties.isActive( QgsPalLayerSettings::ObstacleFactor ) )
  {
    context.setOriginalValueVariable( mObstacleFactor );
    QVariant exprVal = properties.value( QgsPalLayerSettings::ObstacleFactor, context );
    if ( !QgsVariantUtils::isNull( exprVal ) )
    {
      bool ok;
      double factorD = exprVal.toDouble( &ok );
      if ( ok )
      {
        factorD = std::clamp( factorD, 0.0, 10.0 );
        factorD = factorD / 5.0 + 0.0001; // convert 0 -> 10 to 0.0001 -> 2.0
        mObstacleFactor = factorD;
      }
    }
  }

}

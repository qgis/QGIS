/***************************************************************************
                        qgsgeometryengine.cpp
  -------------------------------------------------------------------
Date                 : 23.12.2015
Copyright            : (C) 2014 by Matthias Kuhn
email                : matthias@opengis.ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsgeometryengine.h"

#include "qgsmultipolygonv2.h"
#include "qgsmultilinestringv2.h"
#include "qgspolygonv2.h"

#include <QTransform>

QgsGeometry QgsGeometryEngine::explode()
{
  QgsGeometry result;
  switch ( mGeometry->wkbType() )
  {
    case QgsWKBTypes::MultiLineString:
    {
      const QgsMultiLineStringV2* multiline = static_cast<const QgsMultiLineStringV2*>( mGeometry );
      QgsMultiLineStringV2* resultMultiLine = new QgsMultiLineStringV2();

      int numGeometries = multiline->numGeometries();

      for ( int i = 0; i < numGeometries; ++i )
      {
        const QgsLineStringV2* line = static_cast<const QgsLineStringV2*>( multiline->geometryN( i ) );

        int nPoints = line->numPoints();

        QgsPointV2 firstPoint = line->pointN( 0 );

        for ( int i = 1; i < nPoints; ++i )
        {
          QgsPointV2 secondPoint = line->pointN( i );

          QgsLineStringV2* segment = new QgsLineStringV2();
          segment->setPoints( QList<QgsPointV2>() << firstPoint << secondPoint );
          firstPoint = secondPoint;

          resultMultiLine->addGeometry( segment );
        }
      }

      result.setGeometry( resultMultiLine );
      break;
    }

    case QgsWKBTypes::LineString:
    {
      const QgsLineStringV2* line = static_cast<const QgsLineStringV2*>( mGeometry );

      QgsMultiLineStringV2* multiLine = new QgsMultiLineStringV2();

      int nPoints = line->numPoints();

      QgsPointV2 firstPoint = line->pointN( 0 );

      for ( int i = 1; i < nPoints; ++i )
      {
        QgsPointV2 secondPoint = line->pointN( i );

        QgsLineStringV2* segment = new QgsLineStringV2();
        segment->setPoints( QList<QgsPointV2>() << firstPoint << secondPoint );
        firstPoint = secondPoint;

        multiLine->addGeometry( segment );
      }

      result.setGeometry( multiLine );
      break;
    }

    default:
      break;
  }

  return result;
}

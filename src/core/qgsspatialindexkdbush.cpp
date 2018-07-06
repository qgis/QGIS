/***************************************************************************
                             qgsspatialindexkdbush.cpp
                             -------------------
    begin                : July 2018
    copyright            : (C) 2018 by Nyall Dawson
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

#include "qgsspatialindexkdbush.h"
#include "kdbush.hpp"
#include "qgsfeatureiterator.h"
#include "qgsfeedback.h"
#include "qgsfeaturesource.h"

class PointXYKDBush : public kdbush::KDBush< std::pair<double, double>, QgsFeatureId >
{
  public:

    explicit PointXYKDBush( QgsFeatureIterator &fi, QgsFeedback *feedback = nullptr )
    {
      fillFromIterator( fi, feedback );
    }

    explicit PointXYKDBush( const QgsFeatureSource &source, QgsFeedback *feedback )
    {
      points.reserve( source.featureCount() );
      ids.reserve( source.featureCount() );
      QgsFeatureIterator it = source.getFeatures( QgsFeatureRequest().setSubsetOfAttributes( QgsAttributeList() ) );
      fillFromIterator( it, feedback );
    }

    void fillFromIterator( QgsFeatureIterator &fi, QgsFeedback *feedback = nullptr )
    {
      QgsFeatureId size = 0;

      QgsFeature f;
      while ( fi.nextFeature( f ) )
      {
        if ( feedback && feedback->isCanceled() )
          return;

        if ( !f.hasGeometry() )
          continue;

        if ( QgsWkbTypes::flatType( f.geometry().wkbType() ) == QgsWkbTypes::Point )
        {
          const QgsPoint *point = qgsgeometry_cast< const QgsPoint * >( f.geometry().constGet() );
          points.emplace_back( point->x(), point->y() );
        }
        else
        {
          // not a point
          continue;
        }

        ids.push_back( f.id() );
        size++;
      }

      sortKD( 0, size - 1, 0 );
    }

};

//
// QgsSpatialIndexKDBush
//

QgsSpatialIndexKDBush::QgsSpatialIndexKDBush( QgsFeatureIterator &fi, QgsFeedback *feedback )
  : mIndex( qgis::make_unique < PointXYKDBush >( fi, feedback ) )
{

}

QgsSpatialIndexKDBush::QgsSpatialIndexKDBush( const QgsFeatureSource &source, QgsFeedback *feedback )
  : mIndex( qgis::make_unique < PointXYKDBush >( source, feedback ) )
{
}

QgsSpatialIndexKDBush::~QgsSpatialIndexKDBush() = default;

QList<QgsFeatureId> QgsSpatialIndexKDBush::within( const QgsPointXY &point, double radius ) const
{
  QList<QgsFeatureId> result;
  mIndex->within( point.x(), point.y(), radius, [&result]( const QgsFeatureId id ) { result << id; } );
  return result;
}

QList<QgsFeatureId> QgsSpatialIndexKDBush::intersect( const QgsRectangle &rectangle ) const
{
  QList<QgsFeatureId> result;
  mIndex->range( rectangle.xMinimum(),
                 rectangle.yMinimum(),
                 rectangle.xMaximum(),
  rectangle.yMaximum(), [&result]( const QgsFeatureId id ) { result << id; } );
  return result;
}

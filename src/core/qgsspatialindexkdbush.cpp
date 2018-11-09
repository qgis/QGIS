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
#include "qgsfeatureiterator.h"
#include "qgsfeedback.h"
#include "qgsfeaturesource.h"
#include "qgsspatialindexkdbush_p.h"

QgsSpatialIndexKDBush::QgsSpatialIndexKDBush( QgsFeatureIterator &fi, QgsFeedback *feedback )
  : d( new QgsSpatialIndexKDBushPrivate( fi, feedback ) )
{

}

QgsSpatialIndexKDBush::QgsSpatialIndexKDBush( const QgsFeatureSource &source, QgsFeedback *feedback )
  : d( new QgsSpatialIndexKDBushPrivate( source, feedback ) )
{
}

QgsSpatialIndexKDBush::QgsSpatialIndexKDBush( const QgsSpatialIndexKDBush &other )
{
  d = other.d;
  d->ref.ref();
}

QgsSpatialIndexKDBush &QgsSpatialIndexKDBush::operator=( const QgsSpatialIndexKDBush &other )
{
  if ( !d->ref.deref() )
  {
    delete d;
  }

  d = other.d;
  d->ref.ref();
  return *this;
}

QgsSpatialIndexKDBush::~QgsSpatialIndexKDBush()
{
  if ( !d->ref.deref() )
    delete d;
}

QList<QgsSpatialIndexKDBushData> QgsSpatialIndexKDBush::within( const QgsPointXY &point, double radius ) const
{
  QList<QgsSpatialIndexKDBushData> result;
  d->index->within( point.x(), point.y(), radius, [&result]( const QgsSpatialIndexKDBushData & p ) { result << p; } );
  return result;
}

void QgsSpatialIndexKDBush::within( const QgsPointXY &point, double radius, const std::function<void( QgsSpatialIndexKDBushData )> &visitor )
{
  d->index->within( point.x(), point.y(), radius, visitor );
}

qgssize QgsSpatialIndexKDBush::size() const
{
  return d->index->size();
}

QList<QgsSpatialIndexKDBushData> QgsSpatialIndexKDBush::intersects( const QgsRectangle &rectangle ) const
{
  QList<QgsSpatialIndexKDBushData> result;
  d->index->range( rectangle.xMinimum(),
                   rectangle.yMinimum(),
                   rectangle.xMaximum(),
  rectangle.yMaximum(), [&result]( const QgsSpatialIndexKDBushData & p ) { result << p; } );
  return result;
}

void QgsSpatialIndexKDBush::intersects( const QgsRectangle &rectangle, const std::function<void( QgsSpatialIndexKDBushData )> &visitor ) const
{
  d->index->range( rectangle.xMinimum(),
                   rectangle.yMinimum(),
                   rectangle.xMaximum(),
                   rectangle.yMaximum(), visitor );
}

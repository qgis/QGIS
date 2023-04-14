/***************************************************************************
                             qgsspatialindexkdbush_p.h
                             -----------------
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

#ifndef QGSSPATIALINDEXKDBUSH_PRIVATE_H
#define QGSSPATIALINDEXKDBUSH_PRIVATE_H

#define SIP_NO_FILE

/// @cond PRIVATE

//
//  W A R N I N G
//  -------------
//
// This file is not part of the QGIS API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//

#include "qgsfeature.h"
#include "qgsspatialindexkdbushdata.h"
#include "qgsfeatureiterator.h"
#include "qgsfeedback.h"
#include "qgsfeaturesource.h"
#include <memory>
#include <QList>
#include "kdbush.hpp"
#include <functional>


class PointXYKDBush : public kdbush::KDBush< std::pair<double, double>, QgsSpatialIndexKDBushData, std::size_t >
{
  public:

    explicit PointXYKDBush( QgsFeatureIterator &fi, QgsFeedback *feedback = nullptr, const std::function< bool( const QgsFeature & ) > *callback = nullptr )
    {
      fillFromIterator( fi, feedback, callback );
    }

    explicit PointXYKDBush( const QgsFeatureSource &source, QgsFeedback *feedback )
    {
      points.reserve( source.featureCount() );
      QgsFeatureIterator it = source.getFeatures( QgsFeatureRequest().setNoAttributes() );
      fillFromIterator( it, feedback, nullptr );
    }

    void fillFromIterator( QgsFeatureIterator &fi, QgsFeedback *feedback = nullptr, const std::function< bool( const QgsFeature & ) > *callback = nullptr )
    {
      std::size_t size = 0;

      QgsFeature f;
      while ( fi.nextFeature( f ) )
      {
        if ( feedback && feedback->isCanceled() )
          return;

        if ( callback && !( *callback )( f ) )
          return;

        if ( !f.hasGeometry() )
          continue;

        if ( QgsWkbTypes::flatType( f.geometry().wkbType() ) == Qgis::WkbType::Point )
        {
          const QgsPoint *point = qgsgeometry_cast< const QgsPoint * >( f.geometry().constGet() );
          points.emplace_back( QgsSpatialIndexKDBushData( f.id(), point->x(), point->y() ) );
        }
        else
        {
          // not a point
          continue;
        }

        size++;
      }

      if ( size == 0 )
        return;

      sortKD( 0, size - 1, 0 );
    }

    std::size_t size() const
    {
      return points.size();
    }

};

class QgsSpatialIndexKDBushPrivate
{
  public:

    explicit QgsSpatialIndexKDBushPrivate( QgsFeatureIterator &fi, QgsFeedback *feedback = nullptr )
      : index( std::make_unique < PointXYKDBush >( fi, feedback ) )
    {}

    explicit QgsSpatialIndexKDBushPrivate( const QgsFeatureSource &source, QgsFeedback *feedback = nullptr )
      : index( std::make_unique < PointXYKDBush >( source, feedback ) )
    {}

    explicit QgsSpatialIndexKDBushPrivate( QgsFeatureIterator &fi, const std::function< bool( const QgsFeature & ) > &callback, QgsFeedback *feedback = nullptr )
      : index( std::make_unique < PointXYKDBush >( fi, feedback, &callback ) )
    {}

    QAtomicInt ref = 1;
    std::unique_ptr< PointXYKDBush > index;
};

/// @endcond

#endif // QGSSPATIALINDEXKDBUSH_PRIVATE_H

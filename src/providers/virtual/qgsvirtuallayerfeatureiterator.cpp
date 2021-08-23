/***************************************************************************
                   qgsvirtuallayerfeatureiterator.cpp
            Feature iterator for the virtual layer provider
begin                : Nov 2015
copyright            : (C) 2015 Hugo Mercier, Oslandia
email                : hugo dot mercier at oslandia dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsvirtuallayerfeatureiterator.h"
#include "qgsmessagelog.h"
#include "qgsgeometry.h"
#include "qgsvirtuallayerblob.h"
#include "qgsexception.h"

#include <stdexcept>

static QString quotedColumn( QString name )
{
  return "\"" + name.replace( QLatin1String( "\"" ), QLatin1String( "\"\"" ) ) + "\"";
}

QgsVirtualLayerFeatureIterator::QgsVirtualLayerFeatureIterator( QgsVirtualLayerFeatureSource *source, bool ownSource, const QgsFeatureRequest &request )
  : QgsAbstractFeatureIteratorFromSource<QgsVirtualLayerFeatureSource>( source, ownSource, request )
{

  // NOTE: this is really bad and should be removed.
  // it's only here to guard mSource->mSqlite - because if the provider is removed
  // then mSqlite will be meaningless.
  // this needs to be totally reworked so that mSqlite no longer depends on the provider
  // and can be fully encapsulated in the source
  if ( !mSource->mProvider )
  {
    close();
    return;
  }

  if ( mRequest.destinationCrs().isValid() && mRequest.destinationCrs() != mSource->mCrs )
  {
    mTransform = QgsCoordinateTransform( mSource->mCrs, mRequest.destinationCrs(), mRequest.transformContext() );
  }
  try
  {
    mFilterRect = filterRectToSourceCrs( mTransform );
  }
  catch ( QgsCsException & )
  {
    // can't reproject mFilterRect
    close();
    return;
  }

  // prepare spatial filter geometries for optimal speed
  switch ( mRequest.spatialFilterType() )
  {
    case Qgis::SpatialFilterType::NoFilter:
    case Qgis::SpatialFilterType::BoundingBox:
      break;

    case Qgis::SpatialFilterType::DistanceWithin:
      if ( !mRequest.referenceGeometry().isEmpty() )
      {
        mDistanceWithinGeom = mRequest.referenceGeometry();
        mDistanceWithinEngine.reset( QgsGeometry::createGeometryEngine( mDistanceWithinGeom.constGet() ) );
        mDistanceWithinEngine->prepareGeometry();
      }
      break;
  }

  try
  {
    const QString tableName = mSource->mTableName;

    QStringList wheres;
    QString offset;
    const QString subset = mSource->mSubset;
    if ( !subset.isEmpty() )
    {
      wheres << subset;
    }

    QVariantList binded;
    if ( !mSource->mDefinition.uid().isNull() )
    {
      // filters are only available when a column with unique id exists
      if ( mSource->mDefinition.hasDefinedGeometry() && !mFilterRect.isNull() )
      {
        const bool do_exact = request.flags() & QgsFeatureRequest::ExactIntersect;
        wheres << quotedColumn( mSource->mDefinition.geometryField() ) + " is not null";
        wheres <<  QStringLiteral( "%1Intersects(%2,BuildMbr(?,?,?,?))" )
               .arg( do_exact ? "" : "Mbr",
                     quotedColumn( mSource->mDefinition.geometryField() ) );

        binded << mFilterRect.xMinimum() << mFilterRect.yMinimum()
               << mFilterRect.xMaximum() << mFilterRect.yMaximum();
      }
      else if ( request.filterType() == QgsFeatureRequest::FilterFid )
      {
        wheres << QStringLiteral( "%1=%2" )
               .arg( quotedColumn( mSource->mDefinition.uid() ) )
               .arg( request.filterFid() );
      }
      else if ( request.filterType() == QgsFeatureRequest::FilterFids )
      {
        QString values = quotedColumn( mSource->mDefinition.uid() ) + " IN (";
        bool first = true;
        const auto constFilterFids = request.filterFids();
        for ( const QgsFeatureId v : constFilterFids )
        {
          if ( !first )
          {
            values += QLatin1Char( ',' );
          }
          first = false;
          values += QString::number( v );
        }
        values += QLatin1Char( ')' );
        wheres << values;
      }
    }
    else
    {
      if ( request.filterType() == QgsFeatureRequest::FilterFid )
      {
        if ( request.filterFid() >= 0 )
          offset = QStringLiteral( " LIMIT 1 OFFSET %1" ).arg( request.filterFid() );
        else // never return a feature if the id is negative
          offset = QStringLiteral( " LIMIT 0" );
      }
      if ( !mFilterRect.isNull() && mRequest.spatialFilterType() == Qgis::SpatialFilterType::BoundingBox
           && mRequest.flags() & QgsFeatureRequest::ExactIntersect )
      {
        // if an exact intersection is requested, prepare the geometry to intersect
        const QgsGeometry rectGeom = QgsGeometry::fromRect( mFilterRect );
        mRectEngine.reset( QgsGeometry::createGeometryEngine( rectGeom.constGet() ) );
        mRectEngine->prepareGeometry();
      }
    }

    if ( request.flags() & QgsFeatureRequest::SubsetOfAttributes )
    {
      // copy only selected fields
      const auto subsetOfAttributes = request.subsetOfAttributes();
      for ( const int idx : subsetOfAttributes )
      {
        mAttributes << idx;
      }

      // ensure that all attributes required for expression filter are being fetched
      if ( request.filterType() == QgsFeatureRequest::FilterExpression )
      {
        const QSet<int> attributeIndexes = request.filterExpression()->referencedAttributeIndexes( mSource->mFields );
        for ( const int attrIdx : attributeIndexes )
        {
          if ( !mAttributes.contains( attrIdx ) )
            mAttributes << attrIdx;
        }
      }

      // also need attributes required by order by
      if ( mRequest.flags() & QgsFeatureRequest::SubsetOfAttributes && !mRequest.orderBy().isEmpty() )
      {
        const auto usedAttributeIndices = mRequest.orderBy().usedAttributeIndices( mSource->mFields );
        for ( const int attrIdx : usedAttributeIndices )
        {
          if ( !mAttributes.contains( attrIdx ) )
            mAttributes << attrIdx;
        }
      }
    }
    else
    {
      mAttributes = mSource->mFields.allAttributesList();
    }

    QString columns;
    {
      // the first column is always the uid (or 0)
      if ( !mSource->mDefinition.uid().isNull() )
      {
        columns = quotedColumn( mSource->mDefinition.uid() );
      }
      else
      {
        if ( request.filterType() == QgsFeatureRequest::FilterFid )
        {
          columns = QString::number( request.filterFid() );
        }
        else
        {
          columns = QStringLiteral( "0" );
        }
      }
      const auto constMAttributes = mAttributes;
      for ( const int i : constMAttributes )
      {
        columns += QLatin1Char( ',' );
        const QString cname = mSource->mFields.at( i ).name().toLower();
        columns += quotedColumn( cname );
      }
    }
    // the last column is the geometry, if any
    if ( ( !( request.flags() & QgsFeatureRequest::NoGeometry )
           || ( request.filterType() == QgsFeatureRequest::FilterExpression && request.filterExpression()->needsGeometry() ) )
         && !mSource->mDefinition.geometryField().isNull() && mSource->mDefinition.geometryField() != QLatin1String( "*no*" ) )
    {
      columns += "," + quotedColumn( mSource->mDefinition.geometryField() );
    }

    mSqlQuery = "SELECT " + columns + " FROM " + tableName;
    if ( !wheres.isEmpty() )
    {
      mSqlQuery += " WHERE " + wheres.join( QLatin1String( " AND " ) );
    }

    if ( !offset.isEmpty() )
    {
      mSqlQuery += offset;
    }

    mQuery.reset( new Sqlite::Query( mSource->mSqlite, mSqlQuery ) );
    for ( const QVariant &toBind : binded )
    {
      mQuery->bind( toBind );
    }

    mFid = 0;
  }
  catch ( std::runtime_error &e )
  {
    QgsMessageLog::logMessage( e.what(), QObject::tr( "VLayer" ) );
    close();
  }
}

QgsVirtualLayerFeatureIterator::~QgsVirtualLayerFeatureIterator()
{
  close();
}

bool QgsVirtualLayerFeatureIterator::rewind()
{
  if ( mClosed )
  {
    return false;
  }

  mQuery->reset();

  return true;
}

bool QgsVirtualLayerFeatureIterator::close()
{
  if ( mClosed )
  {
    return false;
  }

  // this call is absolutely needed
  iteratorClosed();

  mClosed = true;
  return true;
}

bool QgsVirtualLayerFeatureIterator::fetchFeature( QgsFeature &feature )
{
  feature.setValid( false );

  if ( mClosed )
  {
    return false;
  }

  bool skipFeature = false;
  do
  {
    if ( mQuery->step() != SQLITE_ROW )
    {
      return false;
    }
    skipFeature = false;

    feature.setFields( mSource->mFields, /* init */ true );

    if ( mSource->mDefinition.uid().isNull() &&
         mRequest.filterType() != QgsFeatureRequest::FilterFid )
    {
      // no id column => autoincrement
      feature.setId( mFid++ );
    }
    else
    {
      // first column: uid
      feature.setId( mQuery->columnInt64( 0 ) );
    }

    const int n = mQuery->columnCount();
    int i = 0;
    const auto constMAttributes = mAttributes;
    for ( const int idx : constMAttributes )
    {
      const int type = mQuery->columnType( i + 1 );
      switch ( type )
      {
        case SQLITE_INTEGER:
          feature.setAttribute( idx, mQuery->columnInt64( i + 1 ) );
          break;
        case SQLITE_FLOAT:
          feature.setAttribute( idx, mQuery->columnDouble( i + 1 ) );
          break;
        case SQLITE_TEXT:
        default:
          feature.setAttribute( idx, mQuery->columnText( i + 1 ) );
          break;
      }
      i++;
    }
    if ( n > mAttributes.size() + 1 )
    {
      // geometry field
      const QByteArray blob( mQuery->columnBlob( n - 1 ) );
      if ( blob.size() > 0 )
      {
        feature.setGeometry( spatialiteBlobToQgsGeometry( blob.constData(), blob.size() ) );
      }
      else
      {
        feature.clearGeometry();
      }
    }

    feature.setValid( true );
    geometryToDestinationCrs( feature, mTransform );

    // if the FilterRect has not been applied on the query
    // apply it here by skipping features until they intersect
    if ( mSource->mDefinition.uid().isNull() && feature.hasGeometry() && mSource->mDefinition.hasDefinedGeometry() )
    {
      if ( !mFilterRect.isNull() )
      {
        if ( mRequest.spatialFilterType() == Qgis::SpatialFilterType::BoundingBox && mRequest.flags() & QgsFeatureRequest::ExactIntersect )
        {
          // using exact test when checking for intersection
          skipFeature = !mRectEngine->intersects( feature.geometry().constGet() );
        }
        else
        {
          // check just bounding box against rect when not using intersection
          skipFeature = !feature.geometry().boundingBox().intersects( mFilterRect );
        }
      }
    }

    if ( !skipFeature && mDistanceWithinEngine )
    {
      if ( mDistanceWithinEngine->distance( feature.geometry().constGet() ) > mRequest.distanceWithin() )
        skipFeature = true;
    }
  }
  while ( skipFeature );

  return true;
}

QgsVirtualLayerFeatureSource::QgsVirtualLayerFeatureSource( const QgsVirtualLayerProvider *p )
  : mProvider( p )
  , mDefinition( p->mDefinition )
  , mFields( p->fields() )
  , mSqlite( p->mSqlite.get() )
  , mTableName( p->mTableName )
  , mSubset( p->mSubset )
  , mCrs( p->crs() )
{
}

QgsFeatureIterator QgsVirtualLayerFeatureSource::getFeatures( const QgsFeatureRequest &request )
{
  return QgsFeatureIterator( new QgsVirtualLayerFeatureIterator( this, false, request ) );
}

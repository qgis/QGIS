/***************************************************************************
    qgsdelimitedtextfeatureiterator.cpp
    ---------------------
    begin                : Oktober 2012
    copyright            : (C) 2012 by Martin Dobias
    email                : wonder dot sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "qgsdelimitedtextfeatureiterator.h"
#include "qgsdelimitedtextprovider.h"
#include "qgsdelimitedtextfile.h"

#include "qgsexpression.h"
#include "qgsgeometry.h"
#include "qgslogger.h"
#include "qgsproject.h"
#include "qgsspatialindex.h"
#include "qgsexception.h"
#include "qgsexpressioncontextutils.h"
#include "qgsgeometryengine.h"

#include <QtAlgorithms>
#include <QTextStream>
#include <QUrlQuery>

QgsDelimitedTextFeatureIterator::QgsDelimitedTextFeatureIterator( QgsDelimitedTextFeatureSource *source, bool ownSource, const QgsFeatureRequest &request )
  : QgsAbstractFeatureIteratorFromSource<QgsDelimitedTextFeatureSource>( source, ownSource, request )
  , mTestSubset( mSource->mSubsetExpression )
{

  // Determine mode to use based on request...
  QgsDebugMsgLevel( QStringLiteral( "Setting up QgsDelimitedTextIterator" ), 4 );

  // Does the layer have geometry - will revise later to determine if we actually need to
  // load it.
  const bool hasGeometry = mSource->mGeomRep != QgsDelimitedTextProvider::GeomNone;

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

  if ( !mFilterRect.isNull() && hasGeometry )
  {
    QgsDebugMsgLevel( QStringLiteral( "Configuring for rectangle select" ), 4 );
    mTestGeometry = true;
    // Exact intersection test only applies for WKT geometries
    mTestGeometryExact = mRequest.spatialFilterType() == Qgis::SpatialFilterType::BoundingBox
                         && mRequest.flags() & Qgis::FeatureRequestFlag::ExactIntersect
                         && mSource->mGeomRep == QgsDelimitedTextProvider::GeomAsWkt;

    // If request doesn't overlap extents, then nothing to return
    if ( ! mFilterRect.intersects( mSource->mExtent.toRectangle() ) && !mTestSubset )
    {
      QgsDebugMsgLevel( QStringLiteral( "Rectangle outside layer extents - no features to return" ), 4 );
      mMode = FeatureIds;
    }
    // If the request extents include the entire layer, then revert to
    // a file scan

    else if ( mFilterRect.contains( mSource->mExtent.toRectangle() ) && !mTestSubset )
    {
      QgsDebugMsgLevel( QStringLiteral( "Rectangle contains layer extents - bypass spatial filter" ), 4 );
      mTestGeometry = false;
    }

    // If we have a spatial index then use it.  The spatial index already accounts
    // for the subset.  Also means we don't have to test geometries unless doing exact
    // intersection

    else if ( mSource->mUseSpatialIndex )
    {
      mFeatureIds = mSource->mSpatialIndex->intersects( mFilterRect );
      // Sort for efficient sequential retrieval
      std::sort( mFeatureIds.begin(), mFeatureIds.end() );
      QgsDebugMsgLevel( QStringLiteral( "Layer has spatial index - selected %1 features from index" ).arg( mFeatureIds.size() ), 4 );
      mMode = FeatureIds;
      mTestSubset = false;
      mTestGeometry = mTestGeometryExact;
    }
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

  if ( request.filterType() == Qgis::FeatureRequestFilterType::Fid )
  {
    QgsDebugMsgLevel( QStringLiteral( "Configuring for returning single id" ), 4 );
    if ( mFilterRect.isNull() || mFeatureIds.contains( request.filterFid() ) )
    {
      mFeatureIds = QList<QgsFeatureId>() << request.filterFid();
    }
    mMode = FeatureIds;
    mTestSubset = false;
  }
  // If have geometry and testing geometry then evaluate options...
  // If we don't have geometry then all records pass geometry filter.
  // CC: 2013-05-09
  // Not sure about intended relationship between filtering on geometry and
  // requesting no geometry? Have preserved current logic of ignoring spatial filter
  // if not requesting geometry.

  else

    // If we have a subset index then use it..
    if ( mMode == FileScan && mSource->mUseSubsetIndex )
    {
      QgsDebugMsgLevel( QStringLiteral( "Layer has subset index - use %1 items from subset index" ).arg( mSource->mSubsetIndex.size() ), 4 );
      mTestSubset = false;
      mMode = SubsetIndex;
    }

  // Otherwise just have to scan the file
  if ( mMode == FileScan )
  {
    QgsDebugMsgLevel( QStringLiteral( "File will be scanned for desired features" ), 4 );
  }

  // If the layer has geometry, do we really need to load it?
  // We need it if it is asked for explicitly in the request,
  // if we are testing geometry (ie spatial filter), or
  // if testing the subset expression.
  if ( hasGeometry
       && (
         !( mRequest.flags() & Qgis::FeatureRequestFlag::NoGeometry )
         || mTestGeometry
         || mDistanceWithinEngine
         || ( mTestSubset && mSource->mSubsetExpression->needsGeometry() )
         || ( request.filterType() == Qgis::FeatureRequestFilterType::Expression && request.filterExpression()->needsGeometry() )
       )
     )
  {
    mLoadGeometry = true;
  }
  else
  {
    QgsDebugMsgLevel( QStringLiteral( "Feature geometries not required" ), 4 );
    mLoadGeometry = false;
  }

  // ensure that all attributes required for expression filter are being fetched
  if ( mRequest.flags() & Qgis::FeatureRequestFlag::SubsetOfAttributes && request.filterType() == Qgis::FeatureRequestFilterType::Expression )
  {
    const QgsAttributeList attrs = request.subsetOfAttributes();
    //ensure that all fields required for filter expressions are prepared
    QSet<int> attributeIndexes = request.filterExpression()->referencedAttributeIndexes( mSource->mFields );
    attributeIndexes += qgis::listToSet( attrs );
    mRequest.setSubsetOfAttributes( qgis::setToList( attributeIndexes ) );
  }
  // also need attributes required by order by
  if ( mRequest.flags() & Qgis::FeatureRequestFlag::SubsetOfAttributes && !mRequest.orderBy().isEmpty() )
  {
    QgsAttributeList attrs = request.subsetOfAttributes();
    const auto usedAttributeIndices = mRequest.orderBy().usedAttributeIndices( mSource->mFields );
    for ( const int attrIndex : usedAttributeIndices )
    {
      if ( !attrs.contains( attrIndex ) )
        attrs << attrIndex;
    }
    mRequest.setSubsetOfAttributes( attrs );
  }

  QgsDebugMsgLevel( QStringLiteral( "Iterator is scanning file: " ) + ( mMode == FileScan ? "Yes" : "No" ), 4 );
  QgsDebugMsgLevel( QStringLiteral( "Iterator is loading geometries: " ) + ( mLoadGeometry ? "Yes" : "No" ), 4 );
  QgsDebugMsgLevel( QStringLiteral( "Iterator is testing geometries: " ) + ( mTestGeometry ? "Yes" : "No" ), 4 );
  QgsDebugMsgLevel( QStringLiteral( "Iterator is testing subset: " ) + ( mTestSubset ? "Yes" : "No" ), 4 );

  rewind();
}

QgsDelimitedTextFeatureIterator::~QgsDelimitedTextFeatureIterator()
{
  close();
}

bool QgsDelimitedTextFeatureIterator::fetchFeature( QgsFeature &feature )
{
  // before we do anything else, assume that there's something wrong with
  // the feature
  feature.setValid( false );

  if ( mClosed )
    return false;

  bool gotFeature = false;
  if ( mMode == FileScan )
  {
    gotFeature = nextFeatureInternal( feature );
  }
  else
  {
    while ( ! gotFeature )
    {
      qint64 fid = -1;
      if ( mMode == FeatureIds )
      {
        if ( mNextId < mFeatureIds.size() )
        {
          fid = mFeatureIds.at( mNextId );
        }
      }
      else if ( mNextId < mSource->mSubsetIndex.size() )
      {
        fid = mSource->mSubsetIndex.at( mNextId );
      }
      if ( fid < 0 )
        break;
      mNextId++;
      gotFeature = ( setNextFeatureId( fid ) && nextFeatureInternal( feature ) );
    }
  }

  // CC: 2013-05-08:  What is the intent of rewind/close.  The following
  // line from previous implementation means that we cannot rewind the iterator
  // after reading last record? Is this correct?  This line can be removed if
  // not.

  if ( !gotFeature )
    close();

  geometryToDestinationCrs( feature, mTransform );

  return gotFeature;
}

bool QgsDelimitedTextFeatureIterator::rewind()
{
  if ( mClosed )
    return false;

  // Skip to first data record
  if ( mMode == FileScan )
  {
    mSource->mFile->reset();
  }
  else
  {
    mNextId = 0;
  }
  return true;
}

bool QgsDelimitedTextFeatureIterator::close()
{
  if ( mClosed )
    return false;

  iteratorClosed();

  mFeatureIds = QList<QgsFeatureId>();
  mClosed = true;
  return true;
}

bool QgsDelimitedTextFeatureIterator::testSpatialFilter( const QgsPointXY &pt ) const
{
  if ( mDistanceWithinEngine )
  {
    if ( !mTransform.isShortCircuited() )
    {
      QgsFeature candidate;
      candidate.setGeometry( QgsGeometry::fromPointXY( pt ) );
      geometryToDestinationCrs( candidate, mTransform );
      return mDistanceWithinEngine->distance( candidate.geometry().constGet() ) <= mRequest.distanceWithin();
    }
    else
    {
      const QgsGeometry ptGeom( QgsGeometry::fromPointXY( pt ) );
      return mDistanceWithinEngine->distance( ptGeom.constGet() ) <= mRequest.distanceWithin();
    }
  }
  else if ( mTestGeometry )
  {
    return mFilterRect.contains( pt );
  }
  else
    return true;
}

bool QgsDelimitedTextFeatureIterator::testSpatialFilter( const QgsGeometry &geom ) const
{
  if ( mDistanceWithinEngine )
  {
    if ( !mTransform.isShortCircuited() )
    {
      QgsFeature candidate;
      candidate.setGeometry( geom );
      geometryToDestinationCrs( candidate, mTransform );
      return candidate.hasGeometry() && mDistanceWithinEngine->distance( candidate.geometry().constGet() ) <= mRequest.distanceWithin();
    }
    else
    {
      return mDistanceWithinEngine->distance( geom.constGet() ) <= mRequest.distanceWithin();
    }
  }
  else if ( mTestGeometry )
  {
    if ( mTestGeometryExact )
      return geom.intersects( mFilterRect );
    else
      return geom.boundingBox().intersects( mFilterRect );
  }
  else
    return true;
}

bool QgsDelimitedTextFeatureIterator::nextFeatureInternal( QgsFeature &feature )
{
  QStringList tokens;

  QgsDelimitedTextFile *file = mSource->mFile.get();

  // If the iterator is not scanning the file, then it will have requested a specific
  // record, so only need to load that one.

  bool first = true;
  const bool scanning = mMode == FileScan;

  while ( scanning || first )
  {
    first = false;

    // before we do anything else, assume that there's something wrong with
    // the feature.  If the provider is not currently valid, then cannot return
    // feature.

    feature.setValid( false );

    const QgsDelimitedTextFile::Status status = file->nextRecord( tokens );
    if ( status == QgsDelimitedTextFile::RecordEOF )
      break;
    if ( status != QgsDelimitedTextFile::RecordOk )
      continue;

    // We ignore empty records, such as added randomly by spreadsheets

    if ( QgsDelimitedTextProvider::recordIsEmpty( tokens ) )
      continue;

    const QgsFeatureId fid = file->recordId();

    while ( tokens.size() < mSource->mFieldCount )
      tokens.append( QString() );

    QgsGeometry geom;

    // Load the geometry if required

    if ( mLoadGeometry )
    {
      bool nullGeom = false;
      if ( mSource->mGeomRep == QgsDelimitedTextProvider::GeomAsWkt )
      {
        geom = loadGeometryWkt( tokens, nullGeom );
      }
      else if ( mSource->mGeomRep == QgsDelimitedTextProvider::GeomAsXy )
      {
        geom = loadGeometryXY( tokens, nullGeom );
      }

      if ( ( geom.isNull() && !nullGeom ) || ( nullGeom && mTestGeometry ) || ( nullGeom && mDistanceWithinEngine ) )
      {
        // if we didn't get a geom and not because it's null, or we got a null
        // geom and we are testing for intersecting geometries then ignore this
        // record
        continue;
      }
    }

    // At this point the current feature values are valid

    feature.setValid( true );
    feature.setFields( mSource->mFields ); // allow name-based attribute lookups
    feature.setId( fid );
    feature.initAttributes( mSource->mFields.count() );
    feature.setGeometry( geom );

    // If we are testing subset expression, then need all attributes just in case.
    // Could be more sophisticated, but probably not worth it!

    if ( ! mTestSubset && ( mRequest.flags() & Qgis::FeatureRequestFlag::SubsetOfAttributes ) )
    {
      const QgsAttributeList attrs = mRequest.subsetOfAttributes();
      for ( QgsAttributeList::const_iterator i = attrs.constBegin(); i != attrs.constEnd(); ++i )
      {
        const int fieldIdx = *i;
        fetchAttribute( feature, fieldIdx, tokens );
      }
    }
    else
    {
      for ( int idx = 0; idx < mSource->mFields.count(); ++idx )
        fetchAttribute( feature, idx, tokens );
    }

    // If the iterator hasn't already filtered out the subset, then do it now

    if ( mTestSubset )
    {
      mSource->mExpressionContext.setFeature( feature );
      const QVariant isOk = mSource->mSubsetExpression->evaluate( &mSource->mExpressionContext );
      if ( mSource->mSubsetExpression->hasEvalError() )
        continue;
      if ( ! isOk.toBool() )
        continue;
    }

    // We have a good record, so return
    return true;

  }

  return false;
}

bool QgsDelimitedTextFeatureIterator::setNextFeatureId( qint64 fid )
{
  return mSource->mFile->setNextRecordId( ( long ) fid );
}

QgsGeometry QgsDelimitedTextFeatureIterator::loadGeometryWkt( const QStringList &tokens, bool &isNull )
{
  QgsGeometry geom;
  QString sWkt = tokens[mSource->mWktFieldIndex];
  if ( sWkt.isEmpty() )
  {
    isNull = true;
    return QgsGeometry();
  }

  isNull = false;
  geom = QgsDelimitedTextProvider::geomFromWkt( sWkt, mSource->mWktHasPrefix );

  if ( !geom.isNull() && geom.type() != mSource->mGeometryType )
  {
    geom = QgsGeometry();
  }
  if ( !geom.isNull() && ! testSpatialFilter( geom ) )
  {
    geom = QgsGeometry();
  }
  return geom;
}

QgsGeometry QgsDelimitedTextFeatureIterator::loadGeometryXY( const QStringList &tokens, bool &isNull )
{
  QString sX = tokens[mSource->mXFieldIndex];
  QString sY = tokens[mSource->mYFieldIndex];
  if ( sX.isEmpty() && sY.isEmpty() )
  {
    isNull = true;
    return QgsGeometry();
  }

  isNull = false;
  QgsPoint *pt = new QgsPoint();
  const bool ok = QgsDelimitedTextProvider::pointFromXY( sX, sY, *pt, mSource->mDecimalPoint, mSource->mXyDms );

  QString sZ, sM;
  if ( mSource->mZFieldIndex > -1 )
    sZ = tokens[mSource->mZFieldIndex];
  if ( mSource->mMFieldIndex > -1 )
    sM = tokens[mSource->mMFieldIndex];

  if ( !sZ.isEmpty() || !sM.isEmpty() )
  {
    QgsDelimitedTextProvider::appendZM( sZ, sM, *pt, mSource->mDecimalPoint );
  }

  if ( ok && testSpatialFilter( *pt ) )
  {
    return QgsGeometry( pt );
  }
  return QgsGeometry();
}

void QgsDelimitedTextFeatureIterator::fetchAttribute( QgsFeature &feature, int fieldIdx, const QStringList &tokens )
{
  if ( fieldIdx < 0 || fieldIdx >= mSource->attributeColumns.count() )
    return;
  const int column = mSource->attributeColumns.at( fieldIdx );
  if ( column < 0 || column >= tokens.count() )
    return;
  const QString &value = tokens[column];
  QVariant val;
  switch ( mSource->mFields.at( fieldIdx ).type() )
  {
    case QMetaType::Type::Bool:
    {
      Q_ASSERT( mSource->mFieldBooleanLiterals.contains( fieldIdx ) );
      if ( value.compare( mSource->mFieldBooleanLiterals[ fieldIdx ].first, Qt::CaseSensitivity::CaseInsensitive ) == 0 )
      {
        val = true;
      }
      else if ( value.compare( mSource->mFieldBooleanLiterals[ fieldIdx ].second, Qt::CaseSensitivity::CaseInsensitive ) == 0 )
      {
        val = false;
      }
      else
      {
        val = QgsVariantUtils::createNullVariant( QMetaType::Type::Bool );
      }
      break;
    }
    case QMetaType::Type::Int:
    {
      int ivalue = 0;
      bool ok = false;
      if ( ! value.isEmpty() )
        ivalue = value.toInt( &ok );
      if ( ok )
        val = QVariant( ivalue );
      else
        val = QgsVariantUtils::createNullVariant( mSource->mFields.at( fieldIdx ).type() );
      break;
    }
    case QMetaType::Type::LongLong:
    {
      if ( ! value.isEmpty() )
      {
        bool ok;
        val = value.toLongLong( &ok );
        if ( ! ok )
        {
          val = QgsVariantUtils::createNullVariant( mSource->mFields.at( fieldIdx ).type() );
        }
      }
      else
      {
        val = QgsVariantUtils::createNullVariant( mSource->mFields.at( fieldIdx ).type() );
      }
      break;
    }
    case QMetaType::Type::Double:
    {
      double dvalue = 0.0;
      bool ok = false;
      if ( ! value.isEmpty() )
      {
        if ( mSource->mDecimalPoint.isEmpty() )
        {
          dvalue = value.toDouble( &ok );
        }
        else
        {
          dvalue = QString( value ).replace( mSource->mDecimalPoint, QLatin1String( "." ) ).toDouble( &ok );
        }
      }
      if ( ok )
      {
        val = QVariant( dvalue );
      }
      else
      {
        val = QgsVariantUtils::createNullVariant( mSource->mFields.at( fieldIdx ).type() );
      }
      break;
    }
    case QMetaType::Type::QDateTime:
    {
      val = QVariant( QDateTime::fromString( value, Qt::ISODate ) );
      break;
    }
    case QMetaType::Type::QDate:
    {
      val = QVariant( QDate::fromString( value, Qt::ISODate ) );
      break;
    }
    case QMetaType::Type::QTime:
    {
      val = QVariant( QTime::fromString( value ) );
      break;
    }
    default:
      val = QVariant( value );
      break;
  }
  feature.setAttribute( fieldIdx, val );
}

// ------------

QgsDelimitedTextFeatureSource::QgsDelimitedTextFeatureSource( const QgsDelimitedTextProvider *p )
  : mGeomRep( p->mGeomRep )
  , mSubsetExpression( p->mSubsetExpression ? new QgsExpression( *p->mSubsetExpression ) : nullptr )
  , mExtent( p->mExtent )
  , mUseSpatialIndex( p->mUseSpatialIndex )
  , mSpatialIndex( p->mSpatialIndex ? new QgsSpatialIndex( *p->mSpatialIndex ) : nullptr )
  , mUseSubsetIndex( p->mUseSubsetIndex )
  , mSubsetIndex( p->mSubsetIndex )
  , mFile( nullptr )
  , mFields( p->attributeFields )
  , mFieldCount( p->mFieldCount )
  , mXFieldIndex( p->mXFieldIndex )
  , mYFieldIndex( p->mYFieldIndex )
  , mZFieldIndex( p->mZFieldIndex )
  , mMFieldIndex( p->mMFieldIndex )
  , mWktFieldIndex( p->mWktFieldIndex )
  , mWktHasPrefix( p->mWktHasPrefix )
  , mGeometryType( p->mGeometryType )
  , mDecimalPoint( p->mDecimalPoint )
  , mXyDms( p->mXyDms )
  , attributeColumns( p->attributeColumns )
  , mCrs( p->mCrs )
  , mFieldBooleanLiterals( p->mFieldBooleanLiterals )
{
  QUrl url = p->mFile->url();

  // make sure watcher not created when using iterator (e.g. for rendering, see issue #15558)
  QUrlQuery query( url );
  if ( query.hasQueryItem( QStringLiteral( "watchFile" ) ) )
  {
    query.removeQueryItem( QStringLiteral( "watchFile" ) );
  }
  url.setQuery( query );

  mFile.reset( new QgsDelimitedTextFile() );
  mFile->setFromUrl( url );

  mExpressionContext << QgsExpressionContextUtils::globalScope()
                     << QgsExpressionContextUtils::projectScope( QgsProject::instance() );
  mExpressionContext.setFields( mFields );
}

QgsFeatureIterator QgsDelimitedTextFeatureSource::getFeatures( const QgsFeatureRequest &request )
{
  return QgsFeatureIterator( new QgsDelimitedTextFeatureIterator( this, false, request ) );
}

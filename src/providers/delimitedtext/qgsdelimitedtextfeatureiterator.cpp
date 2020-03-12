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
#include "qgsmessagelog.h"
#include "qgsproject.h"
#include "qgsspatialindex.h"
#include "qgsexception.h"
#include "qgsexpressioncontextutils.h"

#include <QtAlgorithms>
#include <QTextStream>

QgsDelimitedTextFeatureIterator::QgsDelimitedTextFeatureIterator( QgsDelimitedTextFeatureSource *source, bool ownSource, const QgsFeatureRequest &request )
  : QgsAbstractFeatureIteratorFromSource<QgsDelimitedTextFeatureSource>( source, ownSource, request )
  , mTestSubset( mSource->mSubsetExpression )
{

  // Determine mode to use based on request...
  QgsDebugMsg( QStringLiteral( "Setting up QgsDelimitedTextIterator" ) );

  // Does the layer have geometry - will revise later to determine if we actually need to
  // load it.
  bool hasGeometry = mSource->mGeomRep != QgsDelimitedTextProvider::GeomNone;

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
    QgsDebugMsg( QStringLiteral( "Configuring for rectangle select" ) );
    mTestGeometry = true;
    // Exact intersection test only applies for WKT geometries
    mTestGeometryExact = mRequest.flags() & QgsFeatureRequest::ExactIntersect
                         && mSource->mGeomRep == QgsDelimitedTextProvider::GeomAsWkt;

    // If request doesn't overlap extents, then nothing to return
    if ( ! mFilterRect.intersects( mSource->mExtent ) && !mTestSubset )
    {
      QgsDebugMsg( QStringLiteral( "Rectangle outside layer extents - no features to return" ) );
      mMode = FeatureIds;
    }
    // If the request extents include the entire layer, then revert to
    // a file scan

    else if ( mFilterRect.contains( mSource->mExtent ) && !mTestSubset )
    {
      QgsDebugMsg( QStringLiteral( "Rectangle contains layer extents - bypass spatial filter" ) );
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
      QgsDebugMsg( QStringLiteral( "Layer has spatial index - selected %1 features from index" ).arg( mFeatureIds.size() ) );
      mMode = FeatureIds;
      mTestSubset = false;
      mTestGeometry = mTestGeometryExact;
    }
  }

  if ( request.filterType() == QgsFeatureRequest::FilterFid )
  {
    QgsDebugMsg( QStringLiteral( "Configuring for returning single id" ) );
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
      QgsDebugMsg( QStringLiteral( "Layer has subset index - use %1 items from subset index" ).arg( mSource->mSubsetIndex.size() ) );
      mTestSubset = false;
      mMode = SubsetIndex;
    }

  // Otherwise just have to scan the file
  if ( mMode == FileScan )
  {
    QgsDebugMsg( QStringLiteral( "File will be scanned for desired features" ) );
  }

  // If the layer has geometry, do we really need to load it?
  // We need it if it is asked for explicitly in the request,
  // if we are testing geometry (ie spatial filter), or
  // if testing the subset expression.
  if ( hasGeometry
       && (
         !( mRequest.flags() & QgsFeatureRequest::NoGeometry )
         || mTestGeometry
         || ( mTestSubset && mSource->mSubsetExpression->needsGeometry() )
         || ( request.filterType() == QgsFeatureRequest::FilterExpression && request.filterExpression()->needsGeometry() )
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
  if ( mRequest.flags() & QgsFeatureRequest::SubsetOfAttributes && request.filterType() == QgsFeatureRequest::FilterExpression )
  {
    QgsAttributeList attrs = request.subsetOfAttributes();
    //ensure that all fields required for filter expressions are prepared
    QSet<int> attributeIndexes = request.filterExpression()->referencedAttributeIndexes( mSource->mFields );
    attributeIndexes += attrs.toSet();
    mRequest.setSubsetOfAttributes( attributeIndexes.toList() );
  }
  // also need attributes required by order by
  if ( mRequest.flags() & QgsFeatureRequest::SubsetOfAttributes && !mRequest.orderBy().isEmpty() )
  {
    QgsAttributeList attrs = request.subsetOfAttributes();
    const auto usedAttributeIndices = mRequest.orderBy().usedAttributeIndices( mSource->mFields );
    for ( int attrIndex : usedAttributeIndices )
    {
      if ( !attrs.contains( attrIndex ) )
        attrs << attrIndex;
    }
    mRequest.setSubsetOfAttributes( attrs );
  }

  QgsDebugMsg( QStringLiteral( "Iterator is scanning file: " ) + ( mMode == FileScan ? "Yes" : "No" ) );
  QgsDebugMsg( QStringLiteral( "Iterator is loading geometries: " ) + ( mLoadGeometry ? "Yes" : "No" ) );
  QgsDebugMsg( QStringLiteral( "Iterator is testing geometries: " ) + ( mTestGeometry ? "Yes" : "No" ) );
  QgsDebugMsg( QStringLiteral( "Iterator is testing subset: " ) + ( mTestSubset ? "Yes" : "No" ) );

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
      if ( fid < 0 ) break;
      mNextId++;
      gotFeature = ( setNextFeatureId( fid ) && nextFeatureInternal( feature ) );
    }
  }

  // CC: 2013-05-08:  What is the intent of rewind/close.  The following
  // line from previous implementation means that we cannot rewind the iterator
  // after reading last record? Is this correct?  This line can be removed if
  // not.

  if ( ! gotFeature ) close();

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

/**
 * Check to see if the point is within the selection rectangle
 */
bool QgsDelimitedTextFeatureIterator::wantGeometry( const QgsPointXY &pt ) const
{
  if ( ! mTestGeometry ) return true;
  return mFilterRect.contains( pt );
}

/**
 * Check to see if the geometry is within the selection rectangle
 */
bool QgsDelimitedTextFeatureIterator::wantGeometry( const QgsGeometry &geom ) const
{
  if ( ! mTestGeometry ) return true;

  if ( mTestGeometryExact )
    return geom.intersects( mFilterRect );
  else
    return geom.boundingBox().intersects( mFilterRect );
}






bool QgsDelimitedTextFeatureIterator::nextFeatureInternal( QgsFeature &feature )
{
  QStringList tokens;

  QgsDelimitedTextFile *file = mSource->mFile.get();

  // If the iterator is not scanning the file, then it will have requested a specific
  // record, so only need to load that one.

  bool first = true;
  bool scanning = mMode == FileScan;

  while ( scanning || first )
  {
    first = false;

    // before we do anything else, assume that there's something wrong with
    // the feature.  If the provider is not currently valid, then cannot return
    // feature.

    feature.setValid( false );

    QgsDelimitedTextFile::Status status = file->nextRecord( tokens );
    if ( status == QgsDelimitedTextFile::RecordEOF ) break;
    if ( status != QgsDelimitedTextFile::RecordOk ) continue;

    // We ignore empty records, such as added randomly by spreadsheets

    if ( QgsDelimitedTextProvider::recordIsEmpty( tokens ) ) continue;

    QgsFeatureId fid = file->recordId();

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

      if ( ( geom.isNull() && !nullGeom ) || ( nullGeom && mTestGeometry ) )
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

    if ( ! mTestSubset && ( mRequest.flags() & QgsFeatureRequest::SubsetOfAttributes ) )
    {
      QgsAttributeList attrs = mRequest.subsetOfAttributes();
      for ( QgsAttributeList::const_iterator i = attrs.constBegin(); i != attrs.constEnd(); ++i )
      {
        int fieldIdx = *i;
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
      QVariant isOk = mSource->mSubsetExpression->evaluate( &mSource->mExpressionContext );
      if ( mSource->mSubsetExpression->hasEvalError() ) continue;
      if ( ! isOk.toBool() ) continue;
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
  if ( !geom.isNull() && ! wantGeometry( geom ) )
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
  bool ok = QgsDelimitedTextProvider::pointFromXY( sX, sY, *pt, mSource->mDecimalPoint, mSource->mXyDms );

  QString sZ, sM;
  if ( mSource->mZFieldIndex > -1 )
    sZ = tokens[mSource->mZFieldIndex];
  if ( mSource->mMFieldIndex > -1 )
    sM = tokens[mSource->mMFieldIndex];

  if ( !sZ.isEmpty() || !sM.isEmpty() )
  {
    QgsDelimitedTextProvider::appendZM( sZ, sM, *pt, mSource->mDecimalPoint );
  }

  if ( ok && wantGeometry( *pt ) )
  {
    return QgsGeometry( pt );
  }
  return QgsGeometry();
}



void QgsDelimitedTextFeatureIterator::fetchAttribute( QgsFeature &feature, int fieldIdx, const QStringList &tokens )
{
  if ( fieldIdx < 0 || fieldIdx >= mSource->attributeColumns.count() ) return;
  int column = mSource->attributeColumns.at( fieldIdx );
  if ( column < 0 || column >= tokens.count() ) return;
  const QString &value = tokens[column];
  QVariant val;
  switch ( mSource->mFields.at( fieldIdx ).type() )
  {
    case QVariant::Int:
    {
      int ivalue = 0;
      bool ok = false;
      if ( ! value.isEmpty() ) ivalue = value.toInt( &ok );
      if ( ok )
        val = QVariant( ivalue );
      else
        val = QVariant( mSource->mFields.at( fieldIdx ).type() );
      break;
    }
    case QVariant::Double:
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
        val = QVariant( mSource->mFields.at( fieldIdx ).type() );
      }
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

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
#include "qgsspatialindex.h"

#include <QtAlgorithms>
#include <QTextStream>

QgsDelimitedTextFeatureIterator::QgsDelimitedTextFeatureIterator( QgsDelimitedTextFeatureSource* source, bool ownSource, const QgsFeatureRequest& request )
    : QgsAbstractFeatureIteratorFromSource( source, ownSource, request )
{

  // Determine mode to use based on request...
  QgsDebugMsg( "Setting up QgsDelimitedTextIterator" );

  // Does the layer have geometry - will revise later to determine if we actually need to
  // load it.
  bool hasGeometry = mSource->mGeomRep != QgsDelimitedTextProvider::GeomNone;

  // Does the layer have an explicit or implicit subset (implicit subset is if we have geometry which can
  // be invalid)

  mTestSubset = mSource->mSubsetExpression;
  mTestGeometry = false;

  mMode = FileScan;
  if ( request.filterType() == QgsFeatureRequest::FilterFid )
  {
    QgsDebugMsg( "Configuring for returning single id" );
    mFeatureIds.append( request.filterFid() );
    mMode = FeatureIds;
    mTestSubset = false;
  }
  // If have geometry and testing geometry then evaluate options...
  // If we don't have geometry then all records pass geometry filter.
  // CC: 2013-05-09
  // Not sure about intended relationship between filtering on geometry and
  // requesting no geometry? Have preserved current logic of ignoring spatial filter
  // if not requesting geometry.

  else if ( request.filterType() == QgsFeatureRequest::FilterRect && hasGeometry )
  {
    QgsDebugMsg( "Configuring for rectangle select" );
    mTestGeometry = true;
    // Exact intersection test only applies for WKT geometries
    mTestGeometryExact = mRequest.flags() & QgsFeatureRequest::ExactIntersect
                         && mSource->mGeomRep == QgsDelimitedTextProvider::GeomAsWkt;

    QgsRectangle rect = request.filterRect();

    // If request doesn't overlap extents, then nothing to return
    if ( ! rect.intersects( mSource->mExtent ) )
    {
      QgsDebugMsg( "Rectangle outside layer extents - no features to return" );
      mMode = FeatureIds;
    }
    // If the request extents include the entire layer, then revert to
    // a file scan

    else if ( rect.contains( mSource->mExtent ) )
    {
      QgsDebugMsg( "Rectangle contains layer extents - bypass spatial filter" );
      mTestGeometry = false;
    }
    // If we have a spatial index then use it.  The spatial index already accounts
    // for the subset.  Also means we don't have to test geometries unless doing exact
    // intersection

    else if ( mSource->mUseSpatialIndex )
    {
      mFeatureIds = mSource->mSpatialIndex->intersects( rect );
      // Sort for efficient sequential retrieval
      qSort( mFeatureIds.begin(), mFeatureIds.end() );
      QgsDebugMsg( QString( "Layer has spatial index - selected %1 features from index" ).arg( mFeatureIds.size() ) );
      mMode = FeatureIds;
      mTestSubset = false;
      mTestGeometry = mTestGeometryExact;
    }
  }

  // If we have a subset index then use it..
  if ( mMode == FileScan && mSource->mUseSubsetIndex )
  {
    QgsDebugMsg( QString( "Layer has subset index - use %1 items from subset index" ).arg( mSource->mSubsetIndex.size() ) );
    mTestSubset = false;
    mMode = SubsetIndex;
  }

  // Otherwise just have to scan the file
  if ( mMode == FileScan )
  {
    QgsDebugMsg( "File will be scanned for desired features" );
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
       )
     )
  {
    mLoadGeometry = true;
  }
  else
  {
    QgsDebugMsgLevel( "Feature geometries not required", 4 );
    mLoadGeometry = false;
  }

  QgsDebugMsg( QString( "Iterator is scanning file: " ) + ( mMode == FileScan ? "Yes" : "No" ) );
  QgsDebugMsg( QString( "Iterator is loading geometries: " ) + ( mLoadGeometry ? "Yes" : "No" ) );
  QgsDebugMsg( QString( "Iterator is testing geometries: " ) + ( mTestGeometry ? "Yes" : "No" ) );
  QgsDebugMsg( QString( "Iterator is testing subset: " ) + ( mTestSubset ? "Yes" : "No" ) );

  rewind();
}

QgsDelimitedTextFeatureIterator::~QgsDelimitedTextFeatureIterator()
{
  close();
}

bool QgsDelimitedTextFeatureIterator::fetchFeature( QgsFeature& feature )
{
  // before we do anything else, assume that there's something wrong with
  // the feature
  feature.setValid( false );

  if ( mClosed )
    return false;

  bool gotFeature = false;
  if ( mMode == FileScan )
  {
    gotFeature =  nextFeatureInternal( feature );
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
          fid = mFeatureIds[mNextId];
        }
      }
      else if ( mNextId < mSource->mSubsetIndex.size() )
      {
        fid = mSource->mSubsetIndex[mNextId];
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
bool QgsDelimitedTextFeatureIterator::wantGeometry( const QgsPoint &pt ) const
{
  if ( ! mTestGeometry ) return true;
  return mRequest.filterRect().contains( pt );
}

/**
 * Check to see if the geometry is within the selection rectangle
 */
bool QgsDelimitedTextFeatureIterator::wantGeometry( QgsGeometry *geom ) const
{
  if ( ! mTestGeometry ) return true;

  if ( mTestGeometryExact )
    return geom->intersects( mRequest.filterRect() );
  else
    return geom->boundingBox().intersects( mRequest.filterRect() );
}






bool QgsDelimitedTextFeatureIterator::nextFeatureInternal( QgsFeature& feature )
{
  QStringList tokens;

  QgsDelimitedTextFile *file = mSource->mFile;

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
      tokens.append( QString::null );

    QgsGeometry *geom = 0;

    // Load the geometry if required

    if ( mLoadGeometry )
    {
      if ( mSource->mGeomRep == QgsDelimitedTextProvider::GeomAsWkt )
      {
        geom = loadGeometryWkt( tokens );
      }
      else if ( mSource->mGeomRep == QgsDelimitedTextProvider::GeomAsXy )
      {
        geom = loadGeometryXY( tokens );
      }

      if ( ! geom )
      {
        continue;
      }
    }

    // At this point the current feature values are valid

    feature.setValid( true );
    feature.setFields( &mSource->mFields ); // allow name-based attribute lookups
    feature.setFeatureId( fid );
    feature.initAttributes( mSource->mFields.count() );

    if ( geom )
      feature.setGeometry( geom );

    // If we are testing subset expression, then need all attributes just in case.
    // Could be more sophisticated, but probably not worth it!

    if ( ! mTestSubset && ( mRequest.flags() & QgsFeatureRequest::SubsetOfAttributes ) )
    {
      const QgsAttributeList& attrs = mRequest.subsetOfAttributes();
      for ( QgsAttributeList::const_iterator i = attrs.begin(); i != attrs.end(); ++i )
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
      QVariant isOk = mSource->mSubsetExpression->evaluate( &feature );
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
  return mSource->mFile->setNextRecordId(( long ) fid );
}



QgsGeometry* QgsDelimitedTextFeatureIterator::loadGeometryWkt( const QStringList& tokens )
{
  QgsGeometry* geom = 0;
  QString sWkt = tokens[mSource->mWktFieldIndex];

  geom = QgsDelimitedTextProvider::geomFromWkt( sWkt, mSource->mWktHasPrefix, mSource->mWktHasZM );

  if ( geom && geom->type() != mSource->mGeometryType )
  {
    delete geom;
    geom = 0;
  }
  if ( geom && ! wantGeometry( geom ) )
  {
    delete geom;
    geom = 0;
  }
  return geom;
}

QgsGeometry* QgsDelimitedTextFeatureIterator::loadGeometryXY( const QStringList& tokens )
{
  QString sX = tokens[mSource->mXFieldIndex];
  QString sY = tokens[mSource->mYFieldIndex];
  QgsPoint pt;
  bool ok = QgsDelimitedTextProvider::pointFromXY( sX, sY, pt, mSource->mDecimalPoint, mSource->mXyDms );

  if ( ok && wantGeometry( pt ) )
  {
    return QgsGeometry::fromPoint( pt );
  }
  return 0;
}



void QgsDelimitedTextFeatureIterator::fetchAttribute( QgsFeature& feature, int fieldIdx, const QStringList& tokens )
{
  if ( fieldIdx < 0 || fieldIdx >= mSource->attributeColumns.count() ) return;
  int column = mSource->attributeColumns[fieldIdx];
  if ( column < 0 || column >= tokens.count() ) return;
  const QString &value = tokens[column];
  QVariant val;
  switch ( mSource->mFields[fieldIdx].type() )
  {
    case QVariant::Int:
    {
      int ivalue = 0;
      bool ok = false;
      if ( ! value.isEmpty() ) ivalue = value.toInt( &ok );
      if ( ok )
        val = QVariant( ivalue );
      else
        val = QVariant( mSource->mFields[fieldIdx].type() );
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
          dvalue = QString( value ).replace( mSource->mDecimalPoint, "." ).toDouble( &ok );
        }
      }
      if ( ok )
      {
        val = QVariant( dvalue );
      }
      else
      {
        val = QVariant( mSource->mFields[fieldIdx].type() );
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

QgsDelimitedTextFeatureSource::QgsDelimitedTextFeatureSource( const QgsDelimitedTextProvider* p )
    : mGeomRep( p->mGeomRep )
    , mSubsetExpression( p->mSubsetExpression )
    , mExtent( p->mExtent )
    , mUseSpatialIndex( p->mUseSpatialIndex )
    , mSpatialIndex( p->mSpatialIndex ? new QgsSpatialIndex( *p->mSpatialIndex ) : 0 )
    , mUseSubsetIndex( p->mUseSubsetIndex )
    , mSubsetIndex( p->mSubsetIndex )
    , mFile( 0 )
    , mFields( p->attributeFields )
    , mFieldCount( p->mFieldCount )
    , mXFieldIndex( p->mXFieldIndex )
    , mYFieldIndex( p->mYFieldIndex )
    , mWktFieldIndex( p->mWktFieldIndex )
    , mWktHasZM( p->mWktHasZM )
    , mWktHasPrefix( p->mWktHasPrefix )
    , mGeometryType( p->mGeometryType )
    , mDecimalPoint( p->mDecimalPoint )
    , mXyDms( p->mXyDms )
    , attributeColumns( p->attributeColumns )
{
  mFile = new QgsDelimitedTextFile();
  mFile->setFromUrl( p->mFile->url() );
}

QgsDelimitedTextFeatureSource::~QgsDelimitedTextFeatureSource()
{
  delete mSpatialIndex;
  delete mFile;
}

QgsFeatureIterator QgsDelimitedTextFeatureSource::getFeatures( const QgsFeatureRequest& request )
{
  return QgsFeatureIterator( new QgsDelimitedTextFeatureIterator( this, false, request ) );
}

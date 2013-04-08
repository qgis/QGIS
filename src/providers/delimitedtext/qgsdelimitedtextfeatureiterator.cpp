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

#include "qgsgeometry.h"
#include "qgsmessagelog.h"

#include <QTextStream>

QgsDelimitedTextFeatureIterator::QgsDelimitedTextFeatureIterator( QgsDelimitedTextProvider* p, const QgsFeatureRequest& request )
    : QgsAbstractFeatureIterator( request ), P( p )
{
  // make sure that only one iterator is active
  if ( P->mActiveIterator )
  {
    QgsMessageLog::logMessage( QObject::tr( "Already active iterator on this provider was closed." ), QObject::tr( "Delimited text" ) );
    P->mActiveIterator->close();
  }
  P->mActiveIterator = this;

  rewind();
}

QgsDelimitedTextFeatureIterator::~QgsDelimitedTextFeatureIterator()
{
  close();
}

bool QgsDelimitedTextFeatureIterator::nextFeature( QgsFeature& feature )
{
  // before we do anything else, assume that there's something wrong with
  // the feature
  feature.setValid( false );

  if ( mClosed )
    return false;

  QStringList tokens;
  while ( true )
  {
    QgsDelimitedTextFile::Status status = P->mFile->nextRecord( tokens );
    if( status == QgsDelimitedTextFile::RecordEOF ) break;
    if( status != QgsDelimitedTextFile::RecordOk ) continue;

    while ( tokens.size() < P->mFieldCount )
      tokens.append( QString::null );

    QgsGeometry *geom = 0;

    if ( P->mWktFieldIndex >= 0 )
    {
      geom = loadGeometryWkt( tokens );
    }
    else if ( P->mXFieldIndex >= 0 && P->mYFieldIndex >= 0 )
    {
      geom = loadGeometryXY( tokens );
    }

    if ( !geom && P->mWkbType != QGis::WKBNoGeometry )
    {
      // Already dealt with invalid lines in provider - no need to repeat
      // P->mInvalidLines << line;
      continue;
    }

    mFid++;

    // At this point the current feature values are valid

    feature.setValid( true );
    feature.setFields( &P->attributeFields ); // allow name-based attribute lookups
    feature.setFeatureId( mFid );
    feature.initAttributes( P->attributeFields.count() );

    if ( geom )
      feature.setGeometry( geom );

    if ( mRequest.flags() & QgsFeatureRequest::SubsetOfAttributes )
    {
      const QgsAttributeList& attrs = mRequest.subsetOfAttributes();
      for ( QgsAttributeList::const_iterator i = attrs.begin(); i != attrs.end(); ++i )
      {
        int fieldIdx = *i;
        if ( fieldIdx < 0 || fieldIdx >= P->attributeColumns.count() )
          continue; // ignore non-existant fields
        fetchAttribute( feature, fieldIdx, tokens );
      }
    }
    else
    {
      for ( int idx = 0; idx < P->attributeFields.count(); ++idx )
        fetchAttribute( feature, idx, tokens );
    }

    // We have a good line, so return
    return true;

  } // !mStream->atEnd()

  // End of the file. If there are any lines that couldn't be
  // loaded, display them now.
  // P->handleInvalidLines();

  close();
  return false;
}

bool QgsDelimitedTextFeatureIterator::rewind()
{
  if ( mClosed )
    return false;

  // Reset feature id to 0
  mFid = 0;
  // Skip to first data record
  P->resetStream();
  return true;
}

bool QgsDelimitedTextFeatureIterator::close()
{
  if ( mClosed )
    return false;

  // tell provider that this iterator is not active anymore
  P->mActiveIterator = 0;
  mClosed = true;
  return true;
}


QgsGeometry* QgsDelimitedTextFeatureIterator::loadGeometryWkt( const QStringList& tokens )
{
  QgsGeometry* geom = 0;
  try
  {
    QString sWkt = tokens[P->mWktFieldIndex];
    // Remove Z and M coordinates if present, as currently fromWkt doesn't
    // support these.
    if ( P->mWktHasZM )
    {
      sWkt.remove( P->mWktZMRegexp ).replace( P->mWktCrdRegexp, "\\1" );
    }

    geom = QgsGeometry::fromWkt( sWkt );
  }
  catch ( ... )
  {
    geom = 0;
  }

  if ( geom && geom->wkbType() != P->mWkbType )
  {
    delete geom;
    geom = 0;
  }
  if ( geom && !boundsCheck( geom ) )
  {
    delete geom;
    geom = 0;
  }
  return geom;
}


QgsGeometry* QgsDelimitedTextFeatureIterator::loadGeometryXY( const QStringList& tokens )
{
  QString sX = tokens[P->mXFieldIndex];
  QString sY = tokens[P->mYFieldIndex];

  if ( !P->mDecimalPoint.isEmpty() )
  {
    sX.replace( P->mDecimalPoint, "." );
    sY.replace( P->mDecimalPoint, "." );
  }

  bool xOk, yOk;
  double x = sX.toDouble( &xOk );
  double y = sY.toDouble( &yOk );
  if ( xOk && yOk )
  {
    if ( boundsCheck( x, y ) )
    {
      return QgsGeometry::fromPoint( QgsPoint( x, y ) );
    }
  }
  return 0;
}



/**
 * Check to see if the point is within the selection rectangle
 */
bool QgsDelimitedTextFeatureIterator::boundsCheck( double x, double y )
{
  // no selection rectangle or geometry => always in the bounds
  if ( mRequest.filterType() != QgsFeatureRequest::FilterRect || ( mRequest.flags() & QgsFeatureRequest::NoGeometry ) )
    return true;

  return mRequest.filterRect().contains( QgsPoint( x, y ) );
}

/**
 * Check to see if the geometry is within the selection rectangle
 */
bool QgsDelimitedTextFeatureIterator::boundsCheck( QgsGeometry *geom )
{
  // no selection rectangle or geometry => always in the bounds
  if ( mRequest.filterType() != QgsFeatureRequest::FilterRect || ( mRequest.flags() & QgsFeatureRequest::NoGeometry ) )
    return true;

  if ( mRequest.flags() & QgsFeatureRequest::ExactIntersect )
    return geom->intersects( mRequest.filterRect() );
  else
    return geom->boundingBox().intersects( mRequest.filterRect() );
}


void QgsDelimitedTextFeatureIterator::fetchAttribute( QgsFeature& feature, int fieldIdx, const QStringList& tokens )
{
  const QString &value = tokens[P->attributeColumns[fieldIdx]];
  QVariant val;
  switch ( P->attributeFields[fieldIdx].type() )
  {
    case QVariant::Int:
      if ( !value.isEmpty() )
        val = QVariant( value );
      else
        val = QVariant( P->attributeFields[fieldIdx].type() );
      break;
    case QVariant::Double:
      if ( !value.isEmpty() )
        val = QVariant( value.toDouble() );
      else
        val = QVariant( P->attributeFields[fieldIdx].type() );
      break;
    default:
      val = QVariant( value );
      break;
  }
  feature.setAttribute( fieldIdx, val );
}

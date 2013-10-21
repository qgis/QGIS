/***************************************************************************
    qgsogrfeatureiterator.cpp
    ---------------------
    begin                : Juli 2012
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
#include "qgsogrfeatureiterator.h"

#include "qgsogrprovider.h"

#include "qgsapplication.h"
#include "qgsgeometry.h"
#include "qgslogger.h"
#include "qgsmessagelog.h"

#include <QTextCodec>
#include <QFile>

// using from provider:
// - setRelevantFields(), mRelevantFieldsForNextFeature
// - ogrLayer
// - mFetchFeaturesWithoutGeom
// - mAttributeFields
// - mEncoding


QgsOgrFeatureIterator::QgsOgrFeatureIterator( QgsOgrProvider* p, const QgsFeatureRequest& request )
    : QgsAbstractFeatureIterator( request )
    , P( p )
    , ogrDataSource( 0 )
    , ogrLayer( 0 )
    , mSubsetStringSet( false )
{
  mFeatureFetched = false;

  ogrDataSource = OGROpen( TO8F( P->filePath() ), false, NULL );

  if ( P->layerName().isNull() )
  {
    ogrLayer = OGR_DS_GetLayer( ogrDataSource, P->layerIndex() );
  }
  else
  {
    ogrLayer = OGR_DS_GetLayerByName( ogrDataSource, TO8( p->layerName() ) );
  }

  if ( !P->subsetString().isEmpty() )
  {
    ogrLayer = P->setSubsetString( ogrLayer, ogrDataSource );
    mSubsetStringSet = true;
  }

  ensureRelevantFields();

  // spatial query to select features
  if ( mRequest.filterType() == QgsFeatureRequest::FilterRect )
  {
    OGRGeometryH filter = 0;
    QString wktExtent = QString( "POLYGON((%1))" ).arg( mRequest.filterRect().asPolygon() );
    QByteArray ba = wktExtent.toAscii();
    const char *wktText = ba;

    OGR_G_CreateFromWkt(( char ** )&wktText, NULL, &filter );
    QgsDebugMsg( "Setting spatial filter using " + wktExtent );
    OGR_L_SetSpatialFilter( ogrLayer, filter );
    OGR_G_DestroyGeometry( filter );
  }
  else
  {
    OGR_L_SetSpatialFilter( ogrLayer, 0 );
  }

  //start with first feature
  rewind();
}

QgsOgrFeatureIterator::~QgsOgrFeatureIterator()
{
  close();
}

void QgsOgrFeatureIterator::ensureRelevantFields()
{
  mFetchGeometry = ( mRequest.filterType() == QgsFeatureRequest::FilterRect ) || !( mRequest.flags() & QgsFeatureRequest::NoGeometry );
  QgsAttributeList attrs = ( mRequest.flags() & QgsFeatureRequest::SubsetOfAttributes ) ? mRequest.subsetOfAttributes() : P->attributeIndexes();
  P->setRelevantFields( mFetchGeometry, attrs );
  P->mRelevantFieldsForNextFeature = true;
}


bool QgsOgrFeatureIterator::fetchFeature( QgsFeature& feature )
{
  feature.setValid( false );

  if ( mClosed )
    return false;

  if ( !P->mRelevantFieldsForNextFeature )
    ensureRelevantFields();

  if ( mRequest.filterType() == QgsFeatureRequest::FilterFid )
  {
    OGRFeatureH fet = OGR_L_GetFeature( ogrLayer, FID_TO_NUMBER( mRequest.filterFid() ) );
    if ( !fet )
    {
      close();
      return false;
    }

    if ( readFeature( fet, feature ) )
      OGR_F_Destroy( fet );

    feature.setValid( true );
    close(); // the feature has been read: we have finished here
    return true;
  }

  OGRFeatureH fet;

  while (( fet = OGR_L_GetNextFeature( ogrLayer ) ) )
  {
    if ( !readFeature( fet, feature ) )
      continue;

    // we have a feature, end this cycle
    feature.setValid( true );
    OGR_F_Destroy( fet );
    return true;

  } // while

  QgsDebugMsg( "Feature is null" );

  close();
  return false;
}


bool QgsOgrFeatureIterator::rewind()
{
  if ( mClosed )
    return false;

  OGR_L_ResetReading( ogrLayer );

  return true;
}


bool QgsOgrFeatureIterator::close()
{
  if ( mClosed )
    return false;

  P->mActiveIterators.remove( this );

  if ( mSubsetStringSet )
  {
    OGR_DS_ReleaseResultSet( ogrDataSource, ogrLayer );
  }

  OGR_DS_Destroy( ogrDataSource );

  mClosed = true;
  ogrDataSource = 0;
  return true;
}


void QgsOgrFeatureIterator::getFeatureAttribute( OGRFeatureH ogrFet, QgsFeature & f, int attindex )
{
  OGRFieldDefnH fldDef = OGR_F_GetFieldDefnRef( ogrFet, attindex );

  if ( ! fldDef )
  {
    QgsDebugMsg( "ogrFet->GetFieldDefnRef(attindex) returns NULL" );
    return;
  }

  QVariant value;

  if ( OGR_F_IsFieldSet( ogrFet, attindex ) )
  {
    switch ( P->mAttributeFields[attindex].type() )
    {
      case QVariant::String: value = QVariant( P->mEncoding->toUnicode( OGR_F_GetFieldAsString( ogrFet, attindex ) ) ); break;
      case QVariant::Int: value = QVariant( OGR_F_GetFieldAsInteger( ogrFet, attindex ) ); break;
      case QVariant::Double: value = QVariant( OGR_F_GetFieldAsDouble( ogrFet, attindex ) ); break;
      case QVariant::Date:
      case QVariant::DateTime:
      {
        int year, month, day, hour, minute, second, tzf;

        OGR_F_GetFieldAsDateTime( ogrFet, attindex, &year, &month, &day, &hour, &minute, &second, &tzf );
        if ( P->mAttributeFields[attindex].type() == QVariant::Date )
          value = QDate( year, month, day );
        else
          value = QDateTime( QDate( year, month, day ), QTime( hour, minute, second ) );
      }
      break;
      default:
        assert( 0 && "unsupported field type" );
    }
  }
  else
  {
    value = QVariant( QString::null );
  }

  f.setAttribute( attindex, value );
}


bool QgsOgrFeatureIterator::readFeature( OGRFeatureH fet, QgsFeature& feature )
{
  feature.setFeatureId( OGR_F_GetFID( fet ) );
  feature.initAttributes( P->fields().count() );
  feature.setFields( &P->mAttributeFields ); // allow name-based attribute lookups

  bool useIntersect = mRequest.flags() & QgsFeatureRequest::ExactIntersect;
  bool geometryTypeFilter = P->mOgrGeometryTypeFilter != wkbUnknown;
  if ( mFetchGeometry || useIntersect || geometryTypeFilter )
  {
    OGRGeometryH geom = OGR_F_GetGeometryRef( fet );

    if ( geom )
    {
      notifyReadedFeature( fet, geom, feature );

      // get the wkb representation
      unsigned char *wkb = new unsigned char[OGR_G_WkbSize( geom )];
      OGR_G_ExportToWkb( geom, ( OGRwkbByteOrder ) QgsApplication::endian(), wkb );

      feature.setGeometryAndOwnership( wkb, OGR_G_WkbSize( geom ) );

      notifyLoadedFeature( fet, feature );
    }
    if (( useIntersect && ( !feature.geometry() || !feature.geometry()->intersects( mRequest.filterRect() ) ) )
        || ( geometryTypeFilter && ( !feature.geometry() || QgsOgrProvider::ogrWkbSingleFlatten(( OGRwkbGeometryType )feature.geometry()->wkbType() ) != P->mOgrGeometryTypeFilter ) ) )
    {
      OGR_F_Destroy( fet );
      return false;
    }
  }

  if ( !mFetchGeometry )
  {
    feature.setGeometry( 0 );
  }

  // fetch attributes
  if ( mRequest.flags() & QgsFeatureRequest::SubsetOfAttributes )
  {
    const QgsAttributeList& attrs = mRequest.subsetOfAttributes();
    for ( QgsAttributeList::const_iterator it = attrs.begin(); it != attrs.end(); ++it )
    {
      getFeatureAttribute( fet, feature, *it );
    }
  }
  else
  {
    // all attributes
    for ( int idx = 0; idx < P->mAttributeFields.count(); ++idx )
    {
      getFeatureAttribute( fet, feature, idx );
    }
  }

  return true;
}

//! notify the OGRFeatureH was readed of the data provider
void QgsOgrFeatureIterator::notifyReadedFeature( OGRFeatureH fet, OGRGeometryH geom, QgsFeature& feature )
{
}
//! notify the OGRFeatureH was loaded to the QgsFeature object
void QgsOgrFeatureIterator::notifyLoadedFeature( OGRFeatureH fet, QgsFeature& feature )
{
}

/***************************************************************************
    MapToPixel simplification classes
    ----------------------
    begin                : October 2013
    copyright            : (C) 2013 by Alvaro Huarte
    email                : http://wiki.osgeo.org/wiki/Alvaro_Huarte

 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <ogr_geometry.h>

//! Provides a specialized FeatureIterator for enable map2pixel simplification of the geometries
QgsOgrSimplifiedFeatureIterator::QgsOgrSimplifiedFeatureIterator( QgsOgrProvider* p, const QgsFeatureRequest& request ) : QgsOgrFeatureIterator( p, request )
{
  mPointBufferCount = 512;
  mPointBufferPtr = (OGRRawPoint*)malloc( mPointBufferCount * sizeof(OGRRawPoint) );
}
QgsOgrSimplifiedFeatureIterator::~QgsOgrSimplifiedFeatureIterator( )
{
  if ( mPointBufferPtr )
  {
    OGRFree( mPointBufferPtr );
    mPointBufferPtr = NULL;
  }
}

//! Returns a point buffer of the specified size
OGRRawPoint* QgsOgrSimplifiedFeatureIterator::mallocPoints( int numPoints )
{
  if ( mPointBufferPtr && mPointBufferCount < numPoints )
  {
    OGRFree( mPointBufferPtr );
    mPointBufferPtr = NULL;
  }
  if ( mPointBufferPtr==NULL )
  {
    mPointBufferCount = numPoints;
    mPointBufferPtr = (OGRRawPoint*)malloc( mPointBufferCount * sizeof(OGRRawPoint) );
  }
  return mPointBufferPtr;
}

//! Simplify the OGR-geometry using the specified tolerance
bool QgsOgrSimplifiedFeatureIterator::simplifyOgrGeometry( const QgsFeatureRequest& request, OGRGeometry* geometry, bool isaLinearRing )
{
  OGRwkbGeometryType wkbGeometryType = wkbFlatten( geometry->getGeometryType() );

  // Simplify the geometry rewriting temporally its WKB-stream for saving calloc's.
  if (wkbGeometryType==wkbLineString)
  {
    OGRLineString* lineString = (OGRLineString*)geometry;

    OGREnvelope env;
    geometry->getEnvelope(&env );
    QgsRectangle envelope( env.MinX, env.MinY, env.MaxX, env.MaxY );

    // Can replace the geometry by its BBOX ?
    if ( request.canbeGeneralizedByMapBoundingBox( envelope ) )
    {
      OGRRawPoint* points = NULL;
      int numPoints = 0;

      double x1 = envelope.xMinimum();
      double y1 = envelope.yMinimum();
      double x2 = envelope.xMaximum();
      double y2 = envelope.yMaximum();

      if ( isaLinearRing )
      {
        numPoints = 5;
        points = mallocPoints( numPoints );
        points[0].x = x1; points[0].y = y1;
        points[1].x = x2; points[1].y = y1;
        points[2].x = x2; points[2].y = y2;
        points[3].x = x1; points[3].y = y2;
        points[4].x = x1; points[4].y = y1;
      }
      else
      {
        numPoints = 2;
        points = mallocPoints( numPoints );
        points[0].x = x1; points[0].y = y1;
        points[1].x = x2; points[1].y = y2;
      }
      lineString->setPoints( numPoints, points );
      lineString->flattenTo2D();
      return true;
    }
    else
    {
      QGis::GeometryType geometryType = isaLinearRing ? QGis::Polygon : QGis::Line;

      int numPoints = lineString->getNumPoints();
      int numSimplifiedPoints = 0;

      OGRRawPoint* points = mallocPoints( numPoints );
      double* xptr = (double*)points;
      double* yptr = xptr+1; 
      lineString->getPoints( points );

      if ( request.simplifyGeometry( geometryType, envelope, xptr, 16, yptr, 16, numPoints, numSimplifiedPoints ) )
      {
        lineString->setPoints(numSimplifiedPoints, points);
        lineString->flattenTo2D();
      }
      return numSimplifiedPoints!=numPoints;
    }
  }
  else
  if (wkbGeometryType==wkbPolygon)
  {
    OGRPolygon* polygon = (OGRPolygon*)geometry;
    bool result = simplifyOgrGeometry( request, polygon->getExteriorRing(), true );

    for (int i = 0, numInteriorRings = polygon->getNumInteriorRings(); i < numInteriorRings; ++i)
    {
      result |= simplifyOgrGeometry( request, polygon->getInteriorRing(i), true );
    }
    if ( result ) polygon->flattenTo2D();
    return result;
  }
  else
  if (wkbGeometryType==wkbMultiLineString || wkbGeometryType==wkbMultiPolygon)
  {
    OGRGeometryCollection* collection = (OGRGeometryCollection*)geometry;
    bool result = false;

    for (int i = 0, numGeometries = collection->getNumGeometries(); i < numGeometries; ++i)
    {
      result |= simplifyOgrGeometry( request, collection->getGeometryRef(i), wkbGeometryType==wkbMultiPolygon );
    }
    if ( result ) collection->flattenTo2D();
    return result;
  }
  return false;
}

//! notify the OGRFeatureH was readed of the data provider
void QgsOgrSimplifiedFeatureIterator::notifyReadedFeature( OGRFeatureH fet, OGRGeometryH geom, QgsFeature& feature )
{
  if ( mRequest.flags() & QgsFeatureRequest::SimplifyGeometries )
  {
    OGRwkbGeometryType wkbType = QgsOgrProvider::ogrWkbSingleFlatten( OGR_G_GetGeometryType(geom) );

    if (wkbType==wkbLineString || wkbType==wkbPolygon)
    {
      simplifyOgrGeometry( mRequest, (OGRGeometry*)geom, wkbType==wkbPolygon );
    }
  }
  QgsOgrFeatureIterator::notifyReadedFeature( fet, geom, feature );
}

/***************************************************************************/

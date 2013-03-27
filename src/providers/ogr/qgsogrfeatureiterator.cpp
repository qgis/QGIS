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

// using from provider:
// - setRelevantFields(), mRelevantFieldsForNextFeature
// - ogrLayer
// - mFetchFeaturesWithoutGeom
// - mAttributeFields
// - mEncoding


QgsOgrFeatureIterator::QgsOgrFeatureIterator( QgsOgrProvider* p, const QgsFeatureRequest& request )
    : QgsAbstractFeatureIterator( request ), P( p )
{
  // make sure that only one iterator is active
  if ( P->mActiveIterator )
  {
    QgsMessageLog::logMessage( QObject::tr( "Already active iterator on this provider was closed." ), QObject::tr( "OGR" ) );
    P->mActiveIterator->close();
  }
  P->mActiveIterator = this;

  mFeatureFetched = false;

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
    OGR_L_SetSpatialFilter( P->ogrLayer, filter );
    OGR_G_DestroyGeometry( filter );
  }
  else
  {
    OGR_L_SetSpatialFilter( P->ogrLayer, 0 );
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
  bool needGeom = ( mRequest.filterType() == QgsFeatureRequest::FilterRect ) || !( mRequest.flags() & QgsFeatureRequest::NoGeometry );
  QgsAttributeList attrs = ( mRequest.flags() & QgsFeatureRequest::SubsetOfAttributes ) ? mRequest.subsetOfAttributes() : P->attributeIndexes();
  P->setRelevantFields( needGeom, attrs );
  P->mRelevantFieldsForNextFeature = true;
}


bool QgsOgrFeatureIterator::nextFeature( QgsFeature& feature )
{
  feature.setValid( false );

  if ( mClosed )
    return false;

  if ( !P->mRelevantFieldsForNextFeature )
    ensureRelevantFields();

  if ( mRequest.filterType() == QgsFeatureRequest::FilterFid )
  {
    OGRFeatureH fet = OGR_L_GetFeature( P->ogrLayer, FID_TO_NUMBER( mRequest.filterFid() ) );
    if ( !fet )
    {
      close();
      return false;
    }

    // skip features without geometry
    if ( !OGR_F_GetGeometryRef( fet ) && !P->mFetchFeaturesWithoutGeom )
    {
      OGR_F_Destroy( fet );
      close();
      return false;
    }

    readFeature( fet, feature );

    feature.setValid( true );
    close(); // the feature has been read: we have finished here
    return true;
  }

  OGRFeatureH fet;

  while (( fet = OGR_L_GetNextFeature( P->ogrLayer ) ) )
  {
    // skip features without geometry
    if ( !P->mFetchFeaturesWithoutGeom && !OGR_F_GetGeometryRef( fet ) )
    {
      OGR_F_Destroy( fet );
      continue;
    }

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

  OGR_L_ResetReading( P->ogrLayer );

  return true;
}


bool QgsOgrFeatureIterator::close()
{
  if ( mClosed )
    return false;

  // tell provider that this iterator is not active anymore
  P->mActiveIterator = 0;

  mClosed = true;
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
        //case QVariant::DateTime: value = QVariant(QDateTime::fromString(str)); break;
      default: assert( NULL && "unsupported field type" );
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

  bool fetchGeom    = !( mRequest.flags() & QgsFeatureRequest::NoGeometry );
  bool useIntersect = mRequest.flags() & QgsFeatureRequest::ExactIntersect;
  if ( fetchGeom || useIntersect )
  {
    OGRGeometryH geom = OGR_F_GetGeometryRef( fet );

    if ( geom )
    {
      // get the wkb representation
      unsigned char *wkb = new unsigned char[OGR_G_WkbSize( geom )];
      OGR_G_ExportToWkb( geom, ( OGRwkbByteOrder ) QgsApplication::endian(), wkb );

      feature.setGeometryAndOwnership( wkb, OGR_G_WkbSize( geom ) );
    }

    if ( useIntersect && ( !feature.geometry() || !feature.geometry()->intersects( mRequest.filterRect() ) ) )
    {
      OGR_F_Destroy( fet );
      return false;
    }
  }

  if ( !fetchGeom )
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

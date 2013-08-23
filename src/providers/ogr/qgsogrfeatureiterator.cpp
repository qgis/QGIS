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
    QByteArray layerName = OGR_FD_GetName( OGR_L_GetLayerDefn( ogrLayer ) );
    if ( P->ogrDriverName == "ODBC" ) //the odbc driver does not like schema names for subset
    {
      QString layerNameString = P->mEncoding->toUnicode( layerName );
      int dotIndex = layerNameString.indexOf( "." );
      if ( dotIndex > 1 )
      {
        QString modifiedLayerName = layerNameString.right( layerNameString.size() - dotIndex - 1 );
        layerName = P->mEncoding->fromUnicode( modifiedLayerName );
      }
    }
    QByteArray sql = "SELECT * FROM " + P->quotedIdentifier( layerName );
    sql += " WHERE " + P->textEncoding()->fromUnicode( P->subsetString() );
    QgsDebugMsg( QString( "SQL: %1" ).arg( P->textEncoding()->toUnicode( sql ) ) );
    ogrLayer = OGR_DS_ExecuteSQL( ogrDataSource, sql.constData(), NULL, NULL );
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

  bool fetchGeom    = !( mRequest.flags() & QgsFeatureRequest::NoGeometry );
  bool useIntersect = mRequest.flags() & QgsFeatureRequest::ExactIntersect;
  bool geometryTypeFilter = P->mOgrGeometryTypeFilter != wkbUnknown;
  if ( fetchGeom || useIntersect || geometryTypeFilter )
  {
    OGRGeometryH geom = OGR_F_GetGeometryRef( fet );

    if ( geom )
    {
      // get the wkb representation
      unsigned char *wkb = new unsigned char[OGR_G_WkbSize( geom )];
      OGR_G_ExportToWkb( geom, ( OGRwkbByteOrder ) QgsApplication::endian(), wkb );

      feature.setGeometryAndOwnership( wkb, OGR_G_WkbSize( geom ) );
    }
    if (( useIntersect && ( !feature.geometry() || !feature.geometry()->intersects( mRequest.filterRect() ) ) )
        || ( geometryTypeFilter && ( !feature.geometry() || QgsOgrProvider::ogrWkbSingleFlatten(( OGRwkbGeometryType )feature.geometry()->wkbType() ) != P->mOgrGeometryTypeFilter ) ) )
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

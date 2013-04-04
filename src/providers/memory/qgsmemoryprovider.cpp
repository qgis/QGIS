/***************************************************************************
    memoryprovider.cpp - provider with storage in memory
    ------------------
    begin                : June 2008
    copyright            : (C) 2008 by Martin Dobias
    email                : wonder dot sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsmemoryprovider.h"
#include "qgsmemoryfeatureiterator.h"

#include "qgsfeature.h"
#include "qgsfield.h"
#include "qgsgeometry.h"
#include "qgslogger.h"
#include "qgsspatialindex.h"
#include "qgscoordinatereferencesystem.h"

#include <QUrl>
#include <QRegExp>


static const QString TEXT_PROVIDER_KEY = "memory";
static const QString TEXT_PROVIDER_DESCRIPTION = "Memory provider";

QgsMemoryProvider::QgsMemoryProvider( QString uri )
    : QgsVectorDataProvider( uri )
    , mSpatialIndex( NULL )
    , mActiveIterator( 0 )
{
  // Initialize the geometry with the uri to support old style uri's
  // (ie, just 'point', 'line', 'polygon')
  QUrl url = QUrl::fromEncoded( uri.toUtf8() );
  QString geometry;
  if ( url.hasQueryItem( "geometry" ) )
  {
    geometry = url.queryItemValue( "geometry" );
  }
  else
  {
    geometry = url.path();
  }

  geometry = geometry.toLower();
  if ( geometry == "point" )
    mWkbType = QGis::WKBPoint;
  else if ( geometry == "linestring" )
    mWkbType = QGis::WKBLineString;
  else if ( geometry == "polygon" )
    mWkbType = QGis::WKBPolygon;
  else if ( geometry == "multipoint" )
    mWkbType = QGis::WKBMultiPoint;
  else if ( geometry == "multilinestring" )
    mWkbType = QGis::WKBMultiLineString;
  else if ( geometry == "multipolygon" )
    mWkbType = QGis::WKBMultiPolygon;
  else
    mWkbType = QGis::WKBUnknown;

  if ( url.hasQueryItem( "crs" ) )
  {
    QString crsDef = url.queryItemValue( "crs" );
    mCrs.createFromString( crsDef );
  }

  mNextFeatureId = 1;

  mNativeTypes
  << QgsVectorDataProvider::NativeType( tr( "Whole number (integer)" ), "integer", QVariant::Int, 0, 10 )
  << QgsVectorDataProvider::NativeType( tr( "Decimal number (real)" ), "double", QVariant::Double, 0, 20, 0, 5 )
  << QgsVectorDataProvider::NativeType( tr( "Text (string)" ), "string", QVariant::String, 0, 255 )
  ;

  if ( url.hasQueryItem( "field" ) )
  {
    QList<QgsField> attributes;
    QRegExp reFieldDef( "\\:"
                        "(int|integer|real|double|string)" // type
                        "(?:\\((\\d+)"                // length
                        "(?:\\,(\\d+))?"                // precision
                        "\\))?"
                        "$", Qt::CaseInsensitive );
    QStringList fields = url.allQueryItemValues( "field" );
    for ( int i = 0; i < fields.size(); i++ )
    {
      QString name = fields.at( i );
      QVariant::Type type = QVariant::String;
      QString typeName( "string" );
      int length = 255;
      int precision = 0;

      int pos = reFieldDef.indexIn( name );
      if ( pos >= 0 )
      {
        name = name.mid( 0, pos );
        typeName = reFieldDef.cap( 1 ).toLower();
        if ( typeName == "int" || typeName == "integer" )
        {
          type = QVariant::Int;
          typeName = "integer";
          length = 10;
        }
        else if ( typeName == "real" || typeName == "double" )
        {
          type = QVariant::Double;
          typeName = "double";
          length = 20;
          precision = 5;
        }

        if ( reFieldDef.cap( 2 ) != "" )
        {
          length = reFieldDef.cap( 2 ).toInt();
        }
        if ( reFieldDef.cap( 3 ) != "" )
        {
          precision = reFieldDef.cap( 3 ).toInt();
        }
      }
      if ( name != "" )
        attributes.append( QgsField( name, type, typeName, length, precision ) );
    }
    addAttributes( attributes );
  }

  if ( url.hasQueryItem( "index" ) && url.queryItemValue( "index" ) == "yes" )
  {
    createSpatialIndex();
  }

}

QgsMemoryProvider::~QgsMemoryProvider()
{
  if ( mActiveIterator )
    mActiveIterator->close();

  delete mSpatialIndex;
}

QString QgsMemoryProvider::dataSourceUri() const
{
  QUrl uri( "memory" );
  QString geometry;
  switch ( mWkbType )
  {
    case QGis::WKBPoint :
      geometry = "Point";
      break;
    case QGis::WKBLineString :
      geometry = "LineString";
      break;
    case QGis::WKBPolygon :
      geometry = "Polygon";
      break;
    case QGis::WKBMultiPoint :
      geometry = "MultiPoint";
      break;
    case QGis::WKBMultiLineString :
      geometry = "MultiLineString";
      break;
    case QGis::WKBMultiPolygon :
      geometry = "MultiPolygon";
      break;
    default:
      geometry = "";
      break;
  }
  uri.addQueryItem( "geometry", geometry );

  if ( mCrs.isValid() )
  {
    QString crsDef( "" );
    QString authid = mCrs.authid();
    if ( authid.startsWith( "EPSG:" ) )
    {
      crsDef = authid;
    }
    else
    {
      int srid = mCrs.postgisSrid();
      if ( srid )
      {
        crsDef = QString( "postgis:%1" ).arg( srid );
      }
      else
      {
        crsDef = QString( "wkt:%1" ).arg( mCrs.toWkt() );
      }
    }
    uri.addQueryItem( "crs", crsDef );
  }
  if ( mSpatialIndex )
  {
    uri.addQueryItem( "index", "yes" );
  }

  QgsAttributeList attrs = const_cast<QgsMemoryProvider *>( this )->attributeIndexes();
  for ( int i = 0; i < attrs.size(); i++ )
  {
    QgsField field = mFields[attrs[i]];
    QString fieldDef = field.name();
    fieldDef.append( QString( ":%2(%3,%4)" ).arg( field.typeName() ).arg( field.length() ).arg( field.precision() ) );
    uri.addQueryItem( "field", fieldDef );
  }

  return QString( uri.toEncoded() );

}

QString QgsMemoryProvider::storageType() const
{
  return "Memory storage";
}

QgsFeatureIterator QgsMemoryProvider::getFeatures( const QgsFeatureRequest& request )
{
  return QgsFeatureIterator( new QgsMemoryFeatureIterator( this, request ) );
}


QgsRectangle QgsMemoryProvider::extent()
{
  return mExtent;
}

QGis::WkbType QgsMemoryProvider::geometryType() const
{
  return mWkbType;
}

long QgsMemoryProvider::featureCount() const
{
  return mFeatures.count();
}

const QgsFields & QgsMemoryProvider::fields() const
{
  return mFields;
}

bool QgsMemoryProvider::isValid()
{
  return ( mWkbType != QGis::WKBUnknown );
}

QgsCoordinateReferenceSystem QgsMemoryProvider::crs()
{
  // TODO: make provider projection-aware
  return mCrs; // return default CRS
}


bool QgsMemoryProvider::addFeatures( QgsFeatureList & flist )
{
  // TODO: sanity checks of fields and geometries
  for ( QgsFeatureList::iterator it = flist.begin(); it != flist.end(); ++it )
  {
    mFeatures[mNextFeatureId] = *it;
    QgsFeature& newfeat = mFeatures[mNextFeatureId];
    newfeat.setFeatureId( mNextFeatureId );
    it->setFeatureId( mNextFeatureId );

    // update spatial index
    if ( mSpatialIndex )
      mSpatialIndex->insertFeature( newfeat );

    mNextFeatureId++;
  }

  updateExtent();

  return true;
}

bool QgsMemoryProvider::deleteFeatures( const QgsFeatureIds & id )
{
  for ( QgsFeatureIds::const_iterator it = id.begin(); it != id.end(); ++it )
  {
    QgsFeatureMap::iterator fit = mFeatures.find( *it );

    // check whether such feature exists
    if ( fit == mFeatures.end() )
      continue;

    // update spatial index
    if ( mSpatialIndex )
      mSpatialIndex->deleteFeature( *fit );

    mFeatures.erase( fit );
  }

  updateExtent();

  return true;
}

bool QgsMemoryProvider::addAttributes( const QList<QgsField> &attributes )
{
  for ( QList<QgsField>::const_iterator it = attributes.begin(); it != attributes.end(); ++it )
  {
    switch ( it->type() )
    {
      case QVariant::Int:
      case QVariant::Double:
      case QVariant::String:
        break;
      default:
        QgsDebugMsg( "Field type not supported: " + it->typeName() );
        continue;
    }

    // add new field as a last one
    mFields.append( *it );

    for ( QgsFeatureMap::iterator fit = mFeatures.begin(); fit != mFeatures.end(); ++fit )
    {
      QgsFeature& f = fit.value();
      f.attributes().append( QVariant() );
    }
  }
  return true;
}

bool QgsMemoryProvider::deleteAttributes( const QgsAttributeIds& attributes )
{
  QList<int> attrIdx = attributes.toList();
  qSort( attrIdx.begin(), attrIdx.end(), qGreater<int>() );

  // delete attributes one-by-one with decreasing index
  for ( QList<int>::const_iterator it = attrIdx.constBegin(); it != attrIdx.constEnd(); ++it )
  {
    int idx = *it;
    mFields.remove( idx );

    for ( QgsFeatureMap::iterator fit = mFeatures.begin(); fit != mFeatures.end(); ++fit )
    {
      QgsFeature& f = fit.value();
      f.attributes().remove( idx );
    }
  }
  return true;
}

bool QgsMemoryProvider::changeAttributeValues( const QgsChangedAttributesMap & attr_map )
{
  for ( QgsChangedAttributesMap::const_iterator it = attr_map.begin(); it != attr_map.end(); ++it )
  {
    QgsFeatureMap::iterator fit = mFeatures.find( it.key() );
    if ( fit == mFeatures.end() )
      continue;

    const QgsAttributeMap& attrs = it.value();
    for ( QgsAttributeMap::const_iterator it2 = attrs.begin(); it2 != attrs.end(); ++it2 )
      fit->setAttribute( it2.key(), it2.value() );
  }
  return true;
}

bool QgsMemoryProvider::changeGeometryValues( QgsGeometryMap & geometry_map )
{
  for ( QgsGeometryMap::const_iterator it = geometry_map.begin(); it != geometry_map.end(); ++it )
  {
    QgsFeatureMap::iterator fit = mFeatures.find( it.key() );
    if ( fit == mFeatures.end() )
      continue;

    // update spatial index
    if ( mSpatialIndex )
      mSpatialIndex->deleteFeature( *fit );

    fit->setGeometry( it.value() );

    // update spatial index
    if ( mSpatialIndex )
      mSpatialIndex->insertFeature( *fit );
  }

  updateExtent();

  return true;
}

bool QgsMemoryProvider::createSpatialIndex()
{
  if ( !mSpatialIndex )
  {
    mSpatialIndex = new QgsSpatialIndex();

    // add existing features to index
    for ( QgsFeatureMap::iterator it = mFeatures.begin(); it != mFeatures.end(); ++it )
    {
      mSpatialIndex->insertFeature( *it );
    }
  }
  return true;
}

int QgsMemoryProvider::capabilities() const
{
  return AddFeatures | DeleteFeatures | ChangeGeometries |
         ChangeAttributeValues | AddAttributes | DeleteAttributes | CreateSpatialIndex |
         SelectAtId | SelectGeometryAtId;
}


void QgsMemoryProvider::updateExtent()
{
  if ( mFeatures.count() == 0 )
  {
    mExtent = QgsRectangle();
  }
  else
  {
    mExtent = mFeatures.begin().value().geometry()->boundingBox();
    for ( QgsFeatureMap::iterator it = mFeatures.begin(); it != mFeatures.end(); ++it )
      mExtent.unionRect( it.value().geometry()->boundingBox() );
  }
}



// --------------------------------

QString  QgsMemoryProvider::name() const
{
  return TEXT_PROVIDER_KEY;
}

QString  QgsMemoryProvider::description() const
{
  return TEXT_PROVIDER_DESCRIPTION;
}

// --------------------------------


/**
 * Class factory to return a pointer to a newly created
 * QgsMemoryProvider object
 */
QGISEXTERN QgsMemoryProvider *classFactory( const QString *uri )
{
  return new QgsMemoryProvider( *uri );
}

/** Required key function (used to map the plugin to a data store type)
 */
QGISEXTERN QString providerKey()
{
  return TEXT_PROVIDER_KEY;
}

/**
 * Required description function
 */
QGISEXTERN QString description()
{
  return TEXT_PROVIDER_DESCRIPTION;
}

/**
 * Required isProvider function. Used to determine if this shared library
 * is a data provider plugin
 */
QGISEXTERN bool isProvider()
{
  return true;
}

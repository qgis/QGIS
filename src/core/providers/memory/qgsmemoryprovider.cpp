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
#include "qgsfields.h"
#include "qgsgeometry.h"
#include "qgslogger.h"
#include "qgsspatialindex.h"
#include "qgscoordinatereferencesystem.h"

#include <QUrl>
#include <QRegExp>

///@cond PRIVATE

static const QString TEXT_PROVIDER_KEY = QStringLiteral( "memory" );
static const QString TEXT_PROVIDER_DESCRIPTION = QStringLiteral( "Memory provider" );

QgsMemoryProvider::QgsMemoryProvider( const QString &uri )
  : QgsVectorDataProvider( uri )

{
  // Initialize the geometry with the uri to support old style uri's
  // (ie, just 'point', 'line', 'polygon')
  QUrl url = QUrl::fromEncoded( uri.toUtf8() );
  QString geometry;
  if ( url.hasQueryItem( QStringLiteral( "geometry" ) ) )
  {
    geometry = url.queryItemValue( QStringLiteral( "geometry" ) );
  }
  else
  {
    geometry = url.path();
  }

  if ( geometry.toLower() == QLatin1String( "none" ) )
  {
    mWkbType = QgsWkbTypes::NoGeometry;
  }
  else
  {
    mWkbType = QgsWkbTypes::parseType( geometry );
  }

  if ( url.hasQueryItem( QStringLiteral( "crs" ) ) )
  {
    QString crsDef = url.queryItemValue( QStringLiteral( "crs" ) );
    mCrs.createFromString( crsDef );
  }

  mNextFeatureId = 1;

  setNativeTypes( QList< NativeType >()
                  << QgsVectorDataProvider::NativeType( tr( "Whole number (integer)" ), QStringLiteral( "integer" ), QVariant::Int, 0, 10 )
                  // Decimal number from OGR/Shapefile/dbf may come with length up to 32 and
                  // precision up to length-2 = 30 (default, if width is not specified in dbf is length = 24 precision = 15)
                  // We know that double (QVariant::Double) has only 15-16 significant numbers,
                  // but setting that correct limits would disable the use of memory provider with
                  // data from Shapefiles. In any case, the data are handled as doubles.
                  // So the limits set here are not correct but enable use of data from Shapefiles.
                  << QgsVectorDataProvider::NativeType( tr( "Decimal number (real)" ), QStringLiteral( "double" ), QVariant::Double, 0, 32, 0, 30 )
                  << QgsVectorDataProvider::NativeType( tr( "Text (string)" ), QStringLiteral( "string" ), QVariant::String, 0, 255 )

                  // date type
                  << QgsVectorDataProvider::NativeType( tr( "Date" ), QStringLiteral( "date" ), QVariant::Date, -1, -1, -1, -1 )
                  << QgsVectorDataProvider::NativeType( tr( "Time" ), QStringLiteral( "time" ), QVariant::Time, -1, -1, -1, -1 )
                  << QgsVectorDataProvider::NativeType( tr( "Date & Time" ), QStringLiteral( "datetime" ), QVariant::DateTime, -1, -1, -1, -1 )

                  // integer types
                  << QgsVectorDataProvider::NativeType( tr( "Whole number (smallint - 16bit)" ), QStringLiteral( "int2" ), QVariant::Int, -1, -1, 0, 0 )
                  << QgsVectorDataProvider::NativeType( tr( "Whole number (integer - 32bit)" ), QStringLiteral( "int4" ), QVariant::Int, -1, -1, 0, 0 )
                  << QgsVectorDataProvider::NativeType( tr( "Whole number (integer - 64bit)" ), QStringLiteral( "int8" ), QVariant::LongLong, -1, -1, 0, 0 )
                  << QgsVectorDataProvider::NativeType( tr( "Decimal number (numeric)" ), QStringLiteral( "numeric" ), QVariant::Double, 1, 20, 0, 20 )
                  << QgsVectorDataProvider::NativeType( tr( "Decimal number (decimal)" ), QStringLiteral( "decimal" ), QVariant::Double, 1, 20, 0, 20 )

                  // floating point
                  << QgsVectorDataProvider::NativeType( tr( "Decimal number (real)" ), QStringLiteral( "real" ), QVariant::Double, -1, -1, -1, -1 )
                  << QgsVectorDataProvider::NativeType( tr( "Decimal number (double)" ), QStringLiteral( "double precision" ), QVariant::Double, -1, -1, -1, -1 )

                  // string types
                  << QgsVectorDataProvider::NativeType( tr( "Text, unlimited length (text)" ), QStringLiteral( "text" ), QVariant::String, -1, -1, -1, -1 )
                );

  if ( url.hasQueryItem( QStringLiteral( "field" ) ) )
  {
    QList<QgsField> attributes;
    QRegExp reFieldDef( "\\:"
                        "(int|integer|long|int8|real|double|string|date|time|datetime)" // type
                        "(?:\\((\\-?\\d+)"                // length
                        "(?:\\,(\\d+))?"                  // precision
                        "\\))?(\\[\\])?"                  // array
                        "$", Qt::CaseInsensitive );
    QStringList fields = url.allQueryItemValues( QStringLiteral( "field" ) );
    for ( int i = 0; i < fields.size(); i++ )
    {
      QString name = fields.at( i );
      QVariant::Type type = QVariant::String;
      QVariant::Type subType = QVariant::Invalid;
      QString typeName( QStringLiteral( "string" ) );
      int length = 255;
      int precision = 0;

      int pos = reFieldDef.indexIn( name );
      if ( pos >= 0 )
      {
        name = name.mid( 0, pos );
        typeName = reFieldDef.cap( 1 ).toLower();
        if ( typeName == QLatin1String( "int" ) || typeName == QLatin1String( "integer" ) )
        {
          type = QVariant::Int;
          typeName = QStringLiteral( "integer" );
          length = -1;
        }
        else if ( typeName == QLatin1String( "int8" ) || typeName == QLatin1String( "long" ) )
        {
          type = QVariant::LongLong;
          typeName = QStringLiteral( "int8" );
          length = -1;
        }
        else if ( typeName == QLatin1String( "real" ) || typeName == QLatin1String( "double" ) )
        {
          type = QVariant::Double;
          typeName = QStringLiteral( "double" );
          length = 20;
          precision = 5;
        }
        else if ( typeName == QLatin1String( "date" ) )
        {
          type = QVariant::Date;
          typeName = QStringLiteral( "date" );
          length = -1;
        }
        else if ( typeName == QLatin1String( "time" ) )
        {
          type = QVariant::Time;
          typeName = QStringLiteral( "time" );
          length = -1;
        }
        else if ( typeName == QLatin1String( "datetime" ) )
        {
          type = QVariant::DateTime;
          typeName = QStringLiteral( "datetime" );
          length = -1;
        }

        if ( !reFieldDef.cap( 2 ).isEmpty() )
        {
          length = reFieldDef.cap( 2 ).toInt();
        }
        if ( !reFieldDef.cap( 3 ).isEmpty() )
        {
          precision = reFieldDef.cap( 3 ).toInt();
        }
        if ( !reFieldDef.cap( 4 ).isEmpty() )
        {
          //array
          subType = type;
          type = ( subType == QVariant::String ? QVariant::StringList : QVariant::List );
        }
      }
      if ( !name.isEmpty() )
        attributes.append( QgsField( name, type, typeName, length, precision, QLatin1String( "" ), subType ) );
    }
    addAttributes( attributes );
  }

  if ( url.hasQueryItem( QStringLiteral( "index" ) ) && url.queryItemValue( QStringLiteral( "index" ) ) == QLatin1String( "yes" ) )
  {
    createSpatialIndex();
  }

}

QgsMemoryProvider::~QgsMemoryProvider()
{
  delete mSpatialIndex;
}

QString QgsMemoryProvider::providerKey()
{
  return TEXT_PROVIDER_KEY;
}

QString QgsMemoryProvider::providerDescription()
{
  return TEXT_PROVIDER_DESCRIPTION;
}

QgsMemoryProvider *QgsMemoryProvider::createProvider( const QString &uri )
{
  return new QgsMemoryProvider( uri );
}

QgsAbstractFeatureSource *QgsMemoryProvider::featureSource() const
{
  return new QgsMemoryFeatureSource( this );
}

QString QgsMemoryProvider::dataSourceUri( bool expandAuthConfig ) const
{
  Q_UNUSED( expandAuthConfig )

  QUrl uri( QStringLiteral( "memory" ) );
  QString geometry = QgsWkbTypes::displayString( mWkbType );
  uri.addQueryItem( QStringLiteral( "geometry" ), geometry );

  if ( mCrs.isValid() )
  {
    QString crsDef( QLatin1String( "" ) );
    QString authid = mCrs.authid();
    if ( authid.startsWith( QLatin1String( "EPSG:" ) ) )
    {
      crsDef = authid;
    }
    else
    {
      int srid = mCrs.postgisSrid();
      if ( srid )
      {
        crsDef = QStringLiteral( "postgis:%1" ).arg( srid );
      }
      else
      {
        crsDef = QStringLiteral( "wkt:%1" ).arg( mCrs.toWkt() );
      }
    }
    uri.addQueryItem( QStringLiteral( "crs" ), crsDef );
  }
  if ( mSpatialIndex )
  {
    uri.addQueryItem( QStringLiteral( "index" ), QStringLiteral( "yes" ) );
  }

  QgsAttributeList attrs = const_cast<QgsMemoryProvider *>( this )->attributeIndexes();
  for ( int i = 0; i < attrs.size(); i++ )
  {
    QgsField field = mFields.at( attrs[i] );
    QString fieldDef = field.name();
    fieldDef.append( QStringLiteral( ":%2(%3,%4)" ).arg( field.typeName() ).arg( field.length() ).arg( field.precision() ) );
    uri.addQueryItem( QStringLiteral( "field" ), fieldDef );
  }

  return QString( uri.toEncoded() );

}

QString QgsMemoryProvider::storageType() const
{
  return QStringLiteral( "Memory storage" );
}

QgsFeatureIterator QgsMemoryProvider::getFeatures( const QgsFeatureRequest &request ) const
{
  return QgsFeatureIterator( new QgsMemoryFeatureIterator( new QgsMemoryFeatureSource( this ), true, request ) );
}


QgsRectangle QgsMemoryProvider::extent() const
{
  if ( mExtent.isEmpty() && !mFeatures.isEmpty() )
  {
    mExtent.setMinimal();
    Q_FOREACH ( const QgsFeature &feat, mFeatures )
    {
      if ( feat.hasGeometry() )
        mExtent.combineExtentWith( feat.geometry().boundingBox() );
    }
  }

  return mExtent;
}

QgsWkbTypes::Type QgsMemoryProvider::wkbType() const
{
  return mWkbType;
}

long QgsMemoryProvider::featureCount() const
{
  if ( mSubsetString.isEmpty() )
    return mFeatures.count();

  // subset string set, no alternative but testing each feature
  QgsFeatureIterator fit = QgsFeatureIterator( new QgsMemoryFeatureIterator( new QgsMemoryFeatureSource( this ), true,  QgsFeatureRequest().setSubsetOfAttributes( QgsAttributeList() ) ) );
  int count = 0;
  QgsFeature feature;
  while ( fit.nextFeature( feature ) )
  {
    count++;
  }
  return count;
}

QgsFields QgsMemoryProvider::fields() const
{
  return mFields;
}

bool QgsMemoryProvider::isValid() const
{
  return ( mWkbType != QgsWkbTypes::Unknown );
}

QgsCoordinateReferenceSystem QgsMemoryProvider::crs() const
{
  // TODO: make provider projection-aware
  return mCrs; // return default CRS
}


bool QgsMemoryProvider::addFeatures( QgsFeatureList &flist, Flags )
{
  // whether or not to update the layer extent on the fly as we add features
  bool updateExtent = mFeatures.isEmpty() || !mExtent.isEmpty();

  // TODO: sanity checks of fields and geometries
  for ( QgsFeatureList::iterator it = flist.begin(); it != flist.end(); ++it )
  {
    it->setId( mNextFeatureId );
    it->setValid( true );

    mFeatures.insert( mNextFeatureId, *it );

    if ( it->hasGeometry() )
    {
      if ( updateExtent )
        mExtent.combineExtentWith( it->geometry().boundingBox() );

      // update spatial index
      if ( mSpatialIndex )
        mSpatialIndex->insertFeature( *it );
    }

    mNextFeatureId++;
  }

  return true;
}

bool QgsMemoryProvider::deleteFeatures( const QgsFeatureIds &id )
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

  updateExtents();

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
      case QVariant::Date:
      case QVariant::Time:
      case QVariant::DateTime:
      case QVariant::LongLong:
      case QVariant::StringList:
      case QVariant::List:
        break;
      default:
        QgsDebugMsg( "Field type not supported: " + it->typeName() );
        continue;
    }
    // add new field as a last one
    mFields.append( *it );

    for ( QgsFeatureMap::iterator fit = mFeatures.begin(); fit != mFeatures.end(); ++fit )
    {
      QgsFeature &f = fit.value();
      QgsAttributes attr = f.attributes();
      attr.append( QVariant() );
      f.setAttributes( attr );
    }
  }
  return true;
}

bool QgsMemoryProvider::renameAttributes( const QgsFieldNameMap &renamedAttributes )
{
  QgsFieldNameMap::const_iterator renameIt = renamedAttributes.constBegin();
  bool result = true;
  for ( ; renameIt != renamedAttributes.constEnd(); ++renameIt )
  {
    int fieldIndex = renameIt.key();
    if ( fieldIndex < 0 || fieldIndex >= mFields.count() )
    {
      result = false;
      continue;
    }
    if ( mFields.indexFromName( renameIt.value() ) >= 0 )
    {
      //field name already in use
      result = false;
      continue;
    }

    mFields[ fieldIndex ].setName( renameIt.value() );
  }
  return result;
}

bool QgsMemoryProvider::deleteAttributes( const QgsAttributeIds &attributes )
{
  QList<int> attrIdx = attributes.toList();
  std::sort( attrIdx.begin(), attrIdx.end(), std::greater<int>() );

  // delete attributes one-by-one with decreasing index
  for ( QList<int>::const_iterator it = attrIdx.constBegin(); it != attrIdx.constEnd(); ++it )
  {
    int idx = *it;
    mFields.remove( idx );

    for ( QgsFeatureMap::iterator fit = mFeatures.begin(); fit != mFeatures.end(); ++fit )
    {
      QgsFeature &f = fit.value();
      QgsAttributes attr = f.attributes();
      attr.remove( idx );
      f.setAttributes( attr );
    }
  }
  return true;
}

bool QgsMemoryProvider::changeAttributeValues( const QgsChangedAttributesMap &attr_map )
{
  for ( QgsChangedAttributesMap::const_iterator it = attr_map.begin(); it != attr_map.end(); ++it )
  {
    QgsFeatureMap::iterator fit = mFeatures.find( it.key() );
    if ( fit == mFeatures.end() )
      continue;

    const QgsAttributeMap &attrs = it.value();
    for ( QgsAttributeMap::const_iterator it2 = attrs.constBegin(); it2 != attrs.constEnd(); ++it2 )
      fit->setAttribute( it2.key(), it2.value() );
  }
  return true;
}

bool QgsMemoryProvider::changeGeometryValues( const QgsGeometryMap &geometry_map )
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

  updateExtents();

  return true;
}

QString QgsMemoryProvider::subsetString() const
{
  return mSubsetString;
}

bool QgsMemoryProvider::setSubsetString( const QString &theSQL, bool updateFeatureCount )
{
  Q_UNUSED( updateFeatureCount );

  if ( !theSQL.isEmpty() )
  {
    QgsExpression tempExpression( theSQL );
    if ( tempExpression.hasParserError() )
      return false;
  }

  mSubsetString = theSQL;
  clearMinMaxCache();

  emit dataChanged();
  return true;
}

bool QgsMemoryProvider::createSpatialIndex()
{
  if ( !mSpatialIndex )
  {
    mSpatialIndex = new QgsSpatialIndex();

    // add existing features to index
    for ( QgsFeatureMap::const_iterator it = mFeatures.constBegin(); it != mFeatures.constEnd(); ++it )
    {
      mSpatialIndex->insertFeature( *it );
    }
  }
  return true;
}

QgsVectorDataProvider::Capabilities QgsMemoryProvider::capabilities() const
{
  return AddFeatures | DeleteFeatures | ChangeGeometries |
         ChangeAttributeValues | AddAttributes | DeleteAttributes | RenameAttributes | CreateSpatialIndex |
         SelectAtId | CircularGeometries;
}


void QgsMemoryProvider::updateExtents()
{
  mExtent.setMinimal();
}

QString QgsMemoryProvider::name() const
{
  return TEXT_PROVIDER_KEY;
}

QString QgsMemoryProvider::description() const
{
  return TEXT_PROVIDER_DESCRIPTION;
}


///@endcond

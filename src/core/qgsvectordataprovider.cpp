/***************************************************************************
    qgsvectordataprovider.cpp - DataProvider Interface for vector layers
     --------------------------------------
    Date                 : 26-Oct-2004
    Copyright            : (C) 2004 by Marco Hugentobler
    email                : marco.hugentobler@autoform.ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
/* $Id$ */

#include <QSettings>
#include <QTextCodec>

#include <cfloat> // for DBL_MAX
#include <climits>

#include "qgsvectordataprovider.h"
#include "qgsfeature.h"
#include "qgsfield.h"
#include "qgslogger.h"

QgsVectorDataProvider::QgsVectorDataProvider( QString uri )
    : QgsDataProvider( uri ),
    mCacheMinMaxDirty( TRUE ),
    mFetchFeaturesWithoutGeom( TRUE )
{
  QSettings settings;
  setEncoding( settings.value( "/UI/encoding", QString( "System" ) ).toString() );
}


QgsVectorDataProvider::~QgsVectorDataProvider()
{
}

QString QgsVectorDataProvider::storageType() const
{
  return "Generic vector file";
}

long QgsVectorDataProvider::updateFeatureCount()
{
  return -1;
}

bool QgsVectorDataProvider::featureAtId( int featureId,
    QgsFeature& feature,
    bool fetchGeometry,
    QgsAttributeList fetchAttributes )
{
  select( fetchAttributes, QgsRectangle(), fetchGeometry );

  while ( nextFeature( feature ) )
  {
    if ( feature.id() == featureId )
      return TRUE;
  }

  return FALSE;
}

QString QgsVectorDataProvider::dataComment() const
{
  return QString();
}

bool QgsVectorDataProvider::addFeatures( QgsFeatureList &flist )
{
  return false;
}

bool QgsVectorDataProvider::deleteFeatures( const QgsFeatureIds & id )
{
  return false;
}

bool QgsVectorDataProvider::addAttributes( const QList<QgsField> & attributes )
{
  return false;
}

bool QgsVectorDataProvider::addAttributes( const QMap<QString, QString> &attributes )
{
  const QMap<QString, QVariant::Type> &map = supportedNativeTypes();
  QList< QgsField > list;

  for ( QMap<QString, QString>::const_iterator it = attributes.constBegin(); it != attributes.constEnd(); it++ )
  {
    if ( !map.contains( it.value() ) )
      return false;

    list << QgsField( it.key(), map[ it.value()], it.value() );
  }

  return addAttributes( list );
}

bool QgsVectorDataProvider::deleteAttributes( const QgsAttributeIds& attributes )
{
  return false;
}

bool QgsVectorDataProvider::changeAttributeValues( const QgsChangedAttributesMap & attr_map )
{
  return false;
}

QVariant QgsVectorDataProvider::defaultValue( int fieldId )
{
  return QVariant();
}

bool QgsVectorDataProvider::changeGeometryValues( QgsGeometryMap & geometry_map )
{
  return false;
}

bool QgsVectorDataProvider::createSpatialIndex()
{
  return false;
}

int QgsVectorDataProvider::capabilities() const
{
  return QgsVectorDataProvider::NoCapabilities;
}


void QgsVectorDataProvider::setEncoding( const QString& e )
{
  QTextCodec* ncodec = QTextCodec::codecForName( e.toLocal8Bit().data() );
  if ( ncodec )
  {
    mEncoding = ncodec;
  }
  else
  {
    QgsDebugMsg( "error finding QTextCodec for " + e );
  }
}

QString QgsVectorDataProvider::encoding() const
{
  if ( mEncoding )
  {
    return mEncoding->name();
  }

  return "";
}

QString QgsVectorDataProvider::capabilitiesString() const
{
  QStringList abilitiesList;

  int abilities = capabilities();

  if ( abilities & QgsVectorDataProvider::AddFeatures )
  {
    abilitiesList += "Add Features";
    QgsDebugMsg( "Capability: Add Features" );
  }

  if ( abilities & QgsVectorDataProvider::DeleteFeatures )
  {
    abilitiesList += "Delete Features";
    QgsDebugMsg( "Capability: Delete Features" );
  }

  if ( abilities & QgsVectorDataProvider::ChangeAttributeValues )
  {
    abilitiesList += "Change Attribute Values";
    QgsDebugMsg( "Capability: Change Attribute Values" );
  }

  if ( abilities & QgsVectorDataProvider::AddAttributes )
  {
    abilitiesList += "Add Attributes";
    QgsDebugMsg( "Capability: Add Attributes" );
  }

  if ( abilities & QgsVectorDataProvider::DeleteAttributes )
  {
    abilitiesList += "Delete Attributes";
    QgsDebugMsg( "Capability: Delete Attributes" );
  }

  if ( abilities & QgsVectorDataProvider::CreateSpatialIndex )
  {
    // TODO: Tighten up this test.  See QgsOgrProvider for details.
    abilitiesList += "Create Spatial Index";
    QgsDebugMsg( "Capability: Create Spatial Index" );
  }

  if ( abilities & QgsVectorDataProvider::SelectAtId )
  {
    abilitiesList += "Fast Access to Features at ID";
    QgsDebugMsg( "Capability: Select at ID" );
  }

  if ( abilities & QgsVectorDataProvider::ChangeGeometries )
  {
    abilitiesList += "Change Geometries";
    QgsDebugMsg( "Capability: Change Geometries" );
  }

  return abilitiesList.join( ", " );

}


int QgsVectorDataProvider::fieldNameIndex( const QString& fieldName ) const
{
  const QgsFieldMap &theFields = fields();

  for ( QgsFieldMap::const_iterator it = theFields.constBegin(); it != theFields.constEnd(); ++it )
  {
    if ( it->name() == fieldName )
    {
      return it.key();
    }
  }
  return -1;
}

QMap<QString, int> QgsVectorDataProvider::fieldNameMap() const
{
  QMap<QString, int> resultMap;

  const QgsFieldMap& theFields = fields();
  QgsFieldMap::const_iterator field_it = theFields.constBegin();
  for ( ; field_it != theFields.constEnd(); ++field_it )
  {
    resultMap.insert( field_it.value().name(), field_it.key() );
  }

  return resultMap;
}

QgsAttributeList QgsVectorDataProvider::attributeIndexes()
{
  uint count = fieldCount();
  QgsAttributeList list;

  for ( uint i = 0; i < count; i++ )
    list.append( i );

  return list;
}

void QgsVectorDataProvider::enableGeometrylessFeatures( bool fetch )
{
  mFetchFeaturesWithoutGeom = fetch;
}

const QList< QgsVectorDataProvider::NativeType > &QgsVectorDataProvider::nativeTypes() const
{
  return mNativeTypes;
}

const QMap<QString, QVariant::Type> &QgsVectorDataProvider::supportedNativeTypes() const
{
  if ( mOldTypeList.size() > 0 )
    return mOldTypeList;

  QgsVectorDataProvider *p = ( QgsVectorDataProvider * )this;

  const QList< QgsVectorDataProvider::NativeType > &types = nativeTypes();

  for ( QList< QgsVectorDataProvider::NativeType >::const_iterator it = types.constBegin(); it != types.constEnd(); it++ )
  {
    p->mOldTypeList.insert( it->mTypeName, it->mType );
  }

  return p->mOldTypeList;
}

bool QgsVectorDataProvider::supportedType( const QgsField &field ) const
{
  int i;
  for ( i = 0; i < mNativeTypes.size(); i++ )
  {
    if ( field.type() == mNativeTypes[i].mType &&
         field.length() >= mNativeTypes[i].mMinLen && field.length() <= mNativeTypes[i].mMaxLen &&
         field.precision() >= mNativeTypes[i].mMinPrec && field.precision() <= mNativeTypes[i].mMaxPrec )
    {
      break;
    }
  }

  return i < mNativeTypes.size();
}

QVariant QgsVectorDataProvider::minimumValue( int index )
{
  if ( !fields().contains( index ) )
  {
    QgsDebugMsg( "Warning: access requested to invalid field index: " + QString::number( index ) );
    return QVariant();
  }

  if ( mCacheMinMaxDirty )
  {
    fillMinMaxCache();
  }

  if ( !mCacheMinValues.contains( index ) )
    return QVariant();

  return mCacheMinValues[index];
}

QVariant QgsVectorDataProvider::maximumValue( int index )
{
  if ( !fields().contains( index ) )
  {
    QgsDebugMsg( "Warning: access requested to invalid field index: " + QString::number( index ) );
    return QVariant();
  }

  if ( mCacheMinMaxDirty )
  {
    fillMinMaxCache();
  }

  if ( !mCacheMaxValues.contains( index ) )
    return QVariant();

  return mCacheMaxValues[index];
}

void QgsVectorDataProvider::uniqueValues( int index, QList<QVariant> &values )
{
  QgsFeature f;
  QgsAttributeList keys;
  keys.append( index );
  select( keys, QgsRectangle(), false );

  QSet<QString> set;
  values.clear();

  while ( nextFeature( f ) )
  {
    if ( !set.contains( f.attributeMap()[index].toString() ) )
    {
      values.append( f.attributeMap()[index] );
      set.insert( f.attributeMap()[index].toString() );
    }
  }
}

void QgsVectorDataProvider::fillMinMaxCache()
{
  const QgsFieldMap& flds = fields();
  for ( QgsFieldMap::const_iterator it = flds.begin(); it != flds.end(); ++it )
  {
    if ( it->type() == QVariant::Int )
    {
      mCacheMinValues[it.key()] = QVariant( INT_MAX );
      mCacheMaxValues[it.key()] = QVariant( INT_MIN );
    }
    else if ( it->type() == QVariant::Double )
    {
      mCacheMinValues[it.key()] = QVariant( DBL_MAX );
      mCacheMaxValues[it.key()] = QVariant( -DBL_MAX );
    }
  }

  QgsFeature f;
  QgsAttributeList keys = mCacheMinValues.keys();
  select( keys, QgsRectangle(), false );

  while ( nextFeature( f ) )
  {
    QgsAttributeMap attrMap = f.attributeMap();
    for ( QgsAttributeList::const_iterator it = keys.begin(); it != keys.end(); ++it )
    {
      const QVariant& varValue = attrMap[*it];

      if ( flds[*it].type() == QVariant::Int )
      {
        int value = varValue.toInt();
        if ( value < mCacheMinValues[*it].toInt() )
          mCacheMinValues[*it] = value;
        if ( value > mCacheMaxValues[*it].toInt() )
          mCacheMaxValues[*it] = value;
      }
      else if ( flds[*it].type() == QVariant::Double )
      {
        double value = varValue.toDouble();
        if ( value < mCacheMinValues[*it].toDouble() )
          mCacheMinValues[*it] = value;
        if ( value > mCacheMaxValues[*it].toDouble() )
          mCacheMaxValues[*it] = value;
      }
    }
  }

  mCacheMinMaxDirty = FALSE;
}

QVariant QgsVectorDataProvider::convertValue( QVariant::Type type, QString value )
{
  QVariant v( value );

  if ( !v.convert( type ) )
    v = QVariant( QString::null );

  return v;
}

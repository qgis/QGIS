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

#include "qgsapplication.h"
#include "qgscoordinatereferencesystem.h"
#include "qgsfeature.h"
#include "qgsfields.h"
#include "qgsgeometry.h"
#include "qgslogger.h"
#include "qgsmemoryfeatureiterator.h"
#include "qgsspatialindex.h"
#include "qgsvariantutils.h"

#include <QIcon>
#include <QRegularExpression>
#include <QUrl>
#include <QUrlQuery>

#include "moc_qgsmemoryprovider.cpp"

///@cond PRIVATE

#define TEXT_PROVIDER_KEY u"memory"_s
#define TEXT_PROVIDER_DESCRIPTION u"Memory provider"_s

QgsMemoryProvider::QgsMemoryProvider( const QString &uri, const ProviderOptions &options, Qgis::DataProviderReadFlags flags )
  : QgsVectorDataProvider( uri, options, flags )
{
  // Initialize the geometry with the uri to support old style uri's
  // (ie, just 'point', 'line', 'polygon')
  const QUrl url = QUrl::fromEncoded( uri.toUtf8() );
  const QUrlQuery query( url );
  QString geometry;
  if ( query.hasQueryItem( u"geometry"_s ) )
  {
    geometry = query.queryItemValue( u"geometry"_s );
  }
  else
  {
    geometry = url.path();
  }

  if ( geometry.compare( "none"_L1, Qt::CaseInsensitive ) == 0 )
  {
    mWkbType = Qgis::WkbType::NoGeometry;
  }
  else
  {
    mWkbType = QgsWkbTypes::parseType( geometry );
  }

  if ( query.hasQueryItem( u"crs"_s ) )
  {
    const QString crsDef = query.queryItemValue( u"crs"_s );
    mCrs.createFromString( crsDef );
  }
  else
  {
    // TODO - remove in QGIS 5.0. Layers without an explicit CRS set SHOULD have an invalid CRS. But in order to maintain
    // 3.x api, we have to be tolerant/shortsighted(?) here and fallback to EPSG:4326
    mCrs = QgsCoordinateReferenceSystem( u"EPSG:4326"_s );
  }

  mNextFeatureId = 1;

  setNativeTypes( QList< NativeType >()
                  << QgsVectorDataProvider::NativeType( tr( "Whole Number (integer)" ), u"integer"_s, QMetaType::Type::Int, 0, 10 )
                  // Decimal number from OGR/Shapefile/dbf may come with length up to 32 and
                  // precision up to length-2 = 30 (default, if width is not specified in dbf is length = 24 precision = 15)
                  // We know that double (QVariant::Double) has only 15-16 significant numbers,
                  // but setting that correct limits would disable the use of memory provider with
                  // data from Shapefiles. In any case, the data are handled as doubles.
                  // So the limits set here are not correct but enable use of data from Shapefiles.
                  << QgsVectorDataProvider::NativeType( tr( "Decimal Number (real)" ), u"double"_s, QMetaType::Type::Double, 0, 32, 0, 30 )
                  << QgsVectorDataProvider::NativeType( tr( "Text (string)" ), u"string"_s, QMetaType::Type::QString, 0, 255 )

                  // date type
                  << QgsVectorDataProvider::NativeType( QgsVariantUtils::typeToDisplayString( QMetaType::Type::QDate ), u"date"_s, QMetaType::Type::QDate, -1, -1, -1, -1 )
                  << QgsVectorDataProvider::NativeType( QgsVariantUtils::typeToDisplayString( QMetaType::Type::QTime ), u"time"_s, QMetaType::Type::QTime, -1, -1, -1, -1 )
                  << QgsVectorDataProvider::NativeType( QgsVariantUtils::typeToDisplayString( QMetaType::Type::QDateTime ), u"datetime"_s, QMetaType::Type::QDateTime, -1, -1, -1, -1 )

                  // integer types
                  << QgsVectorDataProvider::NativeType( tr( "Whole Number (smallint - 16bit)" ), u"int2"_s, QMetaType::Type::Int, -1, -1, 0, 0 )
                  << QgsVectorDataProvider::NativeType( tr( "Whole Number (integer - 32bit)" ), u"int4"_s, QMetaType::Type::Int, -1, -1, 0, 0 )
                  << QgsVectorDataProvider::NativeType( tr( "Whole Number (integer - 64bit)" ), u"int8"_s, QMetaType::Type::LongLong, -1, -1, 0, 0 )
                  << QgsVectorDataProvider::NativeType( tr( "Decimal Number (numeric)" ), u"numeric"_s, QMetaType::Type::Double, 1, 20, 0, 20 )
                  << QgsVectorDataProvider::NativeType( tr( "Decimal Number (decimal)" ), u"decimal"_s, QMetaType::Type::Double, 1, 20, 0, 20 )

                  // floating point
                  << QgsVectorDataProvider::NativeType( tr( "Decimal Number (real)" ), u"real"_s, QMetaType::Type::Double, -1, -1, -1, -1 )
                  << QgsVectorDataProvider::NativeType( tr( "Decimal Number (double)" ), u"double precision"_s, QMetaType::Type::Double, -1, -1, -1, -1 )

                  // string types
                  << QgsVectorDataProvider::NativeType( tr( "Text, unlimited length (text)" ), u"text"_s, QMetaType::Type::QString, -1, -1, -1, -1 )

                  // boolean
                  << QgsVectorDataProvider::NativeType( QgsVariantUtils::typeToDisplayString( QMetaType::Type::Bool ), u"boolean"_s, QMetaType::Type::Bool )

                  // blob
                  << QgsVectorDataProvider::NativeType( QgsVariantUtils::typeToDisplayString( QMetaType::Type::QByteArray ), u"binary"_s, QMetaType::Type::QByteArray )

                  // list types
                  << QgsVectorDataProvider::NativeType( QgsVariantUtils::typeToDisplayString( QMetaType::Type::QStringList ), u"stringlist"_s, QMetaType::Type::QStringList, 0, 0, 0, 0, QMetaType::Type::QString )
                  << QgsVectorDataProvider::NativeType( QgsVariantUtils::typeToDisplayString( QMetaType::Type::QVariantList, QMetaType::Type::Int ), u"integerlist"_s, QMetaType::Type::QVariantList, 0, 0, 0, 0, QMetaType::Type::Int )
                  << QgsVectorDataProvider::NativeType( QgsVariantUtils::typeToDisplayString( QMetaType::Type::QVariantList, QMetaType::Type::Double ), u"doublelist"_s, QMetaType::Type::QVariantList, 0, 0, 0, 0, QMetaType::Type::Double )
                  << QgsVectorDataProvider::NativeType( QgsVariantUtils::typeToDisplayString( QMetaType::Type::QVariantList, QMetaType::Type::LongLong ), u"integer64list"_s, QMetaType::Type::QVariantList, 0, 0, 0, 0, QMetaType::Type::LongLong )

                  // complex types
                  << QgsVectorDataProvider::NativeType( QgsVariantUtils::typeToDisplayString( QMetaType::Type::QVariantMap ), u"map"_s, QMetaType::Type::QVariantMap, -1, -1, -1, -1 )
                  << QgsVectorDataProvider::NativeType( tr( "Geometry" ), u"geometry"_s, QMetaType::Type::User )
                );

  if ( query.hasQueryItem( u"field"_s ) )
  {
    QList<QgsField> attributes;
    const thread_local QRegularExpression reFieldDef( "\\:"
        "([\\w\\s]+)"                // type
        "(?:\\((\\-?\\d+)"           // length
        "(?:\\,(\\-?\\d+))?"         // precision
        "\\))?(\\[\\])?"             // array
        "$",
        QRegularExpression::CaseInsensitiveOption );
    const QStringList fields = query.allQueryItemValues( u"field"_s );
    for ( int i = 0; i < fields.size(); i++ )
    {
      QString name = QUrl::fromPercentEncoding( fields.at( i ).toUtf8() );
      const QRegularExpressionMatch regularExpressionMatch = reFieldDef.match( name );

      // If no match -> use string as type
      QMetaType::Type type = QMetaType::Type::QString;
      QMetaType::Type subType = QMetaType::Type::UnknownType;
      QString typeName( u"string"_s );
      int length = 255;
      int precision = 0;

      if ( regularExpressionMatch.hasMatch() )
      {
        name = name.mid( 0, regularExpressionMatch.capturedStart() );
        typeName = regularExpressionMatch.captured( 1 ).toLower();

        // Search typeName correspondence in native types
        bool isNativeType = false;
        const QList<QgsVectorDataProvider::NativeType> nativeTypesList( nativeTypes() );
        for ( const NativeType &nativeType : nativeTypesList )
        {
          if ( nativeType.mTypeName.toLower() == typeName )
          {
            isNativeType = true;
            type = nativeType.mType;
            subType = nativeType.mSubType;
            typeName = nativeType.mTypeName;
            break;
          }
        }

        // Not a native type -> check other supported types:
        if ( isNativeType == false )
        {
          if ( typeName == "int"_L1 )
          {
            type = QMetaType::Type::Int;
            typeName = u"integer"_s;
          }
          else if ( typeName == "long"_L1 )
          {
            type = QMetaType::Type::LongLong;
            typeName = u"int8"_s;
          }
          else if ( typeName == "bool"_L1 )
          {
            type = QMetaType::Type::Bool;
            typeName = u"boolean"_s;
          }
          else
          {
            QgsLogger::warning( tr( "Unsupported typeName '%1'. Will be handled as string." ).arg( typeName ) );
            type = QMetaType::Type::QString;
            typeName = u"string"_s;
          }
        }

        // Set default length/precision for double/real
        if ( typeName == "real"_L1 || typeName == "double"_L1 )
        {
          length = 20;
          precision = 5;
        }

        if ( !regularExpressionMatch.captured( 2 ).isEmpty() )
          length = regularExpressionMatch.captured( 2 ).toInt();

        if ( !regularExpressionMatch.captured( 3 ).isEmpty() )
          precision = regularExpressionMatch.captured( 3 ).toInt();

        // Array
        if ( !regularExpressionMatch.captured( 4 ).isEmpty() )
        {
          if ( subType == QMetaType::Type::UnknownType )
            subType = type;

          if ( type != QMetaType::Type::QVariantList && type != QMetaType::Type::QStringList )
            type = type == QMetaType::Type::QString ? QMetaType::Type::QStringList : QMetaType::Type::QVariantList;

          const QLatin1String listSuffix( "list" );
          if ( !typeName.endsWith( listSuffix ) )
            typeName += "list"_L1;
        }
      }

      attributes.append( QgsField( name, type, typeName, length, precision, QString(), subType ) );
    }
    addAttributes( attributes );
  }

  if ( query.hasQueryItem( u"index"_s ) && query.queryItemValue( u"index"_s ) == "yes"_L1 )
  {
    createSpatialIndex();
  }

}

QgsMemoryProvider::~QgsMemoryProvider()
{

}

QString QgsMemoryProvider::providerKey()
{
  return TEXT_PROVIDER_KEY;
}

QString QgsMemoryProvider::providerDescription()
{
  return TEXT_PROVIDER_DESCRIPTION;
}

QgsAbstractFeatureSource *QgsMemoryProvider::featureSource() const
{
  return new QgsMemoryFeatureSource( this );
}

QString QgsMemoryProvider::dataSourceUri( bool expandAuthConfig ) const
{
  Q_UNUSED( expandAuthConfig )

  QUrl uri( u"memory"_s );
  QUrlQuery query;
  const QString geometry = QgsWkbTypes::displayString( mWkbType );
  query.addQueryItem( u"geometry"_s, geometry );

  if ( mCrs.isValid() )
  {
    QString crsDef;
    const QString authid = mCrs.authid();
    if ( authid.startsWith( "EPSG:"_L1 ) )
    {
      crsDef = authid;
    }
    else
    {
      crsDef = u"wkt:%1"_s.arg( mCrs.toWkt( Qgis::CrsWktVariant::Preferred ) );
    }
    query.addQueryItem( u"crs"_s, crsDef );
  }
  if ( mSpatialIndex )
  {
    query.addQueryItem( u"index"_s, u"yes"_s );
  }

  QgsAttributeList attrs = const_cast<QgsMemoryProvider *>( this )->attributeIndexes();
  for ( int i = 0; i < attrs.size(); i++ )
  {
    const QgsField field = mFields.at( attrs[i] );
    QString fieldDef = field.name();

    QString typeName = field.typeName();
    bool isList = false;
    if ( field.type() == QMetaType::Type::QVariantList || field.type() == QMetaType::Type::QStringList )
    {
      switch ( field.subType() )
      {
        case QMetaType::Type::Int:
          typeName = u"integer"_s;
          break;

        case QMetaType::Type::LongLong:
          typeName = u"long"_s;
          break;

        case QMetaType::Type::Double:
          typeName = u"double"_s;
          break;

        case QMetaType::Type::QString:
          typeName = u"string"_s;
          break;

        default:
          break;
      }
      isList = true;
    }

    fieldDef.append( u":%2(%3,%4)%5"_s.arg( typeName ).arg( field.length() ).arg( field.precision() ).arg( isList ? u"[]"_s : QString() ) );
    query.addQueryItem( u"field"_s, fieldDef );
  }
  uri.setQuery( query );

  return QString( uri.toEncoded() );

}

QString QgsMemoryProvider::storageType() const
{
  return u"Memory storage"_s;
}

QgsFeatureIterator QgsMemoryProvider::getFeatures( const QgsFeatureRequest &request ) const
{
  return QgsFeatureIterator( new QgsMemoryFeatureIterator( new QgsMemoryFeatureSource( this ), true, request ) );
}


QgsRectangle QgsMemoryProvider::extent() const
{
  if ( mExtent.isEmpty() && !mFeatures.isEmpty() )
  {
    mExtent.setNull();
    if ( mSubsetString.isEmpty() )
    {
      // fast way - iterate through all features
      const auto constMFeatures = mFeatures;
      for ( const QgsFeature &feat : constMFeatures )
      {
        if ( feat.hasGeometry() )
          mExtent.combineExtentWith( feat.geometry().boundingBox() );
      }
    }
    else
    {
      QgsFeature f;
      QgsFeatureIterator fi = getFeatures( QgsFeatureRequest().setNoAttributes() );
      while ( fi.nextFeature( f ) )
      {
        if ( f.hasGeometry() )
          mExtent.combineExtentWith( f.geometry().boundingBox() );
      }
    }
  }
  else if ( mFeatures.isEmpty() )
  {
    mExtent.setNull();
  }

  return mExtent;
}

Qgis::WkbType QgsMemoryProvider::wkbType() const
{
  return mWkbType;
}

long long QgsMemoryProvider::featureCount() const
{
  if ( mSubsetString.isEmpty() )
    return mFeatures.count();

  // subset string set, no alternative but testing each feature
  QgsFeatureIterator fit = QgsFeatureIterator( new QgsMemoryFeatureIterator( new QgsMemoryFeatureSource( this ), true,  QgsFeatureRequest().setNoAttributes() ) );
  long long count = 0;
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
  return ( mWkbType != Qgis::WkbType::Unknown );
}

QgsCoordinateReferenceSystem QgsMemoryProvider::crs() const
{
  // TODO: make provider projection-aware
  return mCrs; // return default CRS
}

void QgsMemoryProvider::handlePostCloneOperations( QgsVectorDataProvider *source )
{
  if ( QgsMemoryProvider *other = qobject_cast< QgsMemoryProvider * >( source ) )
  {
    // these properties aren't copied when cloning a memory provider by uri, so we need to do it manually
    mFeatures = other->mFeatures;
    mNextFeatureId = other->mNextFeatureId;
    mExtent = other->mExtent;
  }
}

// returns TRUE if all features were added successfully, or FALSE if any feature could not be added
bool QgsMemoryProvider::addFeatures( QgsFeatureList &flist, Flags flags )
{
  bool result = true;
  // whether or not to update the layer extent on the fly as we add features
  const bool updateExtent = mFeatures.isEmpty() || !mExtent.isEmpty();

  const int fieldCount = mFields.count();

  // For rollback
  const auto oldExtent { mExtent };
  const auto oldNextFeatureId { mNextFeatureId };
  QgsFeatureIds addedFids ;

  for ( QgsFeatureList::iterator it = flist.begin(); it != flist.end() && result ; ++it )
  {
    it->setId( mNextFeatureId );
    it->setValid( true );
    const int attributeCount = it->attributeCount();
    if ( attributeCount < fieldCount )
    {
      // ensure features have the correct number of attributes by padding
      // them with null attributes for missing values
      QgsAttributes attributes = it->attributes();
      for ( int i = attributeCount; i < mFields.count(); ++i )
      {
        attributes.append( QgsVariantUtils::createNullVariant( mFields.at( i ).type() ) );
      }
      it->setAttributes( attributes );
    }
    else if ( attributeCount > fieldCount )
    {
      // too many attributes
      pushError( tr( "Feature has too many attributes (expecting %1, received %2)" ).arg( fieldCount ).arg( attributeCount ) );
      QgsAttributes attributes = it->attributes();
      attributes.resize( mFields.count() );
      it->setAttributes( attributes );
    }

    if ( it->hasGeometry() && mWkbType == Qgis::WkbType::NoGeometry )
    {
      it->clearGeometry();
    }
    else if ( it->hasGeometry() && QgsWkbTypes::geometryType( it->geometry().wkbType() ) !=
              QgsWkbTypes::geometryType( mWkbType ) )
    {
      pushError( tr( "Could not add feature with geometry type %1 to layer of type %2" ).arg( QgsWkbTypes::displayString( it->geometry().wkbType() ),
                 QgsWkbTypes::displayString( mWkbType ) ) );
      result = false;
      continue;
    }

    // Check attribute conversion
    bool conversionError { false };
    QString errorMessage;
    for ( int i = 0; i < mFields.count(); ++i )
    {
      const QVariant originalValue = it->attribute( i );
      QVariant attrValue = originalValue;
      if ( ! QgsVariantUtils::isNull( attrValue ) && ! mFields.at( i ).convertCompatible( attrValue, &errorMessage ) )
      {
        // Push first conversion error only
        if ( result )
        {
          pushError( tr( "Could not store attribute \"%1\": %2" )
                     .arg( mFields.at( i ).name(), errorMessage ) );
        }
        result = false;
        conversionError = true;
        continue;
      }
      else if ( attrValue.userType() != originalValue.userType() )
      {
        // convertCompatible has resulted in a data type conversion
        it->setAttribute( i, attrValue );
      }
    }

    // Skip the feature if there is at least one conversion error
    if ( conversionError )
    {
      if ( flags.testFlag( QgsFeatureSink::Flag::RollBackOnErrors ) )
      {
        break;
      }
      continue;
    }

    mFeatures.insert( mNextFeatureId, *it );
    addedFids.insert( mNextFeatureId );

    if ( it->hasGeometry() )
    {
      if ( updateExtent )
        mExtent.combineExtentWith( it->geometry().boundingBox() );

      // update spatial index
      if ( mSpatialIndex )
        mSpatialIndex->addFeature( *it );
    }

    mNextFeatureId++;
  }

  // Roll back
  if ( ! result && flags.testFlag( QgsFeatureSink::Flag::RollBackOnErrors ) )
  {
    for ( const QgsFeatureId &addedFid : addedFids )
    {
      mFeatures.remove( addedFid );
    }
    mExtent = oldExtent;
    mNextFeatureId = oldNextFeatureId;
  }
  else
  {
    clearMinMaxCache();
  }

  return result;
}

bool QgsMemoryProvider::deleteFeatures( const QgsFeatureIds &id )
{
  for ( QgsFeatureIds::const_iterator it = id.begin(); it != id.end(); ++it )
  {
    const QgsFeatureMap::iterator fit = mFeatures.find( *it );

    // check whether such feature exists
    if ( fit == mFeatures.end() )
      continue;

    // update spatial index
    if ( mSpatialIndex )
      mSpatialIndex->deleteFeature( *fit );

    mFeatures.erase( fit );
  }

  updateExtents();
  clearMinMaxCache();

  return true;
}

bool QgsMemoryProvider::addAttributes( const QList<QgsField> &attributes )
{
  bool fieldWasAdded { false };
  for ( QgsField field : attributes )
  {
    if ( !supportedType( field ) )
      continue;

    // Make sure added attributes typeName correspond to a native type name
    bool isNativeTypeName = false;
    NativeType nativeTypeCandidate( QString(), QString(), QMetaType::Type::UnknownType );
    const QList<QgsVectorDataProvider::NativeType> nativeTypesList( nativeTypes() );
    for ( const NativeType &nativeType : nativeTypesList )
    {
      if ( nativeType.mTypeName.toLower() == field.typeName().toLower() )
      {
        isNativeTypeName = true;
        break;
      }

      if ( nativeType.mType == field.type()
           && nativeTypeCandidate.mType == QMetaType::Type::UnknownType )
        nativeTypeCandidate = nativeType;
    }
    if ( !isNativeTypeName )
    {
      if ( nativeTypeCandidate.mType == QMetaType::Type::UnknownType )
      {
        QgsLogger::warning( "Field type not supported: " + field.typeName() );
        continue;
      }

      field.setTypeName( nativeTypeCandidate.mTypeName );
    }

    // add new field as a last one
    mFields.append( field );
    fieldWasAdded = true;

    for ( QgsFeatureMap::iterator fit = mFeatures.begin(); fit != mFeatures.end(); ++fit )
    {
      QgsFeature &f = fit.value();
      QgsAttributes attr = f.attributes();
      attr.append( QVariant() );
      f.setAttributes( attr );
    }
  }
  return fieldWasAdded;
}

bool QgsMemoryProvider::renameAttributes( const QgsFieldNameMap &renamedAttributes )
{
  QgsFieldNameMap::const_iterator renameIt = renamedAttributes.constBegin();
  bool result = true;
  for ( ; renameIt != renamedAttributes.constEnd(); ++renameIt )
  {
    const int fieldIndex = renameIt.key();
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

    mFields.rename( fieldIndex, renameIt.value() );
  }
  return result;
}

bool QgsMemoryProvider::deleteAttributes( const QgsAttributeIds &attributes )
{
  QList<int> attrIdx( attributes.begin(), attributes.end() );
  std::sort( attrIdx.begin(), attrIdx.end(), std::greater<int>() );

  // delete attributes one-by-one with decreasing index
  for ( QList<int>::const_iterator it = attrIdx.constBegin(); it != attrIdx.constEnd(); ++it )
  {
    const int idx = *it;
    mFields.remove( idx );

    for ( QgsFeatureMap::iterator fit = mFeatures.begin(); fit != mFeatures.end(); ++fit )
    {
      QgsFeature &f = fit.value();
      QgsAttributes attr = f.attributes();
      attr.remove( idx );
      f.setAttributes( attr );
    }
  }
  clearMinMaxCache();
  return true;
}

bool QgsMemoryProvider::changeAttributeValues( const QgsChangedAttributesMap &attr_map )
{
  bool result { true };

  QgsChangedAttributesMap rollBackMap;

  QString errorMessage;
  for ( QgsChangedAttributesMap::const_iterator it = attr_map.begin(); it != attr_map.end(); ++it )
  {
    const QgsFeatureMap::iterator fit = mFeatures.find( it.key() );
    if ( fit == mFeatures.end() )
      continue;

    const QgsAttributeMap &attrs = it.value();
    QgsAttributeMap rollBackAttrs;

    // Break on errors
    for ( QgsAttributeMap::const_iterator it2 = attrs.constBegin(); it2 != attrs.constEnd(); ++it2 )
    {
      const int fieldIndex = it2.key();
      if ( fieldIndex < 0 || fieldIndex >= mFields.count() )
        continue;

      QVariant attrValue = it2.value();
      if ( QgsVariantUtils::isUnsetAttributeValue( attrValue ) )
        continue;

      // Check attribute conversion
      const bool conversionError { ! QgsVariantUtils::isNull( attrValue )
                                   && ! mFields.at( it2.key() ).convertCompatible( attrValue, &errorMessage ) };
      if ( conversionError )
      {
        // Push first conversion error only
        if ( result )
        {
          pushError( tr( "Could not change attribute %1 having type %2 for feature %4: %3" )
                     .arg( mFields.at( it2.key() ).name(), it2.value( ).typeName(),
                           errorMessage ).arg( it.key() ) );
        }
        result = false;
        break;
      }
      rollBackAttrs.insert( it2.key(), fit->attribute( it2.key() ) );
      fit->setAttribute( it2.key(), attrValue );
    }
    rollBackMap.insert( it.key(), rollBackAttrs );
  }

  // Roll back
  if ( ! result )
  {
    changeAttributeValues( rollBackMap );
  }
  else
  {
    clearMinMaxCache();
  }
  return result;
}

bool QgsMemoryProvider::changeGeometryValues( const QgsGeometryMap &geometry_map )
{
  for ( QgsGeometryMap::const_iterator it = geometry_map.begin(); it != geometry_map.end(); ++it )
  {
    const QgsFeatureMap::iterator fit = mFeatures.find( it.key() );
    if ( fit == mFeatures.end() )
      continue;

    // update spatial index
    if ( mSpatialIndex )
      mSpatialIndex->deleteFeature( *fit );

    fit->setGeometry( it.value() );

    // update spatial index
    if ( mSpatialIndex )
      mSpatialIndex->addFeature( *fit );
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
  Q_UNUSED( updateFeatureCount )

  if ( !theSQL.isEmpty() )
  {
    const QgsExpression tempExpression( theSQL );
    if ( tempExpression.hasParserError() )
      return false;
  }

  if ( theSQL == mSubsetString )
    return true;

  mSubsetString = theSQL;
  clearMinMaxCache();
  mExtent.setNull();

  emit dataChanged();
  return true;
}

bool QgsMemoryProvider::supportsSubsetString() const
{
  return true;
}

QString QgsMemoryProvider::subsetStringDialect() const
{
  return tr( "QGIS expression" );
}

QString QgsMemoryProvider::subsetStringHelpUrl() const
{
  // unfortunately we can't access QgsHelp here, that's a GUI class!
  return QString();
}

bool QgsMemoryProvider::createSpatialIndex()
{
  if ( !mSpatialIndex )
  {
    mSpatialIndex = std::make_unique<QgsSpatialIndex>();

    // add existing features to index
    for ( QgsFeatureMap::iterator it = mFeatures.begin(); it != mFeatures.end(); ++it )
    {
      mSpatialIndex->addFeature( *it );
    }
  }
  return true;
}

Qgis::SpatialIndexPresence QgsMemoryProvider::hasSpatialIndex() const
{
  return mSpatialIndex ? Qgis::SpatialIndexPresence::Present : Qgis::SpatialIndexPresence::NotPresent;
}

Qgis::VectorProviderCapabilities QgsMemoryProvider::capabilities() const
{
  Qgis::VectorProviderCapabilities caps { Qgis::VectorProviderCapability::AddFeatures | Qgis::VectorProviderCapability::DeleteFeatures |
                                          Qgis::VectorProviderCapability::ChangeAttributeValues | Qgis::VectorProviderCapability::AddAttributes | Qgis::VectorProviderCapability::DeleteAttributes | Qgis::VectorProviderCapability::RenameAttributes |
                                          Qgis::VectorProviderCapability::SelectAtId | Qgis::VectorProviderCapability::FastTruncate };
  if ( mWkbType != Qgis::WkbType::NoGeometry )
  {
    caps |=  Qgis::VectorProviderCapability::CreateSpatialIndex | Qgis::VectorProviderCapability::CircularGeometries | Qgis::VectorProviderCapability::ChangeGeometries ;
  }
  return caps;
}

bool QgsMemoryProvider::truncate()
{
  mFeatures.clear();
  clearMinMaxCache();
  mExtent.setNull();
  return true;
}

void QgsMemoryProvider::updateExtents()
{
  mExtent.setNull();
}

QString QgsMemoryProvider::name() const
{
  return TEXT_PROVIDER_KEY;
}

QString QgsMemoryProvider::description() const
{
  return TEXT_PROVIDER_DESCRIPTION;
}


QgsMemoryProviderMetadata::QgsMemoryProviderMetadata()
  : QgsProviderMetadata( QgsMemoryProvider::providerKey(), QgsMemoryProvider::providerDescription() )
{
}

QIcon QgsMemoryProviderMetadata::icon() const
{
  return QgsApplication::getThemeIcon( u"mIconMemory.svg"_s );
}

QgsDataProvider *QgsMemoryProviderMetadata::createProvider( const QString &uri, const QgsDataProvider::ProviderOptions &options, Qgis::DataProviderReadFlags flags )
{
  return new QgsMemoryProvider( uri, options, flags );
}

QList<Qgis::LayerType> QgsMemoryProviderMetadata::supportedLayerTypes() const
{
  return { Qgis::LayerType::Vector };
}

#undef TEXT_PROVIDER_KEY
#undef TEXT_PROVIDER_DESCRIPTION

///@endcond

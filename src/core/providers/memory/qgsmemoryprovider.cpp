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
#include "qgsvariantutils.h"

#include <QUrl>
#include <QUrlQuery>
#include <QRegularExpression>

///@cond PRIVATE

#define TEXT_PROVIDER_KEY QStringLiteral( "memory" )
#define TEXT_PROVIDER_DESCRIPTION QStringLiteral( "Memory provider" )

QgsMemoryProvider::QgsMemoryProvider( const QString &uri, const ProviderOptions &options, QgsDataProvider::ReadFlags flags )
  : QgsVectorDataProvider( uri, options, flags )
{
  // Initialize the geometry with the uri to support old style uri's
  // (ie, just 'point', 'line', 'polygon')
  const QUrl url = QUrl::fromEncoded( uri.toUtf8() );
  const QUrlQuery query( url );
  QString geometry;
  if ( query.hasQueryItem( QStringLiteral( "geometry" ) ) )
  {
    geometry = query.queryItemValue( QStringLiteral( "geometry" ) );
  }
  else
  {
    geometry = url.path();
  }

  if ( geometry.compare( QLatin1String( "none" ), Qt::CaseInsensitive ) == 0 )
  {
    mWkbType = QgsWkbTypes::NoGeometry;
  }
  else
  {
    mWkbType = QgsWkbTypes::parseType( geometry );
  }

  if ( query.hasQueryItem( QStringLiteral( "crs" ) ) )
  {
    const QString crsDef = query.queryItemValue( QStringLiteral( "crs" ) );
    mCrs.createFromString( crsDef );
  }
  else
  {
    // TODO - remove in QGIS 4.0. Layers without an explicit CRS set SHOULD have an invalid CRS. But in order to maintain
    // 3.x api, we have to be tolerant/shortsighted(?) here and fallback to EPSG:4326
    mCrs = QgsCoordinateReferenceSystem( QStringLiteral( "EPSG:4326" ) );
  }

  mNextFeatureId = 1;

  setNativeTypes( QList< NativeType >()
                  << QgsVectorDataProvider::NativeType( tr( "Whole Number (integer)" ), QStringLiteral( "integer" ), QVariant::Int, 0, 10 )
                  // Decimal number from OGR/Shapefile/dbf may come with length up to 32 and
                  // precision up to length-2 = 30 (default, if width is not specified in dbf is length = 24 precision = 15)
                  // We know that double (QVariant::Double) has only 15-16 significant numbers,
                  // but setting that correct limits would disable the use of memory provider with
                  // data from Shapefiles. In any case, the data are handled as doubles.
                  // So the limits set here are not correct but enable use of data from Shapefiles.
                  << QgsVectorDataProvider::NativeType( tr( "Decimal Number (real)" ), QStringLiteral( "double" ), QVariant::Double, 0, 32, 0, 30 )
                  << QgsVectorDataProvider::NativeType( tr( "Text (string)" ), QStringLiteral( "string" ), QVariant::String, 0, 255 )

                  // date type
                  << QgsVectorDataProvider::NativeType( QgsVariantUtils::typeToDisplayString( QVariant::Date ), QStringLiteral( "date" ), QVariant::Date, -1, -1, -1, -1 )
                  << QgsVectorDataProvider::NativeType( QgsVariantUtils::typeToDisplayString( QVariant::Time ), QStringLiteral( "time" ), QVariant::Time, -1, -1, -1, -1 )
                  << QgsVectorDataProvider::NativeType( QgsVariantUtils::typeToDisplayString( QVariant::DateTime ), QStringLiteral( "datetime" ), QVariant::DateTime, -1, -1, -1, -1 )

                  // integer types
                  << QgsVectorDataProvider::NativeType( tr( "Whole Number (smallint - 16bit)" ), QStringLiteral( "int2" ), QVariant::Int, -1, -1, 0, 0 )
                  << QgsVectorDataProvider::NativeType( tr( "Whole Number (integer - 32bit)" ), QStringLiteral( "int4" ), QVariant::Int, -1, -1, 0, 0 )
                  << QgsVectorDataProvider::NativeType( tr( "Whole Number (integer - 64bit)" ), QStringLiteral( "int8" ), QVariant::LongLong, -1, -1, 0, 0 )
                  << QgsVectorDataProvider::NativeType( tr( "Decimal Number (numeric)" ), QStringLiteral( "numeric" ), QVariant::Double, 1, 20, 0, 20 )
                  << QgsVectorDataProvider::NativeType( tr( "Decimal Number (decimal)" ), QStringLiteral( "decimal" ), QVariant::Double, 1, 20, 0, 20 )

                  // floating point
                  << QgsVectorDataProvider::NativeType( tr( "Decimal Number (real)" ), QStringLiteral( "real" ), QVariant::Double, -1, -1, -1, -1 )
                  << QgsVectorDataProvider::NativeType( tr( "Decimal Number (double)" ), QStringLiteral( "double precision" ), QVariant::Double, -1, -1, -1, -1 )

                  // string types
                  << QgsVectorDataProvider::NativeType( tr( "Text, unlimited length (text)" ), QStringLiteral( "text" ), QVariant::String, -1, -1, -1, -1 )

                  // boolean
                  << QgsVectorDataProvider::NativeType( QgsVariantUtils::typeToDisplayString( QVariant::Bool ), QStringLiteral( "boolean" ), QVariant::Bool )

                  // blob
                  << QgsVectorDataProvider::NativeType( QgsVariantUtils::typeToDisplayString( QVariant::ByteArray ), QStringLiteral( "binary" ), QVariant::ByteArray )

                  // list types
                  << QgsVectorDataProvider::NativeType( QgsVariantUtils::typeToDisplayString( QVariant::StringList ), QStringLiteral( "stringlist" ), QVariant::StringList, 0, 0, 0, 0, QVariant::String )
                  << QgsVectorDataProvider::NativeType( QgsVariantUtils::typeToDisplayString( QVariant::List, QVariant::Int ), QStringLiteral( "integerlist" ), QVariant::List, 0, 0, 0, 0, QVariant::Int )
                  << QgsVectorDataProvider::NativeType( QgsVariantUtils::typeToDisplayString( QVariant::List, QVariant::Double ), QStringLiteral( "doublelist" ), QVariant::List, 0, 0, 0, 0, QVariant::Double )
                  << QgsVectorDataProvider::NativeType( QgsVariantUtils::typeToDisplayString( QVariant::List, QVariant::LongLong ), QStringLiteral( "integer64list" ), QVariant::List, 0, 0, 0, 0, QVariant::LongLong )

                );

  if ( query.hasQueryItem( QStringLiteral( "field" ) ) )
  {
    QList<QgsField> attributes;
    const thread_local QRegularExpression reFieldDef( "\\:"
        "([\\w\\s]+)"                // type
        "(?:\\((\\-?\\d+)"           // length
        "(?:\\,(\\-?\\d+))?"         // precision
        "\\))?(\\[\\])?"             // array
        "$",
        QRegularExpression::CaseInsensitiveOption );
    const QStringList fields = query.allQueryItemValues( QStringLiteral( "field" ) );
    for ( int i = 0; i < fields.size(); i++ )
    {
      QString name = QUrl::fromPercentEncoding( fields.at( i ).toUtf8() );
      const QRegularExpressionMatch regularExpressionMatch = reFieldDef.match( name );

      // If no match -> use string as type
      QVariant::Type type = QVariant::String;
      QVariant::Type subType = QVariant::Invalid;
      QString typeName( QStringLiteral( "string" ) );
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
          if ( typeName == QLatin1String( "int" ) )
          {
            type = QVariant::Int;
            typeName = QStringLiteral( "integer" );
          }
          else if ( typeName == QLatin1String( "long" ) )
          {
            type = QVariant::LongLong;
            typeName = QStringLiteral( "int8" );
          }
          else if ( typeName == QLatin1String( "bool" ) )
          {
            type = QVariant::Bool;
            typeName = QStringLiteral( "boolean" );
          }
          else
          {
            QgsLogger::warning( tr( "Unsupported typeName '%1'. Will be handled as string." ).arg( typeName ) );
            type = QVariant::String;
            typeName = QStringLiteral( "string" );
          }
        }

        // Set default length/precision for double/real
        if ( typeName == QLatin1String( "real" ) || typeName == QLatin1String( "double" ) )
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
          if ( subType == QVariant::Invalid )
            subType = type;

          if ( type != QVariant::List && type != QVariant::StringList )
            type = type == QVariant::String ? QVariant::StringList : QVariant::List;

          const QLatin1String listSuffix( "list" );
          if ( !typeName.endsWith( listSuffix ) )
            typeName += QLatin1String( "list" );
        }
      }

      attributes.append( QgsField( name, type, typeName, length, precision, QString(), subType ) );
    }
    addAttributes( attributes );
  }

  if ( query.hasQueryItem( QStringLiteral( "index" ) ) && query.queryItemValue( QStringLiteral( "index" ) ) == QLatin1String( "yes" ) )
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

QgsMemoryProvider *QgsMemoryProvider::createProvider( const QString &uri,
    const ProviderOptions &options,
    QgsDataProvider::ReadFlags flags )
{
  return new QgsMemoryProvider( uri, options, flags );
}

QgsAbstractFeatureSource *QgsMemoryProvider::featureSource() const
{
  return new QgsMemoryFeatureSource( this );
}

QString QgsMemoryProvider::dataSourceUri( bool expandAuthConfig ) const
{
  Q_UNUSED( expandAuthConfig )

  QUrl uri( QStringLiteral( "memory" ) );
  QUrlQuery query;
  const QString geometry = QgsWkbTypes::displayString( mWkbType );
  query.addQueryItem( QStringLiteral( "geometry" ), geometry );

  if ( mCrs.isValid() )
  {
    QString crsDef;
    const QString authid = mCrs.authid();
    if ( authid.startsWith( QLatin1String( "EPSG:" ) ) )
    {
      crsDef = authid;
    }
    else
    {
      crsDef = QStringLiteral( "wkt:%1" ).arg( mCrs.toWkt( QgsCoordinateReferenceSystem::WKT_PREFERRED ) );
    }
    query.addQueryItem( QStringLiteral( "crs" ), crsDef );
  }
  if ( mSpatialIndex )
  {
    query.addQueryItem( QStringLiteral( "index" ), QStringLiteral( "yes" ) );
  }

  QgsAttributeList attrs = const_cast<QgsMemoryProvider *>( this )->attributeIndexes();
  for ( int i = 0; i < attrs.size(); i++ )
  {
    const QgsField field = mFields.at( attrs[i] );
    QString fieldDef = field.name();

    QString typeName = field.typeName();
    bool isList = false;
    if ( field.type() == QVariant::List || field.type() == QVariant::StringList )
    {
      switch ( field.subType() )
      {
        case QVariant::Int:
          typeName = QStringLiteral( "integer" );
          break;

        case QVariant::LongLong:
          typeName = QStringLiteral( "long" );
          break;

        case QVariant::Double:
          typeName = QStringLiteral( "double" );
          break;

        case QVariant::String:
          typeName = QStringLiteral( "string" );
          break;

        default:
          break;
      }
      isList = true;
    }

    fieldDef.append( QStringLiteral( ":%2(%3,%4)%5" ).arg( typeName ).arg( field.length() ).arg( field.precision() ).arg( isList ? QStringLiteral( "[]" ) : QString() ) );
    query.addQueryItem( QStringLiteral( "field" ), fieldDef );
  }
  uri.setQuery( query );

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
    mExtent.setMinimal();
  }

  return mExtent;
}

QgsWkbTypes::Type QgsMemoryProvider::wkbType() const
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
  return ( mWkbType != QgsWkbTypes::Unknown );
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
    if ( it->attributes().count() < fieldCount )
    {
      // ensure features have the correct number of attributes by padding
      // them with null attributes for missing values
      QgsAttributes attributes = it->attributes();
      for ( int i = it->attributes().count(); i < mFields.count(); ++i )
      {
        attributes.append( QVariant( mFields.at( i ).type() ) );
      }
      it->setAttributes( attributes );
    }
    else if ( it->attributes().count() > fieldCount )
    {
      // too many attributes
      pushError( tr( "Feature has too many attributes (expecting %1, received %2)" ).arg( fieldCount ).arg( it->attributes().count() ) );
      QgsAttributes attributes = it->attributes();
      attributes.resize( mFields.count() );
      it->setAttributes( attributes );
    }

    if ( it->hasGeometry() && mWkbType == QgsWkbTypes::NoGeometry )
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
      if ( ! attrValue.isNull() && ! mFields.at( i ).convertCompatible( attrValue, &errorMessage ) )
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
      else if ( attrValue.type() != originalValue.type() )
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
  for ( QgsField field : attributes )
  {
    if ( !supportedType( field ) )
      continue;

    // Make sure added attributes typeName correspond to a native type name
    bool isNativeTypeName = false;
    NativeType nativeTypeCandidate( QString(), QString(), QVariant::Invalid );
    const QList<QgsVectorDataProvider::NativeType> nativeTypesList( nativeTypes() );
    for ( const NativeType &nativeType : nativeTypesList )
    {
      if ( nativeType.mTypeName.toLower() == field.typeName().toLower() )
      {
        isNativeTypeName = true;
        break;
      }

      if ( nativeType.mType == field.type()
           && nativeTypeCandidate.mType == QVariant::Invalid )
        nativeTypeCandidate = nativeType;
    }
    if ( !isNativeTypeName )
    {
      if ( nativeTypeCandidate.mType == QVariant::Invalid )
      {
        QgsLogger::warning( "Field type not supported: " + field.typeName() );
        continue;
      }

      field.setTypeName( nativeTypeCandidate.mTypeName );
    }

    // add new field as a last one
    mFields.append( field );

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
  QList<int> attrIdx = qgis::setToList( attributes );
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
      QVariant attrValue = it2.value();
      // Check attribute conversion
      const bool conversionError { ! attrValue.isNull()
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
  mExtent.setMinimal();

  emit dataChanged();
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
      mSpatialIndex->addFeature( *it );
    }
  }
  return true;
}

QgsFeatureSource::SpatialIndexPresence QgsMemoryProvider::hasSpatialIndex() const
{
  return mSpatialIndex ? SpatialIndexPresent : SpatialIndexNotPresent;
}

QgsVectorDataProvider::Capabilities QgsMemoryProvider::capabilities() const
{
  return AddFeatures | DeleteFeatures | ChangeGeometries |
         ChangeAttributeValues | AddAttributes | DeleteAttributes | RenameAttributes | CreateSpatialIndex |
         SelectAtId | CircularGeometries | FastTruncate;
}

bool QgsMemoryProvider::truncate()
{
  mFeatures.clear();
  clearMinMaxCache();
  mExtent.setMinimal();
  return true;
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

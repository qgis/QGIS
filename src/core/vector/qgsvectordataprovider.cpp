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

#include <QTextCodec>

#include <limits>

#include "qgsvectordataprovider.h"
#include "moc_qgsvectordataprovider.cpp"
#include "qgscircularstring.h"
#include "qgscompoundcurve.h"
#include "qgsfeature.h"
#include "qgsfeatureiterator.h"
#include "qgsfeaturerequest.h"
#include "qgsfeedback.h"
#include "qgsfields.h"
#include "qgsgeometry.h"
#include "qgsgeometrycollection.h"
#include "qgsgeometryfactory.h"
#include "qgslogger.h"
#include "qgsmessagelog.h"
#include "qgsogrproxytextcodec.h"
#include "qgsthreadingutils.h"
#include <mutex>
#include "qgsdataproviderelevationproperties.h"

QgsVectorDataProvider::QgsVectorDataProvider( const QString &uri, const ProviderOptions &options,
    Qgis::DataProviderReadFlags flags )
  : QgsDataProvider( uri, options, flags )
  , mTemporalCapabilities( std::make_unique< QgsVectorDataProviderTemporalCapabilities >() )
  , mElevationProperties( std::make_unique< QgsDataProviderElevationProperties >() )
{
}

QString QgsVectorDataProvider::storageType() const
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  return QStringLiteral( "Generic vector file" );
}

bool QgsVectorDataProvider::empty() const
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  QgsFeature f;
  QgsFeatureRequest request;
  request.setNoAttributes();
  request.setFlags( Qgis::FeatureRequestFlag::NoGeometry );
  request.setLimit( 1 );
  if ( getFeatures( request ).nextFeature( f ) )
    return false;
  else
    return true;
}

bool QgsVectorDataProvider::isSqlQuery() const
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  return vectorLayerTypeFlags().testFlag( Qgis::VectorLayerTypeFlag::SqlQuery );
}

Qgis::VectorLayerTypeFlags QgsVectorDataProvider::vectorLayerTypeFlags() const
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  return Qgis::VectorLayerTypeFlags();
}

Qgis::FeatureAvailability QgsVectorDataProvider::hasFeatures() const
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  if ( empty() )
    return Qgis::FeatureAvailability::NoFeaturesAvailable;
  else
    return Qgis::FeatureAvailability::FeaturesAvailable;
}

QgsCoordinateReferenceSystem QgsVectorDataProvider::sourceCrs() const
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  return crs();
}

QgsRectangle QgsVectorDataProvider::sourceExtent() const
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  return extent();
}

QgsBox3D QgsVectorDataProvider::sourceExtent3D() const
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  return extent3D();
}

QString QgsVectorDataProvider::dataComment() const
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  return QString();
}

bool QgsVectorDataProvider::addFeatures( QgsFeatureList &flist, Flags flags )
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  Q_UNUSED( flist )
  Q_UNUSED( flags )
  return false;
}

QString QgsVectorDataProvider::lastError() const
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  return mErrors.isEmpty() ? QString() : mErrors.last();
}

bool QgsVectorDataProvider::deleteFeatures( const QgsFeatureIds &ids )
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  Q_UNUSED( ids )
  return false;
}

bool QgsVectorDataProvider::truncate()
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  if ( !( capabilities() & Qgis::VectorProviderCapability::DeleteFeatures ) )
    return false;

  QgsFeatureIds toDelete;
  QgsFeatureIterator it = getFeatures( QgsFeatureRequest().setFlags( Qgis::FeatureRequestFlag::NoGeometry ).setNoAttributes() );
  QgsFeature f;
  while ( it.nextFeature( f ) )
    toDelete << f.id();

  return deleteFeatures( toDelete );
}

bool QgsVectorDataProvider::addAttributes( const QList<QgsField> &attributes )
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  Q_UNUSED( attributes )
  return false;
}

bool QgsVectorDataProvider::deleteAttributes( const QgsAttributeIds &attributes )
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  Q_UNUSED( attributes )
  return false;
}

bool QgsVectorDataProvider::renameAttributes( const QgsFieldNameMap &renamedAttributes )
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  Q_UNUSED( renamedAttributes )
  return false;
}

bool QgsVectorDataProvider::changeAttributeValues( const QgsChangedAttributesMap &attr_map )
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  Q_UNUSED( attr_map )
  return false;
}

QVariant QgsVectorDataProvider::defaultValue( int fieldId ) const
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  Q_UNUSED( fieldId )
  return QVariant();
}

QString QgsVectorDataProvider::defaultValueClause( int fieldIndex ) const
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  Q_UNUSED( fieldIndex )
  return QString();
}

QgsFieldConstraints::Constraints QgsVectorDataProvider::fieldConstraints( int fieldIndex ) const
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  const QgsFields f = fields();
  if ( fieldIndex < 0 || fieldIndex >= f.count() )
    return QgsFieldConstraints::Constraints();

  return f.at( fieldIndex ).constraints().constraints();
}

bool QgsVectorDataProvider::skipConstraintCheck( int, QgsFieldConstraints::Constraint, const QVariant & ) const
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  return false;
}

bool QgsVectorDataProvider::changeGeometryValues( const QgsGeometryMap &geometry_map )
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  Q_UNUSED( geometry_map )
  return false;
}

bool QgsVectorDataProvider::changeFeatures( const QgsChangedAttributesMap &attr_map,
    const QgsGeometryMap &geometry_map )
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  if ( !( capabilities() & Qgis::VectorProviderCapability::ChangeAttributeValues ) || !( capabilities() & Qgis::VectorProviderCapability::ChangeGeometries ) )
    return false;

  bool result = true;
  result = result && changeAttributeValues( attr_map );
  result = result && changeGeometryValues( geometry_map );
  return result;
}

bool QgsVectorDataProvider::createSpatialIndex()
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  return false;
}

bool QgsVectorDataProvider::createAttributeIndex( int field )
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  Q_UNUSED( field )
  return true;
}

Qgis::VectorProviderCapabilities QgsVectorDataProvider::capabilities() const
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  return Qgis::VectorProviderCapability::NoCapabilities;
}

void QgsVectorDataProvider::setEncoding( const QString &e )
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  // Use UTF-8 if no encoding is specified
  if ( e.isEmpty() )
  {
    mEncoding = QTextCodec::codecForName( "UTF-8" );
  }
  else
  {
    mEncoding = QTextCodec::codecForName( e.toLocal8Bit().constData() );
  }
  if ( !mEncoding && e != QLatin1String( "System" ) )
  {
    if ( !e.isEmpty() )
    {
      // can we use the OGR proxy codec?
      if ( QgsOgrProxyTextCodec::supportedCodecs().contains( e, Qt::CaseInsensitive ) )
      {
        //from the Qt docs (https://doc.qt.io/qt-5/qtextcodec.html#QTextCodec-1)
        // "The QTextCodec should always be constructed on the heap (i.e. with new).
        // Qt takes ownership and will delete it when the application terminates."
        mEncoding = new QgsOgrProxyTextCodec( e.toLocal8Bit() );
      }
      else
      {
        QgsMessageLog::logMessage( tr( "Codec %1 not found. Falling back to system locale" ).arg( e ) );
        mEncoding = QTextCodec::codecForName( "System" );
      }
    }
  }

  if ( !mEncoding )
    mEncoding = QTextCodec::codecForLocale();

  Q_ASSERT( mEncoding );
}

QString QgsVectorDataProvider::encoding() const
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  if ( mEncoding )
  {
    return mEncoding->name();
  }

  return QString();
}

QString QgsVectorDataProvider::capabilitiesString() const
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  QStringList abilitiesList;

  const Qgis::VectorProviderCapabilities abilities = capabilities();

  if ( abilities & Qgis::VectorProviderCapability::AddFeatures )
  {
    abilitiesList += tr( "Add Features" );
  }

  if ( abilities & Qgis::VectorProviderCapability::DeleteFeatures )
  {
    abilitiesList += tr( "Delete Features" );
  }

  if ( abilities & Qgis::VectorProviderCapability::ChangeAttributeValues )
  {
    abilitiesList += tr( "Change Attribute Values" );
  }

  if ( abilities & Qgis::VectorProviderCapability::AddAttributes )
  {
    abilitiesList += tr( "Add Attributes" );
  }

  if ( abilities & Qgis::VectorProviderCapability::DeleteAttributes )
  {
    abilitiesList += tr( "Delete Attributes" );
  }

  if ( abilities & Qgis::VectorProviderCapability::RenameAttributes )
  {
    abilitiesList += tr( "Rename Attributes" );
  }

  if ( abilities & Qgis::VectorProviderCapability::CreateSpatialIndex )
  {
    // TODO: Tighten up this test.  See QgsOgrProvider for details.
    abilitiesList += tr( "Create Spatial Index" );
  }

  if ( abilities & Qgis::VectorProviderCapability::CreateAttributeIndex )
  {
    abilitiesList += tr( "Create Attribute Indexes" );
  }

  if ( abilities & Qgis::VectorProviderCapability::SelectAtId )
  {
    abilitiesList += tr( "Fast Access to Features at ID" );
  }

  if ( abilities & Qgis::VectorProviderCapability::ChangeGeometries )
  {
    abilitiesList += tr( "Change Geometries" );
  }

  if ( abilities & Qgis::VectorProviderCapability::SimplifyGeometries )
  {
    abilitiesList += tr( "Presimplify Geometries" );
  }

  if ( abilities & Qgis::VectorProviderCapability::SimplifyGeometriesWithTopologicalValidation )
  {
    abilitiesList += tr( "Presimplify Geometries with Validity Check" );
  }

  if ( abilities & Qgis::VectorProviderCapability::ChangeFeatures )
  {
    abilitiesList += tr( "Simultaneous Geometry and Attribute Updates" );
  }

  if ( abilities & Qgis::VectorProviderCapability::TransactionSupport )
  {
    abilitiesList += tr( "Transactions" );
  }

  if ( abilities & Qgis::VectorProviderCapability::CircularGeometries )
  {
    abilitiesList += tr( "Curved Geometries" );
  }

  if ( abilities & Qgis::VectorProviderCapability::FeatureSymbology )
  {
    abilitiesList += tr( "Feature Symbology" );
  }

  return abilitiesList.join( QLatin1String( ", " ) );
}

Qgis::VectorDataProviderAttributeEditCapabilities QgsVectorDataProvider::attributeEditCapabilities() const
{
  return Qgis::VectorDataProviderAttributeEditCapabilities();
}

int QgsVectorDataProvider::fieldNameIndex( const QString &fieldName ) const
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  return fields().lookupField( fieldName );
}

QMap<QString, int> QgsVectorDataProvider::fieldNameMap() const
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  QMap<QString, int> resultMap;

  const QgsFields fieldsCopy = fields();
  for ( int i = 0; i < fieldsCopy.count(); ++i )
  {
    resultMap.insert( fieldsCopy.at( i ).name(), i );
  }

  return resultMap;
}

QgsAttributeList QgsVectorDataProvider::attributeIndexes() const
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  return fields().allAttributesList();
}

QgsAttributeList QgsVectorDataProvider::pkAttributeIndexes() const
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  return QgsAttributeList();
}

QString QgsVectorDataProvider::geometryColumnName() const
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  return QString();
}

QList<QgsVectorDataProvider::NativeType> QgsVectorDataProvider::nativeTypes() const
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  return mNativeTypes;
}

QgsAttrPalIndexNameHash QgsVectorDataProvider::palAttributeIndexNames() const
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  return QgsAttrPalIndexNameHash();
}

bool QgsVectorDataProvider::supportedType( const QgsField &field ) const
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  QgsDebugMsgLevel( QStringLiteral( "field name = %1 type = %2 length = %3 precision = %4" )
                    .arg( field.name(),
                          QVariant::typeToName( field.type() ) )
                    .arg( field.length() )
                    .arg( field.precision() ), 2 );

  for ( const NativeType &nativeType : mNativeTypes )
  {
    QgsDebugMsgLevel( QStringLiteral( "native field type = %1 min length = %2 max length = %3 min precision = %4 max precision = %5" )
                      .arg( QVariant::typeToName( nativeType.mType ) )
                      .arg( nativeType.mMinLen )
                      .arg( nativeType.mMaxLen )
                      .arg( nativeType.mMinPrec )
                      .arg( nativeType.mMaxPrec ), 2 );

    if ( field.type() != nativeType.mType )
      continue;

    if ( field.length() > 0 )
    {
      // source length limited
      if ( ( nativeType.mMinLen > 0 && field.length() < nativeType.mMinLen ) ||
           ( nativeType.mMaxLen > 0 && field.length() > nativeType.mMaxLen ) )
      {
        // source length exceeds destination limits
        continue;
      }
    }

    if ( field.precision() > 0 )
    {
      // source precision limited
      if ( ( nativeType.mMinPrec > 0 && field.precision() < nativeType.mMinPrec ) ||
           ( nativeType.mMaxPrec > 0 && field.precision() > nativeType.mMaxPrec ) )
      {
        // source precision exceeds destination limits
        continue;
      }
    }

    QgsDebugMsgLevel( QStringLiteral( "native type matches" ), 3 );
    return true;
  }

  QgsDebugError( QStringLiteral( "no sufficient native type found" ) );
  return false;
}

QVariant QgsVectorDataProvider::minimumValue( int index ) const
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  if ( index < 0 || index >= fields().count() )
  {
    QgsDebugError( "Warning: access requested to invalid field index: " + QString::number( index ) );
    return QVariant();
  }

  fillMinMaxCache();

  if ( !mCacheMinValues.contains( index ) )
    return QVariant();

  return mCacheMinValues[index];
}

QVariant QgsVectorDataProvider::maximumValue( int index ) const
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  if ( index < 0 || index >= fields().count() )
  {
    QgsDebugError( "Warning: access requested to invalid field index: " + QString::number( index ) );
    return QVariant();
  }

  fillMinMaxCache();

  if ( !mCacheMaxValues.contains( index ) )
    return QVariant();

  return mCacheMaxValues[index];
}

QStringList QgsVectorDataProvider::uniqueStringsMatching( int index, const QString &substring, int limit, QgsFeedback *feedback ) const
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  QStringList results;

  // Safety belt
  if ( index < 0 || index >= fields().count() )
    return results;

  QgsFeature f;
  QgsAttributeList keys;
  keys.append( index );

  QgsFeatureRequest request;
  request.setSubsetOfAttributes( keys );
  request.setFlags( Qgis::FeatureRequestFlag::NoGeometry );
  const QString fieldName = fields().at( index ).name();
  request.setFilterExpression( QStringLiteral( "\"%1\" ILIKE '%%2%'" ).arg( fieldName, substring ) );
  QgsFeatureIterator fi = getFeatures( request );

  QSet<QString> set;

  while ( fi.nextFeature( f ) )
  {
    const QString value = f.attribute( index ).toString();
    if ( !set.contains( value ) )
    {
      results.append( value );
      set.insert( value );
    }

    if ( ( limit >= 0 && results.size() >= limit ) || ( feedback && feedback->isCanceled() ) )
      break;
  }
  return results;
}

QVariant QgsVectorDataProvider::aggregate( Qgis::Aggregate aggregate, int index,
    const QgsAggregateCalculator::AggregateParameters &parameters, QgsExpressionContext *context, bool &ok, QgsFeatureIds *fids ) const
{
  // non fatal for now -- the "aggregate" functions are not thread safe and call this
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS_NON_FATAL

  //base implementation does nothing
  Q_UNUSED( aggregate )
  Q_UNUSED( index )
  Q_UNUSED( parameters )
  Q_UNUSED( context )
  Q_UNUSED( fids )

  ok = false;
  return QVariant();
}

void QgsVectorDataProvider::clearMinMaxCache()
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  mCacheMinMaxDirty = true;
  mCacheMinValues.clear();
  mCacheMaxValues.clear();
}

void QgsVectorDataProvider::fillMinMaxCache() const
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  if ( !mCacheMinMaxDirty )
    return;

  const QgsFields flds = fields();
  for ( int i = 0; i < flds.count(); ++i )
  {
    if ( flds.at( i ).type() == QMetaType::Type::Int )
    {
      mCacheMinValues[i] = QVariant( std::numeric_limits<int>::max() );
      mCacheMaxValues[i] = QVariant( std::numeric_limits<int>::lowest() );
    }
    else if ( flds.at( i ).type() == QMetaType::Type::LongLong )
    {
      mCacheMinValues[i] = QVariant( std::numeric_limits<qlonglong>::max() );
      mCacheMaxValues[i] = QVariant( std::numeric_limits<qlonglong>::lowest() );
    }
    else if ( flds.at( i ).type() == QMetaType::Type::Double )
    {
      mCacheMinValues[i] = QVariant( std::numeric_limits<double>::max() );
      mCacheMaxValues[i] = QVariant( std::numeric_limits<double>::lowest() );

    }
    else
    {
      mCacheMinValues[i] = QVariant();
      mCacheMaxValues[i] = QVariant();
    }
  }

  QgsFeature f;
  const QgsAttributeList keys = mCacheMinValues.keys();
  QgsFeatureIterator fi = getFeatures( QgsFeatureRequest().setSubsetOfAttributes( keys )
                                       .setFlags( Qgis::FeatureRequestFlag::NoGeometry ) );

  while ( fi.nextFeature( f ) )
  {
    const QgsAttributes attrs = f.attributes();
    for ( const int attributeIndex : keys )
    {
      const QVariant &varValue = attrs.at( attributeIndex );

      if ( QgsVariantUtils::isNull( varValue ) )
        continue;

      switch ( flds.at( attributeIndex ).type() )
      {
        case QMetaType::Type::Int:
        {
          const int value = varValue.toInt();
          if ( value < mCacheMinValues[ attributeIndex ].toInt() )
            mCacheMinValues[ attributeIndex ] = value;
          if ( value > mCacheMaxValues[ attributeIndex ].toInt() )
            mCacheMaxValues[ attributeIndex ] = value;
          break;
        }
        case QMetaType::Type::LongLong:
        {
          const qlonglong value = varValue.toLongLong();
          if ( value < mCacheMinValues[ attributeIndex ].toLongLong() )
            mCacheMinValues[ attributeIndex ] = value;
          if ( value > mCacheMaxValues[ attributeIndex ].toLongLong() )
            mCacheMaxValues[ attributeIndex ] = value;
          break;
        }
        case QMetaType::Type::Double:
        {
          const double value = varValue.toDouble();
          if ( value < mCacheMinValues[ attributeIndex ].toDouble() )
            mCacheMinValues[attributeIndex ] = value;
          if ( value > mCacheMaxValues[ attributeIndex ].toDouble() )
            mCacheMaxValues[ attributeIndex ] = value;
          break;
        }
        case QMetaType::Type::QDateTime:
        {
          const QDateTime value = varValue.toDateTime();
          if ( value < mCacheMinValues[ attributeIndex ].toDateTime() || !mCacheMinValues[ attributeIndex ].isValid() )
            mCacheMinValues[attributeIndex ] = value;
          if ( value > mCacheMaxValues[ attributeIndex ].toDateTime() || !mCacheMaxValues[ attributeIndex ].isValid() )
            mCacheMaxValues[ attributeIndex ] = value;
          break;
        }
        case QMetaType::Type::QDate:
        {
          const QDate value = varValue.toDate();
          if ( value < mCacheMinValues[ attributeIndex ].toDate() || !mCacheMinValues[ attributeIndex ].isValid() )
            mCacheMinValues[attributeIndex ] = value;
          if ( value > mCacheMaxValues[ attributeIndex ].toDate() || !mCacheMaxValues[ attributeIndex ].isValid() )
            mCacheMaxValues[ attributeIndex ] = value;
          break;
        }
        case QMetaType::Type::QTime:
        {
          const QTime value = varValue.toTime();
          if ( value < mCacheMinValues[ attributeIndex ].toTime() || !mCacheMinValues[ attributeIndex ].isValid() )
            mCacheMinValues[attributeIndex ] = value;
          if ( value > mCacheMaxValues[ attributeIndex ].toTime() || !mCacheMaxValues[ attributeIndex ].isValid() )
            mCacheMaxValues[ attributeIndex ] = value;
          break;
        }
        default:
        {
          const QString value = varValue.toString();
          if ( QgsVariantUtils::isNull( mCacheMinValues[ attributeIndex ] ) || value < mCacheMinValues[attributeIndex ].toString() )
          {
            mCacheMinValues[attributeIndex] = value;
          }
          if ( QgsVariantUtils::isNull( mCacheMaxValues[attributeIndex] ) || value > mCacheMaxValues[attributeIndex].toString() )
          {
            mCacheMaxValues[attributeIndex] = value;
          }
          break;
        }
      }
    }
  }

  mCacheMinMaxDirty = false;
}

QVariant QgsVectorDataProvider::convertValue( QMetaType::Type type, const QString &value )
{
  QVariant v( value );

  if ( !v.convert( type ) || value.isNull() )
    v = QgsVariantUtils::createNullVariant( type );

  return v;
}

QVariant QgsVectorDataProvider::convertValue( QVariant::Type type, const QString &value )
{
  return convertValue( QgsVariantUtils::variantTypeToMetaType( type ), value );
}

QgsTransaction *QgsVectorDataProvider::transaction() const
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  return nullptr;
}

static bool _compareEncodings( const QString &s1, const QString &s2 )
{
  return s1.toLower() < s2.toLower();
}

static bool _removeDuplicateEncodings( const QString &s1, const QString &s2 )
{
  return s1.compare( s2, Qt::CaseInsensitive ) == 0;
}

QStringList QgsVectorDataProvider::availableEncodings()
{
  static std::once_flag initialized;
  std::call_once( initialized, [ = ]
  {
    const auto codecs { QTextCodec::availableCodecs() };
    for ( const QByteArray &codec : codecs )
    {
      sEncodings << codec;
    }
#if 0
    smEncodings << "BIG5";
    smEncodings << "BIG5-HKSCS";
    smEncodings << "EUCJP";
    smEncodings << "EUCKR";
    smEncodings << "GB2312";
    smEncodings << "GBK";
    smEncodings << "GB18030";
    smEncodings << "JIS7";
    smEncodings << "SHIFT-JIS";
    smEncodings << "TSCII";
    smEncodings << "UTF-8";
    smEncodings << "UTF-16";
    smEncodings << "KOI8-R";
    smEncodings << "KOI8-U";
    smEncodings << "ISO8859-1";
    smEncodings << "ISO8859-2";
    smEncodings << "ISO8859-3";
    smEncodings << "ISO8859-4";
    smEncodings << "ISO8859-5";
    smEncodings << "ISO8859-6";
    smEncodings << "ISO8859-7";
    smEncodings << "ISO8859-8";
    smEncodings << "ISO8859-8-I";
    smEncodings << "ISO8859-9";
    smEncodings << "ISO8859-10";
    smEncodings << "ISO8859-11";
    smEncodings << "ISO8859-12";
    smEncodings << "ISO8859-13";
    smEncodings << "ISO8859-14";
    smEncodings << "ISO8859-15";
    smEncodings << "IBM 850";
    smEncodings << "IBM 866";
    smEncodings << "CP874";
    smEncodings << "CP1250";
    smEncodings << "CP1251";
    smEncodings << "CP1252";
    smEncodings << "CP1253";
    smEncodings << "CP1254";
    smEncodings << "CP1255";
    smEncodings << "CP1256";
    smEncodings << "CP1257";
    smEncodings << "CP1258";
    smEncodings << "Apple Roman";
    smEncodings << "TIS-620";
    smEncodings << "System";
#endif

    // Do case-insensitive sorting of encodings then remove duplicates
    std::sort( sEncodings.begin(), sEncodings.end(), _compareEncodings );
    const auto last = std::unique( sEncodings.begin(), sEncodings.end(), _removeDuplicateEncodings );
    sEncodings.erase( last, sEncodings.end() );

  } );

  return sEncodings;
}

void QgsVectorDataProvider::clearErrors()
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  mErrors.clear();
}

bool QgsVectorDataProvider::hasErrors() const
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  return !mErrors.isEmpty();
}

QStringList QgsVectorDataProvider::errors() const
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  return mErrors;
}

QgsFeatureRenderer *QgsVectorDataProvider::createRenderer( const QVariantMap & ) const
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  return nullptr;
}

QgsAbstractVectorLayerLabeling *QgsVectorDataProvider::createLabeling( const QVariantMap & ) const
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  return nullptr;
}

void QgsVectorDataProvider::pushError( const QString &msg ) const
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  QgsDebugError( msg );
  mErrors << msg;
  emit raiseError( msg );
}

QSet<QgsMapLayerDependency> QgsVectorDataProvider::dependencies() const
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  return QSet<QgsMapLayerDependency>();
}

QgsGeometry QgsVectorDataProvider::convertToProviderType( const QgsGeometry &geom ) const
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  // Call the static version
  return QgsVectorDataProvider::convertToProviderType( geom, wkbType() );

}

void QgsVectorDataProvider::setNativeTypes( const QList<NativeType> &nativeTypes )
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  mNativeTypes = nativeTypes;
}

QTextCodec *QgsVectorDataProvider::textEncoding() const
{
  // non fatal for now -- the "rasterize" processing algorithm is not thread safe and calls this
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS_NON_FATAL

  return mEncoding;
}

QgsGeometry QgsVectorDataProvider::convertToProviderType( const QgsGeometry &geometry, Qgis::WkbType providerGeometryType )
{
  if ( geometry.isNull() )
  {
    return QgsGeometry();
  }

  const QgsAbstractGeometry *convertedGeometry = geometry.constGet();
  if ( !convertedGeometry )
  {
    return QgsGeometry();
  }


  //geom is already in the provider geometry type
  if ( convertedGeometry->wkbType() == providerGeometryType )
  {
    return QgsGeometry();
  }

  std::unique_ptr< QgsAbstractGeometry > outputGeom;

  //convert compoundcurve to circularstring (possible if compoundcurve consists of one circular string)
  if ( QgsWkbTypes::flatType( providerGeometryType ) == Qgis::WkbType::CircularString )
  {
    QgsCompoundCurve *compoundCurve = qgsgeometry_cast<QgsCompoundCurve *>( convertedGeometry );
    if ( compoundCurve )
    {
      if ( compoundCurve->nCurves() == 1 )
      {
        const QgsCircularString *circularString = qgsgeometry_cast<const QgsCircularString *>( compoundCurve->curveAt( 0 ) );
        if ( circularString )
        {
          outputGeom.reset( circularString->clone() );
        }
      }
    }
  }

  //convert to curved type if necessary
  if ( !QgsWkbTypes::isCurvedType( convertedGeometry->wkbType() ) && QgsWkbTypes::isCurvedType( providerGeometryType ) )
  {
    QgsAbstractGeometry *curveGeom = outputGeom ? outputGeom->toCurveType() : convertedGeometry->toCurveType();
    if ( curveGeom )
    {
      outputGeom.reset( curveGeom );
    }
  }

  //convert to linear type from curved type
  if ( QgsWkbTypes::isCurvedType( convertedGeometry->wkbType() ) && !QgsWkbTypes::isCurvedType( providerGeometryType ) )
  {
    QgsAbstractGeometry *segmentizedGeom = outputGeom ? outputGeom->segmentize() : convertedGeometry->segmentize();
    if ( segmentizedGeom )
    {
      outputGeom.reset( segmentizedGeom );
    }
  }

  //convert to multitype if necessary
  if ( QgsWkbTypes::isMultiType( providerGeometryType ) && !QgsWkbTypes::isMultiType( convertedGeometry->wkbType() ) )
  {
    std::unique_ptr< QgsAbstractGeometry > collGeom( QgsGeometryFactory::geomFromWkbType( providerGeometryType ) );
    QgsGeometryCollection *geomCollection = qgsgeometry_cast<QgsGeometryCollection *>( collGeom.get() );
    if ( geomCollection )
    {
      if ( geomCollection->addGeometry( outputGeom ? outputGeom->clone() : convertedGeometry->clone() ) )
      {
        outputGeom.reset( collGeom.release() );
      }
    }
  }

  //convert to single type if there's a single part of compatible type
  if ( !QgsWkbTypes::isMultiType( providerGeometryType ) && QgsWkbTypes::isMultiType( convertedGeometry->wkbType() ) )
  {
    const QgsGeometryCollection *collection = qgsgeometry_cast<const QgsGeometryCollection *>( convertedGeometry );
    if ( collection )
    {
      if ( collection->numGeometries() == 1 )
      {
        const QgsAbstractGeometry *firstGeom = collection->geometryN( 0 );
        if ( firstGeom && firstGeom->wkbType() == providerGeometryType )
        {
          outputGeom.reset( firstGeom->clone() );
        }
      }
    }
  }

  //set z/m types
  if ( QgsWkbTypes::hasZ( providerGeometryType ) )
  {
    if ( !outputGeom )
    {
      outputGeom.reset( convertedGeometry->clone() );
    }
    outputGeom->addZValue();
  }

  if ( QgsWkbTypes::hasM( providerGeometryType ) )
  {
    if ( !outputGeom )
    {
      outputGeom.reset( convertedGeometry->clone() );
    }
    outputGeom->addMValue();
  }

  if ( outputGeom )
  {
    return QgsGeometry( outputGeom.release() );
  }

  return QgsGeometry();
}

bool QgsVectorDataProvider::cancelReload()
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  return false;
}

QStringList QgsVectorDataProvider::sEncodings;

QList<QgsRelation> QgsVectorDataProvider::discoverRelations( const QgsVectorLayer *, const QList<QgsVectorLayer *> & ) const
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  return QList<QgsRelation>();
}

void QgsVectorDataProvider::handlePostCloneOperations( QgsVectorDataProvider * )
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

}

QgsVectorDataProviderTemporalCapabilities *QgsVectorDataProvider::temporalCapabilities()
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  return mTemporalCapabilities.get();
}

const QgsVectorDataProviderTemporalCapabilities *QgsVectorDataProvider::temporalCapabilities() const
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  return mTemporalCapabilities.get();
}

QgsDataProviderElevationProperties *QgsVectorDataProvider::elevationProperties()
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  return mElevationProperties.get();
}

const QgsDataProviderElevationProperties *QgsVectorDataProvider::elevationProperties() const
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  return mElevationProperties.get();
}

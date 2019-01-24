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

#include <cfloat>
#include <climits>
#include <limits>

#include "qgsvectordataprovider.h"
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
#include "qgssettings.h"
#include <mutex>

QgsVectorDataProvider::QgsVectorDataProvider( const QString &uri, const ProviderOptions &options )
  : QgsDataProvider( uri, options )
{
  QgsSettings settings;
  setEncoding( settings.value( QStringLiteral( "UI/encoding" ), "System" ).toString() );
}

QString QgsVectorDataProvider::storageType() const
{
  return QStringLiteral( "Generic vector file" );
}

bool QgsVectorDataProvider::empty() const
{
  QgsFeature f;
  QgsFeatureRequest request;
  request.setNoAttributes();
  request.setFlags( QgsFeatureRequest::NoGeometry );
  request.setLimit( 1 );
  if ( getFeatures( request ).nextFeature( f ) )
    return false;
  else
    return true;
}

QgsFeatureSource::FeatureAvailability QgsVectorDataProvider::hasFeatures() const
{
  if ( empty() )
    return QgsFeatureSource::FeatureAvailability::NoFeaturesAvailable;
  else
    return QgsFeatureSource::FeatureAvailability::FeaturesAvailable;
}

QgsCoordinateReferenceSystem QgsVectorDataProvider::sourceCrs() const
{
  return crs();
}

QgsRectangle QgsVectorDataProvider::sourceExtent() const
{
  return extent();
}

QString QgsVectorDataProvider::dataComment() const
{
  return QString();
}

bool QgsVectorDataProvider::addFeatures( QgsFeatureList &flist, Flags flags )
{
  Q_UNUSED( flist );
  Q_UNUSED( flags );
  return false;
}

bool QgsVectorDataProvider::deleteFeatures( const QgsFeatureIds &ids )
{
  Q_UNUSED( ids );
  return false;
}

bool QgsVectorDataProvider::truncate()
{
  if ( !( capabilities() & DeleteFeatures ) )
    return false;

  QgsFeatureIds toDelete;
  QgsFeatureIterator it = getFeatures( QgsFeatureRequest().setFlags( QgsFeatureRequest::NoGeometry ).setNoAttributes() );
  QgsFeature f;
  while ( it.nextFeature( f ) )
    toDelete << f.id();

  return deleteFeatures( toDelete );
}

bool QgsVectorDataProvider::addAttributes( const QList<QgsField> &attributes )
{
  Q_UNUSED( attributes );
  return false;
}

bool QgsVectorDataProvider::deleteAttributes( const QgsAttributeIds &attributes )
{
  Q_UNUSED( attributes );
  return false;
}

bool QgsVectorDataProvider::renameAttributes( const QgsFieldNameMap &renamedAttributes )
{
  Q_UNUSED( renamedAttributes );
  return false;
}

bool QgsVectorDataProvider::changeAttributeValues( const QgsChangedAttributesMap &attr_map )
{
  Q_UNUSED( attr_map );
  return false;
}

QVariant QgsVectorDataProvider::defaultValue( int fieldId ) const
{
  Q_UNUSED( fieldId );
  return QVariant();
}

QString QgsVectorDataProvider::defaultValueClause( int fieldIndex ) const
{
  Q_UNUSED( fieldIndex );
  return QString();
}

QgsFieldConstraints::Constraints QgsVectorDataProvider::fieldConstraints( int fieldIndex ) const
{
  QgsFields f = fields();
  if ( fieldIndex < 0 || fieldIndex >= f.count() )
    return nullptr;

  return f.at( fieldIndex ).constraints().constraints();
}

bool QgsVectorDataProvider::skipConstraintCheck( int, QgsFieldConstraints::Constraint, const QVariant & ) const
{
  return false;
}

bool QgsVectorDataProvider::changeGeometryValues( const QgsGeometryMap &geometry_map )
{
  Q_UNUSED( geometry_map );
  return false;
}

bool QgsVectorDataProvider::changeFeatures( const QgsChangedAttributesMap &attr_map,
    const QgsGeometryMap &geometry_map )
{
  if ( !( capabilities() & ChangeAttributeValues ) || !( capabilities() & ChangeGeometries ) )
    return false;

  bool result = true;
  result = result && changeAttributeValues( attr_map );
  result = result && changeGeometryValues( geometry_map );
  return result;
}

bool QgsVectorDataProvider::createSpatialIndex()
{
  return false;
}

bool QgsVectorDataProvider::createAttributeIndex( int field )
{
  Q_UNUSED( field );
  return true;
}

QgsVectorDataProvider::Capabilities QgsVectorDataProvider::capabilities() const
{
  return QgsVectorDataProvider::NoCapabilities;
}


void QgsVectorDataProvider::setEncoding( const QString &e )
{
  mEncoding = QTextCodec::codecForName( e.toLocal8Bit().constData() );

  if ( !mEncoding && e != QLatin1String( "System" ) )
  {
    QgsMessageLog::logMessage( tr( "Codec %1 not found. Falling back to system locale" ).arg( e ) );
    mEncoding = QTextCodec::codecForName( "System" );
  }

  if ( !mEncoding )
    mEncoding = QTextCodec::codecForLocale();

  Q_ASSERT( mEncoding );
}

QString QgsVectorDataProvider::encoding() const
{
  if ( mEncoding )
  {
    return mEncoding->name();
  }

  return QString();
}

QString QgsVectorDataProvider::capabilitiesString() const
{
  QStringList abilitiesList;

  int abilities = capabilities();

  if ( abilities & QgsVectorDataProvider::AddFeatures )
  {
    abilitiesList += tr( "Add Features" );
  }

  if ( abilities & QgsVectorDataProvider::DeleteFeatures )
  {
    abilitiesList += tr( "Delete Features" );
  }

  if ( abilities & QgsVectorDataProvider::ChangeAttributeValues )
  {
    abilitiesList += tr( "Change Attribute Values" );
  }

  if ( abilities & QgsVectorDataProvider::AddAttributes )
  {
    abilitiesList += tr( "Add Attributes" );
  }

  if ( abilities & QgsVectorDataProvider::DeleteAttributes )
  {
    abilitiesList += tr( "Delete Attributes" );
  }

  if ( abilities & QgsVectorDataProvider::RenameAttributes )
  {
    abilitiesList += tr( "Rename Attributes" );
  }

  if ( abilities & QgsVectorDataProvider::CreateSpatialIndex )
  {
    // TODO: Tighten up this test.  See QgsOgrProvider for details.
    abilitiesList += tr( "Create Spatial Index" );
  }

  if ( abilities & QgsVectorDataProvider::CreateAttributeIndex )
  {
    abilitiesList += tr( "Create Attribute Indexes" );
  }

  if ( abilities & QgsVectorDataProvider::SelectAtId )
  {
    abilitiesList += tr( "Fast Access to Features at ID" );
  }

  if ( abilities & QgsVectorDataProvider::ChangeGeometries )
  {
    abilitiesList += tr( "Change Geometries" );
  }

  if ( abilities & QgsVectorDataProvider::SimplifyGeometries )
  {
    abilitiesList += tr( "Presimplify Geometries" );
  }

  if ( abilities & QgsVectorDataProvider::SimplifyGeometriesWithTopologicalValidation )
  {
    abilitiesList += tr( "Presimplify Geometries with Validity Check" );
  }

  if ( abilities & QgsVectorDataProvider::ChangeFeatures )
  {
    abilitiesList += tr( "Simultaneous Geometry and Attribute Updates" );
  }

  if ( abilities & QgsVectorDataProvider::TransactionSupport )
  {
    abilitiesList += tr( "Transactions" );
  }

  if ( abilities & QgsVectorDataProvider::CircularGeometries )
  {
    abilitiesList += tr( "Curved Geometries" );
  }

  return abilitiesList.join( QStringLiteral( ", " ) );
}


int QgsVectorDataProvider::fieldNameIndex( const QString &fieldName ) const
{
  return fields().lookupField( fieldName );
}

QMap<QString, int> QgsVectorDataProvider::fieldNameMap() const
{
  QMap<QString, int> resultMap;

  QgsFields fieldsCopy = fields();
  for ( int i = 0; i < fieldsCopy.count(); ++i )
  {
    resultMap.insert( fieldsCopy.at( i ).name(), i );
  }

  return resultMap;
}

QgsAttributeList QgsVectorDataProvider::attributeIndexes() const
{
  return fields().allAttributesList();
}

QgsAttributeList QgsVectorDataProvider::pkAttributeIndexes() const
{
  return QgsAttributeList();
}

QList<QgsVectorDataProvider::NativeType> QgsVectorDataProvider::nativeTypes() const
{
  return mNativeTypes;
}

QgsAttrPalIndexNameHash QgsVectorDataProvider::palAttributeIndexNames() const
{
  return QgsAttrPalIndexNameHash();
}

bool QgsVectorDataProvider::supportedType( const QgsField &field ) const
{
  QgsDebugMsgLevel( QStringLiteral( "field name = %1 type = %2 length = %3 precision = %4" )
                    .arg( field.name(),
                          QVariant::typeToName( field.type() ) )
                    .arg( field.length() )
                    .arg( field.precision() ), 2 );

  Q_FOREACH ( const NativeType &nativeType, mNativeTypes )
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

    QgsDebugMsg( QStringLiteral( "native type matches" ) );
    return true;
  }

  QgsDebugMsg( QStringLiteral( "no sufficient native type found" ) );
  return false;
}

QVariant QgsVectorDataProvider::minimumValue( int index ) const
{
  if ( index < 0 || index >= fields().count() )
  {
    QgsDebugMsg( "Warning: access requested to invalid field index: " + QString::number( index ) );
    return QVariant();
  }

  fillMinMaxCache();

  if ( !mCacheMinValues.contains( index ) )
    return QVariant();

  return mCacheMinValues[index];
}

QVariant QgsVectorDataProvider::maximumValue( int index ) const
{
  if ( index < 0 || index >= fields().count() )
  {
    QgsDebugMsg( "Warning: access requested to invalid field index: " + QString::number( index ) );
    return QVariant();
  }

  fillMinMaxCache();

  if ( !mCacheMaxValues.contains( index ) )
    return QVariant();

  return mCacheMaxValues[index];
}


QStringList QgsVectorDataProvider::uniqueStringsMatching( int index, const QString &substring, int limit, QgsFeedback *feedback ) const
{
  QStringList results;

  // Safety belt
  if ( index < 0 || index >= fields().count() )
    return results;

  QgsFeature f;
  QgsAttributeList keys;
  keys.append( index );

  QgsFeatureRequest request;
  request.setSubsetOfAttributes( keys );
  request.setFlags( QgsFeatureRequest::NoGeometry );
  QString fieldName = fields().at( index ).name();
  request.setFilterExpression( QStringLiteral( "\"%1\" ILIKE '%%2%'" ).arg( fieldName, substring ) );
  QgsFeatureIterator fi = getFeatures( request );

  QSet<QString> set;

  while ( fi.nextFeature( f ) )
  {
    QString value = f.attribute( index ).toString();
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

QVariant QgsVectorDataProvider::aggregate( QgsAggregateCalculator::Aggregate aggregate, int index,
    const QgsAggregateCalculator::AggregateParameters &parameters, QgsExpressionContext *context, bool &ok, const QgsFeatureIds fids ) const
{
  //base implementation does nothing
  Q_UNUSED( aggregate );
  Q_UNUSED( index );
  Q_UNUSED( parameters );
  Q_UNUSED( context );
  Q_UNUSED( fids );

  ok = false;
  return QVariant();
}

void QgsVectorDataProvider::clearMinMaxCache()
{
  mCacheMinMaxDirty = true;
  mCacheMinValues.clear();
  mCacheMaxValues.clear();
}

void QgsVectorDataProvider::fillMinMaxCache() const
{
  if ( !mCacheMinMaxDirty )
    return;

  QgsFields flds = fields();
  for ( int i = 0; i < flds.count(); ++i )
  {
    if ( flds.at( i ).type() == QVariant::Int )
    {
      mCacheMinValues[i] = QVariant( std::numeric_limits<int>::max() );
      mCacheMaxValues[i] = QVariant( std::numeric_limits<int>::lowest() );
    }
    else if ( flds.at( i ).type() == QVariant::LongLong )
    {
      mCacheMinValues[i] = QVariant( std::numeric_limits<qlonglong>::max() );
      mCacheMaxValues[i] = QVariant( std::numeric_limits<qlonglong>::lowest() );
    }
    else if ( flds.at( i ).type() == QVariant::Double )
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
                                       .setFlags( QgsFeatureRequest::NoGeometry ) );

  while ( fi.nextFeature( f ) )
  {
    QgsAttributes attrs = f.attributes();
    for ( int attributeIndex : keys )
    {
      const QVariant &varValue = attrs.at( attributeIndex );

      if ( varValue.isNull() )
        continue;

      switch ( flds.at( attributeIndex ).type() )
      {
        case QVariant::Int:
        {
          int value = varValue.toInt();
          if ( value < mCacheMinValues[ attributeIndex ].toInt() )
            mCacheMinValues[ attributeIndex ] = value;
          if ( value > mCacheMaxValues[ attributeIndex ].toInt() )
            mCacheMaxValues[ attributeIndex ] = value;
          break;
        }
        case QVariant::LongLong:
        {
          qlonglong value = varValue.toLongLong();
          if ( value < mCacheMinValues[ attributeIndex ].toLongLong() )
            mCacheMinValues[ attributeIndex ] = value;
          if ( value > mCacheMaxValues[ attributeIndex ].toLongLong() )
            mCacheMaxValues[ attributeIndex ] = value;
          break;
        }
        case QVariant::Double:
        {
          double value = varValue.toDouble();
          if ( value < mCacheMinValues[ attributeIndex ].toDouble() )
            mCacheMinValues[attributeIndex ] = value;
          if ( value > mCacheMaxValues[ attributeIndex ].toDouble() )
            mCacheMaxValues[ attributeIndex ] = value;
          break;
        }
        default:
        {
          QString value = varValue.toString();
          if ( mCacheMinValues[ attributeIndex ].isNull() || value < mCacheMinValues[attributeIndex ].toString() )
          {
            mCacheMinValues[attributeIndex] = value;
          }
          if ( mCacheMaxValues[attributeIndex].isNull() || value > mCacheMaxValues[attributeIndex].toString() )
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

QVariant QgsVectorDataProvider::convertValue( QVariant::Type type, const QString &value )
{
  QVariant v( value );

  if ( !v.convert( type ) || value.isNull() )
    v = QVariant( type );

  return v;
}

QgsTransaction *QgsVectorDataProvider::transaction() const
{
  return nullptr;
}

void QgsVectorDataProvider::forceReload()
{
  emit dataChanged();
}

static bool _compareEncodings( const QString &s1, const QString &s2 )
{
  return s1.toLower() < s2.toLower();
}

QStringList QgsVectorDataProvider::availableEncodings()
{
  static std::once_flag initialized;
  std::call_once( initialized, [ = ]
  {
    Q_FOREACH ( const QString &codec, QTextCodec::availableCodecs() )
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

    // Do case-insensitive sorting of encodings
    std::sort( sEncodings.begin(), sEncodings.end(), _compareEncodings );

  } );

  return sEncodings;
}

void QgsVectorDataProvider::clearErrors()
{
  mErrors.clear();
}

bool QgsVectorDataProvider::hasErrors() const
{
  return !mErrors.isEmpty();
}

QStringList QgsVectorDataProvider::errors() const
{
  return mErrors;
}

bool QgsVectorDataProvider::isSaveAndLoadStyleToDatabaseSupported() const
{
  return false;
}

bool QgsVectorDataProvider::isDeleteStyleFromDatabaseSupported() const
{
  return false;
}

QgsFeatureRenderer *QgsVectorDataProvider::createRenderer( const QVariantMap & ) const
{
  return nullptr;
}

QgsAbstractVectorLayerLabeling *QgsVectorDataProvider::createLabeling( const QVariantMap & ) const
{
  return nullptr;
}

void QgsVectorDataProvider::pushError( const QString &msg ) const
{
  QgsDebugMsg( msg );
  mErrors << msg;
  emit raiseError( msg );
}

QSet<QgsMapLayerDependency> QgsVectorDataProvider::dependencies() const
{
  return QSet<QgsMapLayerDependency>();
}

QgsGeometry QgsVectorDataProvider::convertToProviderType( const QgsGeometry &geom ) const
{
  if ( geom.isNull() )
  {
    return QgsGeometry();
  }

  const QgsAbstractGeometry *geometry = geom.constGet();
  if ( !geometry )
  {
    return QgsGeometry();
  }

  QgsWkbTypes::Type providerGeomType = wkbType();

  //geom is already in the provider geometry type
  if ( geometry->wkbType() == providerGeomType )
  {
    return QgsGeometry();
  }

  std::unique_ptr< QgsAbstractGeometry > outputGeom;

  //convert compoundcurve to circularstring (possible if compoundcurve consists of one circular string)
  if ( QgsWkbTypes::flatType( providerGeomType ) == QgsWkbTypes::CircularString )
  {
    QgsCompoundCurve *compoundCurve = qgsgeometry_cast<QgsCompoundCurve *>( geometry );
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
  if ( !QgsWkbTypes::isCurvedType( geometry->wkbType() ) && QgsWkbTypes::isCurvedType( providerGeomType ) )
  {
    QgsAbstractGeometry *curveGeom = outputGeom ? outputGeom->toCurveType() : geometry->toCurveType();
    if ( curveGeom )
    {
      outputGeom.reset( curveGeom );
    }
  }

  //convert to linear type from curved type
  if ( QgsWkbTypes::isCurvedType( geometry->wkbType() ) && !QgsWkbTypes::isCurvedType( providerGeomType ) )
  {
    QgsAbstractGeometry *segmentizedGeom = outputGeom ? outputGeom->segmentize() : geometry->segmentize();
    if ( segmentizedGeom )
    {
      outputGeom.reset( segmentizedGeom );
    }
  }

  //convert to multitype if necessary
  if ( QgsWkbTypes::isMultiType( providerGeomType ) && !QgsWkbTypes::isMultiType( geometry->wkbType() ) )
  {
    std::unique_ptr< QgsAbstractGeometry > collGeom( QgsGeometryFactory::geomFromWkbType( providerGeomType ) );
    QgsGeometryCollection *geomCollection = qgsgeometry_cast<QgsGeometryCollection *>( collGeom.get() );
    if ( geomCollection )
    {
      if ( geomCollection->addGeometry( outputGeom ? outputGeom->clone() : geometry->clone() ) )
      {
        outputGeom.reset( collGeom.release() );
      }
    }
  }

  //set z/m types
  if ( QgsWkbTypes::hasZ( providerGeomType ) )
  {
    if ( !outputGeom )
    {
      outputGeom.reset( geometry->clone() );
    }
    outputGeom->addZValue();
  }

  if ( QgsWkbTypes::hasM( providerGeomType ) )
  {
    if ( !outputGeom )
    {
      outputGeom.reset( geometry->clone() );
    }
    outputGeom->addMValue();
  }

  if ( outputGeom )
  {
    return QgsGeometry( outputGeom.release() );
  }

  return QgsGeometry();
}

void QgsVectorDataProvider::setNativeTypes( const QList<NativeType> &nativeTypes )
{
  mNativeTypes = nativeTypes;
}

QTextCodec *QgsVectorDataProvider::textEncoding() const
{
  return mEncoding;
}

bool QgsVectorDataProvider::cancelReload()
{
  return false;
}

QStringList QgsVectorDataProvider::sEncodings;

QList<QgsRelation> QgsVectorDataProvider::discoverRelations( const QgsVectorLayer *, const QList<QgsVectorLayer *> & ) const
{
  return QList<QgsRelation>();
}

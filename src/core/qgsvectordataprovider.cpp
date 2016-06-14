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

#include <QSettings>
#include <QTextCodec>

#include <cfloat> // for DBL_MAX
#include <climits>
#include <limits>

#include "qgsvectordataprovider.h"
#include "qgscircularstringv2.h"
#include "qgscompoundcurvev2.h"
#include "qgsfeature.h"
#include "qgsfeatureiterator.h"
#include "qgsfeaturerequest.h"
#include "qgsfield.h"
#include "qgsgeometry.h"
#include "qgsgeometrycollectionv2.h"
#include "qgsgeometryfactory.h"
#include "qgslogger.h"
#include "qgsmessagelog.h"

QgsVectorDataProvider::QgsVectorDataProvider( const QString& uri )
    : QgsDataProvider( uri )
    , mCacheMinMaxDirty( true )
    , mAttrPalIndexName( QgsAttrPalIndexNameHash() )
{
  QSettings settings;
  setEncoding( settings.value( "/UI/encoding", "System" ).toString() );
}


QgsVectorDataProvider::~QgsVectorDataProvider()
{
}

QString QgsVectorDataProvider::storageType() const
{
  return "Generic vector file";
}

QString QgsVectorDataProvider::dataComment() const
{
  return QString();
}

bool QgsVectorDataProvider::addFeatures( QgsFeatureList &flist )
{
  Q_UNUSED( flist );
  return false;
}

bool QgsVectorDataProvider::deleteFeatures( const QgsFeatureIds &ids )
{
  Q_UNUSED( ids );
  return false;
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

bool QgsVectorDataProvider::renameAttributes( const QgsFieldNameMap& renamedAttributes )
{
  Q_UNUSED( renamedAttributes );
  return false;
}

bool QgsVectorDataProvider::changeAttributeValues( const QgsChangedAttributesMap &attr_map )
{
  Q_UNUSED( attr_map );
  return false;
}

QVariant QgsVectorDataProvider::defaultValue( int fieldId )
{
  Q_UNUSED( fieldId );
  return QVariant();
}

bool QgsVectorDataProvider::changeGeometryValues( const QgsGeometryMap &geometry_map )
{
  Q_UNUSED( geometry_map );
  return false;
}

bool QgsVectorDataProvider::changeFeatures( const QgsChangedAttributesMap &attr_map,
    const QgsGeometryMap &geometry_map )
{
  Q_UNUSED( attr_map );
  Q_UNUSED( geometry_map );
  return false;
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

int QgsVectorDataProvider::capabilities() const
{
  return QgsVectorDataProvider::NoCapabilities;
}


void QgsVectorDataProvider::setEncoding( const QString& e )
{
  QTextCodec* ncodec = QTextCodec::codecForName( e.toLocal8Bit().constData() );
  if ( ncodec )
  {
    mEncoding = ncodec;
  }
  else
  {
    QgsMessageLog::logMessage( tr( "Codec %1 not found. Falling back to system locale" ).arg( e ) );
    mEncoding = QTextCodec::codecForName( "System" );

    if ( !mEncoding )
      mEncoding = QTextCodec::codecForLocale();

    Q_ASSERT( mEncoding );
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
    abilitiesList += tr( "Add Features" );
    QgsDebugMsg( "Capability: Add Features" );
  }

  if ( abilities & QgsVectorDataProvider::DeleteFeatures )
  {
    abilitiesList += tr( "Delete Features" );
    QgsDebugMsg( "Capability: Delete Features" );
  }

  if ( abilities & QgsVectorDataProvider::ChangeAttributeValues )
  {
    abilitiesList += tr( "Change Attribute Values" );
    QgsDebugMsg( "Capability: Change Attribute Values" );
  }

  if ( abilities & QgsVectorDataProvider::AddAttributes )
  {
    abilitiesList += tr( "Add Attributes" );
    QgsDebugMsg( "Capability: Add Attributes" );
  }

  if ( abilities & QgsVectorDataProvider::DeleteAttributes )
  {
    abilitiesList += tr( "Delete Attributes" );
    QgsDebugMsg( "Capability: Delete Attributes" );
  }

  if ( abilities & QgsVectorDataProvider::RenameAttributes )
  {
    abilitiesList += tr( "Rename Attributes" );
    QgsDebugMsg( "Capability: Rename Attributes" );
  }

  if ( abilities & QgsVectorDataProvider::CreateSpatialIndex )
  {
    // TODO: Tighten up this test.  See QgsOgrProvider for details.
    abilitiesList += tr( "Create Spatial Index" );
    QgsDebugMsg( "Capability: Create Spatial Index" );
  }

  if ( abilities & QgsVectorDataProvider::CreateAttributeIndex )
  {
    abilitiesList += tr( "Create Attribute Indexes" );
    QgsDebugMsg( "Capability: Create Attribute Index" );
  }

  if ( abilities & QgsVectorDataProvider::SelectAtId )
  {
    abilitiesList += tr( "Fast Access to Features at ID" );
    QgsDebugMsg( "Capability: Select at ID" );
  }

  if ( abilities & QgsVectorDataProvider::ChangeGeometries )
  {
    abilitiesList += tr( "Change Geometries" );
    QgsDebugMsg( "Capability: Change Geometries" );
  }

  if ( abilities & QgsVectorDataProvider::SimplifyGeometries )
  {
    abilitiesList += tr( "Presimplify Geometries" );
    QgsDebugMsg( "Capability: Simplify Geometries before fetching the feature" );
  }

  if ( abilities & QgsVectorDataProvider::SimplifyGeometriesWithTopologicalValidation )
  {
    abilitiesList += tr( "Presimplify Geometries with Validity Check" );
    QgsDebugMsg( "Capability: Simplify Geometries before fetch the feature ensuring that the result is a valid geometry" );
  }

  if ( abilities & QgsVectorDataProvider::ChangeFeatures )
  {
    abilitiesList += tr( "Simultaneous Geometry and Attribute Updates" );
    QgsDebugMsg( "Capability: change both feature attributes and geometry at once" );
  }

  if ( abilities & QgsVectorDataProvider::TransactionSupport )
  {
    abilitiesList += tr( "Transactions" );
    QgsDebugMsg( "Capability: transactions" );
  }

  if ( abilities & QgsVectorDataProvider::CircularGeometries )
  {
    abilitiesList += tr( "Curved Geometries" );
    QgsDebugMsg( "Supports circular geometry types (circularstring, compoundcurve, curvepolygon)" );
  }

  return abilitiesList.join( ", " );
}


int QgsVectorDataProvider::fieldNameIndex( const QString& fieldName ) const
{
  return fields().fieldNameIndex( fieldName );
}

QMap<QString, int> QgsVectorDataProvider::fieldNameMap() const
{
  QMap<QString, int> resultMap;

  const QgsFields& theFields = fields();
  for ( int i = 0; i < theFields.count(); ++i )
  {
    resultMap.insert( theFields[i].name(), i );
  }

  return resultMap;
}

QgsAttributeList QgsVectorDataProvider::attributeIndexes()
{
  return fields().allAttributesList();
}

const QList< QgsVectorDataProvider::NativeType > &QgsVectorDataProvider::nativeTypes() const
{
  return mNativeTypes;
}

bool QgsVectorDataProvider::supportedType( const QgsField &field ) const
{
  int i;
  QgsDebugMsgLevel( QString( "field name = %1 type = %2 length = %3 precision = %4" )
                    .arg( field.name(),
                          QVariant::typeToName( field.type() ) )
                    .arg( field.length() )
                    .arg( field.precision() ), 2 );
  for ( i = 0; i < mNativeTypes.size(); i++ )
  {
    QgsDebugMsgLevel( QString( "native field type = %1 min length = %2 max length = %3 min precision = %4 max precision = %5" )
                      .arg( QVariant::typeToName( mNativeTypes[i].mType ) )
                      .arg( mNativeTypes[i].mMinLen )
                      .arg( mNativeTypes[i].mMaxLen )
                      .arg( mNativeTypes[i].mMinPrec )
                      .arg( mNativeTypes[i].mMaxPrec ), 2 );

    if ( field.type() != mNativeTypes[i].mType )
      continue;

    if ( field.length() == -1 )
    {
      // source length unlimited
      if ( mNativeTypes[i].mMinLen > -1 || mNativeTypes[i].mMaxLen > -1 )
      {
        // destination limited
        continue;
      }
    }
    else
    {
      // source length limited
      if ( mNativeTypes[i].mMinLen > -1 && mNativeTypes[i].mMaxLen > -1 &&
           ( field.length() < mNativeTypes[i].mMinLen || field.length() > mNativeTypes[i].mMaxLen ) )
      {
        // source length exceeds destination limits
        continue;
      }
    }

    if ( field.precision() == -1 )
    {
      // source precision unlimited / n/a
      if ( mNativeTypes[i].mMinPrec > -1 || mNativeTypes[i].mMaxPrec > -1 )
      {
        // destination limited
        continue;
      }
    }
    else
    {
      // source precision unlimited / n/a
      if ( mNativeTypes[i].mMinPrec > -1 && mNativeTypes[i].mMaxPrec > -1 &&
           ( field.precision() < mNativeTypes[i].mMinPrec || field.precision() > mNativeTypes[i].mMaxPrec ) )
      {
        // source precision exceeds destination limits
        continue;
      }
    }

    QgsDebugMsg( "native type matches" );
    return true;
  }

  QgsDebugMsg( "no sufficient native type found" );
  return false;
}

QVariant QgsVectorDataProvider::minimumValue( int index )
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

QVariant QgsVectorDataProvider::maximumValue( int index )
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

void QgsVectorDataProvider::uniqueValues( int index, QList<QVariant> &values, int limit )
{
  QgsFeature f;
  QgsAttributeList keys;
  keys.append( index );
  QgsFeatureIterator fi = getFeatures( QgsFeatureRequest().setSubsetOfAttributes( keys ) );

  QSet<QString> set;
  values.clear();

  while ( fi.nextFeature( f ) )
  {
    if ( !set.contains( f.attribute( index ).toString() ) )
    {
      values.append( f.attribute( index ) );
      set.insert( f.attribute( index ).toString() );
    }

    if ( limit >= 0 && values.size() >= limit )
      break;
  }
}

QVariant QgsVectorDataProvider::aggregate( QgsAggregateCalculator::Aggregate aggregate, int index,
    const QgsAggregateCalculator::AggregateParameters& parameters, QgsExpressionContext* context, bool& ok )
{
  //base implementation does nothing
  Q_UNUSED( aggregate );
  Q_UNUSED( index );
  Q_UNUSED( parameters );
  Q_UNUSED( context );

  ok = false;
  return QVariant();
}

void QgsVectorDataProvider::clearMinMaxCache()
{
  mCacheMinMaxDirty = true;
}

void QgsVectorDataProvider::fillMinMaxCache()
{
  if ( !mCacheMinMaxDirty )
    return;

  const QgsFields& flds = fields();
  for ( int i = 0; i < flds.count(); ++i )
  {
    if ( flds[i].type() == QVariant::Int )
    {
      mCacheMinValues[i] = QVariant( INT_MAX );
      mCacheMaxValues[i] = QVariant( INT_MIN );
    }
    else if ( flds[i].type() == QVariant::LongLong )
    {
      mCacheMinValues[i] = QVariant( std::numeric_limits<qlonglong>::max() );
      mCacheMaxValues[i] = QVariant( std::numeric_limits<qlonglong>::min() );
    }
    else if ( flds[i].type() == QVariant::Double )
    {
      mCacheMinValues[i] = QVariant( DBL_MAX );
      mCacheMaxValues[i] = QVariant( -DBL_MAX );
    }
    else
    {
      mCacheMinValues[i] = QVariant();
      mCacheMaxValues[i] = QVariant();
    }
  }

  QgsFeature f;
  QgsAttributeList keys = mCacheMinValues.keys();
  QgsFeatureIterator fi = getFeatures( QgsFeatureRequest().setSubsetOfAttributes( keys ) );

  while ( fi.nextFeature( f ) )
  {
    QgsAttributes attrs = f.attributes();
    for ( QgsAttributeList::const_iterator it = keys.begin(); it != keys.end(); ++it )
    {
      const QVariant& varValue = attrs.at( *it );

      if ( varValue.isNull() )
        continue;

      if ( flds[*it].type() == QVariant::Int )
      {
        int value = varValue.toInt();
        if ( value < mCacheMinValues[*it].toInt() )
          mCacheMinValues[*it] = value;
        if ( value > mCacheMaxValues[*it].toInt() )
          mCacheMaxValues[*it] = value;
      }
      else if ( flds[*it].type() == QVariant::LongLong )
      {
        qlonglong value = varValue.toLongLong();
        if ( value < mCacheMinValues[*it].toLongLong() )
          mCacheMinValues[*it] = value;
        if ( value > mCacheMaxValues[*it].toLongLong() )
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
      else
      {
        QString value = varValue.toString();
        if ( mCacheMinValues[*it].isNull() || value < mCacheMinValues[*it].toString() )
        {
          mCacheMinValues[*it] = value;
        }
        if ( mCacheMaxValues[*it].isNull() || value > mCacheMaxValues[*it].toString() )
        {
          mCacheMaxValues[*it] = value;
        }
      }
    }
  }

  mCacheMinMaxDirty = false;
}

QVariant QgsVectorDataProvider::convertValue( QVariant::Type type, const QString& value )
{
  QVariant v( value );

  if ( !v.convert( type ) || value.isNull() )
    v = QVariant( type );

  return v;
}

static bool _compareEncodings( const QString& s1, const QString& s2 )
{
  return s1.toLower() < s2.toLower();
}

const QStringList &QgsVectorDataProvider::availableEncodings()
{
  if ( smEncodings.isEmpty() )
  {
    Q_FOREACH ( const QString& codec, QTextCodec::availableCodecs() )
    {
      smEncodings << codec;
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
  }

  // Do case-insensitive sorting of encodings
  qSort( smEncodings.begin(), smEncodings.end(), _compareEncodings );

  return smEncodings;
}

void QgsVectorDataProvider::clearErrors()
{
  mErrors.clear();
}

bool QgsVectorDataProvider::hasErrors()
{
  return !mErrors.isEmpty();
}

QStringList QgsVectorDataProvider::errors()
{
  return mErrors;
}

void QgsVectorDataProvider::pushError( const QString& msg )
{
  QgsDebugMsg( msg );
  mErrors << msg;
  emit raiseError( msg );
}

QSet<QString> QgsVectorDataProvider::layerDependencies() const
{
  return QSet<QString>();
}

QgsGeometry* QgsVectorDataProvider::convertToProviderType( const QgsGeometry* geom ) const
{
  if ( !geom )
  {
    return nullptr;
  }

  QgsAbstractGeometryV2* geometry = geom->geometry();
  if ( !geometry )
  {
    return nullptr;
  }

  QgsWKBTypes::Type providerGeomType = QGis::fromOldWkbType( geometryType() );

  //geom is already in the provider geometry type
  if ( geometry->wkbType() == providerGeomType )
  {
    return nullptr;
  }

  QgsAbstractGeometryV2* outputGeom = nullptr;

  //convert compoundcurve to circularstring (possible if compoundcurve consists of one circular string)
  if ( QgsWKBTypes::flatType( providerGeomType ) == QgsWKBTypes::CircularString && QgsWKBTypes::flatType( geometry->wkbType() ) == QgsWKBTypes::CompoundCurve )
  {
    QgsCompoundCurveV2* compoundCurve = static_cast<QgsCompoundCurveV2*>( geometry );
    if ( compoundCurve )
    {
      if ( compoundCurve->nCurves() == 1 )
      {
        const QgsCircularStringV2* circularString = dynamic_cast<const QgsCircularStringV2*>( compoundCurve->curveAt( 0 ) );
        if ( circularString )
        {
          outputGeom = circularString->clone();
        }
      }
    }
  }

  //convert to multitype if necessary
  if ( QgsWKBTypes::isMultiType( providerGeomType ) && !QgsWKBTypes::isMultiType( geometry->wkbType() ) )
  {
    outputGeom = QgsGeometryFactory::geomFromWkbType( providerGeomType );
    QgsGeometryCollectionV2* geomCollection = dynamic_cast<QgsGeometryCollectionV2*>( outputGeom );
    if ( geomCollection )
    {
      geomCollection->addGeometry( geometry->clone() );
    }
  }

  //convert to curved type if necessary
  if ( !QgsWKBTypes::isCurvedType( geometry->wkbType() ) && QgsWKBTypes::isCurvedType( providerGeomType ) )
  {
    QgsAbstractGeometryV2* curveGeom = outputGeom ? outputGeom->toCurveType() : geometry->toCurveType();
    if ( curveGeom )
    {
      delete outputGeom;
      outputGeom = curveGeom;
    }
  }

  //convert to linear type from curved type
  if ( QgsWKBTypes::isCurvedType( geometry->wkbType() ) && !QgsWKBTypes::isCurvedType( providerGeomType ) )
  {
    QgsAbstractGeometryV2* segmentizedGeom = nullptr;
    segmentizedGeom = outputGeom ? outputGeom->segmentize() : geometry->segmentize();
    if ( segmentizedGeom )
    {
      delete outputGeom;
      outputGeom = segmentizedGeom;
    }
  }

  //set z/m types
  if ( QgsWKBTypes::hasZ( providerGeomType ) )
  {
    if ( !outputGeom )
    {
      outputGeom = geometry->clone();
    }
    outputGeom->addZValue();
  }
  if ( QgsWKBTypes::hasM( providerGeomType ) )
  {
    if ( !outputGeom )
    {
      outputGeom = geometry->clone();
    }
    outputGeom->addMValue();
  }

  if ( outputGeom )
  {
    return new QgsGeometry( outputGeom );
  }
  return nullptr;
}

QStringList QgsVectorDataProvider::smEncodings;

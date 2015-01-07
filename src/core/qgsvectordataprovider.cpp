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

#include "qgsvectordataprovider.h"
#include "qgsfeature.h"
#include "qgsfeatureiterator.h"
#include "qgsfeaturerequest.h"
#include "qgsfield.h"
#include "qgslogger.h"
#include "qgsmessagelog.h"

QgsVectorDataProvider::QgsVectorDataProvider( QString uri )
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

bool QgsVectorDataProvider::changeGeometryValues( QgsGeometryMap &geometry_map )
{
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

  if ( abilities & QgsVectorDataProvider::CreateSpatialIndex )
  {
    // TODO: Tighten up this test.  See QgsOgrProvider for details.
    abilitiesList += tr( "Create Spatial Index" );
    QgsDebugMsg( "Capability: Create Spatial Index" );
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
    abilitiesList += tr( "Simplify Geometries" );
    QgsDebugMsg( "Capability: Simplify Geometries before fetch the feature" );
  }

  if ( abilities & QgsVectorDataProvider::SimplifyGeometriesWithTopologicalValidation )
  {
    abilitiesList += tr( "Simplify Geometries with topological validation" );
    QgsDebugMsg( "Capability: Simplify Geometries before fetch the feature ensuring that the result is a valid geometry" );
  }

  return abilitiesList.join( ", " );

}


int QgsVectorDataProvider::fieldNameIndex( const QString& fieldName ) const
{
  const QgsFields &theFields = fields();

  for ( int i = 0; i < theFields.count(); ++i )
  {
    if ( QString::compare( theFields[i].name(), fieldName, Qt::CaseInsensitive ) == 0 )
    {
      return i;
    }
  }
  return -1;
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
                    .arg( field.name() )
                    .arg( QVariant::typeToName( field.type() ) )
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
    const QgsAttributes& attrs = f.attributes();
    for ( QgsAttributeList::const_iterator it = keys.begin(); it != keys.end(); ++it )
    {
      const QVariant& varValue = attrs[*it];

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

QVariant QgsVectorDataProvider::convertValue( QVariant::Type type, QString value )
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
    foreach ( QString codec, QTextCodec::availableCodecs() )
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

void QgsVectorDataProvider::pushError( QString msg )
{
  mErrors << msg;
}

QStringList QgsVectorDataProvider::smEncodings;

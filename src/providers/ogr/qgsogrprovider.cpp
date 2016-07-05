/***************************************************************************
           qgsogrprovider.cpp Data provider for OGR supported formats
                    Formerly known as qgsshapefileprovider.cpp
begin                : Oct 29, 2003
copyright            : (C) 2003 by Gary E.Sherman
email                : sherman at mrcc.com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsogrprovider.h"
#include "qgsogrfeatureiterator.h"
#include "qgslogger.h"
#include "qgsmessagelog.h"
#include "qgslocalec.h"

#define CPL_SUPRESS_CPLUSPLUS
#include <gdal.h>         // to collect version information
#include <ogr_api.h>
#include <ogr_srs_api.h>
#include <cpl_string.h>

#include <limits>

#include <QtDebug>
#include <QFile>
#include <QDir>
#include <QFileInfo>
#include <QMap>
#include <QString>
#include <QTextCodec>
#include <QSettings>

#include "qgsapplication.h"
#include "qgscrscache.h"
#include "qgsdataitem.h"
#include "qgsdataprovider.h"
#include "qgsfeature.h"
#include "qgsfield.h"
#include "qgsgeometry.h"
#include "qgscoordinatereferencesystem.h"
#include "qgsvectorlayerimport.h"
#include "qgslocalec.h"

static const QString TEXT_PROVIDER_KEY = "ogr";
static const QString TEXT_PROVIDER_DESCRIPTION =
  QString( "OGR data provider" )
  + " (compiled against GDAL/OGR library version "
  + GDAL_RELEASE_NAME
  + ", running against GDAL/OGR library version "
  + GDALVersionInfo( "RELEASE_NAME" )
  + ')';


class QgsCPLErrorHandler
{
    static void CPL_STDCALL showError( CPLErr errClass, int errNo, const char *msg )
    {
      if ( errNo != OGRERR_NONE )
        QgsMessageLog::logMessage( QObject::tr( "OGR[%1] error %2: %3" ).arg( errClass ).arg( errNo ).arg( msg ), QObject::tr( "OGR" ) );
    }

  public:
    QgsCPLErrorHandler()
    {
      CPLPushErrorHandler( showError );
    }

    ~QgsCPLErrorHandler()
    {
      CPLPopErrorHandler();
    }
};


bool QgsOgrProvider::convertField( QgsField &field, const QTextCodec &encoding )
{
  OGRFieldType ogrType = OFTString; //default to string
  int ogrWidth = field.length();
  int ogrPrecision = field.precision();
  if ( ogrPrecision > 0 )
    ogrWidth += 1;
  switch ( field.type() )
  {
    case QVariant::LongLong:
#if defined(GDAL_VERSION_NUM) && GDAL_VERSION_NUM >= 2000000
      ogrType = OFTInteger64;
      ogrPrecision = 0;
#else
      ogrType = OFTString;
      ogrPrecision = -1;
#endif
      ogrWidth = ogrWidth > 0 && ogrWidth <= 21 ? ogrWidth : 21;
      break;

    case QVariant::String:
      ogrType = OFTString;
      if ( ogrWidth < 0 || ogrWidth > 255 )
        ogrWidth = 255;
      break;

    case QVariant::Int:
      ogrType = OFTInteger;
      ogrWidth = ogrWidth > 0 && ogrWidth <= 10 ? ogrWidth : 10;
      ogrPrecision = 0;
      break;

    case QVariant::Double:
      ogrType = OFTReal;
      break;

    case QVariant::Date:
      ogrType = OFTDate;
      break;

    case QVariant::Time:
      ogrType = OFTTime;
      break;

    case QVariant::DateTime:
      ogrType = OFTDateTime;
      break;

    default:
      return false;
  }

  field.setTypeName( encoding.toUnicode( OGR_GetFieldTypeName( ogrType ) ) );
  field.setLength( ogrWidth );
  field.setPrecision( ogrPrecision );
  return true;
}

void QgsOgrProvider::repack()
{
  if ( !mValid || ogrDriverName != "ESRI Shapefile" || !ogrOrigLayer )
    return;

  QByteArray layerName = OGR_FD_GetName( OGR_L_GetLayerDefn( ogrOrigLayer ) );

  // run REPACK on shape files
  QByteArray sql = QByteArray( "REPACK " ) + layerName;   // don't quote the layer name as it works with spaces in the name and won't work if the name is quoted
  QgsDebugMsg( QString( "SQL: %1" ).arg( FROM8( sql ) ) );
  OGR_DS_ExecuteSQL( ogrDataSource, sql.constData(), nullptr, nullptr );

  if ( mFilePath.endsWith( ".shp", Qt::CaseInsensitive ) || mFilePath.endsWith( ".dbf", Qt::CaseInsensitive ) )
  {
    QString packedDbf( mFilePath.left( mFilePath.size() - 4 ) + "_packed.dbf" );
    if ( QFile::exists( packedDbf ) )
    {
      QgsMessageLog::logMessage( tr( "Possible corruption after REPACK detected. %1 still exists. This may point to a permission or locking problem of the original DBF." ).arg( packedDbf ), tr( "OGR" ), QgsMessageLog::CRITICAL );

      OGR_DS_Destroy( ogrDataSource );
      ogrLayer = ogrOrigLayer = nullptr;

      ogrDataSource = QgsOgrProviderUtils::OGROpenWrapper( TO8F( mFilePath ), true, nullptr );
      if ( ogrDataSource )
      {
        if ( mLayerName.isNull() )
        {
          ogrOrigLayer = OGR_DS_GetLayer( ogrDataSource, mLayerIndex );
        }
        else
        {
          ogrOrigLayer = OGR_DS_GetLayerByName( ogrDataSource, TO8( mLayerName ) );
        }

        if ( !ogrOrigLayer )
        {
          QgsMessageLog::logMessage( tr( "Original layer could not be reopened." ), tr( "OGR" ), QgsMessageLog::CRITICAL );
          mValid = false;
        }

        ogrLayer = ogrOrigLayer;
      }
      else
      {
        QgsMessageLog::logMessage( tr( "Original datasource could not be reopened." ), tr( "OGR" ), QgsMessageLog::CRITICAL );
        mValid = false;
      }
    }

  }

  long oldcount = mFeaturesCounted;
  recalculateFeatureCount();
  if ( oldcount != mFeaturesCounted )
    emit dataChanged();
}


QgsVectorLayerImport::ImportError QgsOgrProvider::createEmptyLayer(
  const QString& uri,
  const QgsFields &fields,
  QGis::WkbType wkbType,
  const QgsCoordinateReferenceSystem *srs,
  bool overwrite,
  QMap<int, int> *oldToNewAttrIdxMap,
  QString *errorMessage,
  const QMap<QString, QVariant> *options )
{
  QString encoding;
  QString driverName = "ESRI Shapefile";
  QStringList dsOptions, layerOptions;

  if ( options )
  {
    if ( options->contains( "fileEncoding" ) )
      encoding = options->value( "fileEncoding" ).toString();

    if ( options->contains( "driverName" ) )
      driverName = options->value( "driverName" ).toString();

    if ( options->contains( "datasourceOptions" ) )
      dsOptions << options->value( "datasourceOptions" ).toStringList();

    if ( options->contains( "layerOptions" ) )
      layerOptions << options->value( "layerOptions" ).toStringList();
  }

  if ( oldToNewAttrIdxMap )
    oldToNewAttrIdxMap->clear();
  if ( errorMessage )
    errorMessage->clear();

  if ( !overwrite )
  {
    QFileInfo fi( uri );
    if ( fi.exists() )
    {
      if ( errorMessage )
        *errorMessage += QObject::tr( "Unable to create the datasource. %1 exists and overwrite flag is false." )
                         .arg( uri );
      return QgsVectorLayerImport::ErrCreateDataSource;
    }
  }

  QgsVectorFileWriter *writer = new QgsVectorFileWriter(
    uri, encoding, fields, wkbType,
    srs, driverName, dsOptions, layerOptions );

  QgsVectorFileWriter::WriterError error = writer->hasError();
  if ( error )
  {
    if ( errorMessage )
      *errorMessage += writer->errorMessage();

    delete writer;
    return ( QgsVectorLayerImport::ImportError ) error;
  }

  if ( oldToNewAttrIdxMap )
  {
    QMap<int, int> attrIdxMap = writer->attrIdxToOgrIdx();
    for ( QMap<int, int>::const_iterator attrIt = attrIdxMap.begin(); attrIt != attrIdxMap.end(); ++attrIt )
    {
      oldToNewAttrIdxMap->insert( attrIt.key(), *attrIt );
    }
  }

  delete writer;
  return QgsVectorLayerImport::NoError;
}


QgsOgrProvider::QgsOgrProvider( QString const & uri )
    : QgsVectorDataProvider( uri )
    , mFirstFieldIsFid( false )
    , ogrDataSource( nullptr )
    , mExtent( nullptr )
    , ogrLayer( nullptr )
    , ogrOrigLayer( nullptr )
    , mLayerIndex( 0 )
    , mIsSubLayer( false )
    , mOgrGeometryTypeFilter( wkbUnknown )
    , ogrDriver( nullptr )
    , mValid( false )
    , mOGRGeomType( wkbUnknown )
    , mFeaturesCounted( -1 )
    , mWriteAccess( false )
    , mWriteAccessPossible( false )
    , mDynamicWriteAccess( false )
    , mShapefileMayBeCorrupted( false )
    , mUpdateModeStackDepth( 0 )
    , mCapabilities( 0 )
{
  QgsApplication::registerOgrDrivers();

  QSettings settings;
  CPLSetConfigOption( "SHAPE_ENCODING", settings.value( "/qgis/ignoreShapeEncoding", true ).toBool() ? "" : nullptr );

  // make connection to the data source

  QgsDebugMsg( "Data source uri is [" + uri + ']' );

  // try to open for update, but disable error messages to avoid a
  // message if the file is read only, because we cope with that
  // ourselves.

  // This part of the code parses the uri transmitted to the ogr provider to
  // get the options the client wants us to apply

  // If there is no | in the uri, then the uri is just the filename. The loaded
  // layer will be layer 0.

  if ( !uri.contains( '|', Qt::CaseSensitive ) )
  {
    mFilePath = uri;
    mLayerIndex = 0;
    mLayerName = QString::null;
  }
  else
  {
    QStringList theURIParts = uri.split( '|' );
    mFilePath = theURIParts.at( 0 );

    for ( int i = 1 ; i < theURIParts.size(); i++ )
    {
      QString part = theURIParts.at( i );
      int pos = part.indexOf( '=' );
      QString field = part.left( pos );
      QString value = part.mid( pos + 1 );

      if ( field == "layerid" )
      {
        bool ok;
        mLayerIndex = value.toInt( &ok );
        if ( ! ok )
        {
          mLayerIndex = -1;
        }
        else
        {
          mIsSubLayer = true;
        }
      }
      else if ( field == "layername" )
      {
        mLayerName = value;
        mIsSubLayer = true;
      }

      if ( field == "subset" )
      {
        mSubsetString = value;
      }

      if ( field == "geometrytype" )
      {
        mOgrGeometryTypeFilter = ogrWkbGeometryTypeFromName( value );
      }
    }
  }

  open( OpenModeInitial );

  mNativeTypes
  << QgsVectorDataProvider::NativeType( tr( "Whole number (integer)" ), "integer", QVariant::Int, 1, 10 )
#if defined(GDAL_VERSION_NUM) && GDAL_VERSION_NUM >= 2000000
  << QgsVectorDataProvider::NativeType( tr( "Whole number (integer 64 bit)" ), "integer64", QVariant::LongLong, 1, 10 )
#endif
  << QgsVectorDataProvider::NativeType( tr( "Decimal number (real)" ), "double", QVariant::Double, 1, 20, 0, 15 )
  << QgsVectorDataProvider::NativeType( tr( "Text (string)" ), "string", QVariant::String, 1, 255 )
  << QgsVectorDataProvider::NativeType( tr( "Date" ), "date", QVariant::Date, 8, 8 );

  // Some drivers do not support datetime type
  // Please help to fill this list
  if ( ogrDriverName != "ESRI Shapefile" )
  {
    mNativeTypes
    << QgsVectorDataProvider::NativeType( tr( "Time" ), "time", QVariant::Time, -1, -1 )
    << QgsVectorDataProvider::NativeType( tr( "Date & Time" ), "datetime", QVariant::DateTime );
  }

  QgsOgrConnPool::instance()->ref( dataSourceUri() );
}

QgsOgrProvider::~QgsOgrProvider()
{
  close();
  QgsOgrConnPool::instance()->unref( dataSourceUri() );
  // We must also make sure to flush unusef cached connections so that
  // the file can be removed (#15137)
  QgsOgrConnPool::instance()->invalidateConnections( dataSourceUri() );
}

QgsAbstractFeatureSource* QgsOgrProvider::featureSource() const
{
  return new QgsOgrFeatureSource( this );
}

bool QgsOgrProvider::setSubsetString( const QString& theSQL, bool updateFeatureCount )
{
  QgsCPLErrorHandler handler;

  if ( !ogrDataSource )
    return false;

  if ( theSQL == mSubsetString && mFeaturesCounted >= 0 )
    return true;

  OGRLayerH prevLayer = ogrLayer;
  QString prevSubsetString = mSubsetString;
  mSubsetString = theSQL;

  if ( !mSubsetString.isEmpty() )
  {

    ogrLayer = setSubsetString( ogrOrigLayer, ogrDataSource );
    if ( !ogrLayer )
    {
      pushError( tr( "OGR[%1] error %2: %3" ).arg( CPLGetLastErrorType() ).arg( CPLGetLastErrorNo() ).arg( CPLGetLastErrorMsg() ) );
      ogrLayer = prevLayer;
      mSubsetString = prevSubsetString;
      return false;
    }
  }
  else
  {
    ogrLayer = ogrOrigLayer;
  }

  if ( prevLayer != ogrOrigLayer )
  {
    OGR_DS_ReleaseResultSet( ogrDataSource, prevLayer );
  }

  QString uri = mFilePath;
  if ( !mLayerName.isNull() )
  {
    uri += QString( "|layername=%1" ).arg( mLayerName );
  }
  else if ( mLayerIndex >= 0 )
  {
    uri += QString( "|layerid=%1" ).arg( mLayerIndex );
  }

  if ( !mSubsetString.isEmpty() )
  {
    uri += QString( "|subset=%1" ).arg( mSubsetString );
  }

  if ( mOgrGeometryTypeFilter != wkbUnknown )
  {
    uri += QString( "|geometrytype=%1" ).arg( ogrWkbGeometryTypeName( mOgrGeometryTypeFilter ) );
  }

  setDataSourceUri( uri );

  OGR_L_ResetReading( ogrLayer );

  // getting the total number of features in the layer
  // TODO: This can be expensive, do we really need it!
  if ( updateFeatureCount )
  {
    recalculateFeatureCount();
  }

  // check the validity of the layer
  QgsDebugMsg( "checking validity" );
  loadFields();
  QgsDebugMsg( "Done checking validity" );

  updateExtents();

  emit dataChanged();

  return true;
}

QString QgsOgrProvider::subsetString()
{
  return mSubsetString;
}

QString QgsOgrProvider::ogrWkbGeometryTypeName( OGRwkbGeometryType type ) const
{
  QString geom;

  // GDAL 2.1 can return M/ZM geometries
#if defined(GDAL_COMPUTE_VERSION) && GDAL_VERSION_NUM >= GDAL_COMPUTE_VERSION(2,1,0)
  if ( wkbHasM( type ) )
  {
    geom = ogrWkbGeometryTypeName( wkbFlatten( type ) );
    if ( wkbHasZ( type ) )
      geom += "Z";
    if ( wkbHasM( type ) )
      geom += "M";
    return geom;
  }
#endif

  switch (( long )type )
  {
    case wkbUnknown:
      geom = "Unknown";
      break;
    case wkbPoint:
      geom = "Point";
      break;
    case wkbLineString:
      geom = "LineString";
      break;
    case wkbPolygon:
      geom = "Polygon";
      break;
    case wkbMultiPoint:
      geom = "MultiPoint";
      break;
    case wkbMultiLineString:
      geom = "MultiLineString";
      break;
    case wkbMultiPolygon:
      geom = "MultiPolygon";
      break;
    case wkbGeometryCollection:
      geom = "GeometryCollection";
      break;
#if defined(GDAL_COMPUTE_VERSION) && GDAL_VERSION_NUM >= GDAL_COMPUTE_VERSION(2,0,0)
    case wkbCircularString:
      geom = "CircularString";
      break;
    case wkbCompoundCurve:
      geom = "CompoundCurve";
      break;
    case wkbCurvePolygon:
      geom = "CurvePolygon";
      break;
    case wkbMultiCurve:
      geom = "MultiCurve";
      break;
    case wkbMultiSurface:
      geom = "MultiSurface";
      break;
    case wkbCircularStringZ:
      geom = "CircularStringZ";
      break;
    case wkbCompoundCurveZ:
      geom = "CompoundCurveZ";
      break;
    case wkbCurvePolygonZ:
      geom = "CurvePolygonZ";
      break;
    case wkbMultiCurveZ:
      geom = "MultiCurveZ";
      break;
    case wkbMultiSurfaceZ:
      geom = "MultiSurfaceZ";
      break;
#endif
    case wkbNone:
      geom = "None";
      break;
    case wkbUnknown | wkb25DBit:
      geom = "Unknown25D";
      break;
    case wkbPoint25D:
      geom = "Point25D";
      break;
    case wkbLineString25D:
      geom = "LineString25D";
      break;
    case wkbPolygon25D:
      geom = "Polygon25D";
      break;
    case wkbMultiPoint25D:
      geom = "MultiPoint25D";
      break;
    case wkbMultiLineString25D:
      geom = "MultiLineString25D";
      break;
    case wkbMultiPolygon25D:
      geom = "MultiPolygon25D";
      break;
    case wkbGeometryCollection25D:
      geom = "GeometryCollection25D";
      break;
    default:
      // Do not use ':', as it will mess with the separator used by QgsSublayersDialog::populateLayers()
      geom = QString( "Unknown WKB (%1)" ).arg( type );
  }
  return geom;
}

OGRwkbGeometryType QgsOgrProvider::ogrWkbGeometryTypeFromName( const QString& typeName ) const
{
  if ( typeName == "Point" ) return wkbPoint;
  else if ( typeName == "LineString" ) return wkbLineString;
  else if ( typeName == "Polygon" ) return wkbPolygon;
  else if ( typeName == "MultiPoint" ) return wkbMultiPoint;
  else if ( typeName == "MultiLineString" ) return wkbMultiLineString;
  else if ( typeName == "MultiPolygon" ) return wkbMultiPolygon;
  else if ( typeName == "GeometryCollection" ) return wkbGeometryCollection;
  else if ( typeName == "None" ) return wkbNone;
  else if ( typeName == "Point25D" ) return wkbPoint25D;
  else if ( typeName == "LineString25D" ) return wkbLineString25D;
  else if ( typeName == "Polygon25D" ) return wkbPolygon25D;
  else if ( typeName == "MultiPoint25D" ) return wkbMultiPoint25D;
  else if ( typeName == "MultiLineString25D" ) return wkbMultiLineString25D;
  else if ( typeName == "MultiPolygon25D" ) return wkbMultiPolygon25D;
  else if ( typeName == "GeometryCollection25D" ) return wkbGeometryCollection25D;
  return wkbUnknown;
}

QStringList QgsOgrProvider::subLayers() const
{
  QgsDebugMsg( "Entered." );
  if ( !mValid )
  {
    return QStringList();
  }

  if ( !mSubLayerList.isEmpty() )
    return mSubLayerList;

  for ( unsigned int i = 0; i < layerCount() ; i++ )
  {
    OGRLayerH layer = OGR_DS_GetLayer( ogrDataSource, i );
    OGRFeatureDefnH fdef = OGR_L_GetLayerDefn( layer );
    QString theLayerName = FROM8( OGR_FD_GetName( fdef ) );
    OGRwkbGeometryType layerGeomType = OGR_FD_GetGeomType( fdef );

    // ignore this layer if a sublayer was requested and it is not this one
    if ( mIsSubLayer &&
         (( !mLayerName.isNull() && theLayerName != mLayerName ) ||
          ( mLayerName.isNull() && mLayerIndex >= 0 && i != ( unsigned int )mLayerIndex ) ) )
    {
      QgsDebugMsg( QString( "subLayers() ignoring layer #%1 (%2)" ).arg( i ).arg( theLayerName ) );
      continue;
    }

    QgsDebugMsg( QString( "id = %1 name = %2 layerGeomType = %3" ).arg( i ).arg( theLayerName ).arg( layerGeomType ) );

    if ( wkbFlatten( layerGeomType ) != wkbUnknown )
    {
      int theLayerFeatureCount = OGR_L_GetFeatureCount( layer, 0 );

      QString geom = ogrWkbGeometryTypeName( layerGeomType );

      mSubLayerList << QString( "%1:%2:%3:%4" ).arg( i ).arg( theLayerName, theLayerFeatureCount == -1 ? tr( "Unknown" ) : QString::number( theLayerFeatureCount ), geom );
    }
    else
    {
      QgsDebugMsg( "Unknown geometry type, count features for each geometry type" );
      // Add virtual sublayers for supported geometry types if layer type is unknown
      // Count features for geometry types
      QMap<OGRwkbGeometryType, int> fCount;
      // TODO: avoid reading attributes, setRelevantFields cannot be called here because it is not constant
      //setRelevantFields( ogrLayer, true, QgsAttributeList() );
      OGR_L_ResetReading( layer );
      OGRFeatureH fet;
      while (( fet = OGR_L_GetNextFeature( layer ) ) )
      {
        OGRGeometryH geom = OGR_F_GetGeometryRef( fet );
        if ( geom )
        {
          OGRwkbGeometryType gType = ogrWkbSingleFlatten( OGR_G_GetGeometryType( geom ) );
          fCount[gType] = fCount.value( gType ) + 1;
        }
        OGR_F_Destroy( fet );
      }
      OGR_L_ResetReading( layer );
      // it may happen that there are no features in the layer, in that case add unknown type
      // to show to user that the layer exists but it is empty
      if ( fCount.isEmpty() )
      {
        fCount[wkbUnknown] = 0;
      }

#if defined(GDAL_COMPUTE_VERSION) && GDAL_VERSION_NUM >= GDAL_COMPUTE_VERSION(2,0,0)
      // When there are CurvePolygons, promote Polygons
      if ( fCount.contains( wkbPolygon ) && fCount.contains( wkbCurvePolygon ) )
      {
        fCount[wkbCurvePolygon] += fCount.value( wkbPolygon );
        fCount.remove( wkbPolygon );
      }
      // When there are CompoundCurves, promote LineStrings and CircularStrings
      if ( fCount.contains( wkbLineString ) && fCount.contains( wkbCompoundCurve ) )
      {
        fCount[wkbCompoundCurve] += fCount.value( wkbLineString );
        fCount.remove( wkbLineString );
      }
      if ( fCount.contains( wkbCircularString ) && fCount.contains( wkbCompoundCurve ) )
      {
        fCount[wkbCompoundCurve] += fCount.value( wkbCircularString );
        fCount.remove( wkbCircularString );
      }
#endif

#if defined(GDAL_COMPUTE_VERSION) && GDAL_VERSION_NUM >= GDAL_COMPUTE_VERSION(2,0,0)
      bool bIs25D = ( wkbHasZ( layerGeomType ) != 0 );
#else
      bool bIs25D = (( layerGeomType & wkb25DBit ) != 0 );
#endif
      QMap<OGRwkbGeometryType, int>::const_iterator countIt = fCount.constBegin();
      for ( ; countIt != fCount.constEnd(); ++countIt )
      {
#if defined(GDAL_COMPUTE_VERSION) && GDAL_VERSION_NUM >= GDAL_COMPUTE_VERSION(2,0,0)
        QString geom = ogrWkbGeometryTypeName(( bIs25D ) ? wkbSetZ( countIt.key() ) : countIt.key() );
#else
        QString geom = ogrWkbGeometryTypeName(( bIs25D ) ? ( OGRwkbGeometryType )( countIt.key() | wkb25DBit ) : countIt.key() );
#endif

        QString sl = QString( "%1:%2:%3:%4" ).arg( i ).arg( theLayerName ).arg( fCount.value( countIt.key() ) ).arg( geom );
        QgsDebugMsg( "sub layer: " + sl );
        mSubLayerList << sl;
      }
    }
  }

  return mSubLayerList;
}

void QgsOgrProvider::setEncoding( const QString& e )
{
#if defined(OLCStringsAsUTF8)
  QSettings settings;
  if (( ogrDriverName == "ESRI Shapefile" && settings.value( "/qgis/ignoreShapeEncoding", true ).toBool() ) || !OGR_L_TestCapability( ogrLayer, OLCStringsAsUTF8 ) )
  {
    QgsVectorDataProvider::setEncoding( e );
  }
  else
  {
    QgsVectorDataProvider::setEncoding( "UTF-8" );
  }
#else
  QgsVectorDataProvider::setEncoding( e );
#endif

  loadFields();
}

// This is reused by dataItem
OGRwkbGeometryType QgsOgrProvider::getOgrGeomType( OGRLayerH ogrLayer )
{
  OGRFeatureDefnH fdef = OGR_L_GetLayerDefn( ogrLayer );
  OGRwkbGeometryType geomType = wkbUnknown;
  if ( fdef )
  {
    geomType = OGR_FD_GetGeomType( fdef );

    // Handle wkbUnknown and its Z/M variants. QGIS has no unknown Z/M variants,
    // so just use flat wkbUnknown
    if ( wkbFlatten( geomType ) == wkbUnknown )
      geomType = wkbUnknown;

    // Some ogr drivers (e.g. GML) are not able to determine the geometry type of a layer like this.
    // In such cases, we use virtual sublayers for each geometry if the layer contains
    // multiple geometries (see subLayers) otherwise we guess geometry type from the first
    // feature that has a geometry (limit us to a few features, not the whole layer)
    if ( geomType == wkbUnknown )
    {
      geomType = wkbNone;
      OGR_L_ResetReading( ogrLayer );
      for ( int i = 0; i < 10; i++ )
      {
        OGRFeatureH nextFeature = OGR_L_GetNextFeature( ogrLayer );
        if ( !nextFeature )
          break;

        OGRGeometryH geometry = OGR_F_GetGeometryRef( nextFeature );
        if ( geometry )
        {
          geomType = OGR_G_GetGeometryType( geometry );
        }
        OGR_F_Destroy( nextFeature );
        if ( geomType != wkbNone )
          break;
      }
      OGR_L_ResetReading( ogrLayer );
    }
  }
  return geomType;
}

void QgsOgrProvider::loadFields()
{
  QgsOgrConnPool::instance()->invalidateConnections( dataSourceUri() );
  //the attribute fields need to be read again when the encoding changes
  mAttributeFields.clear();
  if ( !ogrLayer )
    return;

  if ( mOgrGeometryTypeFilter != wkbUnknown )
  {
    mOGRGeomType = mOgrGeometryTypeFilter;
  }
  else
  {
    mOGRGeomType = getOgrGeomType( ogrLayer );
  }
  OGRFeatureDefnH fdef = OGR_L_GetLayerDefn( ogrLayer );
  if ( fdef )
  {
    // Expose the OGR FID if it comes from a "real" column (typically GPKG)
    // and make sure that this FID column is not exposed as a regular OGR field (shouldn't happen normally)
    mFirstFieldIsFid = !( EQUAL( OGR_L_GetFIDColumn( ogrLayer ), "" ) ) &&
                       OGR_FD_GetFieldIndex( fdef, OGR_L_GetFIDColumn( ogrLayer ) ) < 0;
    if ( mFirstFieldIsFid )
    {
      mAttributeFields.append(
        QgsField(
          OGR_L_GetFIDColumn( ogrLayer ),
          QVariant::LongLong,
          "Integer64"
        )
      );
    }

    for ( int i = 0; i < OGR_FD_GetFieldCount( fdef ); ++i )
    {
      OGRFieldDefnH fldDef = OGR_FD_GetFieldDefn( fdef, i );
      OGRFieldType ogrType = OGR_Fld_GetType( fldDef );
      QVariant::Type varType;
      switch ( ogrType )
      {
        case OFTInteger:
          varType = QVariant::Int;
          break;
#if defined(GDAL_VERSION_NUM) && GDAL_VERSION_NUM >= 2000000
        case OFTInteger64:
          varType = QVariant::LongLong;
          break;
#endif
        case OFTReal:
          varType = QVariant::Double;
          break;
#if defined(GDAL_VERSION_NUM) && GDAL_VERSION_NUM >= 1400
        case OFTDate:
          varType = QVariant::Date;
          break;
        case OFTTime:
          varType = QVariant::Time;
          break;
        case OFTDateTime:
          varType = QVariant::DateTime;
          break;
        case OFTString:
#endif
        default:
          varType = QVariant::String; // other unsupported, leave it as a string
      }

      //TODO: fix this hack
#ifdef ANDROID
      QString name = OGR_Fld_GetNameRef( fldDef );
#else
      QString name = mEncoding->toUnicode( OGR_Fld_GetNameRef( fldDef ) );
#endif

      if ( mAttributeFields.indexFromName( name ) != -1 )
      {

        QString tmpname = name + "_%1";
        int fix = 0;

        while ( mAttributeFields.indexFromName( name ) != -1 )
        {
          name = tmpname.arg( ++fix );
        }
      }

      int width = OGR_Fld_GetWidth( fldDef );
      int prec = OGR_Fld_GetPrecision( fldDef );
      if ( prec > 0 )
        width -= 1;

      mAttributeFields.append(
        QgsField(
          name,
          varType,
#ifdef ANDROID
          OGR_GetFieldTypeName( ogrType ),
#else
          mEncoding->toUnicode( OGR_GetFieldTypeName( ogrType ) ),
#endif
          width, prec
        )
      );
    }
  }
}


QString QgsOgrProvider::storageType() const
{
  // Delegate to the driver loaded in by OGR
  return ogrDriverName;
}


void QgsOgrProvider::setRelevantFields( OGRLayerH ogrLayer, bool fetchGeometry, const QgsAttributeList &fetchAttributes )
{
  QgsOgrProviderUtils::setRelevantFields( ogrLayer, mAttributeFields.count(), fetchGeometry, fetchAttributes, mFirstFieldIsFid );
}


void QgsOgrProviderUtils::setRelevantFields( OGRLayerH ogrLayer, int fieldCount, bool fetchGeometry, const QgsAttributeList &fetchAttributes, bool firstAttrIsFid )
{
#if defined(GDAL_VERSION_NUM) && GDAL_VERSION_NUM >= 1800
  if ( OGR_L_TestCapability( ogrLayer, OLCIgnoreFields ) )
  {
    QVector<const char*> ignoredFields;
    OGRFeatureDefnH featDefn = OGR_L_GetLayerDefn( ogrLayer );
    for ( int i = ( firstAttrIsFid ? 1 : 0 ); i < fieldCount; i++ )
    {
      if ( !fetchAttributes.contains( i ) )
      {
        // add to ignored fields
        ignoredFields.append( OGR_Fld_GetNameRef( OGR_FD_GetFieldDefn( featDefn, firstAttrIsFid ? i - 1 : i ) ) );
      }
    }

    if ( !fetchGeometry )
      ignoredFields.append( "OGR_GEOMETRY" );
    ignoredFields.append( "OGR_STYLE" ); // not used by QGIS
    ignoredFields.append( nullptr );

    OGR_L_SetIgnoredFields( ogrLayer, ignoredFields.data() );
  }
#else
  Q_UNUSED( ogrLayer );
  Q_UNUSED( fetchGeometry );
  Q_UNUSED( fetchAttributes );
#endif
}

QgsFeatureIterator QgsOgrProvider::getFeatures( const QgsFeatureRequest& request )
{
  return QgsFeatureIterator( new QgsOgrFeatureIterator( static_cast<QgsOgrFeatureSource*>( featureSource() ), true, request ) );
}


unsigned char * QgsOgrProvider::getGeometryPointer( OGRFeatureH fet )
{
  OGRGeometryH geom = OGR_F_GetGeometryRef( fet );
  unsigned char *gPtr = nullptr;

  if ( !geom )
    return nullptr;

  // get the wkb representation
  gPtr = new unsigned char[OGR_G_WkbSize( geom )];

  OGR_G_ExportToWkb( geom, ( OGRwkbByteOrder ) QgsApplication::endian(), gPtr );
  return gPtr;
}


QgsRectangle QgsOgrProvider::extent()
{
  if ( !mExtent )
  {
    mExtent = new OGREnvelope();

    // get the extent_ (envelope) of the layer
    QgsDebugMsg( "Starting get extent" );

    // TODO: This can be expensive, do we really need it!
    if ( ogrLayer == ogrOrigLayer )
    {
      OGR_L_GetExtent( ogrLayer, mExtent, true );
    }
    else
    {
      mExtent->MinX = std::numeric_limits<double>::max();
      mExtent->MinY = std::numeric_limits<double>::max();
      mExtent->MaxX = -std::numeric_limits<double>::max();
      mExtent->MaxY = -std::numeric_limits<double>::max();

      OGRFeatureH f;

      OGR_L_ResetReading( ogrLayer );
      while (( f = OGR_L_GetNextFeature( ogrLayer ) ) )
      {
        OGRGeometryH g = OGR_F_GetGeometryRef( f );
        if ( g )
        {
          OGREnvelope env;
          OGR_G_GetEnvelope( g, &env );

          mExtent->MinX = qMin( mExtent->MinX, env.MinX );
          mExtent->MinY = qMin( mExtent->MinY, env.MinY );
          mExtent->MaxX = qMax( mExtent->MaxX, env.MaxX );
          mExtent->MaxY = qMax( mExtent->MaxY, env.MaxY );
        }

        OGR_F_Destroy( f );
      }
      OGR_L_ResetReading( ogrLayer );
    }

    QgsDebugMsg( "Finished get extent" );
  }

  mExtentRect.set( mExtent->MinX, mExtent->MinY, mExtent->MaxX, mExtent->MaxY );
  return mExtentRect;
}

void QgsOgrProvider::updateExtents()
{
  delete mExtent;
  mExtent = nullptr;
}

size_t QgsOgrProvider::layerCount() const
{
  if ( !mValid )
    return 0;
  return OGR_DS_GetLayerCount( ogrDataSource );
} // QgsOgrProvider::layerCount()


/**
 * Return the feature type
 */
QGis::WkbType QgsOgrProvider::geometryType() const
{
  return static_cast<QGis::WkbType>( mOGRGeomType );
}

/**
 * Return the feature count
 */
long QgsOgrProvider::featureCount() const
{
  return mFeaturesCounted;
}


const QgsFields & QgsOgrProvider::fields() const
{
  return mAttributeFields;
}


//TODO - add sanity check for shape file layers, to include cheking to
//       see if the .shp, .dbf, .shx files are all present and the layer
//       actually has features
bool QgsOgrProvider::isValid()
{
  return mValid;
}

// Drivers may be more tolerant than we really wish (e.g. GeoPackage driver
// may accept any geometry type)
OGRGeometryH QgsOgrProvider::ConvertGeometryIfNecessary( OGRGeometryH hGeom )
{
  if ( hGeom == nullptr )
    return hGeom;
  OGRwkbGeometryType layerGeomType = OGR_L_GetGeomType( ogrLayer );
  OGRwkbGeometryType flattenLayerGeomType = wkbFlatten( layerGeomType );
  OGRwkbGeometryType geomType = OGR_G_GetGeometryType( hGeom );
  OGRwkbGeometryType flattenGeomType = wkbFlatten( geomType );

  if ( flattenLayerGeomType == wkbUnknown || flattenLayerGeomType == flattenGeomType )
  {
    return hGeom;
  }
  if ( flattenLayerGeomType == wkbMultiPolygon && flattenGeomType == wkbPolygon )
  {
    return OGR_G_ForceToMultiPolygon( hGeom );
  }
  if ( flattenLayerGeomType == wkbMultiLineString && flattenGeomType == wkbLineString )
  {
    return OGR_G_ForceToMultiLineString( hGeom );
  }
#if defined(GDAL_COMPUTE_VERSION) && GDAL_VERSION_NUM >= GDAL_COMPUTE_VERSION(2,0,0)
  return OGR_G_ForceTo( hGeom, layerGeomType, nullptr );
#else
  return hGeom;
#endif
}

bool QgsOgrProvider::addFeature( QgsFeature& f )
{
  bool returnValue = true;
  OGRFeatureDefnH fdef = OGR_L_GetLayerDefn( ogrLayer );
  OGRFeatureH feature = OGR_F_Create( fdef );

  if ( f.constGeometry() && f.constGeometry()->wkbSize() > 0 )
  {
    const unsigned char* wkb = f.constGeometry()->asWkb();
    OGRGeometryH geom = nullptr;

    if ( wkb )
    {
      if ( OGR_G_CreateFromWkb( const_cast<unsigned char *>( wkb ), nullptr, &geom, f.constGeometry()->wkbSize() ) != OGRERR_NONE )
      {
        pushError( tr( "OGR error creating wkb for feature %1: %2" ).arg( f.id() ).arg( CPLGetLastErrorMsg() ) );
        return false;
      }

      geom = ConvertGeometryIfNecessary( geom );

      OGR_F_SetGeometryDirectly( feature, geom );
    }
  }

  QgsAttributes attrs = f.attributes();

  QgsLocaleNumC l;

  int qgisAttId = ( mFirstFieldIsFid ) ? 1 : 0;
  // If the first attribute is the FID and the user has set it, then use it
  if ( mFirstFieldIsFid && attrs.count() > 0 )
  {
    QVariant attrFid = attrs.at( 0 );
    if ( !attrFid.isNull() )
    {
      bool ok = false;
      qlonglong id = attrFid.toLongLong( &ok );
      if ( ok )
      {
#if GDAL_VERSION_MAJOR >= 2
        OGR_F_SetFID( feature, static_cast<GIntBig>( id ) );
#else
        OGR_F_SetFID( feature, static_cast<long>( id ) );
#endif
      }
    }
  }

  //add possible attribute information
  for ( int ogrAttId = 0; qgisAttId < attrs.count(); ++qgisAttId, ++ogrAttId )
  {
    // don't try to set field from attribute map if it's not present in layer
    if ( ogrAttId >= OGR_FD_GetFieldCount( fdef ) )
      continue;

    //if(!s.isEmpty())
    // continue;
    //
    OGRFieldDefnH fldDef = OGR_FD_GetFieldDefn( fdef, ogrAttId );
    OGRFieldType type = OGR_Fld_GetType( fldDef );

    QVariant attrVal = attrs.at( qgisAttId );
    if ( attrVal.isNull() || ( type != OFTString && attrVal.toString().isEmpty() ) )
    {
      OGR_F_UnsetField( feature, ogrAttId );
    }
    else
    {
      switch ( type )
      {
        case OFTInteger:
          OGR_F_SetFieldInteger( feature, ogrAttId, attrVal.toInt() );
          break;


#if defined(GDAL_VERSION_NUM) && GDAL_VERSION_NUM >= 2000000
        case OFTInteger64:
          OGR_F_SetFieldInteger64( feature, ogrAttId, attrVal.toLongLong() );
          break;
#endif

        case OFTReal:
          OGR_F_SetFieldDouble( feature, ogrAttId, attrVal.toDouble() );
          break;

        case OFTDate:
          OGR_F_SetFieldDateTime( feature, ogrAttId,
                                  attrVal.toDate().year(),
                                  attrVal.toDate().month(),
                                  attrVal.toDate().day(),
                                  0, 0, 0,
                                  0 );
          break;

        case OFTTime:
          OGR_F_SetFieldDateTime( feature, ogrAttId,
                                  0, 0, 0,
                                  attrVal.toTime().hour(),
                                  attrVal.toTime().minute(),
                                  attrVal.toTime().second(),
                                  0 );
          break;

        case OFTDateTime:
          OGR_F_SetFieldDateTime( feature, ogrAttId,
                                  attrVal.toDateTime().date().year(),
                                  attrVal.toDateTime().date().month(),
                                  attrVal.toDateTime().date().day(),
                                  attrVal.toDateTime().time().hour(),
                                  attrVal.toDateTime().time().minute(),
                                  attrVal.toDateTime().time().second(),
                                  0 );
          break;

        case OFTString:
          QgsDebugMsg( QString( "Writing string attribute %1 with %2, encoding %3" )
                       .arg( qgisAttId )
                       .arg( attrVal.toString(),
                             mEncoding->name().data() ) );
          OGR_F_SetFieldString( feature, ogrAttId, mEncoding->fromUnicode( attrVal.toString() ).constData() );
          break;

        default:
          QgsMessageLog::logMessage( tr( "type %1 for attribute %2 not found" ).arg( type ).arg( qgisAttId ), tr( "OGR" ) );
          break;
      }
    }
  }

  if ( OGR_L_CreateFeature( ogrLayer, feature ) != OGRERR_NONE )
  {
    pushError( tr( "OGR error creating feature %1: %2" ).arg( f.id() ).arg( CPLGetLastErrorMsg() ) );
    returnValue = false;
  }
  else
  {
    QgsFeatureId id = static_cast<QgsFeatureId>( OGR_F_GetFID( feature ) );
    if ( id >= 0 )
    {
      f.setFeatureId( id );

      if ( mFirstFieldIsFid && attrs.count() > 0 )
      {
        f.setAttribute( 0, id );
      }
    }
  }
  OGR_F_Destroy( feature );

  return returnValue;
}


bool QgsOgrProvider::addFeatures( QgsFeatureList & flist )
{
  if ( !doInitialActionsForEdition() )
    return false;

  setRelevantFields( ogrLayer, true, attributeIndexes() );

  bool returnvalue = true;
  for ( QgsFeatureList::iterator it = flist.begin(); it != flist.end(); ++it )
  {
    if ( !addFeature( *it ) )
    {
      returnvalue = false;
    }
  }

  if ( !syncToDisc() )
  {
    returnvalue = false;
  }

  recalculateFeatureCount();

  if ( returnvalue )
    clearMinMaxCache();

  return returnvalue;
}

bool QgsOgrProvider::addAttributes( const QList<QgsField> &attributes )
{
  if ( !doInitialActionsForEdition() )
    return false;

  if ( ogrDriverName == "MapInfo File" )
  {
    // adding attributes in mapinfo requires to be able to delete the .dat file
    // so drop any cached connections.
    QgsOgrConnPool::instance()->invalidateConnections( dataSourceUri() );
  }

  bool returnvalue = true;

  for ( QList<QgsField>::const_iterator iter = attributes.begin(); iter != attributes.end(); ++iter )
  {
    OGRFieldType type;

    switch ( iter->type() )
    {
      case QVariant::Int:
        type = OFTInteger;
        break;
#if defined(GDAL_VERSION_NUM) && GDAL_VERSION_NUM >= 2000000
      case QVariant::LongLong:
        type = OFTInteger64;
        break;
#endif
      case QVariant::Double:
        type = OFTReal;
        break;
      case QVariant::Date:
        type = OFTDate;
        break;
      case QVariant::Time:
        type = OFTTime;
        break;
      case QVariant::DateTime:
        type = OFTDateTime;
        break;
      case QVariant::String:
        type = OFTString;
        break;
      default:
        pushError( tr( "type %1 for field %2 not found" ).arg( iter->typeName(), iter->name() ) );
        returnvalue = false;
        continue;
    }

    OGRFieldDefnH fielddefn = OGR_Fld_Create( mEncoding->fromUnicode( iter->name() ).constData(), type );
    int width = iter->length();
    if ( iter->precision() )
      width += 1;
    OGR_Fld_SetWidth( fielddefn, width );
    OGR_Fld_SetPrecision( fielddefn, iter->precision() );

    if ( OGR_L_CreateField( ogrLayer, fielddefn, true ) != OGRERR_NONE )
    {
      pushError( tr( "OGR error creating field %1: %2" ).arg( iter->name(), CPLGetLastErrorMsg() ) );
      returnvalue = false;
    }
    OGR_Fld_Destroy( fielddefn );
  }
  loadFields();
  return returnvalue;
}

bool QgsOgrProvider::deleteAttributes( const QgsAttributeIds &attributes )
{
  if ( !doInitialActionsForEdition() )
    return false;

#if defined(GDAL_VERSION_NUM) && GDAL_VERSION_NUM >= 1900
  bool res = true;
  QList<int> attrsLst = attributes.toList();
  // sort in descending order
  qSort( attrsLst.begin(), attrsLst.end(), qGreater<int>() );
  Q_FOREACH ( int attr, attrsLst )
  {
    if ( mFirstFieldIsFid )
    {
      if ( attr == 0 )
      {
        pushError( tr( "Cannot delete feature id column" ) );
        res = false;
        break;
      }
      else
      {
        --attr;
      }
    }
    if ( OGR_L_DeleteField( ogrLayer, attr ) != OGRERR_NONE )
    {
      pushError( tr( "OGR error deleting field %1: %2" ).arg( attr ).arg( CPLGetLastErrorMsg() ) );
      res = false;
    }
  }
  loadFields();
  return res;
#else
  Q_UNUSED( attributes );
  pushError( tr( "Deleting fields is not supported prior to GDAL 1.9.0" ) );
  return false;
#endif
}

bool QgsOgrProvider::renameAttributes( const QgsFieldNameMap& renamedAttributes )
{
  if ( !doInitialActionsForEdition() )
    return false;

#if defined(GDAL_VERSION_NUM) && GDAL_VERSION_NUM >= 1900
  QgsFieldNameMap::const_iterator renameIt = renamedAttributes.constBegin();
  bool result = true;
  for ( ; renameIt != renamedAttributes.constEnd(); ++renameIt )
  {
    int fieldIndex = renameIt.key();
    if ( fieldIndex < 0 || fieldIndex >= mAttributeFields.count() )
    {
      pushError( tr( "Invalid attribute index" ) );
      result = false;
      continue;
    }
    if ( mAttributeFields.indexFromName( renameIt.value() ) >= 0 )
    {
      //field name already in use
      pushError( tr( "Error renaming field %1: name '%2' already exists" ).arg( fieldIndex ).arg( renameIt.value() ) );
      result = false;
      continue;
    }
    int ogrFieldIndex = fieldIndex;
    if ( mFirstFieldIsFid )
    {
      ogrFieldIndex -= 1;
      if ( ogrFieldIndex < 0 )
      {
        pushError( tr( "Invalid attribute index" ) );
        result = false;
        continue;
      }
    }

    //type does not matter, it will not be used
    OGRFieldDefnH fld = OGR_Fld_Create( mEncoding->fromUnicode( renameIt.value() ), OFTReal );
    if ( OGR_L_AlterFieldDefn( ogrLayer, ogrFieldIndex, fld, ALTER_NAME_FLAG ) != OGRERR_NONE )
    {
      pushError( tr( "OGR error renaming field %1: %2" ).arg( fieldIndex ).arg( CPLGetLastErrorMsg() ) );
      result = false;
    }
    OGR_Fld_Destroy( fld );
  }
  loadFields();
  return result;
#else
  Q_UNUSED( attributes );
  pushError( tr( "Renaming fields is not supported prior to GDAL 1.9.0" ) );
  return false;
#endif
}


bool QgsOgrProvider::changeAttributeValues( const QgsChangedAttributesMap &attr_map )
{
  if ( !doInitialActionsForEdition() )
    return false;

  if ( attr_map.isEmpty() )
    return true;

  clearMinMaxCache();

  setRelevantFields( ogrLayer, true, attributeIndexes() );

  for ( QgsChangedAttributesMap::const_iterator it = attr_map.begin(); it != attr_map.end(); ++it )
  {
    QgsFeatureId fid = it.key();

    if ( FID_TO_NUMBER( fid ) > std::numeric_limits<long>::max() )
    {
      pushError( tr( "OGR error on feature %1: id too large" ).arg( fid ) );
      continue;
    }

    const QgsAttributeMap &attr = it.value();
    if ( attr.isEmpty() )
      continue;

    OGRFeatureH of = OGR_L_GetFeature( ogrLayer, static_cast<long>( FID_TO_NUMBER( fid ) ) );
    if ( !of )
    {
      pushError( tr( "Feature %1 for attribute update not found." ).arg( fid ) );
      continue;
    }

    QgsLocaleNumC l;

    for ( QgsAttributeMap::const_iterator it2 = attr.begin(); it2 != attr.end(); ++it2 )
    {
      int f = it2.key();
      if ( mFirstFieldIsFid )
      {
        if ( f == 0 )
        {
          if ( it2->toLongLong() != fid )
          {
            pushError( tr( "Changing feature id of feature %1 is not allowed." ).arg( fid ) );
            continue;
          }
        }
        else
        {
          --f;
        }
      }

      OGRFieldDefnH fd = OGR_F_GetFieldDefnRef( of, f );
      if ( !fd )
      {
        pushError( tr( "Field %1 of feature %2 doesn't exist." ).arg( f ).arg( fid ) );
        continue;
      }

      OGRFieldType type = OGR_Fld_GetType( fd );

      if ( it2->isNull() || ( type != OFTString && it2->toString().isEmpty() ) )
      {
        OGR_F_UnsetField( of, f );
      }
      else
      {

        switch ( type )
        {
          case OFTInteger:
            OGR_F_SetFieldInteger( of, f, it2->toInt() );
            break;
#if defined(GDAL_VERSION_NUM) && GDAL_VERSION_NUM >= 2000000
          case OFTInteger64:
            OGR_F_SetFieldInteger64( of, f, it2->toLongLong() );
            break;
#endif
          case OFTReal:
            OGR_F_SetFieldDouble( of, f, it2->toDouble() );
            break;
          case OFTDate:
            OGR_F_SetFieldDateTime( of, f,
                                    it2->toDate().year(),
                                    it2->toDate().month(),
                                    it2->toDate().day(),
                                    0, 0, 0,
                                    0 );
            break;
          case OFTTime:
            OGR_F_SetFieldDateTime( of, f,
                                    0, 0, 0,
                                    it2->toTime().hour(),
                                    it2->toTime().minute(),
                                    it2->toTime().second(),
                                    0 );
            break;
          case OFTDateTime:
            OGR_F_SetFieldDateTime( of, f,
                                    it2->toDateTime().date().year(),
                                    it2->toDateTime().date().month(),
                                    it2->toDateTime().date().day(),
                                    it2->toDateTime().time().hour(),
                                    it2->toDateTime().time().minute(),
                                    it2->toDateTime().time().second(),
                                    0 );
            break;
          case OFTString:
            OGR_F_SetFieldString( of, f, mEncoding->fromUnicode( it2->toString() ).constData() );
            break;
          default:
            pushError( tr( "Type %1 of attribute %2 of feature %3 unknown." ).arg( type ).arg( fid ).arg( f ) );
            break;
        }
      }
    }

    if ( OGR_L_SetFeature( ogrLayer, of ) != OGRERR_NONE )
    {
      pushError( tr( "OGR error setting feature %1: %2" ).arg( fid ).arg( CPLGetLastErrorMsg() ) );
    }

    OGR_F_Destroy( of );
  }

  if ( OGR_L_SyncToDisk( ogrLayer ) != OGRERR_NONE )
  {
    pushError( tr( "OGR error syncing to disk: %1" ).arg( CPLGetLastErrorMsg() ) );
  }
  QgsOgrConnPool::instance()->invalidateConnections( dataSourceUri() );
  return true;
}

bool QgsOgrProvider::changeGeometryValues( const QgsGeometryMap &geometry_map )
{
  if ( !doInitialActionsForEdition() )
    return false;

  setRelevantFields( ogrLayer, true, attributeIndexes() );

  for ( QgsGeometryMap::const_iterator it = geometry_map.constBegin(); it != geometry_map.constEnd(); ++it )
  {
    if ( FID_TO_NUMBER( it.key() ) > std::numeric_limits<long>::max() )
    {
      pushError( tr( "OGR error on feature %1: id too large" ).arg( it.key() ) );
      continue;
    }

    OGRFeatureH theOGRFeature = OGR_L_GetFeature( ogrLayer, static_cast<long>( FID_TO_NUMBER( it.key() ) ) );
    if ( !theOGRFeature )
    {
      pushError( tr( "OGR error changing geometry: feature %1 not found" ).arg( it.key() ) );
      continue;
    }

    OGRGeometryH theNewGeometry = nullptr;
    // We might receive null geometries. It is ok, but don't go through the
    // OGR_G_CreateFromWkb() route then
    if ( it->wkbSize() != 0 )
    {
      //create an OGRGeometry
      if ( OGR_G_CreateFromWkb( const_cast<unsigned char*>( it->asWkb() ),
                                OGR_L_GetSpatialRef( ogrLayer ),
                                &theNewGeometry,
                                it->wkbSize() ) != OGRERR_NONE )
      {
        pushError( tr( "OGR error creating geometry for feature %1: %2" ).arg( it.key() ).arg( CPLGetLastErrorMsg() ) );
        OGR_G_DestroyGeometry( theNewGeometry );
        theNewGeometry = nullptr;
        OGR_F_Destroy( theOGRFeature );
        continue;
      }

      if ( !theNewGeometry )
      {
        pushError( tr( "OGR error in feature %1: geometry is null" ).arg( it.key() ) );
        OGR_F_Destroy( theOGRFeature );
        continue;
      }

      theNewGeometry = ConvertGeometryIfNecessary( theNewGeometry );
    }

    //set the new geometry
    if ( OGR_F_SetGeometryDirectly( theOGRFeature, theNewGeometry ) != OGRERR_NONE )
    {
      pushError( tr( "OGR error setting geometry of feature %1: %2" ).arg( it.key() ).arg( CPLGetLastErrorMsg() ) );
      // Shouldn't happen normally. If it happens, ownership of the geometry
      // may be not really well defined, so better not destroy it, but just
      // the feature.
      OGR_F_Destroy( theOGRFeature );
      continue;
    }


    if ( OGR_L_SetFeature( ogrLayer, theOGRFeature ) != OGRERR_NONE )
    {
      pushError( tr( "OGR error setting feature %1: %2" ).arg( it.key() ).arg( CPLGetLastErrorMsg() ) );
      OGR_F_Destroy( theOGRFeature );
      continue;
    }
    mShapefileMayBeCorrupted = true;

    OGR_F_Destroy( theOGRFeature );
  }
  QgsOgrConnPool::instance()->invalidateConnections( dataSourceUri() );
  return syncToDisc();
}

bool QgsOgrProvider::createSpatialIndex()
{
  if ( !doInitialActionsForEdition() )
    return false;

  if ( ogrDriverName != "ESRI Shapefile" )
    return false;

  QByteArray layerName = OGR_FD_GetName( OGR_L_GetLayerDefn( ogrOrigLayer ) );

  if ( ogrDataSource )
  {
    QByteArray sql = "CREATE SPATIAL INDEX ON " + quotedIdentifier( layerName );  // quote the layer name so spaces are handled
    QgsDebugMsg( QString( "SQL: %1" ).arg( FROM8( sql ) ) );
    OGR_DS_ExecuteSQL( ogrDataSource, sql.constData(), OGR_L_GetSpatialFilter( ogrOrigLayer ), nullptr );
  }

  QFileInfo fi( mFilePath );     // to get the base name
  //find out, if the .qix file is there
  QFile indexfile( fi.path().append( '/' ).append( fi.completeBaseName() ).append( ".qix" ) );
  return indexfile.exists();
}

bool QgsOgrProvider::createAttributeIndex( int field )
{
  if ( !doInitialActionsForEdition() )
    return false;

  QByteArray quotedLayerName = quotedIdentifier( OGR_FD_GetName( OGR_L_GetLayerDefn( ogrOrigLayer ) ) );
  QByteArray dropSql = "DROP INDEX ON " + quotedLayerName;
  OGR_DS_ExecuteSQL( ogrDataSource, dropSql.constData(), OGR_L_GetSpatialFilter( ogrOrigLayer ), nullptr );
  QByteArray createSql = "CREATE INDEX ON " + quotedLayerName + " USING " + mEncoding->fromUnicode( fields()[field].name() );
  OGR_DS_ExecuteSQL( ogrDataSource, createSql.constData(), OGR_L_GetSpatialFilter( ogrOrigLayer ), nullptr );

  QFileInfo fi( mFilePath );     // to get the base name
  //find out, if the .idm file is there
  QFile indexfile( fi.path().append( '/' ).append( fi.completeBaseName() ).append( ".idm" ) );
  return indexfile.exists();
}

bool QgsOgrProvider::deleteFeatures( const QgsFeatureIds & id )
{
  if ( !doInitialActionsForEdition() )
    return false;

  bool returnvalue = true;
  for ( QgsFeatureIds::const_iterator it = id.begin(); it != id.end(); ++it )
  {
    if ( !deleteFeature( *it ) )
    {
      returnvalue = false;
    }
  }

  if ( !syncToDisc() )
  {
    returnvalue = false;
  }

  recalculateFeatureCount();

  clearMinMaxCache();

  updateExtents();

  return returnvalue;
}

bool QgsOgrProvider::deleteFeature( QgsFeatureId id )
{
  if ( !doInitialActionsForEdition() )
    return false;

  if ( FID_TO_NUMBER( id ) > std::numeric_limits<long>::max() )
  {
    pushError( tr( "OGR error on feature %1: id too large" ).arg( id ) );
    return false;
  }

  if ( OGR_L_DeleteFeature( ogrLayer, FID_TO_NUMBER( id ) ) != OGRERR_NONE )
  {
    pushError( tr( "OGR error deleting feature %1: %2" ).arg( id ).arg( CPLGetLastErrorMsg() ) );
    return false;
  }

  mShapefileMayBeCorrupted = true;

  return true;
}

bool QgsOgrProvider::doInitialActionsForEdition()
{
  if ( !mValid )
    return false;

  if ( !mWriteAccess && mWriteAccessPossible && mDynamicWriteAccess )
  {
    QgsDebugMsg( "Enter update mode implictly" );
    if ( !enterUpdateMode() )
      return false;
  }

  return true;
}

int QgsOgrProvider::capabilities() const
{
  return mCapabilities;
}

void QgsOgrProvider::computeCapabilities()
{
  int ability = 0;

  // collect abilities reported by OGR
  if ( ogrLayer )
  {
    // Whilst the OGR documentation (e.g. at
    // http://www.gdal.org/ogr/classOGRLayer.html#a17) states "The capability
    // codes that can be tested are represented as strings, but #defined
    // constants exists to ensure correct spelling", we always use strings
    // here.  This is because older versions of OGR don't always have all
    // the #defines we want to test for here.

    if ( OGR_L_TestCapability( ogrLayer, "RandomRead" ) )
      // true if the GetFeature() method works *efficiently* for this layer.
      // TODO: Perhaps influence if QGIS caches into memory
      //       (vs read from disk every time) based on this setting.
    {
      // the latter flag is here just for compatibility
      ability |= QgsVectorDataProvider::SelectAtId | QgsVectorDataProvider::SelectGeometryAtId;
    }

    if ( mWriteAccessPossible && OGR_L_TestCapability( ogrLayer, "SequentialWrite" ) )
      // true if the CreateFeature() method works for this layer.
    {
      ability |= QgsVectorDataProvider::AddFeatures;
    }

    if ( mWriteAccessPossible && OGR_L_TestCapability( ogrLayer, "DeleteFeature" ) )
      // true if this layer can delete its features
    {
      ability |= DeleteFeatures;
    }

    if ( mWriteAccessPossible && OGR_L_TestCapability( ogrLayer, "RandomWrite" ) )
      // true if the SetFeature() method is operational on this layer.
    {
      // TODO According to http://shapelib.maptools.org/ (Shapefile C Library V1.2)
      // TODO "You can't modify the vertices of existing structures".
      // TODO Need to work out versions of shapelib vs versions of GDAL/OGR
      // TODO And test appropriately.

      ability |= ChangeAttributeValues;
      ability |= ChangeGeometries;
    }

#if 0
    if ( OGR_L_TestCapability( ogrLayer, "FastSpatialFilter" ) )
      // true if this layer implements spatial filtering efficiently.
      // Layers that effectively read all features, and test them with the
      // OGRFeature intersection methods should return false.
      // This can be used as a clue by the application whether it should build
      // and maintain it's own spatial index for features in this layer.
    {
      // TODO: Perhaps use as a clue by QGIS whether it should build and maintain it's own spatial index for features in this layer.
    }

    if ( OGR_L_TestCapability( ogrLayer, "FastFeatureCount" ) )
      // true if this layer can return a feature count
      // (via OGRLayer::GetFeatureCount()) efficiently ... ie. without counting
      // the features. In some cases this will return true until a spatial
      // filter is installed after which it will return false.
    {
      // TODO: Perhaps use as a clue by QGIS whether it should spawn a thread to count features.
    }

    if ( OGR_L_TestCapability( ogrLayer, "FastGetExtent" ) )
      // true if this layer can return its data extent
      // (via OGRLayer::GetExtent()) efficiently ... ie. without scanning
      // all the features. In some cases this will return true until a
      // spatial filter is installed after which it will return false.
    {
      // TODO: Perhaps use as a clue by QGIS whether it should spawn a thread to calculate extent.
    }

    if ( OGR_L_TestCapability( ogrLayer, "FastSetNextByIndex" ) )
      // true if this layer can perform the SetNextByIndex() call efficiently.
    {
      // No use required for this QGIS release.
    }
#endif

    if ( mWriteAccessPossible && OGR_L_TestCapability( ogrLayer, "CreateField" ) )
    {
      ability |= AddAttributes;
    }

    if ( mWriteAccessPossible && OGR_L_TestCapability( ogrLayer, "DeleteField" ) )
    {
      ability |= DeleteAttributes;
    }

    if ( mWriteAccessPossible && OGR_L_TestCapability( ogrLayer, "AlterFieldDefn" ) )
    {
      ability |= RenameAttributes;
    }

#if defined(OLCStringsAsUTF8)
    if ( !OGR_L_TestCapability( ogrLayer, OLCStringsAsUTF8 ) )
    {
      ability |= SelectEncoding;
    }
#else
    ability |= SelectEncoding;
#endif

    // OGR doesn't handle shapefiles without attributes, ie. missing DBFs well, fixes #803
    if ( ogrDriverName == "ESRI Shapefile" )
    {
      ability |= CreateSpatialIndex;
      ability |= CreateAttributeIndex;

      if ( mAttributeFields.size() == 0 )
      {
        QgsMessageLog::logMessage( tr( "Shapefiles without attribute are considered read-only." ), tr( "OGR" ) );
        ability &= ~( AddFeatures | DeleteFeatures | ChangeAttributeValues | AddAttributes | DeleteAttributes );
      }

      if (( ability & ChangeAttributeValues ) == 0 )
      {
        // on readonly shapes OGR reports that it can delete features although it can't RandomWrite
        ability &= ~( AddAttributes | DeleteFeatures );
      }
    }

    /* Curve geometries are available in some drivers starting with GDAL 2.0 */
    if ( OGR_L_TestCapability( ogrLayer, "CurveGeometries" ) )
    {
      ability |= CircularGeometries;
    }
  }

  mCapabilities = ability;
}


QString QgsOgrProvider::name() const
{
  return TEXT_PROVIDER_KEY;
} // QgsOgrProvider::name()


QString  QgsOgrProvider::description() const
{
  return TEXT_PROVIDER_DESCRIPTION;
} //  QgsOgrProvider::description()


/**

  Convenience function for readily creating file filters.

  Given a long name for a file filter and a regular expression, return
  a file filter string suitable for use in a QFileDialog::OpenFiles()
  call.  The regular express, glob, will have both all lower and upper
  case versions added.

  @note

  Copied from qgisapp.cpp.

  @todo XXX This should probably be generalized and moved to a standard
            utility type thingy.

*/
static QString createFileFilter_( QString const &longName, QString const &glob )
{
  // return longName + " [OGR] (" + glob.toLower() + ' ' + glob.toUpper() + ");;";
  return longName + " (" + glob.toLower() + ' ' + glob.toUpper() + ");;";
} // createFileFilter_


QString createFilters( QString type )
{
  /** Database drivers available*/
  static QString myDatabaseDrivers;
  /** Protocol drivers available*/
  static QString myProtocolDrivers;
  /** File filters*/
  static QString myFileFilters;
  /** Directory drivers*/
  static QString myDirectoryDrivers;
  /** Extensions*/
  static QStringList myExtensions;
  /** Wildcards*/
  static QStringList myWildcards;

  // if we've already built the supported vector string, just return what
  // we've already built

  if ( myFileFilters.isEmpty() || myFileFilters.isNull() )
  {
    // register ogr plugins
    QgsApplication::registerOgrDrivers();

    // first get the GDAL driver manager
    OGRSFDriverH driver;          // current driver
    QString driverName;           // current driver name

    // Grind through all the drivers and their respective metadata.
    // We'll add a file filter for those drivers that have a file
    // extension defined for them; the others, welll, even though
    // theoreticaly we can open those files because there exists a
    // driver for them, the user will have to use the "All Files" to
    // open datasets with no explicitly defined file name extension.
    QgsDebugMsg( QString( "Driver count: %1" ).arg( OGRGetDriverCount() ) );

    for ( int i = 0; i < OGRGetDriverCount(); ++i )
    {
      driver = OGRGetDriver( i );

      Q_CHECK_PTR( driver );

      if ( !driver )
      {
        QgsMessageLog::logMessage( QObject::tr( "Unable to get driver %1" ).arg( i ), QObject::tr( "OGR" ) );
        continue;
      }

      driverName = OGR_Dr_GetName( driver );

      if ( driverName.startsWith( "AVCBin" ) )
      {
        myDirectoryDrivers += QObject::tr( "Arc/Info Binary Coverage" ) + ",AVCBin;";
      }
      else if ( driverName.startsWith( "AVCE00" ) )
      {
        myFileFilters += createFileFilter_( QObject::tr( "Arc/Info ASCII Coverage" ), "*.e00" );
        myExtensions << "e00";
      }
      else if ( driverName.startsWith( "BNA" ) )
      {
        myFileFilters += createFileFilter_( QObject::tr( "Atlas BNA" ), "*.bna" );
        myExtensions << "bna";
      }
      else if ( driverName.startsWith( "CSV" ) )
      {
        myFileFilters += createFileFilter_( QObject::tr( "Comma Separated Value" ), "*.csv" );
        myExtensions << "csv";
      }
      else if ( driverName.startsWith( QObject::tr( "DODS" ) ) )
      {
        myProtocolDrivers += "DODS/OPeNDAP,DODS;";
      }
      else if ( driverName.startsWith( QObject::tr( "CouchDB" ) ) )
      {
        myProtocolDrivers += "CouchDB;";
      }
      else if ( driverName.startsWith( "FileGDB" ) )
      {
        myDirectoryDrivers += QObject::tr( "ESRI FileGDB" ) + ",FileGDB;";
      }
      else if ( driverName.startsWith( "PGeo" ) )
      {
        myDatabaseDrivers += QObject::tr( "ESRI Personal GeoDatabase" ) + ",PGeo;";
#ifdef Q_OS_WIN
        myFileFilters += createFileFilter_( QObject::tr( "ESRI Personal GeoDatabase" ), "*.mdb" );
        myExtensions << "mdb";
#endif
      }
      else if ( driverName.startsWith( "SDE" ) )
      {
        myDatabaseDrivers += QObject::tr( "ESRI ArcSDE" ) + ",SDE;";
      }
      else if ( driverName.startsWith( "ESRI" ) )
      {
        myFileFilters += createFileFilter_( QObject::tr( "ESRI Shapefiles" ), "*.shp" );
        myExtensions << "shp" << "dbf";
      }
      else if ( driverName.startsWith( QObject::tr( "FMEObjects Gateway" ) ) )
      {
        myFileFilters += createFileFilter_( QObject::tr( "FMEObjects Gateway" ), "*.fdd" );
        myExtensions << "fdd";
      }
      else if ( driverName.startsWith( "GeoJSON" ) )
      {
        myProtocolDrivers += "GeoJSON,GeoJSON;";
        myFileFilters += createFileFilter_( QObject::tr( "GeoJSON" ), "*.geojson" );
        myExtensions << "geojson";
      }
      else if ( driverName.startsWith( "GeoRSS" ) )
      {
        myFileFilters += createFileFilter_( QObject::tr( "GeoRSS" ), "*.xml" );
        myExtensions << "xml";
      }
      else if ( driverName.startsWith( "GML" ) )
      {
        myFileFilters += createFileFilter_( QObject::tr( "Geography Markup Language [GML]" ), "*.gml" );
        myExtensions << "gml";
      }
      else if ( driverName.startsWith( "GMT" ) )
      {
        myFileFilters += createFileFilter_( QObject::tr( "Generic Mapping Tools [GMT]" ), "*.gmt" );
        myExtensions << "gmt";
      }
      else if ( driverName.startsWith( "GPX" ) )
      {
        myFileFilters += createFileFilter_( QObject::tr( "GPS eXchange Format [GPX]" ), "*.gpx" );
        myExtensions << "gpx";
      }
      else if ( driverName.startsWith( "GPKG" ) )
      {
        myFileFilters += createFileFilter_( QObject::tr( "GeoPackage" ), "*.gpkg" );
        myExtensions << "gpkg";
      }
      else if ( driverName.startsWith( "GRASS" ) )
      {
        myDirectoryDrivers += QObject::tr( "Grass Vector" ) + ",GRASS;";
      }
      else if ( driverName.startsWith( "IDB" ) )
      {
        myDatabaseDrivers += QObject::tr( "Informix DataBlade" ) + ",IDB;";
      }
      else if ( driverName.startsWith( "Interlis 1" ) )
      {
        myFileFilters += createFileFilter_( QObject::tr( "INTERLIS 1" ), "*.itf *.xml *.ili" );
        myExtensions << "itf" << "xml" << "ili";
      }
      else if ( driverName.startsWith( "Interlis 2" ) )
      {
        myFileFilters += createFileFilter_( QObject::tr( "INTERLIS 2" ), "*.itf *.xml *.ili" );
        myExtensions << "itf" << "xml" << "ili";
      }
      else if ( driverName.startsWith( "Ingres" ) )
      {
        myDatabaseDrivers += QObject::tr( "Ingres" ) + ",Ingres;";
      }
      else if ( driverName.startsWith( "KML" ) )
      {
        myFileFilters += createFileFilter_( QObject::tr( "Keyhole Markup Language [KML]" ), "*.kml *.kmz" );
        myExtensions << "kml" << "kmz";
      }
      else if ( driverName.startsWith( "MapInfo File" ) )
      {
        myFileFilters += createFileFilter_( QObject::tr( "Mapinfo File" ), "*.mif *.tab" );
        myExtensions << "mif" << "tab";
      }
      else if ( driverName.startsWith( "DGN" ) )
      {
        myFileFilters += createFileFilter_( QObject::tr( "Microstation DGN" ), "*.dgn" );
        myExtensions << "dgn";
      }
      else if ( driverName.startsWith( "MySQL" ) )
      {
        myDatabaseDrivers += QObject::tr( "MySQL" ) + ",MySQL;";
      }
      else if ( driverName.startsWith( "MSSQL" ) )
      {
        myDatabaseDrivers += QObject::tr( "MSSQL" ) + ",MSSQL;";
      }
      else if ( driverName.startsWith( "OCI" ) )
      {
        myDatabaseDrivers += QObject::tr( "Oracle Spatial" ) + ",OCI;";
      }
      else if ( driverName.startsWith( "ODBC" ) )
      {
        myDatabaseDrivers += QObject::tr( "ODBC" ) + ",ODBC;";
      }
      else if ( driverName.startsWith( "OGDI" ) )
      {
        myDatabaseDrivers += QObject::tr( "OGDI Vectors" ) + ",OGDI;";
      }
      else if ( driverName.startsWith( "OpenFileGDB" ) )
      {
        myDirectoryDrivers += QObject::tr( "OpenFileGDB" ) + ",OpenFileGDB;";
      }
      else if ( driverName.startsWith( "PostgreSQL" ) )
      {
        myDatabaseDrivers += QObject::tr( "PostgreSQL" ) + ",PostgreSQL;";
      }
      else if ( driverName.startsWith( "S57" ) )
      {
        myFileFilters += createFileFilter_( QObject::tr( "S-57 Base file" ),
                                            "*.000" );
        myExtensions << "000";
      }
      else if ( driverName.startsWith( "SDTS" ) )
      {
        myFileFilters += createFileFilter_( QObject::tr( "Spatial Data Transfer Standard [SDTS]" ),
                                            "*catd.ddf" );
        myWildcards << "*catd.ddf";
      }
      else if ( driverName.startsWith( "SOSI" ) )
      {
        myFileFilters += createFileFilter_( QObject::tr( "Systematic Organization of Spatial Information [SOSI]" ), "*.sos" );
        myExtensions << "sos";
      }
      else if ( driverName.startsWith( "SQLite" ) )
      {
        myFileFilters += createFileFilter_( QObject::tr( "SQLite/SpatiaLite" ), "*.sqlite *.db *.sqlite3 *.db3 *.s3db *.sl3" );
        myExtensions << "sqlite" << "db" << "sqlite3" << "db3" << "s3db" << "sl3";
      }
      else if ( driverName.startsWith( "SXF" ) )
      {
        myFileFilters += createFileFilter_( QObject::tr( "Storage and eXchange Format" ), "*.sxf" );
        myExtensions << "sxf";
      }
      else if ( driverName.startsWith( "UK .NTF" ) )
      {
        myDirectoryDrivers += QObject::tr( "UK. NTF2" ) + ",UK. NTF;";
      }
      else if ( driverName.startsWith( "TIGER" ) )
      {
        myDirectoryDrivers += QObject::tr( "U.S. Census TIGER/Line" ) + ",TIGER;";
      }
      else if ( driverName.startsWith( "VRT" ) )
      {
        myFileFilters += createFileFilter_( QObject::tr( "VRT - Virtual Datasource" ),
                                            "*.vrt *.ovf" );
        myExtensions << "vrt" << "ovf";
      }
      else if ( driverName.startsWith( "XPlane" ) )
      {
        myFileFilters += createFileFilter_( QObject::tr( "X-Plane/Flightgear" ),
                                            "apt.dat nav.dat fix.dat awy.dat" );
        myWildcards << "apt.dat" << "nav.dat" << "fix.dat" << "awy.dat";
      }
      else if ( driverName.startsWith( "Geoconcept" ) )
      {
        myFileFilters += createFileFilter_( QObject::tr( "Geoconcept" ), "*.gxt *.txt" );
        myExtensions << "gxt" << "txt";
      }
      else if ( driverName.startsWith( "DXF" ) )
      {
        myFileFilters += createFileFilter_( QObject::tr( "AutoCAD DXF" ), "*.dxf" );
        myExtensions << "dxf";
      }
      else if ( driverName.startsWith( "ODS" ) )
      {
        myFileFilters += createFileFilter_( QObject::tr( "Open Document Spreadsheet" ), "*.ods" );
        myExtensions << "ods";
      }
      else if ( driverName.startsWith( "XLSX" ) )
      {
        myFileFilters += createFileFilter_( QObject::tr( "MS Office Open XML spreadsheet" ), "*.xlsx" );
        myExtensions << "xlsx";
      }
      else if ( driverName.endsWith( "XLS" ) )
      {
        myFileFilters += createFileFilter_( QObject::tr( "MS Excel format" ), "*.xls" );
        myExtensions << "xls";
      }
      else if ( driverName.startsWith( "EDIGEO" ) )
      {
        myFileFilters += createFileFilter_( QObject::tr( "EDIGEO" ), "*.thf" );
        myExtensions << "thf";
      }
      else if ( driverName.startsWith( "NAS" ) )
      {
        myFileFilters += createFileFilter_( QObject::tr( "NAS - ALKIS" ), "*.xml" );
        myExtensions << "xml";
      }
      else if ( driverName.startsWith( "WAsP" ) )
      {
        myFileFilters += createFileFilter_( QObject::tr( "WAsP" ), "*.map" );
        myExtensions << "map";
      }
      else if ( driverName.startsWith( "PCIDSK" ) )
      {
        myFileFilters += createFileFilter_( QObject::tr( "PCI Geomatics Database File" ), "*.pix" );
        myExtensions << "pix";
      }
      else if ( driverName.startsWith( "GPSTrackMaker" ) )
      {
        myFileFilters += createFileFilter_( QObject::tr( "GPSTrackMaker" ), "*.gtm *.gtz" );
        myExtensions << "gtm" << "gtz";
      }
      else if ( driverName.startsWith( "VFK" ) )
      {
        myFileFilters += createFileFilter_( QObject::tr( "Czech Cadastral Exchange Data Format" ), "*.vfk" );
        myExtensions << "vfk";
      }
      else if ( driverName.startsWith( "OSM" ) )
      {
        myFileFilters += createFileFilter_( QObject::tr( "OpenStreetMap" ), "*.osm *.pbf" );
        myExtensions << "osm" << "pbf";
      }
      else if ( driverName.startsWith( "SUA" ) )
      {
        myFileFilters += createFileFilter_( QObject::tr( "Special Use Airspace Format" ), "*.sua" );
        myExtensions << "sua";
      }
      else if ( driverName.startsWith( "OpenAir" ) )
      {
        myFileFilters += createFileFilter_( QObject::tr( "OpenAir Special Use Airspace Format" ), "*.txt" );
        myExtensions << "txt";
      }
      else if ( driverName.startsWith( "PDS" ) )
      {
        myFileFilters += createFileFilter_( QObject::tr( "Planetary Data Systems TABLE" ), "*.xml" );
        myExtensions << "xml";
      }
      else if ( driverName.startsWith( "HTF" ) )
      {
        myFileFilters += createFileFilter_( QObject::tr( "Hydrographic Transfer Format" ), "*.htf" );
        myExtensions << "htf";
      }
      else if ( driverName.startsWith( "SVG" ) )
      {
        myFileFilters += createFileFilter_( QObject::tr( "Scalable Vector Graphics" ), "*.svg" );
        myExtensions << "svg";
      }
      else if ( driverName.startsWith( "ARCGEN" ) )
      {
        myFileFilters += createFileFilter_( QObject::tr( "Arc/Info Generate" ), "*.gen" );
        myExtensions << "gen";
      }
      else if ( driverName.startsWith( "PDF" ) )
      {
        myFileFilters += createFileFilter_( QObject::tr( "Geospatial PDF" ), "*.pdf" );
        myExtensions << "pdf";
      }
      else if ( driverName.startsWith( "SEGY" ) )
      {
        myFileFilters += createFileFilter_( QObject::tr( "SEG-Y" ), "*.sgy *.segy" );
        myExtensions << "sgy" << "segy";
      }
      else if ( driverName.startsWith( "SEGUKOOA" ) )
      {
        myFileFilters += createFileFilter_( QObject::tr( "SEG-P1" ), "*.seg *.seg1 *.sp1" );
        myFileFilters += createFileFilter_( QObject::tr( "UKOOA P1/90" ), "*.uko *.ukooa" );
        myExtensions << "seg" << "seg1" << "sp1" << "uko" << "ukooa";
      }
      else
      {
        // NOP, we don't know anything about the current driver
        // with regards to a proper file filter string
        QgsDebugMsg( QString( "Unknown driver %1 for file filters." ).arg( driverName ) );
      }

    }                          // each loaded OGR driver

    // sort file filters alphabetically
    QgsDebugMsg( "myFileFilters: " + myFileFilters );
    QStringList filters = myFileFilters.split( ";;", QString::SkipEmptyParts );
    filters.sort();
    myFileFilters = filters.join( ";;" ) + ";;";
    QgsDebugMsg( "myFileFilters: " + myFileFilters );

    // VSIFileHandler (.zip and .gz files) - second
    //   see http://trac.osgeo.org/gdal/wiki/UserDocs/ReadInZip
    // Requires GDAL>=1.6.0 with libz support, let's assume we have it.
    // This does not work for some file types, see VSIFileHandler doc.
#if defined(GDAL_VERSION_NUM) && GDAL_VERSION_NUM >= 1600
    QSettings settings;
    if ( settings.value( "/qgis/scanZipInBrowser2", "basic" ).toString() != "no" )
    {
      myFileFilters.prepend( createFileFilter_( QObject::tr( "GDAL/OGR VSIFileHandler" ), "*.zip *.gz *.tar *.tar.gz *.tgz" ) );
      myExtensions << "zip" << "gz" << "tar" << "tar.gz" << "tgz";

    }
#endif

    // can't forget the default case - first
    myFileFilters.prepend( QObject::tr( "All files" ) + " (*);;" );

    // cleanup
    if ( myFileFilters.endsWith( ";;" ) ) myFileFilters.chop( 2 );

    QgsDebugMsg( "myFileFilters: " + myFileFilters );
  }

  if ( type == "file" )
  {
    return myFileFilters;
  }
  if ( type == "database" )
  {
    return myDatabaseDrivers;
  }
  if ( type == "protocol" )
  {
    return myProtocolDrivers;
  }
  if ( type == "directory" )
  {
    return myDirectoryDrivers;
  }
  if ( type == "extensions" )
  {
    return myExtensions.join( "|" );
  }
  if ( type == "wildcards" )
  {
    return myWildcards.join( "|" );
  }
  else
  {
    return "";
  }
}


QGISEXTERN QString fileVectorFilters()
{
  return createFilters( "file" );
}

QString QgsOgrProvider::fileVectorFilters() const
{
  return createFilters( "file" );
}

QGISEXTERN QString databaseDrivers()
{
  return createFilters( "database" );
}

QString QgsOgrProvider::databaseDrivers() const
{
  return createFilters( "database" );
}

QGISEXTERN QString protocolDrivers()
{
  return createFilters( "protocol" );
}

QString QgsOgrProvider::protocolDrivers() const
{
  return createFilters( "protocol" );
}

QGISEXTERN QString directoryDrivers()
{
  return  createFilters( "directory" );
}

QString QgsOgrProvider::directoryDrivers() const
{
  return  createFilters( "directory" );
}

QGISEXTERN QStringList fileExtensions()
{
  return  createFilters( "extensions" ).split( '|' );
}

QGISEXTERN QStringList wildcards()
{
  return  createFilters( "wildcards" ).split( '|' );
}


/**
 * Class factory to return a pointer to a newly created
 * QgsOgrProvider object
 */
QGISEXTERN QgsOgrProvider * classFactory( const QString *uri )
{
  return new QgsOgrProvider( *uri );
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

/** Creates an empty data source
@param uri location to store the file(s)
@param format data format (e.g. "ESRI Shapefile"
@param vectortype point/line/polygon or multitypes
@param attributes a list of name/type pairs for the initial attributes
@return true in case of success*/
QGISEXTERN bool createEmptyDataSource( const QString &uri,
                                       const QString &format,
                                       const QString &encoding,
                                       QGis::WkbType vectortype,
                                       const QList< QPair<QString, QString> > &attributes,
                                       const QgsCoordinateReferenceSystem *srs = nullptr )
{
  QgsDebugMsg( QString( "Creating empty vector layer with format: %1" ).arg( format ) );

  OGRSFDriverH driver;
  QgsApplication::registerOgrDrivers();
  driver = OGRGetDriverByName( format.toAscii() );
  if ( !driver )
  {
    return false;
  }

  QString driverName = OGR_Dr_GetName( driver );

  if ( driverName == "ESRI Shapefile" )
  {
    if ( !uri.endsWith( ".shp", Qt::CaseInsensitive ) )
    {
      QgsDebugMsg( QString( "uri %1 doesn't end with .shp" ).arg( uri ) );
      return false;
    }

    // check for duplicate fieldnames
    QSet<QString> fieldNames;
    QList<QPair<QString, QString> >::const_iterator fldIt;
    for ( fldIt = attributes.begin(); fldIt != attributes.end(); ++fldIt )
    {
      QString name = fldIt->first.left( 10 );
      if ( fieldNames.contains( name ) )
      {
        QgsMessageLog::logMessage( QObject::tr( "Duplicate field (10 significant characters): %1" ).arg( name ), QObject::tr( "OGR" ) );
        return false;
      }
      fieldNames << name;
    }

    QgsVectorFileWriter::deleteShapeFile( uri );
  }
  else
  {
    QFile::remove( uri );
  }

  OGRDataSourceH dataSource;
  dataSource = OGR_Dr_CreateDataSource( driver, TO8F( uri ), nullptr );
  if ( !dataSource )
  {
    QgsMessageLog::logMessage( QObject::tr( "Creating the data source %1 failed: %2" ).arg( uri, QString::fromUtf8( CPLGetLastErrorMsg() ) ), QObject::tr( "OGR" ) );
    return false;
  }

  //consider spatial reference system
  OGRSpatialReferenceH reference = nullptr;

  QgsCoordinateReferenceSystem mySpatialRefSys;
  if ( srs )
  {
    mySpatialRefSys = *srs;
  }
  else
  {
    mySpatialRefSys.validate();
  }

  QString myWkt = mySpatialRefSys.toWkt();

  if ( !myWkt.isNull()  &&  myWkt.length() != 0 )
  {
    reference = OSRNewSpatialReference( myWkt.toLocal8Bit().data() );
  }

  // Map the qgis geometry type to the OGR geometry type
  OGRwkbGeometryType OGRvectortype = wkbUnknown;
  switch ( vectortype )
  {
    case QGis::WKBPoint:
      OGRvectortype = wkbPoint;
      break;
    case QGis::WKBLineString:
      OGRvectortype = wkbLineString;
      break;
    case QGis::WKBPolygon:
      OGRvectortype = wkbPolygon;
      break;
    case QGis::WKBMultiPoint:
      OGRvectortype = wkbMultiPoint;
      break;
    case QGis::WKBMultiLineString:
      OGRvectortype = wkbMultiLineString;
      break;
    case QGis::WKBMultiPolygon:
      OGRvectortype = wkbMultiPolygon;
      break;
    default:
    {
      QgsMessageLog::logMessage( QObject::tr( "Unknown vector type of %1" ).arg(( int )( vectortype ) ), QObject::tr( "OGR" ) );
      return false;
    }
  }

  char **papszOptions = nullptr;
  if ( driverName == "ESRI Shapefile" )
  {
    papszOptions = CSLSetNameValue( papszOptions, "ENCODING", QgsVectorFileWriter::convertCodecNameForEncodingOption( encoding ).toLocal8Bit().data() );
    // OGR Shapefile fails to create fields if given encoding is not supported by its side
    // so disable encoding conversion of OGR Shapefile layer
    CPLSetConfigOption( "SHAPE_ENCODING", "" );
  }

  OGRLayerH layer;
  layer = OGR_DS_CreateLayer( dataSource, TO8F( QFileInfo( uri ).completeBaseName() ), reference, OGRvectortype, papszOptions );
  CSLDestroy( papszOptions );

  QSettings settings;
  if ( !settings.value( "/qgis/ignoreShapeEncoding", true ).toBool() )
  {
    CPLSetConfigOption( "SHAPE_ENCODING", nullptr );
  }

  if ( !layer )
  {
    QgsMessageLog::logMessage( QObject::tr( "Creation of OGR data source %1 failed: %2" ).arg( uri, QString::fromUtf8( CPLGetLastErrorMsg() ) ), QObject::tr( "OGR" ) );
    return false;
  }

  //create the attribute fields

  QTextCodec* codec = QTextCodec::codecForName( encoding.toLocal8Bit().data() );
  if ( !codec )
  {
    // fall back to "System" codec
    codec = QTextCodec::codecForLocale();
    Q_ASSERT( codec );
  }

  for ( QList<QPair<QString, QString> >::const_iterator it = attributes.begin(); it != attributes.end(); ++it )
  {
    QStringList fields = it->second.split( ';' );

    if ( fields.isEmpty() )
      continue;

    int width = fields.size() > 1 ? fields[1].toInt() : -1;
    int precision = fields.size() > 2 ? fields[2].toInt() : -1;
    if ( precision > 0 )
      width += 1;

    OGRFieldDefnH field;
    if ( fields[0] == "Real" )
    {
      if ( width < 0 )
        width = 32;
      if ( precision < 0 )
        precision = 3;

      field = OGR_Fld_Create( codec->fromUnicode( it->first ).constData(), OFTReal );
      OGR_Fld_SetWidth( field, width );
      OGR_Fld_SetPrecision( field, precision );
    }
    else if ( fields[0] == "Integer" )
    {
      if ( width < 0 || width > 10 )
        width = 10;

      field = OGR_Fld_Create( codec->fromUnicode( it->first ).constData(), OFTInteger );
      // limit to 10.  otherwise OGR sets it to 11 and recognizes as OFTDouble later
      OGR_Fld_SetWidth( field, width );
    }
    else if ( fields[0] == "String" )
    {
      if ( width < 0 || width > 255 )
        width = 255;

      field = OGR_Fld_Create( codec->fromUnicode( it->first ).constData(), OFTString );
      OGR_Fld_SetWidth( field, width );
    }
    else if ( fields[0] == "Date" )
    {
      field = OGR_Fld_Create( codec->fromUnicode( it->first ).constData(), OFTDate );
    }
    else if ( fields[0] == "Time" )
    {
      field = OGR_Fld_Create( codec->fromUnicode( it->first ).constData(), OFTTime );
    }
    else if ( fields[0] == "DateTime" )
    {
      field = OGR_Fld_Create( codec->fromUnicode( it->first ).constData(), OFTDateTime );
    }
    else
    {
      QgsMessageLog::logMessage( QObject::tr( "field %1 with unsupported type %2 skipped" ).arg( it->first, fields[0] ), QObject::tr( "OGR" ) );
      continue;
    }

    if ( OGR_L_CreateField( layer, field, true ) != OGRERR_NONE )
    {
      QgsMessageLog::logMessage( QObject::tr( "creation of field %1 failed" ).arg( it->first ), QObject::tr( "OGR" ) );
    }
  }

  OGR_DS_Destroy( dataSource );

  if ( driverName == "ESRI Shapefile" )
  {
    QString layerName = uri.left( uri.indexOf( ".shp", Qt::CaseInsensitive ) );
    QFile prjFile( layerName + ".qpj" );
    if ( prjFile.open( QIODevice::WriteOnly ) )
    {
      QTextStream prjStream( &prjFile );
      prjStream << myWkt.toLocal8Bit().data() << endl;
      prjFile.close();
    }
    else
    {
      QgsMessageLog::logMessage( QObject::tr( "Couldn't create file %1.qpj" ).arg( layerName ), QObject::tr( "OGR" ) );
    }
  }

  QgsDebugMsg( QString( "GDAL Version number %1" ).arg( GDAL_VERSION_NUM ) );
#if defined(GDAL_VERSION_NUM) && GDAL_VERSION_NUM >= 1310
  if ( reference )
  {
    OSRRelease( reference );
  }
#endif //GDAL_VERSION_NUM
  return true;
}

QgsCoordinateReferenceSystem QgsOgrProvider::crs()
{
  QgsDebugMsg( "Entering." );

  QgsCoordinateReferenceSystem srs;
  if ( !mValid )
    return srs;

  if ( ogrDriver )
  {
    QString driverName = OGR_Dr_GetName( ogrDriver );

    if ( driverName == "ESRI Shapefile" )
    {
      QString layerName = mFilePath.left( mFilePath.indexOf( ".shp", Qt::CaseInsensitive ) );
      QFile prjFile( layerName + ".qpj" );
      if ( prjFile.open( QIODevice::ReadOnly ) )
      {
        QTextStream prjStream( &prjFile );
        QString myWktString = prjStream.readLine();
        prjFile.close();

        srs = QgsCRSCache::instance()->crsByWkt( myWktString.toUtf8().constData() );
        if ( srs.isValid() )
          return srs;
      }
    }
  }

  // add towgs84 parameter
  QgsCoordinateReferenceSystem::setupESRIWktFix();

  OGRSpatialReferenceH mySpatialRefSys = OGR_L_GetSpatialRef( ogrLayer );
  if ( mySpatialRefSys )
  {
    // get the proj4 text
    char *pszProj4;
    OSRExportToProj4( mySpatialRefSys, &pszProj4 );
    QgsDebugMsg( pszProj4 );
    OGRFree( pszProj4 );

    char *pszWkt = nullptr;
    OSRExportToWkt( mySpatialRefSys, &pszWkt );

    srs = QgsCRSCache::instance()->crsByWkt( pszWkt );
    OGRFree( pszWkt );
  }
  else
  {
    QgsDebugMsg( "no spatial reference found" );
  }

  return srs;
}

void QgsOgrProvider::uniqueValues( int index, QList<QVariant> &uniqueValues, int limit )
{
  uniqueValues.clear();

  if ( !mValid || index < 0 || index >= mAttributeFields.count() )
    return;

  const QgsField& fld = mAttributeFields.at( index );
  if ( fld.name().isNull() )
  {
    return; //not a provider field
  }

#if defined(GDAL_VERSION_NUM) && GDAL_VERSION_NUM < 1910
  // avoid GDAL #4509
  return QgsVectorDataProvider::uniqueValues( index, uniqueValues, limit );
#else
  QByteArray sql = "SELECT DISTINCT " + quotedIdentifier( mEncoding->fromUnicode( fld.name() ) );
  sql += " FROM " + quotedIdentifier( OGR_FD_GetName( OGR_L_GetLayerDefn( ogrLayer ) ) );

  if ( !mSubsetString.isEmpty() )
  {
    sql += " WHERE " + mEncoding->fromUnicode( mSubsetString );
  }

  sql += " ORDER BY " + mEncoding->fromUnicode( fld.name() ) + " ASC";  // quoting of fieldname produces a syntax error

  QgsDebugMsg( QString( "SQL: %1" ).arg( mEncoding->toUnicode( sql ) ) );
  OGRLayerH l = OGR_DS_ExecuteSQL( ogrDataSource, sql.constData(), nullptr, nullptr );
  if ( !l )
  {
    QgsDebugMsg( "Failed to execute SQL" );
    return QgsVectorDataProvider::uniqueValues( index, uniqueValues, limit );
  }

  OGRFeatureH f;
  while (( f = OGR_L_GetNextFeature( l ) ) )
  {
    uniqueValues << ( OGR_F_IsFieldSet( f, 0 ) ? convertValue( fld.type(), mEncoding->toUnicode( OGR_F_GetFieldAsString( f, 0 ) ) ) : QVariant( fld.type() ) );
    OGR_F_Destroy( f );

    if ( limit >= 0 && uniqueValues.size() >= limit )
      break;
  }

  OGR_DS_ReleaseResultSet( ogrDataSource, l );
#endif
}

QVariant QgsOgrProvider::minimumValue( int index )
{
  if ( !mValid || index < 0 || index >= mAttributeFields.count() )
  {
    return QVariant();
  }
  const QgsField& fld = mAttributeFields.at( index );

  // Don't quote column name (see https://trac.osgeo.org/gdal/ticket/5799#comment:9)
  QByteArray sql = "SELECT MIN(" + mEncoding->fromUnicode( fld.name() );
  sql += ") FROM " + quotedIdentifier( OGR_FD_GetName( OGR_L_GetLayerDefn( ogrLayer ) ) );

  if ( !mSubsetString.isEmpty() )
  {
    sql += " WHERE " + mEncoding->fromUnicode( mSubsetString );
  }

  OGRLayerH l = OGR_DS_ExecuteSQL( ogrDataSource, sql.constData(), nullptr, nullptr );
  if ( !l )
  {
    QgsDebugMsg( QString( "Failed to execute SQL: %1" ).arg( mEncoding->toUnicode( sql ) ) );
    return QgsVectorDataProvider::minimumValue( index );
  }

  OGRFeatureH f = OGR_L_GetNextFeature( l );
  if ( !f )
  {
    OGR_DS_ReleaseResultSet( ogrDataSource, l );
    return QVariant();
  }

  QVariant value = OGR_F_IsFieldSet( f, 0 ) ? convertValue( fld.type(), mEncoding->toUnicode( OGR_F_GetFieldAsString( f, 0 ) ) ) : QVariant( fld.type() );
  OGR_F_Destroy( f );

  OGR_DS_ReleaseResultSet( ogrDataSource, l );

  return value;
}

QVariant QgsOgrProvider::maximumValue( int index )
{
  if ( !mValid || index < 0 || index >= mAttributeFields.count() )
  {
    return QVariant();
  }
  const QgsField& fld = mAttributeFields.at( index );

  // Don't quote column name (see https://trac.osgeo.org/gdal/ticket/5799#comment:9)
  QByteArray sql = "SELECT MAX(" + mEncoding->fromUnicode( fld.name() );
  sql += ") FROM " + quotedIdentifier( OGR_FD_GetName( OGR_L_GetLayerDefn( ogrLayer ) ) );

  if ( !mSubsetString.isEmpty() )
  {
    sql += " WHERE " + mEncoding->fromUnicode( mSubsetString );
  }

  OGRLayerH l = OGR_DS_ExecuteSQL( ogrDataSource, sql.constData(), nullptr, nullptr );
  if ( !l )
  {
    QgsDebugMsg( QString( "Failed to execute SQL: %1" ).arg( mEncoding->toUnicode( sql ) ) );
    return QgsVectorDataProvider::maximumValue( index );
  }

  OGRFeatureH f = OGR_L_GetNextFeature( l );
  if ( !f )
  {
    OGR_DS_ReleaseResultSet( ogrDataSource, l );
    return QVariant();
  }

  QVariant value = OGR_F_IsFieldSet( f, 0 ) ? convertValue( fld.type(), mEncoding->toUnicode( OGR_F_GetFieldAsString( f, 0 ) ) ) : QVariant( fld.type() );
  OGR_F_Destroy( f );

  OGR_DS_ReleaseResultSet( ogrDataSource, l );

  return value;
}

QByteArray QgsOgrProvider::quotedIdentifier( QByteArray field ) const
{
  return QgsOgrProviderUtils::quotedIdentifier( field, ogrDriverName );
}

void QgsOgrProvider::forceReload()
{
  QgsOgrConnPool::instance()->invalidateConnections( dataSourceUri() );
}

OGRDataSourceH QgsOgrProviderUtils::OGROpenWrapper( const char* pszPath, bool bUpdate, OGRSFDriverH *phDriver )
{
  CPLErrorReset();
  OGRSFDriverH hDriver = nullptr;
  OGRDataSourceH hDS = OGROpen( pszPath, bUpdate, &hDriver );
  if ( phDriver )
    *phDriver = hDriver;
  if ( !hDS )
    return nullptr;
  // GDAL < 1.11.5 has a crashing bug with GeoPackage databases with curve geometry
  // types (https://trac.osgeo.org/gdal/ticket/6558)
#if GDAL_VERSION_MAJOR == 1 && GDAL_VERSION_MINOR == 11 && GDAL_VERSION_MACRO < 5
  const char* pszLastErrorMsg = CPLGetLastErrorMsg();
  if ( hDriver == OGRGetDriverByName( "GPKG" ) &&
       strstr( pszLastErrorMsg, "geometry column" ) &&
       strstr( pszLastErrorMsg, "of type" ) &&
       strstr( pszLastErrorMsg, "ignored" ) )
  {
    QgsDebugMsg( QString( "Ignoring %1 that is a GeoPackage DB with curve geometries" ).arg( pszPath ) );
    OGR_DS_Destroy( hDS );
    hDS = nullptr;
  }
#endif
  return hDS;
}


QByteArray QgsOgrProviderUtils::quotedIdentifier( QByteArray field, const QString& ogrDriverName )
{
  if ( ogrDriverName == "MySQL" )
  {
    field.replace( '\\', "\\\\" );
    field.replace( '`', "``" );
    return field.prepend( '`' ).append( '`' );
  }
  else
  {
    field.replace( '\\', "\\\\" );
    field.replace( '"', "\\\"" );
    field.replace( '\'', "\\'" );
    return field.prepend( '\"' ).append( '\"' );
  }
}

QString QgsOgrProviderUtils::quotedValue( const QVariant& value )
{
  if ( value.isNull() )
    return "NULL";

  switch ( value.type() )
  {
    case QVariant::Int:
    case QVariant::LongLong:
    case QVariant::Double:
      return value.toString();

    case QVariant::Bool:
      //OGR does not support boolean literals
      return value.toBool() ? "1" : "0";

    default:
    case QVariant::String:
      QString v = value.toString();
      v.replace( '\'', "''" );
      if ( v.contains( '\\' ) )
        return v.replace( '\\', "\\\\" ).prepend( "E'" ).append( '\'' );
      else
        return v.prepend( '\'' ).append( '\'' );
  }
}

bool QgsOgrProvider::syncToDisc()
{
  //for shapefiles, remove spatial index files and create a new index
  QgsOgrConnPool::instance()->unref( dataSourceUri() );
  bool shapeIndex = false;
  if ( ogrDriverName == "ESRI Shapefile" )
  {
    QString sbnIndexFile;
    QFileInfo fi( mFilePath );
    int suffixLength = fi.suffix().length();
    sbnIndexFile = mFilePath;
    sbnIndexFile.chop( suffixLength );
    sbnIndexFile.append( "sbn" );

    if ( QFile::exists( sbnIndexFile ) )
    {
      shapeIndex = true;
      close();
      QgsOgrConnPool::instance()->invalidateConnections( dataSourceUri() );
      QFile::remove( sbnIndexFile );
      open( OpenModeSameAsCurrent );
      if ( !mValid )
        return false;
    }
  }

  if ( OGR_L_SyncToDisk( ogrLayer ) != OGRERR_NONE )
  {
    pushError( tr( "OGR error syncing to disk: %1" ).arg( CPLGetLastErrorMsg() ) );
  }

  if ( mShapefileMayBeCorrupted )
    repack();

  mShapefileMayBeCorrupted = false;

  QgsOgrConnPool::instance()->ref( dataSourceUri() );
  if ( shapeIndex )
  {
    return createSpatialIndex();
  }

  return true;
}

void QgsOgrProvider::recalculateFeatureCount()
{
  if ( !ogrLayer )
  {
    mFeaturesCounted = 0;
    return;
  }

  OGRGeometryH filter = OGR_L_GetSpatialFilter( ogrLayer );
  if ( filter )
  {
    filter = OGR_G_Clone( filter );
    OGR_L_SetSpatialFilter( ogrLayer, nullptr );
  }

  // feature count returns number of features within current spatial filter
  // so we remove it if there's any and then put it back
  if ( mOgrGeometryTypeFilter == wkbUnknown )
  {
    mFeaturesCounted = OGR_L_GetFeatureCount( ogrLayer, true );
  }
  else
  {
    mFeaturesCounted = 0;
    OGR_L_ResetReading( ogrLayer );
    setRelevantFields( ogrLayer, true, QgsAttributeList() );
    OGR_L_ResetReading( ogrLayer );
    OGRFeatureH fet;
    while (( fet = OGR_L_GetNextFeature( ogrLayer ) ) )
    {
      OGRGeometryH geom = OGR_F_GetGeometryRef( fet );
      if ( geom )
      {
        OGRwkbGeometryType gType = OGR_G_GetGeometryType( geom );
        if ( gType == mOgrGeometryTypeFilter ) mFeaturesCounted++;
      }
      OGR_F_Destroy( fet );
    }
    OGR_L_ResetReading( ogrLayer );

  }

  if ( filter )
  {
    OGR_L_SetSpatialFilter( ogrLayer, filter );
  }

  QgsOgrConnPool::instance()->invalidateConnections( dataSourceUri() );
}

bool QgsOgrProvider::doesStrictFeatureTypeCheck() const
{
  // FIXME probably other drivers too...
  return ogrDriverName != "ESRI Shapefile" || ( mOGRGeomType == wkbPoint || mOGRGeomType == wkbPoint25D );
}

OGRwkbGeometryType QgsOgrProvider::ogrWkbSingleFlatten( OGRwkbGeometryType type )
{
  type = wkbFlatten( type );
  switch ( type )
  {
    case wkbMultiPoint:
      return wkbPoint;
    case wkbMultiLineString:
      return wkbLineString;
    case wkbMultiPolygon:
      return wkbPolygon;
#if defined(GDAL_COMPUTE_VERSION) && GDAL_VERSION_NUM >= GDAL_COMPUTE_VERSION(2,0,0)
    case wkbMultiCurve:
      return wkbCompoundCurve;
    case wkbMultiSurface:
      return wkbCurvePolygon;
#endif
    default:
      return type;
  }
}

OGRLayerH QgsOgrProvider::setSubsetString( OGRLayerH layer, OGRDataSourceH ds )
{
  return QgsOgrProviderUtils::setSubsetString( layer, ds, mEncoding, mSubsetString );
}

OGRLayerH QgsOgrProviderUtils::setSubsetString( OGRLayerH layer, OGRDataSourceH ds, QTextCodec* encoding, const QString& subsetString )
{
  QByteArray layerName = OGR_FD_GetName( OGR_L_GetLayerDefn( layer ) );
  OGRSFDriverH ogrDriver = OGR_DS_GetDriver( ds );
  QString ogrDriverName = OGR_Dr_GetName( ogrDriver );

  if ( ogrDriverName == "ODBC" ) //the odbc driver does not like schema names for subset
  {
    QString layerNameString = encoding->toUnicode( layerName );
    int dotIndex = layerNameString.indexOf( '.' );
    if ( dotIndex > 1 )
    {
      QString modifiedLayerName = layerNameString.right( layerNameString.size() - dotIndex - 1 );
      layerName = encoding->fromUnicode( modifiedLayerName );
    }
  }
  QByteArray sql = "SELECT * FROM " + quotedIdentifier( layerName, ogrDriverName );
  sql += " WHERE " + encoding->fromUnicode( subsetString );

  QgsDebugMsg( QString( "SQL: %1" ).arg( encoding->toUnicode( sql ) ) );
  return OGR_DS_ExecuteSQL( ds, sql.constData(), nullptr, nullptr );
}

void QgsOgrProvider::open( OpenMode mode )
{
  bool openReadOnly = false;

  // Try to open using VSIFileHandler
  //   see http://trac.osgeo.org/gdal/wiki/UserDocs/ReadInZip
  QString vsiPrefix = QgsZipItem::vsiPrefix( dataSourceUri() );
  if ( vsiPrefix != "" )
  {
    // GDAL>=1.8.0 has write support for zip, but read and write operations
    // cannot be interleaved, so for now just use read-only.
    openReadOnly = true;
    if ( !mFilePath.startsWith( vsiPrefix ) )
    {
      mFilePath = vsiPrefix + mFilePath;
      setDataSourceUri( mFilePath );
    }
    QgsDebugMsg( QString( "Trying %1 syntax, mFilePath= %2" ).arg( vsiPrefix, mFilePath ) );
  }

  QgsDebugMsg( "mFilePath: " + mFilePath );
  QgsDebugMsg( "mLayerIndex: " + QString::number( mLayerIndex ) );
  QgsDebugMsg( "mLayerName: " + mLayerName );
  QgsDebugMsg( "mSubsetString: " + mSubsetString );
  CPLSetConfigOption( "OGR_ORGANIZE_POLYGONS", "ONLY_CCW" );  // "SKIP" returns MULTIPOLYGONs for multiringed POLYGONs
  CPLSetConfigOption( "GPX_ELE_AS_25D", "YES" );  // use GPX elevation as z values

  if ( mFilePath.startsWith( "MySQL:" ) && !mLayerName.isEmpty() && !mFilePath.endsWith( ",tables=" + mLayerName ) )
  {
    mFilePath += ",tables=" + mLayerName;
  }

  if ( mode == OpenModeForceReadOnly )
    openReadOnly = true;
  else if ( mode == OpenModeSameAsCurrent && !mWriteAccess )
    openReadOnly = true;

  // first try to open in update mode (unless specified otherwise)
  if ( !openReadOnly )
    ogrDataSource = QgsOgrProviderUtils::OGROpenWrapper( TO8F( mFilePath ), true, &ogrDriver );

  mValid = false;
  if ( ogrDataSource )
  {
    mWriteAccess = true;
    mWriteAccessPossible = true;
  }
  else
  {
    mWriteAccess = false;
    if ( !openReadOnly )
    {
      QgsDebugMsg( "OGR failed to opened in update mode, trying in read-only mode" );
    }

    // try to open read-only
    ogrDataSource = QgsOgrProviderUtils::OGROpenWrapper( TO8F( mFilePath ), false, &ogrDriver );
  }

  if ( ogrDataSource )
  {
    QgsDebugMsg( "OGR opened using Driver " + QString( OGR_Dr_GetName( ogrDriver ) ) );

    ogrDriverName = OGR_Dr_GetName( ogrDriver );

    // We get the layer which was requested by the uri. The layername
    // has precedence over the layerid if both are given.
    if ( mLayerName.isNull() )
    {
      ogrOrigLayer = OGR_DS_GetLayer( ogrDataSource, mLayerIndex );
    }
    else
    {
      ogrOrigLayer = OGR_DS_GetLayerByName( ogrDataSource, TO8( mLayerName ) );
    }

    ogrLayer = ogrOrigLayer;
    if ( ogrLayer )
    {
      // check that the initial encoding setting is fit for this layer
      setEncoding( encoding() );

      mValid = setSubsetString( mSubsetString );
      if ( mValid )
      {
        if ( mode == OpenModeInitial )
        {
          computeCapabilities();
        }
        QgsDebugMsg( "Data source is valid" );
      }
      else
      {
        QgsMessageLog::logMessage( tr( "Data source is invalid (%1)" ).arg( QString::fromUtf8( CPLGetLastErrorMsg() ) ), tr( "OGR" ) );
      }
    }
    else
    {
      QgsMessageLog::logMessage( tr( "Data source is invalid, no layer found (%1)" ).arg( QString::fromUtf8( CPLGetLastErrorMsg() ) ), tr( "OGR" ) );
    }
  }
  else
  {
    QgsMessageLog::logMessage( tr( "Data source is invalid (%1)" ).arg( QString::fromUtf8( CPLGetLastErrorMsg() ) ), tr( "OGR" ) );
  }

  // For shapefiles or MapInfo .tab, so as to allow concurrent opening between
  // QGIS and MapInfo, we go back to read-only mode for now.
  // We limit to those drivers as re-opening is relatively cheap (other drivers
  // like GeoJSON might do full content ingestion for example)
  if ( mValid && mode == OpenModeInitial && mWriteAccess &&
       ( ogrDriverName == "ESRI Shapefile" || ogrDriverName == "MapInfo File" ) )
  {
    OGR_DS_Destroy( ogrDataSource );
    ogrLayer = ogrOrigLayer = nullptr;
    mValid = false;

    ogrDataSource = QgsOgrProviderUtils::OGROpenWrapper( TO8F( mFilePath ), false, &ogrDriver );

    mWriteAccess = false;

    if ( ogrDataSource )
    {
      // We get the layer which was requested by the uri. The layername
      // has precedence over the layerid if both are given.
      if ( mLayerName.isNull() )
      {
        ogrOrigLayer = OGR_DS_GetLayer( ogrDataSource, mLayerIndex );
      }
      else
      {
        ogrOrigLayer = OGR_DS_GetLayerByName( ogrDataSource, TO8( mLayerName ) );
      }

      ogrLayer = ogrOrigLayer;
    }
    if ( ogrLayer )
    {
      mValid = true;
      mDynamicWriteAccess = true;

      if ( !mSubsetString.isEmpty() )
      {
        int featuresCountedBackup = mFeaturesCounted;
        mFeaturesCounted = -1;
        mValid = setSubsetString( mSubsetString, false );
        mFeaturesCounted = featuresCountedBackup;
      }
    }
  }

  // For debug/testing purposes
  if ( !mValid )
    setProperty( "_debug_open_mode", "invalid" );
  else if ( mWriteAccess )
    setProperty( "_debug_open_mode", "read-write" );
  else
    setProperty( "_debug_open_mode", "read-only" );
}

void QgsOgrProvider::close()
{
  if ( ogrLayer != ogrOrigLayer )
  {
    OGR_DS_ReleaseResultSet( ogrDataSource, ogrLayer );
  }

  if ( ogrDataSource )
  {
    OGR_DS_Destroy( ogrDataSource );
  }
  ogrDataSource = nullptr;
  ogrLayer = nullptr;
  ogrOrigLayer = nullptr;
  mValid = false;
  setProperty( "_debug_open_mode", "invalid" );

  updateExtents();
}

void QgsOgrProvider::reloadData()
{
  forceReload();
  close();
  open( OpenModeSameAsCurrent );
  if ( !mValid )
    pushError( tr( "Cannot reopen datasource %1" ).arg( dataSourceUri() ) );
}

bool QgsOgrProvider::enterUpdateMode()
{
  if ( !mWriteAccessPossible )
  {
    return false;
  }
  if ( mWriteAccess )
  {
    ++mUpdateModeStackDepth;
    return true;
  }
  if ( mUpdateModeStackDepth == 0 )
  {
    Q_ASSERT( mDynamicWriteAccess );
    QgsDebugMsg( QString( "Reopening %1 in update mode" ).arg( dataSourceUri() ) );
    close();
    open( OpenModeForceUpdate );
    if ( !ogrDataSource || !mWriteAccess )
    {
      QgsMessageLog::logMessage( tr( "Cannot reopen datasource %1 in update mode" ).arg( dataSourceUri() ), tr( "OGR" ) );
      pushError( tr( "Cannot reopen datasource %1 in update mode" ).arg( dataSourceUri() ) );
      return false;
    }
  }
  ++mUpdateModeStackDepth;
  return true;
}

bool QgsOgrProvider::leaveUpdateMode()
{
  if ( !mWriteAccessPossible )
  {
    return false;
  }
  --mUpdateModeStackDepth;
  if ( mUpdateModeStackDepth < 0 )
  {
    QgsMessageLog::logMessage( tr( "Unbalanced call to leaveUpdateMode() w.r.t. enterUpdateMode()" ), tr( "OGR" ) );
    mUpdateModeStackDepth = 0;
    return false;
  }
  if ( !mDynamicWriteAccess )
  {
    return true;
  }
  if ( mUpdateModeStackDepth == 0 )
  {
    QgsDebugMsg( QString( "Reopening %1 in read-only mode" ).arg( dataSourceUri() ) );
    close();
    open( OpenModeForceReadOnly );
    if ( !ogrDataSource )
    {
      QgsMessageLog::logMessage( tr( "Cannot reopen datasource %1 in read-only mode" ).arg( dataSourceUri() ), tr( "OGR" ) );
      pushError( tr( "Cannot reopen datasource %1 in read-only mode" ).arg( dataSourceUri() ) );
      return false;
    }
  }
  return true;
}

// ---------------------------------------------------------------------------

QGISEXTERN QgsVectorLayerImport::ImportError createEmptyLayer(
  const QString& uri,
  const QgsFields &fields,
  QGis::WkbType wkbType,
  const QgsCoordinateReferenceSystem *srs,
  bool overwrite,
  QMap<int, int> *oldToNewAttrIdxMap,
  QString *errorMessage,
  const QMap<QString, QVariant> *options )
{
  return QgsOgrProvider::createEmptyLayer(
           uri, fields, wkbType, srs, overwrite,
           oldToNewAttrIdxMap, errorMessage, options
         );
}

QGISEXTERN void cleanupProvider()
{
  QgsOgrConnPool::cleanupInstance();
  // NOTE: QgsApplication takes care of
  // calling OGRCleanupAll();
}

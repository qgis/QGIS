/***************************************************************************
                          qgsvectorfilewriter.cpp
                          generic vector file writer
                             -------------------
    begin                : Sat Jun 16 2004
    copyright            : (C) 2004 by Tim Sutton
    email                : tim at linfiniti.com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsapplication.h"
#include "qgsfield.h"
#include "qgsfeature.h"
#include "qgsgeometry.h"
#include "qgslogger.h"
#include "qgscoordinatereferencesystem.h"
#include "qgsvectorfilewriter.h"

#include <QFile>
#include <QSettings>
#include <QFileInfo>
#include <QDir>
#include <QTextCodec>
#include <QTextStream>
#include <QSet>
#include <QMetaType>

#include <cassert>
#include <cstdlib> // size_t
#include <limits> // std::numeric_limits

#include <ogr_api.h>
#include <ogr_srs_api.h>
#include <cpl_error.h>
#include <cpl_conv.h>

#if defined(GDAL_VERSION_NUM) && GDAL_VERSION_NUM >= 1800
#define TO8(x)   (x).toUtf8().constData()
#define TO8F(x)  (x).toUtf8().constData()
#else
#define TO8(x)   (x).toLocal8Bit().constData()
#define TO8F(x)  QFile::encodeName( x ).constData()
#endif


QgsVectorFileWriter::QgsVectorFileWriter(
  const QString &theVectorFileName,
  const QString &theFileEncoding,
  const QgsFieldMap& fields,
  QGis::WkbType geometryType,
  const QgsCoordinateReferenceSystem* srs,
  const QString& driverName,
  const QStringList &datasourceOptions,
  const QStringList &layerOptions
)
    : mDS( NULL )
    , mLayer( NULL )
    , mGeom( NULL )
    , mError( NoError )
{
  QString vectorFileName = theVectorFileName;
  QString fileEncoding = theFileEncoding;

  // find driver in OGR
  OGRSFDriverH poDriver;
  QgsApplication::registerOgrDrivers();
  poDriver = OGRGetDriverByName( driverName.toLocal8Bit().data() );

  if ( poDriver == NULL )
  {
    mErrorMessage = QObject::tr( "OGR driver for '%1' not found (OGR error: %2)" )
                    .arg( driverName )
                    .arg( QString::fromUtf8( CPLGetLastErrorMsg() ) );
    mError = ErrDriverNotFound;
    return;
  }

  if ( driverName == "ESRI Shapefile" )
  {
    if ( !vectorFileName.endsWith( ".shp", Qt::CaseInsensitive ) &&
         !vectorFileName.endsWith( ".dbf", Qt::CaseInsensitive ) )
    {
      vectorFileName += ".shp";
    }

#if defined(GDAL_VERSION_NUM) && GDAL_VERSION_NUM < 1700
    // check for unique fieldnames
    QSet<QString> fieldNames;
    QgsFieldMap::const_iterator fldIt;
    for ( fldIt = fields.begin(); fldIt != fields.end(); ++fldIt )
    {
      QString name = fldIt.value().name().left( 10 );
      if ( fieldNames.contains( name ) )
      {
        mErrorMessage = QObject::tr( "trimming attribute name '%1' to ten significant characters produces duplicate column name." )
                        .arg( fldIt.value().name() );
        mError = ErrAttributeCreationFailed;
        return;
      }
      fieldNames << name;
    }
#endif

    deleteShapeFile( vectorFileName );
  }
  else if ( driverName == "KML" )
  {
    if ( !vectorFileName.endsWith( ".kml", Qt::CaseInsensitive ) )
    {
      vectorFileName += ".kml";
    }

    if ( fileEncoding.compare( "UTF-8", Qt::CaseInsensitive ) != 0 )
    {
      QgsDebugMsg( "forced UTF-8 encoding for KML" );
      fileEncoding = "UTF-8";
    }

    QFile::remove( vectorFileName );
  }
  else
  {
    QString longName;
    QString trLongName;
    QString glob;
    QString exts;
    if ( QgsVectorFileWriter::driverMetadata( driverName, longName, trLongName, glob, exts ) )
    {
      QStringList allExts = exts.split( " ", QString::SkipEmptyParts );
      bool found = false;
      foreach( QString ext, allExts )
      {
        if ( vectorFileName.endsWith( "." + ext, Qt::CaseInsensitive ) )
        {
          found = true;
          break;
        }
      }

      if ( !found )
      {
        vectorFileName += "." + allExts[0];
      }
    }

    QFile::remove( vectorFileName );
  }

  char **options = NULL;
  if ( !datasourceOptions.isEmpty() )
  {
    options = new char *[ datasourceOptions.size()+1 ];
    for ( int i = 0; i < datasourceOptions.size(); i++ )
    {
      options[i] = CPLStrdup( datasourceOptions[i].toLocal8Bit().data() );
    }
    options[ datasourceOptions.size()] = NULL;
  }

  // create the data source
  mDS = OGR_Dr_CreateDataSource( poDriver, TO8( vectorFileName ), options );

  if ( options )
  {
    for ( int i = 0; i < datasourceOptions.size(); i++ )
      CPLFree( options[i] );
    delete [] options;
    options = NULL;
  }

  if ( mDS == NULL )
  {
    mError = ErrCreateDataSource;
    mErrorMessage = QObject::tr( "creation of data source failed (OGR error:%1)" )
                    .arg( QString::fromUtf8( CPLGetLastErrorMsg() ) );
    return;
  }

  QgsDebugMsg( "Created data source" );

  // use appropriate codec
  mCodec = QTextCodec::codecForName( fileEncoding.toLocal8Bit().data() );
  if ( !mCodec )
  {
    QSettings settings;
    QString enc = settings.value( "/UI/encoding", QString( "System" ) ).toString();
    QgsDebugMsg( "error finding QTextCodec for " + fileEncoding );
    mCodec = QTextCodec::codecForName( enc.toLocal8Bit().data() );
    if ( !mCodec )
    {
      QgsDebugMsg( "error finding QTextCodec for " + enc );
      mCodec = QTextCodec::codecForLocale();
    }
  }

  // consider spatial reference system of the layer
  OGRSpatialReferenceH ogrRef = NULL;
  if ( srs )
  {
    QString srsWkt = srs->toWkt();
    QgsDebugMsg( "WKT to save as is " + srsWkt );
    ogrRef = OSRNewSpatialReference( srsWkt.toLocal8Bit().data() );
  }

  // datasource created, now create the output layer
  QString layerName = QFileInfo( vectorFileName ).baseName();
  OGRwkbGeometryType wkbType = static_cast<OGRwkbGeometryType>( geometryType );

  if ( !layerOptions.isEmpty() )
  {
    options = new char *[ layerOptions.size()+1 ];
    for ( int i = 0; i < layerOptions.size(); i++ )
    {
      options[i] = CPLStrdup( layerOptions[i].toLocal8Bit().data() );
    }
    options[ layerOptions.size()] = NULL;
  }

  mLayer = OGR_DS_CreateLayer( mDS, TO8F( layerName ), ogrRef, wkbType, options );

  if ( options )
  {
    for ( int i = 0; i < layerOptions.size(); i++ )
      CPLFree( options[i] );
    delete [] options;
    options = NULL;
  }

  if ( srs )
  {
    if ( driverName == "ESRI Shapefile" )
    {
      QString layerName = vectorFileName.left( vectorFileName.indexOf( ".shp", Qt::CaseInsensitive ) );
      QFile prjFile( layerName + ".qpj" );
      if ( prjFile.open( QIODevice::WriteOnly ) )
      {
        QTextStream prjStream( &prjFile );
        prjStream << srs->toWkt().toLocal8Bit().data() << endl;
        prjFile.close();
      }
      else
      {
        QgsDebugMsg( "Couldn't open file " + layerName + ".qpj" );
      }
    }

    OSRDestroySpatialReference( ogrRef );
  }

  if ( mLayer == NULL )
  {
    mErrorMessage = QObject::tr( "creation of layer failed (OGR error:%1)" )
                    .arg( QString::fromUtf8( CPLGetLastErrorMsg() ) );
    mError = ErrCreateLayer;
    return;
  }

  OGRFeatureDefnH defn = OGR_L_GetLayerDefn( mLayer );

  QgsDebugMsg( "created layer" );

  // create the fields
  QgsDebugMsg( "creating " + QString::number( fields.size() ) + " fields" );

  mFields = fields;
  mAttrIdxToOgrIdx.clear();

  QgsFieldMap::const_iterator fldIt;
  for ( fldIt = fields.begin(); fldIt != fields.end(); ++fldIt )
  {
    const QgsField& attrField = fldIt.value();

    OGRFieldType ogrType = OFTString; //default to string
    int ogrWidth = fldIt->length();
    int ogrPrecision = fldIt->precision();
    switch ( attrField.type() )
    {
      case QVariant::LongLong:
        ogrType = OFTString;
        ogrWidth = ogrWidth > 0 && ogrWidth <= 21 ? ogrWidth : 21;
        ogrPrecision = -1;
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

      default:
        //assert(0 && "invalid variant type!");
        mErrorMessage = QObject::tr( "unsupported type for field %1" )
                        .arg( attrField.name() );
        mError = ErrAttributeTypeUnsupported;
        return;
    }

    // create field definition
    OGRFieldDefnH fld = OGR_Fld_Create( mCodec->fromUnicode( attrField.name() ), ogrType );
    if ( ogrWidth > 0 )
    {
      OGR_Fld_SetWidth( fld, ogrWidth );
    }

    if ( ogrPrecision >= 0 )
    {
      OGR_Fld_SetPrecision( fld, ogrPrecision );
    }

    // create the field
    QgsDebugMsg( "creating field " + attrField.name() +
                 " type " + QString( QVariant::typeToName( attrField.type() ) ) +
                 " width " + QString::number( ogrWidth ) +
                 " precision " + QString::number( ogrPrecision ) );
    if ( OGR_L_CreateField( mLayer, fld, true ) != OGRERR_NONE )
    {
      QgsDebugMsg( "error creating field " + attrField.name() );
      mErrorMessage = QObject::tr( "creation of field %1 failed (OGR error: %2)" )
                      .arg( attrField.name() )
                      .arg( QString::fromUtf8( CPLGetLastErrorMsg() ) );
      mError = ErrAttributeCreationFailed;
      return;
    }

    int ogrIdx = OGR_FD_GetFieldIndex( defn, mCodec->fromUnicode( attrField.name() ) );
    if ( ogrIdx < 0 )
    {
#if defined(GDAL_VERSION_NUM) && GDAL_VERSION_NUM < 1700
      // if we didn't find our new column, assume it's name was truncated and
      // it was the last one added (like for shape files)
      int fieldCount = OGR_FD_GetFieldCount( defn );

      OGRFieldDefnH fdefn = OGR_FD_GetFieldDefn( defn, fieldCount - 1 );
      if ( fdefn )
      {
        const char *fieldName = OGR_Fld_GetNameRef( fdefn );

        if ( attrField.name().left( strlen( fieldName ) ) == fieldName )
        {
          ogrIdx = fieldCount - 1;
        }
      }
#else
      // GDAL 1.7 not just truncates, but launders more aggressivly.
      ogrIdx = OGR_FD_GetFieldCount( defn ) - 1;
#endif

      if ( ogrIdx < 0 )
      {
        QgsDebugMsg( "error creating field " + attrField.name() );
        mErrorMessage = QObject::tr( "created field %1 not found (OGR error: %2)" )
                        .arg( attrField.name() )
                        .arg( QString::fromUtf8( CPLGetLastErrorMsg() ) );
        mError = ErrAttributeCreationFailed;
        return;
      }
    }

    mAttrIdxToOgrIdx.insert( fldIt.key(), ogrIdx );
  }

  QgsDebugMsg( "Done creating fields" );

  mWkbType = geometryType;
  if ( mWkbType != QGis::WKBNoGeometry )
  {
    // create geometry which will be used for import
    mGeom = createEmptyGeometry( mWkbType );
  }
}

OGRGeometryH QgsVectorFileWriter::createEmptyGeometry( QGis::WkbType wkbType )
{
  return OGR_G_CreateGeometry(( OGRwkbGeometryType ) wkbType );
}


QgsVectorFileWriter::WriterError QgsVectorFileWriter::hasError()
{
  return mError;
}

QString QgsVectorFileWriter::errorMessage()
{
  return mErrorMessage;
}

bool QgsVectorFileWriter::addFeature( QgsFeature& feature )
{
  // create the feature
  OGRFeatureH poFeature = OGR_F_Create( OGR_L_GetLayerDefn( mLayer ) );

  qint64 fid = FID_TO_NUMBER( feature.id() );
  if ( fid > std::numeric_limits<int>::max() )
  {
    QgsDebugMsg( QString( "feature id %1 too large." ).arg( fid ) );
  }

  OGRErr err = OGR_F_SetFID( poFeature, static_cast<long>( fid ) );
  if ( err != OGRERR_NONE )
  {
    QgsDebugMsg( QString( "Failed to set feature id to %1: %2 (OGR error: %3)" )
                 .arg( feature.id() )
                 .arg( err ).arg( CPLGetLastErrorMsg() )
               );
  }

  // attribute handling
  QgsFieldMap::const_iterator fldIt;
  for ( fldIt = mFields.begin(); fldIt != mFields.end(); ++fldIt )
  {
    if ( !feature.attributeMap().contains( fldIt.key() ) )
    {
      QgsDebugMsg( QString( "no attribute for field %1" ).arg( fldIt.key() ) );
      continue;
    }

    if ( !mAttrIdxToOgrIdx.contains( fldIt.key() ) )
    {
      QgsDebugMsg( QString( "no ogr field for field %1" ).arg( fldIt.key() ) );
      continue;
    }

    const QVariant& attrValue = feature.attributeMap()[ fldIt.key()];
    int ogrField = mAttrIdxToOgrIdx[ fldIt.key()];

    switch ( attrValue.type() )
    {
      case QVariant::Int:
        OGR_F_SetFieldInteger( poFeature, ogrField, attrValue.toInt() );
        break;
      case QVariant::Double:
        OGR_F_SetFieldDouble( poFeature, ogrField, attrValue.toDouble() );
        break;
      case QVariant::LongLong:
      case QVariant::String:
        OGR_F_SetFieldString( poFeature, ogrField, mCodec->fromUnicode( attrValue.toString() ).data() );
        break;
      case QVariant::Invalid:
        break;
      default:
        mErrorMessage = QObject::tr( "Invalid variant type for field %1[%2]: received %3 with type %4" )
                        .arg( fldIt.value().name() )
                        .arg( ogrField )
                        .arg( QMetaType::typeName( attrValue.type() ) )
                        .arg( attrValue.toString() );
        QgsDebugMsg( mErrorMessage );
        mError = ErrFeatureWriteFailed;
        return false;
    }
  }

  if ( mWkbType != QGis::WKBNoGeometry )
  {
    // build geometry from WKB
    QgsGeometry *geom = feature.geometry();
    if ( geom && geom->wkbType() != mWkbType )
    {
      // there's a problem when layer type is set as wkbtype Polygon
      // although there are also features of type MultiPolygon
      // (at least in OGR provider)
      // If the feature's wkbtype is different from the layer's wkbtype,
      // try to export it too.
      //
      // Btw. OGRGeometry must be exactly of the type of the geometry which it will receive
      // i.e. Polygons can't be imported to OGRMultiPolygon

      OGRGeometryH mGeom2 = createEmptyGeometry( geom->wkbType() );

      if ( !mGeom2 )
      {
        QgsDebugMsg( QString( "Failed to create empty geometry for type %1 (OGR error: %2)" ).arg( geom->wkbType() ).arg( CPLGetLastErrorMsg() ) );
        mErrorMessage = QObject::tr( "Feature geometry not imported (OGR error: %1)" )
                        .arg( QString::fromUtf8( CPLGetLastErrorMsg() ) );
        mError = ErrFeatureWriteFailed;
        OGR_F_Destroy( poFeature );
        return false;
      }

      OGRErr err = OGR_G_ImportFromWkb( mGeom2, geom->asWkb(), geom->wkbSize() );
      if ( err != OGRERR_NONE )
      {
        QgsDebugMsg( QString( "Failed to import geometry from WKB: %1 (OGR error: %2)" ).arg( err ).arg( CPLGetLastErrorMsg() ) );
        mErrorMessage = QObject::tr( "Feature geometry not imported (OGR error: %1)" )
                        .arg( QString::fromUtf8( CPLGetLastErrorMsg() ) );
        mError = ErrFeatureWriteFailed;
        OGR_F_Destroy( poFeature );
        return false;
      }

      // pass ownership to geometry
      OGR_F_SetGeometryDirectly( poFeature, mGeom2 );
    }
    else if ( geom )
    {
      OGRErr err = OGR_G_ImportFromWkb( mGeom, geom->asWkb(), geom->wkbSize() );
      if ( err != OGRERR_NONE )
      {
        QgsDebugMsg( QString( "Failed to import geometry from WKB: %1 (OGR error: %2)" ).arg( err ).arg( CPLGetLastErrorMsg() ) );
        mErrorMessage = QObject::tr( "Feature geometry not imported (OGR error: %1)" )
                        .arg( QString::fromUtf8( CPLGetLastErrorMsg() ) );
        mError = ErrFeatureWriteFailed;
        OGR_F_Destroy( poFeature );
        return false;
      }

      // set geometry (ownership is not passed to OGR)
      OGR_F_SetGeometry( poFeature, mGeom );
    }
  }

  // put the created feature to layer
  if ( OGR_L_CreateFeature( mLayer, poFeature ) != OGRERR_NONE )
  {
    mErrorMessage = QObject::tr( "Feature creation error (OGR error: %1)" ).arg( QString::fromUtf8( CPLGetLastErrorMsg() ) );
    mError = ErrFeatureWriteFailed;

    QgsDebugMsg( mErrorMessage );
    OGR_F_Destroy( poFeature );
    return false;
  }

  OGR_F_Destroy( poFeature );

  return true;
}

QgsVectorFileWriter::~QgsVectorFileWriter()
{
  if ( mGeom )
  {
    OGR_G_DestroyGeometry( mGeom );
  }

  if ( mDS )
  {
    OGR_DS_Destroy( mDS );
  }
}




QgsVectorFileWriter::WriterError
QgsVectorFileWriter::writeAsShapefile( QgsVectorLayer* layer,
                                       const QString& shapefileName,
                                       const QString& fileEncoding,
                                       const QgsCoordinateReferenceSystem* destCRS,
                                       bool onlySelected,
                                       QString *errorMessage,
                                       const QStringList &datasourceOptions,
                                       const QStringList &layerOptions )
{
  return writeAsVectorFormat( layer, shapefileName, fileEncoding, destCRS, "ESRI Shapefile", onlySelected, errorMessage, datasourceOptions, layerOptions );
}

QgsVectorFileWriter::WriterError
QgsVectorFileWriter::writeAsVectorFormat( QgsVectorLayer* layer,
    const QString& fileName,
    const QString& fileEncoding,
    const QgsCoordinateReferenceSystem *destCRS,
    const QString& driverName,
    bool onlySelected,
    QString *errorMessage,
    const QStringList &datasourceOptions,
    const QStringList &layerOptions,
    bool skipAttributeCreation )
{
  const QgsCoordinateReferenceSystem* outputCRS;
  QgsCoordinateTransform* ct = 0;
  int shallTransform = false;

  if ( layer == NULL )
  {
    return ErrInvalidLayer;
  }

  if ( destCRS && destCRS->isValid() )
  {
    // This means we should transform
    outputCRS = destCRS;
    shallTransform = true;
  }
  else
  {
    // This means we shouldn't transform, use source CRS as output (if defined)
    outputCRS = &layer->crs();
  }
  QgsVectorFileWriter* writer =
    new QgsVectorFileWriter( fileName, fileEncoding, skipAttributeCreation ? QgsFieldMap() : layer->pendingFields(), layer->wkbType(), outputCRS, driverName, datasourceOptions, layerOptions );

  // check whether file creation was successful
  WriterError err = writer->hasError();
  if ( err != NoError )
  {
    if ( errorMessage )
      *errorMessage = writer->errorMessage();
    delete writer;
    return err;
  }

  if ( errorMessage )
  {
    errorMessage->clear();
  }

  QgsAttributeList allAttr = skipAttributeCreation ? QgsAttributeList() : layer->pendingAllAttributesList();
  QgsFeature fet;

  layer->select( allAttr, QgsRectangle(), layer->wkbType() != QGis::WKBNoGeometry );

  const QgsFeatureIds& ids = layer->selectedFeaturesIds();

  // Create our transform
  if ( destCRS )
  {
    ct = new QgsCoordinateTransform( layer->crs(), *destCRS );
  }

  // Check for failure
  if ( ct == NULL )
  {
    shallTransform = false;
  }

  int n = 0, errors = 0;

  // write all features
  while ( layer->nextFeature( fet ) )
  {
    if ( onlySelected && !ids.contains( fet.id() ) )
      continue;

    if ( shallTransform )
    {
      try
      {
        if ( fet.geometry() )
        {
          fet.geometry()->transform( *ct );
        }
      }
      catch ( QgsCsException &e )
      {
        delete ct;
        delete writer;

        QString msg = QObject::tr( "Failed to transform a point while drawing a feature of type '%1'. Writing stopped. (Exception: %2)" )
                      .arg( fet.typeName() ).arg( e.what() );
        QgsLogger::warning( msg );
        if ( errorMessage )
          *errorMessage = msg;

        return ErrProjection;
      }
    }
    if ( skipAttributeCreation )
    {
      fet.clearAttributeMap();
    }
    if ( !writer->addFeature( fet ) )
    {
      WriterError err = writer->hasError();
      if ( err != NoError && errorMessage )
      {
        if ( errorMessage->isEmpty() )
        {
          *errorMessage = QObject::tr( "Feature write errors:" );
        }
        *errorMessage += "\n" + writer->errorMessage();
      }
      errors++;

      if ( errors > 1000 )
      {
        if ( errorMessage )
        {
          *errorMessage += QObject::tr( "Stopping after %1 errors" ).arg( errors );
        }

        n = -1;
        break;
      }
    }
    n++;
  }

  delete writer;

  if ( shallTransform )
  {
    delete ct;
  }

  if ( errors > 0 && errorMessage && n > 0 )
  {
    *errorMessage += QObject::tr( "\nOnly %1 of %2 features written." ).arg( n - errors ).arg( n );
  }

  return errors == 0 ? NoError : ErrFeatureWriteFailed;
}


bool QgsVectorFileWriter::deleteShapeFile( QString theFileName )
{
  QFileInfo fi( theFileName );
  QDir dir = fi.dir();

  QStringList filter;
  const char *suffixes[] = { ".shp", ".shx", ".dbf", ".prj", ".qix", ".qpj" };
  for ( std::size_t i = 0; i < sizeof( suffixes ) / sizeof( *suffixes ); i++ )
  {
    filter << fi.completeBaseName() + suffixes[i];
  }

  bool ok = true;
  foreach( QString file, dir.entryList( filter ) )
  {
    if ( !QFile::remove( dir.canonicalPath() + "/" + file ) )
    {
      QgsDebugMsg( "Removing file failed : " + file );
      ok = false;
    }
  }

  return ok;
}

QMap< QString, QString> QgsVectorFileWriter::supportedFiltersAndFormats()
{
  QMap<QString, QString> resultMap;

  QgsApplication::registerOgrDrivers();
  int const drvCount = OGRGetDriverCount();

  for ( int i = 0; i < drvCount; ++i )
  {
    OGRSFDriverH drv = OGRGetDriver( i );
    if ( drv )
    {
      QString drvName = OGR_Dr_GetName( drv );
      if ( OGR_Dr_TestCapability( drv, "CreateDataSource" ) != 0 )
      {
        QString filterString = filterForDriver( drvName );
        if ( filterString.isEmpty() )
          continue;

        resultMap.insert( filterString, drvName );
      }
    }
  }

  return resultMap;
}

QMap<QString, QString> QgsVectorFileWriter::ogrDriverList()
{
  QMap<QString, QString> resultMap;

  QgsApplication::registerOgrDrivers();
  int const drvCount = OGRGetDriverCount();

  for ( int i = 0; i < drvCount; ++i )
  {
    OGRSFDriverH drv = OGRGetDriver( i );
    if ( drv )
    {
      QString drvName = OGR_Dr_GetName( drv );
      if ( OGR_Dr_TestCapability( drv, "CreateDataSource" ) != 0 )
      {
        QString longName;
        QString trLongName;
        QString glob;
        QString exts;
        if ( QgsVectorFileWriter::driverMetadata( drvName, longName, trLongName, glob, exts ) && !trLongName.isEmpty() )
        {
          resultMap.insert( trLongName, drvName );
        }
      }
    }
  }

  return resultMap;
}

QString QgsVectorFileWriter::fileFilterString()
{
  QString filterString;
  QMap< QString, QString> driverFormatMap = supportedFiltersAndFormats();
  QMap< QString, QString>::const_iterator it = driverFormatMap.constBegin();
  for ( ; it != driverFormatMap.constEnd(); ++it )
  {
    if ( filterString.isEmpty() )
      filterString += ";;";

    filterString += it.key();
  }
  return filterString;
}

QString QgsVectorFileWriter::filterForDriver( const QString& driverName )
{
  QString longName;
  QString trLongName;
  QString glob;
  QString exts;
  if ( !driverMetadata( driverName, longName, trLongName, glob, exts ) || trLongName.isEmpty() || glob.isEmpty() )
    return "";

  return trLongName + " [OGR] (" + glob.toLower() + " " + glob.toUpper() + ")";
}

bool QgsVectorFileWriter::driverMetadata( QString driverName, QString &longName, QString &trLongName, QString &glob, QString &ext )
{
  if ( driverName.startsWith( "AVCE00" ) )
  {
    longName = "Arc/Info ASCII Coverage";
    trLongName = QObject::tr( "Arc/Info ASCII Coverage" );
    glob = "*.e00";
    ext = "e00";
  }
  else if ( driverName.startsWith( "BNA" ) )
  {
    longName = "Atlas BNA";
    trLongName = QObject::tr( "Atlas BNA" );
    glob = "*.bna";
    ext = "bna";
  }
  else if ( driverName.startsWith( "CSV" ) )
  {
    longName = "Comma Separated Value";
    trLongName = QObject::tr( "Comma Separated Value" );
    glob = "*.csv";
    ext = "csv";
  }
  else if ( driverName.startsWith( "ESRI" ) )
  {
    longName = "ESRI Shapefile";
    trLongName = QObject::tr( "ESRI Shapefile" );
    glob = "*.shp";
    ext = "shp";
  }
  else if ( driverName.startsWith( "FMEObjects Gateway" ) )
  {
    longName = "FMEObjects Gateway";
    trLongName = QObject::tr( "FMEObjects Gateway" );
    glob = "*.fdd";
    ext = "fdd";
  }
  else if ( driverName.startsWith( "GeoJSON" ) )
  {
    longName = "GeoJSON";
    trLongName = QObject::tr( "GeoJSON" );
    glob = "*.geojson";
    ext = "geojson";
  }
  else if ( driverName.startsWith( "GeoRSS" ) )
  {
    longName = "GeoRSS";
    trLongName = QObject::tr( "GeoRSS" );
    glob = "*.xml";
    ext = "xml";
  }
  else if ( driverName.startsWith( "GML" ) )
  {
    longName = "Geography Markup Language [GML]";
    trLongName = QObject::tr( "Geography Markup Language [GML]" );
    glob = "*.gml";
    ext = "gml";
  }
  else if ( driverName.startsWith( "GMT" ) )
  {
    longName = "Generic Mapping Tools [GMT]";
    trLongName = QObject::tr( "Generic Mapping Tools [GMT]" );
    glob = "*.gmt";
    ext = "gmt";
  }
  else if ( driverName.startsWith( "GPX" ) )
  {
    longName = "GPS eXchange Format [GPX]";
    trLongName = QObject::tr( "GPS eXchange Format [GPX]" );
    glob = "*.gpx";
    ext = "gpx";
  }
  else if ( driverName.startsWith( "Interlis 1" ) )
  {
    longName = "INTERLIS 1";
    trLongName = QObject::tr( "INTERLIS 1" );
    glob = "*.itf *.xml *.ili";
    ext = "ili";
  }
  else if ( driverName.startsWith( "Interlis 2" ) )
  {
    longName = "INTERLIS 2";
    trLongName = QObject::tr( "INTERLIS 2" );
    glob = "*.itf *.xml *.ili";
    ext = "ili";
  }
  else if ( driverName.startsWith( "KML" ) )
  {
    longName = "Keyhole Markup Language [KML]";
    trLongName = QObject::tr( "Keyhole Markup Language [KML]" );
    glob = "*.kml" ;
    ext = "kml" ;
  }
  else if ( driverName.startsWith( "MapInfo File" ) )
  {
    longName = "Mapinfo File";
    trLongName = QObject::tr( "Mapinfo File" );
    glob = "*.mif *.tab";
    ext = "mif tab";
  }
  else if ( driverName.startsWith( "DGN" ) )
  {
    longName = "Microstation DGN";
    trLongName = QObject::tr( "Microstation DGN" );
    glob = "*.dgn";
    ext = "dgn";
  }
  else if ( driverName.startsWith( "S57" ) )
  {
    longName = "S-57 Base file";
    trLongName = QObject::tr( "S-57 Base file" );
    glob = "*.000";
    ext = "000";
  }
  else if ( driverName.startsWith( "SDTS" ) )
  {
    longName = "Spatial Data Transfer Standard [SDTS]";
    trLongName = QObject::tr( "Spatial Data Transfer Standard [SDTS]" );
    glob = "*catd.ddf";
    ext = "ddf";
  }
  else if ( driverName.startsWith( "SQLite" ) )
  {
    longName = "SQLite";
    trLongName = QObject::tr( "SQLite" );
    glob = "*.sqlite";
    ext = "sqlite";
  }
  else if ( driverName.startsWith( "DXF" ) )
  {
    longName = "AutoCAD DXF";
    trLongName = QObject::tr( "AutoCAD DXF" );
    glob = "*.dxf";
    ext = "dxf";
  }
  else if ( driverName.startsWith( "Geoconcept" ) )
  {
    longName = "Geoconcept";
    trLongName = QObject::tr( "Geoconcept" );
    glob = "*.gxt *.txt";
    ext = "gxt";
  }
  else
  {
    return false;
  }

  return true;
}

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
#include "qgsmessagelog.h"
#include "qgscoordinatereferencesystem.h"
#include "qgsvectorfilewriter.h"
#include "qgsrendererv2.h"
#include "qgssymbollayerv2.h"

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
  const QgsFields& fields,
  QGis::WkbType geometryType,
  const QgsCoordinateReferenceSystem* srs,
  const QString& driverName,
  const QStringList &datasourceOptions,
  const QStringList &layerOptions,
  QString *newFilename,
  SymbologyExport symbologyExport
)
    : mDS( NULL )
    , mLayer( NULL )
    , mGeom( NULL )
    , mError( NoError )
    , mSymbologyExport( symbologyExport )
{
  QString vectorFileName = theVectorFileName;
  QString fileEncoding = theFileEncoding;
  QStringList layOptions = layerOptions;
  QStringList dsOptions = datasourceOptions;

  QString ogrDriverName;
  if ( driverName == "MapInfo MIF" )
  {
    ogrDriverName = "MapInfo File";
  }
  else if ( driverName == "SpatiaLite" )
  {
    ogrDriverName = "SQLite";
    if ( !dsOptions.contains( "SPATIALITE=YES" ) )
    {
      dsOptions.append( "SPATIALITE=YES" );
    }
  }
  else
  {
    ogrDriverName = driverName;
  }

  // find driver in OGR
  OGRSFDriverH poDriver;
  QgsApplication::registerOgrDrivers();

  poDriver = OGRGetDriverByName( ogrDriverName.toLocal8Bit().data() );

  if ( poDriver == NULL )
  {
    mErrorMessage = QObject::tr( "OGR driver for '%1' not found (OGR error: %2)" )
                    .arg( driverName )
                    .arg( QString::fromUtf8( CPLGetLastErrorMsg() ) );
    mError = ErrDriverNotFound;
    return;
  }

  QSettings settings;

  if ( driverName == "ESRI Shapefile" )
  {
    if ( layOptions.join( "" ).toUpper().indexOf( "ENCODING=" ) == -1 )
    {
      layOptions.append( "ENCODING=" + fileEncoding );
    }

    if ( settings.value( "/qgis/ignoreShapeEncoding", true ).toBool() )
    {
      CPLSetConfigOption( "SHAPE_ENCODING", "" );
    }
    else
    {
      CPLSetConfigOption( "SHAPE_ENCODING", fileEncoding.toLocal8Bit().data() );
      // WARNING!! If SHAPE_ENCODING and -lco ENCODING are used, the fileEncoding must be set to the layer internal encoding!!
      fileEncoding = "UTF-8";
    }

    if ( !vectorFileName.endsWith( ".shp", Qt::CaseInsensitive ) &&
         !vectorFileName.endsWith( ".dbf", Qt::CaseInsensitive ) )
    {
      vectorFileName += ".shp";
    }

#if defined(GDAL_VERSION_NUM) && GDAL_VERSION_NUM < 1700
    // check for unique fieldnames
    QSet<QString> fieldNames;
    for ( int i = 0; i < fields.count(); ++i )
    {
      QString name = fields[i].name().left( 10 );
      if ( fieldNames.contains( name ) )
      {
        mErrorMessage = QObject::tr( "trimming attribute name '%1' to ten significant characters produces duplicate column name." )
                        .arg( fields[i].name() );
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
      foreach ( QString ext, allExts )
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
  if ( !dsOptions.isEmpty() )
  {
    options = new char *[ dsOptions.size()+1 ];
    for ( int i = 0; i < dsOptions.size(); i++ )
    {
      options[i] = CPLStrdup( dsOptions[i].toLocal8Bit().data() );
    }
    options[ dsOptions.size()] = NULL;
  }

  // create the data source
  mDS = OGR_Dr_CreateDataSource( poDriver, TO8( vectorFileName ), options );

  if ( options )
  {
    for ( int i = 0; i < dsOptions.size(); i++ )
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
  mCodec = QTextCodec::codecForName( fileEncoding.toLocal8Bit().constData() );
  if ( !mCodec )
  {
    QgsDebugMsg( "error finding QTextCodec for " + fileEncoding );

    QString enc = settings.value( "/UI/encoding", "System" ).toString();
    mCodec = QTextCodec::codecForName( enc.toLocal8Bit().constData() );
    if ( !mCodec )
    {
      QgsDebugMsg( "error finding QTextCodec for " + enc );
      mCodec = QTextCodec::codecForLocale();
      Q_ASSERT( mCodec );
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

  if ( !layOptions.isEmpty() )
  {
    options = new char *[ layOptions.size()+1 ];
    for ( int i = 0; i < layOptions.size(); i++ )
    {
      options[i] = CPLStrdup( layOptions[i].toLocal8Bit().data() );
    }
    options[ layOptions.size()] = NULL;
  }

  mLayer = OGR_DS_CreateLayer( mDS, TO8F( layerName ), ogrRef, wkbType, options );

  if ( options )
  {
    for ( int i = 0; i < layOptions.size(); i++ )
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

  for ( int fldIdx = 0; fldIdx < fields.count(); ++fldIdx )
  {
    const QgsField& attrField = fields[fldIdx];

    OGRFieldType ogrType = OFTString; //default to string
    int ogrWidth = attrField.length();
    int ogrPrecision = attrField.precision();
    switch ( attrField.type() )
    {
      case QVariant::LongLong:
        ogrType = OFTString;
        ogrWidth = ogrWidth > 0 && ogrWidth <= 21 ? ogrWidth : 21;
        ogrPrecision = -1;
        break;

      case QVariant::String:
        ogrType = OFTString;
        if ( ogrWidth <= 0 || ogrWidth > 255 )
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

    mAttrIdxToOgrIdx.insert( fldIdx, ogrIdx );
  }

  QgsDebugMsg( "Done creating fields" );

  mWkbType = geometryType;
  if ( mWkbType != QGis::WKBNoGeometry )
  {
    // create geometry which will be used for import
    mGeom = createEmptyGeometry( mWkbType );
  }

  if ( newFilename )
    *newFilename = vectorFileName;
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

bool QgsVectorFileWriter::addFeature( QgsFeature& feature, QgsFeatureRendererV2* renderer, QGis::UnitType outputUnit )
{
  // create the feature
  OGRFeatureH poFeature = createFeature( feature );

  //add OGR feature style type
  if ( mSymbologyExport != NoSymbology && renderer )
  {
    //SymbolLayerSymbology: concatenate ogr styles of all symbollayers
    QgsSymbolV2List symbols = renderer->symbolsForFeature( feature );
    QString styleString;
    QString currentStyle;

    QgsSymbolV2List::const_iterator symbolIt = symbols.constBegin();
    for ( ; symbolIt != symbols.constEnd(); ++symbolIt )
    {
      int nSymbolLayers = ( *symbolIt )->symbolLayerCount();
      for ( int i = 0; i < nSymbolLayers; ++i )
      {
#if 0
        QMap< QgsSymbolLayerV2*, QString >::const_iterator it = mSymbolLayerTable.find(( *symbolIt )->symbolLayer( i ) );
        if ( it == mSymbolLayerTable.constEnd() )
        {
          continue;
        }
#endif
        double mmsf = mmScaleFactor( mSymbologyScaleDenominator, ( *symbolIt )->outputUnit(), outputUnit );
        double musf = mapUnitScaleFactor( mSymbologyScaleDenominator, ( *symbolIt )->outputUnit(), outputUnit );

        currentStyle = ( *symbolIt )->symbolLayer( i )->ogrFeatureStyle( mmsf, musf );//"@" + it.value();

        if ( mSymbologyExport == FeatureSymbology )
        {
          if ( symbolIt != symbols.constBegin() || i != 0 )
          {
            styleString.append( ";" );
          }
          styleString.append( currentStyle );
        }
        else if ( mSymbologyExport == SymbolLayerSymbology )
        {
          OGR_F_SetStyleString( poFeature, currentStyle.toLocal8Bit().data() );
          if ( !writeFeature( mLayer, poFeature ) )
          {
            return false;
          }
        }
      }
    }
    OGR_F_SetStyleString( poFeature, styleString.toLocal8Bit().data() );
  }

  if ( mSymbologyExport == NoSymbology || mSymbologyExport == FeatureSymbology )
  {
    if ( !writeFeature( mLayer, poFeature ) )
    {
      return false;
    }
  }

  OGR_F_Destroy( poFeature );
  return true;
}

OGRFeatureH QgsVectorFileWriter::createFeature( QgsFeature& feature )
{
  OGRFeatureH poFeature = OGR_F_Create( OGR_L_GetLayerDefn( mLayer ) );

  qint64 fid = FID_TO_NUMBER( feature.id() );
  if ( fid > std::numeric_limits<int>::max() )
  {
    QgsDebugMsg( QString( "feature id %1 too large." ).arg( fid ) );
    OGRErr err = OGR_F_SetFID( poFeature, static_cast<long>( fid ) );
    if ( err != OGRERR_NONE )
    {
      QgsDebugMsg( QString( "Failed to set feature id to %1: %2 (OGR error: %3)" )
                   .arg( feature.id() )
                   .arg( err ).arg( CPLGetLastErrorMsg() )
                 );
    }
  }

  // attribute handling
  for ( int fldIdx = 0; fldIdx < mFields.count(); ++fldIdx )
  {
    if ( !mAttrIdxToOgrIdx.contains( fldIdx ) )
    {
      QgsDebugMsg( QString( "no ogr field for field %1" ).arg( fldIdx ) );
      continue;
    }

    const QVariant& attrValue = feature.attribute( fldIdx );
    int ogrField = mAttrIdxToOgrIdx[ fldIdx ];

    if ( !attrValue.isValid() || attrValue.isNull() )
      continue;

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
                        .arg( mFields[fldIdx].name() )
                        .arg( ogrField )
                        .arg( QMetaType::typeName( attrValue.type() ) )
                        .arg( attrValue.toString() );
        QgsMessageLog::logMessage( mErrorMessage, QObject::tr( "OGR" ) );
        mError = ErrFeatureWriteFailed;
        return 0;
    }
  }

  if ( mWkbType != QGis::WKBNoGeometry )
  {
    // build geometry from WKB
    QgsGeometry *geom = feature.geometry();

    // turn single geoemetry to multi geometry if needed
    if ( geom && geom->wkbType() != mWkbType && geom->wkbType() == QGis::singleType( mWkbType ) )
    {
      geom->convertToMultiType();
    }

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
        mErrorMessage = QObject::tr( "Feature geometry not imported (OGR error: %1)" )
                        .arg( QString::fromUtf8( CPLGetLastErrorMsg() ) );
        mError = ErrFeatureWriteFailed;
        QgsMessageLog::logMessage( mErrorMessage, QObject::tr( "OGR" ) );
        OGR_F_Destroy( poFeature );
        return 0;
      }

      OGRErr err = OGR_G_ImportFromWkb( mGeom2, geom->asWkb(), geom->wkbSize() );
      if ( err != OGRERR_NONE )
      {
        mErrorMessage = QObject::tr( "Feature geometry not imported (OGR error: %1)" )
                        .arg( QString::fromUtf8( CPLGetLastErrorMsg() ) );
        mError = ErrFeatureWriteFailed;
        QgsMessageLog::logMessage( mErrorMessage, QObject::tr( "OGR" ) );
        OGR_F_Destroy( poFeature );
        return 0;
      }

      // pass ownership to geometry
      OGR_F_SetGeometryDirectly( poFeature, mGeom2 );
    }
    else if ( geom )
    {
      OGRErr err = OGR_G_ImportFromWkb( mGeom, geom->asWkb(), geom->wkbSize() );
      if ( err != OGRERR_NONE )
      {
        mErrorMessage = QObject::tr( "Feature geometry not imported (OGR error: %1)" )
                        .arg( QString::fromUtf8( CPLGetLastErrorMsg() ) );
        mError = ErrFeatureWriteFailed;
        QgsMessageLog::logMessage( mErrorMessage, QObject::tr( "OGR" ) );
        OGR_F_Destroy( poFeature );
        return 0;
      }

      // set geometry (ownership is not passed to OGR)
      OGR_F_SetGeometry( poFeature, mGeom );
    }
  }
  return poFeature;
}

bool QgsVectorFileWriter::writeFeature( OGRLayerH layer, OGRFeatureH feature )
{
  if ( OGR_L_CreateFeature( layer, feature ) != OGRERR_NONE )
  {
    mErrorMessage = QObject::tr( "Feature creation error (OGR error: %1)" ).arg( QString::fromUtf8( CPLGetLastErrorMsg() ) );
    mError = ErrFeatureWriteFailed;
    QgsMessageLog::logMessage( mErrorMessage, QObject::tr( "OGR" ) );
    OGR_F_Destroy( feature );
    return false;
  }
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
QgsVectorFileWriter::writeAsVectorFormat( QgsVectorLayer* layer,
    const QString& fileName,
    const QString& fileEncoding,
    const QgsCoordinateReferenceSystem *destCRS,
    const QString& driverName,
    bool onlySelected,
    QString *errorMessage,
    const QStringList &datasourceOptions,
    const QStringList &layerOptions,
    bool skipAttributeCreation,
    QString *newFilename,
    SymbologyExport symbologyExport,
    double symbologyScale )
{
  QgsDebugMsg( "fileName = " + fileName );
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
    new QgsVectorFileWriter( fileName, fileEncoding, skipAttributeCreation ? QgsFields() : layer->pendingFields(), layer->wkbType(), outputCRS, driverName, datasourceOptions, layerOptions, newFilename, symbologyExport );
  writer->setSymbologyScaleDenominator( symbologyScale );

  if ( newFilename )
  {
    QgsDebugMsg( "newFilename = " + *newFilename );
  }

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

  //add possible attributes needed by renderer
  writer->addRendererAttributes( layer, allAttr );

  QgsFeatureRequest req;
  if ( layer->wkbType() == QGis::WKBNoGeometry )
  {
    req.setFlags( QgsFeatureRequest::NoGeometry );
  }
  req.setSubsetOfAttributes( allAttr );
  QgsFeatureIterator fit = layer->getFeatures( req );

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

  //create symbol table if needed
  if ( writer->symbologyExport() != NoSymbology )
  {
    //writer->createSymbolLayerTable( layer,  writer->mDS );
  }

  if ( writer->symbologyExport() == SymbolLayerSymbology && layer->isUsingRendererV2() )
  {
    QgsFeatureRendererV2* r = layer->rendererV2();
    if ( r->capabilities() & QgsFeatureRendererV2::SymbolLevels
         && r->usingSymbolLevels() )
    {
      QgsVectorFileWriter::WriterError error = writer->exportFeaturesSymbolLevels( layer, fit, ct, errorMessage );
      delete writer;
      delete ct;
      return ( error == NoError ) ? NoError : ErrFeatureWriteFailed;
    }
  }

  int n = 0, errors = 0;

  //unit type
  QGis::UnitType mapUnits = layer->crs().mapUnits();
  if ( ct )
  {
    mapUnits = ct->destCRS().mapUnits();
  }

  writer->startRender( layer );

  // write all features
  while ( fit.nextFeature( fet ) )
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

        QString msg = QObject::tr( "Failed to transform a point while drawing a feature with ID '%1'. Writing stopped. (Exception: %2)" )
                      .arg( fet.id() ).arg( e.what() );
        QgsLogger::warning( msg );
        if ( errorMessage )
          *errorMessage = msg;

        return ErrProjection;
      }
    }
    if ( allAttr.size() < 1 && skipAttributeCreation )
    {
      fet.initAttributes( 0 );
    }

    if ( !writer->addFeature( fet, layer->rendererV2(), mapUnits ) )
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

  writer->stopRender( layer );
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
  foreach ( QString file, dir.entryList( filter ) )
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

  QStringList writableDrivers;
  for ( int i = 0; i < drvCount; ++i )
  {
    OGRSFDriverH drv = OGRGetDriver( i );
    if ( drv )
    {
      QString drvName = OGR_Dr_GetName( drv );
      if ( OGR_Dr_TestCapability( drv, "CreateDataSource" ) != 0 )
      {
        // Add separate format for Mapinfo MIF (MITAB is OGR default)
        if ( drvName == "MapInfo File" )
        {
          writableDrivers << "MapInfo MIF";
        }
        else if ( drvName == "SQLite" )
        {
          // Unfortunately it seems that there is no simple way to detect if
          // OGR SQLite driver is compiled with SpatiaLite support.
          // We have HAVE_SPATIALITE in QGIS, but that may differ from OGR
          // http://lists.osgeo.org/pipermail/gdal-dev/2012-November/034580.html
          // -> test if creation failes
          QString option = "SPATIALITE=YES";
          char **options =  new char *[2];
          options[0] = CPLStrdup( option.toLocal8Bit().data() );
          options[1] = NULL;
          OGRSFDriverH poDriver;
          QgsApplication::registerOgrDrivers();
          poDriver = OGRGetDriverByName( drvName.toLocal8Bit().data() );
          if ( poDriver )
          {
            OGRDataSourceH ds = OGR_Dr_CreateDataSource( poDriver, TO8( QString( "/vsimem/spatialitetest.sqlite" ) ), options );
            if ( ds )
            {
              writableDrivers << "SpatiaLite";
              OGR_DS_Destroy( ds );
            }
          }
          CPLFree( options[0] );
          delete [] options;
        }
        writableDrivers << drvName;
      }
    }
  }

  foreach ( QString drvName, writableDrivers )
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
    longName = "Mapinfo TAB";
    trLongName = QObject::tr( "Mapinfo TAB" );
    glob = "*.tab";
    ext = "tab";
  }
  // 'MapInfo MIF' is internal QGIS addition to distinguish between MITAB and MIF
  else if ( driverName.startsWith( "MapInfo MIF" ) )
  {
    longName = "Mapinfo MIF";
    trLongName = QObject::tr( "Mapinfo MIF" );
    glob = "*.mif";
    ext = "mif";
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
  // QGIS internal addition for SpatialLite
  else if ( driverName.startsWith( "SpatiaLite" ) )
  {
    longName = "SpatiaLite";
    trLongName = QObject::tr( "SpatiaLite" );
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
  else if ( driverName.startsWith( "FileGDB" ) )
  {
    longName = "ESRI FileGDB";
    trLongName = QObject::tr( "ESRI FileGDB" );
    glob = "*.gdb";
    ext = "gdb";
  }
  else
  {
    return false;
  }

  return true;
}

void QgsVectorFileWriter::createSymbolLayerTable( QgsVectorLayer* vl,  const QgsCoordinateTransform* ct, OGRDataSourceH ds )
{
  if ( !vl || !ds )
  {
    return;
  }

  if ( !vl->isUsingRendererV2() )
  {
    return;
  }

  QgsFeatureRendererV2* renderer = vl->rendererV2();
  if ( !renderer )
  {
    return;
  }

  //unit type
  QGis::UnitType mapUnits = vl->crs().mapUnits();
  if ( ct )
  {
    mapUnits = ct->destCRS().mapUnits();
  }

#if defined(GDAL_VERSION_NUM) && GDAL_VERSION_NUM >= 1700
  mSymbolLayerTable.clear();
  OGRStyleTableH ogrStyleTable = OGR_STBL_Create();
  OGRStyleMgrH styleManager = OGR_SM_Create( ogrStyleTable );

  //get symbols
  int nTotalLevels = 0;
  QgsSymbolV2List symbolList = renderer->symbols();
  QgsSymbolV2List::iterator symbolIt = symbolList.begin();
  for ( ; symbolIt != symbolList.end(); ++symbolIt )
  {
    double mmsf = mmScaleFactor( mSymbologyScaleDenominator, ( *symbolIt )->outputUnit(), mapUnits );
    double musf = mapUnitScaleFactor( mSymbologyScaleDenominator, ( *symbolIt )->outputUnit(), mapUnits );

    int nLevels = ( *symbolIt )->symbolLayerCount();
    for ( int i = 0; i < nLevels; ++i )
    {
      mSymbolLayerTable.insert(( *symbolIt )->symbolLayer( i ), QString::number( nTotalLevels ) );
      OGR_SM_AddStyle( styleManager, QString::number( nTotalLevels ).toLocal8Bit(),
                       ( *symbolIt )->symbolLayer( i )->ogrFeatureStyle( mmsf, musf ).toLocal8Bit() );
      ++nTotalLevels;
    }
  }
  OGR_DS_SetStyleTableDirectly( ds, ogrStyleTable );
#endif
}

QgsVectorFileWriter::WriterError QgsVectorFileWriter::exportFeaturesSymbolLevels( QgsVectorLayer* layer, QgsFeatureIterator& fit,
    const QgsCoordinateTransform* ct, QString* errorMessage )
{
  if ( !layer || !layer->isUsingRendererV2() )
  {
    //return error
  }
  QgsFeatureRendererV2* renderer = layer->rendererV2();
  if ( !renderer )
  {
    //return error
  }
  QHash< QgsSymbolV2*, QList<QgsFeature> > features;

  //unit type
  QGis::UnitType mapUnits = layer->crs().mapUnits();
  if ( ct )
  {
    mapUnits = ct->destCRS().mapUnits();
  }

  startRender( layer );

  //fetch features
  QgsFeature fet;
  QgsSymbolV2* featureSymbol = 0;
  while ( fit.nextFeature( fet ) )
  {
    if ( ct )
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

        QString msg = QObject::tr( "Failed to transform, writing stopped. (Exception: %1)" )
                      .arg( e.what() );
        QgsLogger::warning( msg );
        if ( errorMessage )
          *errorMessage = msg;

        return ErrProjection;
      }
    }

    featureSymbol = renderer->symbolForFeature( fet );
    if ( !featureSymbol )
    {
      continue;
    }

    QHash< QgsSymbolV2*, QList<QgsFeature> >::iterator it = features.find( featureSymbol );
    if ( it == features.end() )
    {
      it = features.insert( featureSymbol, QList<QgsFeature>() );
    }
    it.value().append( fet );
  }

  //find out order
  QgsSymbolV2LevelOrder levels;
  QgsSymbolV2List symbols = renderer->symbols();
  for ( int i = 0; i < symbols.count(); i++ )
  {
    QgsSymbolV2* sym = symbols[i];
    for ( int j = 0; j < sym->symbolLayerCount(); j++ )
    {
      int level = sym->symbolLayer( j )->renderingPass();
      if ( level < 0 || level >= 1000 ) // ignore invalid levels
        continue;
      QgsSymbolV2LevelItem item( sym, j );
      while ( level >= levels.count() ) // append new empty levels
        levels.append( QgsSymbolV2Level() );
      levels[level].append( item );
    }
  }

  int nErrors = 0;
  int nTotalFeatures = 0;

  //export symbol layers and symbology
  for ( int l = 0; l < levels.count(); l++ )
  {
    QgsSymbolV2Level& level = levels[l];
    for ( int i = 0; i < level.count(); i++ )
    {
      QgsSymbolV2LevelItem& item = level[i];
      QHash< QgsSymbolV2*, QList<QgsFeature> >::iterator levelIt = features.find( item.symbol() );
      if ( levelIt == features.end() )
      {
        ++nErrors;
        continue;
      }

      double mmsf = mmScaleFactor( mSymbologyScaleDenominator, levelIt.key()->outputUnit(), mapUnits );
      double musf = mapUnitScaleFactor( mSymbologyScaleDenominator, levelIt.key()->outputUnit(), mapUnits );

      int llayer = item.layer();
      QList<QgsFeature>& featureList = levelIt.value();
      QList<QgsFeature>::iterator featureIt = featureList.begin();
      for ( ; featureIt != featureList.end(); ++featureIt )
      {
        ++nTotalFeatures;
        OGRFeatureH ogrFeature = createFeature( *featureIt );
        if ( !ogrFeature )
        {
          ++nErrors;
          continue;
        }

        QString styleString = levelIt.key()->symbolLayer( llayer )->ogrFeatureStyle( mmsf, musf );
        if ( !styleString.isEmpty() )
        {
          OGR_F_SetStyleString( ogrFeature, styleString.toLocal8Bit().data() );
          if ( ! writeFeature( mLayer, ogrFeature ) )
          {
            ++nErrors;
          }
        }
        OGR_F_Destroy( ogrFeature );
      }
    }
  }

  stopRender( layer );

  if ( nErrors > 0 && errorMessage )
  {
    *errorMessage += QObject::tr( "\nOnly %1 of %2 features written." ).arg( nTotalFeatures - nErrors ).arg( nTotalFeatures );
  }

  return ( nErrors > 0 ) ? QgsVectorFileWriter::ErrFeatureWriteFailed : QgsVectorFileWriter::NoError;
}

double QgsVectorFileWriter::mmScaleFactor( double scaleDenominator, QgsSymbolV2::OutputUnit symbolUnits, QGis::UnitType mapUnits )
{
  if ( symbolUnits == QgsSymbolV2::MM )
  {
    return 1.0;
  }
  else
  {
    //conversion factor map units -> mm
    if ( mapUnits == QGis::Meters )
    {
      return 1000 / scaleDenominator;
    }

  }
  return 1.0; //todo: map units
}

double QgsVectorFileWriter::mapUnitScaleFactor( double scaleDenominator, QgsSymbolV2::OutputUnit symbolUnits, QGis::UnitType mapUnits )
{
  if ( symbolUnits == QgsSymbolV2::MapUnit )
  {
    return 1.0;
  }
  else
  {
    if ( symbolUnits == QgsSymbolV2::MM && mapUnits == QGis::Meters )
    {
      return scaleDenominator / 1000;
    }
  }
  return 1.0;
}

QgsRenderContext QgsVectorFileWriter::renderContext() const
{
  QgsRenderContext context;
  context.setRendererScale( mSymbologyScaleDenominator );
  return context;
}

void QgsVectorFileWriter::startRender( QgsVectorLayer* vl ) const
{
  QgsFeatureRendererV2* renderer = symbologyRenderer( vl );
  if ( !renderer )
  {
    return;
  }

  QgsRenderContext ctx = renderContext();
  renderer->startRender( ctx, vl );
}

void QgsVectorFileWriter::stopRender( QgsVectorLayer* vl ) const
{
  QgsFeatureRendererV2* renderer = symbologyRenderer( vl );
  if ( !renderer )
  {
    return;
  }

  QgsRenderContext ctx = renderContext();
  renderer->stopRender( ctx );
}

QgsFeatureRendererV2* QgsVectorFileWriter::symbologyRenderer( QgsVectorLayer* vl ) const
{
  if ( mSymbologyExport == NoSymbology )
  {
    return 0;
  }
  if ( !vl || !vl->isUsingRendererV2() )
  {
    return 0;
  }

  return vl->rendererV2();
}

void QgsVectorFileWriter::addRendererAttributes( QgsVectorLayer* vl, QgsAttributeList& attList )
{
  QgsFeatureRendererV2* renderer = symbologyRenderer( vl );
  if ( renderer )
  {
    QList<QString> rendererAttributes = renderer->usedAttributes();
    for ( int i = 0; i < rendererAttributes.size(); ++i )
    {
      int index = vl->fieldNameIndex( rendererAttributes.at( i ) );
      if ( index != -1 )
      {
        attList.push_back( vl->fieldNameIndex( rendererAttributes.at( i ) ) );
      }
    }
  }
}

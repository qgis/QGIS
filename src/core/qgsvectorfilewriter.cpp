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
/* $Id$ */

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

#include <ogr_api.h>
#include <ogr_srs_api.h>
#include <cpl_error.h>


QgsVectorFileWriter::QgsVectorFileWriter(
  const QString &theVectorFileName,
  const QString &theFileEncoding,
  const QgsFieldMap& fields,
  QGis::WkbType geometryType,
  const QgsCoordinateReferenceSystem* srs,
  const QString& driverName )
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
    if ( !vectorFileName.endsWith( ".shp", Qt::CaseInsensitive ) )
    {
      vectorFileName += ".shp";
    }

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
    QFile::remove( vectorFileName );
  }

  // create the data source
  mDS = OGR_Dr_CreateDataSource( poDriver, vectorFileName.toLocal8Bit().data(), NULL );
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
  mLayer = OGR_DS_CreateLayer( mDS, QFile::encodeName( layerName ).data(), ogrRef, wkbType, NULL );

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
      QgsDebugMsg( "error creating field " + attrField.name() );
      mErrorMessage = QObject::tr( "created field %1 not found (OGR error: %2)" )
                      .arg( attrField.name() )
                      .arg( QString::fromUtf8( CPLGetLastErrorMsg() ) );
      mError = ErrAttributeCreationFailed;
      return;
    }

    mAttrIdxToOgrIdx.insert( fldIt.key(), ogrIdx );
  }

  QgsDebugMsg( "Done creating fields" );

  mWkbType = geometryType;
  // create geometry which will be used for import
  mGeom = createEmptyGeometry( mWkbType );
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
  if ( hasError() != NoError )
    return false;

  QgsAttributeMap::const_iterator it;

  // create the feature
  OGRFeatureH poFeature = OGR_F_Create( OGR_L_GetLayerDefn( mLayer ) );

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
      default:
        QgsDebugMsg( "Invalid variant type for field " + QString( fldIt.value().name() ) + " " 
            + QString::number( ogrField ) + ": Received Type " + QMetaType::typeName (  attrValue.type() )
            + " : With Value : " +  attrValue.toString() 
            );
        return false;
    }
  }

  // build geometry from WKB
  QgsGeometry *geom = feature.geometry();
  if ( !geom )
  {
    QgsDebugMsg( "invalid geometry" );
    OGR_F_Destroy( poFeature );
    return false;
  }

  if ( geom->wkbType() != mWkbType )
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

    OGRErr err = OGR_G_ImportFromWkb( mGeom2, geom->asWkb(), geom->wkbSize() );
    if ( err != OGRERR_NONE )
    {
      QgsDebugMsg( "Failed to import geometry from WKB: " + QString::number( err ) );
      OGR_F_Destroy( poFeature );
      return false;
    }

    // pass ownership to geometry
    OGR_F_SetGeometryDirectly( poFeature, mGeom2 );
  }
  else
  {
    OGRErr err = OGR_G_ImportFromWkb( mGeom, geom->asWkb(), geom->wkbSize() );
    if ( err != OGRERR_NONE )
    {
      QgsDebugMsg( "Failed to import geometry from WKB: " + QString::number( err ) );
      OGR_F_Destroy( poFeature );
      return false;
    }

    // set geometry (ownership is not passed to OGR)
    OGR_F_SetGeometry( poFeature, mGeom );
  }

  // put the created feature to layer
  if ( OGR_L_CreateFeature( mLayer, poFeature ) != OGRERR_NONE )
  {
    QgsDebugMsg( "Failed to create feature in shapefile" );
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
                                       QString *errorMessage )
{
  return writeAsVectorFormat( layer, shapefileName, fileEncoding, destCRS, "ESRI Shapefile", onlySelected, errorMessage );
}

QgsVectorFileWriter::WriterError
QgsVectorFileWriter::writeAsVectorFormat( QgsVectorLayer* layer,
    const QString& fileName,
    const QString& fileEncoding,
    const QgsCoordinateReferenceSystem *destCRS,
    const QString& driverName,
    bool onlySelected,
    QString *errorMessage )
{
  const QgsCoordinateReferenceSystem* outputCRS;
  QgsCoordinateTransform* ct = 0;
  int shallTransform = false;

  if ( destCRS && destCRS->isValid() )
  {
    // This means we should transform
    outputCRS = destCRS;
    shallTransform = true;
  }
  else
  {
    // This means we shouldn't transform, use source CRS as output (if defined)
    outputCRS = &layer->srs();
  }
  QgsVectorFileWriter* writer =
    new QgsVectorFileWriter( fileName, fileEncoding, layer->pendingFields(), layer->wkbType(), outputCRS, driverName );

  // check whether file creation was successful
  WriterError err = writer->hasError();
  if ( err != NoError )
  {
    if ( errorMessage )
      *errorMessage = writer->errorMessage();
    delete writer;
    return err;
  }

  QgsAttributeList allAttr = layer->pendingAllAttributesList();
  QgsFeature fet;

  layer->select( allAttr, QgsRectangle(), true );

  const QgsFeatureIds& ids = layer->selectedFeaturesIds();

  // Create our transform
  if ( destCRS )
  {
    ct = new QgsCoordinateTransform( layer->srs(), *destCRS );
  }

  // Check for failure
  if ( ct == NULL )
  {
    shallTransform = false;
  }

  // write all features
  while ( layer->nextFeature( fet ) )
  {
    if ( onlySelected && !ids.contains( fet.id() ) )
      continue;

    if ( shallTransform )
    {
      try
      {
        fet.geometry()->transform( *ct );
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
    writer->addFeature( fet );
  }

  delete writer;

  if ( shallTransform )
  {
    delete ct;
  }

  return NoError;
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
        QPair<QString, QString> p = nameAndGlob( drvName );
        if ( p.first.isEmpty() )
          continue;

        resultMap.insert( p.first, drvName );
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
  QPair<QString, QString> p = nameAndGlob( driverName );

  if ( p.first.isEmpty() || p.second.isEmpty() )
    return "";

  return "[OGR] " + p.first + " (" + p.second.toLower() + " " + p.second.toUpper() + ")";
}

QPair<QString, QString> QgsVectorFileWriter::nameAndGlob( QString driverName )
{
  QString longName;
  QString glob;

  if ( driverName.startsWith( "AVCE00" ) )
  {
    longName = "Arc/Info ASCII Coverage";
    glob = "*.e00";
  }
  else if ( driverName.startsWith( "BNA" ) )
  {
    longName = "Atlas BNA";
    glob = "*.bna";
  }
  else if ( driverName.startsWith( "CSV" ) )
  {
    longName = "Comma Separated Value";
    glob = "*.csv";
  }
  else if ( driverName.startsWith( "ESRI" ) )
  {
    longName = "ESRI Shapefile";
    glob = "*.shp";
  }
  else if ( driverName.startsWith( "FMEObjects Gateway" ) )
  {
    longName = "FMEObjects Gateway";
    glob = "*.fdd";
  }
  else if ( driverName.startsWith( "GeoJSON" ) )
  {
    longName = "GeoJSON";
    glob = "*.geojson";
  }
  else if ( driverName.startsWith( "GeoRSS" ) )
  {
    longName = "GeoRSS";
    glob = "*.xml";
  }
  else if ( driverName.startsWith( "GML" ) )
  {
    longName = "Geography Markup Language";
    glob = "*.gml";
  }
  else if ( driverName.startsWith( "GMT" ) )
  {
    longName = "Generic Mapping Tools";
    glob = "*.gmt";
  }
  else if ( driverName.startsWith( "GPX" ) )
  {
    longName = "GPS eXchange Format";
    glob = "*.gpx";
  }
  else if ( driverName.startsWith( "Interlis 1" ) )
  {
    longName = "INTERLIS 1";
    glob = "*.itf *.xml *.ili";
  }
  else if ( driverName.startsWith( "Interlis 2" ) )
  {
    longName = "INTERLIS 2";
    glob = "*.itf *.xml *.ili";
  }
  else if ( driverName.startsWith( "KML" ) )
  {
    longName = "Keyhole Markup Language";
    glob = "*.kml" ;
  }
  else if ( driverName.startsWith( "MapInfo File" ) )
  {
    longName = "Mapinfo File";
    glob = "*.mif *.tab";
  }
  else if ( driverName.startsWith( "DGN" ) )
  {
    longName = "Microstation DGN";
    glob = "*.dgn";
  }
  else if ( driverName.startsWith( "S57" ) )
  {
    longName = "S-57 Base file";
    glob = "*.000";
  }
  else if ( driverName.startsWith( "SDTS" ) )
  {
    longName = "Spatial Data Transfer Standard";
    glob = "*catd.ddf";
  }
  else if ( driverName.startsWith( "SQLite" ) )
  {
    longName = "SQLite";
    glob = "*.sqlite";
  }
  else if ( driverName.startsWith( "DXF" ) )
  {
    longName = "AutoCAD DXF";
    glob = "*.dxf";
  }
  else if ( driverName.startsWith( "Geoconcept" ) )
  {
    longName = "Geoconcept";
    glob = "*.gxt *.txt";
  }

  return QPair<QString, QString>( longName, glob );
}

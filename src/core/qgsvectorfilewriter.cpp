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
#include "qgsvectordataprovider.h"

#include <QFile>
#include <QSettings>
#include <QFileInfo>
#include <QTextCodec>
#include <QTextStream>

#include <cassert>
#include <cstdlib> // size_t

#include <ogr_api.h>
#include <ogr_srs_api.h>


QgsVectorFileWriter::QgsVectorFileWriter( const QString& shapefileName,
    const QString& fileEncoding,
    const QgsFieldMap& fields,
    QGis::WkbType geometryType,
    const QgsCoordinateReferenceSystem* srs,
    const QString& driverName )
    : mDS( NULL ), mLayer( NULL ), mGeom( NULL ), mError( NoError )
{
  // save the layer as a shapefile

  // find driver in OGR
  OGRSFDriverH poDriver;
  QgsApplication::registerOgrDrivers();
  poDriver = OGRGetDriverByName( driverName.toLocal8Bit().data() );

  if ( poDriver == NULL )
  {
    mError = ErrDriverNotFound;
    return;
  }

  // create the data source
  mDS = OGR_Dr_CreateDataSource( poDriver, shapefileName.toLocal8Bit().data(), NULL );
  if ( mDS == NULL )
  {
    mError = ErrCreateDataSource;
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
  QString layerName = shapefileName.left( shapefileName.lastIndexOf( ".shp", Qt::CaseInsensitive ) );
  OGRwkbGeometryType wkbType = static_cast<OGRwkbGeometryType>( geometryType );
  mLayer = OGR_DS_CreateLayer( mDS, QFile::encodeName( layerName ).data(), ogrRef, wkbType, NULL );

  if ( srs )
  {
    if ( shapefileName != layerName )
    {
      QFile prjFile( layerName + ".prj" );
      if ( prjFile.open( QIODevice::WriteOnly ) )
      {
        QTextStream prjStream( &prjFile );
        prjStream << srs->toWkt().toLocal8Bit().data() << endl;
        prjFile.close();
      }
      else
      {
        QgsDebugMsg( "Couldn't open file " + layerName + ".prj" );
      }
    }

    OSRDestroySpatialReference( ogrRef );
  }

  if ( mLayer == NULL )
  {
    mError = ErrCreateLayer;
    return;
  }

  QgsDebugMsg( "created layer" );

  // create the fields
  QgsDebugMsg( "creating " + QString::number( fields.size() ) + " fields" );

  mFields = fields;
  mAttrIdxToOgrIdx.clear();

  int ogrIdx = 0;
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
    if ( OGR_L_CreateField( mLayer, fld, TRUE ) != OGRERR_NONE )
    {
      QgsDebugMsg( "error creating field " + attrField.name() );
      mError = ErrAttributeCreationFailed;
      return;
    }

    mAttrIdxToOgrIdx.insert( fldIt.key(), ogrIdx++ );
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
        QgsDebugMsg( "Invalid variant type for field " + QString::number( ogrField ) + ": " + QString::number( attrValue.type() ) );
        return false;
    }
  }

  // build geometry from WKB
  QgsGeometry *geom = feature.geometry();

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
                                       bool onlySelected )
{

  const QgsCoordinateReferenceSystem* outputCRS;
  QgsCoordinateTransform* ct = 0;

  QgsVectorDataProvider* provider = layer->dataProvider();
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
    new QgsVectorFileWriter( shapefileName, fileEncoding, provider->fields(), provider->geometryType(), outputCRS );

  // check whether file creation was successful
  WriterError err = writer->hasError();
  if ( err != NoError )
  {
    delete writer;
    return err;
  }

  QgsAttributeList allAttr = provider->attributeIndexes();
  QgsFeature fet;

  provider->select( allAttr, QgsRectangle(), true );

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
  while ( provider->nextFeature( fet ) )
  {
    if ( onlySelected && !ids.contains( fet.id() ) )
      continue;

    if ( shallTransform )
    {
      fet.geometry()->transform( *ct );
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
  //
  // Remove old copies that may be lying around
  // TODO: should be case-insensitive
  //
  QString myFileBase = theFileName.replace( ".shp", "" );
  bool ok = TRUE;

  const char* suffixes[] = { ".shp", ".shx", ".dbf", ".prj", ".qix" };
  for ( std::size_t i = 0; i < sizeof( suffixes ) / sizeof( char* ); i++ )
  {
    QString file = myFileBase + suffixes[i];
    QFileInfo myInfo( file );
    if ( myInfo.exists() )
    {
      if ( !QFile::remove( file ) )
      {
        QgsDebugMsg( "Removing file failed : " + file );
        ok = FALSE;
      }
    }
  }

  return ok;
}

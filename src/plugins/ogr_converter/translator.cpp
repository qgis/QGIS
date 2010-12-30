// $Id$
//////////////////////////////////////////////////////////////////////////////
//
// Copyright (C) 2008 by Mateusz Loskot <mateusz@loskot.net>
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License,
// or (at your option) any later version.
//
//////////////////////////////////////////////////////////////////////////////

// qgis::plugin::ogrconv
#include "translator.h"
// QGIS
#include <qgsapplication.h>
#include <qgslogger.h>
// Qt4
#include <QString>

#include <ogr_api.h>
#include <cpl_error.h>

Translator::Translator()
    : mDstUpdate( false ), mDstLayerOverwrite( true )
{
}

Translator::Translator( QString const& src, QString const& dst, QString const& format )
    : mSrcUrl( src ), mDstUrl( dst ), mDstFormat( format ),
    mDstUpdate( false ), mDstLayerOverwrite( true )
{
}

QString const& Translator::targetFormat() const
{
  return mDstFormat;
}

void Translator::setTargetFormat( QString const& format )
{
  mDstFormat = format;
}

QString const& Translator::targetLayer() const
{
  return mDstLayer;
}

void Translator::setTargetLayer( QString const& layer )
{
  mDstLayer = layer;
}

QString const& Translator::sourceLayer() const
{
  return mSrcLayer;
}

void Translator::setSourceLayer( QString const& layer )
{
  mSrcLayer = layer;
}

QString const& Translator::targetReferenceSystem() const
{
  return mDstSrs;
}

void Translator::setTargetReferenceSystem( QString const& srs )
{
  mDstSrs = srs;
}

QString const& Translator::sourceReferenceSystem() const
{
  return mSrcSrs;
}

void Translator::setSourceReferenceSystem( QString const& srs )
{
  mSrcSrs = srs;
}

bool Translator::isTargetUpdate() const
{
  return mDstUpdate;
}

void Translator::setUpdateTarget( bool update )
{
  mDstUpdate = update;
}

bool Translator::isTargetLayerOverwrite() const
{
  return mDstLayerOverwrite;
}

bool Translator::translate()
{
  bool success = false;

  // TODO: RAII for OGR handlers!!!

  // Open input data source
  OGRDataSourceH srcDs = openDataSource( mSrcUrl, true );
  if ( 0 == srcDs )
  {
    QgsDebugMsg( "Open source failed: " + mSrcUrl );
    return false;
  }

  // Open output data source
  OGRDataSourceH dstDs = openDataTarget( mDstUrl, mDstUpdate );
  if ( 0 == dstDs )
  {
    QgsDebugMsg( "Open target failed: " + mDstUrl );
    OGR_DS_Destroy( srcDs );
    return false;
  }

  // TODO: Support translation of all layers from input data source
  //for (int i = 0; i < OGR_DS_GetLayerCount(); ++i)

  OGRLayerH srcLayer = OGR_DS_GetLayerByName( srcDs, mSrcLayer.toAscii().constData() );
  if ( 0 == srcLayer )
  {
    QgsDebugMsg( "Can not find layer: " + mSrcLayer );
    OGR_DS_Destroy( srcDs );
    OGR_DS_Destroy( dstDs );
    return false;
  }

  if ( mDstLayer.isEmpty() )
  {
    QgsDebugMsg( "Using source layer name: " + mDstLayer );
    mDstLayer = mSrcLayer;
  }

  QgsDebugMsg( "START LAYER TRANSLATION ------" );

  success = translateLayer( srcDs, srcLayer, dstDs );

  QgsDebugMsg( "END LAYER TRANSLATION ------" );

  OGR_DS_Destroy( dstDs );
  OGR_DS_Destroy( srcDs );

  return success;
}

bool Translator::translateLayer( OGRDataSourceH srcDs, OGRLayerH srcLayer, OGRDataSourceH dstDs )
{
  // Implementation based on TranslateLayer function from ogr2ogr.cpp, from GDAL/OGR.
  Q_ASSERT( 0 != srcDs );
  Q_ASSERT( 0 != srcLayer );
  Q_ASSERT( 0 != dstDs );

  bool success = false;

  // Get source layer schema
  OGRFeatureDefnH srcLayerDefn = OGR_L_GetLayerDefn( srcLayer );
  Q_ASSERT( 0 != srcLayerDefn );

  // Find if layer exists in target data source
  int dstLayerIndex = 0;
  OGRLayerH dstLayer = findLayer( dstDs, mDstLayer, dstLayerIndex );

  // If the user requested overwrite, and we have the layer in question
  // we need to delete it now so it will get recreated overwritten
  if ( 0 != dstLayer && mDstLayerOverwrite
       && 0 != OGR_DS_TestCapability( dstDs, ODsCDeleteLayer ) )
  {
    if ( OGRERR_NONE != OGR_DS_DeleteLayer( dstDs, dstLayerIndex ) )
    {
      QgsDebugMsg( "Delete layer failed when overwrite requested" );
      return false;
    }
  }

  if ( 0 == dstLayer )
  {
    QgsDebugMsg( "Destination layer not found, will attempt to create" );

    // If the layer does not exist, then create it
    if ( 0 == OGR_DS_TestCapability( dstDs, ODsCCreateLayer ) )
    {
      QgsDebugMsg( "Layer " + mDstLayer + " not found, and CreateLayer not supported by driver" );
      return false;
    }

    // FIXME: Do we need it here?
    //CPLErrorReset();

    // TODO: -nlt option support
    OGRwkbGeometryType geomType = OGR_FD_GetGeomType( srcLayerDefn );

    // TODO: Implement SRS transformation
    OGRSpatialReferenceH dstLayerSrs = OGR_L_GetSpatialRef( srcLayer );

    dstLayer = OGR_DS_CreateLayer( dstDs, mDstLayer.toAscii().constData(),
                                   dstLayerSrs, geomType, 0 );
  }

  if ( 0 == dstLayer )
  {
    qWarning( QString( "Layer %1 not found and CreateLayer failed [OGR: %2]\n" ).arg( mDstLayer ).arg( CPLGetLastErrorMsg() ).toUtf8() );
    return false;
  }

  // TODO: Append and createion options not implemented
  // else if (!mDstLayerAppend)

  Q_ASSERT( 0 != dstLayer );

  // Transfer attributes schema
  if ( !copyFields( srcLayerDefn, dstLayer ) )
  {
    QgsDebugMsg( "Faild to copy fields from layer " + mSrcLayer );
    return false;
  }

  // Transfer features
  success = copyFeatures( srcLayer, dstLayer );

  return success;
}

bool Translator::copyFields( OGRFeatureDefnH layerDefn, OGRLayerH layer )
{
  Q_ASSERT( 0 != layerDefn );
  Q_ASSERT( 0 != layer );

  int const count = OGR_FD_GetFieldCount( layerDefn );
  for ( int i = 0; i < count; ++i )
  {
    OGRFieldDefnH fieldDefn = OGR_FD_GetFieldDefn( layerDefn, i );
    Q_ASSERT( 0 != fieldDefn );

    if ( OGRERR_NONE != OGR_L_CreateField( layer, fieldDefn, true ) )
    {
      return false;
    }
  }

  return true;
}

bool Translator::copyFeatures( OGRLayerH srcLayer, OGRLayerH dstLayer )
{
  Q_ASSERT( 0 != srcLayer );
  Q_ASSERT( 0 != dstLayer );

  bool success = false;
  OGRFeatureDefnH srcLayerDefn = OGR_L_GetLayerDefn( srcLayer );
  long srcFid = 0;
  long count = 0;

  // TODO: RAII for feature handlers!!!

  while ( true )
  {
    OGRFeatureH srcFeat = OGR_L_GetNextFeature( srcLayer );
    if ( 0 == srcFeat )
    {
      QgsDebugMsg( "Next source feature is null, finishing" );
      success = true;
      break;
    }
    srcFid = OGR_F_GetFID( srcFeat );

    // FIXME: Do we need it here?
    //CPLErrorReset();

    OGRFeatureH dstFeat = OGR_F_Create( srcLayerDefn );

    if ( OGRERR_NONE !=  OGR_F_SetFrom( dstFeat, srcFeat, true ) )
    {
      QString msg = QString( "Unable to translate feature %1 from layer %2" ).arg( srcFid ).arg( mSrcLayer );
      QgsDebugMsg( msg );

      OGR_F_Destroy( srcFeat );
      OGR_F_Destroy( dstFeat );
      success = false;
      break;
    }
    Q_ASSERT( 0 != dstFeat );

    // TODO: Transform feature geometry

    OGR_F_Destroy( srcFeat );

    // FIXME: Do we need it here?
    //CPLErrorReset();

    // TODO: Skip failures support
    if ( OGRERR_NONE != OGR_L_CreateFeature( dstLayer, dstFeat ) )
    {
      QgsDebugMsg( "Feature creation failed" );
      OGR_F_Destroy( dstFeat );
      success = false;
      break;
    }

    OGR_F_Destroy( dstFeat );

    count += 1;
    success = true;
  }

  QgsDebugMsg( QString( "Number of copied features: %1" ).arg( count ) );

  return success;
}

OGRSFDriverH Translator::findDriver( QString const& name )
{
  QgsApplication::registerOgrDrivers();
  int const drvCount = OGRGetDriverCount();

  OGRSFDriverH drv = 0;
  QString drvName;

  for ( int i = 0; i < drvCount; ++i )
  {
    OGRSFDriverH drvTmp = OGRGetDriver( i );
    Q_CHECK_PTR( drvTmp );
    if ( 0 != drvTmp )
    {
      drvName = OGR_Dr_GetName( drvTmp );
      if ( name == drvName
           && 0 != OGR_Dr_TestCapability( drvTmp, ODrCCreateDataSource ) )
      {
        QgsDebugMsg( "Driver found: " + name );
        drv = drvTmp;
        break;
      }
    }
  }

  return drv;
}

OGRLayerH Translator::findLayer( OGRDataSourceH ds, QString const& name, int& index )
{
  if ( 0 == ds )
  {
    index = -1;
    return 0;
  }

  OGRLayerH lyr = 0;
  int const count = OGR_DS_GetLayerCount( ds );

  for ( int i = 0; i < count; ++i )
  {
    OGRLayerH lyrTmp = OGR_DS_GetLayer( ds, i );
    if ( 0 != lyrTmp )
    {
      OGRFeatureDefnH defn = OGR_L_GetLayerDefn( lyrTmp );
      Q_ASSERT( 0 != defn );

      QString nameTmp( OGR_FD_GetName( defn ) );
      if ( name == nameTmp )
      {
        QgsDebugMsg( "Layer found: " + nameTmp );
        index = i;
        lyr = lyrTmp;
        break;
      }
    }
  }

  return lyr;
}

OGRDataSourceH Translator::openDataSource( QString const& url, bool readOnly )
{
  OGRDataSourceH ds = OGROpen( url.toAscii().constData(), !readOnly, 0 );
  if ( 0 == ds )
  {
    QgsDebugMsg( "Failed to open: " + url );
  }

  return ds;
}

OGRDataSourceH Translator::openDataTarget( QString const& url, bool update )
{
  OGRDataSourceH ds = 0;

  if ( update )
  {
    // Try opening the output datasource as an existing, writable
    ds = openDataSource( url, false );
  }
  else
  {
    // Find the output driver
    OGRSFDriverH drv = findDriver( mDstFormat );
    if ( 0 == drv )
    {
      QgsDebugMsg( "Could not find driver: " + mDstFormat );
      return 0;
    }

    // Create the output data source
    //
    // TODO: Add support for creation options
    ds = OGR_Dr_CreateDataSource( drv, url.toAscii().constData(), 0 );
    if ( 0 == ds )
    {
      QgsDebugMsg( "Failed to open: " + url );
    }
  }

  return ds;
}

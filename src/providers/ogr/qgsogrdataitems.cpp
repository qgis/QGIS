/***************************************************************************
                             qgsogrdataitems.cpp
                             -------------------
    begin                : 2011-04-01
    copyright            : (C) 2011 Radim Blazek
    email                : radim dot blazek at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsogrdataitems.h"

#include "qgslogger.h"
#include "qgsmessagelog.h"

#include <QFileInfo>
#include <QTextStream>
#include <QSettings>

#include <ogr_srs_api.h>
#include <cpl_error.h>
#include <cpl_conv.h>

// these are defined in qgsogrprovider.cpp
QGISEXTERN QStringList fileExtensions();
QGISEXTERN QStringList wildcards();


QgsOgrLayerItem::QgsOgrLayerItem( QgsDataItem* parent,
                                  QString name, QString path, QString uri, LayerType layerType )
    : QgsLayerItem( parent, name, path, uri, layerType, "ogr" )
{
  mToolTip = uri;
  mPopulated = true; // children are not expected
}

QgsOgrLayerItem::~QgsOgrLayerItem()
{
}

QgsLayerItem::Capability QgsOgrLayerItem::capabilities()
{
  QgsDebugMsg( "mPath = " + mPath );
  OGRRegisterAll();
  OGRSFDriverH hDriver;
  OGRDataSourceH hDataSource = OGROpen( TO8F( mPath ), true, &hDriver );

  if ( !hDataSource )
    return NoCapabilities;

  QString  driverName = OGR_Dr_GetName( hDriver );
  OGR_DS_Destroy( hDataSource );

  if ( driverName == "ESRI Shapefile" )
    return SetCrs;

  return NoCapabilities;
}

bool QgsOgrLayerItem::setCrs( QgsCoordinateReferenceSystem crs )
{
  QgsDebugMsg( "mPath = " + mPath );
  OGRRegisterAll();
  OGRSFDriverH hDriver;
  OGRDataSourceH hDataSource = OGROpen( TO8F( mPath ), true, &hDriver );

  if ( !hDataSource )
    return false;

  QString  driverName = OGR_Dr_GetName( hDriver );
  OGR_DS_Destroy( hDataSource );

  // we are able to assign CRS only to shapefiles :-(
  if ( driverName == "ESRI Shapefile" )
  {
    QString layerName = mPath.left( mPath.indexOf( ".shp", Qt::CaseInsensitive ) );
    QString wkt = crs.toWkt();

    // save ordinary .prj file
    OGRSpatialReferenceH hSRS = OSRNewSpatialReference( wkt.toLocal8Bit().data() );
    OSRMorphToESRI( hSRS ); // this is the important stuff for shapefile .prj
    char* pszOutWkt = NULL;
    OSRExportToWkt( hSRS, &pszOutWkt );
    QFile prjFile( layerName + ".prj" );
    if ( prjFile.open( QIODevice::WriteOnly ) )
    {
      QTextStream prjStream( &prjFile );
      prjStream << pszOutWkt << endl;
      prjFile.close();
    }
    else
    {
      QgsMessageLog::logMessage( tr( "Couldn't open file %1.prj" ).arg( layerName ), tr( "OGR" ) );
      return false;
    }
    OSRDestroySpatialReference( hSRS );
    CPLFree( pszOutWkt );

    // save qgis-specific .qpj file (maybe because of better wkt compatibility?)
    QFile qpjFile( layerName + ".qpj" );
    if ( qpjFile.open( QIODevice::WriteOnly ) )
    {
      QTextStream qpjStream( &qpjFile );
      qpjStream << wkt.toLocal8Bit().data() << endl;
      qpjFile.close();
    }
    else
    {
      QgsMessageLog::logMessage( tr( "Couldn't open file %1.qpj" ).arg( layerName ), tr( "OGR" ) );
      return false;
    }

    return true;
  }

  // It it is impossible to assign a crs to an existing layer
  // No OGR_L_SetSpatialRef : http://trac.osgeo.org/gdal/ticket/4032
  return false;
}

QString QgsOgrLayerItem::layerName() const
{
  QFileInfo info( name() );
  if ( info.suffix() == "gz" )
    return info.baseName();
  else
    return info.completeBaseName();
}

// -------

static QgsOgrLayerItem* dataItemForLayer( QgsDataItem* parentItem, QString name, QString path, OGRDataSourceH hDataSource, int layerId )
{
  OGRLayerH hLayer = OGR_DS_GetLayer( hDataSource, layerId );
  OGRFeatureDefnH hDef = OGR_L_GetLayerDefn( hLayer );

  QgsLayerItem::LayerType layerType = QgsLayerItem::Vector;
  int ogrType = QgsOgrProvider::getOgrGeomType( hLayer );
  switch ( ogrType )
  {
    case wkbUnknown:
    case wkbGeometryCollection:
      break;
    case wkbNone:
      layerType = QgsLayerItem::TableLayer;
      break;
    case wkbPoint:
    case wkbMultiPoint:
    case wkbPoint25D:
    case wkbMultiPoint25D:
      layerType = QgsLayerItem::Point;
      break;
    case wkbLineString:
    case wkbMultiLineString:
    case wkbLineString25D:
    case wkbMultiLineString25D:
      layerType = QgsLayerItem::Line;
      break;
    case wkbPolygon:
    case wkbMultiPolygon:
    case wkbPolygon25D:
    case wkbMultiPolygon25D:
      layerType = QgsLayerItem::Polygon;
      break;
    default:
      break;
  }

  QgsDebugMsg( QString( "ogrType = %1 layertype = %2" ).arg( ogrType ).arg( layerType ) );

  QString layerUri = path;

  if ( name.isEmpty() )
  {
    // we are in a collection
    name = FROM8( OGR_FD_GetName( hDef ) );
    QgsDebugMsg( "OGR layer name : " + name );

    layerUri += "|layerid=" + QString::number( layerId );

    path += "/" + name;
  }

  QgsDebugMsg( "OGR layer uri : " + layerUri );

  return new QgsOgrLayerItem( parentItem, name, path, layerUri, layerType );
}

// ----

QgsOgrDataCollectionItem::QgsOgrDataCollectionItem( QgsDataItem* parent, QString name, QString path )
    : QgsDataCollectionItem( parent, name, path )
{
}

QgsOgrDataCollectionItem::~QgsOgrDataCollectionItem()
{
}

QVector<QgsDataItem*> QgsOgrDataCollectionItem::createChildren()
{
  QVector<QgsDataItem*> children;

  OGRSFDriverH hDriver;
  OGRDataSourceH hDataSource = OGROpen( TO8F( mPath ), false, &hDriver );
  if ( !hDataSource )
    return children;
  int numLayers = OGR_DS_GetLayerCount( hDataSource );

  for ( int i = 0; i < numLayers; i++ )
  {
    QgsOgrLayerItem* item = dataItemForLayer( this, QString(), mPath, hDataSource, i );
    children.append( item );
  }

  OGR_DS_Destroy( hDataSource );

  return children;
}

// ---------------------------------------------------------------------------

QGISEXTERN int dataCapabilities()
{
  return  QgsDataProvider::File | QgsDataProvider::Dir;
}

QGISEXTERN QgsDataItem * dataItem( QString thePath, QgsDataItem* parentItem )
{
  if ( thePath.isEmpty() )
    return 0;

  QgsDebugMsg( "thePath: " + thePath );

  // zip settings + info
  QSettings settings;
  QString scanZipSetting = settings.value( "/qgis/scanZipInBrowser", "basic" ).toString();
  QString vsiPrefix = QgsZipItem::vsiPrefix( thePath );
  bool is_vsizip = ( vsiPrefix == "/vsizip/" );
  bool is_vsigzip = ( vsiPrefix == "/vsigzip/" );
  bool is_vsitar = ( vsiPrefix == "/vsitar/" );

  // get suffix, removing .gz if present
  QString tmpPath = thePath; //path used for testing, not for layer creation
  if ( is_vsigzip )
    tmpPath.chop( 3 );
  QFileInfo info( tmpPath );
  QString suffix = info.suffix().toLower();
  // extract basename with extension
  info.setFile( thePath );
  QString name = info.fileName();

  QgsDebugMsg( "thePath= " + thePath + " tmpPath= " + tmpPath + " name= " + name
               + " suffix= " + suffix + " vsiPrefix= " + vsiPrefix );

  // allow only normal files or VSIFILE items to continue
  if ( !info.isFile() && vsiPrefix == "" )
    return 0;

  QStringList myExtensions = fileExtensions();

  // skip *.aux.xml files (GDAL auxilary metadata files) and .shp.xml files (ESRI metadata)
  // unless that extension is in the list (*.xml might be though)
  if ( thePath.endsWith( ".aux.xml", Qt::CaseInsensitive ) &&
       !myExtensions.contains( "aux.xml" ) )
    return 0;
  if ( thePath.endsWith( ".shp.xml", Qt::CaseInsensitive ) &&
       !myExtensions.contains( "shp.xml" ) )
    return 0;

  // We have to filter by extensions, otherwise e.g. all Shapefile files are displayed
  // because OGR drive can open also .dbf, .shx.
  if ( myExtensions.indexOf( suffix ) < 0 )
  {
    bool matches = false;
    foreach( QString wildcard, wildcards() )
    {
      QRegExp rx( wildcard, Qt::CaseInsensitive, QRegExp::Wildcard );
      if ( rx.exactMatch( info.fileName() ) )
      {
        matches = true;
        break;
      }
    }
    if ( !matches )
      return 0;
  }

  // .dbf should probably appear if .shp is not present
  if ( suffix == "dbf" )
  {
    QString pathShp = thePath.left( thePath.count() - 4 ) + ".shp";
    if ( QFileInfo( pathShp ).exists() )
      return 0;
  }

  // fix vsifile path and name
  if ( vsiPrefix != "" )
  {
    // add vsiPrefix to path if needed
    if ( !thePath.startsWith( vsiPrefix ) )
      thePath = vsiPrefix + thePath;
    // if this is a /vsigzip/path_to_zip.zip/file_inside_zip remove the full path from the name
    if (( is_vsizip || is_vsitar ) && ( thePath != vsiPrefix + parentItem->path() ) )
    {
      name = thePath;
      name = name.replace( vsiPrefix + parentItem->path() + "/", "" );
    }
  }

  // return a /vsizip/ item without testing if:
  // zipfile and scan zip == "Basic scan"
  // not zipfile and scan items == "Check extension"
  if ((( is_vsizip || is_vsitar ) && scanZipSetting == "basic" ) ||
      ( !is_vsizip && !is_vsitar &&
        ( settings.value( "/qgis/scanItemsInBrowser",
                          "extension" ).toString() == "extension" ) ) )
  {
    // if this is a VRT file make sure it is vector VRT to avoid duplicates
    if ( suffix == "vrt" )
    {
      OGRSFDriverH hDriver = OGRGetDriverByName( "VRT" );
      if ( hDriver )
      {
        // do not print errors, but write to debug
        CPLPushErrorHandler( CPLQuietErrorHandler );
        CPLErrorReset();
        OGRDataSourceH hDataSource = OGR_Dr_Open( hDriver, thePath.toLocal8Bit().constData(), 0 );
        CPLPopErrorHandler();
        if ( ! hDataSource )
        {
          QgsDebugMsg( "Skipping VRT file because root is not a OGR VRT" );
          return 0;
        }
        OGR_DS_Destroy( hDataSource );
      }
    }
    // add the item
    // TODO: how to handle collections?
    QgsLayerItem * item = new QgsOgrLayerItem( parentItem, name, thePath, thePath, QgsLayerItem::Vector );
    if ( item )
      return item;
  }

  // test that file is valid with OGR
  OGRRegisterAll();
  OGRSFDriverH hDriver;
  // do not print errors, but write to debug
  CPLPushErrorHandler( CPLQuietErrorHandler );
  CPLErrorReset();
  OGRDataSourceH hDataSource = OGROpen( TO8F( thePath ), false, &hDriver );
  CPLPopErrorHandler();

  if ( ! hDataSource )
  {
    QgsDebugMsg( QString( "OGROpen error # %1 : %2 " ).arg( CPLGetLastErrorNo() ).arg( CPLGetLastErrorMsg() ) );
    return 0;
  }

  QString  driverName = OGR_Dr_GetName( hDriver );
  QgsDebugMsg( "OGR Driver : " + driverName );

  int numLayers = OGR_DS_GetLayerCount( hDataSource );

  QgsDataItem* item = 0;

  if ( numLayers == 1 )
  {
    QgsDebugMsg( QString( "using name = %1" ).arg( name ) );
    item = dataItemForLayer( parentItem, name, thePath, hDataSource, 0 );
  }
  else if ( numLayers > 1 )
  {
    QgsDebugMsg( QString( "using name = %1" ).arg( name ) );
    item = new QgsOgrDataCollectionItem( parentItem, name, thePath );
  }

  OGR_DS_Destroy( hDataSource );
  return item;
}

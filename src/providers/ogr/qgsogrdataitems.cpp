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

  QFileInfo info( thePath );
  QString name = info.fileName();
  QSettings settings;
  int scanItemsSetting = settings.value( "/qgis/scanItemsInBrowser", 0 ).toInt();
  int scanZipSetting = settings.value( "/qgis/scanZipInBrowser", 1 ).toInt();

  // allow normal files or VSIFILE items to pass
  if ( !info.isFile() &&
       !thePath.startsWith( "/vsizip/" ) &&
       !thePath.startsWith( "/vsigzip/" ) )
    return 0;

  QStringList myExtensions = fileExtensions();

  // skip *.aux.xml files (GDAL auxilary metadata files) and .shp.xml files (ESRI metadata)
  // unless that extension is in the list (*.xml might be though)
  if ( thePath.right( 8 ).toLower() == ".aux.xml" &&
       myExtensions.indexOf( "aux.xml" ) < 0 )
    return 0;
  if ( thePath.right( 8 ).toLower() == ".shp.xml" &&
       myExtensions.indexOf( "shp.xml" ) < 0 )
    return 0;

  // skip .tar.gz files
  if ( thePath.right( 7 ) == ".tar.gz" )
    return 0;

  // We have to filter by extensions, otherwise e.g. all Shapefile files are displayed
  // because OGR drive can open also .dbf, .shx.
  if ( myExtensions.indexOf( info.suffix().toLower() ) < 0 )
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
  if ( info.suffix().toLower() == "dbf" )
  {
    QString pathShp = thePath.left( thePath.count() - 4 ) + ".shp";
    if ( QFileInfo( pathShp ).exists() )
      return 0;
  }

  // vsifile : depending on options we should just add the item without testing
  if ( thePath.startsWith( "/vsizip/" ) )
  {
    // if this is a /vsigzip/path.zip/file_inside_zip change the name
    if ( thePath != "/vsizip/" + parentItem->path() )
    {
      name = thePath;
      name = name.replace( "/vsizip/" + parentItem->path() + "/", "" );
    }

    // if setting== 2 (Basic scan), return an item without testing
    if ( scanZipSetting == 2 )
    {
      QgsLayerItem * item = new QgsOgrLayerItem( parentItem, name, thePath, thePath, QgsLayerItem::Vector );
      if ( item )
        return item;
    }
  }

  // if scan items == "Check extension", add item here without trying to open
  if ( scanItemsSetting == 1 )
  {
    QgsLayerItem * item = new QgsOgrLayerItem( parentItem, name, thePath, thePath, QgsLayerItem::Vector );
    if ( item )
      return item;
  }

  // try to open using VSIFileHandler
  if ( thePath.endsWith( ".zip", Qt::CaseInsensitive ) )
  {
    if ( !thePath.startsWith( "/vsizip/" ) )
      thePath = "/vsizip/" + thePath;
  }
  else if ( thePath.endsWith( ".gz", Qt::CaseInsensitive ) )
  {
    if ( !thePath.startsWith( "/vsigzip/" ) )
      thePath = "/vsigzip/" + thePath;
  }

  // test that file is valid with OGR
  OGRRegisterAll();
  OGRSFDriverH hDriver;
  // do not print errors, but write to debug
  CPLErrorHandler oErrorHandler = CPLSetErrorHandler( CPLQuietErrorHandler );
  CPLErrorReset();
  OGRDataSourceH hDataSource = OGROpen( TO8F( thePath ), false, &hDriver );
  CPLSetErrorHandler( oErrorHandler );

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

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
#include "qgssettings.h"
#include "qgsproject.h"

#include <QFileInfo>
#include <QTextStream>
#include <QAction>
#include <QMessageBox>

#include <ogr_srs_api.h>
#include <cpl_error.h>
#include <cpl_conv.h>

// these are defined in qgsogrprovider.cpp
QGISEXTERN QStringList fileExtensions();
QGISEXTERN QStringList wildcards();

QGISEXTERN bool deleteLayer( const QString &uri, const QString &errCause );


QgsOgrLayerItem::QgsOgrLayerItem( QgsDataItem *parent,
                                  QString name, QString path, QString uri, LayerType layerType, bool isSubLayer )
  : QgsLayerItem( parent, name, path, uri, layerType, QStringLiteral( "ogr" ) )
{
  mIsSubLayer = isSubLayer;
  mToolTip = uri;
  setState( Populated ); // children are not expected

  OGRRegisterAll();
  OGRSFDriverH hDriver;
  OGRDataSourceH hDataSource = QgsOgrProviderUtils::OGROpenWrapper( mPath.toUtf8().constData(), true, &hDriver );

  if ( hDataSource )
  {
    QString driverName = OGR_Dr_GetName( hDriver );
    OGR_DS_Destroy( hDataSource );

    if ( driverName == QLatin1String( "ESRI Shapefile" ) )
      mCapabilities |= SetCrs;

    // It it is impossible to assign a crs to an existing layer
    // No OGR_L_SetSpatialRef : http://trac.osgeo.org/gdal/ticket/4032
  }
}


bool QgsOgrLayerItem::setCrs( const QgsCoordinateReferenceSystem &crs )
{
  if ( !( mCapabilities & SetCrs ) )
    return false;

  QString layerName = mPath.left( mPath.indexOf( QLatin1String( ".shp" ), Qt::CaseInsensitive ) );
  QString wkt = crs.toWkt();

  // save ordinary .prj file
  OGRSpatialReferenceH hSRS = OSRNewSpatialReference( wkt.toLocal8Bit().data() );
  OSRMorphToESRI( hSRS ); // this is the important stuff for shapefile .prj
  char *pszOutWkt = nullptr;
  OSRExportToWkt( hSRS, &pszOutWkt );
  QFile prjFile( layerName + ".prj" );
  if ( prjFile.open( QIODevice::WriteOnly | QIODevice::Truncate ) )
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
  if ( qpjFile.open( QIODevice::WriteOnly | QIODevice::Truncate ) )
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

QString QgsOgrLayerItem::layerName() const
{
  QFileInfo info( name() );
  if ( info.suffix() == QLatin1String( "gz" ) )
    return info.baseName();
  else
    return info.completeBaseName();
}

#ifdef HAVE_GUI
QList<QAction *> QgsOgrLayerItem::actions()
{
  QList<QAction *> lst;
  // Messages are different for files and tables
  QString message = mIsSubLayer ? QObject::tr( "Delete layer '%1'..." ).arg( mName ) : QObject::tr( "Delete file '%1'..." ).arg( mUri );
  QAction *actionDeleteLayer = new QAction( message, this );
  connect( actionDeleteLayer, &QAction::triggered, this, &QgsOgrLayerItem::deleteLayer );
  lst.append( actionDeleteLayer );
  return lst;
}

void QgsOgrLayerItem::deleteLayer()
{
  // Messages are different for files and tables
  QString title = mIsSubLayer ? QObject::tr( "Delete Layer" ) : QObject::tr( "Delete File" );
  // Check if the layer is in the registry
  const QgsMapLayer *projectLayer = nullptr;
  Q_FOREACH ( const QgsMapLayer *layer, QgsProject::instance()->mapLayers() )
  {
    if ( layer->publicSource() == mUri )
    {
      projectLayer = layer;
    }
  }
  if ( ! projectLayer )
  {
    QString confirmMessage;
    if ( mIsSubLayer )
    {
      confirmMessage = QObject::tr( "Are you sure you want to delete layer '%1' from datasource?" ).arg( mName );
    }
    else
    {
      confirmMessage = QObject::tr( "Are you sure you want to delete file '%1'?" ).arg( mUri );
    }
    if ( QMessageBox::question( nullptr, title,
                                confirmMessage,
                                QMessageBox::Yes | QMessageBox::No, QMessageBox::No ) != QMessageBox::Yes )
      return;

    QString errCause;
    bool res = ::deleteLayer( mUri, errCause );
    if ( !res )
    {
      QMessageBox::warning( nullptr, title, errCause );
    }
    else
    {
      QMessageBox::information( nullptr, title, mIsSubLayer ? tr( "Layer deleted successfully." ) :  tr( "File deleted successfully." ) );
      if ( mParent )
        mParent->refresh();
    }
  }
  else
  {
    QMessageBox::warning( nullptr, title, QObject::tr( "The layer '%1' cannot be deleted because it is in the current project as '%2',"
                          " remove it from the project and retry." ).arg( mName, projectLayer->name() ) );
  }
}
#endif

// -------

static QgsOgrLayerItem *dataItemForLayer( QgsDataItem *parentItem, QString name, QString path, OGRDataSourceH hDataSource, int layerId, bool isSubLayer = false )
{
  OGRLayerH hLayer = OGR_DS_GetLayer( hDataSource, layerId );
  OGRFeatureDefnH hDef = OGR_L_GetLayerDefn( hLayer );

  QgsLayerItem::LayerType layerType = QgsLayerItem::Vector;
  OGRwkbGeometryType ogrType = QgsOgrProvider::getOgrGeomType( hLayer );
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

  QgsDebugMsgLevel( QString( "ogrType = %1 layertype = %2" ).arg( ogrType ).arg( layerType ), 2 );

  QString layerUri = path;

  if ( name.isEmpty() )
  {
    // we are in a collection
    name = QString::fromUtf8( OGR_FD_GetName( hDef ) );
    QgsDebugMsg( "OGR layer name : " + name );

    layerUri += "|layerid=" + QString::number( layerId );

    path += '/' + name;
  }

  QgsDebugMsgLevel( "OGR layer uri : " + layerUri, 2 );

  return new QgsOgrLayerItem( parentItem, name, path, layerUri, layerType, isSubLayer );
}

// ----

QgsOgrDataCollectionItem::QgsOgrDataCollectionItem( QgsDataItem *parent, QString name, QString path )
  : QgsDataCollectionItem( parent, name, path )
{
}

QVector<QgsDataItem *> QgsOgrDataCollectionItem::createChildren()
{
  QVector<QgsDataItem *> children;

  OGRSFDriverH hDriver;
  OGRDataSourceH hDataSource = QgsOgrProviderUtils::OGROpenWrapper( mPath.toUtf8().constData(), false, &hDriver );
  if ( !hDataSource )
    return children;
  int numLayers = OGR_DS_GetLayerCount( hDataSource );

  children.reserve( numLayers );
  for ( int i = 0; i < numLayers; ++i )
  {
    QgsOgrLayerItem *item = dataItemForLayer( this, QString(), mPath, hDataSource, i, true );
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

QGISEXTERN QgsDataItem *dataItem( QString path, QgsDataItem *parentItem )
{
  if ( path.isEmpty() )
    return nullptr;

  QgsDebugMsgLevel( "thePath: " + path, 2 );

  // zip settings + info
  QgsSettings settings;
  QString scanZipSetting = settings.value( QStringLiteral( "qgis/scanZipInBrowser2" ), "basic" ).toString();
  QString vsiPrefix = QgsZipItem::vsiPrefix( path );
  bool is_vsizip = ( vsiPrefix == QLatin1String( "/vsizip/" ) );
  bool is_vsigzip = ( vsiPrefix == QLatin1String( "/vsigzip/" ) );
  bool is_vsitar = ( vsiPrefix == QLatin1String( "/vsitar/" ) );

  // should we check ext. only?
  // check if scanItemsInBrowser2 == extension or parent dir in scanItemsFastScanUris
  // TODO - do this in dir item, but this requires a way to inform which extensions are supported by provider
  // maybe a callback function or in the provider registry?
  bool scanExtSetting = false;
  if ( ( settings.value( QStringLiteral( "qgis/scanItemsInBrowser2" ),
                         "extension" ).toString() == QLatin1String( "extension" ) ) ||
       ( parentItem && settings.value( QStringLiteral( "qgis/scanItemsFastScanUris" ),
                                       QStringList() ).toStringList().contains( parentItem->path() ) ) ||
       ( ( is_vsizip || is_vsitar ) && parentItem && parentItem->parent() &&
         settings.value( QStringLiteral( "qgis/scanItemsFastScanUris" ),
                         QStringList() ).toStringList().contains( parentItem->parent()->path() ) ) )
  {
    scanExtSetting = true;
  }

  // get suffix, removing .gz if present
  QString tmpPath = path; //path used for testing, not for layer creation
  if ( is_vsigzip )
    tmpPath.chop( 3 );
  QFileInfo info( tmpPath );
  QString suffix = info.suffix().toLower();
  // extract basename with extension
  info.setFile( path );
  QString name = info.fileName();

  QgsDebugMsgLevel( "thePath= " + path + " tmpPath= " + tmpPath + " name= " + name
                    + " suffix= " + suffix + " vsiPrefix= " + vsiPrefix, 3 );

  // allow only normal files or VSIFILE items to continue
  if ( !info.isFile() && vsiPrefix == QLatin1String( "" ) )
    return nullptr;

  QStringList myExtensions = fileExtensions();

  // skip *.aux.xml files (GDAL auxiliary metadata files),
  // *.shp.xml files (ESRI metadata) and *.tif.xml files (TIFF metadata)
  // unless that extension is in the list (*.xml might be though)
  if ( path.endsWith( QLatin1String( ".aux.xml" ), Qt::CaseInsensitive ) &&
       !myExtensions.contains( QStringLiteral( "aux.xml" ) ) )
    return nullptr;
  if ( path.endsWith( QLatin1String( ".shp.xml" ), Qt::CaseInsensitive ) &&
       !myExtensions.contains( QStringLiteral( "shp.xml" ) ) )
    return nullptr;
  if ( path.endsWith( QLatin1String( ".tif.xml" ), Qt::CaseInsensitive ) &&
       !myExtensions.contains( QStringLiteral( "tif.xml" ) ) )
    return nullptr;

  // We have to filter by extensions, otherwise e.g. all Shapefile files are displayed
  // because OGR drive can open also .dbf, .shx.
  if ( myExtensions.indexOf( suffix ) < 0 )
  {
    bool matches = false;
    Q_FOREACH ( const QString &wildcard, wildcards() )
    {
      QRegExp rx( wildcard, Qt::CaseInsensitive, QRegExp::Wildcard );
      if ( rx.exactMatch( info.fileName() ) )
      {
        matches = true;
        break;
      }
    }
    if ( !matches )
      return nullptr;
  }

  // .dbf should probably appear if .shp is not present
  if ( suffix == QLatin1String( "dbf" ) )
  {
    QString pathShp = path.left( path.count() - 4 ) + ".shp";
    if ( QFileInfo::exists( pathShp ) )
      return nullptr;
  }

  // fix vsifile path and name
  if ( vsiPrefix != QLatin1String( "" ) )
  {
    // add vsiPrefix to path if needed
    if ( !path.startsWith( vsiPrefix ) )
      path = vsiPrefix + path;
    // if this is a /vsigzip/path_to_zip.zip/file_inside_zip remove the full path from the name
    // no need to change the name I believe
#if 0
    if ( ( is_vsizip || is_vsitar ) && ( path != vsiPrefix + parentItem->path() ) )
    {
      name = path;
      name = name.replace( vsiPrefix + parentItem->path() + '/', "" );
    }
#endif
  }

  // return item without testing if:
  // scanExtSetting
  // or zipfile and scan zip == "Basic scan"
  if ( scanExtSetting ||
       ( ( is_vsizip || is_vsitar ) && scanZipSetting == QLatin1String( "basic" ) ) )
  {
    // if this is a VRT file make sure it is vector VRT to avoid duplicates
    if ( suffix == QLatin1String( "vrt" ) )
    {
      OGRSFDriverH hDriver = OGRGetDriverByName( "VRT" );
      if ( hDriver )
      {
        // do not print errors, but write to debug
        CPLPushErrorHandler( CPLQuietErrorHandler );
        CPLErrorReset();
        OGRDataSourceH hDataSource = OGR_Dr_Open( hDriver, path.toLocal8Bit().constData(), 0 );
        CPLPopErrorHandler();
        if ( ! hDataSource )
        {
          QgsDebugMsgLevel( "Skipping VRT file because root is not a OGR VRT", 2 );
          return nullptr;
        }
        OGR_DS_Destroy( hDataSource );
      }
    }
    // Handle collections
    // Check if the layer has sublayers by comparing the extension
    QgsDataItem *item;
    QStringList multipleLayersExtensions;
    // TODO: add more OGR supported multiple layers formats here!
    multipleLayersExtensions << QLatin1String( "gpkg" ) << QLatin1String( "sqlite" ) << QLatin1String( "db" );
    if ( ! multipleLayersExtensions.contains( suffix ) )
      item = new QgsOgrLayerItem( parentItem, name, path, path, QgsLayerItem::Vector );
    else
      item = new QgsOgrDataCollectionItem( parentItem, name, path );

    if ( item )
      return item;
  }

  // test that file is valid with OGR
  OGRRegisterAll();
  OGRSFDriverH hDriver;
  // do not print errors, but write to debug
  CPLPushErrorHandler( CPLQuietErrorHandler );
  CPLErrorReset();
  OGRDataSourceH hDataSource = QgsOgrProviderUtils::OGROpenWrapper( path.toUtf8().constData(), false, &hDriver );
  CPLPopErrorHandler();

  if ( ! hDataSource )
  {
    QgsDebugMsg( QString( "OGROpen error # %1 : %2 on %3" ).arg( CPLGetLastErrorNo() ).arg( CPLGetLastErrorMsg() ).arg( path ) );
    return nullptr;
  }

  QgsDebugMsgLevel( QString( "OGR Driver : %1" ).arg( OGR_Dr_GetName( hDriver ) ), 2 );

  int numLayers = OGR_DS_GetLayerCount( hDataSource );

  QgsDataItem *item = nullptr;

  if ( numLayers == 1 )
  {
    QgsDebugMsgLevel( QString( "using name = %1" ).arg( name ), 2 );
    item = dataItemForLayer( parentItem, name, path, hDataSource, 0 );
  }
  else if ( numLayers > 1 )
  {
    QgsDebugMsgLevel( QString( "using name = %1" ).arg( name ), 2 );
    item = new QgsOgrDataCollectionItem( parentItem, name, path );
  }

  OGR_DS_Destroy( hDataSource );
  return item;
}

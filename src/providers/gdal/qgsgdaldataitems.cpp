/***************************************************************************
    qgsgdaldataitems.cpp
    ---------------------
    begin                : October 2011
    copyright            : (C) 2011 by Martin Dobias
    email                : wonder.sk at gmail.com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "qgsgdaldataitems.h"
#include "qgsgdalprovider.h"
#include "qgslogger.h"
#include "qgsdatasourceuri.h"
#include "qgswcssourceselect.h"
#include "qgsowsconnection.h"
#include "qgsnewhttpconnection.h"

#include <QFileInfo>
#include <QSettings>

// defined in qgsgdalprovider.cpp
void buildSupportedRasterFileFilterAndExtensions( QString & theFileFiltersString, QStringList & theExtensions, QStringList & theWildcards );


QgsGdalLayerItem::QgsGdalLayerItem( QgsDataItem* parent,
                                    QString name, QString path, QString uri,
                                    QStringList *theSublayers )
    : QgsLayerItem( parent, name, path, uri, QgsLayerItem::Raster, "gdal" )
{
  mToolTip = uri;
  // save sublayers for subsequent access
  // if there are sublayers, set populated=false so item can be populated on demand
  if ( theSublayers && theSublayers->size() > 0 )
  {
    sublayers = *theSublayers;
    mPopulated = false;
  }
  else
    mPopulated = true;
}

QgsGdalLayerItem::~QgsGdalLayerItem()
{
}

QgsLayerItem::Capability QgsGdalLayerItem::capabilities()
{
  // Check if data source can be opened for update
  QgsDebugMsg( "mPath = " + mPath );
  GDALAllRegister();
  GDALDatasetH hDS = GDALOpen( TO8F( mPath ), GA_Update );

  if ( !hDS )
    return NoCapabilities;

  return SetCrs;
}

bool QgsGdalLayerItem::setCrs( QgsCoordinateReferenceSystem crs )
{
  QgsDebugMsg( "mPath = " + mPath );
  GDALAllRegister();
  GDALDatasetH hDS = GDALOpen( TO8F( mPath ), GA_Update );

  if ( !hDS )
    return false;

  QString wkt = crs.toWkt();
  if ( GDALSetProjection( hDS, wkt.toLocal8Bit().data() ) != CE_None )
  {
    QgsDebugMsg( "Could not set CRS" );
    return false;
  }
  GDALClose( hDS );
  return true;
}

QVector<QgsDataItem*> QgsGdalLayerItem::createChildren( )
{
  QgsDebugMsg( "Entered, path=" + path() );
  QVector<QgsDataItem*> children;

  // get children from sublayers
  if ( sublayers.count() > 0 )
  {
    QgsDataItem * childItem = NULL;
    QgsDebugMsg( QString( "got %1 sublayers" ).arg( sublayers.count() ) );
    for ( int i = 0; i < sublayers.count(); i++ )
    {
      QString name = sublayers[i];
      // replace full path with basename+extension
      name.replace( mPath, mName );
      // use subdataset name only - perhaps only if name is long
      if ( name.length() > 50 )
        name = name.split( mName )[1].mid( 2 );
      childItem = new QgsGdalLayerItem( this, name, sublayers[i], sublayers[i] );
      if ( childItem )
        this->addChildItem( childItem );
    }
  }

  return children;
}

QString QgsGdalLayerItem::layerName() const
{
  QFileInfo info( name() );
  if ( info.suffix() == "gz" )
    return info.baseName();
  else
    return info.completeBaseName();
}

// ---------------------------------------------------------------------------
QgsWCSConnectionItem::QgsWCSConnectionItem( QgsDataItem* parent, QString name, QString path )
    : QgsDataCollectionItem( parent, name, path )
{
  mIcon = QIcon( getThemePixmap( "mIconWcs.png" ) );
}

QgsWCSConnectionItem::~QgsWCSConnectionItem()
{
  QgsDebugMsg( "Entered" );
}

QVector<QgsDataItem*> QgsWCSConnectionItem::createChildren()
{
  QgsDebugMsg( "Entered" );
  QVector<QgsDataItem*> children;

  QString encodedUri = mPath;
  QgsDataSourceURI uri;
  uri.setEncodedUri( encodedUri );
  QgsDebugMsg( "encodedUri = " + encodedUri );

  mCapabilities.setUri( uri );

  // Attention: supportedLayers() gives tree leafes, not top level
  if ( !mCapabilities.lastError().isEmpty() )
  {
    //children.append( new QgsErrorItem( this, tr( "Failed to retrieve layers" ), mPath + "/error" ) );
    // TODO: show the error without adding child
    return children;
  }

  foreach( QgsWcsCoverageSummary coverageSummary, mCapabilities.capabilities().contents.coverageSummary )
  {
    // Attention, the name may be empty
    QgsDebugMsg( QString::number( coverageSummary.orderId ) + " " + coverageSummary.identifier + " " + coverageSummary.title );
    QString pathName = coverageSummary.identifier.isEmpty() ? QString::number( coverageSummary.orderId ) : coverageSummary.identifier;

    QgsWCSLayerItem * layer = new QgsWCSLayerItem( this, coverageSummary.title, mPath + "/" + pathName, mCapabilities.capabilities(), uri, coverageSummary );

    children.append( layer );
  }
  return children;
}

bool QgsWCSConnectionItem::equal( const QgsDataItem *other )
{
  if ( type() != other->type() )
  {
    return false;
  }
  const QgsWCSConnectionItem *o = dynamic_cast<const QgsWCSConnectionItem *>( other );
  if ( !o )
  {
    return false;
  }

  return ( mPath == o->mPath && mName == o->mName );
}

QList<QAction*> QgsWCSConnectionItem::actions()
{
  QList<QAction*> lst;

  QAction* actionEdit = new QAction( tr( "Edit..." ), this );
  connect( actionEdit, SIGNAL( triggered() ), this, SLOT( editConnection() ) );
  lst.append( actionEdit );

  QAction* actionDelete = new QAction( tr( "Delete" ), this );
  connect( actionDelete, SIGNAL( triggered() ), this, SLOT( deleteConnection() ) );
  lst.append( actionDelete );

  return lst;
}

void QgsWCSConnectionItem::editConnection()
{
  QgsNewHttpConnection nc( 0, "/Qgis/connections-wcs/", mName );

  if ( nc.exec() )
  {
    // the parent should be updated
    mParent->refresh();
  }
}

void QgsWCSConnectionItem::deleteConnection()
{
  QgsOWSConnection::deleteConnection( "WCS", mName );
  // the parent should be updated
  mParent->refresh();
}


// ---------------------------------------------------------------------------

QgsWCSLayerItem::QgsWCSLayerItem( QgsDataItem* parent, QString name, QString path, QgsWcsCapabilitiesProperty capabilitiesProperty, QgsDataSourceURI dataSourceUri, QgsWcsCoverageSummary coverageSummary )
    : QgsLayerItem( parent, name, path, QString(), QgsLayerItem::Raster, "gdal" ),
    mCapabilities( capabilitiesProperty ),
    mDataSourceUri( dataSourceUri ),
    mCoverageSummary( coverageSummary )
{
  QgsDebugMsg( "uri = " + mDataSourceUri.encodedUri() );
  mUri = createUri();
  // Populate everything, it costs nothing, all info about layers is collected
  foreach( QgsWcsCoverageSummary coverageSummary, mCoverageSummary.coverageSummary )
  {
    // Attention, the name may be empty
    QgsDebugMsg( QString::number( coverageSummary.orderId ) + " " + coverageSummary.identifier + " " + coverageSummary.title );
    QString pathName = coverageSummary.identifier.isEmpty() ? QString::number( coverageSummary.orderId ) : coverageSummary.identifier;
    QgsWCSLayerItem * layer = new QgsWCSLayerItem( this, coverageSummary.title, mPath + "/" + pathName, mCapabilities, mDataSourceUri, coverageSummary );
    mChildren.append( layer );
  }

  if ( mChildren.size() == 0 )
  {
    //mIcon = iconRaster();
    mIcon = QIcon( getThemePixmap( "mIconWcs.png" ) );
  }
  mPopulated = true;
}

QgsWCSLayerItem::~QgsWCSLayerItem()
{
}

QString QgsWCSLayerItem::createUri()
{
  if ( mCoverageSummary.identifier.isEmpty() )
    return ""; // layer collection

  // Number of styles must match number of layers
  mDataSourceUri.setParam( "identifier", mCoverageSummary.identifier );

  // TODO(?): with WCS 1.0 GetCapabilities does not contain CRS and formats,
  // to get them we would need to call QgsWcsCapabilities::describeCoverage
  // but it is problematic to get QgsWcsCapabilities here (copy not allowed
  // by QObject, pointer is dangerous (OWS provider is changing parent))
  // We leave CRS and format default for now.

  QString format;
  // get first supported by GDAL and server
  QStringList mimes = QgsGdalProvider::supportedMimes().keys();
  // prefer tiff
  if ( mimes.contains( "image/tiff" ) && mCoverageSummary.supportedFormat.contains( "image/tiff" ) )
  {
    format = "image/tiff";
  }
  else
  {
    foreach( QString f, mimes )
    {
      if ( mCoverageSummary.supportedFormat.indexOf( f ) >= 0 )
      {
        format = f;
        break;
      }
    }
  }
  if ( !format.isEmpty() )
  {
    mDataSourceUri.setParam( "format", format );
  }

  QString crs;

  // TODO: prefer project CRS
  // get first known if possible
  QgsCoordinateReferenceSystem testCrs;
  foreach( QString c, mCoverageSummary.supportedCrs )
  {
    testCrs.createFromOgcWmsCrs( c );
    if ( testCrs.isValid() )
    {
      crs = c;
      break;
    }
  }
  if ( crs.isEmpty() && mCoverageSummary.supportedCrs.size() > 0 )
  {
    crs = mCoverageSummary.supportedCrs.value( 0 );
  }
  if ( !crs.isEmpty() )
  {
    mDataSourceUri.setParam( "crs", crs );
  }

  return mDataSourceUri.encodedUri();
}

// ---------------------------------------------------------------------------

QgsWCSRootItem::QgsWCSRootItem( QgsDataItem* parent, QString name, QString path )
    : QgsDataCollectionItem( parent, name, path )
{
  mIcon = QIcon( getThemePixmap( "mIconWcs.png" ) );

  populate();
}

QgsWCSRootItem::~QgsWCSRootItem()
{
}

QVector<QgsDataItem*>QgsWCSRootItem::createChildren()
{
  QVector<QgsDataItem*> connections;
  foreach( QString connName, QgsOWSConnection::connectionList( "WCS" ) )
  {
    //QgsDataItem * conn = new QgsWCSConnectionItem( this, connName, mPath + "/" + connName );
    QgsOWSConnection connection( "WCS", connName );
    QgsDataItem * conn = new QgsWCSConnectionItem( this, connName, connection.uri().encodedUri() );

    conn->setIcon( QIcon( getThemePixmap( "mIconConnect.png" ) ) );
    connections.append( conn );
  }
  return connections;
}

QList<QAction*> QgsWCSRootItem::actions()
{
  QList<QAction*> lst;

  QAction* actionNew = new QAction( tr( "New Connection..." ), this );
  connect( actionNew, SIGNAL( triggered() ), this, SLOT( newConnection() ) );
  lst.append( actionNew );

  return lst;
}


QWidget * QgsWCSRootItem::paramWidget()
{
  QgsWCSSourceSelect *select = new QgsWCSSourceSelect( 0, 0, true, true );
  connect( select, SIGNAL( connectionsChanged() ), this, SLOT( connectionsChanged() ) );
  return select;
  return 0;
}
void QgsWCSRootItem::connectionsChanged()
{
  refresh();
}

void QgsWCSRootItem::newConnection()
{
  QgsNewHttpConnection nc( 0, "/Qgis/connections-wcs/" );

  if ( nc.exec() )
  {
    refresh();
  }
}

// ---------------------------------------------------------------------------

static QString filterString;
static QStringList extensions = QStringList();
static QStringList wildcards = QStringList();

QGISEXTERN int dataCapabilities()
{
  return  QgsDataProvider::File | QgsDataProvider::Dir | QgsDataProvider::Net;
}

QGISEXTERN QgsDataItem * dataItem( QString thePath, QgsDataItem* parentItem )
{
  QgsDebugMsg( "thePath = " + thePath );
  if ( thePath.isEmpty() )
  {
    // Top level WCS
    return new QgsWCSRootItem( parentItem, "WCS", "wcs:" );
  }

  if ( thePath.contains( "url=" ) )
  {
    // OWS server
    QgsDebugMsg( "connection found in uri" );
    return new QgsWCSConnectionItem( parentItem, "WCS", thePath );
  }

  // zip settings + info
  QSettings settings;
  QString scanZipSetting = settings.value( "/qgis/scanZipInBrowser", "basic" ).toString();
  bool is_vsizip = ( thePath.startsWith( "/vsizip/" ) ||
                     thePath.endsWith( ".zip", Qt::CaseInsensitive ) );
  bool is_vsigzip = ( thePath.startsWith( "/vsigzip/" ) ||
                      thePath.endsWith( ".gz", Qt::CaseInsensitive ) );

  // get suffix, removing .gz if present
  QString tmpPath = thePath; //path used for testing, not for layer creation
  if ( is_vsigzip )
    tmpPath.chop( 3 );
  QFileInfo info( tmpPath );
  QString suffix = info.suffix().toLower();
  // extract basename with extension
  info.setFile( thePath );
  QString name = info.fileName();

  QgsDebugMsg( "thePath= " + thePath + " tmpPath= " + tmpPath + " name= " + name + " suffix= " + suffix );

  // allow only normal files or VSIFILE items to continue
  if ( !info.isFile() && !is_vsizip && !is_vsigzip )
    return 0;

  // get supported extensions
  if ( extensions.isEmpty() )
  {
    buildSupportedRasterFileFilterAndExtensions( filterString, extensions, wildcards );
    QgsDebugMsg( "extensions: " + extensions.join( " " ) );
    QgsDebugMsg( "wildcards: " + wildcards.join( " " ) );
  }

  // skip *.aux.xml files (GDAL auxilary metadata files)
  // unless that extension is in the list (*.xml might be though)
  if ( thePath.endsWith( ".aux.xml", Qt::CaseInsensitive ) &&
       !extensions.contains( "aux.xml" ) )
    return 0;

  // skip .tar.gz files
  if ( thePath.endsWith( ".tar.gz", Qt::CaseInsensitive ) )
    return 0;

  // Filter files by extension
  if ( !extensions.contains( suffix ) )
  {
    bool matches = false;
    foreach( QString wildcard, wildcards )
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

  // add /vsizip/ or /vsigzip/ to path if file extension is .zip or .gz
  if ( is_vsigzip )
  {
    if ( !thePath.startsWith( "/vsigzip/" ) )
      thePath = "/vsigzip/" + thePath;
  }
  else if ( is_vsizip )
  {
    if ( !thePath.startsWith( "/vsizip/" ) )
      thePath = "/vsizip/" + thePath;
    // if this is a /vsigzip/path_to_zip.zip/file_inside_zip remove the full path from the name
    if ( thePath != "/vsizip/" + parentItem->path() )
    {
      name = thePath;
      name = name.replace( "/vsizip/" + parentItem->path() + "/", "" );
    }
  }

  // return a /vsizip/ item without testing if:
  // zipfile and scan zip == "Basic scan"
  // not zipfile and scan items == "Check extension"
  if (( is_vsizip && scanZipSetting == "basic" ) ||
      ( !is_vsizip && ( settings.value( "/qgis/scanItemsInBrowser",
                                        "extension" ).toString() == "extension" ) ) )
  {
    // if this is a VRT file make sure it is raster VRT to avoid duplicates
    if ( suffix == "vrt" )
    {
      // do not print errors, but write to debug
      CPLPushErrorHandler( CPLQuietErrorHandler );
      CPLErrorReset();
      if ( ! GDALIdentifyDriver( thePath.toLocal8Bit().constData(), 0 ) )
      {
        QgsDebugMsg( "Skipping VRT file because root is not a GDAL VRT" );
        CPLPopErrorHandler();
        return 0;
      }
      CPLPopErrorHandler();
    }
    // add the item
    QStringList sublayers;
    QgsDebugMsg( QString( "adding item name=%1 thePath=%2" ).arg( name ).arg( thePath ) );
    QgsLayerItem * item = new QgsGdalLayerItem( parentItem, name, thePath, thePath, &sublayers );
    if ( item )
      return item;
  }

  // test that file is valid with GDAL
  GDALAllRegister();
  // do not print errors, but write to debug
  CPLPushErrorHandler( CPLQuietErrorHandler );
  CPLErrorReset();
  GDALDatasetH hDS = GDALOpen( TO8F( thePath ), GA_ReadOnly );
  CPLPopErrorHandler();

  if ( ! hDS )
  {
    QgsDebugMsg( QString( "GDALOpen error # %1 : %2 " ).arg( CPLGetLastErrorNo() ).arg( CPLGetLastErrorMsg() ) );
    return 0;
  }

  QStringList sublayers = QgsGdalProvider::subLayers( hDS );

  GDALClose( hDS );

  QgsDebugMsg( "GdalDataset opened " + thePath );

  QgsLayerItem * item = new QgsGdalLayerItem( parentItem, name, thePath, thePath,
      &sublayers );

  return item;
}

QGISEXTERN QgsWCSSourceSelect * selectWidget( QWidget * parent, Qt::WFlags fl )
{
  return new QgsWCSSourceSelect( parent, fl );
}

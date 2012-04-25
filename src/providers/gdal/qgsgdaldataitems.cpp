#include "qgsgdaldataitems.h"
#include "qgsgdalprovider.h"
#include "qgslogger.h"

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

// ---------------------------------------------------------------------------

static QString filterString;
static QStringList extensions = QStringList();
static QStringList wildcards = QStringList();

QGISEXTERN int dataCapabilities()
{
  return  QgsDataProvider::File | QgsDataProvider::Dir;
}

QGISEXTERN QgsDataItem * dataItem( QString thePath, QgsDataItem* parentItem )
{
  if ( thePath.isEmpty() )
    return 0;

  // zip settings + info
  QSettings settings;
  int scanItemsSetting = settings.value( "/qgis/scanItemsInBrowser", 0 ).toInt();
  int scanZipSetting = settings.value( "/qgis/scanZipInBrowser", 1 ).toInt();
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

  QgsDebugMsg( "thePath= " + thePath + " tmpPath= " + tmpPath );

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
  if ( !extensions.contains( info.suffix().toLower() ) )
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

  // if setting = 2 (Basic scan), return a /vsizip/ item without testing
  if ( is_vsizip && scanZipSetting == 2 )
  {
    QStringList sublayers;
    QgsDebugMsg( QString( "adding item name=%1 thePath=%2" ).arg( name ).arg( thePath ) );
    QgsLayerItem * item = new QgsGdalLayerItem( parentItem, name, thePath, thePath, &sublayers );
    if ( item )
      return item;
  }

  // if scan items == "Check extension", add item here without trying to open
  // unless item is /vsizip
  if ( scanItemsSetting == 1 && !is_vsizip )
  {
    QStringList sublayers;
    QgsDebugMsg( QString( "adding item name=%1 thePath=%2" ).arg( name ).arg( thePath ) );
    QgsLayerItem * item = new QgsGdalLayerItem( parentItem, name, thePath, thePath, &sublayers );
    if ( item )
      return item;
  }

  // test that file is valid with GDAL
  GDALAllRegister();
  // do not print errors, but write to debug
  CPLErrorHandler oErrorHandler = CPLSetErrorHandler( CPLQuietErrorHandler );
  CPLErrorReset();
  GDALDatasetH hDS = GDALOpen( TO8F( thePath ), GA_ReadOnly );
  CPLSetErrorHandler( oErrorHandler );

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

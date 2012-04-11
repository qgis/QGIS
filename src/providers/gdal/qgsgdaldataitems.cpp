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

  QgsDebugMsg( "thePath= " + thePath );

  QString uri = thePath;
  QFileInfo info( thePath );
  QSettings settings;
  //extract basename with extension
  QString name = info.fileName();
  int scanItemsSetting = settings.value( "/qgis/scanItemsInBrowser", 0 ).toInt();
  int scanZipSetting = settings.value( "/qgis/scanZipInBrowser", 1 ).toInt();

  // allow normal files or VSIFILE items to pass
  if ( ! info.isFile() &&
       thePath.left( 8 ) != "/vsizip/" &&
       thePath.left( 9 ) != "/vsigzip/" )
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
  if ( thePath.right( 8 ).toLower() == ".aux.xml" &&
       extensions.indexOf( "aux.xml" ) < 0 )
    return 0;

  // skip .tar.gz files
  if ( thePath.right( 7 ) == ".tar.gz" )
    return 0;

  // Filter files by extension
  if ( extensions.indexOf( info.suffix().toLower() ) < 0 )
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

  // vsifile : depending on options we should just add the item without testing
  if ( thePath.left( 8 ) == "/vsizip/" )
  {
    // if this is a /vsigzip/path.zip/file_inside_zip change the name
    if ( thePath != "/vsizip/" + parentItem->path() )
    {
      name = thePath;
      name = name.replace( "/vsizip/" + parentItem->path() + "/", "" );
    }

    // if setting = 2 (Basic scan), return an item without testing
    if ( scanZipSetting == 2 )
    {
      QStringList sublayers;
      QgsDebugMsg( QString( "adding item name=%1 thePath=%2 uri=%3" ).arg( name ).arg( thePath ).arg( uri ) );
      QgsLayerItem * item = new QgsGdalLayerItem( parentItem, name, thePath, thePath, &sublayers );
      if ( item )
        return item;
    }
  }

  // if scan items == "Check extension", add item here without trying to open
  if ( scanItemsSetting == 1 )
  {
    QStringList sublayers;
    QgsDebugMsg( QString( "adding item name=%1 thePath=%2 uri=%3" ).arg( name ).arg( thePath ).arg( uri ) );
    QgsLayerItem * item = new QgsGdalLayerItem( parentItem, name, thePath, thePath, &sublayers );
    if ( item )
      return item;
  }


  // try to open using VSIFileHandler
  if ( thePath.right( 4 ) == ".zip" )
  {
    if ( thePath.left( 8 ) != "/vsizip/" )
      thePath = "/vsizip/" + thePath;
  }
  else if ( thePath.right( 3 ) == ".gz" )
  {
    if ( thePath.left( 9 ) != "/vsigzip/" )
      thePath = "/vsigzip/" + thePath;
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

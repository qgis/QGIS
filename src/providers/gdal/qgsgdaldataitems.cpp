#include "qgsgdaldataitems.h"
#include "qgsgdalprovider.h"
#include "qgslogger.h"

#include <QFileInfo>

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

  QFileInfo info( thePath );
  if ( info.isFile() )
  {
    // get supported extensions
    if ( extensions.isEmpty() )
    {
      QString filterString;
      buildSupportedRasterFileFilterAndExtensions( filterString, extensions, wildcards );
      QgsDebugMsg( "extensions: " + extensions.join( " " ) );
      QgsDebugMsg( "wildcards: " + wildcards.join( " " ) );
    }

    // skip *.aux.xml files (GDAL auxilary metadata files)
    // unless that extension is in the list (*.xml might be though)
    if ( thePath.right( 8 ) == ".aux.xml" &&
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

    // try to open using VSIFileHandler
    // TODO use the file name of the file inside the zip for layer name
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

    GDALAllRegister();
    GDALDatasetH hDS = GDALOpen( TO8F( thePath ), GA_ReadOnly );

    if ( !hDS )
      return 0;

    // get layers list now so we can pass it to item
    QStringList sublayers = QgsGdalProvider::subLayers( hDS );

    GDALClose( hDS );

    QgsDebugMsg( "GdalDataset opened " + thePath );

    //extract basename with extension
    QString name = info.completeBaseName() + "." + info.suffix();
    QString uri = thePath;

    QgsLayerItem * item = new QgsGdalLayerItem( parentItem, name, thePath, uri,
        &sublayers );

    return item;
  }
  return 0;
}

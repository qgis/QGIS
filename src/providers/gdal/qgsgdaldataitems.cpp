#include "qgsgdaldataitems.h"
#include "qgsgdalprovider.h"
#include "qgslogger.h"

#include <QFileInfo>

// defined in qgsgdalprovider.cpp
void buildSupportedRasterFileFilterAndExtensions( QString & theFileFiltersString, QStringList & theExtensions, QStringList & theWildcards );


QgsGdalLayerItem::QgsGdalLayerItem( QgsDataItem* parent,
                                    QString name, QString path, QString uri )
    : QgsLayerItem( parent, name, path, uri, QgsLayerItem::Raster, "gdal" )
{
  mToolTip = uri;
  mPopulated = true; // children are not expected
}

QgsGdalLayerItem::~QgsGdalLayerItem()
{
}

QgsLayerItem::Capability QgsGdalLayerItem::capabilities()
{
  // Check if data sour can be opened for update
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
    // Filter files by extension
    if ( extensions.isEmpty() )
    {
      QString filterString;
      buildSupportedRasterFileFilterAndExtensions( filterString, extensions, wildcards );
      QgsDebugMsg( "extensions: " + extensions.join( " " ) );
      QgsDebugMsg( "wildcards: " + wildcards.join( " " ) );
    }
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

    GDALAllRegister();
    GDALDatasetH hDS = GDALOpen( TO8F( thePath ), GA_ReadOnly );

    if ( !hDS )
      return 0;

    QStringList sublayers = QgsGdalProvider::subLayers( hDS );

    GDALClose( hDS );

    QgsDebugMsg( "GdalDataset opened " + thePath );

    QString name = info.completeBaseName();
    QString uri = thePath;

    QgsLayerItem * item = new QgsGdalLayerItem( parentItem, name, thePath, uri );

    QgsDataItem * childItem = NULL;
    GDALDatasetH hChildDS = NULL;

    if ( item && sublayers.count() > 1 )
    {
      QgsDebugMsg( QString( "dataItem() got %1 sublayers" ).arg( sublayers.count() ) );
      for ( int i = 0; i < sublayers.count(); i++ )
      {
        hChildDS = GDALOpen( TO8F( sublayers[i] ), GA_ReadOnly );
        if ( hChildDS )
        {
          GDALClose( hChildDS );
          QgsDebugMsg( QString( "add child #%1 - %2" ).arg( i ).arg( sublayers[i] ) );
          childItem = new QgsGdalLayerItem( item, sublayers[i], thePath + "/" + sublayers[i], sublayers[i] );
          if ( childItem )
            item->addChildItem( childItem );
        }
      }
    }

    return item;
  }
  return 0;
}

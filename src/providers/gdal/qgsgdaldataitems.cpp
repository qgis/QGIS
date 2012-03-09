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
    // skip *.aux.xml files (GDAL auxilary metadata files)
    // unless that extension is in the list (*.xml might be though)
    if ( thePath.right( 8 ) == ".aux.xml" &&
         extensions.indexOf( "aux.xml" ) < 0 )
      return 0;

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

    //extract basename with extension
    QString name = info.completeBaseName() + "." + QFileInfo( thePath ).suffix();
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

          QString name = sublayers[i];
          //replace full path with basename+extension
          name.replace( thePath, QFileInfo( thePath ).completeBaseName() + "." + QFileInfo( thePath ).suffix() );
          childItem = new QgsGdalLayerItem( item, name, thePath + "/" + name, sublayers[i] );
          if ( childItem )
            item->addChildItem( childItem );
        }
      }
    }

    return item;
  }
  return 0;
}

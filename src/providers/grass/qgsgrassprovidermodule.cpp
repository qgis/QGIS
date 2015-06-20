/***************************************************************************
    qgsgrassprovidermodule.cpp -  Data provider for GRASS format
                     -------------------
    begin                : March, 2004
    copyright            : (C) 2004 by Gary E.Sherman, Radim Blazek
    email                : sherman@mrcc.com, blazek@itc.it
 ***************************************************************************/
/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsmessageoutput.h"
#include "qgsmimedatautils.h"
#include "qgsnewnamedialog.h"
#include "qgsproviderregistry.h"
#include "qgsrasterlayer.h"
#include "qgslogger.h"

#include "qgsgrassprovidermodule.h"
#include "qgsgrassprovider.h"
#include "qgsgrassoptions.h"
#include "qgsgrass.h"

#include <QAction>
#include <QFileInfo>
#include <QDir>
#include <QLabel>
#include <QObject>

//----------------------- QgsGrassItemActions ------------------------------
QgsGrassItemActions* QgsGrassItemActions::instance()
{
  static QgsGrassItemActions mInstance;
  return &mInstance;
}

QList<QAction*> QgsGrassItemActions::actions()
{
  QList<QAction*> lst;
  QAction* actionOptions = new QAction( tr( "GRASS Options" ), this );
  connect( actionOptions, SIGNAL( triggered() ), instance(), SLOT( openOptions() ) );
  lst.append( actionOptions );
  return lst;
}

void QgsGrassItemActions::openOptions()
{
  QgsGrassOptions dialog;
  dialog.exec();
}

//----------------------- QgsGrassLocationItem ------------------------------

QgsGrassLocationItem::QgsGrassLocationItem( QgsDataItem* parent, QString dirPath, QString path )
    : QgsDirectoryItem( parent, "", dirPath, path )
{
  // modify path to distinguish from directory, so that it can be expanded by path in browser
  QDir dir( mDirPath );
  mName = dir.dirName();

  mIconName = "grass_location.png";

  // set Directory type so that when sorted it gets into dirs (after the dir it represents)
  mType = QgsDataItem::Directory;
}

QVector<QgsDataItem*>QgsGrassLocationItem::createChildren()
{
  QVector<QgsDataItem*> mapsets;

  QDir dir( mDirPath );

  QStringList entries = dir.entryList( QDir::Dirs | QDir::NoDotAndDotDot, QDir::Name );
  foreach ( QString name, entries )
  {
    QString path = dir.absoluteFilePath( name );

    if ( QgsGrass::isMapset( path ) )
    {
      QgsGrassMapsetItem * mapset = new QgsGrassMapsetItem( this, path, mPath + "/" + name );
      mapsets.append( mapset );
    }
  }
  return mapsets;
}

QList<QAction*> QgsGrassLocationItem::actions()
{
  return QgsGrassItemActions::instance()->actions();
}

//----------------------- QgsGrassMapsetItem ------------------------------

QList<QgsGrassImport*> QgsGrassMapsetItem::mImports;

QgsGrassMapsetItem::QgsGrassMapsetItem( QgsDataItem* parent, QString dirPath, QString path )
    : QgsDirectoryItem( parent, "", dirPath, path )
{
  QDir dir( mDirPath );
  mName = dir.dirName();
  dir.cdUp();
  mLocation = dir.dirName();
  dir.cdUp();
  mGisdbase = dir.path();

  mIconName = "grass_mapset.png";
}

QVector<QgsDataItem*> QgsGrassMapsetItem::createChildren()
{
  QgsDebugMsg( "Entered" );

  QVector<QgsDataItem*> items;

  QStringList vectorNames = QgsGrass::vectors( mDirPath );

  foreach ( QString name, vectorNames )
  {
    QString mapPath = mPath + "/vector/" + name;
    QStringList layerNames;
    try
    {
      layerNames = QgsGrass::vectorLayers( mGisdbase, mLocation, mName, name );
    }
    catch ( QgsGrass::Exception &e )
    {
      QgsErrorItem * errorItem = new QgsErrorItem( this, name + " : " + e.what(), mapPath );
      items.append( errorItem );
      continue;
    }

    QgsGrassObject vectorObject( mGisdbase, mLocation, mName, name, QgsGrassObject::Vector );
    QgsGrassVectorItem *map = 0;
    if ( layerNames.size() == 0 )
    {
      // TODO: differentiate if it is layer with no layers or without topo (throw exception from QgsGrass::vectorLayers)
      // TODO: refresh (remove) error if topo was build
      QgsErrorItem * errorItem = new QgsErrorItem( this, name, mapPath );
      items.append( errorItem );
      continue;
    }
    else if ( layerNames.size() > 1 )
    {
      //map = new QgsDataCollectionItem( this, name, mapPath );
      //map->setCapabilities( QgsDataItem::NoCapabilities ); // disable fertility
      map = new QgsGrassVectorItem( this, vectorObject, mapPath );
    }
    foreach ( QString layerName, layerNames )
    {
      // don't use QDir::separator(), windows work with '/' and backslash may be lost if
      // somewhere not properly escaped (there was bug in QgsMimeDataUtils for example)
      QString uri = mDirPath + "/" + name + "/" + layerName;
      QgsLayerItem::LayerType layerType = QgsLayerItem::Vector;
      QString typeName = layerName.split( "_" ).value( 1 );
      QString baseLayerName = layerName.split( "_" ).value( 0 );

      if ( typeName == "point" )
        layerType = QgsLayerItem::Point;
      else if ( typeName == "line" )
        layerType = QgsLayerItem::Line;
      else if ( typeName == "polygon" )
        layerType = QgsLayerItem::Polygon;

      QString layerPath = mapPath + "/" + layerName;
      if ( !map )
      {
        /* This may happen (one layer only) in GRASS 7 with points (no topo layers) or if topo layers are disabled */
        QgsLayerItem *layer = new QgsGrassVectorLayerItem( this, vectorObject, name, layerPath, uri, layerType, true );
        //layer->setState( Populated );
        items.append( layer );
      }
      else
      {
        QgsLayerItem *layer = new QgsGrassVectorLayerItem( map, vectorObject, baseLayerName, layerPath, uri, layerType, false );
        map->addChild( layer );
      }
    }
    if ( map )
    {
      map->setState( Populated );
      items.append( map );
    }
  }

  QStringList rasterNames = QgsGrass::rasters( mDirPath );

  foreach ( QString name, rasterNames )
  {
    QString path = mPath + "/" + "raster" + "/" + name;
    QString uri = mDirPath + "/" + "cellhd" + "/" + name;
    QgsDebugMsg( "uri = " + uri );

    QgsGrassObject rasterObject( mGisdbase, mLocation, mName, name, QgsGrassObject::Raster );
    QgsGrassRasterItem *layer = new QgsGrassRasterItem( this, rasterObject, path, uri, QgsGrass::isExternal( rasterObject ) );
    items.append( layer );
  }

  QStringList groupNames = QgsGrass::groups( mDirPath );
  foreach ( QString name, groupNames )
  {
    QString path = mPath + "/" + "group" + "/" + name;
    QString uri = mDirPath + "/" + "group" + "/" + name;
    QgsDebugMsg( "uri = " + uri );

    QgsGrassObject rasterObject( mGisdbase, mLocation, mName, name, QgsGrassObject::Group );
    QgsGrassGroupItem *layer = new QgsGrassGroupItem( this, rasterObject, path, uri );
    items.append( layer );
  }

  QgsGrassObject mapsetObject( mGisdbase, mLocation, mName );
  foreach ( QgsGrassImport* import, mImports )
  {
    if ( !import )
    {
      continue;
    }
    if ( !import->grassObject().mapsetIdentical( mapsetObject ) )
    {
      continue;
    }
    foreach ( QString name, import->names() )
    {
      QString path = mPath + "/" + import->grassObject().elementName() + "/" + name;
      items.append( new QgsGrassImportItem( this, name, path, import ) );
    }
  }

  return items;
}

QList<QAction*> QgsGrassMapsetItem::actions()
{
  return QgsGrassItemActions::instance()->actions();
}

bool QgsGrassMapsetItem::handleDrop( const QMimeData * data, Qt::DropAction )
{
  if ( !QgsMimeDataUtils::isUriList( data ) )
    return false;

  // Init animated icon on main thread
  QgsGrassImportItem::initIcon();

  QSettings settings;

  QgsGrassObject mapsetObject( mGisdbase, mLocation, mName );
  QgsCoordinateReferenceSystem mapsetCrs = QgsGrass::crsDirect( mGisdbase, mLocation );

  QStringList existingRasters = QgsGrass::rasters( mapsetObject.mapsetPath() );
  QStringList existingVectors = QgsGrass::vectors( mapsetObject.mapsetPath() );
  // add currently being imported
  foreach ( QgsGrassImport* import, mImports )
  {
    if ( import && import->grassObject().type() == QgsGrassObject::Raster )
    {
      existingRasters.append( import->names() );
    }
    else if ( import && import->grassObject().type() == QgsGrassObject::Vector )
    {
      existingVectors.append( import->names() );
    }
  }

  QStringList errors;
  QgsMimeDataUtils::UriList lst = QgsMimeDataUtils::decodeUriList( data );
  Qt::CaseSensitivity caseSensitivity = QgsGrass::caseSensitivity();

  foreach ( const QgsMimeDataUtils::Uri& u, lst )
  {
    if ( u.layerType != "raster" && u.layerType != "vector" )
    {
      errors.append( tr( "%1 layer type not supported" ).arg( u.name ) );
      continue;
    }
    QgsRasterDataProvider* rasterProvider = 0;
    QgsVectorDataProvider* vectorProvider = 0;
    QgsDataProvider* provider = 0;
    QStringList extensions;
    QStringList existingNames;
    QRegExp regExp;
    QgsGrassObject srcObject;
    QString srcName;

    // use g.copy for GRASS maps in the same location
    bool useCopy = false;

    if ( u.layerType == "raster" )
    {
      if ( u.providerKey == "grassraster" && srcObject.setFromUri( u.uri )
           && srcObject.locationIdentical( mapsetObject ) )
      {
        useCopy = true;
      }
      else
      {
        rasterProvider = qobject_cast<QgsRasterDataProvider*>( QgsProviderRegistry::instance()->provider( u.providerKey, u.uri ) );
        provider = rasterProvider;
      }
      existingNames = existingRasters;
      regExp = QgsGrassObject::newNameRegExp( QgsGrassObject::Raster );
    }
    else if ( u.layerType == "vector" )
    {
      if ( u.providerKey == "grass" && srcObject.setFromUri( u.uri )
           && srcObject.locationIdentical( mapsetObject ) )
      {
        useCopy = true;
      }
      else
      {
        vectorProvider = qobject_cast<QgsVectorDataProvider*>( QgsProviderRegistry::instance()->provider( u.providerKey, u.uri ) );
        provider = vectorProvider;
      }
      existingNames = existingVectors;
      regExp = QgsGrassObject::newNameRegExp( QgsGrassObject::Vector );
    }
    QgsDebugMsg( "existingNames = " + existingNames.join( "," ) );

    if ( useCopy )
    {
      srcName = srcObject.name();
    }
    else
    {
      if ( !provider )
      {
        errors.append( tr( "Cannot create provider %1 : %2" ).arg( u.providerKey ).arg( u.uri ) );
        continue;
      }
      if ( !provider->isValid() )
      {
        errors.append( tr( "Provider is not valid  %1 : %2" ).arg( u.providerKey ).arg( u.uri ) );
        delete provider;
        continue;
      }
      if ( u.layerType == "raster" )
      {
        extensions = QgsGrassRasterImport::extensions( rasterProvider );
      }
      srcName = u.name;
    }

    // TODO: add a method in QgsGrass to convert a name to GRASS valid name
    QString destName = srcName.replace( " ", "_" );
    if ( QgsNewNameDialog::exists( destName, extensions, existingNames, caseSensitivity ) )
    {
      QgsNewNameDialog dialog( srcName, destName, extensions, existingNames, regExp, caseSensitivity );
      if ( dialog.exec() != QDialog::Accepted )
      {
        delete provider;
        continue;
      }
      destName = dialog.name();
    }

    QgsGrassImport *import = 0;
    QStringList newNames;
    if ( useCopy )
    {
      QgsDebugMsg( "location is the same -> g.copy" );
      QgsGrassObject destObject( mapsetObject );
      destObject.setName( destName );
      destObject.setType( srcObject.type() );
      import = new QgsGrassCopy( srcObject, destObject );
    }

    else if ( u.layerType == "raster" )
    {
      QgsRectangle newExtent = rasterProvider->extent();
      int newXSize;
      int newYSize;
      bool useSrcRegion = true;
      if ( rasterProvider->capabilities() & QgsRasterInterface::Size )
      {
        newXSize = rasterProvider->xSize();
        newYSize = rasterProvider->ySize();
      }
      else
      {
        // TODO: open dialog with size options
        // use location default
        QgsDebugMsg( "Unknown size -> using default location region" );
        struct Cell_head window;
        if ( !QgsGrass::defaultRegion( mGisdbase, mLocation, &window ) )
        {
          errors.append( tr( "Cannot get default location region." ) );
          delete rasterProvider;
          continue;
        }
        newXSize = window.cols;

        newYSize = window.rows;

        newExtent = QgsGrass::extent( &window );
        useSrcRegion = false;
      }

      QgsRasterPipe* pipe = new QgsRasterPipe();
      pipe->set( rasterProvider );

      QgsCoordinateReferenceSystem providerCrs = rasterProvider->crs();
      QgsDebugMsg( "providerCrs = " + providerCrs.toWkt() );
      QgsDebugMsg( "mapsetCrs = " + mapsetCrs.toWkt() );

      bool settingsExternal = settings.value( "/GRASS/browser/import/external", true ).toBool();
      QgsGrassObject rasterObject( mGisdbase, mLocation, mName, destName, QgsGrassObject::Raster );
      if ( providerCrs.isValid() && mapsetCrs.isValid() && providerCrs == mapsetCrs
           && rasterProvider->name() == "gdal" && settingsExternal )
      {
        import = new QgsGrassExternal( rasterProvider->dataSourceUri(), rasterObject );
        delete rasterProvider;
      }
      else
      {
        if ( providerCrs.isValid() && mapsetCrs.isValid() && providerCrs != mapsetCrs )
        {
          QgsRasterProjector * projector = new QgsRasterProjector;
          projector->setCRS( providerCrs, mapsetCrs );
          if ( useSrcRegion )
          {
            projector->destExtentSize( rasterProvider->extent(), rasterProvider->xSize(), rasterProvider->ySize(),
                                       newExtent, newXSize, newYSize );
          }
          QgsRasterProjector::Precision precision = ( QgsRasterProjector::Precision ) settings.value( "/GRASS/browser/import/crsTransform", QgsRasterProjector::Approximate ).toInt();
          projector->setPrecision( precision );

          pipe->set( projector );
        }
        QgsDebugMsg( "newExtent = " + newExtent.toString() );
        QgsDebugMsg( QString( "newXSize = %1 newYSize = %2" ).arg( newXSize ).arg( newYSize ) );

        //QString path = mPath + "/" + "raster" + "/" + u.name;
        import = new QgsGrassRasterImport( pipe, rasterObject, newExtent, newXSize, newYSize ); // takes pipe ownership
      }
    }
    else if ( u.layerType == "vector" )
    {
      QString path = mPath + "/" + "raster" + "/" + u.name;
      QgsGrassObject vectorObject( mGisdbase, mLocation, mName, destName, QgsGrassObject::Vector );
      import = new QgsGrassVectorImport( vectorProvider, vectorObject ); // takes provider ownership
    }

    connect( import, SIGNAL( finished( QgsGrassImport* ) ), SLOT( onImportFinished( QgsGrassImport* ) ) );

    // delete existing files (confirmed before in dialog)
    bool deleteOk = true;
    foreach ( QString name, import->names() )
    {
      QgsGrassObject obj( import->grassObject() );
      obj.setName( name );
      if ( QgsGrass::objectExists( obj ) )
      {
        QgsDebugMsg( name + " exists -> delete" );
        if ( !QgsGrass::deleteObject( obj ) )
        {
          errors.append( tr( "Cannot delete %1" ).arg( name ) );
          deleteOk = false;
        }
      }
    }
    if ( !deleteOk )
    {
      delete import;
      continue;
    }

    import->importInThread();
    mImports.append( import );
    if ( u.layerType == "raster" )
    {
      existingRasters.append( import->names() );
    }
    else if ( u.layerType == "vector" )
    {
      existingVectors.append( import->names() );
    }
    refresh(); // after each new item so that it is visible if dialog is opened on next item
  }

  if ( !errors.isEmpty() )
  {
    QgsMessageOutput::showMessage( tr( "Import to GRASS mapset" ),
                                   tr( "Failed to import some layers!\n\n" ) + errors.join( "\n" ),
                                   QgsMessageOutput::MessageText );
  }

  return true;
}

void QgsGrassMapsetItem::onImportFinished( QgsGrassImport* import )
{
  QgsDebugMsg( "entered" );
  if ( !import->error().isEmpty() )
  {
    QgsMessageOutput *output = QgsMessageOutput::createMessageOutput();
    output->setTitle( tr( "Import to GRASS mapset failed" ) );
    output->setMessage( tr( "Failed to import %1 to %2: %3" ).arg( import->srcDescription() ).arg( import->grassObject()
                        .mapsetPath() ).arg( import->error() ), QgsMessageOutput::MessageText );
    output->showMessage();
  }

  mImports.removeOne( import );
  import->deleteLater();
  refresh();
}

//----------------------- QgsGrassObjectItemBase ------------------------------

QgsGrassObjectItemBase::QgsGrassObjectItemBase( QgsGrassObject grassObject ) :
    mGrassObject( grassObject )
{
}

void QgsGrassObjectItemBase::renameGrassObject( QgsDataItem* parent )
{
  QgsDebugMsg( "Entered" );

  QStringList existingNames = QgsGrass::grassObjects( mGrassObject.mapsetPath(), mGrassObject.type() );
  // remove current name to avoid warning that exists
  existingNames.removeOne( mGrassObject.name() );
  QgsDebugMsg( "existingNames = " + existingNames.join( "," ) );
  QRegExp regExp = QgsGrassObject::newNameRegExp( mGrassObject.type() );
  Qt::CaseSensitivity caseSensitivity = QgsGrass::caseSensitivity();
  QgsNewNameDialog dialog( mGrassObject.name(), mGrassObject.name(), QStringList(), existingNames, regExp, caseSensitivity );

  if ( dialog.exec() != QDialog::Accepted || dialog.name() == mGrassObject.name() )
  {
    return;
  }

  QgsDebugMsg( "rename " + mGrassObject.name() + " -> " + dialog.name() );

  QgsGrassObject obj( mGrassObject );
  obj.setName( dialog.name() );
  QString errorTitle = QObject::tr( "Rename GRASS %1" ).arg( mGrassObject.elementName() );
  if ( QgsGrass::objectExists( obj ) )
  {
    QgsDebugMsg( obj.name() + " exists -> delete" );
    if ( !QgsGrass::deleteObject( obj ) )
    {
      QgsMessageOutput::showMessage( errorTitle, QObject::tr( "Cannot delete %1" ).arg( obj.name() ), QgsMessageOutput::MessageText );
      return;
    }
  }

  try
  {
    QgsGrass::renameObject( mGrassObject, obj.name() );
    if ( parent )
    {
      parent->refresh();
    }
  }
  catch ( QgsGrass::Exception &e )
  {
    QgsMessageOutput::showMessage( errorTitle,
                                   QObject::tr( "Cannot rename %1 to %2" ).arg( mGrassObject.name() ).arg( obj.name() ) + "\n" + e.what(),
                                   QgsMessageOutput::MessageText );
  }
}

void QgsGrassObjectItemBase::deleteGrassObject( QgsDataItem* parent )
{
  QgsDebugMsg( "Entered" );

  if ( !QgsGrass::deleteObjectDialog( mGrassObject ) )
    return;

  // Warning if failed is currently in QgsGrass::deleteMap
  if ( QgsGrass::deleteObject( mGrassObject ) )
  {
    // message on success is redundant, like in normal file browser
    if ( parent )
      parent->refresh();
  }
}

QgsGrassObjectItem::QgsGrassObjectItem( QgsDataItem* parent, QgsGrassObject grassObject,
                                        QString name, QString path, QString uri,
                                        LayerType layerType, QString providerKey,
                                        bool showObjectActions )
    : QgsLayerItem( parent, name, path, uri, layerType, providerKey )
    , QgsGrassObjectItemBase( grassObject )
    , mShowObjectActions( showObjectActions )
{
  setState( Populated ); // no children, to show non expandable in browser
}

QList<QAction*> QgsGrassObjectItem::actions()
{
  QgsDebugMsg( QString( "mShowObjectActions = %1" ).arg( mShowObjectActions ) );
  QList<QAction*> lst = QgsGrassItemActions::instance()->actions();

  if ( mShowObjectActions )
  {
    QAction* actionRename = new QAction( tr( "Rename" ), this );
    connect( actionRename, SIGNAL( triggered() ), this, SLOT( renameGrassObject() ) );
    lst.append( actionRename );

    QAction* actionDelete = new QAction( tr( "Delete" ), this );
    connect( actionDelete, SIGNAL( triggered() ), this, SLOT( deleteGrassObject() ) );
    lst.append( actionDelete );
  }

  return lst;
}

bool QgsGrassObjectItem::equal( const QgsDataItem *other )
{
  const QgsGrassObjectItem * item = qobject_cast<const QgsGrassObjectItem *>( other );
  return QgsLayerItem::equal( other ) && item && mGrassObject == item->mGrassObject
         && mShowObjectActions == item->mShowObjectActions;
}

void QgsGrassObjectItem::renameGrassObject()
{
  QgsGrassObjectItemBase::renameGrassObject( parent() );
}

void QgsGrassObjectItem::deleteGrassObject()
{
  QgsGrassObjectItemBase::deleteGrassObject( parent() );
}

//----------------------- QgsGrassVectorItem ------------------------------

QgsGrassVectorItem::QgsGrassVectorItem( QgsDataItem* parent, QgsGrassObject grassObject, QString path ) :
    QgsDataCollectionItem( parent, grassObject.name(), path )
    , QgsGrassObjectItemBase( grassObject )
{
  QgsDebugMsg( "name = " + grassObject.name() + " path = " + path );
  setCapabilities( QgsDataItem::NoCapabilities ); // disable fertility
}

QList<QAction*> QgsGrassVectorItem::actions()
{
  QList<QAction*> lst;

  QAction* actionRename = new QAction( tr( "Rename" ), this );
  connect( actionRename, SIGNAL( triggered() ), this, SLOT( renameGrassObject() ) );
  lst.append( actionRename );

  QAction* actionDelete = new QAction( tr( "Delete" ), this );
  connect( actionDelete, SIGNAL( triggered() ), this, SLOT( deleteGrassObject() ) );
  lst.append( actionDelete );

  return lst;
}

void QgsGrassVectorItem::renameGrassObject()
{
  QgsGrassObjectItemBase::renameGrassObject( parent() );
}

void QgsGrassVectorItem::deleteGrassObject()
{
  QgsGrassObjectItemBase::deleteGrassObject( parent() );
}

//----------------------- QgsGrassVectorLayerItem ------------------------------

QgsGrassVectorLayerItem::QgsGrassVectorLayerItem( QgsDataItem* parent, QgsGrassObject grassObject, QString layerName,
    QString path, QString uri,
    LayerType layerType, bool singleLayer )
    : QgsGrassObjectItem( parent, grassObject, layerName, path, uri, layerType, "grass", singleLayer )
    , mSingleLayer( singleLayer )
{
}

QString QgsGrassVectorLayerItem::layerName() const
{
  if ( mSingleLayer )
  {
    return name();
  }
  else
  {
    // to get map + layer when added from browser
    return mGrassObject.name() + " " + name();
  }
}

bool QgsGrassVectorLayerItem::equal( const QgsDataItem *other )
{
  const QgsGrassVectorLayerItem * item = qobject_cast<const QgsGrassVectorLayerItem *>( other );
  return QgsGrassObjectItem::equal( other ) && item && mSingleLayer == item->mSingleLayer;
}

//----------------------- QgsGrassRasterItem ------------------------------

QgsGrassRasterItem::QgsGrassRasterItem( QgsDataItem* parent, QgsGrassObject grassObject,
                                        QString path, QString uri, bool isExternal )
    : QgsGrassObjectItem( parent, grassObject, grassObject.name(), path, uri, QgsLayerItem::Raster, "grassraster" )
    , mExternal( isExternal )
{
}

QIcon QgsGrassRasterItem::icon()
{
  static QIcon linkIcon;

  if ( mExternal )
  {
    if ( linkIcon.isNull() )
    {
      linkIcon = QgsApplication::getThemeIcon( "/mIconRasterLink.svg" );
    }
    return linkIcon;
  }
  return QgsDataItem::icon();
}

bool QgsGrassRasterItem::equal( const QgsDataItem *other )
{
  const QgsGrassRasterItem * item = qobject_cast<const QgsGrassRasterItem *>( other );
  return QgsGrassObjectItem::equal( other ) && item && mExternal == item->mExternal;
}

//----------------------- QgsGrassGroupItem ------------------------------

QgsGrassGroupItem::QgsGrassGroupItem( QgsDataItem* parent, QgsGrassObject grassObject,
                                      QString path, QString uri )
    : QgsGrassObjectItem( parent, grassObject, grassObject.name(), path, uri, QgsLayerItem::Raster, "grassraster" )
{
}

QIcon QgsGrassGroupItem::icon()
{
  static QIcon linkIcon;

  if ( linkIcon.isNull() )
  {
    linkIcon = QgsApplication::getThemeIcon( "/mIconRasterGroup.svg" );
  }
  return linkIcon;
}

//----------------------- QgsGrassImportItem ------------------------------

QgsAnimatedIcon *QgsGrassImportItem::mImportIcon = 0;

QgsGrassImportItem::QgsGrassImportItem( QgsDataItem* parent, const QString& name, const QString& path, QgsGrassImport* import )
    : QgsDataItem( QgsDataItem::Layer, parent, name, path )
    , QgsGrassObjectItemBase( import->grassObject() )
    , mImport( import )
{
  setCapabilities( QgsDataItem::NoCapabilities ); // disable fertility
  setState( Populating );

  if ( mImportIcon )
  {
    mImportIcon->connectFrameChanged( this, SLOT( emitDataChanged() ) );
  }
}

QgsGrassImportItem::~QgsGrassImportItem()
{
  if ( mImportIcon )
  {
    mImportIcon->disconnectFrameChanged( this, SLOT( emitDataChanged() ) );
  }
}

QList<QAction*> QgsGrassImportItem::actions()
{
  QList<QAction*> lst;

  QAction* actionRename = new QAction( tr( "Cancel" ), this );
  connect( actionRename, SIGNAL( triggered() ), this, SLOT( cancel() ) );
  lst.append( actionRename );

  return lst;
}

void QgsGrassImportItem::cancel()
{
  QgsDebugMsg( "Entered" );
  if ( !mImport ) // should not happen
  {
    QgsDebugMsg( "mImport is null" );
    return;
  }
  mImport->cancel();
}

QIcon QgsGrassImportItem::icon()
{
  if ( mImportIcon )
  {
    return mImportIcon->icon();
  }
  return QIcon();
}

void QgsGrassImportItem::initIcon()
{
  if ( !mImportIcon )
  {
    mImportIcon = new QgsAnimatedIcon( QgsApplication::iconPath( "/mIconImport.gif" ) );
  }
}

//-------------------------------------------------------------------------

QGISEXTERN int dataCapabilities()
{
  return QgsDataProvider::Dir;
}

QGISEXTERN QgsDataItem * dataItem( QString theDirPath, QgsDataItem* parentItem )
{
  if ( QgsGrass::isLocation( theDirPath ) )
  {
    QString path;
    QDir dir( theDirPath );
    QString dirName = dir.dirName();
    if ( parentItem )
    {
      path = parentItem->path();
    }
    else
    {
      dir.cdUp();
      path = dir.path();
    }
    path = path + "/" + "grass:" + dirName;
    QgsGrassLocationItem * location = new QgsGrassLocationItem( parentItem, theDirPath, path );
    return location;
  }
  return 0;
}

/**
* Class factory to return a pointer to a newly created
* QgsGrassProvider object
*/
QGISEXTERN QgsGrassProvider * classFactory( const QString *uri )
{
  return new QgsGrassProvider( *uri );
}

/** Required key function (used to map the plugin to a data store type)
*/
QGISEXTERN QString providerKey()
{
  return QString( "grass" );
}

/**
* Required description function
*/
QGISEXTERN QString description()
{
  return QString( "GRASS %1 vector provider" ).arg( GRASS_VERSION_MAJOR );
}

/**
* Required isProvider function. Used to determine if this shared library
* is a data provider plugin
*/
QGISEXTERN bool isProvider()
{
  // Init GRASS in the first function called by provider registry so that it is called
  // on main thread, not sure but suspicious that init in thread is causing problems,
  // at least on Windows, not that dataItem() is called in thread
  QgsGrass::init();
  return true;
}

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
#include "qgsproviderregistry.h"
#include "qgsrasterdataprovider.h"
#include "qgsrasterprojector.h"
#include "qgslogger.h"
#include "qgssettings.h"
#include "qgsdataitemprovider.h"
#include "qgsgrassprovidermodule.h"
#include "qgsgrassprovider.h"
#include "qgsgrass.h"
#include "qgsgrassvector.h"
#include "qgsproject.h"
#include "qgsapplication.h"

#ifdef HAVE_GUI
#include "qgsnewnamedialog.h"
#endif

#include <QAction>
#include <QFileInfo>
#include <QDir>
#include <QLabel>
#include <QObject>
#include <QProgressBar>
#include <QScrollBar>
#include <QTextEdit>
#include <QRegularExpression>

//----------------------- QgsGrassItemActions ------------------------------
#ifdef HAVE_GUI
QgsGrassItemActions::QgsGrassItemActions( const QgsGrassObject &grassObject, bool valid, QObject *parent )
  : QObject( parent )
  , mGrassObject( grassObject )
  , mValid( valid )
{
}

QList<QAction *> QgsGrassItemActions::actions( QWidget *parent )
{
  QList<QAction *> list;

  QAction *optionsAction = new QAction( tr( "GRASS Options…" ), parent );
  connect( optionsAction, &QAction::triggered, QgsGrass::instance(), &QgsGrass::openOptions );
  list << optionsAction;

  bool isMapsetOwner = QgsGrass::isOwner( mGrassObject.gisdbase(), mGrassObject.location(), mGrassObject.mapset() );

  // TODO: add icons to provider
  // TODO: check ownership
  if ( mGrassObject.type() == QgsGrassObject::Location )
  {
    QAction *newMapsetAction = new QAction( QgsApplication::getThemeIcon( QStringLiteral( "grass_new_mapset.png" ) ), tr( "New Mapset…" ), parent );
    connect( newMapsetAction, &QAction::triggered, this, &QgsGrassItemActions::newMapset );
    list << newMapsetAction;
  }

  if ( mGrassObject.type() == QgsGrassObject::Mapset && isMapsetOwner )
  {
    QAction *openMapsetAction = new QAction( QgsApplication::getThemeIcon( QStringLiteral( "grass_open_mapset.png" ) ), tr( "Open Mapset" ), parent );
    connect( openMapsetAction, &QAction::triggered, this, &QgsGrassItemActions::openMapset );
    list << openMapsetAction;
  }

  if ( mGrassObject.type() == QgsGrassObject::Mapset && mGrassObject.locationIdentical( QgsGrass::getDefaultLocationObject() )
       && mGrassObject.mapset() != QgsGrass::getDefaultMapset() )
  {
    if ( !QgsGrass::instance()->isMapsetInSearchPath( mGrassObject.mapset() ) )
    {
      QAction *openMapsetAction = new QAction( tr( "Add Mapset to Search Path" ), parent );
      connect( openMapsetAction, &QAction::triggered, this, &QgsGrassItemActions::addMapsetToSearchPath );
      list << openMapsetAction;
    }
    else
    {
      QAction *openMapsetAction = new QAction( tr( "Remove Mapset from Search Path" ), parent );
      connect( openMapsetAction, &QAction::triggered, this, &QgsGrassItemActions::removeMapsetFromSearchPath );
      list << openMapsetAction;
    }
  }

  if ( ( mGrassObject.type() == QgsGrassObject::Raster || mGrassObject.type() == QgsGrassObject::Vector
         ||  mGrassObject.type() == QgsGrassObject::Group ) && isMapsetOwner )
  {
    QAction *renameAction = new QAction( tr( "Rename…" ), parent );
    connect( renameAction, &QAction::triggered, this, &QgsGrassItemActions::renameGrassObject );
    list << renameAction;

    QAction *deleteAction = new QAction( tr( "Delete…" ), parent );
    connect( deleteAction, &QAction::triggered, this, &QgsGrassItemActions::deleteGrassObject );
    list << deleteAction;
  }

  if ( ( mGrassObject.type() == QgsGrassObject::Mapset || mGrassObject.type() == QgsGrassObject::Vector )
       && mValid && isMapsetOwner )
  {
    // TODO: disable new layer actions on maps currently being edited
    QAction *newPointAction = new QAction( tr( "New Point Layer…" ), parent );
    connect( newPointAction, &QAction::triggered, this, &QgsGrassItemActions::newPointLayer );
    list << newPointAction;

    QAction *newLineAction = new QAction( tr( "New Line Layer…" ), parent );
    connect( newLineAction, &QAction::triggered, this, &QgsGrassItemActions::newLineLayer );
    list << newLineAction;

    QAction *newPolygonAction = new QAction( tr( "New Polygon Layer…" ), parent );
    connect( newPolygonAction, &QAction::triggered, this, &QgsGrassItemActions::newPolygonLayer );
    list << newPolygonAction;
  }

  return list;
}

void QgsGrassItemActions::newMapset()
{

  QStringList existingNames = QgsGrass::mapsets( mGrassObject.gisdbase(), mGrassObject.mapsetPath() );
  QgsDebugMsgLevel( QStringLiteral( "existingNames = " ) + existingNames.join( ',' ), 2 );
  Qt::CaseSensitivity caseSensitivity = QgsGrass::caseSensitivity();
  QgsNewNameDialog dialog( QString(), QString(), QStringList(), existingNames, caseSensitivity );
  dialog.setRegularExpression( QgsGrassObject::newNameRegExp( QgsGrassObject::Mapset ) );

  if ( dialog.exec() != QDialog::Accepted )
  {
    return;
  }
  QString name = dialog.name();
  QgsDebugMsgLevel( "name = " + name, 2 );
  QString error;
  QgsGrass::createMapset( mGrassObject.gisdbase(), mGrassObject.location(), name, error );
  if ( !error.isEmpty() )
  {
    QgsGrass::warning( tr( "Cannot create new mapset: %1" ).arg( error ) );
  }
}

void QgsGrassItemActions::openMapset()
{
  QString error = QgsGrass::openMapset( mGrassObject.gisdbase(), mGrassObject.location(), mGrassObject.mapset() );
  if ( !error.isEmpty() )
  {
    QgsGrass::warning( error );
    return;
  }
  QgsGrass::saveMapset();
}

void QgsGrassItemActions::addMapsetToSearchPath()
{
  QString error;
  QgsGrass::instance()->addMapsetToSearchPath( mGrassObject.mapset(), error );
  if ( !error.isEmpty() )
  {
    QgsGrass::warning( error );
    return;
  }
}

void QgsGrassItemActions::removeMapsetFromSearchPath()
{
  QString error;
  QgsGrass::instance()->removeMapsetFromSearchPath( mGrassObject.mapset(), error );
  if ( !error.isEmpty() )
  {
    QgsGrass::warning( error );
    return;
  }
}

void QgsGrassItemActions::renameGrassObject()
{

  QStringList existingNames = QgsGrass::grassObjects( mGrassObject, mGrassObject.type() );
  // remove current name to avoid warning that exists
  existingNames.removeOne( mGrassObject.name() );
  QgsDebugMsgLevel( QStringLiteral( "existingNames = " ) + existingNames.join( ',' ), 2 );
  Qt::CaseSensitivity caseSensitivity = QgsGrass::caseSensitivity();
  QgsNewNameDialog dialog( mGrassObject.name(), mGrassObject.name(), QStringList(), existingNames, caseSensitivity );
  dialog.setRegularExpression( QgsGrassObject::newNameRegExp( mGrassObject.type() ) );

  if ( dialog.exec() != QDialog::Accepted || dialog.name() == mGrassObject.name() )
  {
    return;
  }

  QgsDebugMsgLevel( "rename " + mGrassObject.name() + " -> " + dialog.name(), 2 );

  QgsGrassObject obj( mGrassObject );
  obj.setName( dialog.name() );
  QString errorTitle = QObject::tr( "Rename GRASS %1" ).arg( mGrassObject.elementName() );
  if ( QgsGrass::objectExists( obj ) )
  {
    QgsDebugMsgLevel( obj.name() + " exists -> delete", 2 );
    if ( !QgsGrass::deleteObject( obj ) )
    {
      QgsMessageOutput::showMessage( errorTitle, QObject::tr( "Cannot delete %1" ).arg( obj.name() ), QgsMessageOutput::MessageText );
      return;
    }
  }

  try
  {
    QgsGrass::renameObject( mGrassObject, obj.name() );
  }
  catch ( QgsGrass::Exception &e )
  {
    QgsMessageOutput::showMessage( errorTitle,
                                   QObject::tr( "Cannot rename %1 to %2" ).arg( mGrassObject.name(), obj.name() ) + "\n" + e.what(),
                                   QgsMessageOutput::MessageText );
  }
}

void QgsGrassItemActions::deleteGrassObject()
{

  if ( !QgsGrass::deleteObjectDialog( mGrassObject ) )
    return;

  // Warning if failed is currently in QgsGrass::deleteMap
  if ( QgsGrass::deleteObject( mGrassObject ) )
  {
    // message on success is redundant, like in normal file browser
  }
}

QString QgsGrassItemActions::newVectorMap()
{

  QStringList existingNames = QgsGrass::grassObjects( mGrassObject, QgsGrassObject::Vector );
  QgsDebugMsgLevel( QStringLiteral( "existingNames = " ) + existingNames.join( ',' ), 2 );
  Qt::CaseSensitivity caseSensitivity = QgsGrass::caseSensitivity();
  QgsNewNameDialog dialog( QString(), QString(), QStringList(), existingNames, caseSensitivity );
  dialog.setRegularExpression( QgsGrassObject::newNameRegExp( QgsGrassObject::Vector ) );

  if ( dialog.exec() != QDialog::Accepted )
  {
    return QString();
  }
  QString name = dialog.name();
  QgsDebugMsgLevel( "name = " + name, 2 );

  QgsGrassObject mapObject = mGrassObject;
  mapObject.setName( name );
  mapObject.setType( QgsGrassObject::Vector );

  QString error;

  QgsGrass::createVectorMap( mapObject, error );
  if ( !error.isEmpty() )
  {
    QgsGrass::warning( error );
    name.clear();
  }
  return name;
}


void QgsGrassItemActions::newLayer( const QString &type )
{
  QString name;
  if ( mGrassObject.type() == QgsGrassObject::Mapset )
  {
    name = newVectorMap();
  }
  else if ( mGrassObject.type() == QgsGrassObject::Vector )
  {
    name = mGrassObject.name();
  }

  QgsDebugMsgLevel( "name = " + name, 2 );
  if ( name.isEmpty() )
  {
    QgsDebugError( QStringLiteral( "could not create map" ) );
    return;
  }

  // TODO: better to get max layer number from item?
  QgsGrassObject grassObject = mGrassObject;
  grassObject.setName( name );
  grassObject.setType( QgsGrassObject::Vector );
  QgsGrassVector vector( grassObject );
  vector.openHead();
  int layerNumber = vector.maxLayerNumber() + 1;

  QgsDebugMsgLevel( QStringLiteral( "layerNumber = %1" ).arg( layerNumber ), 2 );

  QString uri = mGrassObject.mapsetPath() + "/" + name + QStringLiteral( "/%1_%2" ).arg( layerNumber ).arg( type );
  QgsDebugMsgLevel( "uri = " + uri, 2 );
  QgsGrass::instance()->emitNewLayer( uri, name );
}

void QgsGrassItemActions::newPointLayer()
{
  newLayer( QStringLiteral( "point" ) );
}

void QgsGrassItemActions::newLineLayer()
{
  newLayer( QStringLiteral( "line" ) );
}

void QgsGrassItemActions::newPolygonLayer()
{
  newLayer( QStringLiteral( "polygon" ) );
}
#endif

//----------------------- QgsGrassObjectItemBase ------------------------------

QgsGrassObjectItemBase::QgsGrassObjectItemBase( const QgsGrassObject &grassObject )
  : mGrassObject( grassObject )
{
}

//----------------------- QgsGrassLocationItem ------------------------------

QgsGrassLocationItem::QgsGrassLocationItem( QgsDataItem *parent, const QString &dirPath, const QString &path )
  : QgsDirectoryItem( parent, QString(), dirPath, path )
  , QgsGrassObjectItemBase( QgsGrassObject() )
{
  // modify path to distinguish from directory, so that it can be expanded by path in browser
  QDir dir( mDirPath );
  mName = dir.dirName();

  dir.cdUp();
  QString gisdbase = dir.path();

  mGrassObject = QgsGrassObject( gisdbase, mName, QString(), QString(), QgsGrassObject::Location );
#ifdef HAVE_GUI
  mActions = new QgsGrassItemActions( mGrassObject, true, this );
#endif

  mIconName = QStringLiteral( "grass_location.svg" );

  // set Directory type so that when sorted it gets into dirs (after the dir it represents)
  mType = Qgis::BrowserItemType::Directory;
}

QVector<QgsDataItem *>QgsGrassLocationItem::createChildren()
{
  QVector<QgsDataItem *> mapsets;

  QDir dir( mDirPath );

  const QStringList entries = dir.entryList( QDir::Dirs | QDir::NoDotAndDotDot, QDir::Name );
  mapsets.reserve( entries.size() );
  for ( const QString &name : entries )
  {
    QString path = dir.absoluteFilePath( name );

    if ( QgsGrass::isMapset( path ) )
    {
      QgsGrassMapsetItem *mapset = new QgsGrassMapsetItem( this, path, mPath + "/" + name );
      mapsets.append( mapset );
    }
  }
  return mapsets;
}

QIcon QgsGrassLocationItem::icon()
{
  return QgsApplication::getThemeIcon( QStringLiteral( "/grass_mapset.svg" ) );
}


//----------------------- QgsGrassMapsetItem ------------------------------

QList<QgsGrassImport *> QgsGrassMapsetItem::sImports;

QgsGrassMapsetItem::QgsGrassMapsetItem( QgsDataItem *parent, const QString &dirPath, const QString &path )
  : QgsDirectoryItem( parent, QString(), dirPath, path )
  , QgsGrassObjectItemBase( QgsGrassObject() )
  , mRefreshLater( false )
{
  QDir dir( mDirPath );
  mName = dir.dirName();
  dir.cdUp();
  QString location = dir.dirName();
  dir.cdUp();
  QString gisdbase = dir.path();

  mGrassObject = QgsGrassObject( gisdbase, location, mName, QString(), QgsGrassObject::Mapset );
#ifdef HAVE_GUI
  mActions = new QgsGrassItemActions( mGrassObject, true, this );
#endif

  // emit data changed to possibly change icon
  connect( QgsGrass::instance(), &QgsGrass::mapsetChanged, this, &QgsGrassMapsetItem::updateIcon );
  connect( QgsGrass::instance(), &QgsGrass::mapsetSearchPathChanged, this, &QgsGrassMapsetItem::updateIcon );

  mIconName = QStringLiteral( "grass_mapset.svg" );
}

QIcon QgsGrassMapsetItem::icon()
{
  if ( mGrassObject == QgsGrass::getDefaultMapsetObject() )
  {
    return QgsApplication::getThemeIcon( QStringLiteral( "/grass_mapset_open.svg" ) );
  }
  else if ( mGrassObject.locationIdentical( QgsGrass::getDefaultLocationObject() ) )
  {
    if ( QgsGrass::instance()->isMapsetInSearchPath( mGrassObject.mapset() ) )
    {
      return QgsApplication::getThemeIcon( QStringLiteral( "/grass_mapset_search.svg" ) );
    }
  }
  return QgsApplication::getThemeIcon( QStringLiteral( "/grass_mapset.svg" ) );
}

void QgsGrassMapsetItem::setState( Qgis::BrowserItemState state )
{

  // TODO: it seems to be causing strange icon switching during import, sometimes
  if ( state == Qgis::BrowserItemState::Populated )
  {
    if ( !mMapsetFileSystemWatcher )
    {
      mMapsetFileSystemWatcher = new QFileSystemWatcher( this );
      mMapsetFileSystemWatcher->addPath( mDirPath + "/vector" );
      mMapsetFileSystemWatcher->addPath( mDirPath + "/cellhd" );
      connect( mMapsetFileSystemWatcher, &QFileSystemWatcher::directoryChanged, this, &QgsGrassMapsetItem::onDirectoryChanged );
    }
  }
  else if ( state == Qgis::BrowserItemState::NotPopulated )
  {
    if ( mMapsetFileSystemWatcher )
    {
      delete mMapsetFileSystemWatcher;
      mMapsetFileSystemWatcher = nullptr;
    }
  }

  QgsDirectoryItem::setState( state );
}

bool QgsGrassMapsetItem::objectInImports( const QgsGrassObject &grassObject )
{
  const auto constSImports = sImports;
  for ( QgsGrassImport *import : constSImports )
  {
    if ( !import )
    {
      continue;
    }
    if ( !import->grassObject().mapsetIdentical( grassObject )
         || import->grassObject().type() != grassObject.type() )
    {
      continue;
    }
    if ( import->names().contains( grassObject.name() ) )
    {
      return true;
    }
  }
  return false;
}

QVector<QgsDataItem *> QgsGrassMapsetItem::createChildren()
{

  QVector<QgsDataItem *> items;

  QStringList vectorNames = QgsGrass::vectors( mDirPath );
  const auto constVectorNames = vectorNames;
  for ( const QString &name : constVectorNames )
  {
    if ( mRefreshLater )
    {
      deleteLater( items );
      return items;
    }

    QgsGrassObject vectorObject = mGrassObject;
    vectorObject.setName( name );
    vectorObject.setType( QgsGrassObject::Vector );

    // Skip temporary import maps. If Vect_open_old during refresh fails due to missing topo, hist file remains open
    // and Windows does not allow deleting the temporary map to qgis.v.in. In any case we don't want to show temporary import maps.
    // TODO: add some auto cleaning mechanism to remove temporary maps left after import fail
    // keep excluded tmp name in sync with qgis.v.in
    QgsDebugMsgLevel( "name = " + name, 2 );
    if ( name.startsWith( QLatin1String( "qgis_import_tmp_" ) ) )
    {
      QgsDebugMsgLevel( "skip tmp import vector " + name, 2 );
      continue;
    }

    if ( objectInImports( vectorObject ) )
    {
      QgsDebugMsgLevel( "skip currently being imported vector " + name, 2 );
      continue;
    }

    QString mapPath = mPath + "/vector/" + name;
    QStringList layerNames;
    QgsGrassVectorItem *map = nullptr;

    // test topo version before getting layers, because GRASS 7 Vect_open_old is calling G_fatal_error
    // if topo version does not match GRASS lib version
    int topoMajor = 0;
    int topoMinor = 0;
    bool gotTopoVersion = QgsGrass::topoVersion( mGrassObject.gisdbase(), mGrassObject.location(), mGrassObject.mapset(), name, topoMajor, topoMinor );
    QgsDebugMsgLevel( QStringLiteral( "name = %1 topoMajor = %2 topoMinor = %3" ).arg( name ).arg( topoMajor ).arg( topoMinor ), 2 );
    QString topoError;
    if ( !gotTopoVersion )
    {
      topoError = tr( "topology missing" );
    }
    // GRASS 5-6: topoMajor = 5 topoMinor = 0
    // GRASS   7: topoMajor = 5 topoMinor = 1
    else if ( topoMajor != 5 )
    {
      topoError = tr( "topology version not supported" );
    }
    else if ( topoMinor == 0 &&  GRASS_VERSION_MAJOR == 7 )
    {
      topoError = tr( "topology version 6" );
    }

    if ( !topoError.isEmpty() )
    {
      map = new QgsGrassVectorItem( this, vectorObject, mapPath, name + " : " + topoError, false );
      items.append( map );
      continue;
    }

    try
    {
      layerNames = QgsGrass::vectorLayers( mGrassObject.gisdbase(), mGrassObject.location(), mGrassObject.mapset(), name );
    }
    catch ( QgsGrass::Exception &e )
    {
      map = new QgsGrassVectorItem( this, vectorObject, mapPath, name + " : " + e.what(), false );
      items.append( map );
      continue;
    }

    if ( layerNames.size() == 0 )
    {
      map = new QgsGrassVectorItem( this, vectorObject, mapPath, name + " : " + tr( "empty" ), true );
      items.append( map );
      continue;
    }
    else if ( layerNames.size() > 1 )
    {
      //map = new QgsDataCollectionItem( this, name, mapPath );
      //map->setCapabilities( QgsDataItem::NoCapabilities ); // disable fertility
      map = new QgsGrassVectorItem( this, vectorObject, mapPath );
    }
    const auto constLayerNames = layerNames;
    for ( const QString &layerName : constLayerNames )
    {
      // don't use QDir::separator(), windows work with '/' and backslash may be lost if
      // somewhere not properly escaped (there was bug in QgsMimeDataUtils for example)
      QString uri = mDirPath + "/" + name + "/" + layerName;
      Qgis::BrowserLayerType layerType = Qgis::BrowserLayerType::Vector;
      QString typeName = layerName.split( '_' ).value( 1 );
      QString baseLayerName = layerName.split( '_' ).value( 0 );

      if ( typeName == QLatin1String( "point" ) || typeName == QLatin1String( "node" ) )
        layerType = Qgis::BrowserLayerType::Point;
      else if ( typeName == QLatin1String( "line" ) )
        layerType = Qgis::BrowserLayerType::Line;
      else if ( typeName == QLatin1String( "polygon" ) )
        layerType = Qgis::BrowserLayerType::Polygon;

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
      map->setState( Qgis::BrowserItemState::Populated );
      items.append( map );
    }
  }

  const QStringList rasterNames = QgsGrass::rasters( mDirPath );
  for ( const QString &name : rasterNames )
  {
    if ( mRefreshLater )
    {
      deleteLater( items );
      return items;
    }
    QString path = mPath + "/" + "raster" + "/" + name;
    QString uri = mDirPath + "/" + "cellhd" + "/" + name;
    QgsDebugMsgLevel( "uri = " + uri, 2 );

    QgsGrassObject rasterObject( mGrassObject.gisdbase(), mGrassObject.location(), mGrassObject.mapset(), name, QgsGrassObject::Raster );
    if ( objectInImports( rasterObject ) )
    {
      QgsDebugMsgLevel( "skip currently being imported raster " + name, 2 );
      continue;
    }

    QgsGrassRasterItem *layer = new QgsGrassRasterItem( this, rasterObject, path, uri, QgsGrass::isExternal( rasterObject ) );
    items.append( layer );
  }

  QStringList groupNames = QgsGrass::groups( mDirPath );
  const auto constGroupNames = groupNames;
  for ( const QString &name : constGroupNames )
  {
    if ( mRefreshLater )
    {
      deleteLater( items );
      return items;
    }
    QString path = mPath + "/" + "group" + "/" + name;
    QString uri = mDirPath + "/" + "group" + "/" + name;
    QgsDebugMsgLevel( "uri = " + uri, 2 );

    QgsGrassObject rasterObject( mGrassObject.gisdbase(), mGrassObject.location(), mGrassObject.mapset(), name, QgsGrassObject::Group );
    QgsGrassGroupItem *layer = new QgsGrassGroupItem( this, rasterObject, path, uri );
    items.append( layer );
  }

  const auto constSImports = sImports;
  for ( QgsGrassImport *import : constSImports )
  {
    if ( mRefreshLater )
    {
      deleteLater( items );
      return items;
    }
    if ( !import )
    {
      continue;
    }
    if ( !import->grassObject().mapsetIdentical( mGrassObject ) )
    {
      continue;
    }
    const auto constNames = import->names();
    for ( const QString &name : constNames )
    {
      QString path = mPath + "/" + import->grassObject().elementName() + "/" + name;
      items.append( new QgsGrassImportItem( this, name, path, import ) );
    }
  }

  return items;
}

bool QgsGrassMapsetItem::acceptDrop()
{
  return QgsGrass::isOwner( mGrassObject.gisdbase(), mGrassObject.location(), mGrassObject.mapset() );
}

bool QgsGrassMapsetItem::handleDrop( const QMimeData *data, Qt::DropAction )
{
  if ( !QgsMimeDataUtils::isUriList( data ) )
    return false;

  QgsSettings settings;

  QgsCoordinateReferenceSystem mapsetCrs = QgsGrass::crsDirect( mGrassObject.gisdbase(), mGrassObject.location() );

  QStringList existingRasters = QgsGrass::rasters( mGrassObject.mapsetPath() );
  QStringList existingVectors = QgsGrass::vectors( mGrassObject.mapsetPath() );
  // add currently being imported
  const auto constSImports = sImports;
  for ( QgsGrassImport *import : constSImports )
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

  const auto constLst = lst;
  for ( const QgsMimeDataUtils::Uri &u : constLst )
  {
    if ( u.layerType != QLatin1String( "raster" ) && u.layerType != QLatin1String( "vector" ) )
    {
      errors.append( tr( "%1 layer type not supported" ).arg( u.name ) );
      continue;
    }
    QgsRasterDataProvider *rasterProvider = nullptr;
    QgsVectorDataProvider *vectorProvider = nullptr;
    QgsDataProvider *provider = nullptr;
    QStringList extensions;
    QStringList existingNames;
    QString regExp;
    QgsGrassObject srcObject;
    QString srcName;

    // use g.copy for GRASS maps in the same location
    bool useCopy = false;

    if ( u.layerType == QLatin1String( "raster" ) )
    {
      if ( u.providerKey == QLatin1String( "grassraster" ) && srcObject.setFromUri( u.uri )
           && srcObject.locationIdentical( mGrassObject ) )
      {
        useCopy = true;
      }
      else
      {
        rasterProvider = qobject_cast<QgsRasterDataProvider *>( QgsProviderRegistry::instance()->createProvider( u.providerKey, u.uri ) );
        provider = rasterProvider;
      }
      existingNames = existingRasters;
      regExp = QgsGrassObject::newNameRegExp( QgsGrassObject::Raster );
    }
    else if ( u.layerType == QLatin1String( "vector" ) )
    {
      if ( u.providerKey == QLatin1String( "grass" ) && srcObject.setFromUri( u.uri )
           && srcObject.locationIdentical( mGrassObject ) )
      {
        useCopy = true;
      }
      else
      {
        vectorProvider = qobject_cast<QgsVectorDataProvider *>( QgsProviderRegistry::instance()->createProvider( u.providerKey, u.uri ) );
        provider = vectorProvider;
      }
      existingNames = existingVectors;
      regExp = QgsGrassObject::newNameRegExp( QgsGrassObject::Vector );
    }
    QgsDebugMsgLevel( QStringLiteral( "existingNames = " ) + existingNames.join( ',' ), 2 );

    if ( useCopy )
    {
      srcName = srcObject.name();
    }
    else
    {
      if ( !provider )
      {
        errors.append( tr( "Cannot create provider %1 : %2" ).arg( u.providerKey, u.uri ) );
        continue;
      }
      if ( !provider->isValid() )
      {
        errors.append( tr( "Provider is not valid  %1 : %2" ).arg( u.providerKey, u.uri ) );
        delete provider;
        continue;
      }
      if ( u.layerType == QLatin1String( "raster" ) )
      {
        extensions = QgsGrassRasterImport::extensions( rasterProvider );
      }
      srcName = u.name;
    }

    // TODO: add a method in QgsGrass to convert a name to GRASS valid name
    QString destName = srcName.replace( QLatin1String( " " ), QLatin1String( "_" ) );
#ifdef HAVE_GUI
    Qt::CaseSensitivity caseSensitivity = QgsGrass::caseSensitivity();

    const QRegularExpression destNameRegExp( QRegularExpression::anchoredPattern( regExp ) );
    const QRegularExpressionMatch destNameRegExpMatch = destNameRegExp.match( destName );

    if ( QgsNewNameDialog::exists( destName, extensions, existingNames, caseSensitivity )
         || !destNameRegExpMatch.hasMatch() )
    {
      QgsNewNameDialog dialog( srcName, destName, extensions, existingNames, caseSensitivity );
      dialog.setRegularExpression( regExp );
      if ( dialog.exec() != QDialog::Accepted )
      {
        delete provider;
        continue;
      }
      destName = dialog.name();
    }
#endif

    QgsGrassImport *import = nullptr;
    if ( useCopy )
    {
      QgsDebugMsgLevel( QStringLiteral( "location is the same -> g.copy" ), 2 );
      QgsGrassObject destObject( mGrassObject );
      destObject.setName( destName );
      destObject.setType( srcObject.type() );
      import = new QgsGrassCopy( srcObject, destObject );
    }

    else if ( u.layerType == QLatin1String( "raster" ) )
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
        QgsDebugMsgLevel( QStringLiteral( "Unknown size -> using default location region" ), 2 );
        struct Cell_head window;
        if ( !QgsGrass::defaultRegion( mGrassObject.gisdbase(), mGrassObject.location(), &window ) )
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

      QgsCoordinateReferenceSystem providerCrs = rasterProvider->crs();
      QgsDebugMsgLevel( "providerCrs = " + providerCrs.toWkt(), 2 );
      QgsDebugMsgLevel( "mapsetCrs = " + mapsetCrs.toWkt(), 2 );

      bool settingsExternal = settings.value( QStringLiteral( "GRASS/browser/import/external" ), true ).toBool();
      QgsGrassObject rasterObject( mGrassObject.gisdbase(), mGrassObject.location(), mGrassObject.mapset(), destName, QgsGrassObject::Raster );
      if ( providerCrs.isValid() && mapsetCrs.isValid() && providerCrs == mapsetCrs
           && rasterProvider->name() == QLatin1String( "gdal" ) && settingsExternal )
      {
        import = new QgsGrassExternal( rasterProvider->dataSourceUri(), rasterObject );
        delete rasterProvider;
      }
      else
      {
        QgsRasterPipe *pipe = new QgsRasterPipe();
        pipe->set( rasterProvider );
        if ( providerCrs.isValid() && mapsetCrs.isValid() && providerCrs != mapsetCrs )
        {
          QgsRasterProjector *projector = new QgsRasterProjector;
          projector->setCrs( providerCrs, mapsetCrs, QgsProject::instance()->transformContext() );
          if ( useSrcRegion )
          {
            projector->destExtentSize( rasterProvider->extent(), rasterProvider->xSize(), rasterProvider->ySize(),
                                       newExtent, newXSize, newYSize );
          }
          QgsRasterProjector::Precision precision = settings.enumValue( QStringLiteral( "GRASS/browser/import/crsTransform" ), QgsRasterProjector::Approximate );
          projector->setPrecision( precision );

          pipe->set( projector );
        }
        QgsDebugMsgLevel( "newExtent = " + newExtent.toString(), 2 );
        QgsDebugMsgLevel( QStringLiteral( "newXSize = %1 newYSize = %2" ).arg( newXSize ).arg( newYSize ), 2 );

        //QString path = mPath + "/" + "raster" + "/" + u.name;
        import = new QgsGrassRasterImport( pipe, rasterObject, newExtent, newXSize, newYSize ); // takes pipe ownership
      }
    }
    else if ( u.layerType == QLatin1String( "vector" ) )
    {
      QgsGrassObject vectorObject( mGrassObject.gisdbase(), mGrassObject.location(), mGrassObject.mapset(), destName, QgsGrassObject::Vector );
      import = new QgsGrassVectorImport( vectorProvider, vectorObject ); // takes provider ownership
    }
    else
    {
      return false;
    }

    connect( import, &QgsGrassImport::finished, this, &QgsGrassMapsetItem::onImportFinished );

    // delete existing files (confirmed before in dialog)
    bool deleteOk = true;
    const auto constNames = import->names();
    for ( const QString &name : constNames )
    {
      QgsGrassObject obj( import->grassObject() );
      obj.setName( name );
      if ( QgsGrass::objectExists( obj ) )
      {
        QgsDebugMsgLevel( name + " exists -> delete", 2 );
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
    sImports.append( import );
    if ( u.layerType == QLatin1String( "raster" ) )
    {
      existingRasters.append( import->names() );
    }
    else if ( u.layerType == QLatin1String( "vector" ) )
    {
      existingVectors.append( import->names() );
    }
    refresh(); // after each new item so that it is visible if dialog is opened on next item
  }

  if ( !errors.isEmpty() )
  {
    QgsMessageOutput::showMessage( tr( "Import to GRASS mapset" ),
                                   tr( "Failed to import some layers!\n\n" ) + errors.join( QLatin1Char( '\n' ) ),
                                   QgsMessageOutput::MessageText );
  }

  return true;
}

void QgsGrassMapsetItem::onImportFinished( QgsGrassImport *import )
{
  if ( !import->error().isEmpty() )
  {
    QgsMessageOutput *output = QgsMessageOutput::createMessageOutput();
    output->setTitle( tr( "Import to GRASS mapset failed" ) );
    output->setMessage( tr( "Failed to import %1 to %2: %3" ).arg( import->srcDescription(),
                        import->grassObject().mapsetPath(),
                        import->error() ), QgsMessageOutput::MessageText );
    output->showMessage();
  }

  sImports.removeOne( import );
  import->deleteLater();
  refresh();
}

void QgsGrassMapsetItem::onDirectoryChanged()
{
  if ( state() == Qgis::BrowserItemState::Populating )
  {
    // schedule to refresh later, because refres() simply returns if Populating
    mRefreshLater = true;
  }
  else
  {
    refresh();
  }
}

void QgsGrassMapsetItem::childrenCreated()
{
  QgsDebugMsgLevel( QStringLiteral( "mRefreshLater = %1" ).arg( mRefreshLater ), 2 );

  if ( mRefreshLater )
  {
    QgsDebugMsgLevel( QStringLiteral( "directory changed during createChildren() -> refresh() again" ), 2 );
    mRefreshLater = false;
    setState( Qgis::BrowserItemState::Populated );
    refresh();
  }
  else
  {
    QgsDirectoryItem::childrenCreated();
  }
}

//------------------------ QgsGrassObjectItem ----------------------------------

QgsGrassObjectItem::QgsGrassObjectItem( QgsDataItem *parent, const QgsGrassObject &grassObject,
                                        const QString &name, const QString &path, const QString &uri,
                                        Qgis::BrowserLayerType layerType, const QString &providerKey )
  : QgsLayerItem( parent, name, path, uri, layerType, providerKey )
  , QgsGrassObjectItemBase( grassObject )
{
  setState( Qgis::BrowserItemState::Populated ); // no children, to show non expandable in browser
#ifdef HAVE_GUI
  mActions = new QgsGrassItemActions( mGrassObject, true, this );
#endif
}

bool QgsGrassObjectItem::equal( const QgsDataItem *other )
{
  const QgsGrassObjectItem *item = qobject_cast<const QgsGrassObjectItem *>( other );
  return QgsLayerItem::equal( other ) && item && mGrassObject == item->mGrassObject;
}

//----------------------- QgsGrassVectorItem ------------------------------

QgsGrassVectorItem::QgsGrassVectorItem( QgsDataItem *parent, const QgsGrassObject &grassObject, const QString &path, const QString &labelName, bool valid )
  : QgsDataCollectionItem( parent, labelName.isEmpty() ? grassObject.name() : labelName, path )
  , QgsGrassObjectItemBase( grassObject )
  , mValid( valid )
{
  QgsDebugMsgLevel( "name = " + grassObject.name() + " path = " + path, 2 );
  mCapabilities = Qgis::BrowserItemCapability::NoCapabilities; // disable Fertile from QgsDataCollectionItem
  setCapabilities( Qgis::BrowserItemCapability::NoCapabilities ); // disable fertility
  if ( !mValid )
  {
    setState( Qgis::BrowserItemState::Populated );
    setIconName( QStringLiteral( "/mIconDelete.svg" ) );
  }
#ifdef HAVE_GUI
  mActions = new QgsGrassItemActions( mGrassObject, mValid, this );
#endif

  QString watchPath = mGrassObject.mapsetPath() + "/vector/" + mGrassObject.name();
  QgsDebugMsgLevel( "add watcher on " + watchPath, 2 );
  // The watcher does not seem to work without parent
  mWatcher = new QFileSystemWatcher( this );
  mWatcher->addPath( watchPath );
  connect( mWatcher, &QFileSystemWatcher::directoryChanged, this, &QgsGrassVectorItem::onDirectoryChanged );
}

QgsGrassVectorItem::~QgsGrassVectorItem()
{
  delete mWatcher;
}

void QgsGrassVectorItem::onDirectoryChanged()
{
  if ( auto *lParent = parent() )
  {
    lParent->refresh();
  }
}

bool QgsGrassVectorItem::equal( const QgsDataItem *other )
{
  if ( QgsDataCollectionItem::equal( other ) )
  {
    const QgsGrassVectorItem *item = qobject_cast<const QgsGrassVectorItem *>( other );
    if ( item && mGrassObject == item->mGrassObject && mValid == item->mValid )
    {
      if ( mChildren.size() == item->mChildren.size() )
      {
        // check children
        for ( int i = 0; i < mChildren.size(); i++ )
        {
          QgsDataItem *child = mChildren.value( i );
          QgsDataItem *otherChild = item->mChildren.value( i );
          if ( !child || !otherChild || !child->equal( otherChild ) )
          {
            return false;
          }
        }
        return true;
      }
    }
  }
  return false;
}

//----------------------- QgsGrassVectorLayerItem ------------------------------

QgsGrassVectorLayerItem::QgsGrassVectorLayerItem( QgsDataItem *parent, const QgsGrassObject &grassObject, const QString &layerName,
    const QString &path, const QString &uri,
    Qgis::BrowserLayerType layerType, bool singleLayer )
  : QgsGrassObjectItem( parent, grassObject, layerName, path, uri, layerType, QStringLiteral( "grass" ) )
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
  const QgsGrassVectorLayerItem *item = qobject_cast<const QgsGrassVectorLayerItem *>( other );
  return QgsGrassObjectItem::equal( other ) && item && mSingleLayer == item->mSingleLayer;
}

//----------------------- QgsGrassRasterItem ------------------------------

QgsGrassRasterItem::QgsGrassRasterItem( QgsDataItem *parent, const QgsGrassObject &grassObject,
                                        const QString &path, const QString &uri, bool isExternal )
  : QgsGrassObjectItem( parent, grassObject, grassObject.name(), path, uri, Qgis::BrowserLayerType::Raster, QStringLiteral( "grassraster" ) )
  , mExternal( isExternal )
{
}

QIcon QgsGrassRasterItem::icon()
{
  if ( mExternal )
  {
    return QgsApplication::getThemeIcon( QStringLiteral( "/mIconRasterLink.svg" ) );
  }
  return QgsDataItem::icon();
}

bool QgsGrassRasterItem::equal( const QgsDataItem *other )
{
  const QgsGrassRasterItem *item = qobject_cast<const QgsGrassRasterItem *>( other );
  return QgsGrassObjectItem::equal( other )  && item && mExternal == item->mExternal;
}

//----------------------- QgsGrassGroupItem ------------------------------

QgsGrassGroupItem::QgsGrassGroupItem( QgsDataItem *parent, const QgsGrassObject &grassObject,
                                      const QString &path, const QString &uri )
  : QgsGrassObjectItem( parent, grassObject, grassObject.name(), path, uri, Qgis::BrowserLayerType::Raster, QStringLiteral( "grassraster" ) )
{
}

QIcon QgsGrassGroupItem::icon()
{
  return QgsApplication::getThemeIcon( QStringLiteral( "/mIconRasterGroup.svg" ) );
}

#ifdef HAVE_GUI
//----------------------- QgsGrassImportItemWidget ------------------------------
QgsGrassImportItemWidget::QgsGrassImportItemWidget( QWidget *parent )
  : QWidget( parent )
{
  QVBoxLayout *layout = new QVBoxLayout( this );

  mTextEdit = new QTextEdit( this );
  mTextEdit->setReadOnly( true );
  layout->addWidget( mTextEdit );

  mProgressBar = new QProgressBar( this );
  layout->addWidget( mProgressBar );
}

void QgsGrassImportItemWidget::setHtml( const QString &html )
{
  if ( mTextEdit )
  {
    mTextEdit->setText( html );
  }
}

void QgsGrassImportItemWidget::onProgressChanged( const QString &recentHtml, const QString &allHtml, int min, int max, int value )
{
  Q_UNUSED( allHtml )
  if ( !recentHtml.isEmpty() )
  {
    mTextEdit->append( recentHtml );
  }
  // TODO: scroll to bottom
  mTextEdit->verticalScrollBar()->setValue( mTextEdit->verticalScrollBar()->maximum() );
  mProgressBar->setRange( min, max );
  mProgressBar->setValue( value );
}
#endif

//----------------------- QgsGrassImportItem ------------------------------

QgsAnimatedIcon *QgsGrassImportItem::sImportIcon = nullptr;

QgsGrassImportItem::QgsGrassImportItem( QgsDataItem *parent, const QString &name, const QString &path, QgsGrassImport *import )
  : QgsDataItem( Qgis::BrowserItemType::Layer, parent, name, path )
  , QgsGrassObjectItemBase( import->grassObject() )
  , mImport( import )
{
  setCapabilities( Qgis::BrowserItemCapability::NoCapabilities ); // disable fertility
  setState( Qgis::BrowserItemState::Populated );

  QgsGrassImportIcon::instance()->connectFrameChanged( this, &QgsGrassImportItem::updateIcon );
}

QgsGrassImportItem::~QgsGrassImportItem()
{
  QgsGrassImportIcon::instance()->disconnectFrameChanged( this, &QgsGrassImportItem::updateIcon );
}

#ifdef HAVE_GUI
QList<QAction *> QgsGrassImportItem::actions( QWidget *parent )
{
  QList<QAction *> lst;

  QAction *actionRename = new QAction( tr( "Cancel" ), parent );
  connect( actionRename, &QAction::triggered, this, &QgsGrassImportItem::cancel );
  lst.append( actionRename );

  return lst;
}

QWidget *QgsGrassImportItem::paramWidget()
{
  QgsGrassImportItemWidget *widget = new QgsGrassImportItemWidget();

  if ( mImport && mImport->progress() )
  {
    connect( mImport->progress(), &QgsGrassImportProgress::progressChanged,
             widget, &QgsGrassImportItemWidget::onProgressChanged );

    widget->setHtml( mImport->progress()->progressHtml() );
  }
  return widget;
}

void QgsGrassImportItem::cancel()
{
  if ( !mImport ) // should not happen
  {
    QgsDebugError( QStringLiteral( "mImport is null" ) );
    return;
  }
  if ( mImport->isCanceled() )
  {
    return;
  }
  mImport->cancel();
  QgsGrassImportIcon::instance()->disconnectFrameChanged( this, &QgsGrassImportItem::updateIcon );
  setName( name() + " : " + tr( "canceling" ) );
  updateIcon();
}
#endif

QIcon QgsGrassImportItem::icon()
{
  if ( mImport && mImport->isCanceled() )
  {
    setIconName( QStringLiteral( "/mIconDelete.svg" ) );
    return QgsDataItem::icon();
  }
  else
  {
    return QgsGrassImportIcon::instance()->icon();
  }
}

//-------------------------------------------------------------------------

static const QString PROVIDER_KEY = QStringLiteral( "grass" );
static const QString PROVIDER_DESCRIPTION = QStringLiteral( "GRASS %1 vector provider" ).arg( GRASS_VERSION_MAJOR );

class QgsGrassDataItemProvider : public QgsDataItemProvider
{
  public:
    QString name() override { return QStringLiteral( "GRASS" ); }

    Qgis::DataItemProviderCapabilities capabilities() const override { return Qgis::DataItemProviderCapability::Directories; }

    QgsDataItem *createDataItem( const QString &dirPath, QgsDataItem *parentItem ) override
    {
      if ( !QgsGrass::init() )
      {
        return nullptr;
      }
      if ( QgsGrass::isLocation( dirPath ) )
      {
        QString path;
        QDir dir( dirPath );
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
        QgsGrassLocationItem *location = new QgsGrassLocationItem( parentItem, dirPath, path );
        return location;
      }
      return nullptr;
    }
};


QgsGrassProviderMetadata::QgsGrassProviderMetadata()
  : QgsProviderMetadata( PROVIDER_KEY, PROVIDER_DESCRIPTION )
{}

QgsDataProvider *QgsGrassProviderMetadata::createProvider( const QString &uri, const QgsDataProvider::ProviderOptions &options, QgsDataProvider::ReadFlags flags )
{
  Q_UNUSED( options )
  Q_UNUSED( flags )
  return new QgsGrassProvider( uri );
}

QList<QgsDataItemProvider *> QgsGrassProviderMetadata::dataItemProviders() const
{
  QList< QgsDataItemProvider * > providers;
  providers << new QgsGrassDataItemProvider;
  return providers;
}

void QgsGrassProviderMetadata::initProvider()
{
  // Init GRASS in the first function called by provider registry so that it is called
  // on main thread, not sure but suspicious that init in thread is causing problems,
  // at least on Windows, not that dataItem() is called in thread
  if ( !QgsGrass::init() )
  {
    QgsDebugError( QStringLiteral( "init failed" ) );
  }
}


QGISEXTERN QgsProviderMetadata *providerMetadataFactory()
{
  return new QgsGrassProviderMetadata();
}


QList<Qgis::LayerType> QgsGrassProviderMetadata::supportedLayerTypes() const
{
  return { Qgis::LayerType::Vector };
}

QIcon QgsGrassProviderMetadata::icon() const
{
  return QgsApplication::getThemeIcon( QStringLiteral( "providerGrass.svg" ) );
}

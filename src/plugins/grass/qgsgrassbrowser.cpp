/*******************************************************************
                              qgsgrassbrowser.cpp
                             -------------------
    begin                : February, 2006
    copyright            : (C) 2006 by Radim Blazek
    email                : radim.blazek@gmail.com
********************************************************************/
/********************************************************************
 This program is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation; either version 2 of the License, or
 (at your option) any later version.
*******************************************************************/
#include "qgsgrassbrowser.h"
#include "qgsgrassmodel.h"
#include "qgsgrassplugin.h"
#include "qgsgrassselect.h"
#include "qgsgrassutils.h"
#include "qgsgrass.h"

#include "qgisinterface.h"
#include "qgslegendinterface.h"
#include "qgsvectorlayer.h"
#include "qgsvectordataprovider.h"
//#include "qgsgrassprovider.h"
#include "qgsapplication.h"
#include "qgslogger.h"

#include <QActionGroup>
#include <QHeaderView>
#include <QMessageBox>
#include <QProcess>
#include <QScrollBar>
#include <QSplitter>
#include <QTextBrowser>
#include <QToolBar>
#include <QTreeView>
#include <QMenu>


QgsGrassBrowser::QgsGrassBrowser( QgisInterface *iface,
                                  QWidget * parent, Qt::WindowFlags f )
    : QMainWindow( parent, Qt::Dialog ), mIface( iface ), mRunningMods( 0 )
{
  Q_UNUSED( f );
  QgsDebugMsg( "QgsGrassBrowser()" );

  QActionGroup *ag = new QActionGroup( this );
  QToolBar *tb = addToolBar( tr( "Tools" ) );

  mActionAddMap = new QAction(
    QgsGrassPlugin::getThemeIcon( "grass_add_map.png" ),
    tr( "Add selected map to canvas" ), this );
  mActionAddMap->setEnabled( false );
  ag->addAction( mActionAddMap );
  tb->addAction( mActionAddMap );
  connect( mActionAddMap, SIGNAL( triggered() ), this, SLOT( addMap() ) );

  mActionCopyMap = new QAction(
    QgsGrassPlugin::getThemeIcon( "grass_copy_map.png" ),
    tr( "Copy selected map" ), this );
  mActionCopyMap->setEnabled( false );
  ag->addAction( mActionCopyMap );
  tb->addAction( mActionCopyMap );
  connect( mActionCopyMap, SIGNAL( triggered() ), this, SLOT( copyMap() ) );

  mActionRenameMap = new QAction(
    QgsGrassPlugin::getThemeIcon( "grass_rename_map.png" ),
    tr( "Rename selected map" ), this );
  mActionRenameMap->setEnabled( false );
  ag->addAction( mActionRenameMap );
  tb->addAction( mActionRenameMap );
  connect( mActionRenameMap, SIGNAL( triggered() ), this, SLOT( renameMap() ) );

  mActionDeleteMap = new QAction(
    QgsGrassPlugin::getThemeIcon( "grass_delete_map.png" ),
    tr( "Delete selected map" ), this );
  mActionDeleteMap->setEnabled( false );
  ag->addAction( mActionDeleteMap );
  tb->addAction( mActionDeleteMap );
  connect( mActionDeleteMap, SIGNAL( triggered() ), this, SLOT( deleteMap() ) );

  mActionSetRegion = new QAction(
    QgsGrassPlugin::getThemeIcon( "grass_set_region.png" ),
    tr( "Set current region to selected map" ), this );
  mActionSetRegion->setEnabled( false );
  ag->addAction( mActionSetRegion );
  tb->addAction( mActionSetRegion );
  connect( mActionSetRegion, SIGNAL( triggered() ), this, SLOT( setRegion() ) );

  mActionRefresh = new QAction(
    QgsGrassPlugin::getThemeIcon( "grass_refresh.png" ),
    tr( "Refresh" ), this );
  ag->addAction( mActionRefresh );
  tb->addAction( mActionRefresh );
  connect( mActionRefresh, SIGNAL( triggered() ), this, SLOT( refresh() ) );

  // Add model
  mModel = new QgsGrassModel( this );

  mTree = new QTreeView( 0 );
  mTree->header()->hide();
  mTree->setModel( mModel );
  mTree->setSelectionMode( QAbstractItemView::ExtendedSelection );
  mTree->setContextMenuPolicy( Qt::CustomContextMenu );

  mTextBrowser = new QTextBrowser( 0 );
  mTextBrowser->setReadOnly( true );

  mSplitter = new QSplitter( 0 );
  mSplitter->addWidget( mTree );
  mSplitter->addWidget( mTextBrowser );

  setCentralWidget( mSplitter );

  connect( mTree, SIGNAL( customContextMenuRequested( const QPoint& ) ),
           this,  SLOT( showContextMenu( const QPoint& ) ) );
  connect( mTree->selectionModel(),
           SIGNAL( selectionChanged( QItemSelection, QItemSelection ) ),
           this, SLOT( selectionChanged( QItemSelection, QItemSelection ) ) );

  connect( mTree->selectionModel(),
           SIGNAL( currentChanged( QModelIndex, QModelIndex ) ),
           this, SLOT( currentChanged( QModelIndex, QModelIndex ) ) );

  connect( mTree, SIGNAL( doubleClicked( QModelIndex ) ),
           this, SLOT( doubleClicked( QModelIndex ) ) );
}

QgsGrassBrowser::~QgsGrassBrowser() { }

void QgsGrassBrowser::refresh()
{
  QgsDebugMsg( "entered." );

  mModel->refresh();
  mTree->update();
}

void QgsGrassBrowser::addMap()
{
  QgsDebugMsg( "entered." );

  QModelIndexList indexes = mTree->selectionModel()->selectedIndexes();

  QList<QModelIndex>::const_iterator it = indexes.begin();
  for ( ; it != indexes.end(); ++it )
  {
    int type = mModel->itemType( *it );
    QString uri = mModel->uri( *it );
    QString mapset = mModel->itemMapset( *it );
    QString map = mModel->itemMap( *it );
    if ( type == QgsGrassModel::Raster )
    {
      QgsDebugMsg( QString( "add raster: %1" ).arg( uri ) );
      //mIface->addRasterLayer( uri, map );
      mIface->addRasterLayer( uri, map, "grassraster" );
    }
    else if ( type == QgsGrassModel::Vector )
    {
      QgsGrassUtils::addVectorLayers( mIface,
                                      QgsGrass::getDefaultGisdbase(),
                                      QgsGrass::getDefaultLocation(),
                                      mapset, map );
    }
    else if ( type == QgsGrassModel::VectorLayer )
    {

      QStringList list = QgsGrass::vectorLayers(
                           QgsGrass::getDefaultGisdbase(),
                           QgsGrass::getDefaultLocation(),
                           mapset, map );

      // TODO: common method for vector names
      QStringList split = uri.split( '/',  QString::SkipEmptyParts );
      QString layer = split.last();

      QString name = QgsGrassUtils::vectorLayerName(
                       map, layer, list.size() );

      mIface->addVectorLayer( uri, name, "grass" );
    }
    else if ( type == QgsGrassModel::Region )
    {
      struct Cell_head window;
      if ( !getItemRegion( *it, &window ) )
        continue;
      writeRegion( &window );
    }
  }
}

void QgsGrassBrowser::doubleClicked( const QModelIndex & index )
{
  Q_UNUSED( index );
  QgsDebugMsg( "entered." );

  addMap();
}

void QgsGrassBrowser::showContextMenu( const QPoint &position )
{
  QList<QAction *> actions;
  if ( mTree->indexAt( position ).isValid() )
  {
    actions.append( mActionAddMap );
    actions.append( mActionDeleteMap );
    actions.append( mActionCopyMap );
    actions.append( mActionRenameMap );
    actions.append( mActionSetRegion );
  }
  if ( actions.count() > 0 )
    QMenu::exec( actions, mTree->mapToGlobal( position ) );
}

QString QgsGrassBrowser::formatMessage( QString msg )
{
  return msg.replace( "<", "&lt;" ).replace( ">", "&gt;" ).replace( "\n", "<br>" );
}

void QgsGrassBrowser::copyMap()
{
  QgsDebugMsg( "entered." );

  // Filter VectorLayer type from selection
  QModelIndexList indexes;
  foreach ( QModelIndex index, mTree->selectionModel()->selectedIndexes() )
  {
    int type = mModel->itemType( index );
    if ( type != QgsGrassModel::VectorLayer )
    {
      indexes << index;
    }
  }

  QList<QModelIndex>::const_iterator it = indexes.begin();
  for ( ; it != indexes.end(); ++it )
  {
    int type = mModel->itemType( *it );
    QString mapset = mModel->itemMapset( *it );
    QString map = mModel->itemMap( *it );

    QString typeName;
    QString element;
    if ( type == QgsGrassModel::Raster )
    {
      element = "cell";
      typeName = "rast";
    }
    else if ( type == QgsGrassModel::Vector )
    {
      element = "vector";
      typeName = "vect";
    }
    else if ( type == QgsGrassModel::Region )
    {
      element = "windows";
      typeName = "region";
    }

    QgsGrassElementDialog ed( this );
    bool ok;
    QString source;
    QString suggest;
    if ( mapset == QgsGrass::getDefaultMapset() )
    {
      source = map;
    }
    else
    {
      suggest = map;
    }
    QString newName = ed.getItem( element, tr( "New name" ),
                                  tr( "New name for layer \"%1\"" ).arg( map ), suggest, source, &ok );

    if ( !ok )
      return;

    QString module = "g.copy";
#ifdef WIN32
    module.append( ".exe" );
#endif
    QProcess process( this );
    QStringList args( typeName + "=" + map + "@" + mapset + "," + newName );
    process.start( module, args );
    if ( !process.waitForFinished() || process.exitCode() != 0 )
    {
      QString output( process.readAllStandardOutput() );
      QString error( process.readAllStandardError() );
      QMessageBox::warning( 0, tr( "Warning" ),
                            tr( "Cannot copy map %1@%2" ).arg( map ).arg( mapset )
                            + tr( "<br>command: %1 %2<br>%3<br>%4" )
                            .arg( module ).arg( args.join( " " ) )
                            .arg( output ).arg( error ) );
    }
    else
    {
      refresh();
    }
  }
}

void QgsGrassBrowser::renameMap()
{
  QgsDebugMsg( "entered." );

  // Filter VectorLayer type from selection
  QModelIndexList indexes;
  foreach ( QModelIndex index, mTree->selectionModel()->selectedIndexes() )
  {
    int type = mModel->itemType( index );
    if ( type != QgsGrassModel::VectorLayer )
    {
      indexes << index;
    }
  }

  QList<QModelIndex>::const_iterator it = indexes.begin();
  for ( ; it != indexes.end(); ++it )
  {
    int type = mModel->itemType( *it );
    QString mapset = mModel->itemMapset( *it );
    QString map = mModel->itemMap( *it );

    if ( mapset != QgsGrass::getDefaultMapset() )
      continue; // should not happen

    QString typeName;
    QString element;
    if ( type == QgsGrassModel::Raster )
    {
      element = "cell";
      typeName = "rast";
    }
    else if ( type == QgsGrassModel::Vector )
    {
      element = "vector";
      typeName = "vect";
    }
    else if ( type == QgsGrassModel::Region )
    {
      element = "windows";
      typeName = "region";
    }

    QgsGrassElementDialog ed( this );
    bool ok;
    QString newName = ed.getItem( element, tr( "New name" ),
                                  tr( "New name for layer \"%1\"" ).arg( map ), "", map, &ok );

    if ( !ok )
      return;

    QString module = "g.rename";
#ifdef WIN32
    module.append( ".exe" );
#endif
    QProcess process( this );
    QStringList args( typeName + "=" + map + "," + newName );
    process.start( module, QStringList( typeName + "=" + map + "," + newName ) );
    if ( !process.waitForFinished() || process.exitCode() != 0 )
    {
      QString output( process.readAllStandardOutput() );
      QString error( process.readAllStandardError() );
      QMessageBox::warning( 0, tr( "Warning" ),
                            tr( "Cannot rename map %1" ).arg( map )
                            + tr( "<br>command: %1 %2<br>%3<br>%4" )
                            .arg( module ).arg( args.join( " " ) )
                            .arg( output ).arg( error ) );
    }
    else
    {
      refresh();
    }
  }
}

void QgsGrassBrowser::deleteMap()
{
  QgsDebugMsg( "entered." );

  QString gisbase = QgsGrass::getDefaultGisdbase();
  QString location = QgsGrass::getDefaultLocation();

  // Filter VectorLayer type from selection
  QModelIndexList indexes;
  foreach ( QModelIndex index, mTree->selectionModel()->selectedIndexes() )
  {
    int type = mModel->itemType( index );
    QString mapset = mModel->itemMapset( index );
    QString map = mModel->itemMap( index );

    // check whether the layer is loaded in canvas
    if ( type == QgsGrassModel::Vector )
    {
      QStringList layers = QgsGrass::vectorLayers( gisbase, location, mapset, map );
      for ( int i = 0; i < layers.count(); i++ )
      {
        QString uri = gisbase + "/" + location + "/"
                      + mapset + "/" + map + "/" + layers[i];

        foreach ( QgsMapLayer *layer, mIface->legendInterface()->layers() )
        {
          QgsVectorLayer *vl = qobject_cast<QgsVectorLayer *>( layer );
          if ( vl && vl->dataProvider()->name() == "grass" && vl->source() == uri )
          {
#if 0
            /* The following lines allow to delete a grass vector layer
              even if it's loaded in canvas, but it needs to compile
              the grass plugin against the grass provider.
              This causes a known problem on OSX
              (see at http://hub.qgis.org/issues/3999) */

            // the layer is loaded in canvas,
            // freeze it! (this allow to delete it from the mapset)
            QgsGrassProvider * grassProvider =
              qobject_cast<QgsGrassProvider *>( vl->dataProvider() );
            if ( grassProvider )
              grassProvider->freeze();
#else
            QMessageBox::information( this, tr( "Information" ),
                                      tr( "Remove the selected layer(s) from canvas before continue." ) );
            return;
#endif
          }
        }
      }
    }

    if ( type != QgsGrassModel::VectorLayer )
    {
      indexes << index;
    }
  }

  if ( QMessageBox::question( this, tr( "Question" ),
                              tr( "Are you sure you want to delete %n selected layer(s)?", "number of layers to delete", indexes.size() ),
                              QMessageBox::Yes | QMessageBox::No ) == QMessageBox::No )
  {
    return;
  }

  QList<QModelIndex>::const_iterator it = indexes.begin();
  for ( ; it != indexes.end(); ++it )
  {
    int type = mModel->itemType( *it );
    QString mapset = mModel->itemMapset( *it );
    QString map = mModel->itemMap( *it );

    if ( mapset != QgsGrass::getDefaultMapset() )
    {
      continue; // should not happen
    }

    QgsGrassObject::Type mapType;
    if ( type == QgsGrassModel::Raster )
      mapType = QgsGrassObject::Raster;
    else if ( type == QgsGrassModel::Vector )
      mapType = QgsGrassObject::Vector;
    else if ( type == QgsGrassModel::Region )
      mapType = QgsGrassObject::Region;

    QgsGrassObject object( gisbase, location, mapset, map, mapType );

    if ( QgsGrass::deleteObject( object ) )
    {
      refresh();
    }
  }
}

void QgsGrassBrowser::setRegion()
{
  QgsDebugMsg( "entered." );

  struct Cell_head window;

  QModelIndexList indexes = mTree->selectionModel()->selectedIndexes();

  // TODO multiple selection - extent region to all maps
  QList<QModelIndex>::const_iterator it = indexes.begin();
  for ( ; it != indexes.end(); ++it )
  {
    if ( !getItemRegion( *it, &window ) )
      return;
  }
  writeRegion( &window );
}

void QgsGrassBrowser::writeRegion( struct Cell_head *window )
{
  QgsDebugMsg( "QgsGrassBrowser::writeRegion()" );

  QgsGrass::setMapset( QgsGrass::getDefaultGisdbase(),
                       QgsGrass::getDefaultLocation(),
                       QgsGrass::getDefaultMapset() );

  if ( G_put_window( window ) == -1 )
  {
    QMessageBox::warning( 0, tr( "Warning" ),
                          tr( "Cannot write new region" ) );
    return;
  }
  emit regionChanged();
}

bool QgsGrassBrowser::getItemRegion( const QModelIndex & index, struct Cell_head *window )
{
  QgsDebugMsg( "entered." );

  int type = mModel->itemType( index );
  QString mapset = mModel->itemMapset( index );
  QString map = mModel->itemMap( index );

  QgsGrassObject::Type mapType = QgsGrassObject::Raster; //default in case no case matches
  switch ( type )
  {
    case QgsGrassModel::Raster :
      mapType = QgsGrassObject::Raster;
      break;
    case QgsGrassModel::Vector :
      mapType = QgsGrassObject::Vector;
      break;
    case QgsGrassModel::Region :
      mapType = QgsGrassObject::Region;
      break;
    default:
      break;
  }

  return QgsGrass::mapRegion( mapType, QgsGrass::getDefaultGisdbase(),
                              QgsGrass::getDefaultLocation(), mapset, map, window );
}

void QgsGrassBrowser::selectionChanged( const QItemSelection & selected, const QItemSelection & deselected )
{
  Q_UNUSED( deselected );
  QgsDebugMsg( "entered." );

  mActionAddMap->setEnabled( false );
  mActionCopyMap->setEnabled( false );
  mActionRenameMap->setEnabled( false );
  mActionDeleteMap->setEnabled( false );
  mActionSetRegion->setEnabled( false );

  QModelIndexList indexes = selected.indexes();

  mTextBrowser->clear();

  QList<QModelIndex>::const_iterator it = indexes.begin();
  for ( ; it != indexes.end(); ++it )
  {
    mTextBrowser->append( mModel->itemInfo( *it ) );
    mTextBrowser->verticalScrollBar()->setValue( 0 );

    int type = mModel->itemType( *it );

    if ( type == QgsGrassModel::Raster ||
         type == QgsGrassModel::Vector ||
         type == QgsGrassModel::VectorLayer )
    {
      mActionAddMap->setEnabled( true );
    }
    if ( type == QgsGrassModel::Raster || type == QgsGrassModel::Vector || type == QgsGrassModel::Region )
    {
      mActionSetRegion->setEnabled( true );
      mActionCopyMap->setEnabled( true );

      QString mapset = mModel->itemMapset( *it );
      if ( mapset == QgsGrass::getDefaultMapset() )
      {
        mActionDeleteMap->setEnabled( true );
        mActionRenameMap->setEnabled( true );
      }
    }
  }
}

void QgsGrassBrowser::currentChanged( const QModelIndex & current, const QModelIndex & previous )
{
  Q_UNUSED( current );
  Q_UNUSED( previous );
  QgsDebugMsg( "entered." );
}

void QgsGrassBrowser::setLocation( const QString &gisbase, const QString &location )
{
  mModel->setLocation( gisbase, location );
}

void QgsGrassBrowser::moduleStarted()
{
  mActionRefresh->setDisabled( ++mRunningMods > 0 );
}

void QgsGrassBrowser::moduleFinished()
{
  mActionRefresh->setDisabled( --mRunningMods > 0 );
}

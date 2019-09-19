/***************************************************************************
    qgssvgselectorwidget.cpp - group and preview selector for SVG files
                               built off of work in qgssymbollayerwidget

    ---------------------
    begin                : April 2, 2013
    copyright            : (C) 2013 by Larry Shaffer
    email                : larrys at dakcarto dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "qgssvgselectorwidget.h"

#include "qgsapplication.h"
#include "qgslogger.h"
#include "qgspathresolver.h"
#include "qgsproject.h"
#include "qgssvgcache.h"
#include "qgssymbollayerutils.h"
#include "qgssettings.h"
#include "qgsgui.h"

#include <QAbstractListModel>
#include <QCheckBox>
#include <QDir>
#include <QFileDialog>
#include <QModelIndex>
#include <QPixmapCache>
#include <QStyle>
#include <QTime>
#include <QMenu>

// QgsSvgSelectorLoader

///@cond PRIVATE
QgsSvgSelectorLoader::QgsSvgSelectorLoader( QObject *parent )
  : QThread( parent )
{
}

QgsSvgSelectorLoader::~QgsSvgSelectorLoader()
{
  stop();
}

void QgsSvgSelectorLoader::run()
{
  mCanceled = false;
  mQueuedSvgs.clear();
  mTraversedPaths.clear();

  // start with a small initial timeout (ms)
  mTimerThreshold = 10;
  mTimer.start();

  loadPath( mPath );

  if ( !mQueuedSvgs.isEmpty() )
  {
    // make sure we notify model of any remaining queued svgs (ie svgs added since last foundSvgs() signal was emitted)
    emit foundSvgs( mQueuedSvgs );
  }
  mQueuedSvgs.clear();
}

void QgsSvgSelectorLoader::stop()
{
  mCanceled = true;
  while ( isRunning() ) {}
}

void QgsSvgSelectorLoader::loadPath( const QString &path )
{
  if ( mCanceled )
    return;

  // QgsDebugMsg( QStringLiteral( "loading path: %1" ).arg( path ) );

  if ( path.isEmpty() )
  {
    QStringList svgPaths = QgsApplication::svgPaths();
    const auto constSvgPaths = svgPaths;
    for ( const QString &svgPath : constSvgPaths )
    {
      if ( mCanceled )
        return;

      if ( !svgPath.isEmpty() )
      {
        loadPath( svgPath );
      }
    }
  }
  else
  {
    QDir dir( path );

    //guard against circular symbolic links
    QString canonicalPath = dir.canonicalPath();
    if ( mTraversedPaths.contains( canonicalPath ) )
      return;

    mTraversedPaths.insert( canonicalPath );

    loadImages( path );

    const auto constEntryList = dir.entryList( QDir::Dirs | QDir::NoDotAndDotDot );
    for ( const QString &item : constEntryList )
    {
      if ( mCanceled )
        return;

      QString newPath = dir.path() + '/' + item;
      loadPath( newPath );
      // QgsDebugMsg( QStringLiteral( "added path: %1" ).arg( newPath ) );
    }
  }
}

void QgsSvgSelectorLoader::loadImages( const QString &path )
{
  QDir dir( path );
  const auto constEntryList = dir.entryList( QStringList( "*.svg" ), QDir::Files );
  for ( const QString &item : constEntryList )
  {
    if ( mCanceled )
      return;

    // TODO test if it is correct SVG
    QString svgPath = dir.path() + '/' + item;
    // QgsDebugMsg( QStringLiteral( "adding svg: %1" ).arg( svgPath ) );

    // add it to the list of queued SVGs
    mQueuedSvgs << svgPath;

    // we need to avoid spamming the model with notifications about new svgs, so foundSvgs
    // is only emitted for blocks of SVGs (otherwise the view goes all flickery)
    if ( mTimer.elapsed() > mTimerThreshold && !mQueuedSvgs.isEmpty() )
    {
      emit foundSvgs( mQueuedSvgs );
      mQueuedSvgs.clear();

      // increase the timer threshold - this ensures that the first lots of svgs loaded are added
      // to the view quickly, but as the list grows new svgs are added at a slower rate.
      // ie, good for initial responsiveness but avoid being spammy as the list grows.
      if ( mTimerThreshold < 1000 )
        mTimerThreshold *= 2;
      mTimer.restart();
    }
  }
}


//
// QgsSvgGroupLoader
//

QgsSvgGroupLoader::QgsSvgGroupLoader( QObject *parent )
  : QThread( parent )
{

}

QgsSvgGroupLoader::~QgsSvgGroupLoader()
{
  stop();
}

void QgsSvgGroupLoader::run()
{
  mCanceled = false;
  mTraversedPaths.clear();

  while ( !mCanceled && !mParentPaths.isEmpty() )
  {
    QString parentPath = mParentPaths.takeFirst();
    loadGroup( parentPath );
  }
}

void QgsSvgGroupLoader::stop()
{
  mCanceled = true;
  while ( isRunning() ) {}
}

void QgsSvgGroupLoader::loadGroup( const QString &parentPath )
{
  QDir parentDir( parentPath );

  //guard against circular symbolic links
  QString canonicalPath = parentDir.canonicalPath();
  if ( mTraversedPaths.contains( canonicalPath ) )
    return;

  mTraversedPaths.insert( canonicalPath );

  const auto constEntryList = parentDir.entryList( QDir::Dirs | QDir::NoDotAndDotDot );
  for ( const QString &item : constEntryList )
  {
    if ( mCanceled )
      return;

    emit foundPath( parentPath, item );
    mParentPaths.append( parentDir.path() + '/' + item );
  }
}

///@endcond

//,
// QgsSvgSelectorListModel
//

QgsSvgSelectorListModel::QgsSvgSelectorListModel( QObject *parent, int iconSize )
  : QAbstractListModel( parent )
  , mSvgLoader( new QgsSvgSelectorLoader( this ) )
  , mIconSize( iconSize )
{
  mSvgLoader->setPath( QString() );
  connect( mSvgLoader, &QgsSvgSelectorLoader::foundSvgs, this, &QgsSvgSelectorListModel::addSvgs );
  mSvgLoader->start();
}

QgsSvgSelectorListModel::QgsSvgSelectorListModel( QObject *parent, const QString &path, int iconSize )
  : QAbstractListModel( parent )
  , mSvgLoader( new QgsSvgSelectorLoader( this ) )
  , mIconSize( iconSize )
{
  mSvgLoader->setPath( path );
  connect( mSvgLoader, &QgsSvgSelectorLoader::foundSvgs, this, &QgsSvgSelectorListModel::addSvgs );
  mSvgLoader->start();
}

int QgsSvgSelectorListModel::rowCount( const QModelIndex &parent ) const
{
  Q_UNUSED( parent )
  return mSvgFiles.count();
}

QPixmap QgsSvgSelectorListModel::createPreview( const QString &entry ) const
{
  // render SVG file
  QColor fill, stroke;
  double strokeWidth, fillOpacity, strokeOpacity;
  bool fillParam, fillOpacityParam, strokeParam, strokeWidthParam, strokeOpacityParam;
  bool hasDefaultFillColor = false, hasDefaultFillOpacity = false, hasDefaultStrokeColor = false,
       hasDefaultStrokeWidth = false, hasDefaultStrokeOpacity = false;
  QgsApplication::svgCache()->containsParams( entry, fillParam, hasDefaultFillColor, fill,
      fillOpacityParam, hasDefaultFillOpacity, fillOpacity,
      strokeParam, hasDefaultStrokeColor, stroke,
      strokeWidthParam, hasDefaultStrokeWidth, strokeWidth,
      strokeOpacityParam, hasDefaultStrokeOpacity, strokeOpacity );

  //if defaults not set in symbol, use these values
  if ( !hasDefaultFillColor )
    fill = QColor( 200, 200, 200 );
  fill.setAlphaF( hasDefaultFillOpacity ? fillOpacity : 1.0 );
  if ( !hasDefaultStrokeColor )
    stroke = Qt::black;
  stroke.setAlphaF( hasDefaultStrokeOpacity ? strokeOpacity : 1.0 );
  if ( !hasDefaultStrokeWidth )
    strokeWidth = 0.2;

  bool fitsInCache; // should always fit in cache at these sizes (i.e. under 559 px ^ 2, or half cache size)
  QImage img = QgsApplication::svgCache()->svgAsImage( entry, mIconSize, fill, stroke, strokeWidth, 3.5 /*appr. 88 dpi*/, fitsInCache );
  return QPixmap::fromImage( img );
}

QVariant QgsSvgSelectorListModel::data( const QModelIndex &index, int role ) const
{
  QString entry = mSvgFiles.at( index.row() );

  if ( role == Qt::DecorationRole ) // icon
  {
    QPixmap pixmap;
    if ( !QPixmapCache::find( entry, pixmap ) )
    {
      pixmap = createPreview( entry );
      QPixmapCache::insert( entry, pixmap );
    }

    return pixmap;
  }
  else if ( role == Qt::UserRole || role == Qt::ToolTipRole )
  {
    return entry;
  }

  return QVariant();
}

void QgsSvgSelectorListModel::addSvgs( const QStringList &svgs )
{
  beginInsertRows( QModelIndex(), mSvgFiles.count(), mSvgFiles.count() + svgs.size() - 1 );
  mSvgFiles.append( svgs );
  endInsertRows();
}





//--- QgsSvgSelectorGroupsModel

QgsSvgSelectorGroupsModel::QgsSvgSelectorGroupsModel( QObject *parent )
  : QStandardItemModel( parent )
  , mLoader( new QgsSvgGroupLoader( this ) )
{
  QStringList svgPaths = QgsApplication::svgPaths();
  QStandardItem *parentItem = invisibleRootItem();
  QStringList parentPaths;
  parentPaths.reserve( svgPaths.size() );

  for ( int i = 0; i < svgPaths.size(); i++ )
  {
    QDir dir( svgPaths.at( i ) );
    QStandardItem *baseGroup = nullptr;

    if ( dir.path().contains( QgsApplication::pkgDataPath() ) )
    {
      baseGroup = new QStandardItem( tr( "App Symbols" ) );
    }
    else if ( dir.path().contains( QgsApplication::qgisSettingsDirPath() ) )
    {
      baseGroup = new QStandardItem( tr( "User Symbols" ) );
    }
    else
    {
      baseGroup = new QStandardItem( dir.dirName() );
    }
    baseGroup->setData( QVariant( svgPaths.at( i ) ) );
    baseGroup->setEditable( false );
    baseGroup->setCheckable( false );
    baseGroup->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "mIconFolder.svg" ) ) );
    baseGroup->setToolTip( dir.path() );
    parentItem->appendRow( baseGroup );
    parentPaths << svgPaths.at( i );
    mPathItemHash.insert( svgPaths.at( i ), baseGroup );
    QgsDebugMsg( QStringLiteral( "SVG base path %1: %2" ).arg( i ).arg( baseGroup->data().toString() ) );
  }
  mLoader->setParentPaths( parentPaths );
  connect( mLoader, &QgsSvgGroupLoader::foundPath, this, &QgsSvgSelectorGroupsModel::addPath );
  mLoader->start();
}

QgsSvgSelectorGroupsModel::~QgsSvgSelectorGroupsModel()
{
  mLoader->stop();
}

void QgsSvgSelectorGroupsModel::addPath( const QString &parentPath, const QString &item )
{
  QStandardItem *parentGroup = mPathItemHash.value( parentPath );
  if ( !parentGroup )
    return;

  QString fullPath = parentPath + '/' + item;
  QStandardItem *group = new QStandardItem( item );
  group->setData( QVariant( fullPath ) );
  group->setEditable( false );
  group->setCheckable( false );
  group->setToolTip( fullPath );
  group->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "mIconFolder.svg" ) ) );
  parentGroup->appendRow( group );
  mPathItemHash.insert( fullPath, group );
}


//-- QgsSvgSelectorWidget

QgsSvgSelectorWidget::QgsSvgSelectorWidget( QWidget *parent )
  : QWidget( parent )
{
  // TODO: in-code gui setup with option to vertically or horizontally stack SVG groups/images widgets
  setupUi( this );

  connect( mSvgSourceLineEdit, &QgsAbstractFileContentSourceLineEdit::sourceChanged, this, &QgsSvgSelectorWidget::svgSourceChanged );

  mIconSize = std::max( 30, static_cast< int >( std::round( Qgis::UI_SCALE_FACTOR * fontMetrics().width( QStringLiteral( "XXXX" ) ) ) ) );
  mImagesListView->setGridSize( QSize( mIconSize * 1.2, mIconSize * 1.2 ) );

  mGroupsTreeView->setHeaderHidden( true );
  populateList();

  connect( mImagesListView->selectionModel(), &QItemSelectionModel::currentChanged,
           this, &QgsSvgSelectorWidget::svgSelectionChanged );
  connect( mGroupsTreeView->selectionModel(), &QItemSelectionModel::currentChanged,
           this, &QgsSvgSelectorWidget::populateIcons );
}

void QgsSvgSelectorWidget::setSvgPath( const QString &svgPath )
{
  mCurrentSvgPath = svgPath;

  whileBlocking( mSvgSourceLineEdit )->setSource( svgPath );

  mImagesListView->selectionModel()->blockSignals( true );
  QAbstractItemModel *m = mImagesListView->model();
  QItemSelectionModel *selModel = mImagesListView->selectionModel();
  for ( int i = 0; i < m->rowCount(); i++ )
  {
    QModelIndex idx( m->index( i, 0 ) );
    if ( m->data( idx ).toString() == svgPath )
    {
      selModel->select( idx, QItemSelectionModel::SelectCurrent );
      selModel->setCurrentIndex( idx, QItemSelectionModel::SelectCurrent );
      mImagesListView->scrollTo( idx );
      break;
    }
  }
  mImagesListView->selectionModel()->blockSignals( false );
}

QString QgsSvgSelectorWidget::currentSvgPath() const
{
  return mCurrentSvgPath;
}

void QgsSvgSelectorWidget::updateCurrentSvgPath( const QString &svgPath )
{
  mCurrentSvgPath = svgPath;
  emit svgSelected( currentSvgPath() );
}

void QgsSvgSelectorWidget::svgSelectionChanged( const QModelIndex &idx )
{
  QString filePath = idx.data( Qt::UserRole ).toString();
  whileBlocking( mSvgSourceLineEdit )->setSource( filePath );
  updateCurrentSvgPath( filePath );
}

void QgsSvgSelectorWidget::populateIcons( const QModelIndex &idx )
{
  QString path = idx.data( Qt::UserRole + 1 ).toString();

  QAbstractItemModel *oldModel = mImagesListView->model();
  QgsSvgSelectorListModel *m = new QgsSvgSelectorListModel( mImagesListView, path, mIconSize );
  mImagesListView->setModel( m );
  delete oldModel; //explicitly delete old model to force any background threads to stop

  connect( mImagesListView->selectionModel(), &QItemSelectionModel::currentChanged,
           this, &QgsSvgSelectorWidget::svgSelectionChanged );
}

void QgsSvgSelectorWidget::svgSourceChanged( const QString &text )
{
  QString resolvedPath = QgsSymbolLayerUtils::svgSymbolNameToPath( text, QgsProject::instance()->pathResolver() );
  bool validSVG = !resolvedPath.isNull();

  updateCurrentSvgPath( validSVG ? resolvedPath : text );
}

void QgsSvgSelectorWidget::populateList()
{
  QgsSvgSelectorGroupsModel *g = new QgsSvgSelectorGroupsModel( mGroupsTreeView );
  mGroupsTreeView->setModel( g );
  // Set the tree expanded at the first level
  int rows = g->rowCount( g->indexFromItem( g->invisibleRootItem() ) );
  for ( int i = 0; i < rows; i++ )
  {
    mGroupsTreeView->setExpanded( g->indexFromItem( g->item( i ) ), true );
  }

  // Initially load the icons in the List view without any grouping
  QAbstractItemModel *oldModel = mImagesListView->model();
  QgsSvgSelectorListModel *m = new QgsSvgSelectorListModel( mImagesListView );
  mImagesListView->setModel( m );
  delete oldModel; //explicitly delete old model to force any background threads to stop
}

//-- QgsSvgSelectorDialog

QgsSvgSelectorDialog::QgsSvgSelectorDialog( QWidget *parent, Qt::WindowFlags fl,
    QDialogButtonBox::StandardButtons buttons,
    Qt::Orientation orientation )
  : QDialog( parent, fl )
{
  // TODO: pass 'orientation' to QgsSvgSelectorWidget for customizing its layout, once implemented
  Q_UNUSED( orientation )

  // create buttonbox
  mButtonBox = new QDialogButtonBox( buttons, orientation, this );
  connect( mButtonBox, &QDialogButtonBox::accepted, this, &QDialog::accept );
  connect( mButtonBox, &QDialogButtonBox::rejected, this, &QDialog::reject );

  setMinimumSize( 480, 320 );

  // dialog's layout
  mLayout = new QVBoxLayout();
  mSvgSelector = new QgsSvgSelectorWidget( this );
  mLayout->addWidget( mSvgSelector );

  mLayout->addWidget( mButtonBox );
  setLayout( mLayout );
}


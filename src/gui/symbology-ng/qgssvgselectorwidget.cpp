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
#include "qgsproject.h"
#include "qgssvgcache.h"
#include "qgssymbollayerutils.h"

#include <QAbstractListModel>
#include <QCheckBox>
#include <QDir>
#include <QFileDialog>
#include <QModelIndex>
#include <QPixmapCache>
#include <QSettings>
#include <QStyle>
#include <QTime>

// QgsSvgSelectorLoader

///@cond PRIVATE
QgsSvgSelectorLoader::QgsSvgSelectorLoader( QObject* parent )
    : QThread( parent )
    , mCancelled( false )
    , mTimerThreshold( 0 )
{
}

QgsSvgSelectorLoader::~QgsSvgSelectorLoader()
{
  stop();
}

void QgsSvgSelectorLoader::run()
{
  mCancelled = false;
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
  mCancelled = true;
  while ( isRunning() ) {}
}

void QgsSvgSelectorLoader::loadPath( const QString& path )
{
  if ( mCancelled )
    return;

  // QgsDebugMsg( QString( "loading path: %1" ).arg( path ) );

  if ( path.isEmpty() )
  {
    QStringList svgPaths = QgsApplication::svgPaths();
    Q_FOREACH ( const QString& svgPath, svgPaths )
    {
      if ( mCancelled )
        return;

      loadPath( svgPath );
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

    Q_FOREACH ( const QString& item, dir.entryList( QDir::Dirs | QDir::NoDotAndDotDot ) )
    {
      if ( mCancelled )
        return;

      QString newPath = dir.path() + '/' + item;
      loadPath( newPath );
      // QgsDebugMsg( QString( "added path: %1" ).arg( newPath ) );
    }
  }
}

void QgsSvgSelectorLoader::loadImages( const QString& path )
{
  QDir dir( path );
  Q_FOREACH ( const QString& item, dir.entryList( QStringList( "*.svg" ), QDir::Files ) )
  {
    if ( mCancelled )
      return;

    // TODO test if it is correct SVG
    QString svgPath = dir.path() + '/' + item;
    // QgsDebugMsg( QString( "adding svg: %1" ).arg( svgPath ) );

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

QgsSvgGroupLoader::QgsSvgGroupLoader( QObject* parent )
    : QThread( parent )
    , mCancelled( false )
{

}

QgsSvgGroupLoader::~QgsSvgGroupLoader()
{
  stop();
}

void QgsSvgGroupLoader::run()
{
  mCancelled = false;
  mTraversedPaths.clear();

  while ( !mCancelled && !mParentPaths.isEmpty() )
  {
    QString parentPath = mParentPaths.takeFirst();
    loadGroup( parentPath );
  }
}

void QgsSvgGroupLoader::stop()
{
  mCancelled = true;
  while ( isRunning() ) {}
}

void QgsSvgGroupLoader::loadGroup( const QString& parentPath )
{
  QDir parentDir( parentPath );

  //guard against circular symbolic links
  QString canonicalPath = parentDir.canonicalPath();
  if ( mTraversedPaths.contains( canonicalPath ) )
    return;

  mTraversedPaths.insert( canonicalPath );

  Q_FOREACH ( const QString& item, parentDir.entryList( QDir::Dirs | QDir::NoDotAndDotDot ) )
  {
    if ( mCancelled )
      return;

    emit foundPath( parentPath, item );
    mParentPaths.append( parentDir.path() + '/' + item );
  }
}

///@endcond

//,
// QgsSvgSelectorListModel
//

QgsSvgSelectorListModel::QgsSvgSelectorListModel( QObject* parent )
    : QAbstractListModel( parent )
    , mSvgLoader( new QgsSvgSelectorLoader( this ) )
{
  mSvgLoader->setPath( QString() );
  connect( mSvgLoader, SIGNAL( foundSvgs( QStringList ) ), this, SLOT( addSvgs( QStringList ) ) );
  mSvgLoader->start();
}

QgsSvgSelectorListModel::QgsSvgSelectorListModel( QObject* parent, const QString& path )
    : QAbstractListModel( parent )
    , mSvgLoader( new QgsSvgSelectorLoader( this ) )
{
  mSvgLoader->setPath( path );
  connect( mSvgLoader, SIGNAL( foundSvgs( QStringList ) ), this, SLOT( addSvgs( QStringList ) ) );
  mSvgLoader->start();
}

int QgsSvgSelectorListModel::rowCount( const QModelIndex& parent ) const
{
  Q_UNUSED( parent );
  return mSvgFiles.count();
}

QPixmap QgsSvgSelectorListModel::createPreview( const QString& entry ) const
{
  // render SVG file
  QColor fill, outline;
  double outlineWidth, fillOpacity, outlineOpacity;
  bool fillParam, fillOpacityParam, outlineParam, outlineWidthParam, outlineOpacityParam;
  bool hasDefaultFillColor = false, hasDefaultFillOpacity = false, hasDefaultOutlineColor = false,
                             hasDefaultOutlineWidth = false, hasDefaultOutlineOpacity = false;
  QgsSvgCache::instance()->containsParams( entry, fillParam, hasDefaultFillColor, fill,
      fillOpacityParam, hasDefaultFillOpacity, fillOpacity,
      outlineParam, hasDefaultOutlineColor, outline,
      outlineWidthParam, hasDefaultOutlineWidth, outlineWidth,
      outlineOpacityParam, hasDefaultOutlineOpacity, outlineOpacity );

  //if defaults not set in symbol, use these values
  if ( !hasDefaultFillColor )
    fill = QColor( 200, 200, 200 );
  fill.setAlphaF( hasDefaultFillOpacity ? fillOpacity : 1.0 );
  if ( !hasDefaultOutlineColor )
    outline = Qt::black;
  outline.setAlphaF( hasDefaultOutlineOpacity ? outlineOpacity : 1.0 );
  if ( !hasDefaultOutlineWidth )
    outlineWidth = 0.2;

  bool fitsInCache; // should always fit in cache at these sizes (i.e. under 559 px ^ 2, or half cache size)
  const QImage& img = QgsSvgCache::instance()->svgAsImage( entry, 30.0, fill, outline, outlineWidth, 3.5 /*appr. 88 dpi*/, 1.0, fitsInCache );
  return QPixmap::fromImage( img );
}

QVariant QgsSvgSelectorListModel::data( const QModelIndex& index, int role ) const
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

void QgsSvgSelectorListModel::addSvgs( const QStringList& svgs )
{
  beginInsertRows( QModelIndex(), mSvgFiles.count(), mSvgFiles.count() + svgs.size() - 1 );
  mSvgFiles.append( svgs );
  endInsertRows();
}





//--- QgsSvgSelectorGroupsModel

QgsSvgSelectorGroupsModel::QgsSvgSelectorGroupsModel( QObject* parent )
    : QStandardItemModel( parent )
    , mLoader( new QgsSvgGroupLoader( this ) )
{
  QStringList svgPaths = QgsApplication::svgPaths();
  QStandardItem *parentItem = invisibleRootItem();
  QStringList parentPaths;

  for ( int i = 0; i < svgPaths.size(); i++ )
  {
    QDir dir( svgPaths.at( i ) );
    QStandardItem *baseGroup;

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
    baseGroup->setIcon( QgsApplication::style()->standardIcon( QStyle::SP_DirIcon ) );
    baseGroup->setToolTip( dir.path() );
    parentItem->appendRow( baseGroup );
    parentPaths << svgPaths.at( i );
    mPathItemHash.insert( svgPaths.at( i ), baseGroup );
    QgsDebugMsg( QString( "SVG base path %1: %2" ).arg( i ).arg( baseGroup->data().toString() ) );
  }
  mLoader->setParentPaths( parentPaths );
  connect( mLoader, SIGNAL( foundPath( QString, QString ) ), this, SLOT( addPath( QString, QString ) ) );
  mLoader->start();
}

QgsSvgSelectorGroupsModel::~QgsSvgSelectorGroupsModel()
{
  mLoader->stop();
}

void QgsSvgSelectorGroupsModel::addPath( const QString& parentPath, const QString& item )
{
  QStandardItem* parentGroup = mPathItemHash.value( parentPath );
  if ( !parentGroup )
    return;

  QString fullPath = parentPath + '/' + item;
  QStandardItem* group = new QStandardItem( item );
  group->setData( QVariant( fullPath ) );
  group->setEditable( false );
  group->setCheckable( false );
  group->setToolTip( fullPath );
  group->setIcon( QgsApplication::style()->standardIcon( QStyle::SP_DirIcon ) );
  parentGroup->appendRow( group );
  mPathItemHash.insert( fullPath, group );
}


//-- QgsSvgSelectorWidget

QgsSvgSelectorWidget::QgsSvgSelectorWidget( QWidget* parent )
    : QWidget( parent )
{
  // TODO: in-code gui setup with option to vertically or horizontally stack SVG groups/images widgets
  setupUi( this );

  mGroupsTreeView->setHeaderHidden( true );
  populateList();

  connect( mImagesListView->selectionModel(), SIGNAL( currentChanged( const QModelIndex&, const QModelIndex& ) ),
           this, SLOT( svgSelectionChanged( const QModelIndex& ) ) );
  connect( mGroupsTreeView->selectionModel(), SIGNAL( currentChanged( const QModelIndex&, const QModelIndex& ) ),
           this, SLOT( populateIcons( const QModelIndex& ) ) );

  QSettings settings;
  bool useRelativePath = ( QgsProject::instance()->readBoolEntry( "Paths", "/Absolute", false )
                           || settings.value( "/Windows/SvgSelectorWidget/RelativePath" ).toBool() );
  mRelativePathChkBx->setChecked( useRelativePath );
}

QgsSvgSelectorWidget::~QgsSvgSelectorWidget()
{
  QSettings settings;
  settings.setValue( "/Windows/SvgSelectorWidget/RelativePath", mRelativePathChkBx->isChecked() );
}

void QgsSvgSelectorWidget::setSvgPath( const QString& svgPath )
{
  QString updatedPath( "" );

  // skip possible urls, excepting those that may locally resolve
  if ( !svgPath.contains( "://" ) || ( svgPath.contains( "file://", Qt::CaseInsensitive ) ) )
  {
    QString resolvedPath = QgsSymbolLayerUtils::symbolNameToPath( svgPath.trimmed() );
    if ( !resolvedPath.isNull() )
    {
      updatedPath = resolvedPath;
    }
  }

  mCurrentSvgPath = updatedPath;

  mFileLineEdit->blockSignals( true );
  mFileLineEdit->setText( updatedPath );
  mFileLineEdit->blockSignals( false );

  mImagesListView->selectionModel()->blockSignals( true );
  QAbstractItemModel* m = mImagesListView->model();
  QItemSelectionModel* selModel = mImagesListView->selectionModel();
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
  if ( mRelativePathChkBx->isChecked() )
    return currentSvgPathToName();

  return mCurrentSvgPath;
}

QString QgsSvgSelectorWidget::currentSvgPathToName() const
{
  return QgsSymbolLayerUtils::symbolPathToName( mCurrentSvgPath );
}

void QgsSvgSelectorWidget::updateCurrentSvgPath( const QString& svgPath )
{
  mCurrentSvgPath = svgPath;
  emit svgSelected( currentSvgPath() );
}

void QgsSvgSelectorWidget::svgSelectionChanged( const QModelIndex& idx )
{
  QString filePath = idx.data( Qt::UserRole ).toString();
  mFileLineEdit->setText( filePath );
  updateCurrentSvgPath( filePath );
}

void QgsSvgSelectorWidget::populateIcons( const QModelIndex& idx )
{
  QString path = idx.data( Qt::UserRole + 1 ).toString();

  QAbstractItemModel* oldModel = mImagesListView->model();
  QgsSvgSelectorListModel* m = new QgsSvgSelectorListModel( mImagesListView, path );
  mImagesListView->setModel( m );
  delete oldModel; //explicitly delete old model to force any background threads to stop

  connect( mImagesListView->selectionModel(), SIGNAL( currentChanged( const QModelIndex&, const QModelIndex& ) ),
           this, SLOT( svgSelectionChanged( const QModelIndex& ) ) );

}

void QgsSvgSelectorWidget::on_mFilePushButton_clicked()
{
  QSettings settings;
  QString openDir = settings.value( "/UI/lastSVGMarkerDir", QDir::homePath() ).toString();

  QString lineEditText = mFileLineEdit->text();
  if ( !lineEditText.isEmpty() )
  {
    QFileInfo openDirFileInfo( lineEditText );
    openDir = openDirFileInfo.path();
  }

  QString file = QFileDialog::getOpenFileName( nullptr,
                 tr( "Select SVG file" ),
                 openDir,
                 tr( "SVG files" ) + " (*.svg *.SVG)" );

  activateWindow(); // return window focus

  if ( file.isNull() )
    return;

  QFileInfo fi( file );
  if ( !fi.exists() || !fi.isReadable() )
  {
    updateCurrentSvgPath( QString() );
    updateLineEditFeedback( false );
    return;
  }
  settings.setValue( "/UI/lastSVGMarkerDir", fi.absolutePath() );
  mFileLineEdit->setText( file );
  updateCurrentSvgPath( file );
}

void QgsSvgSelectorWidget::updateLineEditFeedback( bool ok, const QString& tip )
{
  // draw red text for path field if invalid (path can't be resolved)
  mFileLineEdit->setStyleSheet( QString( !ok ? "QLineEdit{ color: rgb(225, 0, 0); }" : "" ) );
  mFileLineEdit->setToolTip( !ok ? tr( "File not found" ) : tip );
}

void QgsSvgSelectorWidget::on_mFileLineEdit_textChanged( const QString& text )
{
  QString resolvedPath = QgsSymbolLayerUtils::symbolNameToPath( text );
  bool validSVG = !resolvedPath.isNull();

  updateLineEditFeedback( validSVG, resolvedPath );
  updateCurrentSvgPath( validSVG ? resolvedPath : QString() );
}

void QgsSvgSelectorWidget::populateList()
{
  QgsSvgSelectorGroupsModel* g = new QgsSvgSelectorGroupsModel( mGroupsTreeView );
  mGroupsTreeView->setModel( g );
  // Set the tree expanded at the first level
  int rows = g->rowCount( g->indexFromItem( g->invisibleRootItem() ) );
  for ( int i = 0; i < rows; i++ )
  {
    mGroupsTreeView->setExpanded( g->indexFromItem( g->item( i ) ), true );
  }

  // Initally load the icons in the List view without any grouping
  QAbstractItemModel* oldModel = mImagesListView->model();
  QgsSvgSelectorListModel* m = new QgsSvgSelectorListModel( mImagesListView );
  mImagesListView->setModel( m );
  delete oldModel; //explicitly delete old model to force any background threads to stop
}

//-- QgsSvgSelectorDialog

QgsSvgSelectorDialog::QgsSvgSelectorDialog( QWidget *parent, Qt::WindowFlags fl,
    const QDialogButtonBox::StandardButtons& buttons,
    Qt::Orientation orientation )
    : QDialog( parent, fl )
{
  // TODO: pass 'orientation' to QgsSvgSelectorWidget for customizing its layout, once implemented
  Q_UNUSED( orientation );

  // create buttonbox
  mButtonBox = new QDialogButtonBox( buttons, orientation, this );
  connect( mButtonBox, SIGNAL( accepted() ), this, SLOT( accept() ) );
  connect( mButtonBox, SIGNAL( rejected() ), this, SLOT( reject() ) );

  setMinimumSize( 480, 320 );

  // dialog's layout
  mLayout = new QVBoxLayout();
  mSvgSelector = new QgsSvgSelectorWidget( this );
  mLayout->addWidget( mSvgSelector );

  mLayout->addWidget( mButtonBox );
  setLayout( mLayout );

  QSettings settings;
  restoreGeometry( settings.value( "/Windows/SvgSelectorDialog/geometry" ).toByteArray() );
}

QgsSvgSelectorDialog::~QgsSvgSelectorDialog()
{
  QSettings settings;
  settings.setValue( "/Windows/SvgSelectorDialog/geometry", saveGeometry() );
}


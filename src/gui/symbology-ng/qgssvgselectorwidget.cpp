/***************************************************************************
    qgssvgselectorwidget.cpp - group and preview selector for SVG files
                               built off of work in qgssymbollayerv2widget

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
#include "qgssymbollayerv2utils.h"

#include <QAbstractListModel>
#include <QCheckBox>
#include <QDir>
#include <QFileDialog>
#include <QModelIndex>
#include <QPixmapCache>
#include <QSettings>
#include <QStyle>
#include <QTime>


//--- QgsSvgSelectorListModel

QgsSvgSelectorListModel::QgsSvgSelectorListModel( QObject* parent )
    : QAbstractListModel( parent )
{
  mSvgFiles = QgsSymbolLayerV2Utils::listSvgFiles();
}

// Constructor to create model for icons in a specific path
QgsSvgSelectorListModel::QgsSvgSelectorListModel( QObject* parent, QString path )
    : QAbstractListModel( parent )
{
  mSvgFiles = QgsSymbolLayerV2Utils::listSvgFilesAt( path );
}

int QgsSvgSelectorListModel::rowCount( const QModelIndex & parent ) const
{
  Q_UNUSED( parent );
  return mSvgFiles.count();
}

QVariant QgsSvgSelectorListModel::data( const QModelIndex & index, int role ) const
{
  QString entry = mSvgFiles.at( index.row() );

  if ( role == Qt::DecorationRole ) // icon
  {
    QPixmap pixmap;
    if ( !QPixmapCache::find( entry, pixmap ) )
    {
      // render SVG file
      QColor fill, outline;
      double outlineWidth;
      bool fillParam, outlineParam, outlineWidthParam;
      QgsSvgCache::instance()->containsParams( entry, fillParam, fill, outlineParam, outline, outlineWidthParam, outlineWidth );

      bool fitsInCache; // should always fit in cache at these sizes (i.e. under 559 px ^ 2, or half cache size)
      const QImage& img = QgsSvgCache::instance()->svgAsImage( entry, 30.0, fill, outline, outlineWidth, 3.5 /*appr. 88 dpi*/, 1.0, fitsInCache );
      pixmap = QPixmap::fromImage( img );
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


//--- QgsSvgSelectorGroupsModel

QgsSvgSelectorGroupsModel::QgsSvgSelectorGroupsModel( QObject* parent )
    : QStandardItemModel( parent )
{
  QStringList svgPaths = QgsApplication::svgPaths();
  QStandardItem *parentItem = invisibleRootItem();

  for ( int i = 0; i < svgPaths.size(); i++ )
  {
    QDir dir( svgPaths[i] );
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
    baseGroup->setData( QVariant( svgPaths[i] ) );
    baseGroup->setEditable( false );
    baseGroup->setCheckable( false );
    baseGroup->setIcon( QgsApplication::style()->standardIcon( QStyle::SP_DirIcon ) );
    baseGroup->setToolTip( dir.path() );
    parentItem->appendRow( baseGroup );
    createTree( baseGroup );
    QgsDebugMsg( QString( "SVG base path %1: %2" ).arg( i ).arg( baseGroup->data().toString() ) );
  }
}

void QgsSvgSelectorGroupsModel::createTree( QStandardItem* &parentGroup )
{
  QDir parentDir( parentGroup->data().toString() );
  foreach ( QString item, parentDir.entryList( QDir::Dirs | QDir::NoDotAndDotDot ) )
  {
    QStandardItem* group = new QStandardItem( item );
    group->setData( QVariant( parentDir.path() + "/" + item ) );
    group->setEditable( false );
    group->setCheckable( false );
    group->setToolTip( parentDir.path() + "/" + item );
    group->setIcon( QgsApplication::style()->standardIcon( QStyle::SP_DirIcon ) );
    parentGroup->appendRow( group );
    createTree( group );
  }
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
    QString resolvedPath = QgsSymbolLayerV2Utils::symbolNameToPath( svgPath.trimmed() );
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
  return QgsSymbolLayerV2Utils::symbolPathToName( mCurrentSvgPath );
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

  QgsSvgSelectorListModel* m = new QgsSvgSelectorListModel( mImagesListView, path );
  mImagesListView->setModel( m );

  connect( mImagesListView->selectionModel(), SIGNAL( currentChanged( const QModelIndex&, const QModelIndex& ) ),
           this, SLOT( svgSelectionChanged( const QModelIndex& ) ) );
}

void QgsSvgSelectorWidget::on_mFilePushButton_clicked()
{
  QSettings settings;
  QString openDir = settings.value( "/UI/lastSVGMarkerDir", "." ).toString();

  QString lineEditText = mFileLineEdit->text();
  if ( !lineEditText.isEmpty() )
  {
    QFileInfo openDirFileInfo( lineEditText );
    openDir = openDirFileInfo.path();
  }

  QString file = QFileDialog::getOpenFileName( 0,
                 tr( "Select SVG file" ),
                 openDir,
                 tr( "SVG files" ) + " (*.svg *.SVG)" );

  activateWindow(); // return window focus

  if ( file.isNull() )
    return;

  QFileInfo fi( file );
  if ( !fi.exists() || !fi.isReadable() )
  {
    updateCurrentSvgPath( QString( "" ) );
    updateLineEditFeedback( false );
    return;
  }
  settings.setValue( "/UI/lastSVGMarkerDir", fi.absolutePath() );
  mFileLineEdit->setText( file );
  updateCurrentSvgPath( file );
}

void QgsSvgSelectorWidget::updateLineEditFeedback( bool ok, QString tip )
{
  // draw red text for path field if invalid (path can't be resolved)
  mFileLineEdit->setStyleSheet( QString( !ok ? "QLineEdit{ color: rgb(225, 0, 0); }" : "" ) );
  mFileLineEdit->setToolTip( !ok ? tr( "File not found" ) : tip );
}

void QgsSvgSelectorWidget::on_mFileLineEdit_textChanged( const QString& text )
{
  QString resolvedPath = QgsSymbolLayerV2Utils::symbolNameToPath( text );
  bool validSVG = !resolvedPath.isNull();

  updateLineEditFeedback( validSVG, resolvedPath );
  updateCurrentSvgPath( validSVG ? resolvedPath : QString( "" ) );
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
  QgsSvgSelectorListModel* m = new QgsSvgSelectorListModel( mImagesListView );
  mImagesListView->setModel( m );
}

//-- QgsSvgSelectorDialog

QgsSvgSelectorDialog::QgsSvgSelectorDialog( QWidget *parent, Qt::WindowFlags fl,
    QDialogButtonBox::StandardButtons buttons,
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

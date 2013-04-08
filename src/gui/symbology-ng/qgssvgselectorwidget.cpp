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
#include "qgssvgcache.h"
#include "qgssymbollayerv2utils.h"

#include <QAbstractListModel>
#include <QDir>
#include <QFileDialog>
#include <QMessageBox>
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
  setupUi( this );

  mGroupsTreeView->setHeaderHidden( true );
  populateList();

  connect( mImagesListView->selectionModel(), SIGNAL( currentChanged( const QModelIndex&, const QModelIndex& ) ),
           this, SLOT( svgSelectionChanged( const QModelIndex& ) ) );
  connect( mGroupsTreeView->selectionModel(), SIGNAL( currentChanged( const QModelIndex&, const QModelIndex& ) ),
           this, SLOT( populateIcons( const QModelIndex& ) ) );
  connect( this, SIGNAL( svgSelected( const QString& ) ), this, SLOT( updateCurrentSvgPath( const QString& ) ) );
}

QgsSvgSelectorWidget::~QgsSvgSelectorWidget()
{
}

void QgsSvgSelectorWidget::setSvgPath( const QString& svgPath )
{
  mCurrentSvgPath = svgPath;

  mFileLineEdit->blockSignals( true );
  mFileLineEdit->setText( svgPath );
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
  return mCurrentSvgPath;
}

void QgsSvgSelectorWidget::updateCurrentSvgPath( const QString& svgPath )
{
  mCurrentSvgPath = svgPath;
}

void QgsSvgSelectorWidget::svgSelectionChanged( const QModelIndex& idx )
{
  QString filePath = idx.data( Qt::UserRole ).toString();
  mFileLineEdit->setText( filePath );
  emit svgSelected( filePath );
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
    emit svgSelected( QString( "" ) );
    QMessageBox::critical( 0, tr( "Invalid file" ), tr( "Error, file does not exist or is not readable" ) );
    return;
  }
  settings.setValue( "/UI/lastSVGMarkerDir", fi.absolutePath() );
  mFileLineEdit->setText( file );
  emit svgSelected( file );
}

void QgsSvgSelectorWidget::on_mFileLineEdit_textEdited( const QString& text )
{
  if ( !QFileInfo( text ).exists() )
  {
    emit svgSelected( QString( "" ) );
    return;
  }
  emit svgSelected( text );
}

void QgsSvgSelectorWidget::on_mFileLineEdit_editingFinished()
{
  if ( !QFileInfo( mFileLineEdit->text() ).exists() )
  {
    QUrl url( mFileLineEdit->text() );
    if ( !url.isValid() )
    {
      emit svgSelected( QString( "" ) );
      QMessageBox::critical( 0, tr( "Invalid file url" ), tr( "Error, file URL is invalid" ) );
      return;
    }
    emit svgSelected( QString( "" ) );
    QMessageBox::critical( 0, tr( "Invalid file" ), tr( "Error, file does not exist or is not readable" ) );
    return;
  }
  emit svgSelected( mFileLineEdit->text() );
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

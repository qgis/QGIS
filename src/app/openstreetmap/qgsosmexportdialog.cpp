/***************************************************************************
  qgsosmexportdialog.cpp
  --------------------------------------
  Date                 : February 2013
  Copyright            : (C) 2013 by Martin Dobias
  Email                : wonder dot sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsosmexportdialog.h"

#include "qgsosmdatabase.h"

#include "qgsdatasourceuri.h"
#include "qgsmaplayerregistry.h"
#include "qgsvectorlayer.h"

#include <QApplication>
#include <QFileDialog>
#include <QMessageBox>
#include <QSettings>
#include <QStandardItemModel>

QgsOSMExportDialog::QgsOSMExportDialog( QWidget *parent ) :
    QDialog( parent ), mDatabase( new QgsOSMDatabase )
{
  setupUi( this );

  connect( btnBrowseDb, SIGNAL( clicked() ), this, SLOT( onBrowse() ) );
  connect( buttonBox, SIGNAL( accepted() ), this, SLOT( onOK() ) );
  connect( buttonBox, SIGNAL( rejected() ), this, SLOT( onClose() ) );
  connect( editDbFileName, SIGNAL( textChanged( QString ) ), this, SLOT( updateLayerName() ) );
  connect( radPoints, SIGNAL( clicked() ), this, SLOT( updateLayerName() ) );
  connect( radPolylines, SIGNAL( clicked() ), this, SLOT( updateLayerName() ) );
  connect( radPolygons, SIGNAL( clicked() ), this, SLOT( updateLayerName() ) );
  connect( btnLoadTags, SIGNAL( clicked() ), this, SLOT( onLoadTags() ) );
  connect( btnSelectAll, SIGNAL( clicked() ), this, SLOT( onSelectAll() ) );
  connect( btnUnselectAll, SIGNAL( clicked() ), this, SLOT( onUnselectAll() ) );

  mTagsModel = new QStandardItemModel( this );
  mTagsModel->setHorizontalHeaderLabels( QStringList() << tr( "Tag" ) << tr( "Count" ) );
  viewTags->setModel( mTagsModel );
}

QgsOSMExportDialog::~QgsOSMExportDialog()
{
  delete mDatabase;
}


void QgsOSMExportDialog::onBrowse()
{
  QSettings settings;
  QString lastDir = settings.value( "/osm/lastDir" ).toString();

  QString fileName = QFileDialog::getOpenFileName( this, QString(), lastDir, tr( "SQLite databases (*.db)" ) );
  if ( fileName.isNull() )
    return;

  settings.setValue( "/osm/lastDir", QFileInfo( fileName ).absolutePath() );
  editDbFileName->setText( fileName );
}

void QgsOSMExportDialog::updateLayerName()
{
  QString baseName = QFileInfo( editDbFileName->text() ).baseName();

  QString layerType;
  if ( radPoints->isChecked() )
    layerType = "points";
  else if ( radPolylines->isChecked() )
    layerType = "polylines";
  else
    layerType = "polygons";
  editLayerName->setText( QString( "%1_%2" ).arg( baseName ).arg( layerType ) );
}


bool QgsOSMExportDialog::openDatabase()
{
  mDatabase->setFileName( editDbFileName->text() );

  if ( !mDatabase->open() )
  {
    QMessageBox::critical( this, QString(), tr( "Unable to open database:\n%1" ).arg( mDatabase->errorString() ) );
    return false;
  }

  return true;
}


void QgsOSMExportDialog::onLoadTags()
{
  if ( !openDatabase() )
    return;

  QApplication::setOverrideCursor( Qt::WaitCursor );

  QList<QgsOSMTagCountPair> pairs = mDatabase->usedTags( !radPoints->isChecked() );
  mDatabase->close();

  mTagsModel->setColumnCount( 2 );
  mTagsModel->setRowCount( pairs.count() );

  for ( int i = 0; i < pairs.count(); ++i )
  {
    const QgsOSMTagCountPair& p = pairs[i];
    QStandardItem* item = new QStandardItem( p.first );
    item->setCheckable( true );
    mTagsModel->setItem( i, 0, item );
    QStandardItem* item2 = new QStandardItem();
    item2->setData( p.second, Qt::DisplayRole );
    mTagsModel->setItem( i, 1, item2 );
  }

  viewTags->resizeColumnToContents( 0 );
  viewTags->sortByColumn( 1, Qt::DescendingOrder );

  QApplication::restoreOverrideCursor();
}


void QgsOSMExportDialog::onOK()
{
  if ( !openDatabase() )
    return;

  QgsOSMDatabase::ExportType type;
  if ( radPoints->isChecked() )
    type = QgsOSMDatabase::Point;
  else if ( radPolylines->isChecked() )
    type = QgsOSMDatabase::Polyline;
  else
    type = QgsOSMDatabase::Polygon;

  buttonBox->setEnabled( false );
  QApplication::setOverrideCursor( Qt::WaitCursor );

  QStringList tagKeys;

  for ( int i = 0; i < mTagsModel->rowCount(); ++i )
  {
    QStandardItem* item = mTagsModel->item( i, 0 );
    if ( item->checkState() == Qt::Checked )
      tagKeys << item->text();
  }

  bool res = mDatabase->exportSpatiaLite( type, editLayerName->text(), tagKeys );

  // load the layer into canvas if that was requested
  if ( chkLoadWhenFinished->isChecked() )
  {
    QgsDataSourceURI uri;
    uri.setDatabase( editDbFileName->text() );
    uri.setDataSource( QString(), editLayerName->text(), "geometry" );
    QgsVectorLayer* vlayer = new QgsVectorLayer( uri.uri(), editLayerName->text(), "spatialite" );
    if ( vlayer->isValid() )
      QgsMapLayerRegistry::instance()->addMapLayer( vlayer );
  }

  QApplication::restoreOverrideCursor();
  buttonBox->setEnabled( true );

  if ( res )
  {
    QMessageBox::information( this, tr( "OpenStreetMap export" ), tr( "Export has been successful." ) );
  }
  else
  {
    QMessageBox::critical( this, tr( "OpenStreetMap export" ), tr( "Failed to export OSM data:\n%1" ).arg( mDatabase->errorString() ) );
  }

  mDatabase->close();
}

void QgsOSMExportDialog::onClose()
{
  reject();
}

void QgsOSMExportDialog::onSelectAll()
{
  for ( int i = 0; i < mTagsModel->rowCount(); ++i )
  {
    mTagsModel->item( i, 0 )->setCheckState( Qt::Checked );
  }
}

void QgsOSMExportDialog::onUnselectAll()
{
  for ( int i = 0; i < mTagsModel->rowCount(); ++i )
  {
    mTagsModel->item( i, 0 )->setCheckState( Qt::Unchecked );
  }
}

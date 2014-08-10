/***************************************************************************
                          qgshandlebadlayers.cpp  -  description
                             -------------------
    begin                : Sat 5 Mar 2011
    copyright            : (C) 2011 by Juergen E. Fischer, norBIT GmbH
    email                : jef at norbit dot de
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgshandlebadlayers.h"
#include "qgisapp.h"
#include "qgisgui.h"
#include "qgsdatasourceuri.h"
#include "qgslogger.h"
#include "qgsrasterlayer.h"
#include "qgsproviderregistry.h"
#include "qgsmessagebar.h"

#include <QDomDocument>
#include <QDomElement>
#include <QPushButton>
#include <QMessageBox>
#include <QUrl>

QgsHandleBadLayersHandler::QgsHandleBadLayersHandler()
{
}

void QgsHandleBadLayersHandler::handleBadLayers( QList<QDomNode> layers, QDomDocument projectDom )
{
  QApplication::setOverrideCursor( Qt::ArrowCursor );
  QgsHandleBadLayers *dialog = new QgsHandleBadLayers( layers, projectDom );

  if ( dialog->layerCount() < layers.size() )
    QgisApp::instance()->messageBar()->pushMessage(
      tr( "Handle bad layers" ),
      tr( "%1 of %2 bad layers were not fixable." )
      .arg( layers.size() - dialog->layerCount() )
      .arg( layers.size() ),
      QgsMessageBar::WARNING, QgisApp::instance()->messageTimeout() );

  if ( dialog->layerCount() > 0 )
    dialog->exec();

  delete dialog;
  QApplication::restoreOverrideCursor();
}


QgsHandleBadLayers::QgsHandleBadLayers( const QList<QDomNode> &layers, const QDomDocument &projectDom )
    : QDialog( QgisApp::instance() )
    , mLayers( layers )
{
  Q_UNUSED( projectDom );
  setupUi( this );

  mVectorFileFilter = QgsProviderRegistry::instance()->fileVectorFilters();
  mRasterFileFilter = QgsProviderRegistry::instance()->fileRasterFilters();

  mBrowseButton = new QPushButton( tr( "Browse" ) );
  buttonBox->addButton( mBrowseButton, QDialogButtonBox::ActionRole );
  mBrowseButton->setDisabled( true );

  connect( mLayerList, SIGNAL( itemSelectionChanged() ), this, SLOT( selectionChanged() ) );
  connect( mBrowseButton, SIGNAL( clicked() ), this, SLOT( browseClicked() ) );
  connect( buttonBox, SIGNAL( accepted() ), this, SLOT( accept() ) );
  connect( buttonBox->button( QDialogButtonBox::Apply ), SIGNAL( clicked() ), this, SLOT( apply() ) );

  mLayerList->clear();
  mLayerList->setSortingEnabled( true );
  mLayerList->setSelectionBehavior( QAbstractItemView::SelectRows );
  mLayerList->setColumnCount( 4 );

  mLayerList->setHorizontalHeaderLabels( QStringList()
                                         << tr( "Layer name" )
                                         << tr( "Type" )
                                         << tr( "Provider" )
                                         << tr( "Datasource" )
                                       );

  int j = 0;
  for ( int i = 0; i < mLayers.size(); i++ )
  {
    const QDomNode &node = mLayers[i];

    QString name = node.namedItem( "layername" ).toElement().text();
    QString type = node.toElement().attribute( "type" );
    QString datasource = node.namedItem( "datasource" ).toElement().text();
    QString provider = type == "vector" ? node.namedItem( "provider" ).toElement().text() : tr( "none" );

    QgsDebugMsg( QString( "name=%1 type=%2 provider=%3 datasource='%4'" )
                 .arg( name )
                 .arg( type )
                 .arg( provider )
                 .arg( datasource ) );

    mLayerList->setRowCount( j + 1 );

    QTableWidgetItem *item;

    item = new QTableWidgetItem( name );
    item->setData( Qt::UserRole + 0, i );
    item->setFlags( item->flags() & ~Qt::ItemIsEditable );
    mLayerList->setItem( j, 0, item );

    item = new QTableWidgetItem( type );
    item->setFlags( item->flags() & ~Qt::ItemIsEditable );
    mLayerList->setItem( j, 1, item );

    item = new QTableWidgetItem( provider );
    item->setFlags( item->flags() & ~Qt::ItemIsEditable );
    mLayerList->setItem( j, 2, item );

    item = new QTableWidgetItem( datasource );
    mLayerList->setItem( j, 3, item );

    j++;
  }

  // mLayerList->resizeColumnsToContents();
}

QgsHandleBadLayers::~QgsHandleBadLayers()
{
}

void QgsHandleBadLayers::selectionChanged()
{
  QgsDebugMsg( "entered." );

  mRows.clear();

  foreach ( QTableWidgetItem *item, mLayerList->selectedItems() )
  {
    if ( item->column() != 0 )
      continue;

    mRows << item->row();
  }

  mBrowseButton->setEnabled( mRows.size() > 0 );
}

QString QgsHandleBadLayers::filename( int row )
{
  QString type = mLayerList->item( row, 1 )->text();
  QString provider = mLayerList->item( row, 2 )->text();
  QString datasource = mLayerList->item( row, 3 )->text();

  if ( type == "vector" )
  {
    if ( provider == "spatialite" )
    {
      QgsDataSourceURI uri( datasource );
      return uri.database();
    }
    else if ( provider == "ogr" )
    {
      QStringList theURIParts = datasource.split( "|" );
      return theURIParts[0];
    }
    else if ( provider == "delimitedtext" )
    {
      return QUrl::fromEncoded( datasource.toAscii() ).toLocalFile();
    }
  }
  else
  {
    return datasource;
  }

  return QString::null;
}

void QgsHandleBadLayers::setFilename( int row, QString filename )
{
  if ( !QFileInfo( filename ).exists() )
    return;

  QString type = mLayerList->item( row, 1 )->text();
  QString provider = mLayerList->item( row, 2 )->text();
  QTableWidgetItem *item = mLayerList->item( row, 3 );

  QString datasource = item->text();

  if ( type == "vector" )
  {
    if ( provider == "spatialite" )
    {
      QgsDataSourceURI uri( datasource );
      uri.setDatabase( filename );
      datasource = uri.uri();
    }
    else if ( provider == "ogr" )
    {
      QStringList theURIParts = datasource.split( "|" );
      theURIParts[0] = filename;
      datasource = theURIParts.join( "|" );
    }
    else if ( provider == "delimitedtext" )
    {
      QUrl uriSource = QUrl::fromEncoded( datasource.toAscii() );
      QUrl uriDest = QUrl::fromLocalFile( filename );
      uriDest.setQueryItems( uriSource.queryItems() );
      datasource = QString::fromAscii( uriDest.toEncoded() );
    }
  }
  else
  {
    datasource = filename;
  }

  item->setText( datasource );
}

void QgsHandleBadLayers::browseClicked()
{
  QgsDebugMsg( "entered." );

  if ( mRows.size() == 1 )
  {
    int row = mRows[0];
    QString type = mLayerList->item( row, 1 )->text();

    QString memoryQualifier, fileFilter;
    if ( type == "vector" )
    {
      memoryQualifier = "lastVectorFileFilter";
      fileFilter = mVectorFileFilter;
    }
    else
    {
      memoryQualifier = "lastRasterFileFilter";
      fileFilter = mRasterFileFilter;
    }

    QString fn = filename( row );
    if ( fn.isNull() )
      return;

    QStringList selectedFiles;
    QString enc;
    QString title = tr( "Select file to replace '%1'" ).arg( fn );

    QgisGui::openFilesRememberingFilter( memoryQualifier, fileFilter, selectedFiles, enc, title );
    if ( selectedFiles.size() != 1 )
    {
      QMessageBox::information( this, title, tr( "Please select exactly one file." ) );
      return;
    }

    setFilename( row, selectedFiles[0] );
  }
  else if ( mRows.size() > 1 )
  {
    QStringList selectedFiles;
    QString enc;
    QString title = tr( "Select new directory of selected files" );

    QgisGui::openFilesRememberingFilter( "missingDirectory", tr( "All files (*)" ), selectedFiles, enc, title );
    if ( selectedFiles.isEmpty() )
    {
      return;
    }

    QFileInfo path( selectedFiles[0] );
    if ( !path.exists() )
    {
      return;
    }

    foreach ( int row, mRows )
    {
      QString type = mLayerList->item( row, 1 )->text();
      QString fn = filename( row );
      if ( fn.isEmpty() )
        continue;

      QFileInfo fi( fn );
      fi.setFile( path.dir(), fi.fileName() );
      if ( !fi.exists() )
        continue;

      setFilename( row, fi.absoluteFilePath() );
    }
  }
}

void QgsHandleBadLayers::apply()
{
  QgsDebugMsg( "entered." );
  for ( int i = 0; i < mLayerList->rowCount(); i++ )
  {
    int idx = mLayerList->item( i, 0 )->data( Qt::UserRole ).toInt();
    QDomNode &node = const_cast<QDomNode &>( mLayers[ idx ] );

    QString type = mLayerList->item( i, 1 )->text();
    QString provider = mLayerList->item( i, 2 )->text();
    QTableWidgetItem *item = mLayerList->item( i, 3 );
    QString datasource = item->text();

    node.namedItem( "datasource" ).toElement().firstChild().toText().setData( datasource );
    if ( QgsProject::instance()->read( node ) )
    {
      mLayerList->removeRow( i-- );
    }
    else
    {
      item->setForeground( QBrush( Qt::red ) );
    }
  }
}

void QgsHandleBadLayers::accept()
{
  QgsDebugMsg( "entered." );
  apply();

  if ( mLayerList->rowCount() > 0  &&
       QMessageBox::warning( this,
                             tr( "Unhandled layer will be lost." ),
                             tr( "There are still %n unhandled layer(s), that will be lost if you closed now.",
                                 "unhandled layers",
                                 mLayerList->rowCount() ),
                             QMessageBox::Ok | QMessageBox::Cancel,
                             QMessageBox::Cancel ) == QMessageBox::Cancel )
  {
    return;
  }

  QDialog::accept();
}

void QgsHandleBadLayers::rejected()
{
  QgsDebugMsg( "entered." );

  if ( mLayerList->rowCount() > 0  &&
       QMessageBox::warning( this,
                             tr( "Unhandled layer will be lost." ),
                             tr( "There are still %n unhandled layer(s), that will be lost if you closed now.",
                                 "unhandled layers",
                                 mLayerList->rowCount() ),
                             QMessageBox::Ok | QMessageBox::Cancel,
                             QMessageBox::Cancel ) == QMessageBox::Cancel )
  {
    return;
  }

  QDialog::reject();
}

int QgsHandleBadLayers::layerCount()
{
  return mLayerList->rowCount();
}

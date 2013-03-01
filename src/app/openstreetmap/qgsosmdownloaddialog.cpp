/***************************************************************************
  qgsosmdownloaddialog.cpp
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

#include "qgsosmdownloaddialog.h"

#include <QFileDialog>
#include <QMessageBox>
#include <QPushButton>

#include "qgisapp.h"
#include "qgsmapcanvas.h"
#include "qgsmaplayer.h"
#include "qgsmaplayerregistry.h"
#include "qgsrectangle.h"

#include "qgsosmdownload.h"

QgsOSMDownloadDialog::QgsOSMDownloadDialog( QWidget* parent )
    : QDialog( parent ), mDownload( new QgsOSMDownload )
{
  setupUi( this );

  editXMin->setValidator( new QDoubleValidator( -180.0, 180.0, 6, this ) );
  editXMax->setValidator( new QDoubleValidator( -180.0, 180.0, 6, this ) );
  editYMin->setValidator( new QDoubleValidator( -90.0, 90.0, 6, this ) );
  editYMax->setValidator( new QDoubleValidator( -90.0, 90.0, 6, this ) );

  populateLayers();
  onExtentCanvas();

  connect( radExtentCanvas, SIGNAL( clicked() ), this, SLOT( onExtentCanvas() ) );
  connect( radExtentLayer, SIGNAL( clicked() ), this, SLOT( onExtentLayer() ) );
  connect( radExtentManual, SIGNAL( clicked() ), this, SLOT( onExtentManual() ) );
  connect( cboLayers, SIGNAL( currentIndexChanged( int ) ), this, SLOT( onCurrentLayerChanged( int ) ) );
  connect( btnBrowse, SIGNAL( clicked() ), this, SLOT( onBrowseClicked() ) );
  connect( buttonBox, SIGNAL( accepted() ), this, SLOT( onOK() ) );
  connect( buttonBox, SIGNAL( rejected() ), this, SLOT( onClose() ) );

  connect( mDownload, SIGNAL( finished() ), this, SLOT( onFinished() ) );
  connect( mDownload, SIGNAL( downloadProgress( qint64, qint64 ) ), this, SLOT( onDownloadProgress( qint64, qint64 ) ) );
}

QgsOSMDownloadDialog::~QgsOSMDownloadDialog()
{
  delete mDownload;
}


void QgsOSMDownloadDialog::populateLayers()
{
  QMap<QString, QgsMapLayer*> layers = QgsMapLayerRegistry::instance()->mapLayers();
  QMap<QString, QgsMapLayer*>::iterator it;
  for ( it = layers.begin(); it != layers.end(); ++it )
  {
    cboLayers->addItem( it.value()->name(), it.key() );
  }
  cboLayers->setCurrentIndex( 0 );
}

void QgsOSMDownloadDialog::setRect( const QgsRectangle& rect )
{
  // these coords should be already lat/lon
  editXMin->setText( QString::number( rect.xMinimum() ) );
  editXMax->setText( QString::number( rect.xMaximum() ) );
  editYMin->setText( QString::number( rect.yMinimum() ) );
  editYMax->setText( QString::number( rect.yMaximum() ) );
}

QgsRectangle QgsOSMDownloadDialog::rect() const
{
  return QgsRectangle( editXMin->text().toDouble(), editYMin->text().toDouble(),
                       editXMax->text().toDouble(), editYMax->text().toDouble() );
}


void QgsOSMDownloadDialog::setRectReadOnly( bool readonly )
{
  editXMin->setReadOnly( readonly );
  editXMax->setReadOnly( readonly );
  editYMin->setReadOnly( readonly );
  editYMax->setReadOnly( readonly );
}


void QgsOSMDownloadDialog::onExtentCanvas()
{
  setRect( QgisApp::instance()->mapCanvas()->extent() );  // TODO: transform to WGS84
  setRectReadOnly( true );
  cboLayers->setEnabled( false );
}

void QgsOSMDownloadDialog::onExtentLayer()
{
  onCurrentLayerChanged( cboLayers->currentIndex() );
  setRectReadOnly( true );
  cboLayers->setEnabled( true );
}

void QgsOSMDownloadDialog::onExtentManual()
{
  setRectReadOnly( false );
  cboLayers->setEnabled( false );
}

void QgsOSMDownloadDialog::onCurrentLayerChanged( int index )
{
  if ( index < 0 )
    return;

  QString layerId = cboLayers->itemData( index ).toString();
  QgsMapLayer* layer = QgsMapLayerRegistry::instance()->mapLayer( layerId );
  if ( !layer )
    return;

  setRect( layer->extent() ); // TODO: transform to WGS84
}

void QgsOSMDownloadDialog::onBrowseClicked()
{
  QSettings settings;
  QString lastDir = settings.value( "/osm/lastDir" ).toString();

  QString fileName = QFileDialog::getSaveFileName( this, QString(), lastDir, tr( "OpenStreetMap files (*.osm)" ) );
  if ( fileName.isNull() )
    return;

  settings.setValue( "/osm/lastDir", QFileInfo( fileName ).absolutePath() );
  editFileName->setText( fileName );
}

void QgsOSMDownloadDialog::onOK()
{
  mDownload->setQuery( QgsOSMDownload::queryFromRect( rect() ) );
  mDownload->setOutputFileName( editFileName->text() );
  if ( !mDownload->start() )
  {
    QMessageBox::critical( this, tr( "Download error" ), mDownload->errorString() );
    return;
  }

  buttonBox->button( QDialogButtonBox::Ok )->setEnabled( false );
  progress->setRange( 0, 0 ); // this will start animating progress bar
}

void QgsOSMDownloadDialog::onClose()
{
  if ( !mDownload->isFinished() )
  {
    int res = QMessageBox::question( this, tr( "OpenStreetMap download" ),
                                     tr( "Would you like to abort download?" ), QMessageBox::Yes | QMessageBox::No );
    if ( res != QMessageBox::Yes )
      return;
  }

  reject();
}

void QgsOSMDownloadDialog::onFinished()
{
  buttonBox->button( QDialogButtonBox::Ok )->setEnabled( true );
  progress->setRange( 0, 1 );

  if ( mDownload->hasError() )
  {
    QMessageBox::critical( this, tr( "OpenStreetMap download" ), tr( "Download failed.\n%1" ).arg( mDownload->errorString() ) );
  }
  else
  {
    QMessageBox::information( this, tr( "OpenStreetMap download" ), tr( "Download has been successful." ) );
  }
}

void QgsOSMDownloadDialog::onDownloadProgress( qint64 bytesReceived, qint64 bytesTotal )
{
  Q_UNUSED( bytesTotal ); // it's -1 anyway (= unknown)
  double mbytesReceived = ( double )bytesReceived / ( 1024 * 1024 );
  editSize->setText( QString( "%1 MB" ).arg( QString::number( mbytesReceived, 'f', 1 ) ) );
}

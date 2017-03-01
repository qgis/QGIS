/***************************************************************************
    qgsmapcanvasdockwidget.cpp
    --------------------------
    begin                : February 2017
    copyright            : (C) 2017 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "qgsmapcanvasdockwidget.h"
#include "qgsmapcanvas.h"
#include "qgsprojectionselectiondialog.h"
#include <QMessageBox>

QgsMapCanvasDockWidget::QgsMapCanvasDockWidget( const QString &name, QWidget *parent )
  : QgsDockWidget( parent )
{
  setupUi( this );
  setAttribute( Qt::WA_DeleteOnClose );

  mContents->layout()->setContentsMargins( 0, 0, 0, 0 );
  mContents->layout()->setMargin( 0 );
  static_cast< QVBoxLayout * >( mContents->layout() )->setSpacing( 0 );

  setWindowTitle( name );
  mMapCanvas = new QgsMapCanvas( this );

  mMainWidget->setLayout( new QVBoxLayout() );
  mMainWidget->layout()->setContentsMargins( 0, 0, 0, 0 );
  mMainWidget->layout()->setMargin( 0 );

  mMainWidget->layout()->addWidget( mMapCanvas );

  connect( mActionSetCrs, &QAction::triggered, this, &QgsMapCanvasDockWidget::setMapCrs );
}

QgsMapCanvas *QgsMapCanvasDockWidget::mapCanvas()
{
  return mMapCanvas;
}

void QgsMapCanvasDockWidget::closeWithoutWarning()
{
  mShowCloseWarning = false;
  close();
}

void QgsMapCanvasDockWidget::closeEvent( QCloseEvent *event )
{
  if ( mShowCloseWarning && mMapCanvas->layerCount() > 0
       && QMessageBox::question( this, tr( "Close map view" ),
                                 tr( "Are you sure you want to close this map view?" ), QMessageBox::Yes | QMessageBox::No, QMessageBox::No ) == QMessageBox::No )
  {
    event->ignore();
  }
  else
  {
    event->accept();
  }
}

void QgsMapCanvasDockWidget::setMapCrs()
{
  QgsProjectionSelectionDialog dlg;
  dlg.setShowNoProjection( true );
  dlg.setCrs( mMapCanvas->mapSettings().destinationCrs() );

  if ( dlg.exec() )
  {
    mMapCanvas->setDestinationCrs( dlg.crs() );
  }
}

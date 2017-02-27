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

QgsMapCanvasDockWidget::QgsMapCanvasDockWidget( const QString &name, QWidget *parent )
  : QgsDockWidget( parent )
{
  setupUi( this );

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

void QgsMapCanvasDockWidget::setMapCrs()
{
  QgsProjectionSelectionDialog dlg;
  dlg.setCrs( mMapCanvas->mapSettings().destinationCrs() );

  if ( dlg.exec() )
  {
    mMapCanvas->setDestinationCrs( dlg.crs() );
  }
}

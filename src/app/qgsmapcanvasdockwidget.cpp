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
#include "qgsscalecombobox.h"
#include <QMessageBox>
#include <QMenu>
#include <QToolBar>
#include <QToolButton>

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
  connect( mActionSyncView, &QAction::toggled, this, &QgsMapCanvasDockWidget::syncView );

  QMenu *menu = new QMenu();

  QToolButton *toolButton = new QToolButton();
  toolButton->setMenu( menu );
  toolButton->setPopupMode( QToolButton::InstantPopup );
  toolButton->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "/mActionMapSettings.svg" ) ) );
  mToolbar->addWidget( toolButton );

  QgsScaleComboAction *scaleAction = new QgsScaleComboAction( menu );
  menu->addAction( scaleAction );
  mScaleCombo = scaleAction->scaleCombo();
  connect( mScaleCombo, &QgsScaleComboBox::scaleChanged, this, [ = ]( double scale )
  {
    if ( !mBlockScaleUpdate )
    {
      mBlockScaleUpdate = true;
      mMapCanvas->zoomScale( 1.0 / scale );
      mBlockScaleUpdate = false;
    }
  } );
  connect( mMapCanvas, &QgsMapCanvas::scaleChanged, this, [ = ]( double scale )
  {
    if ( !mBlockScaleUpdate )
    {
      mBlockScaleUpdate = true;
      mScaleCombo->setScale( 1.0 / scale );
      mBlockScaleUpdate = false;
    }
  } );
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

void QgsMapCanvasDockWidget::syncView( bool enabled )
{
  if ( enabled )
  {
    connect( mMainCanvas, &QgsMapCanvas::extentsChanged, this, &QgsMapCanvasDockWidget::mapExtentChanged, Qt::UniqueConnection );
    connect( mMapCanvas, &QgsMapCanvas::extentsChanged, this, &QgsMapCanvasDockWidget::mapExtentChanged, Qt::UniqueConnection );
  }
  else
  {
    disconnect( mMainCanvas, &QgsMapCanvas::extentsChanged, this, &QgsMapCanvasDockWidget::mapExtentChanged );
    disconnect( mMapCanvas, &QgsMapCanvas::extentsChanged, this, &QgsMapCanvasDockWidget::mapExtentChanged );
  }
}

void QgsMapCanvasDockWidget::mapExtentChanged()
{
  QgsMapCanvas *sourceCanvas = qobject_cast< QgsMapCanvas * >( sender() );
  if ( !sourceCanvas )
    return;

  // avoid infinite recursion
  syncView( false );

  QgsMapCanvas *destCanvas = sourceCanvas == mMapCanvas ? mMainCanvas : mMapCanvas;
  destCanvas->setExtent( sourceCanvas->extent() );
  destCanvas->refresh();

  syncView( true );
}

QgsScaleComboAction::QgsScaleComboAction( QWidget *parent )
  : QWidgetAction( parent )
{
  mScaleCombo = new QgsScaleComboBox();

  QHBoxLayout *hLayout = new QHBoxLayout();
  hLayout->setContentsMargins( 2, 2, 2, 2 );
  QLabel *label = new QLabel( tr( "Scale" ) );
  hLayout->addWidget( label );
  hLayout->addWidget( mScaleCombo );
  QWidget *w = new QWidget();
  w->setLayout( hLayout );
  setDefaultWidget( w );
}

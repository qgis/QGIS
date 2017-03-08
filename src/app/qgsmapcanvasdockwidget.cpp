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
#include "qgsstatusbarmagnifierwidget.h"
#include "qgsdoublespinbox.h"
#include "qgssettings.h"
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

  QgsMapSettingsAction *settingsAction = new QgsMapSettingsAction( menu );
  menu->addAction( settingsAction );
  mScaleCombo = settingsAction->scaleCombo();
  mRotationEdit = settingsAction->rotationSpinBox();
  mMagnificationEdit = settingsAction->magnifierSpinBox();
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

  connect( mRotationEdit, static_cast < void ( QgsDoubleSpinBox::* )( double ) > ( &QgsDoubleSpinBox::valueChanged ), this, [ = ]( double value )
  {
    if ( !mBlockRotationUpdate )
    {
      mBlockRotationUpdate = true;
      mMapCanvas->setRotation( value );
      mMapCanvas->refresh();
      mBlockRotationUpdate = false;
    }
  } );

  connect( mMapCanvas, &QgsMapCanvas::rotationChanged, this, [ = ]( double rotation )
  {
    if ( !mBlockRotationUpdate )
    {
      mBlockRotationUpdate = true;
      mRotationEdit->setValue( rotation );
      mBlockRotationUpdate = false;
    }
  } );

  connect( mMagnificationEdit, static_cast < void ( QgsDoubleSpinBox::* )( double ) > ( &QgsDoubleSpinBox::valueChanged ), this, [ = ]( double value )
  {
    if ( !mBlockMagnificationUpdate )
    {
      mBlockMagnificationUpdate = true;
      mMapCanvas->setMagnificationFactor( value / 100 );
      mMapCanvas->refresh();
      mBlockMagnificationUpdate = false;
    }
  } );

  connect( mMapCanvas, &QgsMapCanvas::magnificationChanged, this, [ = ]( double factor )
  {
    if ( !mBlockMagnificationUpdate )
    {
      mBlockMagnificationUpdate = true;
      mMagnificationEdit->setValue( factor * 100 );
      mBlockMagnificationUpdate = false;
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

QgsMapSettingsAction::QgsMapSettingsAction( QWidget *parent )
  : QWidgetAction( parent )
{
  QGridLayout *gLayout = new QGridLayout();
  gLayout->setContentsMargins( 3, 2, 3, 2 );
  QLabel *label = new QLabel( tr( "Scale" ) );
  gLayout->addWidget( label, 0, 0 );

  mScaleCombo = new QgsScaleComboBox();
  gLayout->addWidget( mScaleCombo, 0, 1 );

  mRotationWidget = new QgsDoubleSpinBox();
  mRotationWidget->setClearValue( 0.0 );
  mRotationWidget->setKeyboardTracking( false );
  mRotationWidget->setMaximumWidth( 120 );
  mRotationWidget->setDecimals( 1 );
  mRotationWidget->setRange( -180.0, 180.0 );
  mRotationWidget->setWrapping( true );
  mRotationWidget->setSingleStep( 5.0 );
  mRotationWidget->setToolTip( tr( "Current clockwise map rotation in degrees" ) );

  label = new QLabel( tr( "Rotation" ) );
  gLayout->addWidget( label, 1, 0 );
  gLayout->addWidget( mRotationWidget, 1, 1 );

  QgsSettings settings;
  int minimumFactor = 100 * QgisGui::CANVAS_MAGNIFICATION_MIN;
  int maximumFactor = 100 * QgisGui::CANVAS_MAGNIFICATION_MAX;
  int defaultFactor = 100 * settings.value( QStringLiteral( "/qgis/magnifier_factor_default" ), 1.0 ).toDouble();

  mMagnifierWidget = new QgsDoubleSpinBox();
  mMagnifierWidget->setSuffix( QStringLiteral( "%" ) );
  mMagnifierWidget->setKeyboardTracking( false );
  mMagnifierWidget->setDecimals( 0 );
  mMagnifierWidget->setRange( minimumFactor, maximumFactor );
  mMagnifierWidget->setWrapping( false );
  mMagnifierWidget->setSingleStep( 50 );
  mMagnifierWidget->setToolTip( tr( "Magnifier level" ) );
  mMagnifierWidget->setClearValueMode( QgsDoubleSpinBox::CustomValue );
  mMagnifierWidget->setClearValue( defaultFactor );
  mMagnifierWidget->setValue( defaultFactor );

  label = new QLabel( tr( "Magnification" ) );
  gLayout->addWidget( label, 2, 0 );
  gLayout->addWidget( mMagnifierWidget, 2, 1 );

  QWidget *w = new QWidget();
  w->setLayout( gLayout );
  setDefaultWidget( w );
}

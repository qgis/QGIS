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
#include "qgscsexception.h"
#include "qgsprojectionselectiondialog.h"
#include "qgsscalecombobox.h"
#include "qgsdoublespinbox.h"
#include "qgssettings.h"
#include "qgsmaptoolpan.h"
#include "qgsmapthemecollection.h"
#include "qgsproject.h"
#include "qgsmapthemes.h"
#include "qgslayertreeview.h"
#include "qgslayertreeviewdefaultactions.h"
#include "qgisapp.h"
#include "qgsvertexmarker.h"
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
  mXyMarker = new QgsVertexMarker( mMapCanvas );
  mXyMarker->setIconType( QgsVertexMarker::ICON_CIRCLE );
  mXyMarker->setIconSize( 6 );
  mXyMarker->setColor( QColor( 30, 30, 30, 225 ) );
  mPanTool = new QgsMapToolPan( mMapCanvas );
  mMapCanvas->setMapTool( mPanTool );

  mMainWidget->setLayout( new QVBoxLayout() );
  mMainWidget->layout()->setContentsMargins( 0, 0, 0, 0 );
  mMainWidget->layout()->setMargin( 0 );

  mMainWidget->layout()->addWidget( mMapCanvas );

  connect( mActionSyncView, &QAction::toggled, this, [ = ]( bool active )
  {
    syncViewExtent( mMainCanvas );
    syncView( active );
  } );

  mMenu = new QMenu();
  connect( mMenu, &QMenu::aboutToShow, this, &QgsMapCanvasDockWidget::menuAboutToShow );

  QToolButton *btnMapThemes = new QToolButton;
  btnMapThemes->setAutoRaise( true );
  btnMapThemes->setToolTip( tr( "Set View Theme" ) );
  btnMapThemes->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "/mActionShowAllLayers.svg" ) ) );
  btnMapThemes->setPopupMode( QToolButton::InstantPopup );
  btnMapThemes->setMenu( mMenu );
  mToolbar->addWidget( btnMapThemes );

  QMenu *settingsMenu = new QMenu();
  QToolButton *settingsButton = new QToolButton();
  settingsButton->setAutoRaise( true );
  settingsButton->setToolTip( tr( "View Settings" ) );
  settingsButton->setMenu( settingsMenu );
  settingsButton->setPopupMode( QToolButton::InstantPopup );
  settingsButton->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "/mActionMapSettings.svg" ) ) );
  mToolbar->addWidget( settingsButton );

  connect( mActionSetCrs, &QAction::triggered, this, &QgsMapCanvasDockWidget::setMapCrs );
  connect( mMapCanvas, &QgsMapCanvas::destinationCrsChanged, this, &QgsMapCanvasDockWidget::mapCrsChanged );
  connect( mActionZoomFullExtent, &QAction::triggered, mMapCanvas, &QgsMapCanvas::zoomToFullExtent );
  connect( mActionZoomToLayer, &QAction::triggered, mMapCanvas, [ = ] { QgisApp::instance()->layerTreeView()->defaultActions()->zoomToLayer( mMapCanvas ); } );
  connect( mActionZoomToSelected, &QAction::triggered, mMapCanvas, [ = ] { mMapCanvas->zoomToSelected(); } );
  mapCrsChanged();

  QgsMapSettingsAction *settingsAction = new QgsMapSettingsAction( settingsMenu );
  settingsMenu->addAction( settingsAction );

  settingsMenu->addSeparator();
  settingsMenu->addAction( mActionSetCrs );
  settingsMenu->addAction( mActionShowAnnotations );
  settingsMenu->addAction( mActionShowCursor );
  settingsMenu->addAction( mActionRename );

  connect( settingsMenu, &QMenu::aboutToShow, this, &QgsMapCanvasDockWidget::settingsMenuAboutToShow );

  connect( mActionRename, &QAction::triggered, this, &QgsMapCanvasDockWidget::renameTriggered );
  mActionShowAnnotations->setChecked( mMapCanvas->annotationsVisible() );
  connect( mActionShowAnnotations, &QAction::toggled, this, [ = ]( bool checked ) { mMapCanvas->setAnnotationsVisible( checked ); } );
  mActionShowCursor->setChecked( true );
  connect( mActionShowCursor, &QAction::toggled, this, [ = ]( bool checked ) { mXyMarker->setVisible( checked ); } );

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

  mResizeTimer.setSingleShot( true );
  connect( &mResizeTimer, &QTimer::timeout, this, [ = ]
  {
    mBlockExtentSync = false;
    syncViewExtent( mMainCanvas );
  } );
}

void QgsMapCanvasDockWidget::setMainCanvas( QgsMapCanvas *canvas )
{
  if ( mMainCanvas )
  {
    disconnect( mMainCanvas, &QgsMapCanvas::xyCoordinates, this, &QgsMapCanvasDockWidget::syncMarker );
  }

  mMainCanvas = canvas;
  connect( mMainCanvas, &QgsMapCanvas::xyCoordinates, this, &QgsMapCanvasDockWidget::syncMarker );
}

QgsMapCanvas *QgsMapCanvasDockWidget::mapCanvas()
{
  return mMapCanvas;
}

void QgsMapCanvasDockWidget::setViewExtentSynchronized( bool enabled )
{
  mActionSyncView->setChecked( enabled );
}

bool QgsMapCanvasDockWidget::isViewExtentSynchronized() const
{
  return mActionSyncView->isChecked();
}

void QgsMapCanvasDockWidget::setCursorMarkerVisible( bool visible )
{
  mActionShowCursor->setChecked( visible );
}

bool QgsMapCanvasDockWidget::isCursorMarkerVisible() const
{
  return mXyMarker->isVisible();
}

void QgsMapCanvasDockWidget::resizeEvent( QResizeEvent * )
{
  mBlockExtentSync = true;
  mResizeTimer.start( 500 );
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

void QgsMapCanvasDockWidget::syncViewExtent( QgsMapCanvas *sourceCanvas )
{
  // avoid infinite recursion
  mBlockExtentSync = true;

  QgsMapCanvas *destCanvas = sourceCanvas == mMapCanvas ? mMainCanvas : mMapCanvas;

  // reproject extent
  QgsCoordinateTransform ct( sourceCanvas->mapSettings().destinationCrs(),
                             destCanvas->mapSettings().destinationCrs() );
  try
  {
    destCanvas->setExtent( ct.transformBoundingBox( sourceCanvas->extent() ) );
  }
  catch ( QgsCsException & )
  {
    destCanvas->setExtent( sourceCanvas->extent() );
  }
  destCanvas->refresh();

  mBlockExtentSync = false;
}

void QgsMapCanvasDockWidget::mapExtentChanged()
{
  if ( mBlockExtentSync )
    return;

  QgsMapCanvas *sourceCanvas = qobject_cast< QgsMapCanvas * >( sender() );
  if ( !sourceCanvas )
    return;

  syncViewExtent( sourceCanvas );
}

void QgsMapCanvasDockWidget::mapCrsChanged()
{
  mActionSetCrs->setText( tr( "Change Map CRS (%1)…" ).arg( mMapCanvas->mapSettings().destinationCrs().isValid() ?
                          mMapCanvas->mapSettings().destinationCrs().authid() :
                          tr( "No projection" ) ) );
}

void QgsMapCanvasDockWidget::menuAboutToShow()
{
  qDeleteAll( mMenuPresetActions );
  mMenuPresetActions.clear();

  QString currentTheme = mMapCanvas->theme();

  QAction *actionFollowMain = new QAction( tr( "(default)" ), mMenu );
  actionFollowMain->setCheckable( true );
  if ( currentTheme.isEmpty() || !QgsProject::instance()->mapThemeCollection()->hasMapTheme( currentTheme ) )
  {
    actionFollowMain->setChecked( true );
  }
  connect( actionFollowMain, &QAction::triggered, this, [ = ]
  {
    mMapCanvas->setTheme( QString() );
    mMapCanvas->refresh();
  } );
  mMenuPresetActions.append( actionFollowMain );

  Q_FOREACH ( const QString &grpName, QgsProject::instance()->mapThemeCollection()->mapThemes() )
  {
    QAction *a = new QAction( grpName, mMenu );
    a->setCheckable( true );
    if ( grpName == currentTheme )
    {
      a->setChecked( true );
    }
    connect( a, &QAction::triggered, this, [a, this]
    {
      mMapCanvas->setTheme( a->text() );
      mMapCanvas->refresh();
    } );
    mMenuPresetActions.append( a );
  }
  mMenu->addActions( mMenuPresetActions );
}

void QgsMapCanvasDockWidget::settingsMenuAboutToShow()
{
  whileBlocking( mActionShowAnnotations )->setChecked( mMapCanvas->annotationsVisible() );
}

void QgsMapCanvasDockWidget::syncMarker( const QgsPoint &p )
{
  if ( !mXyMarker->isVisible() )
    return;

  // reproject point
  QgsCoordinateTransform ct( mMainCanvas->mapSettings().destinationCrs(),
                             mMapCanvas->mapSettings().destinationCrs() );
  QgsPoint t = p;
  try
  {
    t = ct.transform( p );
  }
  catch ( QgsCsException & )
  {}

  mXyMarker->setCenter( t );
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

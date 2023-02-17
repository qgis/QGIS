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
#include "qgsexception.h"
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
#include "qgsrubberband.h"
#include "qgsvectorlayer.h"
#include "qgsapplication.h"
#include <QMessageBox>
#include <QMenu>
#include <QToolBar>
#include <QToolButton>
#include <QRadioButton>


QgsMapCanvasDockWidget::QgsMapCanvasDockWidget( const QString &name, QWidget *parent )
  : QgsDockWidget( parent )
{
  setupUi( this );
  setAttribute( Qt::WA_DeleteOnClose );

  mContents->layout()->setContentsMargins( 0, 0, 0, 0 );
  static_cast< QVBoxLayout * >( mContents->layout() )->setSpacing( 0 );

  setWindowTitle( name );
  mToolbar->setIconSize( QgisApp::instance()->iconSize( true ) );

  mMapCanvas = new QgsMapCanvas( this );
  mXyMarker = new QgsVertexMarker( mMapCanvas );
  mXyMarker->setIconType( QgsVertexMarker::ICON_CIRCLE );
  mXyMarker->setIconSize( 6 );
  mXyMarker->setColor( QColor( 30, 30, 30, 225 ) );
  mXyMarker->setFillColor( QColor( 255, 255, 255, 225 ) );

  mExtentRubberBand = new QgsRubberBand( mMapCanvas, QgsWkbTypes::PolygonGeometry );
  mExtentRubberBand->setStrokeColor( Qt::red );
  mExtentRubberBand->setSecondaryStrokeColor( QColor( 255, 255, 255, 225 ) );
  mExtentRubberBand->setFillColor( Qt::transparent );

  mPanTool = new QgsMapToolPan( mMapCanvas );
  mMapCanvas->setMapTool( mPanTool );

  mMainWidget->setLayout( new QVBoxLayout() );
  mMainWidget->layout()->setContentsMargins( 0, 0, 0, 0 );

  mMainWidget->layout()->addWidget( mMapCanvas );

  mMenu = new QMenu();
  connect( mMenu, &QMenu::aboutToShow, this, &QgsMapCanvasDockWidget::menuAboutToShow );

  mToolbar->addSeparator();

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
  settingsButton->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "/mActionOptions.svg" ) ) );
  mToolbar->addWidget( settingsButton );

  connect( mActionSetCrs, &QAction::triggered, this, &QgsMapCanvasDockWidget::setMapCrs );
  connect( mMapCanvas, &QgsMapCanvas::destinationCrsChanged, this, &QgsMapCanvasDockWidget::mapCrsChanged );
  connect( mMapCanvas, &QgsMapCanvas::destinationCrsChanged, this, &QgsMapCanvasDockWidget::updateExtentRect );
  connect( mActionZoomFullExtent, &QAction::triggered, mMapCanvas, &QgsMapCanvas::zoomToProjectExtent );
  connect( mActionZoomToLayers, &QAction::triggered, mMapCanvas, [ = ] { QgisApp::instance()->layerTreeView()->defaultActions()->zoomToLayers( mMapCanvas ); } );
  connect( mActionZoomToSelected, &QAction::triggered, mMapCanvas, [ = ] { mMapCanvas->zoomToSelected(); } );
  mapCrsChanged();

  QgsMapSettingsAction *settingsAction = new QgsMapSettingsAction( settingsMenu );
  settingsMenu->addAction( settingsAction );

  settingsMenu->addSeparator();
  settingsMenu->addAction( mActionShowAnnotations );
  settingsMenu->addAction( mActionShowCursor );
  settingsMenu->addAction( mActionShowExtent );
  settingsMenu->addAction( mActionShowLabels );
  settingsMenu->addSeparator();
  settingsMenu->addAction( mActionSetCrs );
  settingsMenu->addAction( mActionRename );

  connect( settingsMenu, &QMenu::aboutToShow, this, &QgsMapCanvasDockWidget::settingsMenuAboutToShow );

  connect( mActionRename, &QAction::triggered, this, &QgsMapCanvasDockWidget::renameTriggered );
  mActionShowAnnotations->setChecked( mMapCanvas->annotationsVisible() );
  connect( mActionShowAnnotations, &QAction::toggled, this, [ = ]( bool checked ) { mMapCanvas->setAnnotationsVisible( checked ); } );
  mActionShowCursor->setChecked( true );
  connect( mActionShowCursor, &QAction::toggled, this, [ = ]( bool checked ) { mXyMarker->setVisible( checked ); } );
  mActionShowExtent->setChecked( false );
  connect( mActionShowExtent, &QAction::toggled, this, [ = ]( bool checked ) { mExtentRubberBand->setVisible( checked ); updateExtentRect(); } );
  mActionShowLabels->setChecked( true );
  connect( mActionShowLabels, &QAction::toggled, this, &QgsMapCanvasDockWidget::showLabels );


  mSyncExtentRadio = settingsAction->syncExtentRadio();
  mSyncSelectionRadio = settingsAction->syncSelectionRadio();
  mScaleCombo = settingsAction->scaleCombo();
  mRotationEdit = settingsAction->rotationSpinBox();
  mMagnificationEdit = settingsAction->magnifierSpinBox();
  mSyncScaleCheckBox = settingsAction->syncScaleCheckBox();
  mScaleFactorWidget = settingsAction->scaleFactorSpinBox();

  connect( mSyncSelectionRadio, &QRadioButton::toggled, this, [ = ]( bool checked )
  {
    autoZoomToSelection( checked );
    if ( checked )
    {
      syncSelection();
    }
  } );

  connect( mSyncExtentRadio, &QRadioButton::toggled, this, [ = ]( bool checked )
  {
    if ( checked )
    {
      syncViewCenter( mMainCanvas );
    }
  } );

  connect( mScaleCombo, &QgsScaleComboBox::scaleChanged, this, [ = ]( double scale )
  {
    if ( !mBlockScaleUpdate )
    {
      mBlockScaleUpdate = true;
      mMapCanvas->zoomScale( scale );
      mBlockScaleUpdate = false;
    }
  } );
  connect( mMapCanvas, &QgsMapCanvas::scaleChanged, this, [ = ]( double scale )
  {
    if ( !mBlockScaleUpdate )
    {
      mBlockScaleUpdate = true;
      mScaleCombo->setScale( scale );
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

  connect( mScaleFactorWidget, static_cast < void ( QgsDoubleSpinBox::* )( double ) > ( &QgsDoubleSpinBox::valueChanged ), this, &QgsMapCanvasDockWidget::mapScaleChanged );
  connect( mSyncScaleCheckBox, &QCheckBox::toggled, this, [ = ]( bool checked )
  {
    if ( checked )
      mapScaleChanged();
  }
         );

  mResizeTimer.setSingleShot( true );
  connect( &mResizeTimer, &QTimer::timeout, this, [ = ]
  {
    mBlockExtentSync = false;
    if ( mSyncExtentRadio->isChecked() )
      syncViewCenter( mMainCanvas );
  } );

  connect( QgsProject::instance()->mapThemeCollection(), &QgsMapThemeCollection::mapThemeRenamed, this, &QgsMapCanvasDockWidget::currentMapThemeRenamed );
}

void QgsMapCanvasDockWidget::setMainCanvas( QgsMapCanvas *canvas )
{
  if ( mMainCanvas )
  {
    disconnect( mMainCanvas, &QgsMapCanvas::xyCoordinates, this, &QgsMapCanvasDockWidget::syncMarker );
    disconnect( mMainCanvas, &QgsMapCanvas::scaleChanged, this, &QgsMapCanvasDockWidget::mapScaleChanged );
    disconnect( mMainCanvas, &QgsMapCanvas::extentsChanged, this, &QgsMapCanvasDockWidget::mapExtentChanged );
    disconnect( mMainCanvas, &QgsMapCanvas::extentsChanged, this, &QgsMapCanvasDockWidget::updateExtentRect );
    disconnect( mMainCanvas, &QgsMapCanvas::destinationCrsChanged, this, &QgsMapCanvasDockWidget::updateExtentRect );
  }

  mMainCanvas = canvas;
  connect( mMainCanvas, &QgsMapCanvas::xyCoordinates, this, &QgsMapCanvasDockWidget::syncMarker );
  connect( mMainCanvas, &QgsMapCanvas::scaleChanged, this, &QgsMapCanvasDockWidget::mapScaleChanged );
  connect( mMainCanvas, &QgsMapCanvas::extentsChanged, this, &QgsMapCanvasDockWidget::mapExtentChanged );
  connect( mMapCanvas, &QgsMapCanvas::extentsChanged, this, &QgsMapCanvasDockWidget::mapExtentChanged, Qt::UniqueConnection );
  connect( mMainCanvas, &QgsMapCanvas::extentsChanged, this, &QgsMapCanvasDockWidget::updateExtentRect );
  connect( mMainCanvas, &QgsMapCanvas::destinationCrsChanged, this, &QgsMapCanvasDockWidget::updateExtentRect );
  updateExtentRect();
}

QgsMapCanvas *QgsMapCanvasDockWidget::mapCanvas()
{
  return mMapCanvas;
}

void QgsMapCanvasDockWidget::setViewCenterSynchronized( bool enabled )
{
  mSyncExtentRadio->setChecked( enabled );
}

bool QgsMapCanvasDockWidget::isViewCenterSynchronized() const
{
  return mSyncExtentRadio->isChecked();
}

bool QgsMapCanvasDockWidget::isAutoZoomToSelected() const
{
  return mSyncSelectionRadio->isChecked();
}

void QgsMapCanvasDockWidget::setAutoZoomToSelected( bool autoZoom )
{
  mSyncSelectionRadio->setChecked( autoZoom );
}

void QgsMapCanvasDockWidget::setCursorMarkerVisible( bool visible )
{
  mActionShowCursor->setChecked( visible );
}

bool QgsMapCanvasDockWidget::isCursorMarkerVisible() const
{
  return mXyMarker->isVisible();
}

void QgsMapCanvasDockWidget::setMainCanvasExtentVisible( bool visible )
{
  mActionShowExtent->setChecked( visible );
}

bool QgsMapCanvasDockWidget::isMainCanvasExtentVisible() const
{
  return mExtentRubberBand->isVisible();
}

void QgsMapCanvasDockWidget::setScaleFactor( double factor )
{
  mScaleFactorWidget->setValue( factor );
}

void QgsMapCanvasDockWidget::setViewScaleSynchronized( bool enabled )
{
  mSyncScaleCheckBox->setChecked( enabled );
}

bool QgsMapCanvasDockWidget::isViewScaleSynchronized() const
{
  return mSyncScaleCheckBox->isChecked();
}

void QgsMapCanvasDockWidget::setLabelsVisible( bool enabled )
{
  mActionShowLabels->setChecked( enabled );
}

bool QgsMapCanvasDockWidget::labelsVisible() const
{
  return mActionShowLabels->isChecked();
}

double QgsMapCanvasDockWidget::scaleFactor() const
{
  return mScaleFactorWidget->value();
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

void QgsMapCanvasDockWidget::syncViewCenter( QgsMapCanvas *sourceCanvas )
{
  // avoid infinite recursion
  mBlockExtentSync = true;

  QgsMapCanvas *destCanvas = sourceCanvas == mMapCanvas ? mMainCanvas : mMapCanvas;

  // reproject extent
  QgsCoordinateTransform ct( sourceCanvas->mapSettings().destinationCrs(),
                             destCanvas->mapSettings().destinationCrs(), QgsProject::instance() );
  ct.setBallparkTransformsAreAppropriate( true );
  try
  {
    destCanvas->setCenter( ct.transform( sourceCanvas->center() ) );
  }
  catch ( QgsCsException & )
  {
    destCanvas->setCenter( sourceCanvas->center() );
  }
  destCanvas->refresh();

  mBlockExtentSync = false;
}

void QgsMapCanvasDockWidget::syncSelection()
{
  QgsVectorLayer *layer = qobject_cast<QgsVectorLayer *>( mMapCanvas->currentLayer() );

  if ( !layer )
    return;

  mMapCanvas->zoomToSelected( layer );
}

void QgsMapCanvasDockWidget::mapExtentChanged()
{
  if ( mBlockExtentSync )
    return;

  QgsMapCanvas *sourceCanvas = qobject_cast< QgsMapCanvas * >( sender() );
  if ( !sourceCanvas )
    return;

  if ( sourceCanvas == mMapCanvas && mSyncScaleCheckBox->isChecked() )
  {
    const double newScaleFactor = mMainCanvas->scale() / mMapCanvas->scale();
    mScaleFactorWidget->setValue( newScaleFactor );
  }

  if ( mSyncExtentRadio->isChecked() )
    syncViewCenter( sourceCanvas );
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

  const QString currentTheme = mMapCanvas->theme();

  QAction *actionFollowMain = new QAction( tr( "(none)" ), mMenu );
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

  const auto constMapThemes = QgsProject::instance()->mapThemeCollection()->mapThemes();
  for ( const QString &grpName : constMapThemes )
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

void QgsMapCanvasDockWidget::currentMapThemeRenamed( const QString &theme, const QString &newTheme )
{
  if ( theme == mMapCanvas->theme() )
  {
    mMapCanvas->setTheme( newTheme );
  }
}

void QgsMapCanvasDockWidget::settingsMenuAboutToShow()
{
  whileBlocking( mActionShowAnnotations )->setChecked( mMapCanvas->annotationsVisible() );
}

void QgsMapCanvasDockWidget::syncMarker( const QgsPointXY &p )
{
  if ( !mXyMarker->isVisible() )
    return;

  // reproject point
  const QgsCoordinateTransform ct( mMainCanvas->mapSettings().destinationCrs(),
                                   mMapCanvas->mapSettings().destinationCrs(), QgsProject::instance() );
  QgsPointXY t = p;
  try
  {
    t = ct.transform( p );
  }
  catch ( QgsCsException & )
  {}

  mXyMarker->setCenter( t );
}

void QgsMapCanvasDockWidget::mapScaleChanged()
{
  if ( !mSyncScaleCheckBox->isChecked() )
    return;

  const double newScale = mMainCanvas->scale() / mScaleFactorWidget->value();
  const bool prev = mBlockExtentSync;
  mBlockExtentSync = true;
  mMapCanvas->zoomScale( newScale );
  mBlockExtentSync = prev;
}

void QgsMapCanvasDockWidget::updateExtentRect()
{
  if ( !mExtentRubberBand->isVisible() )
    return;

  QPolygonF mainCanvasPoly = mMainCanvas->mapSettings().visiblePolygon();
  // close polygon
  mainCanvasPoly << mainCanvasPoly.at( 0 );
  QgsGeometry g = QgsGeometry::fromQPolygonF( mainCanvasPoly );
  if ( mMainCanvas->mapSettings().destinationCrs() !=
       mMapCanvas->mapSettings().destinationCrs() )
  {
    // reproject extent
    const QgsCoordinateTransform ct( mMainCanvas->mapSettings().destinationCrs(),
                                     mMapCanvas->mapSettings().destinationCrs(), QgsProject::instance() );
    g = g.densifyByCount( 5 );
    try
    {
      g.transform( ct );
    }
    catch ( QgsCsException & )
    {
    }
  }
  mExtentRubberBand->setToGeometry( g, nullptr );
}

void QgsMapCanvasDockWidget::showLabels( bool show )
{
  Qgis::MapSettingsFlags flags = mMapCanvas->mapSettings().flags();
  if ( show )
    flags = flags | Qgis::MapSettingsFlag::DrawLabeling;
  else
    flags = flags & ~( static_cast< int >( Qgis::MapSettingsFlag::DrawLabeling ) );
  mMapCanvas->setMapSettingsFlags( flags );
}

void QgsMapCanvasDockWidget::autoZoomToSelection( bool autoZoom )
{
  if ( autoZoom )
    connect( mMapCanvas, &QgsMapCanvas::selectionChanged, mMapCanvas, qOverload<QgsMapLayer *>( &QgsMapCanvas::zoomToSelected ) );
  else
    disconnect( mMapCanvas, &QgsMapCanvas::selectionChanged, mMapCanvas, qOverload<QgsMapLayer *>( &QgsMapCanvas::zoomToSelected ) );
}

QgsMapSettingsAction::QgsMapSettingsAction( QWidget *parent )
  : QWidgetAction( parent )
{
  QGridLayout *gLayout = new QGridLayout();
  gLayout->setContentsMargins( 3, 2, 3, 2 );

  mSyncExtentRadio = new QRadioButton( tr( "Synchronize View Center with Main Map" ) );
  gLayout->addWidget( mSyncExtentRadio, 0, 0, 1, 2 );

  mSyncSelectionRadio = new QRadioButton( tr( "Synchronize View to Selection" ) );
  gLayout->addWidget( mSyncSelectionRadio, 1, 0, 1, 2 );

  QLabel *label = new QLabel( tr( "Scale" ) );
  gLayout->addWidget( label, 2, 0 );

  mScaleCombo = new QgsScaleComboBox();
  gLayout->addWidget( mScaleCombo, 2, 1 );

  mRotationWidget = new QgsDoubleSpinBox();
  mRotationWidget->setClearValue( 0.0 );
  mRotationWidget->setKeyboardTracking( false );
  mRotationWidget->setMaximumWidth( 120 );
  mRotationWidget->setDecimals( 1 );
  mRotationWidget->setRange( -180.0, 180.0 );
  mRotationWidget->setWrapping( true );
  mRotationWidget->setSingleStep( 5.0 );
  mRotationWidget->setSuffix( tr( " °" ) );
  mRotationWidget->setToolTip( tr( "Current clockwise map rotation in degrees" ) );

  label = new QLabel( tr( "Rotation" ) );
  gLayout->addWidget( label, 3, 0 );
  gLayout->addWidget( mRotationWidget, 3, 1 );

  const QgsSettings settings;
  const int minimumFactor = 100 * QgsGuiUtils::CANVAS_MAGNIFICATION_MIN;
  const int maximumFactor = 100 * QgsGuiUtils::CANVAS_MAGNIFICATION_MAX;
  const int defaultFactor = 100 * settings.value( QStringLiteral( "/qgis/magnifier_factor_default" ), 1.0 ).toDouble();

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
  gLayout->addWidget( label, 4, 0 );
  gLayout->addWidget( mMagnifierWidget, 4, 1 );

  mSyncScaleCheckBox = new QCheckBox( tr( "Synchronize scale" ) );
  gLayout->addWidget( mSyncScaleCheckBox, 5, 0, 1, 2 );

  mScaleFactorWidget = new QgsDoubleSpinBox();
  mScaleFactorWidget->setSuffix( tr( "×" ) );
  mScaleFactorWidget->setDecimals( 2 );
  mScaleFactorWidget->setRange( 0.01, 100000 );
  mScaleFactorWidget->setWrapping( false );
  mScaleFactorWidget->setSingleStep( 0.1 );
  mScaleFactorWidget->setToolTip( tr( "Multiplication factor for main canvas scale to view scale" ) );
  mScaleFactorWidget->setClearValueMode( QgsDoubleSpinBox::CustomValue );
  mScaleFactorWidget->setClearValue( 1.0 );
  mScaleFactorWidget->setValue( 1.0 );
  mScaleFactorWidget->setEnabled( false );

  connect( mSyncScaleCheckBox, &QCheckBox::toggled, mScaleFactorWidget, &QgsDoubleSpinBox::setEnabled );

  label = new QLabel( tr( "Scale Factor" ) );
  gLayout->addWidget( label, 6, 0 );
  gLayout->addWidget( mScaleFactorWidget, 6, 1 );

  QWidget *w = new QWidget();
  w->setLayout( gLayout );
  setDefaultWidget( w );
}

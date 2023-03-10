/***************************************************************************
  qgsextent3dwidget.cpp
  --------------------------------------
  Date                 : July 2024
  Copyright            : (C) 2024 by Jean Felder
  Email                : jean dot felder at oslandia dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsextent3dwidget.h"

#include "qgsapplication.h"
#include "qgsbookmarkmodel.h"
#include "qgsdoublevalidator.h"
#include "qgsextentwidget.h"
#include "qgslayoutitemmap.h"
#include "qgslayoutmanager.h"
#include "qgsmapcanvas.h"
#include "qgsmaplayermodel.h"
#include "qgsmaplayerproxymodel.h"
#include "qgsmaptoolextent.h"
#include "qgsprintlayout.h"
#include "qgsspinbox.h"
#include "qgsreferencedgeometry.h"


QgsExtent3DWidget::QgsExtent3DWidget( QWidget *parent )
  : QgsCollapsibleGroupBox( parent )
{
  setupUi( this );
  mCoordinateTable->setMaximumWidth( mCoordinateTable->horizontalHeader()->length() );

  mLayerMenu = new QMenu( tr( "Calculate from Layer" ), this );
  connect( mLayerMenu, &QMenu::aboutToShow, this, &QgsExtent3DWidget::layerMenuAboutToShow );
  mMapLayerModel = new QgsMapLayerProxyModel( this );
  mMapLayerModel->setFilters( Qgis::LayerFilter::SpatialLayer );

  mLayoutMenu = new QMenu( tr( "Calculate from Layout Map" ), this );
  connect( mLayoutMenu, &QMenu::aboutToShow, this, &QgsExtent3DWidget::layoutMenuAboutToShow );

  mBookmarkMenu = new QMenu( tr( "Calculate from Bookmark" ), this );
  connect( mBookmarkMenu, &QMenu::aboutToShow, this, &QgsExtent3DWidget::bookmarkMenuAboutToShow );

  mDrawOnCanvasAction = new QAction( tr( "Draw on Map Canvas" ), this );
  mDrawOnCanvasAction->setVisible( false );
  connect( mDrawOnCanvasAction, &QAction::triggered, this, &QgsExtent3DWidget::setOutputExtentFromDrawOnCanvas );

  mUseCanvasExtentAction = new QAction( tr( "Use Current Map Canvas Extent" ), this );
  mUseCanvasExtentAction->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "/mActionMapIdentification.svg" ) ) );
  mUseCanvasExtentAction->setVisible( false );
  connect( mUseCanvasExtentAction, &QAction::triggered, this, &QgsExtent3DWidget::setOutputExtentFromCurrent );

  mUseDefaultExtentAction = new QAction( tr( "Use Current Layer/Default Extent" ), this );
  mUseDefaultExtentAction->setVisible( false );
  connect( mUseDefaultExtentAction, &QAction::triggered, this, &QgsExtent3DWidget::setOutputExtentFromCurrent );

  mMenu = new QMenu( this );
  mMenu->addMenu( mLayerMenu );
  mMenu->addMenu( mLayoutMenu );
  mMenu->addMenu( mBookmarkMenu );
  mMenu->addSeparator();
  mMenu->addAction( mUseCanvasExtentAction );
  mMenu->addAction( mDrawOnCanvasAction );
  mMenu->addAction( mUseDefaultExtentAction );

  mCalculateButton->setMenu( mMenu );
  mCalculateButton->setPopupMode( QToolButton::InstantPopup );

  connect( this, &QgsExtent3DWidget::toggleDialogVisibility, this, [ = ]( bool visible )
  {
    QWidget *w = window();
    // Don't hide the main window or we'll get locked outside!
    if ( w->objectName() == QLatin1String( "QgisApp" ) )
      return;
    w->setVisible( visible );
  } );

  connect( mRotationSpinBox, qOverload<int>( &QgsSpinBox::valueChanged ), this, &QgsExtent3DWidget::onRotationChanged );
  connect( mRotationSlider, &QSlider::valueChanged, this, &QgsExtent3DWidget::onRotationChanged );
}

void QgsExtent3DWidget::setMapCanvas( QgsMapCanvas *canvas )
{
  if ( canvas )
  {
    mCanvas = canvas;

    mUseCanvasExtentAction->setVisible( true );
    mDrawOnCanvasAction->setVisible( true );

    mCalculateButton->setToolTip( tr( "Set to current map canvas extent" ) );
    mCalculateButton->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "/mActionMapIdentification.svg" ) ) );
    connect( mCalculateButton, &QAbstractButton::clicked, this, &QgsExtent3DWidget::setOutputExtentFromCurrent );
    mCalculateButton->setPopupMode( QToolButton::MenuButtonPopup );
  }
  else
  {
    mDrawOnCanvasAction->setVisible( true );
    mUseCanvasExtentAction->setVisible( false );

    mCalculateButton->setToolTip( QString() );
    mCalculateButton->setIcon( QIcon() );
    disconnect( mCalculateButton, &QAbstractButton::clicked, this, &QgsExtent3DWidget::setOutputExtentFromCurrent );
    mCalculateButton->setPopupMode( QToolButton::InstantPopup );
  }
}

void QgsExtent3DWidget::layerMenuAboutToShow()
{
  qDeleteAll( mLayerMenuActions );
  mLayerMenuActions.clear();
  mLayerMenu->clear();
  for ( int i = 0; i < mMapLayerModel->rowCount(); ++i )
  {
    const QModelIndex index = mMapLayerModel->index( i, 0 );
    const QIcon icon = qvariant_cast<QIcon>( mMapLayerModel->data( index, Qt::DecorationRole ) );
    const QString text = mMapLayerModel->data( index, Qt::DisplayRole ).toString();
    QAction *act = new QAction( icon, text, mLayerMenu );
    act->setToolTip( mMapLayerModel->data( index, Qt::ToolTipRole ).toString() );
    const QString layerId = mMapLayerModel->data( index, static_cast< int >( QgsMapLayerModel::CustomRole::LayerId ) ).toString();
    if ( mExtentLayer && mExtentLayer->id() == layerId )
    {
      act->setCheckable( true );
      act->setChecked( true );
    }
    connect( act, &QAction::triggered, this, [this, layerId]
    {
      QgsMapLayer *layer = QgsProject::instance()->mapLayer( layerId );
      if ( !layer )
        return;

      mExtentLayer = layer;
      setOutputExtent( layer->extent(), layer->crs(), QgsExtentWidget::ProjectLayerExtent );
    } );
    mLayerMenu->addAction( act );
    mLayerMenuActions << act;
  }
}

void QgsExtent3DWidget::layoutMenuAboutToShow()
{
  mLayoutMenu->clear();

  if ( QgsLayoutManager *manager = QgsProject::instance()->layoutManager() )
  {
    const QList<QgsPrintLayout *> layouts = manager->printLayouts();
    for ( const QgsPrintLayout *layout : layouts )
    {
      QList< QgsLayoutItemMap * > maps;
      layout->layoutItems( maps );
      if ( maps.empty() )
        continue;

      QMenu *layoutMenu = new QMenu( layout->name(), mMenu );
      for ( const QgsLayoutItemMap *map : std::as_const( maps ) )
      {
        QgsRectangle extent = map->extent();
        QgsCoordinateReferenceSystem crs = map->crs();
        QAction *mapExtentAction = new QAction( tr( "%1" ).arg( map->displayName() ), mLayoutMenu );
        connect( mapExtentAction, &QAction::triggered, this, [this, extent, crs] { setOutputExtent( extent, crs, QgsExtentWidget::UserExtent ); } );
        layoutMenu->addAction( mapExtentAction );
      }
      mLayoutMenu->addMenu( layoutMenu );
    }
  }
}

void QgsExtent3DWidget::bookmarkMenuAboutToShow()
{
  mBookmarkMenu->clear();

  if ( !mBookmarkModel )
    mBookmarkModel = new QgsBookmarkManagerProxyModel( QgsApplication::bookmarkManager(), QgsProject::instance()->bookmarkManager(), this );

  QMap< QString, QMenu * > groupMenus;
  for ( int i = 0; i < mBookmarkModel->rowCount(); ++i )
  {
    const QString group = mBookmarkModel->data( mBookmarkModel->index( i, 0 ), static_cast< int >( QgsBookmarkManagerModel::CustomRole::Group ) ).toString();
    QMenu *destMenu = mBookmarkMenu;
    if ( !group.isEmpty() )
    {
      destMenu = groupMenus.value( group );
      if ( !destMenu )
      {
        destMenu = new QMenu( group, mBookmarkMenu );
        groupMenus[ group ] = destMenu;
      }
    }
    QAction *action = new QAction( mBookmarkModel->data( mBookmarkModel->index( i, 0 ), static_cast< int >( QgsBookmarkManagerModel::CustomRole::Name ) ).toString(), mBookmarkMenu );
    const QgsReferencedRectangle extent = mBookmarkModel->data( mBookmarkModel->index( i, 0 ), static_cast< int >( QgsBookmarkManagerModel::CustomRole::Extent ) ).value< QgsReferencedRectangle >();
    connect( action, &QAction::triggered, this, [ = ] { setOutputExtent( extent, extent.crs(), QgsExtentWidget::UserExtent ); } );
    destMenu->addAction( action );
  }

  QStringList groupKeys = groupMenus.keys();
  groupKeys.sort( Qt::CaseInsensitive );
  for ( int i = 0; i < groupKeys.count(); ++i )
  {
    if ( mBookmarkMenu->actions().value( i ) )
      mBookmarkMenu->insertMenu( mBookmarkMenu->actions().at( i ), groupMenus.value( groupKeys.at( i ) ) );
    else
      mBookmarkMenu->addMenu( groupMenus.value( groupKeys.at( i ) ) );
  }
}

void QgsExtent3DWidget::setOutputExtentFromCurrent()
{
  if ( mCanvas )
  {
    // Use unrotated visible extent to insure output size and scale matches canvas
    QgsMapSettings mapSettings = mCanvas->mapSettings();
    mapSettings.setRotation( 0 );
    setOutputExtent( mapSettings.visibleExtent(), mapSettings.destinationCrs(), QgsExtentWidget::CurrentExtent );
  }
  else
  {
    setOutputExtent( mDefaultExtent, mCrs, QgsExtentWidget::CurrentExtent );
  }
}

void QgsExtent3DWidget::setOutputExtentFromDrawOnCanvas()
{
  if ( mCanvas )
  {
    mMapToolPrevious = mCanvas->mapTool();
    if ( !mMapToolExtent )
    {
      mMapToolExtent.reset( new QgsMapToolExtent( mCanvas ) );
      connect( mMapToolExtent.get(), &QgsMapToolExtent::extentChanged, this, &QgsExtent3DWidget::extentDrawn );
      connect( mMapToolExtent.get(), &QgsMapTool::deactivated, this, &QgsExtent3DWidget::mapToolDeactivated );
    }
    mCanvas->setMapTool( mMapToolExtent.get() );
    emit toggleDialogVisibility( false );
  }
}

void QgsExtent3DWidget::extentDrawn( const QgsRectangle &extent )
{
  setOutputExtent( extent, mCanvas->mapSettings().destinationCrs(), QgsExtentWidget::DrawOnCanvas );
  mCanvas->setMapTool( mMapToolPrevious );
  emit toggleDialogVisibility( true );
  mMapToolPrevious = nullptr;
}

void QgsExtent3DWidget::mapToolDeactivated()
{
  emit toggleDialogVisibility( true );
  mMapToolPrevious = nullptr;
}

void QgsExtent3DWidget::setDefaultExtent( const QgsRectangle &defaultExtent, const QgsCoordinateReferenceSystem &crs )
{
  mDefaultExtent = defaultExtent;
  mCrs = crs;
  mUseDefaultExtentAction->setVisible( true );
  setOutputExtent( defaultExtent, crs, QgsExtentWidget::UserExtent );
  mCrsValue->setText( mCrs.authid() );
}

void QgsExtent3DWidget::setOutputExtent( const QgsRectangle &rectangle, const QgsCoordinateReferenceSystem &srcCrs, const QgsExtentWidget::ExtentState &state )
{
  QgsRectangle extent;
  if ( mCrs == srcCrs )
  {
    extent = rectangle;
  }
  else
  {
    try
    {
      QgsCoordinateTransform ct( srcCrs, mCrs, QgsProject::instance() );
      ct.setBallparkTransformsAreAppropriate( true );
      extent = ct.transformBoundingBox( rectangle );
    }
    catch ( QgsCsException & )
    {
      // can't reproject
      extent = rectangle;
    }
  }

  int decimals = 4;
  switch ( mCrs.mapUnits() )
  {
    case Qgis::DistanceUnit::Degrees:
    case Qgis::DistanceUnit::Unknown:
      decimals = 9;
      break;
    case Qgis::DistanceUnit::Meters:
    case Qgis::DistanceUnit::Kilometers:
    case Qgis::DistanceUnit::Feet:
    case Qgis::DistanceUnit::NauticalMiles:
    case Qgis::DistanceUnit::Yards:
    case Qgis::DistanceUnit::Miles:
    case Qgis::DistanceUnit::Centimeters:
    case Qgis::DistanceUnit::Millimeters:
    case Qgis::DistanceUnit::Inches:
      decimals = 4;
      break;
  }

  whileBlocking( mCoordinateTable )->item( 0, 0 )->setText( QLocale().toString( extent.xMinimum(), 'f', decimals ) );
  whileBlocking( mCoordinateTable )->item( 0, 1 )->setText( QLocale().toString( extent.xMaximum(), 'f', decimals ) );
  whileBlocking( mCoordinateTable )->item( 1, 0 )->setText( QLocale().toString( extent.yMinimum(), 'f', decimals ) );
  whileBlocking( mCoordinateTable )->item( 1, 1 )->setText( QLocale().toString( extent.yMaximum(), 'f', decimals ) );

  mSurfaceValue->setText( QLocale().toString( extent.area(), 'f', 2 ) );
  mLengthValue->setText( QLocale().toString( extent.height(), 'f', 2 ) );
  mWidthValue->setText( QLocale().toString( extent.width(), 'f', 2 ) );

  if ( state != QgsExtentWidget::ProjectLayerExtent )
  {
    mExtentLayer.clear();
  }

  updateTitle( state );
}

void QgsExtent3DWidget::updateTitle( const QgsExtentWidget::ExtentState &state )
{
  QString msg;
  switch ( state )
  {
    case QgsExtentWidget::OriginalExtent:
      msg = tr( "layer" );
      break;
    case QgsExtentWidget::CurrentExtent:
      msg = tr( "map view" );
      break;
    case QgsExtentWidget::UserExtent:
      msg = tr( "user defined" );
      break;
    case QgsExtentWidget::ProjectLayerExtent:
      msg = mExtentLayer.data()->name();
      break;
    case QgsExtentWidget::DrawOnCanvas:
      msg = tr( "drawn on canvas" );
      break;
  }
  msg = tr( "Extent (current: %1)" ).arg( msg );

  setTitle( msg );
}

QgsRectangle QgsExtent3DWidget::extent() const
{
  return QgsRectangle( QgsDoubleValidator::toDouble( mCoordinateTable->item( 0, 0 )->text() ),
                       QgsDoubleValidator::toDouble( mCoordinateTable->item( 1, 0 )->text() ),
                       QgsDoubleValidator::toDouble( mCoordinateTable->item( 0, 1 )->text() ),
                       QgsDoubleValidator::toDouble( mCoordinateTable->item( 1, 1 )->text() ) );
}

void QgsExtent3DWidget::onRotationChanged( int rotation )
{
  setRotation( rotation );
}

void QgsExtent3DWidget::setRotation( double rotation )
{
  whileBlocking( mRotationSpinBox )->setValue( static_cast<int>( rotation ) );
  whileBlocking( mRotationSlider )->setValue( static_cast<int>( rotation ) );
}

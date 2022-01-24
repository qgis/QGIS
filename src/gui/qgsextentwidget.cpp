/***************************************************************************
    qgsextentwidget.cpp
    ---------------------
    begin                : March 2020
    copyright            : (C) 2020 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsextentwidget.h"

#include "qgsapplication.h"
#include "qgslogger.h"
#include "qgscoordinatetransform.h"
#include "qgsmapcanvas.h"
#include "qgsmaplayerproxymodel.h"
#include "qgsmaplayermodel.h"
#include "qgsexception.h"
#include "qgsproject.h"
#include "qgsdoublevalidator.h"
#include "qgslayoutmanager.h"
#include "qgslayoutitemmap.h"
#include "qgsprintlayout.h"
#include "qgsbookmarkmodel.h"
#include "qgsreferencedgeometry.h"

#include <QMenu>
#include <QAction>
#include <QRegularExpression>

QgsExtentWidget::QgsExtentWidget( QWidget *parent, WidgetStyle style )
  : QWidget( parent )
{
  setupUi( this );
  connect( mXMinLineEdit, &QLineEdit::textEdited, this, &QgsExtentWidget::setOutputExtentFromLineEdit );
  connect( mXMaxLineEdit, &QLineEdit::textEdited, this, &QgsExtentWidget::setOutputExtentFromLineEdit );
  connect( mYMinLineEdit, &QLineEdit::textEdited, this, &QgsExtentWidget::setOutputExtentFromLineEdit );
  connect( mYMaxLineEdit, &QLineEdit::textEdited, this, &QgsExtentWidget::setOutputExtentFromLineEdit );

  mCondensedRe = QRegularExpression( QStringLiteral( "\\s*([\\d\\.\\-]+)\\s*,\\s*([\\d\\.\\-]+)\\s*,\\s*([\\d\\.\\-]+)\\s*,\\s*([\\d\\.\\-]+)\\s*(?:\\[(.*?)\\])?" ) );
  mCondensedLineEdit->setValidator( new QRegularExpressionValidator( mCondensedRe, this ) );
  mCondensedLineEdit->setShowClearButton( false );
  connect( mCondensedLineEdit, &QgsFilterLineEdit::cleared, this, &QgsExtentWidget::clear );
  connect( mCondensedLineEdit, &QLineEdit::textEdited, this, &QgsExtentWidget::setOutputExtentFromCondensedLineEdit );

  mLayerMenu = new QMenu( tr( "Calculate from Layer" ), this );
  mButtonCalcFromLayer->setMenu( mLayerMenu );
  connect( mLayerMenu, &QMenu::aboutToShow, this, &QgsExtentWidget::layerMenuAboutToShow );
  mMapLayerModel = new QgsMapLayerProxyModel( this );
  mMapLayerModel->setFilters( QgsMapLayerProxyModel::Filter::SpatialLayer );

  mLayoutMenu = new QMenu( tr( "Calculate from Layout Map" ), this );
  mButtonCalcFromLayout->setMenu( mLayoutMenu );
  connect( mLayoutMenu, &QMenu::aboutToShow, this, &QgsExtentWidget::layoutMenuAboutToShow );

  mBookmarkMenu = new QMenu( tr( "Calculate from Bookmark" ), this );
  mButtonCalcFromBookmark->setMenu( mBookmarkMenu );
  connect( mBookmarkMenu, &QMenu::aboutToShow, this, &QgsExtentWidget::bookmarkMenuAboutToShow );

  mMenu = new QMenu( this );
  mUseCanvasExtentAction = new QAction( tr( "Use Current Map Canvas Extent" ), this );
  mUseCanvasExtentAction->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "/mActionMapIdentification.svg" ) ) );
  mUseCanvasExtentAction->setVisible( false );
  connect( mUseCanvasExtentAction, &QAction::triggered, this, &QgsExtentWidget::setOutputExtentFromCurrent );

  mUseCurrentExtentAction = new QAction( tr( "Use Current Layer/Default Extent" ), this );
  mUseCurrentExtentAction->setVisible( false );
  connect( mUseCurrentExtentAction, &QAction::triggered, this, &QgsExtentWidget::setOutputExtentFromCurrent );

  mDrawOnCanvasAction = new QAction( tr( "Draw on Map Canvas" ), this );
  mDrawOnCanvasAction->setVisible( false );
  connect( mDrawOnCanvasAction, &QAction::triggered, this, &QgsExtentWidget::setOutputExtentFromDrawOnCanvas );

  mMenu->addMenu( mLayerMenu );
  mMenu->addMenu( mLayoutMenu );
  mMenu->addMenu( mBookmarkMenu );
  mMenu->addSeparator();
  mMenu->addAction( mUseCanvasExtentAction );
  mMenu->addAction( mDrawOnCanvasAction );
  mMenu->addAction( mUseCurrentExtentAction );

  mCondensedToolButton->setMenu( mMenu );
  mCondensedToolButton->setPopupMode( QToolButton::InstantPopup );

  mXMinLineEdit->setValidator( new QgsDoubleValidator( this ) );
  mXMaxLineEdit->setValidator( new QgsDoubleValidator( this ) );
  mYMinLineEdit->setValidator( new QgsDoubleValidator( this ) );
  mYMaxLineEdit->setValidator( new QgsDoubleValidator( this ) );

  mOriginalExtentButton->setVisible( false );
  mButtonDrawOnCanvas->setVisible( false );
  mCurrentExtentButton->setVisible( false );

  connect( mCurrentExtentButton, &QAbstractButton::clicked, this, &QgsExtentWidget::setOutputExtentFromCurrent );
  connect( mOriginalExtentButton, &QAbstractButton::clicked, this, &QgsExtentWidget::setOutputExtentFromOriginal );
  connect( mButtonDrawOnCanvas, &QAbstractButton::clicked, this, &QgsExtentWidget::setOutputExtentFromDrawOnCanvas );

  switch ( style )
  {
    case CondensedStyle:
      mExpandedWidget->hide();
      break;

    case ExpandedStyle:
      mCondensedFrame->hide();
      break;
  }

  setAcceptDrops( true );
}

QgsExtentWidget::~QgsExtentWidget()
{
  if ( mMapToolExtent )
  {
    // disconnect from deactivated signal -- this will be called when the map tool is being destroyed,
    // and we don't want to act on that anymore (the mapToolDeactivated slot tries to show the widget again, but
    // that's being destroyed!)
    disconnect( mMapToolExtent.get(), &QgsMapToolExtent::deactivated, this, &QgsExtentWidget::mapToolDeactivated );
  }
}

void QgsExtentWidget::setOriginalExtent( const QgsRectangle &originalExtent, const QgsCoordinateReferenceSystem &originalCrs )
{
  mOriginalExtent = originalExtent;
  mOriginalCrs = originalCrs;

  mOriginalExtentButton->setVisible( true );
}


void QgsExtentWidget::setCurrentExtent( const QgsRectangle &currentExtent, const QgsCoordinateReferenceSystem &currentCrs )
{
  mCurrentExtent = currentExtent;
  mCurrentCrs = currentCrs;

  mCurrentExtentButton->setVisible( true );
  mUseCurrentExtentAction->setVisible( true );
}

void QgsExtentWidget::setOutputCrs( const QgsCoordinateReferenceSystem &outputCrs )
{
  mHasFixedOutputCrs = true;
  if ( mOutputCrs != outputCrs )
  {
    const bool prevExtentEnabled = mIsValid;
    switch ( mExtentState )
    {
      case CurrentExtent:
        mOutputCrs = outputCrs;
        setOutputExtentFromCurrent();
        break;

      case OriginalExtent:
        mOutputCrs = outputCrs;
        setOutputExtentFromOriginal();
        break;

      case ProjectLayerExtent:
        mOutputCrs = outputCrs;
        setOutputExtentFromLayer( mExtentLayer.data() );
        break;

      case DrawOnCanvas:
        mOutputCrs = outputCrs;
        extentDrawn( outputExtent() );
        break;

      case UserExtent:
        try
        {
          QgsCoordinateTransform ct( mOutputCrs, outputCrs, QgsProject::instance() );
          ct.setBallparkTransformsAreAppropriate( true );
          const QgsRectangle extent = ct.transformBoundingBox( outputExtent() );
          mOutputCrs = outputCrs;
          setOutputExtentFromUser( extent, outputCrs );
        }
        catch ( QgsCsException & )
        {
          // can't reproject
          mOutputCrs = outputCrs;
        }
        break;
    }

    if ( !prevExtentEnabled )
      setValid( false );
  }
}

void QgsExtentWidget::setOutputExtent( const QgsRectangle &r, const QgsCoordinateReferenceSystem &srcCrs, ExtentState state )
{
  QgsRectangle extent;
  if ( !mHasFixedOutputCrs )
  {
    mOutputCrs = srcCrs;
    extent = r;
  }
  else
  {
    if ( mOutputCrs == srcCrs )
    {
      extent = r;
    }
    else
    {
      try
      {
        QgsCoordinateTransform ct( srcCrs, mOutputCrs, QgsProject::instance() );
        ct.setBallparkTransformsAreAppropriate( true );
        extent = ct.transformBoundingBox( r );
      }
      catch ( QgsCsException & )
      {
        // can't reproject
        extent = r;
      }
    }
  }

  int decimals = 4;
  switch ( mOutputCrs.mapUnits() )
  {
    case QgsUnitTypes::DistanceDegrees:
    case QgsUnitTypes::DistanceUnknownUnit:
      decimals = 9;
      break;
    case QgsUnitTypes::DistanceMeters:
    case QgsUnitTypes::DistanceKilometers:
    case QgsUnitTypes::DistanceFeet:
    case QgsUnitTypes::DistanceNauticalMiles:
    case QgsUnitTypes::DistanceYards:
    case QgsUnitTypes::DistanceMiles:
    case QgsUnitTypes::DistanceCentimeters:
    case QgsUnitTypes::DistanceMillimeters:
      decimals = 4;
      break;
  }
  mXMinLineEdit->setText( QLocale().toString( extent.xMinimum(), 'f', decimals ) );
  mXMaxLineEdit->setText( QLocale().toString( extent.xMaximum(), 'f', decimals ) );
  mYMinLineEdit->setText( QLocale().toString( extent.yMinimum(), 'f', decimals ) );
  mYMaxLineEdit->setText( QLocale().toString( extent.yMaximum(), 'f', decimals ) );

  QString condensed = QStringLiteral( "%1,%2,%3,%4" ).arg( QString::number( extent.xMinimum(), 'f', decimals ),
                      QString::number( extent.xMaximum(), 'f', decimals ),
                      QString::number( extent.yMinimum(), 'f', decimals ),
                      QString::number( extent.yMaximum(), 'f', decimals ) );
  condensed += QStringLiteral( " [%1]" ).arg( mOutputCrs.userFriendlyIdentifier( QgsCoordinateReferenceSystem::ShortString ) );
  mCondensedLineEdit->setText( condensed );

  mExtentState = state;

  if ( !mIsValid )
    setValid( true );

  emit extentChanged( extent );
}

void QgsExtentWidget::setOutputExtentFromLineEdit()
{
  mExtentState = UserExtent;
  emit extentChanged( outputExtent() );
}

void QgsExtentWidget::setOutputExtentFromCondensedLineEdit()
{
  const QString text = mCondensedLineEdit->text();
  if ( text.isEmpty() )
  {
    clear();
  }
  else
  {
    const QRegularExpressionMatch match = mCondensedRe.match( text );
    if ( match.hasMatch() )
    {
      // Localization
      whileBlocking( mXMinLineEdit )->setText( QLocale().toString( match.captured( 1 ).toDouble() ) );
      whileBlocking( mXMaxLineEdit )->setText( QLocale().toString( match.captured( 2 ).toDouble() ) );
      whileBlocking( mYMinLineEdit )->setText( QLocale().toString( match.captured( 3 ).toDouble() ) );
      whileBlocking( mYMaxLineEdit )->setText( QLocale().toString( match.captured( 4 ).toDouble() ) );
      if ( !match.captured( 5 ).isEmpty() )
      {
        mOutputCrs = QgsCoordinateReferenceSystem( match.captured( 5 ) );
      }

      emit extentChanged( outputExtent() );
      if ( !mIsValid )
        setValid( true );
    }
  }
}

void QgsExtentWidget::clear()
{
  const bool prevWasNull = mIsValid;

  whileBlocking( mXMinLineEdit )->clear();
  whileBlocking( mXMaxLineEdit )->clear();
  whileBlocking( mYMinLineEdit )->clear();
  whileBlocking( mYMaxLineEdit )->clear();
  whileBlocking( mCondensedLineEdit )->clearValue();
  setValid( false );

  if ( prevWasNull )
    emit extentChanged( outputExtent() );
}

QString QgsExtentWidget::extentLayerName() const
{
  return mExtentLayerName;
}

bool QgsExtentWidget::isValid() const
{
  return mIsValid;
}

void QgsExtentWidget::setNullValueAllowed( bool allowed, const QString &notSetText )
{
  mCondensedLineEdit->setShowClearButton( allowed );
  mCondensedLineEdit->setNullValue( notSetText );
}

void QgsExtentWidget::setValid( bool valid )
{
  if ( valid == mIsValid )
    return;

  mIsValid = valid;
  emit validationChanged( mIsValid );
}

void QgsExtentWidget::layerMenuAboutToShow()
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
    const QString layerId = mMapLayerModel->data( index, QgsMapLayerModel::LayerIdRole ).toString();
    if ( mExtentState == ProjectLayerExtent && mExtentLayer && mExtentLayer->id() == layerId )
    {
      act->setCheckable( true );
      act->setChecked( true );
    }
    connect( act, &QAction::triggered, this, [this, layerId]
    {
      setExtentToLayerExtent( layerId );
    } );
    mLayerMenu->addAction( act );
    mLayerMenuActions << act;
  }
}

void QgsExtentWidget::layoutMenuAboutToShow()
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
        connect( mapExtentAction, &QAction::triggered, this, [this, extent, crs] { setOutputExtentFromUser( extent, crs ); } );
        layoutMenu->addAction( mapExtentAction );
      }
      mLayoutMenu->addMenu( layoutMenu );
    }
  }
}

void QgsExtentWidget::bookmarkMenuAboutToShow()
{
  mBookmarkMenu->clear();

  if ( !mBookmarkModel )
    mBookmarkModel = new QgsBookmarkManagerProxyModel( QgsApplication::bookmarkManager(), QgsProject::instance()->bookmarkManager(), this );

  QMap< QString, QMenu * > groupMenus;
  for ( int i = 0; i < mBookmarkModel->rowCount(); ++i )
  {
    const QString group = mBookmarkModel->data( mBookmarkModel->index( i, 0 ), QgsBookmarkManagerModel::RoleGroup ).toString();
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
    QAction *action = new QAction( mBookmarkModel->data( mBookmarkModel->index( i, 0 ), QgsBookmarkManagerModel::RoleName ).toString(), mBookmarkMenu );
    const QgsReferencedRectangle extent = mBookmarkModel->data( mBookmarkModel->index( i, 0 ), QgsBookmarkManagerModel::RoleExtent ).value< QgsReferencedRectangle >();
    connect( action, &QAction::triggered, this, [ = ] { setOutputExtentFromUser( extent, extent.crs() ); } );
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

void QgsExtentWidget::setExtentToLayerExtent( const QString &layerId )
{
  QgsMapLayer *layer = QgsProject::instance()->mapLayer( layerId );
  if ( !layer )
    return;

  setOutputExtentFromLayer( layer );
}

QgsMapLayer *QgsExtentWidget::mapLayerFromMimeData( const QMimeData *data ) const
{
  const QgsMimeDataUtils::UriList uriList = QgsMimeDataUtils::decodeUriList( data );
  for ( const QgsMimeDataUtils::Uri &u : uriList )
  {
    // is this uri from the current project?
    if ( QgsMapLayer *layer = u.mapLayer() )
    {
      return layer;
    }
  }
  return nullptr;
}

void QgsExtentWidget::setOutputExtentFromCurrent()
{
  if ( mCanvas )
  {
    // Use unrotated visible extent to insure output size and scale matches canvas
    QgsMapSettings ms = mCanvas->mapSettings();
    ms.setRotation( 0 );
    setOutputExtent( ms.visibleExtent(), ms.destinationCrs(), CurrentExtent );
  }
  else
  {
    setOutputExtent( mCurrentExtent, mCurrentCrs, CurrentExtent );
  }
}


void QgsExtentWidget::setOutputExtentFromOriginal()
{
  setOutputExtent( mOriginalExtent, mOriginalCrs, OriginalExtent );
}

void QgsExtentWidget::setOutputExtentFromUser( const QgsRectangle &extent, const QgsCoordinateReferenceSystem &crs )
{
  setOutputExtent( extent, crs, UserExtent );
}

void QgsExtentWidget::setOutputExtentFromLayer( const QgsMapLayer *layer )
{
  if ( !layer )
    return;

  mExtentLayer = layer;
  mExtentLayerName = layer->name();

  setOutputExtent( layer->extent(), layer->crs(), ProjectLayerExtent );
}

void QgsExtentWidget::setOutputExtentFromDrawOnCanvas()
{
  if ( mCanvas )
  {
    mMapToolPrevious = mCanvas->mapTool();
    if ( !mMapToolExtent )
    {
      mMapToolExtent.reset( new QgsMapToolExtent( mCanvas ) );
      connect( mMapToolExtent.get(), &QgsMapToolExtent::extentChanged, this, &QgsExtentWidget::extentDrawn );
      connect( mMapToolExtent.get(), &QgsMapTool::deactivated, this, &QgsExtentWidget::mapToolDeactivated );
    }
    mMapToolExtent->setRatio( mRatio );
    mCanvas->setMapTool( mMapToolExtent.get() );

    emit toggleDialogVisibility( false );
  }
}

void QgsExtentWidget::extentDrawn( const QgsRectangle &extent )
{
  setOutputExtent( extent, mCanvas->mapSettings().destinationCrs(), DrawOnCanvas );
  mCanvas->setMapTool( mMapToolPrevious );
  emit toggleDialogVisibility( true );
  mMapToolPrevious = nullptr;
}

void QgsExtentWidget::mapToolDeactivated()
{
  emit toggleDialogVisibility( true );
  mMapToolPrevious = nullptr;
}

QgsRectangle QgsExtentWidget::outputExtent() const
{
  return QgsRectangle( QgsDoubleValidator::toDouble( mXMinLineEdit->text() ), QgsDoubleValidator::toDouble( mYMinLineEdit->text() ),
                       QgsDoubleValidator::toDouble( mXMaxLineEdit->text() ), QgsDoubleValidator::toDouble( mYMaxLineEdit->text() ) );
}

void QgsExtentWidget::setMapCanvas( QgsMapCanvas *canvas, bool drawOnCanvasOption )
{
  if ( canvas )
  {
    mCanvas = canvas;
    mButtonDrawOnCanvas->setVisible( drawOnCanvasOption );
    mCurrentExtentButton->setVisible( true );

    mUseCanvasExtentAction->setVisible( true );
    if ( drawOnCanvasOption )
      mDrawOnCanvasAction->setVisible( true );

    mCondensedToolButton->setToolTip( tr( "Set to current map canvas extent" ) );
    mCondensedToolButton->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "/mActionMapIdentification.svg" ) ) );
    connect( mCondensedToolButton, &QAbstractButton::clicked, this, &QgsExtentWidget::setOutputExtentFromCurrent );
    mCondensedToolButton->setPopupMode( QToolButton::MenuButtonPopup );
  }
  else
  {
    mButtonDrawOnCanvas->setVisible( false );
    mCurrentExtentButton->setVisible( false );
    mUseCanvasExtentAction->setVisible( false );
    mUseCanvasExtentAction->setVisible( false );

    mCondensedToolButton->setToolTip( QString() );
    mCondensedToolButton->setIcon( QIcon() );
    disconnect( mCondensedToolButton, &QAbstractButton::clicked, this, &QgsExtentWidget::setOutputExtentFromCurrent );
    mCondensedToolButton->setPopupMode( QToolButton::InstantPopup );
  }
}

void QgsExtentWidget::dragEnterEvent( QDragEnterEvent *event )
{
  if ( !( event->possibleActions() & Qt::CopyAction ) )
  {
    event->ignore();
    return;
  }

  if ( mapLayerFromMimeData( event->mimeData() ) )
  {
    // dragged an acceptable layer, phew
    event->setDropAction( Qt::CopyAction );
    event->accept();
    mCondensedLineEdit->setHighlighted( true );
    update();
  }
  else
  {
    event->ignore();
  }
}

void QgsExtentWidget::dragLeaveEvent( QDragLeaveEvent *event )
{
  if ( mCondensedLineEdit->isHighlighted() )
  {
    event->accept();
    mCondensedLineEdit->setHighlighted( false );
    update();
  }
  else
  {
    event->ignore();
  }
}

void QgsExtentWidget::dropEvent( QDropEvent *event )
{
  if ( !( event->possibleActions() & Qt::CopyAction ) )
  {
    event->ignore();
    return;
  }

  if ( QgsMapLayer *layer = mapLayerFromMimeData( event->mimeData() ) )
  {
    // dropped a map layer
    setFocus( Qt::MouseFocusReason );
    event->setDropAction( Qt::CopyAction );
    event->accept();

    setOutputExtentFromLayer( layer );
  }
  else
  {
    event->ignore();
  }
  mCondensedLineEdit->setHighlighted( false );
  update();
}

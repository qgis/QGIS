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

#include "qgslogger.h"
#include "qgscoordinatetransform.h"
#include "qgsmapcanvas.h"
#include "qgsmaplayermodel.h"
#include "qgsexception.h"
#include "qgsproject.h"
#include "qgsdoublevalidator.h"

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
  mMapLayerModel = new QgsMapLayerModel( this );

  mMenu = new QMenu( this );
  mUseCanvasExtentAction = new QAction( tr( "Use Map Canvas Extent" ), this );
  connect( mUseCanvasExtentAction, &QAction::triggered, this, &QgsExtentWidget::setOutputExtentFromCurrent );

  mUseCurrentExtentAction = new QAction( tr( "Use Current Layer Extent" ), this );
  connect( mUseCurrentExtentAction, &QAction::triggered, this, &QgsExtentWidget::setOutputExtentFromCurrent );

  mDrawOnCanvasAction = new QAction( tr( "Draw on Canvas" ), this );
  connect( mDrawOnCanvasAction, &QAction::triggered, this, &QgsExtentWidget::setOutputExtentFromDrawOnCanvas );

  mMenu->addMenu( mLayerMenu );

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
  mMenu->addAction( mUseCurrentExtentAction );
}

void QgsExtentWidget::setOutputCrs( const QgsCoordinateReferenceSystem &outputCrs )
{
  mHasFixedOutputCrs = true;
  if ( mOutputCrs != outputCrs )
  {
    bool prevExtentEnabled = mIsValid;
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
          QgsRectangle extent = ct.transformBoundingBox( outputExtent() );
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
  bool prevWasNull = mIsValid;

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
    QModelIndex index = mMapLayerModel->index( i, 0 );
    QIcon icon = qvariant_cast<QIcon>( mMapLayerModel->data( index, Qt::DecorationRole ) );
    QString text = mMapLayerModel->data( index, Qt::DisplayRole ).toString();
    QAction *act = new QAction( icon, text, mLayerMenu );
    act->setToolTip( mMapLayerModel->data( index, Qt::ToolTipRole ).toString() );
    QString layerId = mMapLayerModel->data( index, QgsMapLayerModel::LayerIdRole ).toString();
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
      connect( mMapToolExtent.get(), &QgsMapTool::deactivated, this, [ = ]
      {
        emit toggleDialogVisibility( true );
        mMapToolPrevious = nullptr;
      } );
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

    mMenu->addAction( mUseCanvasExtentAction );
    if ( drawOnCanvasOption )
      mMenu->addAction( mDrawOnCanvasAction );
  }
  else
  {
    mButtonDrawOnCanvas->setVisible( false );
    mCurrentExtentButton->setVisible( false );
    mMenu->removeAction( mUseCanvasExtentAction );
    mMenu->removeAction( mDrawOnCanvasAction );
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

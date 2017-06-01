/***************************************************************************
    qgsextentgroupbox.cpp
    ---------------------
    begin                : March 2014
    copyright            : (C) 2014 by Martin Dobias
    email                : wonder dot sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "qgsextentgroupbox.h"

#include "qgscoordinatetransform.h"
#include "qgsrasterblock.h"
#include "qgsmaplayermodel.h"
#include "qgscsexception.h"
#include "qgsproject.h"
#include <QMenu>
#include <QAction>

QgsExtentGroupBox::QgsExtentGroupBox( QWidget *parent )
  : QgsCollapsibleGroupBox( parent )
  , mTitleBase( tr( "Extent" ) )
  , mExtentState( OriginalExtent )
{
  setupUi( this );

  mLayerMenu = new QMenu( this );
  mButtonCalcFromLayer->setMenu( mLayerMenu );
  connect( mLayerMenu, &QMenu::aboutToShow, this, &QgsExtentGroupBox::layerMenuAboutToShow );
  mMapLayerModel = new QgsMapLayerModel( this );

  mXMinLineEdit->setValidator( new QDoubleValidator( this ) );
  mXMaxLineEdit->setValidator( new QDoubleValidator( this ) );
  mYMinLineEdit->setValidator( new QDoubleValidator( this ) );
  mYMaxLineEdit->setValidator( new QDoubleValidator( this ) );

  mOriginalExtentButton->setVisible( false );

  connect( mCurrentExtentButton, &QAbstractButton::clicked, this, &QgsExtentGroupBox::setOutputExtentFromCurrent );
  connect( mOriginalExtentButton, &QAbstractButton::clicked, this, &QgsExtentGroupBox::setOutputExtentFromOriginal );
  connect( this, &QGroupBox::clicked, this, &QgsExtentGroupBox::groupBoxClicked );
}


void QgsExtentGroupBox::setOriginalExtent( const QgsRectangle &originalExtent, const QgsCoordinateReferenceSystem &originalCrs )
{
  mOriginalExtent = originalExtent;
  mOriginalCrs = originalCrs;

  mOriginalExtentButton->setVisible( true );
}


void QgsExtentGroupBox::setCurrentExtent( const QgsRectangle &currentExtent, const QgsCoordinateReferenceSystem &currentCrs )
{
  mCurrentExtent = currentExtent;
  mCurrentCrs = currentCrs;
}

void QgsExtentGroupBox::setOutputCrs( const QgsCoordinateReferenceSystem &outputCrs )
{
  if ( mOutputCrs != outputCrs )
  {
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

      case UserExtent:
        try
        {
          QgsCoordinateTransform ct( mOutputCrs, outputCrs );
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

  }

}

void QgsExtentGroupBox::setOutputExtent( const QgsRectangle &r, const QgsCoordinateReferenceSystem &srcCrs, ExtentState state )
{
  QgsRectangle extent;
  if ( mOutputCrs == srcCrs )
  {
    extent = r;
  }
  else
  {
    try
    {
      QgsCoordinateTransform ct( srcCrs, mOutputCrs );
      extent = ct.transformBoundingBox( r );
    }
    catch ( QgsCsException & )
    {
      // can't reproject
      extent = r;
    }
  }

  mXMinLineEdit->setText( QgsRasterBlock::printValue( extent.xMinimum() ) );
  mXMaxLineEdit->setText( QgsRasterBlock::printValue( extent.xMaximum() ) );
  mYMinLineEdit->setText( QgsRasterBlock::printValue( extent.yMinimum() ) );
  mYMaxLineEdit->setText( QgsRasterBlock::printValue( extent.yMaximum() ) );

  mExtentState = state;

  if ( isCheckable() && !isChecked() )
    setChecked( true );

  updateTitle();

  emit extentChanged( extent );
}


void QgsExtentGroupBox::setOutputExtentFromLineEdit()
{
  mExtentState = UserExtent;

  updateTitle();

  emit extentChanged( outputExtent() );
}


void QgsExtentGroupBox::updateTitle()
{
  QString msg;
  switch ( mExtentState )
  {
    case OriginalExtent:
      msg = tr( "layer" );
      break;
    case CurrentExtent:
      msg = tr( "map view" );
      break;
    case UserExtent:
      msg = tr( "user defined" );
      break;
    case ProjectLayerExtent:
      msg = mExtentLayerName;
      break;
  }
  if ( isCheckable() && !isChecked() )
    msg = tr( "none" );
  msg = tr( "%1 (current: %2)" ).arg( mTitleBase, msg );

  setTitle( msg );
}

void QgsExtentGroupBox::layerMenuAboutToShow()
{
  qDeleteAll( mMenuActions );
  mMenuActions.clear();
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
    mMenuActions << act;
  }
}

void QgsExtentGroupBox::setExtentToLayerExtent( const QString &layerId )
{
  QgsMapLayer *layer = QgsProject::instance()->mapLayer( layerId );
  if ( !layer )
    return;

  setOutputExtentFromLayer( layer );
}

void QgsExtentGroupBox::setOutputExtentFromCurrent()
{
  setOutputExtent( mCurrentExtent, mCurrentCrs, CurrentExtent );
}


void QgsExtentGroupBox::setOutputExtentFromOriginal()
{
  setOutputExtent( mOriginalExtent, mOriginalCrs, OriginalExtent );
}

void QgsExtentGroupBox::setOutputExtentFromUser( const QgsRectangle &extent, const QgsCoordinateReferenceSystem &crs )
{
  setOutputExtent( extent, crs, UserExtent );
}

void QgsExtentGroupBox::setOutputExtentFromLayer( const QgsMapLayer *layer )
{
  if ( !layer )
    return;

  mExtentLayer = layer;
  mExtentLayerName = layer->name();

  setOutputExtent( layer->extent(), layer->crs(), ProjectLayerExtent );
}

void QgsExtentGroupBox::groupBoxClicked()
{
  if ( !isCheckable() )
    return;

  updateTitle();

  // output extent just went from null to something (or vice versa)
  emit extentChanged( outputExtent() );
}


QgsRectangle QgsExtentGroupBox::outputExtent() const
{
  if ( isCheckable() && !isChecked() )
    return QgsRectangle();

  return QgsRectangle( mXMinLineEdit->text().toDouble(), mYMinLineEdit->text().toDouble(),
                       mXMaxLineEdit->text().toDouble(), mYMaxLineEdit->text().toDouble() );
}

void QgsExtentGroupBox::setTitleBase( const QString &title )
{
  mTitleBase = title;
  updateTitle();
}

QString QgsExtentGroupBox::titleBase() const
{
  return mTitleBase;
}

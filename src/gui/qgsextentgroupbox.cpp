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
#include "qgsextentwidget.h"

QgsExtentGroupBox::QgsExtentGroupBox( QWidget *parent )
  : QgsCollapsibleGroupBox( parent )
  , mTitleBase( tr( "Extent" ) )
{
  mWidget = new QgsExtentWidget( nullptr, QgsExtentWidget::ExpandedStyle );
  QVBoxLayout *layout = new QVBoxLayout();
  layout->addWidget( mWidget );
  setLayout( layout );

  connect( this, &QGroupBox::toggled, this, &QgsExtentGroupBox::groupBoxClicked );
  connect( mWidget, &QgsExtentWidget::extentChanged, this, &QgsExtentGroupBox::widgetExtentChanged );
  connect( mWidget, &QgsExtentWidget::validationChanged, this, &QgsExtentGroupBox::validationChanged );

  connect( mWidget, &QgsExtentWidget::toggleDialogVisibility, this, [ = ]( bool visible )
  {
    QWidget *w = window();
    // Don't hide the main window or we'll get locked outside!
    if ( w->objectName() == QLatin1String( "QgisApp" ) )
      return;
    w->setVisible( visible );
  } );
}

void QgsExtentGroupBox::setOriginalExtent( const QgsRectangle &originalExtent, const QgsCoordinateReferenceSystem &originalCrs )
{
  mWidget->setOriginalExtent( originalExtent, originalCrs );
}

QgsRectangle QgsExtentGroupBox::originalExtent() const
{
  return mWidget->originalExtent();
}

QgsCoordinateReferenceSystem QgsExtentGroupBox::originalCrs() const
{
  return mWidget->originalCrs();
}

void QgsExtentGroupBox::setCurrentExtent( const QgsRectangle &currentExtent, const QgsCoordinateReferenceSystem &currentCrs )
{
  mWidget->setCurrentExtent( currentExtent, currentCrs );
}

QgsRectangle QgsExtentGroupBox::currentExtent() const
{
  return mWidget->currentExtent();
}

QgsCoordinateReferenceSystem QgsExtentGroupBox::currentCrs() const
{
  return mWidget->currentCrs();
}

void QgsExtentGroupBox::setOutputCrs( const QgsCoordinateReferenceSystem &outputCrs )
{
  mWidget->setOutputCrs( outputCrs );
}

void QgsExtentGroupBox::updateTitle()
{
  QString msg;
  switch ( mWidget->extentState() )
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
      msg = mWidget->extentLayerName();
      break;
    case QgsExtentWidget::DrawOnCanvas:
      msg = tr( "drawn on canvas" );
      break;
  }
  if ( isCheckable() && !isChecked() )
    msg = tr( "none" );
  msg = tr( "%1 (current: %2)" ).arg( mTitleBase, msg );

  setTitle( msg );
}

void QgsExtentGroupBox::setOutputExtentFromCurrent()
{
  mWidget->setOutputExtentFromCurrent();
}

void QgsExtentGroupBox::setOutputExtentFromOriginal()
{
  mWidget->setOutputExtentFromOriginal();
}

void QgsExtentGroupBox::setOutputExtentFromUser( const QgsRectangle &extent, const QgsCoordinateReferenceSystem &crs )
{
  mWidget->setOutputExtentFromUser( extent, crs );
}

void QgsExtentGroupBox::setOutputExtentFromLayer( const QgsMapLayer *layer )
{
  mWidget->setOutputExtentFromLayer( layer );
}

void QgsExtentGroupBox::setOutputExtentFromDrawOnCanvas()
{
  mWidget->setOutputExtentFromDrawOnCanvas();
}

void QgsExtentGroupBox::setRatio( QSize ratio )
{
  mWidget->setRatio( ratio );
}

void QgsExtentGroupBox::groupBoxClicked()
{
  if ( !isCheckable() )
    return;

  updateTitle();

  // output extent just went from null to something (or vice versa)
  emit extentChanged( outputExtent() );
}

void QgsExtentGroupBox::widgetExtentChanged()
{
  updateTitle();

  emit extentChanged( outputExtent() );
}

void QgsExtentGroupBox::validationChanged( bool valid )
{
  if ( valid )
  {
    if ( isCheckable() && !isChecked() )
      setChecked( true );
  }
  else if ( isCheckable() && isChecked() )
    setChecked( false );
}

QgsRectangle QgsExtentGroupBox::outputExtent() const
{
  if ( isCheckable() && !isChecked() )
    return QgsRectangle();

  return mWidget->outputExtent();
}

QgsCoordinateReferenceSystem QgsExtentGroupBox::outputCrs() const
{
  return mWidget->outputCrs();
}

QgsExtentGroupBox::ExtentState QgsExtentGroupBox::extentState() const
{
  return static_cast< QgsExtentGroupBox::ExtentState >( mWidget->extentState() );
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

void QgsExtentGroupBox::setMapCanvas( QgsMapCanvas *canvas, bool drawOnCanvasOption )
{
  mWidget->setMapCanvas( canvas, drawOnCanvasOption );
}

QSize QgsExtentGroupBox::ratio() const
{
  return mWidget->ratio();
}

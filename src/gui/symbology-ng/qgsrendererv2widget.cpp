/***************************************************************************
    qgsrendererv2widget.cpp
    ---------------------
    begin                : November 2009
    copyright            : (C) 2009 by Martin Dobias
    email                : wonder dot sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "qgsrendererv2widget.h"
#include "qgssymbol.h"
#include "qgsvectorlayer.h"
#include "qgscolordialog.h"
#include "qgssymbollevelsdialog.h"
#include "qgsexpressionbuilderdialog.h"
#include "qgsmapcanvas.h"
#include "qgspanelwidget.h"
#include "qgsdatadefined.h"

#include <QMessageBox>
#include <QInputDialog>
#include <QMenu>

QgsRendererV2Widget::QgsRendererV2Widget( QgsVectorLayer* layer, QgsStyleV2* style )
    : QgsPanelWidget()
    , mLayer( layer )
    , mStyle( style )
    , mMapCanvas( nullptr )
{
  contextMenu = new QMenu( tr( "Renderer Options" ), this );

  mCopyAction = contextMenu->addAction( tr( "Copy" ), this, SLOT( copy() ) );
  mCopyAction->setShortcut( QKeySequence( QKeySequence::Copy ) );
  mPasteAction = contextMenu->addAction( tr( "Paste" ), this, SLOT( paste() ) );
  mPasteAction->setShortcut( QKeySequence( QKeySequence::Paste ) );

  contextMenu->addSeparator();
  contextMenu->addAction( tr( "Change color" ), this, SLOT( changeSymbolColor() ) );
  contextMenu->addAction( tr( "Change transparency" ), this, SLOT( changeSymbolTransparency() ) );
  contextMenu->addAction( tr( "Change output unit" ), this, SLOT( changeSymbolUnit() ) );

  if ( mLayer && mLayer->geometryType() == QgsWkbTypes::LineGeometry )
  {
    contextMenu->addAction( tr( "Change width" ), this, SLOT( changeSymbolWidth() ) );
  }
  else if ( mLayer && mLayer->geometryType() == QgsWkbTypes::PointGeometry )
  {
    contextMenu->addAction( tr( "Change size" ), this, SLOT( changeSymbolSize() ) );
    contextMenu->addAction( tr( "Change angle" ), this, SLOT( changeSymbolAngle() ) );
  }
}

void QgsRendererV2Widget::contextMenuViewCategories( QPoint )
{
  contextMenu->exec( QCursor::pos() );
}

void QgsRendererV2Widget::changeSymbolColor()
{
  QList<QgsSymbol*> symbolList = selectedSymbols();
  if ( symbolList.isEmpty() )
  {
    return;
  }

  QgsSymbol* firstSymbol = nullptr;
  Q_FOREACH ( QgsSymbol* symbol, symbolList )
  {
    if ( symbol )
    {
      firstSymbol = symbol;
      break;
    }
  }
  if ( !firstSymbol )
    return;

  QColor color = QgsColorDialog::getColor( firstSymbol->color(), this, "Change Symbol Color", true );
  if ( color.isValid() )
  {
    Q_FOREACH ( QgsSymbol* symbol, symbolList )
    {
      if ( symbol )
        symbol->setColor( color );
    }
    refreshSymbolView();
  }
}

void QgsRendererV2Widget::changeSymbolTransparency()
{
  QList<QgsSymbol*> symbolList = selectedSymbols();
  if ( symbolList.isEmpty() )
  {
    return;
  }

  QgsSymbol* firstSymbol = nullptr;
  Q_FOREACH ( QgsSymbol* symbol, symbolList )
  {
    if ( symbol )
    {
      firstSymbol = symbol;
      break;
    }
  }
  if ( !firstSymbol )
    return;

  bool ok;
  double oldTransparency = ( 1 - firstSymbol->alpha() ) * 100; // convert to percents
  double transparency = QInputDialog::getDouble( this, tr( "Transparency" ), tr( "Change symbol transparency [%]" ), oldTransparency, 0.0, 100.0, 0, &ok );
  if ( ok )
  {
    Q_FOREACH ( QgsSymbol* symbol, symbolList )
    {
      if ( symbol )
        symbol->setAlpha( 1 - transparency / 100 );
    }
    refreshSymbolView();
  }
}

void QgsRendererV2Widget::changeSymbolUnit()
{
  QList<QgsSymbol*> symbolList = selectedSymbols();
  if ( symbolList.isEmpty() )
  {
    return;
  }

  QgsSymbol* firstSymbol = nullptr;
  Q_FOREACH ( QgsSymbol* symbol, symbolList )
  {
    if ( symbol )
    {
      firstSymbol = symbol;
      break;
    }
  }
  if ( !firstSymbol )
    return;

  bool ok;
  int currentUnit = ( firstSymbol->outputUnit() == QgsUnitTypes::RenderMillimeters ) ? 0 : 1;
  QString item = QInputDialog::getItem( this, tr( "Symbol unit" ), tr( "Select symbol unit" ), QStringList() << tr( "Millimeter" ) << tr( "Map unit" ), currentUnit, false, &ok );
  if ( ok )
  {
    QgsUnitTypes::RenderUnit unit = ( item.compare( tr( "Millimeter" ) ) == 0 ) ? QgsUnitTypes::RenderMillimeters : QgsUnitTypes::RenderMapUnits;

    Q_FOREACH ( QgsSymbol* symbol, symbolList )
    {
      if ( symbol )
        symbol->setOutputUnit( unit );
    }
    refreshSymbolView();
  }
}

void QgsRendererV2Widget::changeSymbolWidth()
{
  QList<QgsSymbol*> symbolList = selectedSymbols();
  if ( symbolList.isEmpty() )
  {
    return;
  }

  QgsDataDefinedWidthDialog dlg( symbolList, mLayer );
  dlg.setMapCanvas( mMapCanvas );

  if ( QDialog::Accepted == dlg.exec() )
  {
    if ( !dlg.mDDBtn->isActive() )
    {
      Q_FOREACH ( QgsSymbol* symbol, symbolList )
      {
        if ( !symbol )
          continue;

        if ( symbol->type() == QgsSymbol::Line )
          static_cast<QgsLineSymbolV2*>( symbol )->setWidth( dlg.mSpinBox->value() );
      }
    }
    refreshSymbolView();
  }
}

void QgsRendererV2Widget::changeSymbolSize()
{
  QList<QgsSymbol*> symbolList = selectedSymbols();
  if ( symbolList.isEmpty() )
  {
    return;
  }

  QgsDataDefinedSizeDialog dlg( symbolList, mLayer );
  dlg.setMapCanvas( mMapCanvas );

  if ( QDialog::Accepted == dlg.exec() )
  {
    if ( !dlg.mDDBtn->isActive() )
    {
      Q_FOREACH ( QgsSymbol* symbol, symbolList )
      {
        if ( !symbol )
          continue;

        if ( symbol->type() == QgsSymbol::Marker )
          static_cast<QgsMarkerSymbolV2*>( symbol )->setSize( dlg.mSpinBox->value() );
      }
    }
    refreshSymbolView();
  }
}

void QgsRendererV2Widget::changeSymbolAngle()
{
  QList<QgsSymbol*> symbolList = selectedSymbols();
  if ( symbolList.isEmpty() )
  {
    return;
  }

  QgsDataDefinedRotationDialog dlg( symbolList, mLayer );
  dlg.setMapCanvas( mMapCanvas );

  if ( QDialog::Accepted == dlg.exec() )
  {
    if ( !dlg.mDDBtn->isActive() )
    {
      Q_FOREACH ( QgsSymbol* symbol, symbolList )
      {
        if ( !symbol )
          continue;

        if ( symbol->type() == QgsSymbol::Marker )
          static_cast<QgsMarkerSymbolV2*>( symbol )->setAngle( dlg.mSpinBox->value() );
      }
    }
    refreshSymbolView();
  }
}

void QgsRendererV2Widget::showSymbolLevelsDialog( QgsFeatureRendererV2* r )
{
  QgsLegendSymbolList symbols = r->legendSymbolItems();

  QgsSymbolLevelsDialog dlg( symbols, r->usingSymbolLevels(), this );

  if ( dlg.exec() )
  {
    r->setUsingSymbolLevels( dlg.usingLevels() );
    emit widgetChanged();
  }
}

void QgsRendererV2Widget::setMapCanvas( QgsMapCanvas *canvas )
{
  mMapCanvas = canvas;
}

const QgsMapCanvas* QgsRendererV2Widget::mapCanvas() const
{
  return mMapCanvas;
}

void QgsRendererV2Widget::applyChanges()
{
  apply();
}

//
// QgsDataDefinedValueDialog
//

QgsDataDefinedValueDialog::QgsDataDefinedValueDialog( const QList<QgsSymbol*>& symbolList, QgsVectorLayer * layer, const QString & label )
    : mSymbolList( symbolList )
    , mLayer( layer )
    , mMapCanvas( nullptr )
{
  setupUi( this );
  setWindowFlags( Qt::WindowStaysOnTopHint );
  mLabel->setText( label );
  connect( mDDBtn, SIGNAL( dataDefinedChanged( const QString& ) ), this, SLOT( dataDefinedChanged() ) );
  connect( mDDBtn, SIGNAL( dataDefinedActivated( bool ) ), this, SLOT( dataDefinedChanged() ) );

}

void QgsDataDefinedValueDialog::setMapCanvas( QgsMapCanvas *canvas )
{
  mMapCanvas = canvas;
  Q_FOREACH ( QgsDataDefinedButton* ddButton, findChildren<QgsDataDefinedButton*>() )
  {
    if ( ddButton->assistant() )
      ddButton->assistant()->setMapCanvas( mMapCanvas );
  }
}

const QgsMapCanvas *QgsDataDefinedValueDialog::mapCanvas() const
{
  return mMapCanvas;
}

static QgsExpressionContext _getExpressionContext( const void* context )
{
  const QgsDataDefinedValueDialog* widget = ( const QgsDataDefinedValueDialog* ) context;

  QgsExpressionContext expContext;
  expContext << QgsExpressionContextUtils::globalScope()
  << QgsExpressionContextUtils::projectScope()
  << QgsExpressionContextUtils::atlasScope( nullptr );
  if ( widget->mapCanvas() )
  {
    expContext << QgsExpressionContextUtils::mapSettingsScope( widget->mapCanvas()->mapSettings() )
    << new QgsExpressionContextScope( widget->mapCanvas()->expressionContextScope() );
  }
  else
  {
    expContext << QgsExpressionContextUtils::mapSettingsScope( QgsMapSettings() );
  }

  if ( widget->vectorLayer() )
    expContext << QgsExpressionContextUtils::layerScope( widget->vectorLayer() );

  return expContext;
}

void QgsDataDefinedValueDialog::init( const QString& description )
{
  QgsDataDefined dd = symbolDataDefined();
  mDDBtn->init( mLayer, &dd, QgsDataDefinedButton::Double, description );
  mDDBtn->registerGetExpressionContextCallback( &_getExpressionContext, this );

  QgsSymbol* initialSymbol = nullptr;
  Q_FOREACH ( QgsSymbol* symbol, mSymbolList )
  {
    if ( symbol )
    {
      initialSymbol = symbol;
    }
  }
  mSpinBox->setValue( initialSymbol ? value( initialSymbol ) : 0 );
  mSpinBox->setEnabled( !mDDBtn->isActive() );
}

QgsDataDefined QgsDataDefinedValueDialog::symbolDataDefined() const
{
  if ( mSymbolList.isEmpty() || !mSymbolList.back() )
    return QgsDataDefined();

  // check that all symbols share the same size expression
  QgsDataDefined dd = symbolDataDefined( mSymbolList.back() );
  Q_FOREACH ( QgsSymbol * it, mSymbolList )
  {
    if ( !it || symbolDataDefined( it ) != dd )
      return QgsDataDefined();
  }
  return dd;
}

void QgsDataDefinedValueDialog::dataDefinedChanged()
{
  QgsDataDefined dd = mDDBtn->currentDataDefined();
  mSpinBox->setEnabled( !dd.isActive() );

  if ( // shall we remove datadefined expressions for layers ?
    ( symbolDataDefined().isActive() && !dd.isActive() )
    // shall we set the "en masse" expression for properties ?
    || dd.isActive() )
  {
    Q_FOREACH ( QgsSymbol * it, mSymbolList )
      setDataDefined( it, dd );
  }
}

QgsDataDefined QgsDataDefinedSizeDialog::symbolDataDefined( const QgsSymbol *symbol ) const
{
  const QgsMarkerSymbolV2* marker = static_cast<const QgsMarkerSymbolV2*>( symbol );
  return marker->dataDefinedSize();
}

void QgsDataDefinedSizeDialog::setDataDefined( QgsSymbol* symbol, const QgsDataDefined& dd )
{
  static_cast<QgsMarkerSymbolV2*>( symbol )->setDataDefinedSize( dd );
  static_cast<QgsMarkerSymbolV2*>( symbol )->setScaleMethod( QgsSymbol::ScaleDiameter );
}


QgsDataDefined QgsDataDefinedRotationDialog::symbolDataDefined( const QgsSymbol *symbol ) const
{
  const QgsMarkerSymbolV2* marker = static_cast<const QgsMarkerSymbolV2*>( symbol );
  return marker->dataDefinedAngle();
}

void QgsDataDefinedRotationDialog::setDataDefined( QgsSymbol *symbol, const QgsDataDefined &dd )
{
  static_cast<QgsMarkerSymbolV2*>( symbol )->setDataDefinedAngle( dd );
}


QgsDataDefined QgsDataDefinedWidthDialog::symbolDataDefined( const QgsSymbol *symbol ) const
{
  const QgsLineSymbolV2* line = static_cast<const QgsLineSymbolV2*>( symbol );
  return line->dataDefinedWidth();
}

void QgsDataDefinedWidthDialog::setDataDefined( QgsSymbol *symbol, const QgsDataDefined &dd )
{
  static_cast<QgsLineSymbolV2*>( symbol )->setDataDefinedWidth( dd );
}

void QgsRendererV2Widget::apply()
{

}

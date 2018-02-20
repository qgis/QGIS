/***************************************************************************
    qgsrendererwidget.cpp
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
#include "qgsrendererwidget.h"

#include "qgsdatadefinedsizelegendwidget.h"
#include "qgssymbol.h"
#include "qgsvectorlayer.h"
#include "qgscolordialog.h"
#include "qgssymbollevelsdialog.h"
#include "qgssymbollayer.h"
#include "qgsexpressionbuilderdialog.h"
#include "qgsmapcanvas.h"
#include "qgspanelwidget.h"
#include "qgsproject.h"

#include <QMessageBox>
#include <QInputDialog>
#include <QMenu>

QgsRendererWidget::QgsRendererWidget( QgsVectorLayer *layer, QgsStyle *style )
  : mLayer( layer )
  , mStyle( style )
{
  contextMenu = new QMenu( tr( "Renderer Options" ), this );

  mCopyAction = contextMenu->addAction( tr( "Copy" ), this, SLOT( copy() ) );
  mCopyAction->setShortcut( QKeySequence( QKeySequence::Copy ) );
  mPasteAction = contextMenu->addAction( tr( "Paste" ), this, SLOT( paste() ) );
  mPasteAction->setShortcut( QKeySequence( QKeySequence::Paste ) );

  contextMenu->addSeparator();
  contextMenu->addAction( tr( "Change color" ), this, SLOT( changeSymbolColor() ) );
  contextMenu->addAction( tr( "Change opacity" ), this, SLOT( changeSymbolOpacity() ) );
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

void QgsRendererWidget::contextMenuViewCategories( QPoint )
{
  contextMenu->exec( QCursor::pos() );
}

void QgsRendererWidget::changeSymbolColor()
{
  QList<QgsSymbol *> symbolList = selectedSymbols();
  if ( symbolList.isEmpty() )
  {
    return;
  }

  QgsSymbol *firstSymbol = nullptr;
  Q_FOREACH ( QgsSymbol *symbol, symbolList )
  {
    if ( symbol )
    {
      firstSymbol = symbol;
      break;
    }
  }
  if ( !firstSymbol )
    return;

  QColor color = QgsColorDialog::getColor( firstSymbol->color(), this, QStringLiteral( "Change Symbol Color" ), true );
  if ( color.isValid() )
  {
    Q_FOREACH ( QgsSymbol *symbol, symbolList )
    {
      if ( symbol )
        symbol->setColor( color );
    }
    refreshSymbolView();
  }
}

void QgsRendererWidget::changeSymbolOpacity()
{
  QList<QgsSymbol *> symbolList = selectedSymbols();
  if ( symbolList.isEmpty() )
  {
    return;
  }

  QgsSymbol *firstSymbol = nullptr;
  Q_FOREACH ( QgsSymbol *symbol, symbolList )
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
  double oldOpacity = firstSymbol->opacity() * 100; // convert to %
  double opacity = QInputDialog::getDouble( this, tr( "Opacity" ), tr( "Change symbol opacity [%]" ), oldOpacity, 0.0, 100.0, 1, &ok );
  if ( ok )
  {
    Q_FOREACH ( QgsSymbol *symbol, symbolList )
    {
      if ( symbol )
        symbol->setOpacity( opacity / 100.0 );
    }
    refreshSymbolView();
  }
}

void QgsRendererWidget::changeSymbolUnit()
{
  QList<QgsSymbol *> symbolList = selectedSymbols();
  if ( symbolList.isEmpty() )
  {
    return;
  }

  QgsSymbol *firstSymbol = nullptr;
  Q_FOREACH ( QgsSymbol *symbol, symbolList )
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

    Q_FOREACH ( QgsSymbol *symbol, symbolList )
    {
      if ( symbol )
        symbol->setOutputUnit( unit );
    }
    refreshSymbolView();
  }
}

void QgsRendererWidget::changeSymbolWidth()
{
  QList<QgsSymbol *> symbolList = selectedSymbols();
  if ( symbolList.isEmpty() )
  {
    return;
  }

  QgsDataDefinedWidthDialog dlg( symbolList, mLayer );

  dlg.setContext( mContext );

  if ( QDialog::Accepted == dlg.exec() )
  {
    if ( !dlg.mDDBtn->isActive() )
    {
      Q_FOREACH ( QgsSymbol *symbol, symbolList )
      {
        if ( !symbol )
          continue;

        if ( symbol->type() == QgsSymbol::Line )
          static_cast<QgsLineSymbol *>( symbol )->setWidth( dlg.mSpinBox->value() );
      }
    }
    refreshSymbolView();
  }
}

void QgsRendererWidget::changeSymbolSize()
{
  QList<QgsSymbol *> symbolList = selectedSymbols();
  if ( symbolList.isEmpty() )
  {
    return;
  }

  QgsDataDefinedSizeDialog dlg( symbolList, mLayer );
  dlg.setContext( mContext );

  if ( QDialog::Accepted == dlg.exec() )
  {
    if ( !dlg.mDDBtn->isActive() )
    {
      Q_FOREACH ( QgsSymbol *symbol, symbolList )
      {
        if ( !symbol )
          continue;

        if ( symbol->type() == QgsSymbol::Marker )
          static_cast<QgsMarkerSymbol *>( symbol )->setSize( dlg.mSpinBox->value() );
      }
    }
    refreshSymbolView();
  }
}

void QgsRendererWidget::changeSymbolAngle()
{
  QList<QgsSymbol *> symbolList = selectedSymbols();
  if ( symbolList.isEmpty() )
  {
    return;
  }

  QgsDataDefinedRotationDialog dlg( symbolList, mLayer );
  dlg.setContext( mContext );

  if ( QDialog::Accepted == dlg.exec() )
  {
    if ( !dlg.mDDBtn->isActive() )
    {
      Q_FOREACH ( QgsSymbol *symbol, symbolList )
      {
        if ( !symbol )
          continue;

        if ( symbol->type() == QgsSymbol::Marker )
          static_cast<QgsMarkerSymbol *>( symbol )->setAngle( dlg.mSpinBox->value() );
      }
    }
    refreshSymbolView();
  }
}

void QgsRendererWidget::showSymbolLevelsDialog( QgsFeatureRenderer *r )
{
  QgsPanelWidget *panel = QgsPanelWidget::findParentPanel( this );
  if ( panel && panel->dockMode() )
  {
    QgsSymbolLevelsWidget *widget = new QgsSymbolLevelsWidget( r, r->usingSymbolLevels(), panel );
    widget->setPanelTitle( tr( "Symbol Levels" ) );
    connect( widget, &QgsPanelWidget::widgetChanged, widget, &QgsSymbolLevelsWidget::apply );
    connect( widget, &QgsPanelWidget::widgetChanged, this, &QgsPanelWidget::widgetChanged );
    panel->openPanel( widget );
    return;
  }

  QgsSymbolLevelsDialog dlg( r, r->usingSymbolLevels(), panel );
  if ( dlg.exec() )
  {
    emit widgetChanged();
  }
}

void QgsRendererWidget::setContext( const QgsSymbolWidgetContext &context )
{
  mContext = context;
}

QgsSymbolWidgetContext QgsRendererWidget::context() const
{
  return mContext;
}

void QgsRendererWidget::applyChanges()
{
  apply();
}

QgsDataDefinedSizeLegendWidget *QgsRendererWidget::createDataDefinedSizeLegendWidget( const QgsMarkerSymbol *symbol, const QgsDataDefinedSizeLegend *ddsLegend )
{
  QgsProperty ddSize = symbol->dataDefinedSize();
  if ( !ddSize || !ddSize.isActive() )
  {
    QMessageBox::warning( this, tr( "Data-defined size legend" ), tr( "Data-defined size is not enabled!" ) );
    return nullptr;
  }

  QgsDataDefinedSizeLegendWidget *panel = new QgsDataDefinedSizeLegendWidget( ddsLegend, ddSize, symbol->clone(), mContext.mapCanvas() );
  connect( panel, &QgsPanelWidget::widgetChanged, this, &QgsPanelWidget::widgetChanged );
  return panel;
}


//
// QgsDataDefinedValueDialog
//

QgsDataDefinedValueDialog::QgsDataDefinedValueDialog( const QList<QgsSymbol *> &symbolList, QgsVectorLayer *layer, const QString &label )
  : mSymbolList( symbolList )
  , mLayer( layer )
{
  setupUi( this );
  setWindowFlags( Qt::WindowStaysOnTopHint );
  mLabel->setText( label );
  connect( mDDBtn, &QgsPropertyOverrideButton::changed, this, &QgsDataDefinedValueDialog::dataDefinedChanged );
}

void QgsDataDefinedValueDialog::setContext( const QgsSymbolWidgetContext &context )
{
  mContext = context;
}

QgsSymbolWidgetContext QgsDataDefinedValueDialog::context() const
{
  return mContext;
}

QgsExpressionContext QgsDataDefinedValueDialog::createExpressionContext() const
{
  QgsExpressionContext expContext;
  expContext << QgsExpressionContextUtils::globalScope()
             << QgsExpressionContextUtils::projectScope( QgsProject::instance() )
             << QgsExpressionContextUtils::atlasScope( nullptr );
  if ( mContext.mapCanvas() )
  {
    expContext << QgsExpressionContextUtils::mapSettingsScope( mContext.mapCanvas()->mapSettings() )
               << new QgsExpressionContextScope( mContext.mapCanvas()->expressionContextScope() );
  }
  else
  {
    expContext << QgsExpressionContextUtils::mapSettingsScope( QgsMapSettings() );
  }

  if ( vectorLayer() )
    expContext << QgsExpressionContextUtils::layerScope( vectorLayer() );

  // additional scopes
  Q_FOREACH ( const QgsExpressionContextScope &scope, mContext.additionalExpressionContextScopes() )
  {
    expContext.appendScope( new QgsExpressionContextScope( scope ) );
  }

  return expContext;
}

void QgsDataDefinedValueDialog::init( int propertyKey )
{
  QgsProperty dd( symbolDataDefined() );

  mDDBtn->init( propertyKey, dd, QgsSymbolLayer::propertyDefinitions(), mLayer );
  mDDBtn->registerExpressionContextGenerator( this );

  QgsSymbol *initialSymbol = nullptr;
  Q_FOREACH ( QgsSymbol *symbol, mSymbolList )
  {
    if ( symbol )
    {
      initialSymbol = symbol;
    }
  }
  mSpinBox->setValue( initialSymbol ? value( initialSymbol ) : 0 );
  mSpinBox->setEnabled( !mDDBtn->isActive() );
}

QgsProperty QgsDataDefinedValueDialog::symbolDataDefined() const
{
  if ( mSymbolList.isEmpty() || !mSymbolList.back() )
    return QgsProperty();

  // check that all symbols share the same size expression
  QgsProperty dd = symbolDataDefined( mSymbolList.back() );
  Q_FOREACH ( QgsSymbol *it, mSymbolList )
  {
    QgsProperty symbolDD( symbolDataDefined( it ) );
    if ( !it || !dd || !symbolDD || symbolDD != dd )
      return QgsProperty();
  }
  return dd;
}

void QgsDataDefinedValueDialog::dataDefinedChanged()
{
  QgsProperty dd( mDDBtn->toProperty() );
  mSpinBox->setEnabled( !dd.isActive() );

  QgsProperty symbolDD( symbolDataDefined() );

  if ( // shall we remove datadefined expressions for layers ?
    ( symbolDD && symbolDD.isActive() && !dd.isActive() )
    // shall we set the "en masse" expression for properties ?
    || dd.isActive() )
  {
    Q_FOREACH ( QgsSymbol *it, mSymbolList )
      setDataDefined( it, dd );
  }
}

QgsProperty QgsDataDefinedSizeDialog::symbolDataDefined( const QgsSymbol *symbol ) const
{
  const QgsMarkerSymbol *marker = static_cast<const QgsMarkerSymbol *>( symbol );
  return marker->dataDefinedSize();
}

void QgsDataDefinedSizeDialog::setDataDefined( QgsSymbol *symbol, const QgsProperty &dd )
{
  static_cast<QgsMarkerSymbol *>( symbol )->setDataDefinedSize( dd );
  static_cast<QgsMarkerSymbol *>( symbol )->setScaleMethod( QgsSymbol::ScaleDiameter );
}


QgsProperty QgsDataDefinedRotationDialog::symbolDataDefined( const QgsSymbol *symbol ) const
{
  const QgsMarkerSymbol *marker = static_cast<const QgsMarkerSymbol *>( symbol );
  return marker->dataDefinedAngle();
}

void QgsDataDefinedRotationDialog::setDataDefined( QgsSymbol *symbol, const QgsProperty &dd )
{
  static_cast<QgsMarkerSymbol *>( symbol )->setDataDefinedAngle( dd );
}


QgsProperty QgsDataDefinedWidthDialog::symbolDataDefined( const QgsSymbol *symbol ) const
{
  const QgsLineSymbol *line = static_cast<const QgsLineSymbol *>( symbol );
  return line->dataDefinedWidth();
}

void QgsDataDefinedWidthDialog::setDataDefined( QgsSymbol *symbol, const QgsProperty &dd )
{
  static_cast<QgsLineSymbol *>( symbol )->setDataDefinedWidth( dd );
}

void QgsRendererWidget::apply()
{

}

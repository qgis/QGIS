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
#include "qgsexpressioncontextutils.h"
#include "qgssymbollayerutils.h"
#include "qgstemporalcontroller.h"
#include "qgsmarkersymbol.h"
#include "qgslinesymbol.h"

#include <QMessageBox>
#include <QInputDialog>
#include <QMenu>
#include <QClipboard>

QgsRendererWidget::QgsRendererWidget( QgsVectorLayer *layer, QgsStyle *style )
  : mLayer( layer )
  , mStyle( style )
{
  contextMenu = new QMenu( tr( "Renderer Options" ), this );

  mCopyAction = new QAction( tr( "Copy" ), this );
  connect( mCopyAction, &QAction::triggered, this, &QgsRendererWidget::copy );
  mCopyAction->setShortcut( QKeySequence( QKeySequence::Copy ) );
  mPasteAction = new QAction( tr( "Paste" ), this );
  mPasteAction->setShortcut( QKeySequence( QKeySequence::Paste ) );
  connect( mPasteAction, &QAction::triggered, this, &QgsRendererWidget::paste );

  mCopySymbolAction = new QAction( tr( "Copy Symbol" ), this );
  contextMenu->addAction( mCopySymbolAction );
  connect( mCopySymbolAction, &QAction::triggered, this, &QgsRendererWidget::copySymbol );
  mPasteSymbolAction = new QAction( tr( "Paste Symbol" ), this );
  contextMenu->addAction( mPasteSymbolAction );
  connect( mPasteSymbolAction, &QAction::triggered, this, &QgsRendererWidget::pasteSymbolToSelection );

  contextMenu->addSeparator();
  contextMenu->addAction( tr( "Change Color…" ), this, SLOT( changeSymbolColor() ) );
  contextMenu->addAction( tr( "Change Opacity…" ), this, SLOT( changeSymbolOpacity() ) );
  contextMenu->addAction( tr( "Change Output Unit…" ), this, SLOT( changeSymbolUnit() ) );

  if ( mLayer && mLayer->geometryType() == QgsWkbTypes::LineGeometry )
  {
    contextMenu->addAction( tr( "Change Width…" ), this, SLOT( changeSymbolWidth() ) );
  }
  else if ( mLayer && mLayer->geometryType() == QgsWkbTypes::PointGeometry )
  {
    contextMenu->addAction( tr( "Change Size…" ), this, SLOT( changeSymbolSize() ) );
    contextMenu->addAction( tr( "Change Angle…" ), this, SLOT( changeSymbolAngle() ) );
  }

  connect( contextMenu, &QMenu::aboutToShow, this, [ = ]
  {
    const std::unique_ptr< QgsSymbol > tempSymbol( QgsSymbolLayerUtils::symbolFromMimeData( QApplication::clipboard()->mimeData() ) );
    mPasteSymbolAction->setEnabled( static_cast< bool >( tempSymbol ) );
  } );
}

void QgsRendererWidget::contextMenuViewCategories( QPoint )
{
  contextMenu->exec( QCursor::pos() );
}

void QgsRendererWidget::changeSymbolColor()
{
  const QList<QgsSymbol *> symbolList = selectedSymbols();
  if ( symbolList.isEmpty() )
  {
    return;
  }

  QgsSymbol *firstSymbol = nullptr;
  for ( QgsSymbol *symbol : symbolList )
  {
    if ( symbol )
    {
      firstSymbol = symbol;
      break;
    }
  }
  if ( !firstSymbol )
    return;

  const QColor currentColor = firstSymbol->color();

  QgsPanelWidget *panel = QgsPanelWidget::findParentPanel( qobject_cast< QWidget * >( parent() ) );
  if ( panel && panel->dockMode() )
  {
    QgsCompoundColorWidget *colorWidget = new QgsCompoundColorWidget( panel, currentColor, QgsCompoundColorWidget::LayoutVertical );
    colorWidget->setPanelTitle( tr( "Change Symbol Color" ) );
    colorWidget->setAllowOpacity( true );
    connect( colorWidget, &QgsCompoundColorWidget::currentColorChanged, this, [ = ]( const QColor & color )
    {
      for ( QgsSymbol *symbol : symbolList )
      {
        if ( symbol )
          symbol->setColor( color );
      }
      refreshSymbolView();
    } );
    panel->openPanel( colorWidget );
  }
  else
  {
    // modal dialog version... yuck
    const QColor color = QgsColorDialog::getColor( firstSymbol->color(), this, QStringLiteral( "Change Symbol Color" ), true );
    if ( color.isValid() )
    {
      for ( QgsSymbol *symbol : symbolList )
      {
        if ( symbol )
          symbol->setColor( color );
      }
      refreshSymbolView();
    }
  }
}

void QgsRendererWidget::changeSymbolOpacity()
{
  const QList<QgsSymbol *> symbolList = selectedSymbols();
  if ( symbolList.isEmpty() )
  {
    return;
  }

  QgsSymbol *firstSymbol = nullptr;
  const auto constSymbolList = symbolList;
  for ( QgsSymbol *symbol : constSymbolList )
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
  const double oldOpacity = firstSymbol->opacity() * 100; // convert to %
  const double opacity = QInputDialog::getDouble( this, tr( "Opacity" ), tr( "Change symbol opacity [%]" ), oldOpacity, 0.0, 100.0, 1, &ok );
  if ( ok )
  {
    const auto constSymbolList = symbolList;
    for ( QgsSymbol *symbol : constSymbolList )
    {
      if ( symbol )
        symbol->setOpacity( opacity / 100.0 );
    }
    refreshSymbolView();
  }
}

void QgsRendererWidget::changeSymbolUnit()
{
  const QList<QgsSymbol *> symbolList = selectedSymbols();
  if ( symbolList.isEmpty() )
  {
    return;
  }

  QgsSymbol *firstSymbol = nullptr;
  const auto constSymbolList = symbolList;
  for ( QgsSymbol *symbol : constSymbolList )
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
  const int currentUnit = ( firstSymbol->outputUnit() == QgsUnitTypes::RenderMillimeters ) ? 0 : 1;
  const QString item = QInputDialog::getItem( this, tr( "Symbol unit" ), tr( "Select symbol unit" ), QStringList() << tr( "Millimeter" ) << tr( "Map unit" ), currentUnit, false, &ok );
  if ( ok )
  {
    const QgsUnitTypes::RenderUnit unit = ( item.compare( tr( "Millimeter" ) ) == 0 ) ? QgsUnitTypes::RenderMillimeters : QgsUnitTypes::RenderMapUnits;

    const auto constSymbolList = symbolList;
    for ( QgsSymbol *symbol : constSymbolList )
    {
      if ( symbol )
        symbol->setOutputUnit( unit );
    }
    refreshSymbolView();
  }
}

void QgsRendererWidget::changeSymbolWidth()
{
  const QList<QgsSymbol *> symbolList = selectedSymbols();
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
      const auto constSymbolList = symbolList;
      for ( QgsSymbol *symbol : constSymbolList )
      {
        if ( !symbol )
          continue;

        if ( symbol->type() == Qgis::SymbolType::Line )
          static_cast<QgsLineSymbol *>( symbol )->setWidth( dlg.mSpinBox->value() );
      }
    }
    refreshSymbolView();
  }
}

void QgsRendererWidget::changeSymbolSize()
{
  const QList<QgsSymbol *> symbolList = selectedSymbols();
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
      const auto constSymbolList = symbolList;
      for ( QgsSymbol *symbol : constSymbolList )
      {
        if ( !symbol )
          continue;

        if ( symbol->type() == Qgis::SymbolType::Marker )
          static_cast<QgsMarkerSymbol *>( symbol )->setSize( dlg.mSpinBox->value() );
      }
    }
    refreshSymbolView();
  }
}

void QgsRendererWidget::changeSymbolAngle()
{
  const QList<QgsSymbol *> symbolList = selectedSymbols();
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
      const auto constSymbolList = symbolList;
      for ( QgsSymbol *symbol : constSymbolList )
      {
        if ( !symbol )
          continue;

        if ( symbol->type() == Qgis::SymbolType::Marker )
          static_cast<QgsMarkerSymbol *>( symbol )->setAngle( dlg.mSpinBox->value() );
      }
    }
    refreshSymbolView();
  }
}

void QgsRendererWidget::pasteSymbolToSelection()
{

}

void QgsRendererWidget::copySymbol()
{
  const QList<QgsSymbol *> symbolList = selectedSymbols();
  if ( symbolList.isEmpty() )
  {
    return;
  }

  QApplication::clipboard()->setMimeData( QgsSymbolLayerUtils::symbolToMimeData( symbolList.at( 0 ) ) );
}

void QgsRendererWidget::showSymbolLevelsDialog( QgsFeatureRenderer *r )
{
  QgsPanelWidget *panel = QgsPanelWidget::findParentPanel( this );
  if ( panel && panel->dockMode() )
  {
    QgsSymbolLevelsWidget *widget = new QgsSymbolLevelsWidget( r->legendSymbolItems(), r->usingSymbolLevels(), panel );
    widget->setPanelTitle( tr( "Symbol Levels" ) );
    connect( widget, &QgsPanelWidget::widgetChanged, this, [ = ]()
    {
      setSymbolLevels( widget->symbolLevels(), widget->usingLevels() );
    } );
    panel->openPanel( widget );
  }
  else
  {
    QgsSymbolLevelsDialog dlg( r, r->usingSymbolLevels(), panel );
    if ( dlg.exec() )
    {
      setSymbolLevels( dlg.symbolLevels(), dlg.usingLevels() );
    }
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

void QgsRendererWidget::setDockMode( bool dockMode )
{
  if ( dockMode )
  {
    // when in dock mode, these shortcuts conflict with the main window shortcuts and cannot be used
    if ( mCopyAction )
      mCopyAction->setShortcut( QKeySequence() );
    if ( mPasteAction )
      mPasteAction->setShortcut( QKeySequence() );
  }
  QgsPanelWidget::setDockMode( dockMode );
}

void QgsRendererWidget::disableSymbolLevels()
{
}

QgsDataDefinedSizeLegendWidget *QgsRendererWidget::createDataDefinedSizeLegendWidget( const QgsMarkerSymbol *symbol, const QgsDataDefinedSizeLegend *ddsLegend )
{
  const QgsProperty ddSize = symbol->dataDefinedSize();
  if ( !ddSize || !ddSize.isActive() )
  {
    QMessageBox::warning( this, tr( "Data-defined Size Legend" ), tr( "Data-defined size is not enabled!" ) );
    return nullptr;
  }

  QgsDataDefinedSizeLegendWidget *panel = new QgsDataDefinedSizeLegendWidget( ddsLegend, ddSize, symbol->clone(), mContext.mapCanvas() );
  connect( panel, &QgsPanelWidget::widgetChanged, this, &QgsPanelWidget::widgetChanged );
  return panel;
}

void QgsRendererWidget::setSymbolLevels( const QList< QgsLegendSymbolItem > &, bool )
{

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
  if ( auto *lMapCanvas = mContext.mapCanvas() )
  {
    expContext << QgsExpressionContextUtils::mapSettingsScope( lMapCanvas->mapSettings() )
               << new QgsExpressionContextScope( lMapCanvas->expressionContextScope() );

    if ( const QgsExpressionContextScopeGenerator *generator = dynamic_cast< const QgsExpressionContextScopeGenerator * >( lMapCanvas->temporalController() ) )
    {
      expContext << generator->createExpressionContextScope();
    }
  }
  else
  {
    expContext << QgsExpressionContextUtils::mapSettingsScope( QgsMapSettings() );
  }

  if ( auto *lVectorLayer = vectorLayer() )
    expContext << QgsExpressionContextUtils::layerScope( lVectorLayer );

  // additional scopes
  const auto constAdditionalExpressionContextScopes = mContext.additionalExpressionContextScopes();
  for ( const QgsExpressionContextScope &scope : constAdditionalExpressionContextScopes )
  {
    expContext.appendScope( new QgsExpressionContextScope( scope ) );
  }

  return expContext;
}

void QgsDataDefinedValueDialog::init( int propertyKey )
{
  const QgsProperty dd( symbolDataDefined() );

  mDDBtn->init( propertyKey, dd, QgsSymbolLayer::propertyDefinitions(), mLayer );
  mDDBtn->registerExpressionContextGenerator( this );

  QgsSymbol *initialSymbol = nullptr;
  const auto constMSymbolList = mSymbolList;
  for ( QgsSymbol *symbol : constMSymbolList )
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
  const QgsProperty dd = symbolDataDefined( mSymbolList.back() );
  const auto constMSymbolList = mSymbolList;
  for ( QgsSymbol *it : constMSymbolList )
  {
    const QgsProperty symbolDD( symbolDataDefined( it ) );
    if ( !it || !dd || !symbolDD || symbolDD != dd )
      return QgsProperty();
  }
  return dd;
}

void QgsDataDefinedValueDialog::dataDefinedChanged()
{
  const QgsProperty dd( mDDBtn->toProperty() );
  mSpinBox->setEnabled( !dd.isActive() );

  const QgsProperty symbolDD( symbolDataDefined() );

  if ( // shall we remove datadefined expressions for layers ?
    ( symbolDD && symbolDD.isActive() && !dd.isActive() )
    // shall we set the "en masse" expression for properties ?
    || dd.isActive() )
  {
    const auto constMSymbolList = mSymbolList;
    for ( QgsSymbol *it : constMSymbolList )
      setDataDefined( it, dd );
  }
}

QgsDataDefinedSizeDialog::QgsDataDefinedSizeDialog( const QList<QgsSymbol *> &symbolList, QgsVectorLayer *layer )
  : QgsDataDefinedValueDialog( symbolList, layer, tr( "Size" ) )
{
  init( QgsSymbolLayer::PropertySize );
  if ( !symbolList.isEmpty() && symbolList.at( 0 ) && vectorLayer() )
  {
    mAssistantSymbol.reset( static_cast<const QgsMarkerSymbol *>( symbolList.at( 0 ) )->clone() );
    mDDBtn->setSymbol( mAssistantSymbol );
  }
}

QgsProperty QgsDataDefinedSizeDialog::symbolDataDefined( const QgsSymbol *symbol ) const
{
  const QgsMarkerSymbol *marker = static_cast<const QgsMarkerSymbol *>( symbol );
  return marker->dataDefinedSize();
}

double QgsDataDefinedSizeDialog::value( const QgsSymbol *symbol ) const
{
  return static_cast<const QgsMarkerSymbol *>( symbol )->size();
}

void QgsDataDefinedSizeDialog::setDataDefined( QgsSymbol *symbol, const QgsProperty &dd )
{
  static_cast<QgsMarkerSymbol *>( symbol )->setDataDefinedSize( dd );
  static_cast<QgsMarkerSymbol *>( symbol )->setScaleMethod( Qgis::ScaleMethod::ScaleDiameter );
}


QgsDataDefinedRotationDialog::QgsDataDefinedRotationDialog( const QList<QgsSymbol *> &symbolList, QgsVectorLayer *layer )
  : QgsDataDefinedValueDialog( symbolList, layer, tr( "Rotation" ) )
{
  init( QgsSymbolLayer::PropertyAngle );
}

QgsProperty QgsDataDefinedRotationDialog::symbolDataDefined( const QgsSymbol *symbol ) const
{
  const QgsMarkerSymbol *marker = static_cast<const QgsMarkerSymbol *>( symbol );
  return marker->dataDefinedAngle();
}

double QgsDataDefinedRotationDialog::value( const QgsSymbol *symbol ) const
{
  return static_cast<const QgsMarkerSymbol *>( symbol )->angle();
}

void QgsDataDefinedRotationDialog::setDataDefined( QgsSymbol *symbol, const QgsProperty &dd )
{
  static_cast<QgsMarkerSymbol *>( symbol )->setDataDefinedAngle( dd );
}


QgsDataDefinedWidthDialog::QgsDataDefinedWidthDialog( const QList<QgsSymbol *> &symbolList, QgsVectorLayer *layer )
  : QgsDataDefinedValueDialog( symbolList, layer, tr( "Width" ) )
{
  init( QgsSymbolLayer::PropertyStrokeWidth );
}

QgsProperty QgsDataDefinedWidthDialog::symbolDataDefined( const QgsSymbol *symbol ) const
{
  const QgsLineSymbol *line = static_cast<const QgsLineSymbol *>( symbol );
  return line->dataDefinedWidth();
}

double QgsDataDefinedWidthDialog::value( const QgsSymbol *symbol ) const
{
  return static_cast<const QgsLineSymbol *>( symbol )->width();
}

void QgsDataDefinedWidthDialog::setDataDefined( QgsSymbol *symbol, const QgsProperty &dd )
{
  static_cast<QgsLineSymbol *>( symbol )->setDataDefinedWidth( dd );
}

void QgsRendererWidget::apply()
{

}

/***************************************************************************
 qgssymbollayerwidget.cpp - symbol layer widgets

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

#include "qgssymbollayerwidget.h"
#include "moc_qgssymbollayerwidget.cpp"

#include "qgslinesymbollayer.h"
#include "qgsmarkersymbollayer.h"
#include "qgsfillsymbollayer.h"
#include "qgsgeometrygeneratorsymbollayer.h"
#include "qgsexpressioncontextutils.h"

#include "characterwidget.h"
#include "qgsdashspacedialog.h"
#include "qgssvgcache.h"
#include "qgssymbollayerutils.h"
#include "qgscolorramp.h"
#include "qgscolorrampbutton.h"
#include "qgsfontutils.h"
#include "qgsproperty.h"
#include "qgsmapcanvas.h"
#include "qgsapplication.h"
#include "qgsvectorlayer.h"
#include "qgssvgselectorwidget.h"
#include "qgsnewauxiliarylayerdialog.h"
#include "qgsnewauxiliaryfielddialog.h"
#include "qgsauxiliarystorage.h"
#include "qgsunitselectionwidget.h"
#include "qgsimagecache.h"
#include "qgslinesymbol.h"
#include "qgsmarkersymbol.h"
#include "qgsfillsymbol.h"
#include "qgsiconutils.h"
#include "qgslinearreferencingsymbollayer.h"
#include "qgsnumericformatselectorwidget.h"

#include <QAbstractButton>
#include <QButtonGroup>
#include <QColorDialog>
#include <QCursor>
#include <QDir>
#include <QFileDialog>
#include <QPainter>
#include <QStandardItemModel>
#include <QSvgRenderer>
#include <QMessageBox>
#include <QMenu>
#include <QAction>
#include <QInputDialog>
#include <QBuffer>
#include <QRegularExpression>
#include <QMovie>

QgsExpressionContext QgsSymbolLayerWidget::createExpressionContext() const
{
  if ( auto *lExpressionContext = mContext.expressionContext() )
    return *lExpressionContext;

  QgsExpressionContext expContext( mContext.globalProjectAtlasMapLayerScopes( vectorLayer() ) );

  QgsExpressionContextScope *symbolScope = QgsExpressionContextUtils::updateSymbolScope( nullptr, new QgsExpressionContextScope() );
  if ( const QgsSymbolLayer *symbolLayer = const_cast<QgsSymbolLayerWidget *>( this )->symbolLayer() )
  {
    //cheat a bit - set the symbol color variable to match the symbol layer's color (when we should really be using the *symbols*
    //color, but that's not accessible here). 99% of the time these will be the same anyway
    symbolScope->addVariable( QgsExpressionContextScope::StaticVariable( QgsExpressionContext::EXPR_SYMBOL_COLOR, symbolLayer->color(), true ) );
  }
  expContext << symbolScope;
  expContext.lastScope()->addVariable( QgsExpressionContextScope::StaticVariable( QgsExpressionContext::EXPR_GEOMETRY_PART_COUNT, 1, true ) );
  expContext.lastScope()->addVariable( QgsExpressionContextScope::StaticVariable( QgsExpressionContext::EXPR_GEOMETRY_PART_NUM, 1, true ) );
  expContext.lastScope()->addVariable( QgsExpressionContextScope::StaticVariable( QgsExpressionContext::EXPR_GEOMETRY_RING_NUM, 0, true ) );
  expContext.lastScope()->addVariable( QgsExpressionContextScope::StaticVariable( QgsExpressionContext::EXPR_GEOMETRY_POINT_COUNT, 1, true ) );
  expContext.lastScope()->addVariable( QgsExpressionContextScope::StaticVariable( QgsExpressionContext::EXPR_GEOMETRY_POINT_NUM, 1, true ) );
  expContext.lastScope()->addVariable( QgsExpressionContextScope::StaticVariable( QStringLiteral( "symbol_layer_count" ), 1, true ) );
  expContext.lastScope()->addVariable( QgsExpressionContextScope::StaticVariable( QStringLiteral( "symbol_layer_index" ), 1, true ) );
  expContext.lastScope()->addVariable( QgsExpressionContextScope::StaticVariable( QStringLiteral( "symbol_marker_row" ), 1, true ) );
  expContext.lastScope()->addVariable( QgsExpressionContextScope::StaticVariable( QStringLiteral( "symbol_marker_column" ), 1, true ) );
  expContext.lastScope()->addVariable( QgsExpressionContextScope::StaticVariable( QStringLiteral( "symbol_frame" ), 1, true ) );

  // additional scopes
  const auto constAdditionalExpressionContextScopes = mContext.additionalExpressionContextScopes();
  for ( const QgsExpressionContextScope &scope : constAdditionalExpressionContextScopes )
  {
    expContext.appendScope( new QgsExpressionContextScope( scope ) );
  }

  //TODO - show actual value
  expContext.setOriginalValueVariable( QVariant() );

  QStringList highlights;
  highlights << QgsExpressionContext::EXPR_ORIGINAL_VALUE << QgsExpressionContext::EXPR_SYMBOL_COLOR
             << QgsExpressionContext::EXPR_GEOMETRY_PART_COUNT << QgsExpressionContext::EXPR_GEOMETRY_PART_NUM
             << QgsExpressionContext::EXPR_GEOMETRY_RING_NUM
             << QgsExpressionContext::EXPR_GEOMETRY_POINT_COUNT << QgsExpressionContext::EXPR_GEOMETRY_POINT_NUM
             << QgsExpressionContext::EXPR_CLUSTER_COLOR << QgsExpressionContext::EXPR_CLUSTER_SIZE
             << QStringLiteral( "symbol_layer_count" ) << QStringLiteral( "symbol_layer_index" ) << QStringLiteral( "symbol_frame" );


  if ( expContext.hasVariable( QStringLiteral( "zoom_level" ) ) )
  {
    highlights << QStringLiteral( "zoom_level" );
  }
  if ( expContext.hasVariable( QStringLiteral( "vector_tile_zoom" ) ) )
  {
    highlights << QStringLiteral( "vector_tile_zoom" );
  }

  expContext.setHighlightedVariables( highlights );

  return expContext;
}

void QgsSymbolLayerWidget::setContext( const QgsSymbolWidgetContext &context )
{
  mContext = context;
  const auto unitSelectionWidgets = findChildren<QgsUnitSelectionWidget *>();
  for ( QgsUnitSelectionWidget *unitWidget : unitSelectionWidgets )
  {
    unitWidget->setMapCanvas( mContext.mapCanvas() );
  }
}

QgsSymbolWidgetContext QgsSymbolLayerWidget::context() const
{
  return mContext;
}

void QgsSymbolLayerWidget::registerDataDefinedButton( QgsPropertyOverrideButton *button, QgsSymbolLayer::Property key )
{
  button->init( static_cast<int>( key ), symbolLayer()->dataDefinedProperties(), QgsSymbolLayer::propertyDefinitions(), mVectorLayer, true );
  connect( button, &QgsPropertyOverrideButton::changed, this, &QgsSymbolLayerWidget::updateDataDefinedProperty );
  connect( button, &QgsPropertyOverrideButton::createAuxiliaryField, this, &QgsSymbolLayerWidget::createAuxiliaryField );

  button->registerExpressionContextGenerator( this );
}

void QgsSymbolLayerWidget::createAuxiliaryField()
{
  // try to create an auxiliary layer if not yet created
  if ( !mVectorLayer->auxiliaryLayer() )
  {
    QgsNewAuxiliaryLayerDialog dlg( mVectorLayer, this );
    dlg.exec();
  }

  // return if still not exists
  if ( !mVectorLayer->auxiliaryLayer() )
    return;

  QgsPropertyOverrideButton *button = qobject_cast<QgsPropertyOverrideButton *>( sender() );
  QgsSymbolLayer::Property key = static_cast<QgsSymbolLayer::Property>( button->propertyKey() );
  QgsPropertyDefinition def = QgsSymbolLayer::propertyDefinitions()[static_cast<int>( key )];

  // create property in auxiliary storage if necessary
  if ( !mVectorLayer->auxiliaryLayer()->exists( def ) )
  {
    QgsNewAuxiliaryFieldDialog dlg( def, mVectorLayer, true, this );
    if ( dlg.exec() == QDialog::Accepted )
      def = dlg.propertyDefinition();
  }

  // return if still not exist
  if ( !mVectorLayer->auxiliaryLayer()->exists( def ) )
    return;

  // update property with join field name from auxiliary storage
  QgsProperty property = button->toProperty();
  property.setField( QgsAuxiliaryLayer::nameFromProperty( def, true ) );
  property.setActive( true );
  button->updateFieldLists();
  button->setToProperty( property );
  symbolLayer()->setDataDefinedProperty( key, button->toProperty() );

  emit changed();
}

void QgsSymbolLayerWidget::updateDataDefinedProperty()
{
  QgsPropertyOverrideButton *button = qobject_cast<QgsPropertyOverrideButton *>( sender() );
  const QgsSymbolLayer::Property key = static_cast<QgsSymbolLayer::Property>( button->propertyKey() );
  symbolLayer()->setDataDefinedProperty( key, button->toProperty() );
  emit changed();
}

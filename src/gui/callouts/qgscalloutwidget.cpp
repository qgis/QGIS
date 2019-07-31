/***************************************************************************
    qgscalloutwidget.cpp
    ---------------------
    begin                : July 2019
    copyright            : (C) 2019 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgscalloutwidget.h"
#include "qgsvectorlayer.h"
#include "qgsexpressioncontextutils.h"
#include "qgsunitselectionwidget.h"
#include "qgscallout.h"
#include "qgsnewauxiliaryfielddialog.h"
#include "qgsnewauxiliarylayerdialog.h"
#include "qgsauxiliarystorage.h"

QgsExpressionContext QgsCalloutWidget::createExpressionContext() const
{
  if ( mContext.expressionContext() )
    return *mContext.expressionContext();

  QgsExpressionContext expContext( mContext.globalProjectAtlasMapLayerScopes( vectorLayer() ) );
  QgsExpressionContextScope *symbolScope = QgsExpressionContextUtils::updateSymbolScope( nullptr, new QgsExpressionContextScope() );
  symbolScope->addVariable( QgsExpressionContextScope::StaticVariable( QgsExpressionContext::EXPR_SYMBOL_COLOR, QColor(), true ) );
  expContext << symbolScope;

  // additional scopes
  const auto constAdditionalExpressionContextScopes = mContext.additionalExpressionContextScopes();
  for ( const QgsExpressionContextScope &scope : constAdditionalExpressionContextScopes )
  {
    expContext.appendScope( new QgsExpressionContextScope( scope ) );
  }

  //TODO - show actual value
  expContext.setOriginalValueVariable( QVariant() );

  expContext.setHighlightedVariables( QStringList() << QgsExpressionContext::EXPR_ORIGINAL_VALUE << QgsExpressionContext::EXPR_SYMBOL_COLOR );

  return expContext;
}

void QgsCalloutWidget::setContext( const QgsSymbolWidgetContext &context )
{
  mContext = context;
  const auto unitSelectionWidgets = findChildren<QgsUnitSelectionWidget *>();
  for ( QgsUnitSelectionWidget *unitWidget : unitSelectionWidgets )
  {
    unitWidget->setMapCanvas( mContext.mapCanvas() );
  }
  const auto symbolButtonWidgets = findChildren<QgsSymbolButton *>();
  for ( QgsSymbolButton *symbolWidget : symbolButtonWidgets )
  {
    symbolWidget->setMapCanvas( mContext.mapCanvas() );
    symbolWidget->setMessageBar( mContext.messageBar() );
  }
}

QgsSymbolWidgetContext QgsCalloutWidget::context() const
{
  return mContext;
}

void QgsCalloutWidget::registerDataDefinedButton( QgsPropertyOverrideButton *button, QgsCallout::Property key )
{
  button->init( key, callout()->dataDefinedProperties(), QgsCallout::propertyDefinitions(), mVectorLayer, true );
  connect( button, &QgsPropertyOverrideButton::changed, this, &QgsCalloutWidget::updateDataDefinedProperty );
  connect( button, &QgsPropertyOverrideButton::createAuxiliaryField, this, &QgsCalloutWidget::createAuxiliaryField );

  button->registerExpressionContextGenerator( this );
}

void QgsCalloutWidget::createAuxiliaryField()
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
  QgsCallout::Property key = static_cast<  QgsCallout::Property >( button->propertyKey() );
  QgsPropertyDefinition def = QgsCallout::propertyDefinitions()[key];

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

  callout()->dataDefinedProperties().setProperty( key, button->toProperty() );

  emit changed();
}

void QgsCalloutWidget::updateDataDefinedProperty()
{
  QgsPropertyOverrideButton *button = qobject_cast<QgsPropertyOverrideButton *>( sender() );
  QgsCallout::Property key = static_cast<  QgsCallout::Property >( button->propertyKey() );
  callout()->dataDefinedProperties().setProperty( key, button->toProperty() );
  emit changed();
}

/// @cond PRIVATE

//
// QgsSimpleLineCalloutWidget
//

QgsSimpleLineCalloutWidget::QgsSimpleLineCalloutWidget( QgsVectorLayer *vl, QWidget *parent )
  : QgsCalloutWidget( parent, vl )
{
  setupUi( this );

  // Callout options - to move to custom widgets when multiple callout styles exist
  mCalloutLineStyleButton->setSymbolType( QgsSymbol::Line );
  mCalloutLineStyleButton->setDialogTitle( tr( "Callout Symbol" ) );
  mCalloutLineStyleButton->registerExpressionContextGenerator( this );

  mCalloutLineStyleButton->setLayer( vl );
  mMinCalloutWidthUnitWidget->setUnits( QgsUnitTypes::RenderUnitList() << QgsUnitTypes::RenderMillimeters << QgsUnitTypes::RenderMetersInMapUnits << QgsUnitTypes::RenderMapUnits << QgsUnitTypes::RenderPixels
                                        << QgsUnitTypes::RenderPoints << QgsUnitTypes::RenderInches );
  mOffsetFromAnchorUnitWidget->setUnits( QgsUnitTypes::RenderUnitList() << QgsUnitTypes::RenderMillimeters << QgsUnitTypes::RenderMetersInMapUnits << QgsUnitTypes::RenderMapUnits << QgsUnitTypes::RenderPixels
                                         << QgsUnitTypes::RenderPoints << QgsUnitTypes::RenderInches );
  mOffsetFromLabelUnitWidget->setUnits( QgsUnitTypes::RenderUnitList() << QgsUnitTypes::RenderMillimeters << QgsUnitTypes::RenderMetersInMapUnits << QgsUnitTypes::RenderMapUnits << QgsUnitTypes::RenderPixels
                                        << QgsUnitTypes::RenderPoints << QgsUnitTypes::RenderInches );

  connect( mMinCalloutWidthUnitWidget, &QgsUnitSelectionWidget::changed, this, &QgsSimpleLineCalloutWidget::minimumLengthUnitWidgetChanged );
  connect( mMinCalloutLengthSpin, static_cast < void ( QDoubleSpinBox::* )( double ) > ( &QDoubleSpinBox::valueChanged ), this, &QgsSimpleLineCalloutWidget::minimumLengthChanged );

  connect( mOffsetFromAnchorUnitWidget, &QgsUnitSelectionWidget::changed, this, &QgsSimpleLineCalloutWidget::offsetFromAnchorUnitWidgetChanged );
  connect( mOffsetFromAnchorSpin, static_cast < void ( QDoubleSpinBox::* )( double ) > ( &QDoubleSpinBox::valueChanged ), this, &QgsSimpleLineCalloutWidget::offsetFromAnchorChanged );
  connect( mOffsetFromLabelUnitWidget, &QgsUnitSelectionWidget::changed, this, &QgsSimpleLineCalloutWidget::offsetFromLabelUnitWidgetChanged );
  connect( mOffsetFromLabelSpin, static_cast < void ( QDoubleSpinBox::* )( double ) > ( &QDoubleSpinBox::valueChanged ), this, &QgsSimpleLineCalloutWidget::offsetFromLabelChanged );

  connect( mDrawToAllPartsCheck, &QCheckBox::toggled, this, &QgsSimpleLineCalloutWidget::drawToAllPartsToggled );

  // Anchor point options
  mAnchorPointComboBox->addItem( tr( "Pole of Inaccessibility" ), static_cast< int >( QgsCallout::PoleOfInaccessibility ) );
  mAnchorPointComboBox->addItem( tr( "Point on Exterior" ), static_cast< int >( QgsCallout::PointOnExterior ) );
  mAnchorPointComboBox->addItem( tr( "Point on Surface" ), static_cast< int >( QgsCallout::PointOnSurface ) );
  mAnchorPointComboBox->addItem( tr( "Centroid" ), static_cast< int >( QgsCallout::Centroid ) );
  connect( mAnchorPointComboBox, static_cast<void ( QComboBox::* )( int )>( &QComboBox::currentIndexChanged ), this, &QgsSimpleLineCalloutWidget::mAnchorPointComboBox_currentIndexChanged );

  connect( mCalloutLineStyleButton, &QgsSymbolButton::changed, this, &QgsSimpleLineCalloutWidget::lineSymbolChanged );
}

void QgsSimpleLineCalloutWidget::setCallout( QgsCallout *callout )
{
  if ( !callout )
    return;

  mCallout.reset( dynamic_cast<QgsSimpleLineCallout *>( callout->clone() ) );
  if ( !mCallout )
    return;

  mMinCalloutWidthUnitWidget->blockSignals( true );
  mMinCalloutWidthUnitWidget->setUnit( mCallout->minimumLengthUnit() );
  mMinCalloutWidthUnitWidget->setMapUnitScale( mCallout->minimumLengthMapUnitScale() );
  mMinCalloutWidthUnitWidget->blockSignals( false );

  whileBlocking( mMinCalloutLengthSpin )->setValue( mCallout->minimumLength() );

  mOffsetFromAnchorUnitWidget->blockSignals( true );
  mOffsetFromAnchorUnitWidget->setUnit( mCallout->offsetFromAnchorUnit() );
  mOffsetFromAnchorUnitWidget->setMapUnitScale( mCallout->offsetFromAnchorMapUnitScale() );
  mOffsetFromAnchorUnitWidget->blockSignals( false );
  mOffsetFromLabelUnitWidget->blockSignals( true );
  mOffsetFromLabelUnitWidget->setUnit( mCallout->offsetFromLabelUnit() );
  mOffsetFromLabelUnitWidget->setMapUnitScale( mCallout->offsetFromLabelMapUnitScale() );
  mOffsetFromLabelUnitWidget->blockSignals( false );
  whileBlocking( mOffsetFromAnchorSpin )->setValue( mCallout->offsetFromAnchor() );
  whileBlocking( mOffsetFromLabelSpin )->setValue( mCallout->offsetFromLabel() );

  whileBlocking( mCalloutLineStyleButton )->setSymbol( mCallout->lineSymbol()->clone() );

  whileBlocking( mDrawToAllPartsCheck )->setChecked( mCallout->drawCalloutToAllParts() );

  whileBlocking( mAnchorPointComboBox )->setCurrentIndex( mAnchorPointComboBox->findData( static_cast< int >( callout->anchorPoint() ) ) );

  registerDataDefinedButton( mMinCalloutLengthDDBtn, QgsCallout::MinimumCalloutLength );
  registerDataDefinedButton( mOffsetFromAnchorDDBtn, QgsCallout::OffsetFromAnchor );
  registerDataDefinedButton( mOffsetFromLabelDDBtn, QgsCallout::OffsetFromLabel );
  registerDataDefinedButton( mDrawToAllPartsDDBtn, QgsCallout::DrawCalloutToAllParts );
  registerDataDefinedButton( mAnchorPointDDBtn, QgsCallout::AnchorPointPosition );
}

void QgsSimpleLineCalloutWidget::setGeometryType( QgsWkbTypes::GeometryType type )
{
  bool isPolygon = type == QgsWkbTypes::PolygonGeometry;
  mAnchorPointLbl->setEnabled( isPolygon );
  mAnchorPointLbl->setVisible( isPolygon );
  mAnchorPointComboBox->setEnabled( isPolygon );
  mAnchorPointComboBox->setVisible( isPolygon );
  mAnchorPointDDBtn->setEnabled( isPolygon );
  mAnchorPointDDBtn->setVisible( isPolygon );
}

QgsCallout *QgsSimpleLineCalloutWidget::callout()
{
  return mCallout.get();
}

void QgsSimpleLineCalloutWidget::minimumLengthChanged()
{
  mCallout->setMinimumLength( mMinCalloutLengthSpin->value() );
  emit changed();
}

void QgsSimpleLineCalloutWidget::minimumLengthUnitWidgetChanged()
{
  mCallout->setMinimumLengthUnit( mMinCalloutWidthUnitWidget->unit() );
  mCallout->setMinimumLengthMapUnitScale( mMinCalloutWidthUnitWidget->getMapUnitScale() );
  emit changed();
}

void QgsSimpleLineCalloutWidget::offsetFromAnchorUnitWidgetChanged()
{
  mCallout->setOffsetFromAnchorUnit( mOffsetFromAnchorUnitWidget->unit() );
  mCallout->setOffsetFromAnchorMapUnitScale( mOffsetFromAnchorUnitWidget->getMapUnitScale() );
  emit changed();
}

void QgsSimpleLineCalloutWidget::offsetFromAnchorChanged()
{
  mCallout->setOffsetFromAnchor( mOffsetFromAnchorSpin->value() );
  emit changed();
}

void QgsSimpleLineCalloutWidget::offsetFromLabelUnitWidgetChanged()
{
  mCallout->setOffsetFromLabelUnit( mOffsetFromLabelUnitWidget->unit() );
  mCallout->setOffsetFromLabelMapUnitScale( mOffsetFromLabelUnitWidget->getMapUnitScale() );
  emit changed();
}

void QgsSimpleLineCalloutWidget::offsetFromLabelChanged()
{
  mCallout->setOffsetFromLabel( mOffsetFromLabelSpin->value() );
  emit changed();
}

void QgsSimpleLineCalloutWidget::lineSymbolChanged()
{
  mCallout->setLineSymbol( mCalloutLineStyleButton->clonedSymbol< QgsLineSymbol >() );
  emit changed();
}

void QgsSimpleLineCalloutWidget::mAnchorPointComboBox_currentIndexChanged( int index )
{
  mCallout->setAnchorPoint( static_cast<QgsCallout::AnchorPoint>( mAnchorPointComboBox->itemData( index ).toInt() ) );
  emit changed();
}

void QgsSimpleLineCalloutWidget::drawToAllPartsToggled( bool active )
{
  mCallout->setDrawCalloutToAllParts( active );
  emit changed();
}


//
// QgsManhattanLineCalloutWidget
//

QgsManhattanLineCalloutWidget::QgsManhattanLineCalloutWidget( QgsVectorLayer *vl, QWidget *parent )
  : QgsSimpleLineCalloutWidget( vl, parent )
{

}


///@endcond

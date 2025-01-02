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
#include "moc_qgscalloutwidget.cpp"
#include "qgsvectorlayer.h"
#include "qgsexpressioncontextutils.h"
#include "qgsunitselectionwidget.h"
#include "qgscallout.h"
#include "qgsnewauxiliaryfielddialog.h"
#include "qgsnewauxiliarylayerdialog.h"
#include "qgsauxiliarystorage.h"
#include "qgslinesymbol.h"
#include "qgsfillsymbol.h"
#include "qgsmarkersymbol.h"

QgsExpressionContext QgsCalloutWidget::createExpressionContext() const
{
  if ( auto *lExpressionContext = mContext.expressionContext() )
    return *lExpressionContext;

  QgsExpressionContext expContext( mContext.globalProjectAtlasMapLayerScopes( layer() ) );
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
  button->init( static_cast<int>( key ), callout()->dataDefinedProperties(), QgsCallout::propertyDefinitions(), qobject_cast<QgsVectorLayer *>( mLayer ), true );
  connect( button, &QgsPropertyOverrideButton::changed, this, &QgsCalloutWidget::updateDataDefinedProperty );
  connect( button, &QgsPropertyOverrideButton::createAuxiliaryField, this, &QgsCalloutWidget::createAuxiliaryField );

  button->registerExpressionContextGenerator( this );
}

void QgsCalloutWidget::createAuxiliaryField()
{
  // try to create an auxiliary layer if not yet created
  QgsVectorLayer *vectorLayer = qobject_cast<QgsVectorLayer *>( mLayer );
  if ( !vectorLayer )
    return;

  if ( !vectorLayer->auxiliaryLayer() )
  {
    QgsNewAuxiliaryLayerDialog dlg( vectorLayer, this );
    dlg.exec();
  }

  // return if still not exists
  if ( !vectorLayer->auxiliaryLayer() )
    return;

  QgsPropertyOverrideButton *button = qobject_cast<QgsPropertyOverrideButton *>( sender() );
  const QgsCallout::Property key = static_cast<QgsCallout::Property>( button->propertyKey() );
  const QgsPropertyDefinition def = QgsCallout::propertyDefinitions()[static_cast<int>( key )];

  // create property in auxiliary storage if necessary
  if ( !vectorLayer->auxiliaryLayer()->exists( def ) )
  {
    vectorLayer->auxiliaryLayer()->addAuxiliaryField( def );
  }

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
  const QgsCallout::Property key = static_cast<QgsCallout::Property>( button->propertyKey() );
  callout()->dataDefinedProperties().setProperty( key, button->toProperty() );
  emit changed();
}

///@cond PRIVATE

//
// QgsSimpleLineCalloutWidget
//

QgsSimpleLineCalloutWidget::QgsSimpleLineCalloutWidget( QgsMapLayer *vl, QWidget *parent )
  : QgsCalloutWidget( parent, vl )
{
  setupUi( this );

  // Callout options - to move to custom widgets when multiple callout styles exist
  mCalloutLineStyleButton->setSymbolType( Qgis::SymbolType::Line );
  mCalloutLineStyleButton->setDialogTitle( tr( "Callout Symbol" ) );
  mCalloutLineStyleButton->registerExpressionContextGenerator( this );

  mCalloutLineStyleButton->setLayer( qobject_cast<QgsVectorLayer *>( vl ) );
  mMinCalloutWidthUnitWidget->setUnits( QgsUnitTypes::RenderUnitList() << Qgis::RenderUnit::Millimeters << Qgis::RenderUnit::MetersInMapUnits << Qgis::RenderUnit::MapUnits << Qgis::RenderUnit::Pixels << Qgis::RenderUnit::Points << Qgis::RenderUnit::Inches );
  mOffsetFromAnchorUnitWidget->setUnits( QgsUnitTypes::RenderUnitList() << Qgis::RenderUnit::Millimeters << Qgis::RenderUnit::MetersInMapUnits << Qgis::RenderUnit::MapUnits << Qgis::RenderUnit::Pixels << Qgis::RenderUnit::Points << Qgis::RenderUnit::Inches );
  mOffsetFromLabelUnitWidget->setUnits( QgsUnitTypes::RenderUnitList() << Qgis::RenderUnit::Millimeters << Qgis::RenderUnit::MetersInMapUnits << Qgis::RenderUnit::MapUnits << Qgis::RenderUnit::Pixels << Qgis::RenderUnit::Points << Qgis::RenderUnit::Inches );

  connect( mMinCalloutWidthUnitWidget, &QgsUnitSelectionWidget::changed, this, &QgsSimpleLineCalloutWidget::minimumLengthUnitWidgetChanged );
  connect( mMinCalloutLengthSpin, static_cast<void ( QDoubleSpinBox::* )( double )>( &QDoubleSpinBox::valueChanged ), this, &QgsSimpleLineCalloutWidget::minimumLengthChanged );

  connect( mOffsetFromAnchorUnitWidget, &QgsUnitSelectionWidget::changed, this, &QgsSimpleLineCalloutWidget::offsetFromAnchorUnitWidgetChanged );
  connect( mOffsetFromAnchorSpin, static_cast<void ( QDoubleSpinBox::* )( double )>( &QDoubleSpinBox::valueChanged ), this, &QgsSimpleLineCalloutWidget::offsetFromAnchorChanged );
  connect( mOffsetFromLabelUnitWidget, &QgsUnitSelectionWidget::changed, this, &QgsSimpleLineCalloutWidget::offsetFromLabelUnitWidgetChanged );
  connect( mOffsetFromLabelSpin, static_cast<void ( QDoubleSpinBox::* )( double )>( &QDoubleSpinBox::valueChanged ), this, &QgsSimpleLineCalloutWidget::offsetFromLabelChanged );

  connect( mDrawToAllPartsCheck, &QCheckBox::toggled, this, &QgsSimpleLineCalloutWidget::drawToAllPartsToggled );

  // Anchor point options
  mAnchorPointComboBox->addItem( tr( "Pole of Inaccessibility" ), static_cast<int>( QgsCallout::PoleOfInaccessibility ) );
  mAnchorPointComboBox->addItem( tr( "Point on Exterior" ), static_cast<int>( QgsCallout::PointOnExterior ) );
  mAnchorPointComboBox->addItem( tr( "Point on Surface" ), static_cast<int>( QgsCallout::PointOnSurface ) );
  mAnchorPointComboBox->addItem( tr( "Centroid" ), static_cast<int>( QgsCallout::Centroid ) );
  connect( mAnchorPointComboBox, static_cast<void ( QComboBox::* )( int )>( &QComboBox::currentIndexChanged ), this, &QgsSimpleLineCalloutWidget::mAnchorPointComboBox_currentIndexChanged );

  mLabelAnchorPointComboBox->addItem( tr( "Closest Point" ), static_cast<int>( QgsCallout::LabelPointOnExterior ) );
  mLabelAnchorPointComboBox->addItem( tr( "Centroid" ), static_cast<int>( QgsCallout::LabelCentroid ) );
  mLabelAnchorPointComboBox->addItem( tr( "Top Left" ), static_cast<int>( QgsCallout::LabelTopLeft ) );
  mLabelAnchorPointComboBox->addItem( tr( "Top Center" ), static_cast<int>( QgsCallout::LabelTopMiddle ) );
  mLabelAnchorPointComboBox->addItem( tr( "Top Right" ), static_cast<int>( QgsCallout::LabelTopRight ) );
  mLabelAnchorPointComboBox->addItem( tr( "Left Middle" ), static_cast<int>( QgsCallout::LabelMiddleLeft ) );
  mLabelAnchorPointComboBox->addItem( tr( "Right Middle" ), static_cast<int>( QgsCallout::LabelMiddleRight ) );
  mLabelAnchorPointComboBox->addItem( tr( "Bottom Left" ), static_cast<int>( QgsCallout::LabelBottomLeft ) );
  mLabelAnchorPointComboBox->addItem( tr( "Bottom Center" ), static_cast<int>( QgsCallout::LabelBottomMiddle ) );
  mLabelAnchorPointComboBox->addItem( tr( "Bottom Right" ), static_cast<int>( QgsCallout::LabelBottomRight ) );
  connect( mLabelAnchorPointComboBox, static_cast<void ( QComboBox::* )( int )>( &QComboBox::currentIndexChanged ), this, &QgsSimpleLineCalloutWidget::mLabelAnchorPointComboBox_currentIndexChanged );

  connect( mCalloutLineStyleButton, &QgsSymbolButton::changed, this, &QgsSimpleLineCalloutWidget::lineSymbolChanged );

  connect( mCalloutBlendComboBox, static_cast<void ( QComboBox::* )( int )>( &QComboBox::currentIndexChanged ), this, &QgsSimpleLineCalloutWidget::mCalloutBlendComboBox_currentIndexChanged );

  mPlacementDDGroupBox->setVisible( qobject_cast<QgsVectorLayer *>( vl ) );
}

void QgsSimpleLineCalloutWidget::setCallout( const QgsCallout *callout )
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

  whileBlocking( mAnchorPointComboBox )->setCurrentIndex( mAnchorPointComboBox->findData( static_cast<int>( callout->anchorPoint() ) ) );
  whileBlocking( mLabelAnchorPointComboBox )->setCurrentIndex( mLabelAnchorPointComboBox->findData( static_cast<int>( callout->labelAnchorPoint() ) ) );

  whileBlocking( mCalloutBlendComboBox )->setBlendMode( mCallout->blendMode() );

  registerDataDefinedButton( mMinCalloutLengthDDBtn, QgsCallout::Property::MinimumCalloutLength );
  registerDataDefinedButton( mOffsetFromAnchorDDBtn, QgsCallout::Property::OffsetFromAnchor );
  registerDataDefinedButton( mOffsetFromLabelDDBtn, QgsCallout::Property::OffsetFromLabel );
  registerDataDefinedButton( mDrawToAllPartsDDBtn, QgsCallout::Property::DrawCalloutToAllParts );
  registerDataDefinedButton( mAnchorPointDDBtn, QgsCallout::Property::AnchorPointPosition );
  registerDataDefinedButton( mLabelAnchorPointDDBtn, QgsCallout::Property::LabelAnchorPointPosition );
  registerDataDefinedButton( mCalloutBlendModeDDBtn, QgsCallout::Property::BlendMode );

  registerDataDefinedButton( mOriginXDDBtn, QgsCallout::Property::OriginX );
  registerDataDefinedButton( mOriginYDDBtn, QgsCallout::Property::OriginY );
  registerDataDefinedButton( mDestXDDBtn, QgsCallout::Property::DestinationX );
  registerDataDefinedButton( mDestYDDBtn, QgsCallout::Property::DestinationY );
}

void QgsSimpleLineCalloutWidget::setGeometryType( Qgis::GeometryType type )
{
  const bool isPolygon = type == Qgis::GeometryType::Polygon;
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
  mCallout->setLineSymbol( mCalloutLineStyleButton->clonedSymbol<QgsLineSymbol>() );
  emit changed();
}

void QgsSimpleLineCalloutWidget::mAnchorPointComboBox_currentIndexChanged( int index )
{
  mCallout->setAnchorPoint( static_cast<QgsCallout::AnchorPoint>( mAnchorPointComboBox->itemData( index ).toInt() ) );
  emit changed();
}

void QgsSimpleLineCalloutWidget::mLabelAnchorPointComboBox_currentIndexChanged( int index )
{
  mCallout->setLabelAnchorPoint( static_cast<QgsCallout::LabelAnchorPoint>( mLabelAnchorPointComboBox->itemData( index ).toInt() ) );
  emit changed();
}

void QgsSimpleLineCalloutWidget::mCalloutBlendComboBox_currentIndexChanged( int )
{
  mCallout->setBlendMode( mCalloutBlendComboBox->blendMode() );
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

QgsManhattanLineCalloutWidget::QgsManhattanLineCalloutWidget( QgsMapLayer *vl, QWidget *parent )
  : QgsSimpleLineCalloutWidget( vl, parent )
{
}


//
// QgsCurvedLineCalloutWidget
//

QgsCurvedLineCalloutWidget::QgsCurvedLineCalloutWidget( QgsMapLayer *vl, QWidget *parent )
  : QgsCalloutWidget( parent, vl )
{
  setupUi( this );

  // Callout options - to move to custom widgets when multiple callout styles exist
  mCalloutLineStyleButton->setSymbolType( Qgis::SymbolType::Line );
  mCalloutLineStyleButton->setDialogTitle( tr( "Callout Symbol" ) );
  mCalloutLineStyleButton->registerExpressionContextGenerator( this );

  mCalloutLineStyleButton->setLayer( qobject_cast<QgsVectorLayer *>( vl ) );
  mMinCalloutWidthUnitWidget->setUnits( QgsUnitTypes::RenderUnitList() << Qgis::RenderUnit::Millimeters << Qgis::RenderUnit::MetersInMapUnits << Qgis::RenderUnit::MapUnits << Qgis::RenderUnit::Pixels << Qgis::RenderUnit::Points << Qgis::RenderUnit::Inches );
  mOffsetFromAnchorUnitWidget->setUnits( QgsUnitTypes::RenderUnitList() << Qgis::RenderUnit::Millimeters << Qgis::RenderUnit::MetersInMapUnits << Qgis::RenderUnit::MapUnits << Qgis::RenderUnit::Pixels << Qgis::RenderUnit::Points << Qgis::RenderUnit::Inches );
  mOffsetFromLabelUnitWidget->setUnits( QgsUnitTypes::RenderUnitList() << Qgis::RenderUnit::Millimeters << Qgis::RenderUnit::MetersInMapUnits << Qgis::RenderUnit::MapUnits << Qgis::RenderUnit::Pixels << Qgis::RenderUnit::Points << Qgis::RenderUnit::Inches );

  connect( mMinCalloutWidthUnitWidget, &QgsUnitSelectionWidget::changed, this, &QgsCurvedLineCalloutWidget::minimumLengthUnitWidgetChanged );
  connect( mMinCalloutLengthSpin, static_cast<void ( QDoubleSpinBox::* )( double )>( &QDoubleSpinBox::valueChanged ), this, &QgsCurvedLineCalloutWidget::minimumLengthChanged );

  connect( mOffsetFromAnchorUnitWidget, &QgsUnitSelectionWidget::changed, this, &QgsCurvedLineCalloutWidget::offsetFromAnchorUnitWidgetChanged );
  connect( mOffsetFromAnchorSpin, static_cast<void ( QDoubleSpinBox::* )( double )>( &QDoubleSpinBox::valueChanged ), this, &QgsCurvedLineCalloutWidget::offsetFromAnchorChanged );
  connect( mOffsetFromLabelUnitWidget, &QgsUnitSelectionWidget::changed, this, &QgsCurvedLineCalloutWidget::offsetFromLabelUnitWidgetChanged );
  connect( mOffsetFromLabelSpin, static_cast<void ( QDoubleSpinBox::* )( double )>( &QDoubleSpinBox::valueChanged ), this, &QgsCurvedLineCalloutWidget::offsetFromLabelChanged );

  connect( mDrawToAllPartsCheck, &QCheckBox::toggled, this, &QgsCurvedLineCalloutWidget::drawToAllPartsToggled );

  mOrientationComboBox->addItem( tr( "Automatic" ), static_cast<int>( QgsCurvedLineCallout::Automatic ) );
  mOrientationComboBox->addItem( tr( "Clockwise" ), static_cast<int>( QgsCurvedLineCallout::Clockwise ) );
  mOrientationComboBox->addItem( tr( "Counter Clockwise" ), static_cast<int>( QgsCurvedLineCallout::CounterClockwise ) );
  connect( mOrientationComboBox, static_cast<void ( QComboBox::* )( int )>( &QComboBox::currentIndexChanged ), this, [=]( int index ) {
    mCallout->setOrientation( static_cast<QgsCurvedLineCallout::Orientation>( mOrientationComboBox->itemData( index ).toInt() ) );
    emit changed();
  } );

  // Anchor point options
  mAnchorPointComboBox->addItem( tr( "Pole of Inaccessibility" ), static_cast<int>( QgsCallout::PoleOfInaccessibility ) );
  mAnchorPointComboBox->addItem( tr( "Point on Exterior" ), static_cast<int>( QgsCallout::PointOnExterior ) );
  mAnchorPointComboBox->addItem( tr( "Point on Surface" ), static_cast<int>( QgsCallout::PointOnSurface ) );
  mAnchorPointComboBox->addItem( tr( "Centroid" ), static_cast<int>( QgsCallout::Centroid ) );
  connect( mAnchorPointComboBox, static_cast<void ( QComboBox::* )( int )>( &QComboBox::currentIndexChanged ), this, &QgsCurvedLineCalloutWidget::mAnchorPointComboBox_currentIndexChanged );

  mLabelAnchorPointComboBox->addItem( tr( "Closest Point" ), static_cast<int>( QgsCallout::LabelPointOnExterior ) );
  mLabelAnchorPointComboBox->addItem( tr( "Centroid" ), static_cast<int>( QgsCallout::LabelCentroid ) );
  mLabelAnchorPointComboBox->addItem( tr( "Top Left" ), static_cast<int>( QgsCallout::LabelTopLeft ) );
  mLabelAnchorPointComboBox->addItem( tr( "Top Center" ), static_cast<int>( QgsCallout::LabelTopMiddle ) );
  mLabelAnchorPointComboBox->addItem( tr( "Top Right" ), static_cast<int>( QgsCallout::LabelTopRight ) );
  mLabelAnchorPointComboBox->addItem( tr( "Left Middle" ), static_cast<int>( QgsCallout::LabelMiddleLeft ) );
  mLabelAnchorPointComboBox->addItem( tr( "Right Middle" ), static_cast<int>( QgsCallout::LabelMiddleRight ) );
  mLabelAnchorPointComboBox->addItem( tr( "Bottom Left" ), static_cast<int>( QgsCallout::LabelBottomLeft ) );
  mLabelAnchorPointComboBox->addItem( tr( "Bottom Center" ), static_cast<int>( QgsCallout::LabelBottomMiddle ) );
  mLabelAnchorPointComboBox->addItem( tr( "Bottom Right" ), static_cast<int>( QgsCallout::LabelBottomRight ) );
  connect( mLabelAnchorPointComboBox, static_cast<void ( QComboBox::* )( int )>( &QComboBox::currentIndexChanged ), this, &QgsCurvedLineCalloutWidget::mLabelAnchorPointComboBox_currentIndexChanged );

  connect( mCalloutLineStyleButton, &QgsSymbolButton::changed, this, &QgsCurvedLineCalloutWidget::lineSymbolChanged );

  connect( mCurvatureSlider, &QSlider::valueChanged, this, [=]( int value ) { mCurvatureSpinBox->setValue( value / 10.0 ); } );
  connect( mCurvatureSpinBox, static_cast<void ( QgsDoubleSpinBox::* )( double )>( &QgsDoubleSpinBox::valueChanged ), this, [=]( double value ) { whileBlocking( mCurvatureSlider )->setValue( value * 10 ); } );
  connect( mCurvatureSpinBox, static_cast<void ( QgsDoubleSpinBox::* )( double )>( &QgsDoubleSpinBox::valueChanged ), this, [=]( double value ) {
    mCallout->setCurvature( value / 100.0 );
    emit changed();
  } );

  connect( mCalloutBlendComboBox, static_cast<void ( QComboBox::* )( int )>( &QComboBox::currentIndexChanged ), this, &QgsCurvedLineCalloutWidget::mCalloutBlendComboBox_currentIndexChanged );

  mPlacementDDGroupBox->setVisible( qobject_cast<QgsVectorLayer *>( vl ) );
}

void QgsCurvedLineCalloutWidget::setCallout( const QgsCallout *callout )
{
  if ( !callout )
    return;

  mCallout.reset( dynamic_cast<QgsCurvedLineCallout *>( callout->clone() ) );
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

  whileBlocking( mOrientationComboBox )->setCurrentIndex( mOrientationComboBox->findData( static_cast<int>( mCallout->orientation() ) ) );

  whileBlocking( mAnchorPointComboBox )->setCurrentIndex( mAnchorPointComboBox->findData( static_cast<int>( callout->anchorPoint() ) ) );
  whileBlocking( mLabelAnchorPointComboBox )->setCurrentIndex( mLabelAnchorPointComboBox->findData( static_cast<int>( callout->labelAnchorPoint() ) ) );

  whileBlocking( mCalloutBlendComboBox )->setBlendMode( mCallout->blendMode() );

  whileBlocking( mCurvatureSpinBox )->setValue( mCallout->curvature() * 100.0 );
  whileBlocking( mCurvatureSlider )->setValue( mCallout->curvature() * 1000.0 );

  registerDataDefinedButton( mMinCalloutLengthDDBtn, QgsCallout::Property::MinimumCalloutLength );
  registerDataDefinedButton( mOffsetFromAnchorDDBtn, QgsCallout::Property::OffsetFromAnchor );
  registerDataDefinedButton( mOffsetFromLabelDDBtn, QgsCallout::Property::OffsetFromLabel );
  registerDataDefinedButton( mDrawToAllPartsDDBtn, QgsCallout::Property::DrawCalloutToAllParts );
  registerDataDefinedButton( mAnchorPointDDBtn, QgsCallout::Property::AnchorPointPosition );
  registerDataDefinedButton( mLabelAnchorPointDDBtn, QgsCallout::Property::LabelAnchorPointPosition );
  registerDataDefinedButton( mCalloutBlendModeDDBtn, QgsCallout::Property::BlendMode );
  registerDataDefinedButton( mCalloutCurvatureDDBtn, QgsCallout::Property::Curvature );
  registerDataDefinedButton( mCalloutOrientationDDBtn, QgsCallout::Property::Orientation );

  registerDataDefinedButton( mOriginXDDBtn, QgsCallout::Property::OriginX );
  registerDataDefinedButton( mOriginYDDBtn, QgsCallout::Property::OriginY );
  registerDataDefinedButton( mDestXDDBtn, QgsCallout::Property::DestinationX );
  registerDataDefinedButton( mDestYDDBtn, QgsCallout::Property::DestinationY );
}

void QgsCurvedLineCalloutWidget::setGeometryType( Qgis::GeometryType type )
{
  const bool isPolygon = type == Qgis::GeometryType::Polygon;
  mAnchorPointLbl->setEnabled( isPolygon );
  mAnchorPointLbl->setVisible( isPolygon );
  mAnchorPointComboBox->setEnabled( isPolygon );
  mAnchorPointComboBox->setVisible( isPolygon );
  mAnchorPointDDBtn->setEnabled( isPolygon );
  mAnchorPointDDBtn->setVisible( isPolygon );
}

QgsCallout *QgsCurvedLineCalloutWidget::callout()
{
  return mCallout.get();
}

void QgsCurvedLineCalloutWidget::minimumLengthChanged()
{
  mCallout->setMinimumLength( mMinCalloutLengthSpin->value() );
  emit changed();
}

void QgsCurvedLineCalloutWidget::minimumLengthUnitWidgetChanged()
{
  mCallout->setMinimumLengthUnit( mMinCalloutWidthUnitWidget->unit() );
  mCallout->setMinimumLengthMapUnitScale( mMinCalloutWidthUnitWidget->getMapUnitScale() );
  emit changed();
}

void QgsCurvedLineCalloutWidget::offsetFromAnchorUnitWidgetChanged()
{
  mCallout->setOffsetFromAnchorUnit( mOffsetFromAnchorUnitWidget->unit() );
  mCallout->setOffsetFromAnchorMapUnitScale( mOffsetFromAnchorUnitWidget->getMapUnitScale() );
  emit changed();
}

void QgsCurvedLineCalloutWidget::offsetFromAnchorChanged()
{
  mCallout->setOffsetFromAnchor( mOffsetFromAnchorSpin->value() );
  emit changed();
}

void QgsCurvedLineCalloutWidget::offsetFromLabelUnitWidgetChanged()
{
  mCallout->setOffsetFromLabelUnit( mOffsetFromLabelUnitWidget->unit() );
  mCallout->setOffsetFromLabelMapUnitScale( mOffsetFromLabelUnitWidget->getMapUnitScale() );
  emit changed();
}

void QgsCurvedLineCalloutWidget::offsetFromLabelChanged()
{
  mCallout->setOffsetFromLabel( mOffsetFromLabelSpin->value() );
  emit changed();
}

void QgsCurvedLineCalloutWidget::lineSymbolChanged()
{
  mCallout->setLineSymbol( mCalloutLineStyleButton->clonedSymbol<QgsLineSymbol>() );
  emit changed();
}

void QgsCurvedLineCalloutWidget::mAnchorPointComboBox_currentIndexChanged( int index )
{
  mCallout->setAnchorPoint( static_cast<QgsCallout::AnchorPoint>( mAnchorPointComboBox->itemData( index ).toInt() ) );
  emit changed();
}

void QgsCurvedLineCalloutWidget::mLabelAnchorPointComboBox_currentIndexChanged( int index )
{
  mCallout->setLabelAnchorPoint( static_cast<QgsCallout::LabelAnchorPoint>( mLabelAnchorPointComboBox->itemData( index ).toInt() ) );
  emit changed();
}

void QgsCurvedLineCalloutWidget::mCalloutBlendComboBox_currentIndexChanged( int )
{
  mCallout->setBlendMode( mCalloutBlendComboBox->blendMode() );
  emit changed();
}

void QgsCurvedLineCalloutWidget::drawToAllPartsToggled( bool active )
{
  mCallout->setDrawCalloutToAllParts( active );
  emit changed();
}


//
// QgsBalloonCalloutWidget
//

QgsBalloonCalloutWidget::QgsBalloonCalloutWidget( QgsMapLayer *vl, QWidget *parent )
  : QgsCalloutWidget( parent, vl )
{
  setupUi( this );

  // Callout options - to move to custom widgets when multiple callout styles exist
  mCalloutFillStyleButton->setSymbolType( Qgis::SymbolType::Fill );
  mCalloutFillStyleButton->setDialogTitle( tr( "Balloon Symbol" ) );
  mCalloutFillStyleButton->registerExpressionContextGenerator( this );

  mMarkerSymbolButton->setSymbolType( Qgis::SymbolType::Marker );
  mMarkerSymbolButton->setDialogTitle( tr( "Marker Symbol" ) );
  mMarkerSymbolButton->registerExpressionContextGenerator( this );
  mMarkerSymbolButton->setShowNull( true );
  mMarkerSymbolButton->setToNull();

  mCalloutFillStyleButton->setLayer( qobject_cast<QgsVectorLayer *>( vl ) );
  mMarkerSymbolButton->setLayer( qobject_cast<QgsVectorLayer *>( vl ) );
  mOffsetFromAnchorUnitWidget->setUnits( QgsUnitTypes::RenderUnitList() << Qgis::RenderUnit::Millimeters << Qgis::RenderUnit::MetersInMapUnits << Qgis::RenderUnit::MapUnits << Qgis::RenderUnit::Pixels << Qgis::RenderUnit::Points << Qgis::RenderUnit::Inches );
  mMarginUnitWidget->setUnits( QgsUnitTypes::RenderUnitList() << Qgis::RenderUnit::Millimeters << Qgis::RenderUnit::MetersInMapUnits << Qgis::RenderUnit::MapUnits << Qgis::RenderUnit::Pixels << Qgis::RenderUnit::Points << Qgis::RenderUnit::Inches );
  mWedgeWidthUnitWidget->setUnits( QgsUnitTypes::RenderUnitList() << Qgis::RenderUnit::Millimeters << Qgis::RenderUnit::MetersInMapUnits << Qgis::RenderUnit::MapUnits << Qgis::RenderUnit::Pixels << Qgis::RenderUnit::Points << Qgis::RenderUnit::Inches );
  mCornerRadiusUnitWidget->setUnits( QgsUnitTypes::RenderUnitList() << Qgis::RenderUnit::Millimeters << Qgis::RenderUnit::MetersInMapUnits << Qgis::RenderUnit::MapUnits << Qgis::RenderUnit::Pixels << Qgis::RenderUnit::Points << Qgis::RenderUnit::Inches );

  mSpinBottomMargin->setClearValue( 0 );
  mSpinTopMargin->setClearValue( 0 );
  mSpinRightMargin->setClearValue( 0 );
  mSpinLeftMargin->setClearValue( 0 );
  mWedgeWidthSpin->setClearValue( 2.64 );
  mCornerRadiusSpin->setClearValue( 0.0 );

  connect( mOffsetFromAnchorUnitWidget, &QgsUnitSelectionWidget::changed, this, &QgsBalloonCalloutWidget::offsetFromAnchorUnitWidgetChanged );
  connect( mOffsetFromAnchorSpin, static_cast<void ( QDoubleSpinBox::* )( double )>( &QDoubleSpinBox::valueChanged ), this, &QgsBalloonCalloutWidget::offsetFromAnchorChanged );

  // Anchor point options
  mAnchorPointComboBox->addItem( tr( "Pole of Inaccessibility" ), static_cast<int>( QgsCallout::PoleOfInaccessibility ) );
  mAnchorPointComboBox->addItem( tr( "Point on Exterior" ), static_cast<int>( QgsCallout::PointOnExterior ) );
  mAnchorPointComboBox->addItem( tr( "Point on Surface" ), static_cast<int>( QgsCallout::PointOnSurface ) );
  mAnchorPointComboBox->addItem( tr( "Centroid" ), static_cast<int>( QgsCallout::Centroid ) );
  connect( mAnchorPointComboBox, static_cast<void ( QComboBox::* )( int )>( &QComboBox::currentIndexChanged ), this, &QgsBalloonCalloutWidget::mAnchorPointComboBox_currentIndexChanged );

  connect( mCalloutFillStyleButton, &QgsSymbolButton::changed, this, &QgsBalloonCalloutWidget::fillSymbolChanged );
  connect( mMarkerSymbolButton, &QgsSymbolButton::changed, this, &QgsBalloonCalloutWidget::markerSymbolChanged );

  connect( mSpinBottomMargin, qOverload<double>( &QDoubleSpinBox::valueChanged ), this, [=]( double value ) {
    QgsMargins margins = mCallout->margins();
    margins.setBottom( value );
    mCallout->setMargins( margins );
    emit changed();
  } );
  connect( mSpinTopMargin, qOverload<double>( &QDoubleSpinBox::valueChanged ), this, [=]( double value ) {
    QgsMargins margins = mCallout->margins();
    margins.setTop( value );
    mCallout->setMargins( margins );
    emit changed();
  } );
  connect( mSpinLeftMargin, qOverload<double>( &QDoubleSpinBox::valueChanged ), this, [=]( double value ) {
    QgsMargins margins = mCallout->margins();
    margins.setLeft( value );
    mCallout->setMargins( margins );
    emit changed();
  } );
  connect( mSpinRightMargin, qOverload<double>( &QDoubleSpinBox::valueChanged ), this, [=]( double value ) {
    QgsMargins margins = mCallout->margins();
    margins.setRight( value );
    mCallout->setMargins( margins );
    emit changed();
  } );
  connect( mMarginUnitWidget, &QgsUnitSelectionWidget::changed, this, [=] {
    mCallout->setMarginsUnit( mMarginUnitWidget->unit() );
    emit changed();
  } );

  connect( mWedgeWidthUnitWidget, &QgsUnitSelectionWidget::changed, this, [=] {
    mCallout->setWedgeWidthUnit( mWedgeWidthUnitWidget->unit() );
    mCallout->setWedgeWidthMapUnitScale( mWedgeWidthUnitWidget->getMapUnitScale() );
    emit changed();
  } );
  connect( mWedgeWidthSpin, qOverload<double>( &QDoubleSpinBox::valueChanged ), this, [=]( double value ) {
    mCallout->setWedgeWidth( value );
    emit changed();
  } );

  connect( mCornerRadiusUnitWidget, &QgsUnitSelectionWidget::changed, this, [=] {
    mCallout->setCornerRadiusUnit( mCornerRadiusUnitWidget->unit() );
    mCallout->setCornerRadiusMapUnitScale( mCornerRadiusUnitWidget->getMapUnitScale() );
    emit changed();
  } );
  connect( mCornerRadiusSpin, qOverload<double>( &QDoubleSpinBox::valueChanged ), this, [=]( double value ) {
    mCallout->setCornerRadius( value );
    emit changed();
  } );

  connect( mCalloutBlendComboBox, static_cast<void ( QComboBox::* )( int )>( &QComboBox::currentIndexChanged ), this, &QgsBalloonCalloutWidget::mCalloutBlendComboBox_currentIndexChanged );

  mPlacementDDGroupBox->setVisible( qobject_cast<QgsVectorLayer *>( vl ) );
}

void QgsBalloonCalloutWidget::setCallout( const QgsCallout *callout )
{
  if ( !callout )
    return;

  mCallout.reset( dynamic_cast<QgsBalloonCallout *>( callout->clone() ) );
  if ( !mCallout )
    return;

  mOffsetFromAnchorUnitWidget->blockSignals( true );
  mOffsetFromAnchorUnitWidget->setUnit( mCallout->offsetFromAnchorUnit() );
  mOffsetFromAnchorUnitWidget->setMapUnitScale( mCallout->offsetFromAnchorMapUnitScale() );
  mOffsetFromAnchorUnitWidget->blockSignals( false );
  whileBlocking( mOffsetFromAnchorSpin )->setValue( mCallout->offsetFromAnchor() );

  whileBlocking( mSpinBottomMargin )->setValue( mCallout->margins().bottom() );
  whileBlocking( mSpinTopMargin )->setValue( mCallout->margins().top() );
  whileBlocking( mSpinLeftMargin )->setValue( mCallout->margins().left() );
  whileBlocking( mSpinRightMargin )->setValue( mCallout->margins().right() );
  whileBlocking( mMarginUnitWidget )->setUnit( mCallout->marginsUnit() );

  mWedgeWidthUnitWidget->blockSignals( true );
  mWedgeWidthUnitWidget->setUnit( mCallout->wedgeWidthUnit() );
  mWedgeWidthUnitWidget->setMapUnitScale( mCallout->wedgeWidthMapUnitScale() );
  mWedgeWidthUnitWidget->blockSignals( false );
  whileBlocking( mWedgeWidthSpin )->setValue( mCallout->wedgeWidth() );

  mCornerRadiusUnitWidget->blockSignals( true );
  mCornerRadiusUnitWidget->setUnit( mCallout->cornerRadiusUnit() );
  mCornerRadiusUnitWidget->setMapUnitScale( mCallout->cornerRadiusMapUnitScale() );
  mCornerRadiusUnitWidget->blockSignals( false );
  whileBlocking( mCornerRadiusSpin )->setValue( mCallout->cornerRadius() );

  whileBlocking( mCalloutFillStyleButton )->setSymbol( mCallout->fillSymbol()->clone() );
  if ( QgsMarkerSymbol *marker = mCallout->markerSymbol() )
    whileBlocking( mMarkerSymbolButton )->setSymbol( marker->clone() );
  else
    whileBlocking( mMarkerSymbolButton )->setToNull();

  whileBlocking( mAnchorPointComboBox )->setCurrentIndex( mAnchorPointComboBox->findData( static_cast<int>( callout->anchorPoint() ) ) );

  whileBlocking( mCalloutBlendComboBox )->setBlendMode( mCallout->blendMode() );

  registerDataDefinedButton( mOffsetFromAnchorDDBtn, QgsCallout::Property::OffsetFromAnchor );
  registerDataDefinedButton( mAnchorPointDDBtn, QgsCallout::Property::AnchorPointPosition );
  registerDataDefinedButton( mCalloutBlendModeDDBtn, QgsCallout::Property::BlendMode );

  registerDataDefinedButton( mDestXDDBtn, QgsCallout::Property::DestinationX );
  registerDataDefinedButton( mDestYDDBtn, QgsCallout::Property::DestinationY );
  registerDataDefinedButton( mMarginsDDBtn, QgsCallout::Property::Margins );
  registerDataDefinedButton( mWedgeWidthDDBtn, QgsCallout::Property::WedgeWidth );
  registerDataDefinedButton( mCornerRadiusDDBtn, QgsCallout::Property::CornerRadius );
}

void QgsBalloonCalloutWidget::setGeometryType( Qgis::GeometryType type )
{
  const bool isPolygon = type == Qgis::GeometryType::Polygon;
  mAnchorPointLbl->setEnabled( isPolygon );
  mAnchorPointLbl->setVisible( isPolygon );
  mAnchorPointComboBox->setEnabled( isPolygon );
  mAnchorPointComboBox->setVisible( isPolygon );
  mAnchorPointDDBtn->setEnabled( isPolygon );
  mAnchorPointDDBtn->setVisible( isPolygon );
}

QgsCallout *QgsBalloonCalloutWidget::callout()
{
  return mCallout.get();
}

void QgsBalloonCalloutWidget::offsetFromAnchorUnitWidgetChanged()
{
  mCallout->setOffsetFromAnchorUnit( mOffsetFromAnchorUnitWidget->unit() );
  mCallout->setOffsetFromAnchorMapUnitScale( mOffsetFromAnchorUnitWidget->getMapUnitScale() );
  emit changed();
}

void QgsBalloonCalloutWidget::offsetFromAnchorChanged()
{
  mCallout->setOffsetFromAnchor( mOffsetFromAnchorSpin->value() );
  emit changed();
}

void QgsBalloonCalloutWidget::fillSymbolChanged()
{
  mCallout->setFillSymbol( mCalloutFillStyleButton->clonedSymbol<QgsFillSymbol>() );
  emit changed();
}

void QgsBalloonCalloutWidget::markerSymbolChanged()
{
  mCallout->setMarkerSymbol( mMarkerSymbolButton->isNull() ? nullptr : mMarkerSymbolButton->clonedSymbol<QgsMarkerSymbol>() );
  emit changed();
}

void QgsBalloonCalloutWidget::mAnchorPointComboBox_currentIndexChanged( int index )
{
  mCallout->setAnchorPoint( static_cast<QgsCallout::AnchorPoint>( mAnchorPointComboBox->itemData( index ).toInt() ) );
  emit changed();
}

void QgsBalloonCalloutWidget::mCalloutBlendComboBox_currentIndexChanged( int )
{
  mCallout->setBlendMode( mCalloutBlendComboBox->blendMode() );
  emit changed();
}

///@endcond

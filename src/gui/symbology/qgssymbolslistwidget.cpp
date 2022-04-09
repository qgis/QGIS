/***************************************************************************
 qgssymbolslist.cpp
 ---------------------
 begin                : June 2012
 copyright            : (C) 2012 by Arunmozhi
 email                : aruntheguy at gmail.com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/


#include "qgssymbolslistwidget.h"

#include "qgsstylemanagerdialog.h"
#include "qgsstylesavedialog.h"
#include "qgsstyleitemslistwidget.h"
#include "qgsvectorlayer.h"
#include "qgsnewauxiliarylayerdialog.h"
#include "qgsauxiliarystorage.h"
#include "qgsmarkersymbol.h"
#include "qgslinesymbol.h"
#include "qgsfillsymbol.h"
#include "qgssymbolanimationsettingswidget.h"

#include <QMessageBox>

QgsSymbolsListWidget::QgsSymbolsListWidget( QgsSymbol *symbol, QgsStyle *style, QMenu *menu, QWidget *parent, QgsVectorLayer *layer )
  : QWidget( parent )
  , mSymbol( symbol )
  , mStyle( style )
  , mAdvancedMenu( menu )
  , mLayer( layer )
{
  setupUi( this );
  spinAngle->setClearValue( 0 );

  mStyleItemsListWidget->setStyle( mStyle );
  mStyleItemsListWidget->setEntityType( QgsStyle::SymbolEntity );
  if ( mSymbol )
    mStyleItemsListWidget->setSymbolType( mSymbol->type() );
  mStyleItemsListWidget->setAdvancedMenu( menu );

  mClipFeaturesAction = new QAction( tr( "Clip Features to Canvas Extent" ), this );
  mClipFeaturesAction->setCheckable( true );
  connect( mClipFeaturesAction, &QAction::toggled, this, &QgsSymbolsListWidget::clipFeaturesToggled );
  mStandardizeRingsAction = new QAction( tr( "Force Right-Hand-Rule Orientation" ), this );
  mStandardizeRingsAction->setCheckable( true );
  connect( mStandardizeRingsAction, &QAction::toggled, this, &QgsSymbolsListWidget::forceRHRToggled );
  mAnimationSettingsAction = new QAction( tr( "Animation Settings…" ), this );
  connect( mAnimationSettingsAction, &QAction::triggered, this, &QgsSymbolsListWidget::showAnimationSettings );

  // select correct page in stacked widget
  QgsPropertyOverrideButton *opacityDDBtn = nullptr;
  switch ( symbol->type() )
  {
    case Qgis::SymbolType::Marker:
    {
      stackedWidget->removeWidget( stackedWidget->widget( 2 ) );
      stackedWidget->removeWidget( stackedWidget->widget( 1 ) );
      mSymbolColorButton = btnMarkerColor;
      opacityDDBtn = mMarkerOpacityDDBtn;
      mSymbolOpacityWidget = mMarkerOpacityWidget;
      mSymbolUnitWidget = mMarkerUnitWidget;
      connect( spinAngle, static_cast < void ( QDoubleSpinBox::* )( double ) > ( &QDoubleSpinBox::valueChanged ), this, &QgsSymbolsListWidget::setMarkerAngle );
      connect( spinSize, static_cast < void ( QDoubleSpinBox::* )( double ) > ( &QDoubleSpinBox::valueChanged ), this, &QgsSymbolsListWidget::setMarkerSize );
      registerDataDefinedButton( mSizeDDBtn, QgsSymbolLayer::PropertySize );
      connect( mSizeDDBtn, &QgsPropertyOverrideButton::changed, this, &QgsSymbolsListWidget::updateDataDefinedMarkerSize );
      registerDataDefinedButton( mRotationDDBtn, QgsSymbolLayer::PropertyAngle );
      connect( mRotationDDBtn, &QgsPropertyOverrideButton::changed, this, &QgsSymbolsListWidget::updateDataDefinedMarkerAngle );
      break;
    }

    case Qgis::SymbolType::Line:
    {
      stackedWidget->removeWidget( stackedWidget->widget( 2 ) );
      stackedWidget->removeWidget( stackedWidget->widget( 0 ) );
      mSymbolColorButton = btnLineColor;
      opacityDDBtn = mLineOpacityDDBtn;
      mSymbolOpacityWidget = mLineOpacityWidget;
      mSymbolUnitWidget = mLineUnitWidget;
      connect( spinWidth, static_cast < void ( QDoubleSpinBox::* )( double ) > ( &QDoubleSpinBox::valueChanged ), this, &QgsSymbolsListWidget::setLineWidth );
      registerDataDefinedButton( mWidthDDBtn, QgsSymbolLayer::PropertyStrokeWidth );
      connect( mWidthDDBtn, &QgsPropertyOverrideButton::changed, this, &QgsSymbolsListWidget::updateDataDefinedLineWidth );
      break;
    }

    case Qgis::SymbolType::Fill:
    {
      stackedWidget->removeWidget( stackedWidget->widget( 1 ) );
      stackedWidget->removeWidget( stackedWidget->widget( 0 ) );
      mSymbolColorButton = btnFillColor;
      opacityDDBtn = mFillOpacityDDBtn;
      mSymbolOpacityWidget = mFillOpacityWidget;
      mSymbolUnitWidget = mFillUnitWidget;
      break;
    }

    case Qgis::SymbolType::Hybrid:
      break;
  }

  stackedWidget->setCurrentIndex( 0 );

  mSymbolUnitWidget->setUnits( QgsUnitTypes::RenderUnitList() << QgsUnitTypes::RenderMillimeters << QgsUnitTypes::RenderMetersInMapUnits << QgsUnitTypes::RenderMapUnits << QgsUnitTypes::RenderPixels
                               << QgsUnitTypes::RenderPoints << QgsUnitTypes::RenderInches );

  if ( mSymbol )
  {
    updateSymbolInfo();
  }

  connect( mSymbolUnitWidget, &QgsUnitSelectionWidget::changed, this, &QgsSymbolsListWidget::mSymbolUnitWidget_changed );
  connect( mSymbolColorButton, &QgsColorButton::colorChanged, this, &QgsSymbolsListWidget::setSymbolColor );

  registerSymbolDataDefinedButton( opacityDDBtn, QgsSymbol::PropertyOpacity );

  connect( this, &QgsSymbolsListWidget::changed, this, &QgsSymbolsListWidget::updateAssistantSymbol );
  updateAssistantSymbol();

  mSymbolColorButton->setAllowOpacity( true );
  mSymbolColorButton->setColorDialogTitle( tr( "Select Color" ) );
  mSymbolColorButton->setContext( QStringLiteral( "symbology" ) );

  connect( mSymbolOpacityWidget, &QgsOpacityWidget::opacityChanged, this, &QgsSymbolsListWidget::opacityChanged );

  connect( mStyleItemsListWidget, &QgsStyleItemsListWidget::selectionChanged, this, &QgsSymbolsListWidget::setSymbolFromStyle );
  connect( mStyleItemsListWidget, &QgsStyleItemsListWidget::saveEntity, this, &QgsSymbolsListWidget::saveSymbol );
}

QgsSymbolsListWidget::~QgsSymbolsListWidget()
{
  // This action was added to the menu by this widget, clean it up
  // The menu can be passed in the constructor, so may live longer than this widget
  mStyleItemsListWidget->advancedMenu()->removeAction( mClipFeaturesAction );
  mStyleItemsListWidget->advancedMenu()->removeAction( mStandardizeRingsAction );
  mStyleItemsListWidget->advancedMenu()->removeAction( mAnimationSettingsAction );
}

void QgsSymbolsListWidget::registerDataDefinedButton( QgsPropertyOverrideButton *button, QgsSymbolLayer::Property key )
{
  button->setProperty( "propertyKey", key );
  button->registerExpressionContextGenerator( this );

  connect( button, &QgsPropertyOverrideButton::createAuxiliaryField, this, &QgsSymbolsListWidget::createAuxiliaryField );
}

void QgsSymbolsListWidget::createAuxiliaryField()
{
  // try to create an auxiliary layer if not yet created
  if ( !mLayer->auxiliaryLayer() )
  {
    QgsNewAuxiliaryLayerDialog dlg( mLayer, this );
    dlg.exec();
  }

  // return if still not exists
  if ( !mLayer->auxiliaryLayer() )
    return;

  QgsPropertyOverrideButton *button = qobject_cast<QgsPropertyOverrideButton *>( sender() );
  const QgsSymbolLayer::Property key = static_cast<  QgsSymbolLayer::Property >( button->propertyKey() );
  const QgsPropertyDefinition def = QgsSymbolLayer::propertyDefinitions()[key];

  // create property in auxiliary storage if necessary
  if ( !mLayer->auxiliaryLayer()->exists( def ) )
    mLayer->auxiliaryLayer()->addAuxiliaryField( def );

  // update property with join field name from auxiliary storage
  QgsProperty property = button->toProperty();
  property.setField( QgsAuxiliaryLayer::nameFromProperty( def, true ) );
  property.setActive( true );
  button->updateFieldLists();
  button->setToProperty( property );

  QgsMarkerSymbol *markerSymbol = static_cast<QgsMarkerSymbol *>( mSymbol );
  QgsLineSymbol *lineSymbol = static_cast<QgsLineSymbol *>( mSymbol );
  switch ( key )
  {
    case QgsSymbolLayer::PropertyAngle:
      if ( markerSymbol )
        markerSymbol->setDataDefinedAngle( button->toProperty() );
      break;

    case QgsSymbolLayer::PropertySize:
      if ( markerSymbol )
      {
        markerSymbol->setDataDefinedSize( button->toProperty() );
        markerSymbol->setScaleMethod( Qgis::ScaleMethod::ScaleDiameter );
      }
      break;

    case QgsSymbolLayer::PropertyStrokeWidth:
      if ( lineSymbol )
        lineSymbol->setDataDefinedWidth( button->toProperty() );
      break;

    default:
      break;
  }

  emit changed();
}

void QgsSymbolsListWidget::createSymbolAuxiliaryField()
{
  // try to create an auxiliary layer if not yet created
  if ( !mLayer->auxiliaryLayer() )
  {
    QgsNewAuxiliaryLayerDialog dlg( mLayer, this );
    dlg.exec();
  }

  // return if still not exists
  if ( !mLayer->auxiliaryLayer() )
    return;

  QgsPropertyOverrideButton *button = qobject_cast<QgsPropertyOverrideButton *>( sender() );
  const QgsSymbol::Property key = static_cast<  QgsSymbol::Property >( button->propertyKey() );
  const QgsPropertyDefinition def = QgsSymbol::propertyDefinitions()[key];

  // create property in auxiliary storage if necessary
  if ( !mLayer->auxiliaryLayer()->exists( def ) )
    mLayer->auxiliaryLayer()->addAuxiliaryField( def );

  // update property with join field name from auxiliary storage
  QgsProperty property = button->toProperty();
  property.setField( QgsAuxiliaryLayer::nameFromProperty( def, true ) );
  property.setActive( true );
  button->updateFieldLists();
  button->setToProperty( property );

  mSymbol->setDataDefinedProperty( key, button->toProperty() );

  emit changed();
}

void QgsSymbolsListWidget::setContext( const QgsSymbolWidgetContext &context )
{
  mContext = context;
  const auto unitSelectionWidgets { findChildren<QgsUnitSelectionWidget *>() };
  for ( QgsUnitSelectionWidget *unitWidget : unitSelectionWidgets )
  {
    unitWidget->setMapCanvas( mContext.mapCanvas() );
  }
}

QgsSymbolWidgetContext QgsSymbolsListWidget::context() const
{
  return mContext;
}

void QgsSymbolsListWidget::forceRHRToggled( bool checked )
{
  if ( !mSymbol )
    return;

  mSymbol->setForceRHR( checked );
  emit changed();
}

void QgsSymbolsListWidget::showAnimationSettings()
{
  QgsPanelWidget *panel = QgsPanelWidget::findParentPanel( this );
  if ( panel && panel->dockMode() )
  {
    QgsSymbolAnimationSettingsWidget *widget = new QgsSymbolAnimationSettingsWidget( panel );
    widget->setPanelTitle( tr( "Animation Settings" ) );
    widget->setAnimationSettings( mSymbol->animationSettings() );
    connect( widget, &QgsPanelWidget::widgetChanged, this, [ this, widget ]()
    {
      mSymbol->setAnimationSettings( widget->animationSettings() );
      emit changed();
    } );
    panel->openPanel( widget );
    return;
  }

  QgsSymbolAnimationSettingsDialog d( this );
  d.setAnimationSettings( mSymbol->animationSettings() );
  if ( d.exec() == QDialog::Accepted )
  {
    mSymbol->setAnimationSettings( d.animationSettings() );
    emit changed();
  }
}

void QgsSymbolsListWidget::saveSymbol()
{
  if ( !mStyle )
    return;

  QgsStyleSaveDialog saveDlg( this );
  saveDlg.setDefaultTags( mStyleItemsListWidget->currentTagFilter() );
  if ( !saveDlg.exec() )
    return;

  if ( saveDlg.name().isEmpty() )
    return;

  // check if there is no symbol with same name
  if ( mStyle->symbolNames().contains( saveDlg.name() ) )
  {
    const int res = QMessageBox::warning( this, tr( "Save Symbol" ),
                                          tr( "Symbol with name '%1' already exists. Overwrite?" )
                                          .arg( saveDlg.name() ),
                                          QMessageBox::Yes | QMessageBox::No );
    if ( res != QMessageBox::Yes )
    {
      return;
    }
    mStyle->removeSymbol( saveDlg.name() );
  }

  const QStringList symbolTags = saveDlg.tags().split( ',' );

  // add new symbol to style and re-populate the list
  QgsSymbol *newSymbol = mSymbol->clone();
  mStyle->addSymbol( saveDlg.name(), newSymbol );

  // make sure the symbol is stored
  mStyle->saveSymbol( saveDlg.name(), newSymbol, saveDlg.isFavorite(), symbolTags );
}

void QgsSymbolsListWidget::updateSymbolDataDefinedProperty()
{
  if ( !mSymbol )
    return;

  QgsPropertyOverrideButton *button = qobject_cast<QgsPropertyOverrideButton *>( sender() );
  const QgsSymbol::Property key = static_cast<  QgsSymbol::Property >( button->propertyKey() );
  mSymbol->setDataDefinedProperty( key, button->toProperty() );
  emit changed();
}

void QgsSymbolsListWidget::registerSymbolDataDefinedButton( QgsPropertyOverrideButton *button, QgsSymbol::Property key )
{
  button->init( key, mSymbol ? mSymbol->dataDefinedProperties() : QgsPropertyCollection(), QgsSymbol::propertyDefinitions(), mLayer, true );
  connect( button, &QgsPropertyOverrideButton::changed, this, &QgsSymbolsListWidget::updateSymbolDataDefinedProperty );
  connect( button, &QgsPropertyOverrideButton::createAuxiliaryField, this, &QgsSymbolsListWidget::createSymbolAuxiliaryField );

  button->registerExpressionContextGenerator( this );
}

void QgsSymbolsListWidget::clipFeaturesToggled( bool checked )
{
  if ( !mSymbol )
    return;

  mSymbol->setClipFeaturesToExtent( checked );
  emit changed();
}

void QgsSymbolsListWidget::setSymbolColor( const QColor &color )
{
  mSymbol->setColor( color );
  emit changed();
}

void QgsSymbolsListWidget::setMarkerAngle( double angle )
{
  QgsMarkerSymbol *markerSymbol = static_cast<QgsMarkerSymbol *>( mSymbol );
  if ( markerSymbol->angle() == angle )
    return;
  markerSymbol->setAngle( angle );
  emit changed();
}

void QgsSymbolsListWidget::updateDataDefinedMarkerAngle()
{
  QgsMarkerSymbol *markerSymbol = static_cast<QgsMarkerSymbol *>( mSymbol );
  const QgsProperty dd( mRotationDDBtn->toProperty() );

  spinAngle->setEnabled( !mRotationDDBtn->isActive() );

  const QgsProperty symbolDD( markerSymbol->dataDefinedAngle() );

  if ( // shall we remove datadefined expressions for layers ?
    ( !symbolDD && !dd )
    // shall we set the "en masse" expression for properties ?
    || dd )
  {
    markerSymbol->setDataDefinedAngle( dd );
    emit changed();
  }
}

void QgsSymbolsListWidget::setMarkerSize( double size )
{
  QgsMarkerSymbol *markerSymbol = static_cast<QgsMarkerSymbol *>( mSymbol );
  if ( markerSymbol->size() == size )
    return;
  markerSymbol->setSize( size );
  emit changed();
}

void QgsSymbolsListWidget::updateDataDefinedMarkerSize()
{
  QgsMarkerSymbol *markerSymbol = static_cast<QgsMarkerSymbol *>( mSymbol );
  const QgsProperty dd( mSizeDDBtn->toProperty() );

  spinSize->setEnabled( !mSizeDDBtn->isActive() );

  const QgsProperty symbolDD( markerSymbol->dataDefinedSize() );

  if ( // shall we remove datadefined expressions for layers ?
    ( !symbolDD && !dd )
    // shall we set the "en masse" expression for properties ?
    || dd )
  {
    markerSymbol->setDataDefinedSize( dd );
    markerSymbol->setScaleMethod( Qgis::ScaleMethod::ScaleDiameter );
    emit changed();
  }
}

void QgsSymbolsListWidget::setLineWidth( double width )
{
  QgsLineSymbol *lineSymbol = static_cast<QgsLineSymbol *>( mSymbol );
  if ( lineSymbol->width() == width )
    return;
  lineSymbol->setWidth( width );
  emit changed();
}

void QgsSymbolsListWidget::updateDataDefinedLineWidth()
{
  QgsLineSymbol *lineSymbol = static_cast<QgsLineSymbol *>( mSymbol );
  const QgsProperty dd( mWidthDDBtn->toProperty() );

  spinWidth->setEnabled( !mWidthDDBtn->isActive() );

  const QgsProperty symbolDD( lineSymbol->dataDefinedWidth() );

  if ( // shall we remove datadefined expressions for layers ?
    ( !symbolDD && !dd )
    // shall we set the "en masse" expression for properties ?
    || dd )
  {
    lineSymbol->setDataDefinedWidth( dd );
    emit changed();
  }
}

void QgsSymbolsListWidget::updateAssistantSymbol()
{
  mAssistantSymbol.reset( mSymbol->clone() );
  if ( mSymbol->type() == Qgis::SymbolType::Marker )
    mSizeDDBtn->setSymbol( mAssistantSymbol );
  else if ( mSymbol->type() == Qgis::SymbolType::Line && mLayer )
    mWidthDDBtn->setSymbol( mAssistantSymbol );
}

void QgsSymbolsListWidget::mSymbolUnitWidget_changed()
{
  if ( mSymbol )
  {

    mSymbol->setOutputUnit( mSymbolUnitWidget->unit() );
    mSymbol->setMapUnitScale( mSymbolUnitWidget->getMapUnitScale() );

    emit changed();
  }
}

void QgsSymbolsListWidget::opacityChanged( double opacity )
{
  if ( mSymbol )
  {
    mSymbol->setOpacity( opacity );
    emit changed();
  }
}

void QgsSymbolsListWidget::updateSymbolColor()
{
  mSymbolColorButton->blockSignals( true );
  mSymbolColorButton->setColor( mSymbol->color() );
  mSymbolColorButton->blockSignals( false );
}

QgsExpressionContext QgsSymbolsListWidget::createExpressionContext() const
{
  if ( auto *lExpressionContext = mContext.expressionContext() )
    return QgsExpressionContext( *lExpressionContext );

  //otherwise create a default symbol context
  QgsExpressionContext expContext( mContext.globalProjectAtlasMapLayerScopes( layer() ) );

  // additional scopes
  const auto constAdditionalExpressionContextScopes = mContext.additionalExpressionContextScopes();
  for ( const QgsExpressionContextScope &scope : constAdditionalExpressionContextScopes )
  {
    expContext.appendScope( new QgsExpressionContextScope( scope ) );
  }

  expContext.setHighlightedVariables( QStringList() << QgsExpressionContext::EXPR_ORIGINAL_VALUE << QgsExpressionContext::EXPR_SYMBOL_COLOR
                                      << QgsExpressionContext::EXPR_GEOMETRY_PART_COUNT << QgsExpressionContext::EXPR_GEOMETRY_PART_NUM
                                      << QgsExpressionContext::EXPR_GEOMETRY_RING_NUM
                                      << QgsExpressionContext::EXPR_GEOMETRY_POINT_COUNT << QgsExpressionContext::EXPR_GEOMETRY_POINT_NUM
                                      << QgsExpressionContext::EXPR_CLUSTER_COLOR << QgsExpressionContext::EXPR_CLUSTER_SIZE
                                      << QStringLiteral( "symbol_layer_count" ) << QStringLiteral( "symbol_layer_index" )
                                      << QStringLiteral( "symbol_frame" ) );

  return expContext;
}

void QgsSymbolsListWidget::updateSymbolInfo()
{
  updateSymbolColor();

  const auto overrideButtons {findChildren< QgsPropertyOverrideButton * >()};
  for ( QgsPropertyOverrideButton *button : overrideButtons )
  {
    button->registerExpressionContextGenerator( this );
  }

  if ( mSymbol->type() == Qgis::SymbolType::Marker )
  {
    QgsMarkerSymbol *markerSymbol = static_cast<QgsMarkerSymbol *>( mSymbol );
    spinSize->setValue( markerSymbol->size() );
    spinAngle->setValue( markerSymbol->angle() );

    if ( mLayer )
    {
      const QgsProperty ddSize( markerSymbol->dataDefinedSize() );
      mSizeDDBtn->init( QgsSymbolLayer::PropertySize, ddSize, QgsSymbolLayer::propertyDefinitions(), mLayer, true );
      spinSize->setEnabled( !mSizeDDBtn->isActive() );
      const QgsProperty ddAngle( markerSymbol->dataDefinedAngle() );
      mRotationDDBtn->init( QgsSymbolLayer::PropertyAngle, ddAngle, QgsSymbolLayer::propertyDefinitions(), mLayer, true );
      spinAngle->setEnabled( !mRotationDDBtn->isActive() );
    }
    else
    {
      mSizeDDBtn->setEnabled( false );
      mRotationDDBtn->setEnabled( false );
    }
  }
  else if ( mSymbol->type() == Qgis::SymbolType::Line )
  {
    QgsLineSymbol *lineSymbol = static_cast<QgsLineSymbol *>( mSymbol );
    spinWidth->setValue( lineSymbol->width() );

    if ( mLayer )
    {
      const QgsProperty dd( lineSymbol->dataDefinedWidth() );
      mWidthDDBtn->init( QgsSymbolLayer::PropertyStrokeWidth, dd, QgsSymbolLayer::propertyDefinitions(), mLayer, true );
      spinWidth->setEnabled( !mWidthDDBtn->isActive() );
    }
    else
    {
      mWidthDDBtn->setEnabled( false );
    }
  }

  mSymbolUnitWidget->blockSignals( true );
  mSymbolUnitWidget->setUnit( mSymbol->outputUnit() );
  mSymbolUnitWidget->setMapUnitScale( mSymbol->mapUnitScale() );
  mSymbolUnitWidget->blockSignals( false );

  mSymbolOpacityWidget->setOpacity( mSymbol->opacity() );

  // Clean up previous advanced symbol actions
  const QList<QAction *> actionList( mStyleItemsListWidget->advancedMenu()->actions() );
  for ( const auto &action : actionList )
  {
    if ( mClipFeaturesAction->text() == action->text() )
    {
      mStyleItemsListWidget->advancedMenu()->removeAction( action );
    }
    else if ( mStandardizeRingsAction->text() == action->text() )
    {
      mStyleItemsListWidget->advancedMenu()->removeAction( action );
    }
    else if ( mAnimationSettingsAction->text() == action->text() )
    {
      mStyleItemsListWidget->advancedMenu()->removeAction( action );
    }
  }

  if ( mSymbol->type() == Qgis::SymbolType::Line || mSymbol->type() == Qgis::SymbolType::Fill )
  {
    //add clip features option for line or fill symbols
    mStyleItemsListWidget->advancedMenu()->addAction( mClipFeaturesAction );
  }
  if ( mSymbol->type() == Qgis::SymbolType::Fill )
  {
    mStyleItemsListWidget->advancedMenu()->addAction( mStandardizeRingsAction );
  }
  mStyleItemsListWidget->advancedMenu()->addAction( mAnimationSettingsAction );

  mStyleItemsListWidget->showAdvancedButton( mAdvancedMenu || !mStyleItemsListWidget->advancedMenu()->isEmpty() );

  whileBlocking( mClipFeaturesAction )->setChecked( mSymbol->clipFeaturesToExtent() );
  whileBlocking( mStandardizeRingsAction )->setChecked( mSymbol->forceRHR() );
}

void QgsSymbolsListWidget::setSymbolFromStyle( const QString &name, QgsStyle::StyleEntity )
{
  // get new instance of symbol from style
  std::unique_ptr< QgsSymbol > s( mStyle->symbol( name ) );
  if ( !s )
    return;

  // remove all symbol layers from original symbolgroupsCombo
  while ( mSymbol->symbolLayerCount() )
    mSymbol->deleteSymbolLayer( 0 );
  // move all symbol layers to our symbol
  while ( s->symbolLayerCount() )
  {
    QgsSymbolLayer *sl = s->takeSymbolLayer( 0 );
    mSymbol->appendSymbolLayer( sl );
  }
  mSymbol->setOpacity( s->opacity() );
  mSymbol->setFlags( s->flags() );

  updateSymbolInfo();
  emit changed();
}

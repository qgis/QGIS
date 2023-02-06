/***************************************************************************
    qgsvectorrelevationpropertieswidget.cpp
    ---------------------
    begin                : February 2022
    copyright            : (C) 2022 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsvectorelevationpropertieswidget.h"
#include "qgsstyle.h"
#include "qgsapplication.h"
#include "qgsmaplayer.h"
#include "qgsvectorlayer.h"
#include "qgsvectorlayerelevationproperties.h"
#include "qgslinesymbol.h"
#include "qgsmarkersymbol.h"
#include "qgsfillsymbol.h"
#include "qgsexpressioncontextutils.h"

QgsVectorElevationPropertiesWidget::QgsVectorElevationPropertiesWidget( QgsVectorLayer *layer, QgsMapCanvas *canvas, QWidget *parent )
  : QgsMapLayerConfigWidget( layer, canvas, parent )
{
  setupUi( this );
  setObjectName( QStringLiteral( "mOptsPage_Elevation" ) );

  mOffsetZSpinBox->setClearValue( 0 );
  mScaleZSpinBox->setClearValue( 1 );
  mExtrusionSpinBox->setClearValue( 0 );

  mLineStyleButton->setSymbolType( Qgis::SymbolType::Line );
  mFillStyleButton->setSymbolType( Qgis::SymbolType::Fill );
  mMarkerStyleButton->setSymbolType( Qgis::SymbolType::Marker );
  mSurfaceLineStyleButton->setSymbolType( Qgis::SymbolType::Line );
  mSurfaceFillStyleButton->setSymbolType( Qgis::SymbolType::Fill );
  mSurfaceMarkerStyleButton->setSymbolType( Qgis::SymbolType::Marker );

  mComboClamping->addItem( tr( "Clamped to Terrain" ), static_cast< int >( Qgis::AltitudeClamping::Terrain ) );
  mComboClamping->addItem( tr( "Relative to Terrain" ), static_cast< int >( Qgis::AltitudeClamping::Relative ) );
  mComboClamping->addItem( tr( "Absolute" ), static_cast< int >( Qgis::AltitudeClamping::Absolute ) );

  mComboBinding->addItem( tr( "Vertex" ), static_cast< int >( Qgis::AltitudeBinding::Vertex ) );
  mComboBinding->addItem( tr( "Centroid" ), static_cast< int >( Qgis::AltitudeBinding::Centroid ) );

  mTypeComboBox->addItem( tr( "Individual Features" ), static_cast< int >( Qgis::VectorProfileType::IndividualFeatures ) );
  mTypeComboBox->addItem( tr( "Continuous Surface (e.g. Contours)" ), static_cast< int >( Qgis::VectorProfileType::ContinuousSurface ) );

  mStyleComboBox->addItem( QgsApplication::getThemeIcon( QStringLiteral( "mIconSurfaceElevationLine.svg" ) ), tr( "Line" ), static_cast< int >( Qgis::ProfileSurfaceSymbology::Line ) );
  mStyleComboBox->addItem( QgsApplication::getThemeIcon( QStringLiteral( "mIconSurfaceElevationFillBelow.svg" ) ), tr( "Fill Below" ), static_cast< int >( Qgis::ProfileSurfaceSymbology::FillBelow ) );

  initializeDataDefinedButton( mOffsetDDBtn, QgsMapLayerElevationProperties::ZOffset );
  initializeDataDefinedButton( mExtrusionDDBtn, QgsMapLayerElevationProperties::ExtrusionHeight );

  syncToLayer( layer );

  connect( mOffsetZSpinBox, qOverload<double >( &QDoubleSpinBox::valueChanged ), this, &QgsVectorElevationPropertiesWidget::onChanged );
  connect( mScaleZSpinBox, qOverload<double >( &QDoubleSpinBox::valueChanged ), this, &QgsVectorElevationPropertiesWidget::onChanged );
  connect( mExtrusionSpinBox, qOverload<double >( &QDoubleSpinBox::valueChanged ), this, &QgsVectorElevationPropertiesWidget::onChanged );
  connect( mExtrusionGroupBox, &QGroupBox::toggled, this, &QgsVectorElevationPropertiesWidget::onChanged );
  connect( mComboClamping, qOverload<int>( &QComboBox::currentIndexChanged ), this, &QgsVectorElevationPropertiesWidget::onChanged );
  connect( mComboBinding, qOverload<int>( &QComboBox::currentIndexChanged ), this, &QgsVectorElevationPropertiesWidget::onChanged );
  connect( mComboClamping, qOverload<int>( &QComboBox::currentIndexChanged ), this, &QgsVectorElevationPropertiesWidget::clampingChanged );
  connect( mComboBinding, qOverload<int>( &QComboBox::currentIndexChanged ), this, &QgsVectorElevationPropertiesWidget::bindingChanged );
  connect( mTypeComboBox, qOverload<int>( &QComboBox::currentIndexChanged ), this, [ = ]
  {
    switch ( static_cast< Qgis::VectorProfileType >( mTypeComboBox->currentData().toInt() ) )
    {
      case Qgis::VectorProfileType::IndividualFeatures:
        mInterpretationStackedWidget->setCurrentWidget( mPageIndividualFeatures );
        break;
      case Qgis::VectorProfileType::ContinuousSurface:
        mInterpretationStackedWidget->setCurrentWidget( mPageContinuousSurface );
        break;
    }
    onChanged();
  } );

  connect( mStyleComboBox, qOverload< int >( &QComboBox::currentIndexChanged ), this, [ = ]
  {
    switch ( static_cast< Qgis::ProfileSurfaceSymbology >( mStyleComboBox->currentData().toInt() ) )
    {
      case Qgis::ProfileSurfaceSymbology::Line:
        mSymbologyStackedWidget->setCurrentWidget( mPageLine );
        break;
      case Qgis::ProfileSurfaceSymbology::FillBelow:
        mSymbologyStackedWidget->setCurrentWidget( mPageFill );
        break;
    }

    onChanged();
  } );

  connect( mCheckRespectLayerSymbology, &QCheckBox::toggled, this, &QgsVectorElevationPropertiesWidget::onChanged );
  connect( mCheckBoxShowMarkersAtSampledPoints, &QCheckBox::toggled, this, &QgsVectorElevationPropertiesWidget::onChanged );
  connect( mFillStyleButton, &QgsSymbolButton::changed, this, &QgsVectorElevationPropertiesWidget::onChanged );
  connect( mLineStyleButton, &QgsSymbolButton::changed, this, &QgsVectorElevationPropertiesWidget::onChanged );
  connect( mMarkerStyleButton, &QgsSymbolButton::changed, this, &QgsVectorElevationPropertiesWidget::onChanged );
  connect( mSurfaceFillStyleButton, &QgsSymbolButton::changed, this, &QgsVectorElevationPropertiesWidget::onChanged );
  connect( mSurfaceLineStyleButton, &QgsSymbolButton::changed, this, &QgsVectorElevationPropertiesWidget::onChanged );
  connect( mSurfaceMarkerStyleButton, &QgsSymbolButton::changed, this, &QgsVectorElevationPropertiesWidget::onChanged );
  connect( mExtrusionGroupBox, &QGroupBox::toggled, this, &QgsVectorElevationPropertiesWidget::toggleSymbolWidgets );

  setProperty( "helpPage", QStringLiteral( "working_with_vector/vector_properties.html#elevation-properties" ) );
}

void QgsVectorElevationPropertiesWidget::syncToLayer( QgsMapLayer *layer )
{
  mLayer = qobject_cast< QgsVectorLayer * >( layer );
  if ( !mLayer )
    return;

  if ( !QgsWkbTypes::hasZ( mLayer->wkbType() ) )
  {
    const int clampingIndex = mComboClamping->findData( static_cast< int >( Qgis::AltitudeClamping::Relative ) );
    if ( clampingIndex >= 0 )
      mComboClamping->removeItem( clampingIndex );
  }

  mBlockUpdates = true;
  const QgsVectorLayerElevationProperties *props = qgis::down_cast< const QgsVectorLayerElevationProperties * >( mLayer->elevationProperties() );

  mComboClamping->setCurrentIndex( mComboClamping->findData( static_cast< int >( props->clamping() ) ) );
  if ( mComboClamping->currentIndex() == -1 )
    mComboClamping->setCurrentIndex( 0 );

  mComboBinding->setCurrentIndex( mComboBinding->findData( static_cast< int >( props->binding() ) ) );
  mOffsetZSpinBox->setValue( props->zOffset() );
  mScaleZSpinBox->setValue( props->zScale() );
  mExtrusionGroupBox->setChecked( props->extrusionEnabled() );
  mExtrusionSpinBox->setValue( props->extrusionHeight() );
  mTypeComboBox->setCurrentIndex( mTypeComboBox->findData( static_cast< int >( props->type() ) ) );
  switch ( props->type() )
  {
    case Qgis::VectorProfileType::IndividualFeatures:
      mInterpretationStackedWidget->setCurrentWidget( mPageIndividualFeatures );
      break;
    case Qgis::VectorProfileType::ContinuousSurface:
      mInterpretationStackedWidget->setCurrentWidget( mPageContinuousSurface );
      break;
  }
  mStyleComboBox->setCurrentIndex( mStyleComboBox->findData( static_cast <int >( props->profileSymbology() ) ) );
  switch ( props->profileSymbology() )
  {
    case Qgis::ProfileSurfaceSymbology::Line:
      mSymbologyStackedWidget->setCurrentWidget( mPageLine );
      break;
    case Qgis::ProfileSurfaceSymbology::FillBelow:
      mSymbologyStackedWidget->setCurrentWidget( mPageFill );
      break;
  }

  mCheckRespectLayerSymbology->setChecked( props->respectLayerSymbology() );
  mCheckBoxShowMarkersAtSampledPoints->setChecked( props->showMarkerSymbolInSurfacePlots() );
  mLineStyleButton->setSymbol( props->profileLineSymbol()->clone() );
  mFillStyleButton->setSymbol( props->profileFillSymbol()->clone() );
  mSurfaceLineStyleButton->setSymbol( props->profileLineSymbol()->clone() );
  mSurfaceFillStyleButton->setSymbol( props->profileFillSymbol()->clone() );
  mMarkerStyleButton->setSymbol( props->profileMarkerSymbol()->clone() );
  mSurfaceMarkerStyleButton->setSymbol( props->profileMarkerSymbol()->clone() );

  mPropertyCollection = props->dataDefinedProperties();
  updateDataDefinedButtons();

  mLineStyleButton->setLayer( mLayer );
  mFillStyleButton->setLayer( mLayer );
  mMarkerStyleButton->setLayer( mLayer );
  mSurfaceLineStyleButton->setLayer( mLayer );
  mSurfaceFillStyleButton->setLayer( mLayer );
  mSurfaceMarkerStyleButton->setLayer( mLayer );

  mLineStyleButton->registerExpressionContextGenerator( this );
  mFillStyleButton->registerExpressionContextGenerator( this );
  mMarkerStyleButton->registerExpressionContextGenerator( this );
  mSurfaceLineStyleButton->registerExpressionContextGenerator( this );
  mSurfaceFillStyleButton->registerExpressionContextGenerator( this );
  mSurfaceMarkerStyleButton->registerExpressionContextGenerator( this );

  toggleSymbolWidgets();

  mBlockUpdates = false;

  clampingChanged();
  bindingChanged();
}

QgsExpressionContext QgsVectorElevationPropertiesWidget::createExpressionContext() const
{
  QgsExpressionContext context;
  context.appendScopes( QgsExpressionContextUtils::globalProjectLayerScopes( mLayer ) );
  return context;
}

void QgsVectorElevationPropertiesWidget::apply()
{
  if ( !mLayer )
    return;

  QgsVectorLayerElevationProperties *props = qgis::down_cast< QgsVectorLayerElevationProperties * >( mLayer->elevationProperties() );

  props->setZOffset( mOffsetZSpinBox->value() );
  props->setZScale( mScaleZSpinBox->value() );
  props->setType( static_cast< Qgis::VectorProfileType >( mTypeComboBox->currentData().toInt() ) );
  props->setClamping( static_cast< Qgis::AltitudeClamping >( mComboClamping->currentData().toInt() ) );
  props->setBinding( static_cast< Qgis::AltitudeBinding >( mComboBinding->currentData().toInt() ) );
  props->setExtrusionEnabled( mExtrusionGroupBox->isChecked() );
  props->setExtrusionHeight( mExtrusionSpinBox->value() );

  props->setRespectLayerSymbology( mCheckRespectLayerSymbology->isChecked() );
  props->setShowMarkerSymbolInSurfacePlots( mCheckBoxShowMarkersAtSampledPoints->isChecked() );

  props->setProfileSymbology( static_cast< Qgis::ProfileSurfaceSymbology >( mStyleComboBox->currentData().toInt() ) );
  switch ( props->type() )
  {
    case Qgis::VectorProfileType::IndividualFeatures:
      props->setProfileLineSymbol( mLineStyleButton->clonedSymbol< QgsLineSymbol >() );
      props->setProfileFillSymbol( mFillStyleButton->clonedSymbol< QgsFillSymbol >() );
      props->setProfileMarkerSymbol( mMarkerStyleButton->clonedSymbol< QgsMarkerSymbol >() );
      break;
    case Qgis::VectorProfileType::ContinuousSurface:
      props->setProfileLineSymbol( mSurfaceLineStyleButton->clonedSymbol< QgsLineSymbol >() );
      props->setProfileFillSymbol( mSurfaceFillStyleButton->clonedSymbol< QgsFillSymbol >() );
      props->setProfileMarkerSymbol( mSurfaceMarkerStyleButton->clonedSymbol< QgsMarkerSymbol >() );
      break;
  }

  props->setDataDefinedProperties( mPropertyCollection );

  mLayer->trigger3DUpdate();
}

void QgsVectorElevationPropertiesWidget::onChanged()
{
  if ( !mBlockUpdates )
    emit widgetChanged();
}

void QgsVectorElevationPropertiesWidget::clampingChanged()
{
  bool enableScale = true;
  bool enableBinding = !mLayer || mLayer->geometryType() != QgsWkbTypes::PointGeometry;
  switch ( static_cast< Qgis::AltitudeClamping >( mComboClamping->currentData().toInt() ) )
  {
    case Qgis::AltitudeClamping::Absolute:
      mLabelClampingExplanation->setText(
        QStringLiteral( "<p><b>%1</b></p><p>%2</p>" ).arg(
          tr( "Elevation will be taken directly from features." ),
          tr( "Z values from the features will be used for elevation, and the terrain height will be ignored." ) )
      );
      enableBinding = false; // not used in absolute mode

      if ( QgsWkbTypes::hasZ( mLayer->wkbType() ) )
      {
        mOffsetLabel->setText( tr( "Offset" ) );
      }
      else
      {
        mOffsetLabel->setText( tr( "Base height" ) );
      }
      break;
    case Qgis::AltitudeClamping::Relative:
      mOffsetLabel->setText( tr( "Offset" ) );
      mLabelClampingExplanation->setText(
        QStringLiteral( "<p><b>%1</b></p><p>%2</p>" ).arg(
          tr( "Elevation is relative to terrain height." ),
          tr( "Any z values present in the features will be added to the terrain height." ) )
      );
      break;
    case Qgis::AltitudeClamping::Terrain:
      mOffsetLabel->setText( tr( "Offset" ) );
      mLabelClampingExplanation->setText(
        QStringLiteral( "<p><b>%1</b></p><p>%2</p>" ).arg(
          tr( "Feature elevation will be taken directly from the terrain height." ),
          tr( "Any existing z values present in the features will be ignored." ) )
      );
      enableScale = false; // not used in terrain mode
      break;
  }
  mLabelScale->setVisible( enableScale );
  mScaleZSpinBox->setVisible( enableScale );
  mBindingGroupBox->setVisible( enableBinding );
  mLabelBindingExplanation->setVisible( enableBinding );
}

void QgsVectorElevationPropertiesWidget::bindingChanged()
{
  switch ( static_cast< Qgis::AltitudeBinding >( mComboBinding->currentData().toInt() ) )
  {
    case Qgis::AltitudeBinding::Vertex:
      mLabelBindingExplanation->setText(
        QStringLiteral( "<p><b>%1</b></p><p>%2</p>" ).arg(
          tr( "Feature elevation is relative to the terrain height at every vertex." ),
          tr( "The terrain will be sampled at every individual vertex before being added to the vertex's z value." )
        )
      );
      break;
    case Qgis::AltitudeBinding::Centroid:
      mLabelBindingExplanation->setText(
        QStringLiteral( "<p><b>%1</b></p><p>%2</p>" ).arg(
          tr( "Feature elevation is relative to the terrain height at feature's centroid only." ),
          tr( "The terrain will be sampled once at the feature's centroid, with the centroid height being added to each vertex's z value." )
        )
      );
      break;

  }
}

void QgsVectorElevationPropertiesWidget::toggleSymbolWidgets()
{
  // see QgsVectorLayerElevationProperties documentation on why certain symbols are
  // enabled here
  switch ( mLayer->geometryType() )
  {
    case QgsWkbTypes::PointGeometry:
      mLineStyleButton->setEnabled( mExtrusionGroupBox->isChecked() );
      mMarkerStyleButton->setEnabled( true );
      mFillStyleButton->setEnabled( false );
      break;

    case QgsWkbTypes::LineGeometry:
      mLineStyleButton->setEnabled( mExtrusionGroupBox->isChecked() );
      mMarkerStyleButton->setEnabled( true );
      mFillStyleButton->setEnabled( false );
      break;

    case QgsWkbTypes::PolygonGeometry:
      mLineStyleButton->setEnabled( true );
      mMarkerStyleButton->setEnabled( true );
      mFillStyleButton->setEnabled( mExtrusionGroupBox->isChecked() );
      break;

    case QgsWkbTypes::UnknownGeometry:
    case QgsWkbTypes::NullGeometry:
      mLineStyleButton->setEnabled( false );
      mMarkerStyleButton->setEnabled( false );
      mFillStyleButton->setEnabled( false );
      break;
  }
}

void QgsVectorElevationPropertiesWidget::updateProperty()
{
  QgsPropertyOverrideButton *button = qobject_cast<QgsPropertyOverrideButton *>( sender() );
  QgsMapLayerElevationProperties::Property key = static_cast<  QgsMapLayerElevationProperties::Property >( button->propertyKey() );
  mPropertyCollection.setProperty( key, button->toProperty() );
}

void QgsVectorElevationPropertiesWidget::initializeDataDefinedButton( QgsPropertyOverrideButton *button, QgsMapLayerElevationProperties::Property key )
{
  button->blockSignals( true );
  button->init( key, mPropertyCollection, QgsMapLayerElevationProperties::propertyDefinitions(), nullptr );
  connect( button, &QgsPropertyOverrideButton::changed, this, &QgsVectorElevationPropertiesWidget::updateProperty );
  button->registerExpressionContextGenerator( this );
  button->blockSignals( false );
}

void QgsVectorElevationPropertiesWidget::updateDataDefinedButtons()
{
  const auto propertyOverrideButtons { findChildren< QgsPropertyOverrideButton * >() };
  for ( QgsPropertyOverrideButton *button : propertyOverrideButtons )
  {
    updateDataDefinedButton( button );
  }
}

void QgsVectorElevationPropertiesWidget::updateDataDefinedButton( QgsPropertyOverrideButton *button )
{
  if ( !button )
    return;

  if ( button->propertyKey() < 0 )
    return;

  QgsMapLayerElevationProperties::Property key = static_cast< QgsMapLayerElevationProperties::Property >( button->propertyKey() );
  whileBlocking( button )->setToProperty( mPropertyCollection.property( key ) );
  whileBlocking( button )->setVectorLayer( mLayer );
}


//
// QgsVectorElevationPropertiesWidgetFactory
//

QgsVectorElevationPropertiesWidgetFactory::QgsVectorElevationPropertiesWidgetFactory( QObject *parent )
  : QObject( parent )
{
  setIcon( QgsApplication::getThemeIcon( QStringLiteral( "propertyicons/elevationscale.svg" ) ) );
  setTitle( tr( "Elevation" ) );
}

QgsMapLayerConfigWidget *QgsVectorElevationPropertiesWidgetFactory::createWidget( QgsMapLayer *layer, QgsMapCanvas *canvas, bool, QWidget *parent ) const
{
  return new QgsVectorElevationPropertiesWidget( qobject_cast< QgsVectorLayer * >( layer ), canvas, parent );
}

bool QgsVectorElevationPropertiesWidgetFactory::supportLayerPropertiesDialog() const
{
  return true;
}

bool QgsVectorElevationPropertiesWidgetFactory::supportsStyleDock() const
{
  return false;
}

bool QgsVectorElevationPropertiesWidgetFactory::supportsLayer( QgsMapLayer *layer ) const
{
  return layer->type() == QgsMapLayerType::VectorLayer;
}

QString QgsVectorElevationPropertiesWidgetFactory::layerPropertiesPagePositionHint() const
{
  return QStringLiteral( "mOptsPage_Metadata" );
}


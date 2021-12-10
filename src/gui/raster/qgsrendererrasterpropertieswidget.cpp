/***************************************************************************
    qgsrendererrasterpropertieswidget.cpp
    ---------------------
    begin                : May 2016
    copyright            : (C) 2016 by Nathan Woodrow
    email                : woodrow dot nathan at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "qgsrendererrasterpropertieswidget.h"

#include "qgis.h"
#include "qgsmapcanvas.h"
#include "qgsbrightnesscontrastfilter.h"
#include "qgshuesaturationfilter.h"
#include "qgsrastercontourrendererwidget.h"
#include "qgsrasterlayer.h"
#include "qgsrasterrendererwidget.h"
#include "qgsrasterrendererregistry.h"
#include "qgssinglebandgrayrendererwidget.h"
#include "qgssinglebandpseudocolorrendererwidget.h"
#include "qgsmultibandcolorrendererwidget.h"
#include "qgspalettedrendererwidget.h"
#include "qgshillshaderendererwidget.h"
#include "qgsmultibandcolorrenderer.h"
#include "qgssinglebandgrayrenderer.h"
#include "qgsapplication.h"
#include "qgscolorrampimpl.h"
#include "qgsproject.h"
#include "qgsprojectutils.h"

#include "qgsmessagelog.h"

static void _initRendererWidgetFunctions()
{
  static bool sInitialized = false;
  if ( sInitialized )
    return;

  QgsApplication::rasterRendererRegistry()->insertWidgetFunction( QStringLiteral( "paletted" ), QgsPalettedRendererWidget::create );
  QgsApplication::rasterRendererRegistry()->insertWidgetFunction( QStringLiteral( "multibandcolor" ), QgsMultiBandColorRendererWidget::create );
  QgsApplication::rasterRendererRegistry()->insertWidgetFunction( QStringLiteral( "singlebandpseudocolor" ), QgsSingleBandPseudoColorRendererWidget::create );
  QgsApplication::rasterRendererRegistry()->insertWidgetFunction( QStringLiteral( "singlebandgray" ), QgsSingleBandGrayRendererWidget::create );
  QgsApplication::rasterRendererRegistry()->insertWidgetFunction( QStringLiteral( "hillshade" ), QgsHillshadeRendererWidget::create );
  QgsApplication::rasterRendererRegistry()->insertWidgetFunction( QStringLiteral( "contour" ), QgsRasterContourRendererWidget::create );

  sInitialized = true;
}



QgsRendererRasterPropertiesWidget::QgsRendererRasterPropertiesWidget( QgsMapLayer *layer, QgsMapCanvas *canvas, QWidget *parent )
  : QgsMapLayerConfigWidget( layer, canvas, parent )
  , mRasterLayer( qobject_cast<QgsRasterLayer *>( layer ) )
{
  if ( !mRasterLayer )
    return;

  setupUi( this );
  connect( mResetColorRenderingBtn, &QToolButton::clicked, this, &QgsRendererRasterPropertiesWidget::mResetColorRenderingBtn_clicked );

  _initRendererWidgetFunctions();

  mResamplingUtils.initWidgets( mRasterLayer, mZoomedInResamplingComboBox, mZoomedOutResamplingComboBox, mMaximumOversamplingSpinBox, mCbEarlyResampling );

  connect( cboRenderers, static_cast<void ( QComboBox::* )( int )>( &QComboBox::currentIndexChanged ), this, &QgsRendererRasterPropertiesWidget::rendererChanged );

  connect( mSliderBrightness, &QAbstractSlider::valueChanged, mBrightnessSpinBox, &QSpinBox::setValue );
  connect( mBrightnessSpinBox, static_cast < void ( QSpinBox::* )( int ) > ( &QSpinBox::valueChanged ), mSliderBrightness, &QAbstractSlider::setValue );
  mBrightnessSpinBox->setClearValue( 0 );

  connect( mSliderContrast, &QAbstractSlider::valueChanged, mContrastSpinBox, &QSpinBox::setValue );
  connect( mContrastSpinBox, static_cast < void ( QSpinBox::* )( int ) > ( &QSpinBox::valueChanged ), mSliderContrast, &QAbstractSlider::setValue );
  mContrastSpinBox->setClearValue( 0 );

  connect( mSliderGamma, &QAbstractSlider::valueChanged, this, &QgsRendererRasterPropertiesWidget::updateGammaSpinBox );
  connect( mGammaSpinBox, static_cast < void ( QDoubleSpinBox::* )( double ) > ( &QDoubleSpinBox::valueChanged ), this, &QgsRendererRasterPropertiesWidget::updateGammaSlider );
  mGammaSpinBox->setClearValue( 1.0 );

  // Connect saturation slider and spin box
  connect( sliderSaturation, &QAbstractSlider::valueChanged, spinBoxSaturation, &QSpinBox::setValue );
  connect( spinBoxSaturation, static_cast < void ( QSpinBox::* )( int ) > ( &QSpinBox::valueChanged ), sliderSaturation, &QAbstractSlider::setValue );
  spinBoxSaturation->setClearValue( 0 );

  // Connect colorize strength slider and spin box
  connect( sliderColorizeStrength, &QAbstractSlider::valueChanged, spinColorizeStrength, &QSpinBox::setValue );
  connect( spinColorizeStrength, static_cast < void ( QSpinBox::* )( int ) > ( &QSpinBox::valueChanged ), sliderColorizeStrength, &QAbstractSlider::setValue );
  spinColorizeStrength->setClearValue( 100 );

  // enable or disable saturation slider and spin box depending on grayscale combo choice
  connect( comboGrayscale, static_cast<void ( QComboBox::* )( int )>( &QComboBox::currentIndexChanged ), this, &QgsRendererRasterPropertiesWidget::toggleSaturationControls );

  // enable or disable colorize colorbutton with colorize checkbox
  connect( mColorizeCheck, &QAbstractButton::toggled, this, &QgsRendererRasterPropertiesWidget::toggleColorizeControls );

  // Just connect the spin boxes because the sliders update the spinners
  connect( mBrightnessSpinBox, static_cast < void ( QSpinBox::* )( int ) > ( &QSpinBox::valueChanged ), this, &QgsPanelWidget::widgetChanged );
  connect( mContrastSpinBox, static_cast < void ( QSpinBox::* )( int ) > ( &QSpinBox::valueChanged ), this, &QgsPanelWidget::widgetChanged );
  connect( mGammaSpinBox, static_cast < void ( QDoubleSpinBox::* )( double ) > ( &QDoubleSpinBox::valueChanged ), this, &QgsPanelWidget::widgetChanged );
  connect( spinBoxSaturation, static_cast < void ( QSpinBox::* )( int ) > ( &QSpinBox::valueChanged ), this, &QgsPanelWidget::widgetChanged );
  connect( spinColorizeStrength, static_cast < void ( QSpinBox::* )( int ) > ( &QSpinBox::valueChanged ), this, &QgsPanelWidget::widgetChanged );
  connect( btnColorizeColor, &QgsColorButton::colorChanged, this, &QgsPanelWidget::widgetChanged );
  connect( mInvertColorsCheck, &QAbstractButton::toggled, this, &QgsPanelWidget::widgetChanged );

  connect( mBlendModeComboBox, static_cast<void ( QComboBox::* )( int )>( &QComboBox::currentIndexChanged ), this, &QgsPanelWidget::widgetChanged );
  connect( mZoomedInResamplingComboBox, static_cast<void ( QComboBox::* )( int )>( &QComboBox::currentIndexChanged ), this, &QgsPanelWidget::widgetChanged );
  connect( mZoomedOutResamplingComboBox, static_cast<void ( QComboBox::* )( int )>( &QComboBox::currentIndexChanged ), this, &QgsPanelWidget::widgetChanged );
  connect( mMaximumOversamplingSpinBox, static_cast < void ( QDoubleSpinBox::* )( double ) > ( &QDoubleSpinBox::valueChanged ), this, &QgsPanelWidget::widgetChanged );
  connect( mCbEarlyResampling,  &QAbstractButton::toggled, this, &QgsPanelWidget::widgetChanged );

  // finally sync to the layer - even though some actions may emit widgetChanged signal,
  // this is not a problem - nobody is listening to our signals yet
  syncToLayer( mRasterLayer );

  connect( mRasterLayer, &QgsMapLayer::styleChanged, this, &QgsRendererRasterPropertiesWidget::refreshAfterStyleChanged );
}

void QgsRendererRasterPropertiesWidget::setMapCanvas( QgsMapCanvas *canvas )
{
  mMapCanvas = canvas;
}

void QgsRendererRasterPropertiesWidget::rendererChanged()
{
  const QString rendererName = cboRenderers->currentData().toString();
  setRendererWidget( rendererName );
  emit widgetChanged();
}

void QgsRendererRasterPropertiesWidget::apply()
{
  if ( QgsBrightnessContrastFilter *brightnessFilter = mRasterLayer->brightnessFilter() )
  {
    brightnessFilter->setBrightness( mSliderBrightness->value() );
    brightnessFilter->setContrast( mSliderContrast->value() );
    brightnessFilter->setGamma( mGammaSpinBox->value() );
  }

  if ( QgsRasterRendererWidget *rendererWidget = dynamic_cast<QgsRasterRendererWidget *>( stackedWidget->currentWidget() ) )
  {
    rendererWidget->doComputations();

    if ( QgsRasterRenderer *newRenderer = rendererWidget->renderer() )
    {
      // there are transparency related data stored in renderer instances, but they
      // are not configured in the widget, so we need to copy them over from existing renderer
      if ( QgsRasterRenderer *oldRenderer = mRasterLayer->renderer() )
        newRenderer->copyCommonProperties( oldRenderer, false );
      mRasterLayer->setRenderer( newRenderer );
    }
  }

  // Hue and saturation controls
  if ( QgsHueSaturationFilter *hueSaturationFilter = mRasterLayer->hueSaturationFilter() )
  {
    hueSaturationFilter->setSaturation( sliderSaturation->value() );
    hueSaturationFilter->setGrayscaleMode( ( QgsHueSaturationFilter::GrayscaleMode ) comboGrayscale->currentIndex() );
    hueSaturationFilter->setColorizeOn( mColorizeCheck->checkState() );
    hueSaturationFilter->setColorizeColor( btnColorizeColor->color() );
    hueSaturationFilter->setColorizeStrength( sliderColorizeStrength->value() );
    hueSaturationFilter->setInvertColors( mInvertColorsCheck->isChecked() );
  }

  mResamplingUtils.refreshLayerFromWidgets();

  mRasterLayer->setBlendMode( mBlendModeComboBox->blendMode() );
}

void QgsRendererRasterPropertiesWidget::syncToLayer( QgsRasterLayer *layer )
{
  mRasterLayer = layer;

  cboRenderers->blockSignals( true );
  cboRenderers->clear();
  QgsRasterRendererRegistryEntry entry;
  const auto constRenderersList = QgsApplication::rasterRendererRegistry()->renderersList();
  for ( const QString &name : constRenderersList )
  {
    if ( QgsApplication::rasterRendererRegistry()->rendererData( name, entry ) )
    {
      if ( ( mRasterLayer->rasterType() != QgsRasterLayer::ColorLayer && entry.name != QLatin1String( "singlebandcolordata" ) ) ||
           ( mRasterLayer->rasterType() == QgsRasterLayer::ColorLayer && entry.name == QLatin1String( "singlebandcolordata" ) ) )
      {
        cboRenderers->addItem( entry.icon(), entry.visibleName, entry.name );
      }
    }
  }
  cboRenderers->setCurrentIndex( -1 );
  cboRenderers->blockSignals( false );

  if ( QgsRasterRenderer *renderer = mRasterLayer->renderer() )
  {
    setRendererWidget( renderer->type() );
  }

  if ( QgsBrightnessContrastFilter *brightnessFilter = mRasterLayer->brightnessFilter() )
  {
    mSliderBrightness->setValue( brightnessFilter->brightness() );
    mSliderContrast->setValue( brightnessFilter->contrast() );
    mGammaSpinBox->setValue( brightnessFilter->gamma() );
  }

  btnColorizeColor->setColorDialogTitle( tr( "Select Color" ) );
  btnColorizeColor->setContext( QStringLiteral( "symbology" ) );

  // Hue and saturation color control
  //set hue and saturation controls to current values
  if ( const QgsHueSaturationFilter *hueSaturationFilter = mRasterLayer->hueSaturationFilter() )
  {
    sliderSaturation->setValue( hueSaturationFilter->saturation() );
    comboGrayscale->setCurrentIndex( ( int ) hueSaturationFilter->grayscaleMode() );

    // Set initial state of saturation controls based on grayscale mode choice
    toggleSaturationControls( static_cast<int>( hueSaturationFilter->grayscaleMode() ) );

    // Set initial state of colorize controls
    mColorizeCheck->setChecked( hueSaturationFilter->colorizeOn() );
    btnColorizeColor->setColor( hueSaturationFilter->colorizeColor() );
    toggleColorizeControls( hueSaturationFilter->colorizeOn() );
    sliderColorizeStrength->setValue( hueSaturationFilter->colorizeStrength() );

    mInvertColorsCheck->setChecked( hueSaturationFilter->invertColors() );
  }

  //blend mode
  mBlendModeComboBox->setShowClippingModes( QgsProjectUtils::layerIsContainedInGroupLayer( QgsProject::instance(), mRasterLayer ) );
  mBlendModeComboBox->setBlendMode( mRasterLayer->blendMode() );

  //set combo boxes to current resampling types
  mResamplingUtils.refreshWidgetsFromLayer();
}

void QgsRendererRasterPropertiesWidget::mResetColorRenderingBtn_clicked()
{
  mBlendModeComboBox->setBlendMode( QPainter::CompositionMode_SourceOver );
  mSliderBrightness->setValue( 0 );
  mSliderContrast->setValue( 0 );
  mGammaSpinBox->setValue( 1.0 );
  sliderSaturation->setValue( 0 );
  comboGrayscale->setCurrentIndex( ( int ) QgsHueSaturationFilter::GrayscaleOff );
  mColorizeCheck->setChecked( false );
  sliderColorizeStrength->setValue( 100 );
  mInvertColorsCheck->setChecked( false );
}

void QgsRendererRasterPropertiesWidget::toggleSaturationControls( int grayscaleMode )
{
  // Enable or disable saturation controls based on choice of grayscale mode
  if ( grayscaleMode == 0 )
  {
    sliderSaturation->setEnabled( true );
    spinBoxSaturation->setEnabled( true );
  }
  else
  {
    sliderSaturation->setEnabled( false );
    spinBoxSaturation->setEnabled( false );
  }
  emit widgetChanged();
}

void QgsRendererRasterPropertiesWidget::toggleColorizeControls( bool colorizeEnabled )
{
  // Enable or disable colorize controls based on checkbox
  btnColorizeColor->setEnabled( colorizeEnabled );
  sliderColorizeStrength->setEnabled( colorizeEnabled );
  spinColorizeStrength->setEnabled( colorizeEnabled );
  emit widgetChanged();
}

void QgsRendererRasterPropertiesWidget::setRendererWidget( const QString &rendererName )
{
  QgsDebugMsgLevel( "rendererName = " + rendererName, 3 );
  QgsRasterRendererWidget *oldWidget = mRendererWidget;

  int alphaBand = -1;
  double opacity = 1;
  QColor nodataColor;
  if ( QgsRasterRenderer *oldRenderer = mRasterLayer->renderer() )
  {
    // Retain alpha band and opacity when switching renderer
    alphaBand = oldRenderer->alphaBand();
    opacity = oldRenderer->opacity();
    nodataColor = oldRenderer->nodataColor();
  }

  QgsRasterRendererRegistryEntry rendererEntry;
  if ( QgsApplication::rasterRendererRegistry()->rendererData( rendererName, rendererEntry ) )
  {
    if ( rendererEntry.widgetCreateFunction ) // Single band color data renderer e.g. has no widget
    {
      QgsDebugMsgLevel( QStringLiteral( "renderer has widgetCreateFunction" ), 3 );
      // Current canvas extent (used to calc min/max) in layer CRS
      const QgsRectangle myExtent = QgsCoordinateTransform::isTransformationPossible( mRasterLayer->crs(), mMapCanvas->mapSettings().destinationCrs() )
                                    ? mMapCanvas->mapSettings().outputExtentToLayerExtent( mRasterLayer, mMapCanvas->extent() )
                                    : mRasterLayer->extent();
      if ( oldWidget )
      {
        std::unique_ptr< QgsRasterRenderer > oldRenderer( oldWidget->renderer() );
        if ( !oldRenderer || oldRenderer->type() != rendererName )
        {
          if ( rendererName == QLatin1String( "singlebandgray" ) )
          {
            whileBlocking( mRasterLayer )->setRenderer( QgsApplication::rasterRendererRegistry()->defaultRendererForDrawingStyle( QgsRaster::SingleBandGray, mRasterLayer->dataProvider() ) );
            whileBlocking( mRasterLayer )->setDefaultContrastEnhancement();
          }
          else if ( rendererName == QLatin1String( "multibandcolor" ) )
          {
            whileBlocking( mRasterLayer )->setRenderer( QgsApplication::rasterRendererRegistry()->defaultRendererForDrawingStyle( QgsRaster::MultiBandColor, mRasterLayer->dataProvider() ) );
            whileBlocking( mRasterLayer )->setDefaultContrastEnhancement();
          }
        }
      }
      mRasterLayer->renderer()->setAlphaBand( alphaBand );
      mRasterLayer->renderer()->setOpacity( opacity );
      mRasterLayer->renderer()->setNodataColor( nodataColor );
      mRendererWidget = rendererEntry.widgetCreateFunction( mRasterLayer, myExtent );
      mRendererWidget->setMapCanvas( mMapCanvas );
      connect( mRendererWidget, &QgsRasterRendererWidget::widgetChanged, this, &QgsPanelWidget::widgetChanged );
      stackedWidget->addWidget( mRendererWidget );
      stackedWidget->setCurrentWidget( mRendererWidget );
      if ( oldWidget )
      {
        // Compare used bands in new and old renderer and reset transparency dialog if different
        QgsRasterRenderer *oldRenderer = oldWidget->renderer();
        QgsRasterRenderer *newRenderer = mRendererWidget->renderer();
#if 0
        QList<int> oldBands = oldRenderer->usesBands();
        QList<int> newBands = newRenderer->usesBands();

        if ( oldBands != newBands )
        {
          populateTransparencyTable( newRenderer );
        }
#endif

        delete oldRenderer;
        delete newRenderer;
      }
    }
  }

  if ( mRendererWidget != oldWidget )
    delete oldWidget;

  const int widgetIndex = cboRenderers->findData( rendererName );
  if ( widgetIndex != -1 )
  {
    whileBlocking( cboRenderers )->setCurrentIndex( widgetIndex );
  }

}

void QgsRendererRasterPropertiesWidget::refreshAfterStyleChanged()
{
  if ( mRendererWidget )
  {
    QgsRasterRenderer *renderer = mRasterLayer->renderer();
    if ( QgsMultiBandColorRenderer *mbcr = dynamic_cast<QgsMultiBandColorRenderer *>( renderer ) )
    {
      const QgsContrastEnhancement *redCe = mbcr->redContrastEnhancement();
      if ( redCe )
      {
        mRendererWidget->setMin( QLocale().toString( redCe->minimumValue() ), 0 );
        mRendererWidget->setMax( QLocale().toString( redCe->maximumValue() ), 0 );
      }
      const QgsContrastEnhancement *greenCe = mbcr->greenContrastEnhancement();
      if ( greenCe )
      {
        mRendererWidget->setMin( QLocale().toString( greenCe->minimumValue() ), 1 );
        mRendererWidget->setMax( QLocale().toString( greenCe->maximumValue() ), 1 );
      }
      const QgsContrastEnhancement *blueCe = mbcr->blueContrastEnhancement();
      if ( blueCe )
      {
        mRendererWidget->setMin( QLocale().toString( blueCe->minimumValue() ), 2 );
        mRendererWidget->setMax( QLocale().toString( blueCe->maximumValue() ), 2 );
      }
    }
    else if ( QgsSingleBandGrayRenderer *sbgr = dynamic_cast<QgsSingleBandGrayRenderer *>( renderer ) )
    {
      const QgsContrastEnhancement *ce = sbgr->contrastEnhancement();
      if ( ce )
      {
        mRendererWidget->setMin( QLocale().toString( ce->minimumValue() ) );
        mRendererWidget->setMax( QLocale().toString( ce->maximumValue() ) );
      }
    }
  }
}

void QgsRendererRasterPropertiesWidget::updateGammaSpinBox( int value )
{
  mGammaSpinBox->setValue( value / 100.0 );
}

void QgsRendererRasterPropertiesWidget::updateGammaSlider( double value )
{
  mSliderGamma->setValue( value * 100 );
}

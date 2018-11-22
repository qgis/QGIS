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
#include "qgsrasterlayer.h"
#include "qgsrasterrendererwidget.h"
#include "qgsrasterrendererregistry.h"
#include "qgssinglebandgrayrendererwidget.h"
#include "qgssinglebandpseudocolorrendererwidget.h"
#include "qgsmultibandcolorrendererwidget.h"
#include "qgspalettedrendererwidget.h"
#include "qgshillshaderendererwidget.h"
#include "qgsrasterresamplefilter.h"
#include "qgsbilinearrasterresampler.h"
#include "qgscubicrasterresampler.h"
#include "qgsmultibandcolorrenderer.h"
#include "qgssinglebandgrayrenderer.h"


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

  sInitialized = true;
}



QgsRendererRasterPropertiesWidget::QgsRendererRasterPropertiesWidget( QgsMapLayer *layer, QgsMapCanvas *canvas, QWidget *parent )
  : QgsMapLayerConfigWidget( layer, canvas, parent )

{
  mRasterLayer = qobject_cast<QgsRasterLayer *>( layer );

  if ( !( mRasterLayer && mRasterLayer->isValid() ) )
    return;

  setupUi( this );
  connect( mResetColorRenderingBtn, &QToolButton::clicked, this, &QgsRendererRasterPropertiesWidget::mResetColorRenderingBtn_clicked );

  _initRendererWidgetFunctions();

  mZoomedInResamplingComboBox->insertItem( 0, tr( "Nearest neighbour" ) );
  mZoomedInResamplingComboBox->insertItem( 1, tr( "Bilinear" ) );
  mZoomedInResamplingComboBox->insertItem( 2, tr( "Cubic" ) );
  mZoomedOutResamplingComboBox->insertItem( 0, tr( "Nearest neighbour" ) );
  mZoomedOutResamplingComboBox->insertItem( 1, tr( "Average" ) );

  connect( cboRenderers, static_cast<void ( QComboBox::* )( int )>( &QComboBox::currentIndexChanged ), this, &QgsRendererRasterPropertiesWidget::rendererChanged );

  connect( mSliderBrightness, &QAbstractSlider::valueChanged, mBrightnessSpinBox, &QSpinBox::setValue );
  connect( mBrightnessSpinBox, static_cast < void ( QSpinBox::* )( int ) > ( &QSpinBox::valueChanged ), mSliderBrightness, &QAbstractSlider::setValue );

  connect( mSliderContrast, &QAbstractSlider::valueChanged, mContrastSpinBox, &QSpinBox::setValue );
  connect( mContrastSpinBox, static_cast < void ( QSpinBox::* )( int ) > ( &QSpinBox::valueChanged ), mSliderContrast, &QAbstractSlider::setValue );

  // Connect saturation slider and spin box
  connect( sliderSaturation, &QAbstractSlider::valueChanged, spinBoxSaturation, &QSpinBox::setValue );
  connect( spinBoxSaturation, static_cast < void ( QSpinBox::* )( int ) > ( &QSpinBox::valueChanged ), sliderSaturation, &QAbstractSlider::setValue );

  // Connect colorize strength slider and spin box
  connect( sliderColorizeStrength, &QAbstractSlider::valueChanged, spinColorizeStrength, &QSpinBox::setValue );
  connect( spinColorizeStrength, static_cast < void ( QSpinBox::* )( int ) > ( &QSpinBox::valueChanged ), sliderColorizeStrength, &QAbstractSlider::setValue );

  // enable or disable saturation slider and spin box depending on grayscale combo choice
  connect( comboGrayscale, static_cast<void ( QComboBox::* )( int )>( &QComboBox::currentIndexChanged ), this, &QgsRendererRasterPropertiesWidget::toggleSaturationControls );

  // enable or disable colorize colorbutton with colorize checkbox
  connect( mColorizeCheck, &QAbstractButton::toggled, this, &QgsRendererRasterPropertiesWidget::toggleColorizeControls );

  // Just connect the spin boxes because the sliders update the spinners
  connect( mBrightnessSpinBox, static_cast < void ( QSpinBox::* )( int ) > ( &QSpinBox::valueChanged ), this, &QgsPanelWidget::widgetChanged );
  connect( mContrastSpinBox, static_cast < void ( QSpinBox::* )( int ) > ( &QSpinBox::valueChanged ), this, &QgsPanelWidget::widgetChanged );
  connect( spinBoxSaturation, static_cast < void ( QSpinBox::* )( int ) > ( &QSpinBox::valueChanged ), this, &QgsPanelWidget::widgetChanged );
  connect( spinColorizeStrength, static_cast < void ( QSpinBox::* )( int ) > ( &QSpinBox::valueChanged ), this, &QgsPanelWidget::widgetChanged );
  connect( btnColorizeColor, &QgsColorButton::colorChanged, this, &QgsPanelWidget::widgetChanged );

  connect( mBlendModeComboBox, static_cast<void ( QComboBox::* )( int )>( &QComboBox::currentIndexChanged ), this, &QgsPanelWidget::widgetChanged );
  connect( mZoomedInResamplingComboBox, static_cast<void ( QComboBox::* )( int )>( &QComboBox::currentIndexChanged ), this, &QgsPanelWidget::widgetChanged );
  connect( mZoomedOutResamplingComboBox, static_cast<void ( QComboBox::* )( int )>( &QComboBox::currentIndexChanged ), this, &QgsPanelWidget::widgetChanged );
  connect( mMaximumOversamplingSpinBox, static_cast < void ( QDoubleSpinBox::* )( double ) > ( &QDoubleSpinBox::valueChanged ), this, &QgsPanelWidget::widgetChanged );

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
  QString rendererName = cboRenderers->currentData().toString();
  setRendererWidget( rendererName );
  emit widgetChanged();
}

void QgsRendererRasterPropertiesWidget::apply()
{

  if ( ! mRasterLayer->isValid() )
    return;

  mRasterLayer->brightnessFilter()->setBrightness( mSliderBrightness->value() );
  mRasterLayer->brightnessFilter()->setContrast( mSliderContrast->value() );

  QgsRasterRendererWidget *rendererWidget = dynamic_cast<QgsRasterRendererWidget *>( stackedWidget->currentWidget() );
  if ( rendererWidget )
  {
    rendererWidget->doComputations();

    QgsRasterRenderer *newRenderer = rendererWidget->renderer();

    // there are transparency related data stored in renderer instances, but they
    // are not configured in the widget, so we need to copy them over from existing renderer
    QgsRasterRenderer *oldRenderer = mRasterLayer->renderer();
    if ( oldRenderer )
      newRenderer->copyCommonProperties( oldRenderer, false );
    mRasterLayer->setRenderer( newRenderer );
  }

  // Hue and saturation controls
  QgsHueSaturationFilter *hueSaturationFilter = mRasterLayer->hueSaturationFilter();
  if ( hueSaturationFilter )
  {
    hueSaturationFilter->setSaturation( sliderSaturation->value() );
    hueSaturationFilter->setGrayscaleMode( ( QgsHueSaturationFilter::GrayscaleMode ) comboGrayscale->currentIndex() );
    hueSaturationFilter->setColorizeOn( mColorizeCheck->checkState() );
    hueSaturationFilter->setColorizeColor( btnColorizeColor->color() );
    hueSaturationFilter->setColorizeStrength( sliderColorizeStrength->value() );
  }

  QgsRasterResampleFilter *resampleFilter = mRasterLayer->resampleFilter();
  if ( resampleFilter )
  {
    QgsRasterResampler *zoomedInResampler = nullptr;
    QString zoomedInResamplingMethod = mZoomedInResamplingComboBox->currentText();
    if ( zoomedInResamplingMethod == tr( "Bilinear" ) )
    {
      zoomedInResampler = new QgsBilinearRasterResampler();
    }
    else if ( zoomedInResamplingMethod == tr( "Cubic" ) )
    {
      zoomedInResampler = new QgsCubicRasterResampler();
    }

    resampleFilter->setZoomedInResampler( zoomedInResampler );

    //raster resampling
    QgsRasterResampler *zoomedOutResampler = nullptr;
    QString zoomedOutResamplingMethod = mZoomedOutResamplingComboBox->currentText();
    if ( zoomedOutResamplingMethod == tr( "Average" ) )
    {
      zoomedOutResampler = new QgsBilinearRasterResampler();
    }

    resampleFilter->setZoomedOutResampler( zoomedOutResampler );

    resampleFilter->setMaxOversampling( mMaximumOversamplingSpinBox->value() );
  }

  mRasterLayer->setBlendMode( mBlendModeComboBox->blendMode() );
}

void QgsRendererRasterPropertiesWidget::syncToLayer( QgsRasterLayer *layer )
{
  mRasterLayer = layer;

  cboRenderers->blockSignals( true );
  cboRenderers->clear();
  QgsRasterRendererRegistryEntry entry;
  Q_FOREACH ( const QString &name, QgsApplication::rasterRendererRegistry()->renderersList() )
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

  QgsRasterRenderer *renderer = mRasterLayer->renderer();
  if ( renderer )
  {
    setRendererWidget( renderer->type() );
  }

  QgsBrightnessContrastFilter *brightnessFilter = mRasterLayer->brightnessFilter();
  if ( brightnessFilter )
  {
    mSliderBrightness->setValue( brightnessFilter->brightness() );
    mSliderContrast->setValue( brightnessFilter->contrast() );
  }

  btnColorizeColor->setColorDialogTitle( tr( "Select Color" ) );
  btnColorizeColor->setContext( QStringLiteral( "symbology" ) );

  // Hue and saturation color control
  const QgsHueSaturationFilter *hueSaturationFilter = mRasterLayer->hueSaturationFilter();
  //set hue and saturation controls to current values
  if ( hueSaturationFilter )
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
  }

  //blend mode
  mBlendModeComboBox->setBlendMode( mRasterLayer->blendMode() );

  const QgsRasterResampleFilter *resampleFilter = mRasterLayer->resampleFilter();
  //set combo boxes to current resampling types
  if ( resampleFilter )
  {
    const QgsRasterResampler *zoomedInResampler = resampleFilter->zoomedInResampler();
    if ( zoomedInResampler )
    {
      if ( zoomedInResampler->type() == QLatin1String( "bilinear" ) )
      {
        mZoomedInResamplingComboBox->setCurrentIndex( 1 );
      }
      else if ( zoomedInResampler->type() == QLatin1String( "cubic" ) )
      {
        mZoomedInResamplingComboBox->setCurrentIndex( 2 );
      }
    }
    else
    {
      mZoomedInResamplingComboBox->setCurrentIndex( 0 );
    }

    const QgsRasterResampler *zoomedOutResampler = resampleFilter->zoomedOutResampler();
    if ( zoomedOutResampler )
    {
      if ( zoomedOutResampler->type() == QLatin1String( "bilinear" ) ) //bilinear resampler does averaging when zooming out
      {
        mZoomedOutResamplingComboBox->setCurrentIndex( 1 );
      }
    }
    else
    {
      mZoomedOutResamplingComboBox->setCurrentIndex( 0 );
    }
    mMaximumOversamplingSpinBox->setValue( resampleFilter->maxOversampling() );
  }
}

void QgsRendererRasterPropertiesWidget::mResetColorRenderingBtn_clicked()
{
  mBlendModeComboBox->setBlendMode( QPainter::CompositionMode_SourceOver );
  mSliderBrightness->setValue( 0 );
  mSliderContrast->setValue( 0 );
  sliderSaturation->setValue( 0 );
  comboGrayscale->setCurrentIndex( ( int ) QgsHueSaturationFilter::GrayscaleOff );
  mColorizeCheck->setChecked( false );
  sliderColorizeStrength->setValue( 100 );
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
  QgsDebugMsg( "rendererName = " + rendererName );
  QgsRasterRendererWidget *oldWidget = mRendererWidget;
  QgsRasterRenderer *oldRenderer = mRasterLayer->renderer();

  int alphaBand = -1;
  double opacity = 1;
  if ( oldRenderer )
  {
    // Retain alpha band and opacity when switching renderer
    alphaBand = oldRenderer->alphaBand();
    opacity = oldRenderer->opacity();
  }

  QgsRasterRendererRegistryEntry rendererEntry;
  if ( QgsApplication::rasterRendererRegistry()->rendererData( rendererName, rendererEntry ) )
  {
    if ( rendererEntry.widgetCreateFunction ) // Single band color data renderer e.g. has no widget
    {
      QgsDebugMsg( QStringLiteral( "renderer has widgetCreateFunction" ) );
      // Current canvas extent (used to calc min/max) in layer CRS
      QgsRectangle myExtent = mMapCanvas->mapSettings().outputExtentToLayerExtent( mRasterLayer, mMapCanvas->extent() );
      if ( oldWidget )
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
      mRasterLayer->renderer()->setAlphaBand( alphaBand );
      mRasterLayer->renderer()->setOpacity( opacity );
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

  int widgetIndex = cboRenderers->findData( rendererName );
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
        mRendererWidget->setMin( QString::number( redCe->minimumValue() ), 0 );
        mRendererWidget->setMax( QString::number( redCe->maximumValue() ), 0 );
      }
      const QgsContrastEnhancement *greenCe = mbcr->greenContrastEnhancement();
      if ( greenCe )
      {
        mRendererWidget->setMin( QString::number( greenCe->minimumValue() ), 1 );
        mRendererWidget->setMax( QString::number( greenCe->maximumValue() ), 1 );
      }
      const QgsContrastEnhancement *blueCe = mbcr->blueContrastEnhancement();
      if ( blueCe )
      {
        mRendererWidget->setMin( QString::number( blueCe->minimumValue() ), 2 );
        mRendererWidget->setMax( QString::number( blueCe->maximumValue() ), 2 );
      }
    }
    else if ( QgsSingleBandGrayRenderer *sbgr = dynamic_cast<QgsSingleBandGrayRenderer *>( renderer ) )
    {
      const QgsContrastEnhancement *ce = sbgr->contrastEnhancement();
      if ( ce )
      {
        mRendererWidget->setMin( QString::number( ce->minimumValue() ) );
        mRendererWidget->setMax( QString::number( ce->maximumValue() ) );
      }
    }
  }
}

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


static void _initRendererWidgetFunctions()
{
  static bool initialized = false;
  if ( initialized )
    return;

  QgsRasterRendererRegistry::instance()->insertWidgetFunction( "paletted", QgsPalettedRendererWidget::create );
  QgsRasterRendererRegistry::instance()->insertWidgetFunction( "multibandcolor", QgsMultiBandColorRendererWidget::create );
  QgsRasterRendererRegistry::instance()->insertWidgetFunction( "singlebandpseudocolor", QgsSingleBandPseudoColorRendererWidget::create );
  QgsRasterRendererRegistry::instance()->insertWidgetFunction( "singlebandgray", QgsSingleBandGrayRendererWidget::create );
  QgsRasterRendererRegistry::instance()->insertWidgetFunction( "hillshade", QgsHillshadeRendererWidget::create );

  initialized = true;
}



QgsRendererRasterPropertiesWidget::QgsRendererRasterPropertiesWidget( QgsMapLayer *layer, QgsMapCanvas* canvas, QWidget *parent )
    : QgsMapLayerConfigWidget( layer, canvas, parent )
    , mRendererWidget( nullptr )
{
  mRasterLayer = qobject_cast<QgsRasterLayer*>( layer );
  if ( !mRasterLayer )
    return;

  setupUi( this );

  _initRendererWidgetFunctions();

  mZoomedInResamplingComboBox->insertItem( 0, tr( "Nearest neighbour" ) );
  mZoomedInResamplingComboBox->insertItem( 1, tr( "Bilinear" ) );
  mZoomedInResamplingComboBox->insertItem( 2, tr( "Cubic" ) );
  mZoomedOutResamplingComboBox->insertItem( 0, tr( "Nearest neighbour" ) );
  mZoomedOutResamplingComboBox->insertItem( 1, tr( "Average" ) );

  connect( cboRenderers, SIGNAL( currentIndexChanged( int ) ), this, SLOT( rendererChanged() ) );

  connect( mSliderBrightness, SIGNAL( valueChanged( int ) ), mBrightnessSpinBox, SLOT( setValue( int ) ) );
  connect( mBrightnessSpinBox, SIGNAL( valueChanged( int ) ), mSliderBrightness, SLOT( setValue( int ) ) );

  connect( mSliderContrast, SIGNAL( valueChanged( int ) ), mContrastSpinBox, SLOT( setValue( int ) ) );
  connect( mContrastSpinBox, SIGNAL( valueChanged( int ) ), mSliderContrast, SLOT( setValue( int ) ) );

  // Connect saturation slider and spin box
  connect( sliderSaturation, SIGNAL( valueChanged( int ) ), spinBoxSaturation, SLOT( setValue( int ) ) );
  connect( spinBoxSaturation, SIGNAL( valueChanged( int ) ), sliderSaturation, SLOT( setValue( int ) ) );

  // Connect colorize strength slider and spin box
  connect( sliderColorizeStrength, SIGNAL( valueChanged( int ) ), spinColorizeStrength, SLOT( setValue( int ) ) );
  connect( spinColorizeStrength, SIGNAL( valueChanged( int ) ), sliderColorizeStrength, SLOT( setValue( int ) ) );

  // enable or disable saturation slider and spin box depending on grayscale combo choice
  connect( comboGrayscale, SIGNAL( currentIndexChanged( int ) ), this, SLOT( toggleSaturationControls( int ) ) );

  // enable or disable colorize colorbutton with colorize checkbox
  connect( mColorizeCheck, SIGNAL( toggled( bool ) ), this, SLOT( toggleColorizeControls( bool ) ) );

  // Just connect the spin boxes because the sliders update the spinners
  connect( mBrightnessSpinBox, SIGNAL( valueChanged( int ) ), this, SIGNAL( widgetChanged() ) );
  connect( mContrastSpinBox, SIGNAL( valueChanged( int ) ), this, SIGNAL( widgetChanged() ) );
  connect( spinBoxSaturation, SIGNAL( valueChanged( int ) ), this, SIGNAL( widgetChanged() ) );
  connect( spinColorizeStrength, SIGNAL( valueChanged( int ) ), this, SIGNAL( widgetChanged() ) );

  connect( mBlendModeComboBox, SIGNAL( currentIndexChanged( int ) ), this, SIGNAL( widgetChanged() ) );
  connect( mZoomedInResamplingComboBox, SIGNAL( currentIndexChanged( int ) ), this, SIGNAL( widgetChanged() ) );
  connect( mZoomedOutResamplingComboBox, SIGNAL( currentIndexChanged( int ) ), this, SIGNAL( widgetChanged() ) );
  connect( mMaximumOversamplingSpinBox, SIGNAL( valueChanged( double ) ), this, SIGNAL( widgetChanged() ) );

  // finally sync to the layer - even though some actions may emit widgetChanged signal,
  // this is not a problem - nobody is listening to our signals yet
  syncToLayer( mRasterLayer );
}

QgsRendererRasterPropertiesWidget::~QgsRendererRasterPropertiesWidget()
{

}

void QgsRendererRasterPropertiesWidget::setMapCanvas( QgsMapCanvas *canvas )
{
  mMapCanvas = canvas;
}

void QgsRendererRasterPropertiesWidget::rendererChanged()
{
  QString rendererName = cboRenderers->itemData( cboRenderers->currentIndex() ).toString();
  setRendererWidget( rendererName );
  emit widgetChanged();
}

void QgsRendererRasterPropertiesWidget::apply()
{
  mRasterLayer->brightnessFilter()->setBrightness( mSliderBrightness->value() );
  mRasterLayer->brightnessFilter()->setContrast( mSliderContrast->value() );

  QgsRasterRendererWidget* rendererWidget = dynamic_cast<QgsRasterRendererWidget*>( stackedWidget->currentWidget() );
  if ( rendererWidget )
  {
    QgsRasterRenderer* newRenderer = rendererWidget->renderer();

    // there are transparency related data stored in renderer instances, but they
    // are not configured in the widget, so we need to copy them over from existing renderer
    QgsRasterRenderer* oldRenderer = mRasterLayer->renderer();
    if ( oldRenderer )
      newRenderer->copyCommonProperties( oldRenderer );

    mRasterLayer->setRenderer( newRenderer );
  }

  // Hue and saturation controls
  QgsHueSaturationFilter *hueSaturationFilter = mRasterLayer->hueSaturationFilter();
  if ( hueSaturationFilter )
  {
    hueSaturationFilter->setSaturation( sliderSaturation->value() );
    hueSaturationFilter->setGrayscaleMode(( QgsHueSaturationFilter::GrayscaleMode ) comboGrayscale->currentIndex() );
    hueSaturationFilter->setColorizeOn( mColorizeCheck->checkState() );
    hueSaturationFilter->setColorizeColor( btnColorizeColor->color() );
    hueSaturationFilter->setColorizeStrength( sliderColorizeStrength->value() );
  }

  QgsRasterResampleFilter* resampleFilter = mRasterLayer->resampleFilter();
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

void QgsRendererRasterPropertiesWidget::syncToLayer( QgsRasterLayer* layer )
{
  mRasterLayer = layer;

  cboRenderers->blockSignals( true );
  cboRenderers->clear();
  QgsRasterRendererRegistryEntry entry;
  Q_FOREACH ( const QString& name, QgsRasterRendererRegistry::instance()->renderersList() )
  {
    if ( QgsRasterRendererRegistry::instance()->rendererData( name, entry ) )
    {
      if (( mRasterLayer->rasterType() != QgsRasterLayer::ColorLayer && entry.name != "singlebandcolordata" ) ||
          ( mRasterLayer->rasterType() == QgsRasterLayer::ColorLayer && entry.name == "singlebandcolordata" ) )
      {
        cboRenderers->addItem( entry.icon(), entry.visibleName, entry.name );
      }
    }
  }
  cboRenderers->setCurrentIndex( -1 );
  cboRenderers->blockSignals( false );

  QgsRasterRenderer* renderer = mRasterLayer->renderer();
  if ( renderer )
  {
    setRendererWidget( renderer->type() );
  }

  QgsBrightnessContrastFilter* brightnessFilter = mRasterLayer->brightnessFilter();
  if ( brightnessFilter )
  {
    mSliderBrightness->setValue( brightnessFilter->brightness() );
    mSliderContrast->setValue( brightnessFilter->contrast() );
  }

  btnColorizeColor->setColorDialogTitle( tr( "Select color" ) );
  btnColorizeColor->setContext( "symbology" );

  // Hue and saturation color control
  const QgsHueSaturationFilter* hueSaturationFilter = mRasterLayer->hueSaturationFilter();
  //set hue and saturation controls to current values
  if ( hueSaturationFilter )
  {
    sliderSaturation->setValue( hueSaturationFilter->saturation() );
    comboGrayscale->setCurrentIndex(( int ) hueSaturationFilter->grayscaleMode() );

    // Set initial state of saturation controls based on grayscale mode choice
    toggleSaturationControls(( int )hueSaturationFilter->grayscaleMode() );

    // Set initial state of colorize controls
    mColorizeCheck->setChecked( hueSaturationFilter->colorizeOn() );
    btnColorizeColor->setColor( hueSaturationFilter->colorizeColor() );
    toggleColorizeControls( hueSaturationFilter->colorizeOn() );
    sliderColorizeStrength->setValue( hueSaturationFilter->colorizeStrength() );
  }

  //blend mode
  mBlendModeComboBox->setBlendMode( mRasterLayer->blendMode() );

  const QgsRasterResampleFilter* resampleFilter = mRasterLayer->resampleFilter();
  //set combo boxes to current resampling types
  if ( resampleFilter )
  {
    const QgsRasterResampler* zoomedInResampler = resampleFilter->zoomedInResampler();
    if ( zoomedInResampler )
    {
      if ( zoomedInResampler->type() == "bilinear" )
      {
        mZoomedInResamplingComboBox->setCurrentIndex( 1 );
      }
      else if ( zoomedInResampler->type() == "cubic" )
      {
        mZoomedInResamplingComboBox->setCurrentIndex( 2 );
      }
    }
    else
    {
      mZoomedInResamplingComboBox->setCurrentIndex( 0 );
    }

    const QgsRasterResampler* zoomedOutResampler = resampleFilter->zoomedOutResampler();
    if ( zoomedOutResampler )
    {
      if ( zoomedOutResampler->type() == "bilinear" ) //bilinear resampler does averaging when zooming out
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

void QgsRendererRasterPropertiesWidget::on_mResetColorRenderingBtn_clicked()
{
  mBlendModeComboBox->setBlendMode( QPainter::CompositionMode_SourceOver );
  mSliderBrightness->setValue( 0 );
  mSliderContrast->setValue( 0 );
  sliderSaturation->setValue( 0 );
  comboGrayscale->setCurrentIndex(( int ) QgsHueSaturationFilter::GrayscaleOff );
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
  QgsRasterRendererWidget* oldWidget = mRendererWidget;

  QgsRasterRendererRegistryEntry rendererEntry;
  if ( QgsRasterRendererRegistry::instance()->rendererData( rendererName, rendererEntry ) )
  {
    if ( rendererEntry.widgetCreateFunction ) //single band color data renderer e.g. has no widget
    {
      QgsDebugMsg( "renderer has widgetCreateFunction" );
      // Current canvas extent (used to calc min/max) in layer CRS
      QgsRectangle myExtent = mMapCanvas->mapSettings().outputExtentToLayerExtent( mRasterLayer, mMapCanvas->extent() );
      mRendererWidget = rendererEntry.widgetCreateFunction( mRasterLayer, myExtent );
      mRendererWidget->setMapCanvas( mMapCanvas );
      connect( mRendererWidget, SIGNAL( widgetChanged() ), this, SIGNAL( widgetChanged() ) );
      stackedWidget->addWidget( mRendererWidget );
      stackedWidget->setCurrentWidget( mRendererWidget );
      if ( oldWidget )
      {
        //compare used bands in new and old renderer and reset transparency dialog if different
        QgsRasterRenderer* oldRenderer = oldWidget->renderer();
        QgsRasterRenderer* newRenderer = mRendererWidget->renderer();
        QList<int> oldBands = oldRenderer->usesBands();
        QList<int> newBands = newRenderer->usesBands();
//        if ( oldBands != newBands )
//        {
//          populateTransparencyTable( newRenderer );
//        }
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

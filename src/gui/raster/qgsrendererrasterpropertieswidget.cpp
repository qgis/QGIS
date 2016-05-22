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


static void _initRendererWidgetFunctions()
{
  static bool initialized = false;
  if ( initialized )
    return;

  QgsRasterRendererRegistry::instance()->insertWidgetFunction( "paletted", QgsPalettedRendererWidget::create );
  QgsRasterRendererRegistry::instance()->insertWidgetFunction( "multibandcolor", QgsMultiBandColorRendererWidget::create );
  QgsRasterRendererRegistry::instance()->insertWidgetFunction( "singlebandpseudocolor", QgsSingleBandPseudoColorRendererWidget::create );
  QgsRasterRendererRegistry::instance()->insertWidgetFunction( "singlebandgray", QgsSingleBandGrayRendererWidget::create );

  initialized = true;
}



QgsRendererRasterPropertiesWidget::QgsRendererRasterPropertiesWidget( QgsMapCanvas* canvas, QWidget *parent )
    : QWidget( parent )
    , mRasterLayer( nullptr )
    , mMapCanvas( canvas )
    , mRendererWidget( nullptr )
{
  setupUi( this );

  _initRendererWidgetFunctions();

  connect( cboRenderers, SIGNAL( currentIndexChanged( int ) ), this, SLOT( rendererChanged() ) );

  connect( mSliderBrightness, SIGNAL( valueChanged( int ) ), mBrightnessSpinBox, SLOT( setValue( int ) ) );
  connect( mBrightnessSpinBox, SIGNAL( valueChanged( int ) ), mSliderBrightness, SLOT( setValue( int ) ) );

  // Just connect the spin box because the slidder updates the spinner
  connect( mBrightnessSpinBox, SIGNAL( valueChanged( int ) ), this, SIGNAL( widgetChanged() ) );

  connect( mSliderContrast, SIGNAL( valueChanged( int ) ), mContrastSpinBox, SLOT( setValue( int ) ) );
  connect( mContrastSpinBox, SIGNAL( valueChanged( int ) ), mSliderContrast, SLOT( setValue( int ) ) );

  // Just connect the spin box because the slidder updates the spinner
  connect( mContrastSpinBox, SIGNAL( valueChanged( int ) ), this, SIGNAL( widgetChanged() ) );

  // Connect saturation slider and spin box
  connect( sliderSaturation, SIGNAL( valueChanged( int ) ), spinBoxSaturation, SLOT( setValue( int ) ) );
  connect( spinBoxSaturation, SIGNAL( valueChanged( int ) ), sliderSaturation, SLOT( setValue( int ) ) );

  // Just connect the spin box because the slidder updates the spinner
  connect( spinBoxSaturation, SIGNAL( valueChanged( int ) ), this, SIGNAL( widgetChanged() ) );

  // Connect colorize strength slider and spin box
  connect( sliderColorizeStrength, SIGNAL( valueChanged( int ) ), spinColorizeStrength, SLOT( setValue( int ) ) );
  connect( spinColorizeStrength, SIGNAL( valueChanged( int ) ), sliderColorizeStrength, SLOT( setValue( int ) ) );

  // Connect colorize strength slider and spin box
  connect( spinColorizeStrength, SIGNAL( valueChanged( int ) ), this, SIGNAL( widgetChanged() ) );

  // enable or disable saturation slider and spin box depending on grayscale combo choice
  connect( comboGrayscale, SIGNAL( currentIndexChanged( int ) ), this, SLOT( toggleSaturationControls( int ) ) );

  connect( mBlendModeComboBox, SIGNAL( currentIndexChanged( int ) ), this, SIGNAL( widgetChanged() ) );

  // enable or disable colorize colorbutton with colorize checkbox
  connect( mColorizeCheck, SIGNAL( toggled( bool ) ), this, SLOT( toggleColorizeControls( bool ) ) );
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
    mRasterLayer->setRenderer( rendererWidget->renderer() );
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

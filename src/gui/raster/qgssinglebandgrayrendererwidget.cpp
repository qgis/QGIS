/***************************************************************************
                         qgssinglebandgrayrendererwidget.h
                         ---------------------------------
    begin                : March 2012
    copyright            : (C) 2012 by Marco Hugentobler
    email                : marco at sourcepole dot ch
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgssinglebandgrayrendererwidget.h"
#include "qgssinglebandgrayrenderer.h"
#include "qgsrasterlayer.h"
#include "qgsrasterdataprovider.h"
#include "qgsrasterminmaxwidget.h"
#include "qgsdoublevalidator.h"
#include "qgscolorramplegendnodewidget.h"

QgsSingleBandGrayRendererWidget::QgsSingleBandGrayRendererWidget( QgsRasterLayer *layer, const QgsRectangle &extent )
  : QgsRasterRendererWidget( layer, extent )
  , mDisableMinMaxWidgetRefresh( false )
{
  setupUi( this );
  connect( mMinLineEdit, &QLineEdit::textChanged, this, &QgsSingleBandGrayRendererWidget::mMinLineEdit_textChanged );
  connect( mMaxLineEdit, &QLineEdit::textChanged, this, &QgsSingleBandGrayRendererWidget::mMaxLineEdit_textChanged );

  connect( mLegendSettingsButton, &QPushButton::clicked, this, &QgsSingleBandGrayRendererWidget::showLegendSettings );

  mGradientComboBox->insertItem( 0, tr( "Black to White" ), QgsSingleBandGrayRenderer::BlackToWhite );
  mGradientComboBox->insertItem( 1, tr( "White to Black" ), QgsSingleBandGrayRenderer::WhiteToBlack );

  mMinLineEdit->setValidator( new QgsDoubleValidator( mMinLineEdit ) );
  mMaxLineEdit->setValidator( new QgsDoubleValidator( mMaxLineEdit ) );

  if ( mRasterLayer )
  {
    QgsRasterDataProvider *provider = mRasterLayer->dataProvider();
    if ( !provider )
    {
      return;
    }

    mMinMaxWidget = new QgsRasterMinMaxWidget( layer, this );
    mMinMaxWidget->setExtent( extent );
    mMinMaxWidget->setMapCanvas( mCanvas );

    QHBoxLayout *layout = new QHBoxLayout();
    layout->setContentsMargins( 0, 0, 0, 0 );
    mMinMaxContainerWidget->setLayout( layout );
    layout->addWidget( mMinMaxWidget );

    connect( mMinMaxWidget, &QgsRasterMinMaxWidget::widgetChanged,
             this, &QgsSingleBandGrayRendererWidget::widgetChanged );

    connect( mMinMaxWidget, &QgsRasterMinMaxWidget::load,
             this, &QgsSingleBandGrayRendererWidget::loadMinMax );

    mGrayBandComboBox->setLayer( mRasterLayer );

    //contrast enhancement algorithms
    mContrastEnhancementComboBox->addItem( tr( "No Enhancement" ), QgsContrastEnhancement::NoEnhancement );
    mContrastEnhancementComboBox->addItem( tr( "Stretch to MinMax" ), QgsContrastEnhancement::StretchToMinimumMaximum );
    mContrastEnhancementComboBox->addItem( tr( "Stretch and Clip to MinMax" ), QgsContrastEnhancement::StretchAndClipToMinimumMaximum );
    mContrastEnhancementComboBox->addItem( tr( "Clip to MinMax" ), QgsContrastEnhancement::ClipToMinimumMaximum );

    setFromRenderer( layer->renderer() );

    connect( mGrayBandComboBox, &QgsRasterBandComboBox::bandChanged, this, &QgsSingleBandGrayRendererWidget::bandChanged );
    connect( mGradientComboBox, static_cast<void ( QComboBox::* )( int )>( &QComboBox::currentIndexChanged ), this, &QgsRasterRendererWidget::widgetChanged );
    connect( mContrastEnhancementComboBox, static_cast<void ( QComboBox::* )( int )>( &QComboBox::currentIndexChanged ), this, &QgsRasterRendererWidget::widgetChanged );
  }
}

QgsRasterRenderer *QgsSingleBandGrayRendererWidget::renderer()
{
  if ( !mRasterLayer )
  {
    return nullptr;
  }
  QgsRasterDataProvider *provider = mRasterLayer->dataProvider();
  if ( !provider )
  {
    return nullptr;
  }
  const int band = mGrayBandComboBox->currentBand();

  QgsContrastEnhancement *e = new QgsContrastEnhancement( ( Qgis::DataType )(
        provider->dataType( band ) ) );
  e->setMinimumValue( QgsDoubleValidator::toDouble( mMinLineEdit->text() ) );
  e->setMaximumValue( QgsDoubleValidator::toDouble( mMaxLineEdit->text() ) );
  e->setContrastEnhancementAlgorithm( ( QgsContrastEnhancement::ContrastEnhancementAlgorithm )( mContrastEnhancementComboBox->currentData().toInt() ) );

  QgsSingleBandGrayRenderer *renderer = new QgsSingleBandGrayRenderer( provider, band );
  renderer->setContrastEnhancement( e );

  renderer->setGradient( ( QgsSingleBandGrayRenderer::Gradient ) mGradientComboBox->currentData().toInt() );
  renderer->setMinMaxOrigin( mMinMaxWidget->minMaxOrigin() );

  renderer->setLegendSettings( new QgsColorRampLegendNodeSettings( mLegendSettings ) );

  return renderer;
}

void QgsSingleBandGrayRendererWidget::doComputations()
{
  mMinMaxWidget->doComputations();
}

void QgsSingleBandGrayRendererWidget::setMapCanvas( QgsMapCanvas *canvas )
{
  QgsRasterRendererWidget::setMapCanvas( canvas );
  mMinMaxWidget->setMapCanvas( canvas );
}

void QgsSingleBandGrayRendererWidget::mMinLineEdit_textChanged( const QString & )
{
  minMaxModified();
}

void QgsSingleBandGrayRendererWidget::mMaxLineEdit_textChanged( const QString & )
{
  minMaxModified();
}

void QgsSingleBandGrayRendererWidget::minMaxModified()
{
  if ( !mDisableMinMaxWidgetRefresh )
  {
    if ( ( QgsContrastEnhancement::ContrastEnhancementAlgorithm )( mContrastEnhancementComboBox->currentData().toInt() ) == QgsContrastEnhancement::NoEnhancement )
    {
      mContrastEnhancementComboBox->setCurrentIndex(
        mContrastEnhancementComboBox->findData( ( int ) QgsContrastEnhancement::StretchToMinimumMaximum ) );
    }
    mMinMaxWidget->userHasSetManualMinMaxValues();
    emit widgetChanged();
  }
}


void QgsSingleBandGrayRendererWidget::loadMinMax( int bandNo, double min, double max )
{
  Q_UNUSED( bandNo )

  QgsDebugMsg( QStringLiteral( "theBandNo = %1 min = %2 max = %3" ).arg( bandNo ).arg( min ).arg( max ) );

  mDisableMinMaxWidgetRefresh = true;
  if ( std::isnan( min ) )
  {
    mMinLineEdit->clear();
  }
  else
  {
    mMinLineEdit->setText( QLocale().toString( min ) );
  }

  if ( std::isnan( max ) )
  {
    mMaxLineEdit->clear();
  }
  else
  {
    mMaxLineEdit->setText( QLocale().toString( max ) );
  }
  mDisableMinMaxWidgetRefresh = false;
}

void QgsSingleBandGrayRendererWidget::bandChanged()
{
  QList<int> myBands;
  myBands.append( mGrayBandComboBox->currentBand() );
  mMinMaxWidget->setBands( myBands );
  emit widgetChanged();
}

void QgsSingleBandGrayRendererWidget::setFromRenderer( const QgsRasterRenderer *r )
{
  const QgsSingleBandGrayRenderer *gr = dynamic_cast<const QgsSingleBandGrayRenderer *>( r );
  if ( gr )
  {
    //band
    mGrayBandComboBox->setBand( gr->grayBand() );
    mMinMaxWidget->setBands( QList< int >() << gr->grayBand() );
    mGradientComboBox->setCurrentIndex( mGradientComboBox->findData( gr->gradient() ) );

    const QgsContrastEnhancement *ce = gr->contrastEnhancement();
    if ( ce )
    {
      //minmax
      mDisableMinMaxWidgetRefresh = true;
      mMinLineEdit->setText( QLocale().toString( ce->minimumValue() ) );
      mMaxLineEdit->setText( QLocale().toString( ce->maximumValue() ) );
      mDisableMinMaxWidgetRefresh = false;
      //contrast enhancement algorithm
      mContrastEnhancementComboBox->setCurrentIndex(
        mContrastEnhancementComboBox->findData( ( int )( ce->contrastEnhancementAlgorithm() ) ) );
    }

    mMinMaxWidget->setFromMinMaxOrigin( gr->minMaxOrigin() );

    if ( gr->legendSettings() )
      mLegendSettings = *gr->legendSettings();
  }
}

void QgsSingleBandGrayRendererWidget::setMin( const QString &value, int )
{
  mDisableMinMaxWidgetRefresh = true;
  mMinLineEdit->setText( value );
  mDisableMinMaxWidgetRefresh = false;
}

void QgsSingleBandGrayRendererWidget::setMax( const QString &value, int )
{
  mDisableMinMaxWidgetRefresh = true;
  mMaxLineEdit->setText( value );
  mDisableMinMaxWidgetRefresh = false;
}

void QgsSingleBandGrayRendererWidget::showLegendSettings()
{
  QgsPanelWidget *panel = QgsPanelWidget::findParentPanel( qobject_cast< QWidget * >( parent() ) );
  if ( panel && panel->dockMode() )
  {
    QgsColorRampLegendNodeWidget *legendPanel = new QgsColorRampLegendNodeWidget();
    legendPanel->setUseContinuousRampCheckBoxVisibility( false );
    legendPanel->setPanelTitle( tr( "Legend Settings" ) );
    legendPanel->setSettings( mLegendSettings );
    connect( legendPanel, &QgsColorRampLegendNodeWidget::widgetChanged, this, [ = ]
    {
      mLegendSettings = legendPanel->settings();
      emit widgetChanged();
    } );
    panel->openPanel( legendPanel );
  }
  else
  {
    QgsColorRampLegendNodeDialog dialog( mLegendSettings, this );
    dialog.setUseContinuousRampCheckBoxVisibility( false );
    dialog.setWindowTitle( tr( "Legend Settings" ) );
    if ( dialog.exec() )
    {
      mLegendSettings = dialog.settings();
      emit widgetChanged();
    }
  }
}

QgsContrastEnhancement::ContrastEnhancementAlgorithm QgsSingleBandGrayRendererWidget::contrastEnhancementAlgorithm() const
{
  return static_cast<QgsContrastEnhancement::ContrastEnhancementAlgorithm>( mContrastEnhancementComboBox->currentData().toInt() );
}

void QgsSingleBandGrayRendererWidget::setContrastEnhancementAlgorithm( QgsContrastEnhancement::ContrastEnhancementAlgorithm algorithm )
{
  mDisableMinMaxWidgetRefresh = true;
  mContrastEnhancementComboBox->setCurrentIndex( mContrastEnhancementComboBox->findData( static_cast<int>( algorithm ) ) );
  mDisableMinMaxWidgetRefresh = false;
}

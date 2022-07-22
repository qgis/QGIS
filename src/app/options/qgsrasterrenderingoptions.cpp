/***************************************************************************
    qgsrasterrenderingoptions.cpp
    -------------------------
    begin                : July 2021
    copyright            : (C) 2021 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsrasterrenderingoptions.h"
#include "qgssettings.h"
#include "qgsapplication.h"
#include "qgssettingsregistrycore.h"
#include "qgscontrastenhancement.h"
#include "qgsrasterminmaxorigin.h"
#include "qgsrasterlayer.h"

//
// QgsRasterRenderingOptionsWidget
//

QgsRasterRenderingOptionsWidget::QgsRasterRenderingOptionsWidget( QWidget *parent )
  : QgsOptionsPageWidget( parent )
{
  setupUi( this );

  QgsSettings settings;
  spnRed->setValue( settings.value( QStringLiteral( "/Raster/defaultRedBand" ), 1 ).toInt() );
  spnRed->setClearValue( 1 );
  spnGreen->setValue( settings.value( QStringLiteral( "/Raster/defaultGreenBand" ), 2 ).toInt() );
  spnGreen->setClearValue( 2 );
  spnBlue->setValue( settings.value( QStringLiteral( "/Raster/defaultBlueBand" ), 3 ).toInt() );
  spnBlue->setClearValue( 3 );

  mZoomedInResamplingComboBox->insertItem( 0, tr( "Nearest Neighbour" ), QStringLiteral( "nearest neighbour" ) );
  mZoomedInResamplingComboBox->insertItem( 1, tr( "Bilinear" ), QStringLiteral( "bilinear" ) );
  mZoomedInResamplingComboBox->insertItem( 2, tr( "Cubic" ), QStringLiteral( "cubic" ) );

  mZoomedOutResamplingComboBox->insertItem( 0, tr( "Nearest Neighbour" ), QStringLiteral( "nearest neighbour" ) );
  mZoomedOutResamplingComboBox->insertItem( 1, tr( "Bilinear" ), QStringLiteral( "bilinear" ) );
  mZoomedOutResamplingComboBox->insertItem( 2, tr( "Cubic" ), QStringLiteral( "cubic" ) );

  QString zoomedInResampling = settings.value( QStringLiteral( "/Raster/defaultZoomedInResampling" ), QStringLiteral( "nearest neighbour" ) ).toString();
  mZoomedInResamplingComboBox->setCurrentIndex( mZoomedInResamplingComboBox->findData( zoomedInResampling ) );
  QString zoomedOutResampling = settings.value( QStringLiteral( "/Raster/defaultZoomedOutResampling" ), QStringLiteral( "nearest neighbour" ) ).toString();
  mZoomedOutResamplingComboBox->setCurrentIndex( mZoomedOutResamplingComboBox->findData( zoomedOutResampling ) );

  spnOversampling->setValue( settings.value( QStringLiteral( "/Raster/defaultOversampling" ), 2.0 ).toDouble() );
  spnOversampling->setClearValue( 2.0 );
  mCbEarlyResampling->setChecked( settings.value( QStringLiteral( "/Raster/defaultEarlyResampling" ), false ).toBool() );

  initContrastEnhancement( cboxContrastEnhancementAlgorithmSingleBand, QStringLiteral( "singleBand" ),
                           QgsContrastEnhancement::contrastEnhancementAlgorithmString( QgsRasterLayer::SINGLE_BAND_ENHANCEMENT_ALGORITHM ) );
  initContrastEnhancement( cboxContrastEnhancementAlgorithmMultiBandSingleByte, QStringLiteral( "multiBandSingleByte" ),
                           QgsContrastEnhancement::contrastEnhancementAlgorithmString( QgsRasterLayer::MULTIPLE_BAND_SINGLE_BYTE_ENHANCEMENT_ALGORITHM ) );
  initContrastEnhancement( cboxContrastEnhancementAlgorithmMultiBandMultiByte, QStringLiteral( "multiBandMultiByte" ),
                           QgsContrastEnhancement::contrastEnhancementAlgorithmString( QgsRasterLayer::MULTIPLE_BAND_MULTI_BYTE_ENHANCEMENT_ALGORITHM ) );

  initMinMaxLimits( cboxContrastEnhancementLimitsSingleBand, QStringLiteral( "singleBand" ),
                    QgsRasterMinMaxOrigin::limitsString( QgsRasterLayer::SINGLE_BAND_MIN_MAX_LIMITS ) );
  initMinMaxLimits( cboxContrastEnhancementLimitsMultiBandSingleByte, QStringLiteral( "multiBandSingleByte" ),
                    QgsRasterMinMaxOrigin::limitsString( QgsRasterLayer::MULTIPLE_BAND_SINGLE_BYTE_MIN_MAX_LIMITS ) );
  initMinMaxLimits( cboxContrastEnhancementLimitsMultiBandMultiByte, QStringLiteral( "multiBandMultiByte" ),
                    QgsRasterMinMaxOrigin::limitsString( QgsRasterLayer::MULTIPLE_BAND_MULTI_BYTE_MIN_MAX_LIMITS ) );

  mRasterCumulativeCutLowerDoubleSpinBox->setValue( 100.0 * settings.value( QStringLiteral( "/Raster/cumulativeCutLower" ), QString::number( QgsRasterMinMaxOrigin::CUMULATIVE_CUT_LOWER ) ).toDouble() );
  mRasterCumulativeCutLowerDoubleSpinBox->setClearValue( QgsRasterMinMaxOrigin::CUMULATIVE_CUT_LOWER * 100 );
  mRasterCumulativeCutUpperDoubleSpinBox->setValue( 100.0 * settings.value( QStringLiteral( "/Raster/cumulativeCutUpper" ), QString::number( QgsRasterMinMaxOrigin::CUMULATIVE_CUT_UPPER ) ).toDouble() );
  mRasterCumulativeCutUpperDoubleSpinBox->setClearValue( QgsRasterMinMaxOrigin::CUMULATIVE_CUT_UPPER * 100 );

  spnThreeBandStdDev->setValue( settings.value( QStringLiteral( "/Raster/defaultStandardDeviation" ), QgsRasterMinMaxOrigin::DEFAULT_STDDEV_FACTOR ).toDouble() );
  spnThreeBandStdDev->setClearValue( QgsRasterMinMaxOrigin::DEFAULT_STDDEV_FACTOR );
}

void QgsRasterRenderingOptionsWidget::apply()
{
  QgsSettings settings;

  settings.setValue( QStringLiteral( "/Raster/defaultRedBand" ), spnRed->value() );
  settings.setValue( QStringLiteral( "/Raster/defaultGreenBand" ), spnGreen->value() );
  settings.setValue( QStringLiteral( "/Raster/defaultBlueBand" ), spnBlue->value() );

  settings.setValue( QStringLiteral( "/Raster/defaultZoomedInResampling" ), mZoomedInResamplingComboBox->currentData().toString() );
  settings.setValue( QStringLiteral( "/Raster/defaultZoomedOutResampling" ), mZoomedOutResamplingComboBox->currentData().toString() );

  settings.setValue( QStringLiteral( "/Raster/defaultOversampling" ), spnOversampling->value() );
  settings.setValue( QStringLiteral( "/Raster/defaultEarlyResampling" ), mCbEarlyResampling->isChecked() );

  saveContrastEnhancement( cboxContrastEnhancementAlgorithmSingleBand, QStringLiteral( "singleBand" ) );
  saveContrastEnhancement( cboxContrastEnhancementAlgorithmMultiBandSingleByte, QStringLiteral( "multiBandSingleByte" ) );
  saveContrastEnhancement( cboxContrastEnhancementAlgorithmMultiBandMultiByte, QStringLiteral( "multiBandMultiByte" ) );

  saveMinMaxLimits( cboxContrastEnhancementLimitsSingleBand, QStringLiteral( "singleBand" ) );
  saveMinMaxLimits( cboxContrastEnhancementLimitsMultiBandSingleByte, QStringLiteral( "multiBandSingleByte" ) );
  saveMinMaxLimits( cboxContrastEnhancementLimitsMultiBandMultiByte, QStringLiteral( "multiBandMultiByte" ) );

  settings.setValue( QStringLiteral( "/Raster/cumulativeCutLower" ), mRasterCumulativeCutLowerDoubleSpinBox->value() / 100.0 );
  settings.setValue( QStringLiteral( "/Raster/cumulativeCutUpper" ), mRasterCumulativeCutUpperDoubleSpinBox->value() / 100.0 );

  settings.setValue( QStringLiteral( "/Raster/defaultStandardDeviation" ), spnThreeBandStdDev->value() );
}

void QgsRasterRenderingOptionsWidget::initContrastEnhancement( QComboBox *cbox, const QString &name, const QString &defaultVal )
{
  QgsSettings settings;

  //add items to the color enhanceContrast combo boxes
  cbox->addItem( tr( "No Stretch" ),
                 QgsContrastEnhancement::contrastEnhancementAlgorithmString( QgsContrastEnhancement::NoEnhancement ) );
  cbox->addItem( tr( "Stretch to MinMax" ),
                 QgsContrastEnhancement::contrastEnhancementAlgorithmString( QgsContrastEnhancement::StretchToMinimumMaximum ) );
  cbox->addItem( tr( "Stretch and Clip to MinMax" ),
                 QgsContrastEnhancement::contrastEnhancementAlgorithmString( QgsContrastEnhancement::StretchAndClipToMinimumMaximum ) );
  cbox->addItem( tr( "Clip to MinMax" ),
                 QgsContrastEnhancement::contrastEnhancementAlgorithmString( QgsContrastEnhancement::ClipToMinimumMaximum ) );

  QString contrastEnhancement = settings.value( "/Raster/defaultContrastEnhancementAlgorithm/" + name, defaultVal ).toString();
  cbox->setCurrentIndex( cbox->findData( contrastEnhancement ) );
}

void QgsRasterRenderingOptionsWidget::saveContrastEnhancement( QComboBox *cbox, const QString &name )
{
  QgsSettings settings;
  QString value = cbox->currentData().toString();
  settings.setValue( "/Raster/defaultContrastEnhancementAlgorithm/" + name, value );
}

void QgsRasterRenderingOptionsWidget::initMinMaxLimits( QComboBox *cbox, const QString &name, const QString &defaultVal )
{
  QgsSettings settings;

  //add items to the color limitsContrast combo boxes
  cbox->addItem( tr( "Cumulative Pixel Count Cut" ),
                 QgsRasterMinMaxOrigin::limitsString( QgsRasterMinMaxOrigin::CumulativeCut ) );
  cbox->addItem( tr( "Minimum / Maximum" ),
                 QgsRasterMinMaxOrigin::limitsString( QgsRasterMinMaxOrigin::MinMax ) );
  cbox->addItem( tr( "Mean +/- Standard Deviation" ),
                 QgsRasterMinMaxOrigin::limitsString( QgsRasterMinMaxOrigin::StdDev ) );

  QString contrastLimits = settings.value( "/Raster/defaultContrastEnhancementLimits/" + name, defaultVal ).toString();
  cbox->setCurrentIndex( cbox->findData( contrastLimits ) );
}

void QgsRasterRenderingOptionsWidget::saveMinMaxLimits( QComboBox *cbox, const QString &name )
{
  QgsSettings settings;
  QString value = cbox->currentData().toString();
  settings.setValue( "/Raster/defaultContrastEnhancementLimits/" + name, value );
}


//
// QgsRasterRenderingOptionsFactory
//
QgsRasterRenderingOptionsFactory::QgsRasterRenderingOptionsFactory()
  : QgsOptionsWidgetFactory( tr( "Raster" ), QIcon() )
{

}

QIcon QgsRasterRenderingOptionsFactory::icon() const
{
  return QgsApplication::getThemeIcon( QStringLiteral( "mIconRaster.svg" ) );
}

QgsOptionsPageWidget *QgsRasterRenderingOptionsFactory::createWidget( QWidget *parent ) const
{
  return new QgsRasterRenderingOptionsWidget( parent );
}

QStringList QgsRasterRenderingOptionsFactory::path() const
{
  return {QStringLiteral( "rendering" ) };
}

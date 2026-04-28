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

#include "qgsapplication.h"
#include "qgscontrastenhancement.h"
#include "qgsrasterlayer.h"
#include "qgsrasterminmaxorigin.h"
#include "qgsrasterrendererregistry.h"
#include "qgssettings.h"
#include "qgssettingsentryimpl.h"
#include "qgssettingsregistrycore.h"

#include <QString>

#include "moc_qgsrasterrenderingoptions.cpp"

using namespace Qt::StringLiterals;

//
// QgsRasterRenderingOptionsWidget
//

QgsRasterRenderingOptionsWidget::QgsRasterRenderingOptionsWidget( QWidget *parent )
  : QgsOptionsPageWidget( parent )
{
  setupUi( this );

  QgsSettings settings;
  spnRed->setValue( settings.value( u"/Raster/defaultRedBand"_s, 1 ).toInt() );
  spnRed->setClearValue( 1 );
  spnGreen->setValue( settings.value( u"/Raster/defaultGreenBand"_s, 2 ).toInt() );
  spnGreen->setClearValue( 2 );
  spnBlue->setValue( settings.value( u"/Raster/defaultBlueBand"_s, 3 ).toInt() );
  spnBlue->setClearValue( 3 );

  mZoomedInResamplingComboBox->insertItem( 0, tr( "Nearest Neighbour" ), u"nearest neighbour"_s );
  mZoomedInResamplingComboBox->insertItem( 1, tr( "Bilinear (2x2 Kernel)" ), u"bilinear"_s );
  mZoomedInResamplingComboBox->insertItem( 2, tr( "Cubic (4x4 Kernel)" ), u"cubic"_s );

  mZoomedOutResamplingComboBox->insertItem( 0, tr( "Nearest Neighbour" ), u"nearest neighbour"_s );
  mZoomedOutResamplingComboBox->insertItem( 1, tr( "Bilinear (2x2 Kernel)" ), u"bilinear"_s );
  mZoomedOutResamplingComboBox->insertItem( 2, tr( "Cubic (4x4 Kernel)" ), u"cubic"_s );

  QString zoomedInResampling = QgsRasterLayer::settingsRasterDefaultZoomedInResampling->value();
  mZoomedInResamplingComboBox->setCurrentIndex( mZoomedInResamplingComboBox->findData( zoomedInResampling ) );
  QString zoomedOutResampling = QgsRasterLayer::settingsRasterDefaultZoomedOutResampling->value();
  mZoomedOutResamplingComboBox->setCurrentIndex( mZoomedOutResamplingComboBox->findData( zoomedOutResampling ) );

  spnOversampling->setValue( QgsRasterLayer::settingsRasterDefaultOversampling->value() );
  spnOversampling->setClearValue( 2.0 );
  mCbEarlyResampling->setChecked( QgsRasterLayer::settingsRasterDefaultEarlyResampling->value() );

  initContrastEnhancement( cboxContrastEnhancementAlgorithmSingleBand, u"singleBand"_s, QgsContrastEnhancement::contrastEnhancementAlgorithmString( QgsRasterLayer::SINGLE_BAND_ENHANCEMENT_ALGORITHM ) );
  initContrastEnhancement( cboxContrastEnhancementAlgorithmMultiBandSingleByte, u"multiBandSingleByte"_s, QgsContrastEnhancement::contrastEnhancementAlgorithmString( QgsRasterLayer::MULTIPLE_BAND_SINGLE_BYTE_ENHANCEMENT_ALGORITHM ) );
  initContrastEnhancement( cboxContrastEnhancementAlgorithmMultiBandMultiByte, u"multiBandMultiByte"_s, QgsContrastEnhancement::contrastEnhancementAlgorithmString( QgsRasterLayer::MULTIPLE_BAND_MULTI_BYTE_ENHANCEMENT_ALGORITHM ) );

  initMinMaxLimits( cboxContrastEnhancementLimitsSingleBand, u"singleBand"_s, QgsRasterMinMaxOrigin::limitsString( QgsRasterLayer::SINGLE_BAND_MIN_MAX_LIMITS ) );
  initMinMaxLimits( cboxContrastEnhancementLimitsMultiBandSingleByte, u"multiBandSingleByte"_s, QgsRasterMinMaxOrigin::limitsString( QgsRasterLayer::MULTIPLE_BAND_SINGLE_BYTE_MIN_MAX_LIMITS ) );
  initMinMaxLimits( cboxContrastEnhancementLimitsMultiBandMultiByte, u"multiBandMultiByte"_s, QgsRasterMinMaxOrigin::limitsString( QgsRasterLayer::MULTIPLE_BAND_MULTI_BYTE_MIN_MAX_LIMITS ) );

  mRasterCumulativeCutLowerDoubleSpinBox->setValue( 100.0 * QgsRasterMinMaxOrigin::settingsCumulativeCutLower->value() );
  mRasterCumulativeCutLowerDoubleSpinBox->setClearValue( QgsRasterMinMaxOrigin::CUMULATIVE_CUT_LOWER * 100 );
  mRasterCumulativeCutUpperDoubleSpinBox->setValue( 100.0 * QgsRasterMinMaxOrigin::settingsCumulativeCutUpper->value() );
  mRasterCumulativeCutUpperDoubleSpinBox->setClearValue( QgsRasterMinMaxOrigin::CUMULATIVE_CUT_UPPER * 100 );

  spnThreeBandStdDev->setValue( QgsRasterRendererRegistry::settingsDefaultStandardDeviation->value() );
  spnThreeBandStdDev->setClearValue( QgsRasterMinMaxOrigin::DEFAULT_STDDEV_FACTOR );
}

QString QgsRasterRenderingOptionsWidget::helpKey() const
{
  return u"introduction/qgis_configuration.html#raster-rendering-options"_s;
}

void QgsRasterRenderingOptionsWidget::apply()
{
  QgsSettings settings;

  settings.setValue( u"/Raster/defaultRedBand"_s, spnRed->value() );
  settings.setValue( u"/Raster/defaultGreenBand"_s, spnGreen->value() );
  settings.setValue( u"/Raster/defaultBlueBand"_s, spnBlue->value() );

  QgsRasterLayer::settingsRasterDefaultZoomedInResampling->setValue( mZoomedInResamplingComboBox->currentData().toString() );
  QgsRasterLayer::settingsRasterDefaultZoomedOutResampling->setValue( mZoomedOutResamplingComboBox->currentData().toString() );

  QgsRasterLayer::settingsRasterDefaultOversampling->setValue( spnOversampling->value() );
  QgsRasterLayer::settingsRasterDefaultEarlyResampling->setValue( mCbEarlyResampling->isChecked() );

  saveContrastEnhancement( cboxContrastEnhancementAlgorithmSingleBand, u"singleBand"_s );
  saveContrastEnhancement( cboxContrastEnhancementAlgorithmMultiBandSingleByte, u"multiBandSingleByte"_s );
  saveContrastEnhancement( cboxContrastEnhancementAlgorithmMultiBandMultiByte, u"multiBandMultiByte"_s );

  saveMinMaxLimits( cboxContrastEnhancementLimitsSingleBand, u"singleBand"_s );
  saveMinMaxLimits( cboxContrastEnhancementLimitsMultiBandSingleByte, u"multiBandSingleByte"_s );
  saveMinMaxLimits( cboxContrastEnhancementLimitsMultiBandMultiByte, u"multiBandMultiByte"_s );

  QgsRasterMinMaxOrigin::settingsCumulativeCutLower->setValue( mRasterCumulativeCutLowerDoubleSpinBox->value() / 100.0 );
  QgsRasterMinMaxOrigin::settingsCumulativeCutUpper->setValue( mRasterCumulativeCutUpperDoubleSpinBox->value() / 100.0 );

  QgsRasterRendererRegistry::settingsDefaultStandardDeviation->setValue( spnThreeBandStdDev->value() );
}

void QgsRasterRenderingOptionsWidget::initContrastEnhancement( QComboBox *cbox, const QString &name, const QString &defaultVal )
{
  QgsSettings settings;

  //add items to the color enhanceContrast combo boxes
  cbox->addItem( tr( "No Stretch" ), QgsContrastEnhancement::contrastEnhancementAlgorithmString( QgsContrastEnhancement::NoEnhancement ) );
  cbox->addItem( tr( "Stretch to MinMax" ), QgsContrastEnhancement::contrastEnhancementAlgorithmString( QgsContrastEnhancement::StretchToMinimumMaximum ) );
  cbox->addItem( tr( "Stretch and Clip to MinMax" ), QgsContrastEnhancement::contrastEnhancementAlgorithmString( QgsContrastEnhancement::StretchAndClipToMinimumMaximum ) );
  cbox->addItem( tr( "Clip to MinMax" ), QgsContrastEnhancement::contrastEnhancementAlgorithmString( QgsContrastEnhancement::ClipToMinimumMaximum ) );

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
  cbox->addItem( tr( "Cumulative Pixel Count Cut" ), QgsRasterMinMaxOrigin::limitsString( Qgis::RasterRangeLimit::CumulativeCut ) );
  cbox->addItem( tr( "Minimum / Maximum" ), QgsRasterMinMaxOrigin::limitsString( Qgis::RasterRangeLimit::MinimumMaximum ) );
  cbox->addItem( tr( "Mean +/- Standard Deviation" ), QgsRasterMinMaxOrigin::limitsString( Qgis::RasterRangeLimit::StdDev ) );

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
  : QgsOptionsWidgetFactory( tr( "Raster" ), QIcon(), u"raster"_s )
{}

QIcon QgsRasterRenderingOptionsFactory::icon() const
{
  return QgsApplication::getThemeIcon( u"mIconRaster.svg"_s );
}

QgsOptionsPageWidget *QgsRasterRenderingOptionsFactory::createWidget( QWidget *parent ) const
{
  return new QgsRasterRenderingOptionsWidget( parent );
}

QStringList QgsRasterRenderingOptionsFactory::path() const
{
  return { u"rendering"_s };
}

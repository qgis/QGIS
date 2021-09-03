/***************************************************************************
                         qgsrasterminmaxwidget.h
                         ---------------------------------
    begin                : July 2012
    copyright            : (C) 2012 by Radim Blazek
    email                : radim dot blazek at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <QSettings>
#include <QMessageBox>

#include "qgsrasterlayer.h"
#include "qgsrasterminmaxwidget.h"
#include "qgsmapcanvas.h"
#include "qgsrasterrenderer.h"
#include "qgsrasterdataprovider.h"
#include "qgsrasterminmaxorigin.h"
#include "qgsdoublespinbox.h"

const int IDX_WHOLE_RASTER = 0;
const int IDX_CURRENT_CANVAS = 1;
const int IDX_UPDATED_CANVAS = 2;

QgsRasterMinMaxWidget::QgsRasterMinMaxWidget( QgsRasterLayer *layer, QWidget *parent )
  : QWidget( parent )
  , mLayer( layer )
  , mLastRectangleValid( false )
  , mBandsChanged( false )
{
  QgsDebugMsgLevel( QStringLiteral( "Entered." ), 4 );
  setupUi( this );

  // use maximum value as a clear value for the upper border of the cumulative cut
  mCumulativeCutUpperDoubleSpinBox->setClearValueMode( QgsDoubleSpinBox::MaximumValue );

  connect( mUserDefinedRadioButton, &QRadioButton::toggled, this, &QgsRasterMinMaxWidget::mUserDefinedRadioButton_toggled );
  connect( mMinMaxRadioButton, &QRadioButton::toggled, this, &QgsRasterMinMaxWidget::mMinMaxRadioButton_toggled );
  connect( mStdDevRadioButton, &QRadioButton::toggled, this, &QgsRasterMinMaxWidget::mStdDevRadioButton_toggled );
  connect( mCumulativeCutRadioButton, &QRadioButton::toggled, this, &QgsRasterMinMaxWidget::mCumulativeCutRadioButton_toggled );
  connect( mStatisticsExtentCombo, static_cast<void ( QComboBox::* )( int )>( &QComboBox::currentIndexChanged ), this, &QgsRasterMinMaxWidget::mStatisticsExtentCombo_currentIndexChanged );
  connect( mCumulativeCutLowerDoubleSpinBox, static_cast < void ( QDoubleSpinBox::* )( double ) > ( &QDoubleSpinBox::valueChanged ), this, &QgsRasterMinMaxWidget::mCumulativeCutLowerDoubleSpinBox_valueChanged );
  connect( mCumulativeCutUpperDoubleSpinBox, static_cast < void ( QDoubleSpinBox::* )( double ) > ( &QDoubleSpinBox::valueChanged ), this, &QgsRasterMinMaxWidget::mCumulativeCutUpperDoubleSpinBox_valueChanged );
  connect( mStdDevSpinBox, static_cast < void ( QDoubleSpinBox::* )( double ) > ( &QDoubleSpinBox::valueChanged ), this, &QgsRasterMinMaxWidget::mStdDevSpinBox_valueChanged );
  connect( cboAccuracy, static_cast<void ( QComboBox::* )( int )>( &QComboBox::currentIndexChanged ), this, &QgsRasterMinMaxWidget::cboAccuracy_currentIndexChanged );

  const QgsRasterMinMaxOrigin defaultMinMaxOrigin;
  setFromMinMaxOrigin( defaultMinMaxOrigin );
}

void QgsRasterMinMaxWidget::setMapCanvas( QgsMapCanvas *canvas )
{
  mCanvas = canvas;
}

QgsMapCanvas *QgsRasterMinMaxWidget::mapCanvas()
{
  return mCanvas;
}

void QgsRasterMinMaxWidget::setBands( const QList<int> &bands )
{
  mBandsChanged = bands != mBands;
  mBands = bands;
}

QgsRectangle QgsRasterMinMaxWidget::extent()
{
  const int nExtentIdx = mStatisticsExtentCombo->currentIndex();
  if ( nExtentIdx != IDX_CURRENT_CANVAS && nExtentIdx != IDX_UPDATED_CANVAS )
    return QgsRectangle();

  if ( mLayer && mCanvas )
    return mCanvas->mapSettings().outputExtentToLayerExtent( mLayer, mCanvas->extent() );
  else if ( mCanvas )
    return mCanvas->extent();
  else
    return QgsRectangle();
}

void QgsRasterMinMaxWidget::userHasSetManualMinMaxValues()
{
  mUserDefinedRadioButton->setChecked( true );
  mStatisticsExtentCombo->setCurrentIndex( IDX_WHOLE_RASTER );
}

void QgsRasterMinMaxWidget::mUserDefinedRadioButton_toggled( bool toggled )
{
  mStatisticsExtentCombo->setEnabled( !toggled );
  cboAccuracy->setEnabled( !toggled );
  emit widgetChanged();
}

void QgsRasterMinMaxWidget::setFromMinMaxOrigin( const QgsRasterMinMaxOrigin &minMaxOrigin )
{
  switch ( minMaxOrigin.limits() )
  {
    case QgsRasterMinMaxOrigin::None:
      mUserDefinedRadioButton->setChecked( true );
      break;

    case QgsRasterMinMaxOrigin::MinMax:
      mMinMaxRadioButton->setChecked( true );
      break;

    case QgsRasterMinMaxOrigin::StdDev:
      mStdDevRadioButton->setChecked( true );
      break;

    case QgsRasterMinMaxOrigin::CumulativeCut:
      mCumulativeCutRadioButton->setChecked( true );
      break;
  }

  switch ( minMaxOrigin.extent() )
  {
    case QgsRasterMinMaxOrigin::WholeRaster:
      mStatisticsExtentCombo->setCurrentIndex( IDX_WHOLE_RASTER );
      break;

    case QgsRasterMinMaxOrigin::CurrentCanvas:
      mStatisticsExtentCombo->setCurrentIndex( IDX_CURRENT_CANVAS );
      break;

    case QgsRasterMinMaxOrigin::UpdatedCanvas:
      mStatisticsExtentCombo->setCurrentIndex( IDX_UPDATED_CANVAS );
      break;
  }

  mCumulativeCutLowerDoubleSpinBox->setValue( 100.0 * minMaxOrigin.cumulativeCutLower() );
  mCumulativeCutUpperDoubleSpinBox->setValue( 100.0 * minMaxOrigin.cumulativeCutUpper() );
  mStdDevSpinBox->setValue( minMaxOrigin.stdDevFactor() );

  cboAccuracy->setCurrentIndex( minMaxOrigin.statAccuracy() == QgsRasterMinMaxOrigin::Estimated ? 0 : 1 );
}

QgsRasterMinMaxOrigin QgsRasterMinMaxWidget::minMaxOrigin()
{
  QgsRasterMinMaxOrigin minMaxOrigin;

  if ( mMinMaxRadioButton->isChecked() )
    minMaxOrigin.setLimits( QgsRasterMinMaxOrigin::MinMax );
  else if ( mStdDevRadioButton->isChecked() )
    minMaxOrigin.setLimits( QgsRasterMinMaxOrigin::StdDev );
  else if ( mCumulativeCutRadioButton->isChecked() )
    minMaxOrigin.setLimits( QgsRasterMinMaxOrigin::CumulativeCut );
  else
    minMaxOrigin.setLimits( QgsRasterMinMaxOrigin::None );

  switch ( mStatisticsExtentCombo->currentIndex() )
  {
    case IDX_WHOLE_RASTER:
    default:
      minMaxOrigin.setExtent( QgsRasterMinMaxOrigin::WholeRaster );
      break;
    case IDX_CURRENT_CANVAS:
      minMaxOrigin.setExtent( QgsRasterMinMaxOrigin::CurrentCanvas );
      break;
    case IDX_UPDATED_CANVAS:
      minMaxOrigin.setExtent( QgsRasterMinMaxOrigin::UpdatedCanvas );
      break;
  }

  if ( cboAccuracy->currentIndex() == 0 )
    minMaxOrigin.setStatAccuracy( QgsRasterMinMaxOrigin::Estimated );
  else
    minMaxOrigin.setStatAccuracy( QgsRasterMinMaxOrigin::Exact );

  minMaxOrigin.setCumulativeCutLower(
    mCumulativeCutLowerDoubleSpinBox->value() / 100.0 );
  minMaxOrigin.setCumulativeCutUpper(
    mCumulativeCutUpperDoubleSpinBox->value() / 100.0 );
  minMaxOrigin.setStdDevFactor( mStdDevSpinBox->value() );

  return minMaxOrigin;
}

void QgsRasterMinMaxWidget::doComputations()
{
  QgsDebugMsgLevel( QStringLiteral( "Entered." ), 4 );
  if ( !mLayer->dataProvider() )
    return;

  const QgsRectangle myExtent = extent(); // empty == full
  const int mySampleSize = sampleSize(); // 0 == exact

  const QgsRasterMinMaxOrigin newMinMaxOrigin = minMaxOrigin();
  if ( mLastRectangleValid && mLastRectangle == myExtent &&
       mLastMinMaxOrigin == newMinMaxOrigin &&
       !mBandsChanged )
  {
    QgsDebugMsg( QStringLiteral( "Does not need to redo statistics computations" ) );
    return;
  }

  mLastRectangleValid = true;
  mLastRectangle = myExtent;
  mLastMinMaxOrigin = newMinMaxOrigin;
  mBandsChanged = false;

  const auto constMBands = mBands;
  for ( const int myBand : constMBands )
  {
    QgsDebugMsg( QStringLiteral( "myBand = %1" ).arg( myBand ) );
    if ( myBand < 1 || myBand > mLayer->dataProvider()->bandCount() )
    {
      continue;
    }
    double myMin = std::numeric_limits<double>::quiet_NaN();
    double myMax = std::numeric_limits<double>::quiet_NaN();

    bool updateMinMax = false;
    if ( mCumulativeCutRadioButton->isChecked() )
    {
      updateMinMax = true;
      const double myLower = mCumulativeCutLowerDoubleSpinBox->value() / 100.0;
      const double myUpper = mCumulativeCutUpperDoubleSpinBox->value() / 100.0;
      mLayer->dataProvider()->cumulativeCut( myBand, myLower, myUpper, myMin, myMax, myExtent, mySampleSize );
    }
    else if ( mMinMaxRadioButton->isChecked() )
    {
      updateMinMax = true;
      // TODO: consider provider minimum/maximumValue() (has to be defined well in povider)
      const QgsRasterBandStats myRasterBandStats = mLayer->dataProvider()->bandStatistics( myBand, QgsRasterBandStats::Min | QgsRasterBandStats::Max, myExtent, mySampleSize );
      myMin = myRasterBandStats.minimumValue;
      myMax = myRasterBandStats.maximumValue;
    }
    else if ( mStdDevRadioButton->isChecked() )
    {
      updateMinMax = true;
      const QgsRasterBandStats myRasterBandStats = mLayer->dataProvider()->bandStatistics( myBand, QgsRasterBandStats::Mean | QgsRasterBandStats::StdDev, myExtent, mySampleSize );
      const double myStdDev = mStdDevSpinBox->value();
      myMin = myRasterBandStats.mean - ( myStdDev * myRasterBandStats.stdDev );
      myMax = myRasterBandStats.mean + ( myStdDev * myRasterBandStats.stdDev );
    }

    if ( updateMinMax )
      emit load( myBand, myMin, myMax );
  }
}

void QgsRasterMinMaxWidget::hideUpdatedExtent()
{
  mStatisticsExtentCombo->removeItem( IDX_UPDATED_CANVAS );
}

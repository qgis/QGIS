/***************************************************************************
    qgsmeshrenderer3daveragingwidget.cpp
    ------------------------------------
    begin                : November 2019
    copyright            : (C) 2019 by Peter Petrik
    email                : zilolv at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <memory>
#include <QSvgRenderer>
#include <QPainter>
#include <QPixmap>

#include "qgsmeshrenderer3daveragingwidget.h"
#include "moc_qgsmeshrenderer3daveragingwidget.cpp"

#include "qgis.h"
#include "qgsmeshlayer.h"
#include "qgsmeshrenderersettings.h"
#include "qgsmesh3daveraging.h"
#include "qgsapplication.h"
#include "qgsscreenhelper.h"


QgsMeshRenderer3DAveragingWidget::QgsMeshRenderer3DAveragingWidget( QWidget *parent )
  : QWidget( parent )

{
  setupUi( this );

  mScreenHelper = new QgsScreenHelper( this );

  connect( mAveragingMethodComboBox, qOverload<int>( &QComboBox::currentIndexChanged ), this, &QgsMeshRenderer3DAveragingWidget::onAveragingMethodChanged );

  // Single Level Average Method (top)
  connect( mSingleVerticalLayerIndexTopSpinBox, qOverload<int>( &QgsSpinBox::valueChanged ), this, &QgsMeshRenderer3DAveragingWidget::widgetChanged );

  // Single Level Average Method (bottom)
  connect( mSingleVerticalLayerIndexBottomSpinBox, qOverload<int>( &QgsSpinBox::valueChanged ), this, &QgsMeshRenderer3DAveragingWidget::widgetChanged );

  // Multi Levels Averaging Method (top)
  connect( mMultiTopVerticalLayerStartIndexSpinBox, qOverload<int>( &QgsSpinBox::valueChanged ), this, &QgsMeshRenderer3DAveragingWidget::widgetChanged );
  connect( mMultiTopVerticalLayerEndIndexSpinBox, qOverload<int>( &QgsSpinBox::valueChanged ), this, &QgsMeshRenderer3DAveragingWidget::widgetChanged );

  // MultiLevels Averaging Method (bottom)
  connect( mMultiBottomVerticalLayerStartIndexSpinBox, qOverload<int>( &QgsSpinBox::valueChanged ), this, &QgsMeshRenderer3DAveragingWidget::widgetChanged );
  connect( mMultiBottomVerticalLayerEndIndexSpinBox, qOverload<int>( &QgsSpinBox::valueChanged ), this, &QgsMeshRenderer3DAveragingWidget::widgetChanged );

  // Sigma Averaging Method
  connect( mSigmaStartFractionSpinBox, qOverload<double>( &QgsDoubleSpinBox::valueChanged ), this, &QgsMeshRenderer3DAveragingWidget::widgetChanged );
  connect( mSigmaEndFractionSpinBox, qOverload<double>( &QgsDoubleSpinBox::valueChanged ), this, &QgsMeshRenderer3DAveragingWidget::widgetChanged );

  // Depth Averaging Method
  connect( mDepthStartSpinBox, qOverload<double>( &QgsDoubleSpinBox::valueChanged ), this, &QgsMeshRenderer3DAveragingWidget::widgetChanged );
  connect( mDepthEndSpinBox, qOverload<double>( &QgsDoubleSpinBox::valueChanged ), this, &QgsMeshRenderer3DAveragingWidget::widgetChanged );

  // Height Averaging Method
  connect( mHeightStartSpinBox, qOverload<double>( &QgsDoubleSpinBox::valueChanged ), this, &QgsMeshRenderer3DAveragingWidget::widgetChanged );
  connect( mHeightEndSpinBox, qOverload<double>( &QgsDoubleSpinBox::valueChanged ), this, &QgsMeshRenderer3DAveragingWidget::widgetChanged );


  // Elevation Averaging Method
  connect( mElevationStartSpinBox, qOverload<double>( &QgsDoubleSpinBox::valueChanged ), this, &QgsMeshRenderer3DAveragingWidget::widgetChanged );
  connect( mElevationEndSpinBox, qOverload<double>( &QgsDoubleSpinBox::valueChanged ), this, &QgsMeshRenderer3DAveragingWidget::widgetChanged );

  updateGraphics();
  connect( mScreenHelper, &QgsScreenHelper::screenDpiChanged, this, &QgsMeshRenderer3DAveragingWidget::updateGraphics );
}

void QgsMeshRenderer3DAveragingWidget::setLayer( QgsMeshLayer *layer )
{
  mMeshLayer = layer;
}

std::unique_ptr<QgsMesh3DAveragingMethod> QgsMeshRenderer3DAveragingWidget::averagingMethod() const
{
  std::unique_ptr<QgsMesh3DAveragingMethod> averaging;

  switch ( mAveragingMethodComboBox->currentIndex() )
  {
    case 0: // single level from top
    {
      const int verticalLevel = mSingleVerticalLayerIndexTopSpinBox->value();
      averaging.reset( new QgsMeshMultiLevelsAveragingMethod( verticalLevel, true ) );
      break;
    }
    case 1: // single level from bottom
    {
      const int verticalLevel = mSingleVerticalLayerIndexBottomSpinBox->value();
      averaging.reset( new QgsMeshMultiLevelsAveragingMethod( verticalLevel, false ) );
      break;
    }
    case 2: // multi level from top
    {
      const int startVerticalLevel = mMultiTopVerticalLayerStartIndexSpinBox->value();
      const int endVerticalLevel = mMultiTopVerticalLayerEndIndexSpinBox->value();
      averaging.reset( new QgsMeshMultiLevelsAveragingMethod( startVerticalLevel, endVerticalLevel, true ) );
      break;
    }
    case 3: // multi level from bottom
    {
      const int startVerticalLevel = mMultiBottomVerticalLayerStartIndexSpinBox->value();
      const int endVerticalLevel = mMultiBottomVerticalLayerEndIndexSpinBox->value();
      averaging.reset( new QgsMeshMultiLevelsAveragingMethod( startVerticalLevel, endVerticalLevel, false ) );
      break;
    }
    case 4: // sigma
    {
      const double startFraction = mSigmaStartFractionSpinBox->value();
      const double endFraction = mSigmaEndFractionSpinBox->value();
      averaging.reset( new QgsMeshSigmaAveragingMethod( startFraction, endFraction ) );
      break;
    }
    case 5: // depth (from surface)
    {
      const double startDepth = mDepthStartSpinBox->value();
      const double endDepth = mDepthEndSpinBox->value();
      averaging.reset( new QgsMeshRelativeHeightAveragingMethod( startDepth, endDepth, true ) );
      break;
    }
    case 6: // height (from bed elevation)
    {
      const double startHeight = mHeightStartSpinBox->value();
      const double endHeight = mHeightEndSpinBox->value();
      averaging.reset( new QgsMeshRelativeHeightAveragingMethod( startHeight, endHeight, false ) );
      break;
    }
    case 7: // elevation
    {
      const double startVerticalLevel = mElevationStartSpinBox->value();
      const double endVerticalLevel = mElevationEndSpinBox->value();
      averaging.reset( new QgsMeshElevationAveragingMethod( startVerticalLevel, endVerticalLevel ) );
      break;
    }
  }
  return averaging;
}

void QgsMeshRenderer3DAveragingWidget::syncToLayer()
{
  if ( !mMeshLayer )
    return;

  const QgsMeshRendererSettings rendererSettings = mMeshLayer->rendererSettings();
  const QgsMesh3DAveragingMethod *method = rendererSettings.averagingMethod();
  if ( method )
  {
    const QgsMesh3DAveragingMethod::Method type = method->method();
    int pageIndex = 0;

    switch ( type )
    {
      case QgsMesh3DAveragingMethod::MultiLevelsAveragingMethod:
      {
        const QgsMeshMultiLevelsAveragingMethod *averagingMethod = static_cast<const QgsMeshMultiLevelsAveragingMethod *>( method );
        if ( averagingMethod->isSingleLevel() )
        {
          if ( averagingMethod->countedFromTop() )
          {
            // Single Vertical Layer settings from top
            whileBlocking( mSingleVerticalLayerIndexTopSpinBox )->setValue( averagingMethod->startVerticalLevel() );
            pageIndex = 0;
          }
          else
          {
            // Single Vertical Layer settings from bottom
            whileBlocking( mSingleVerticalLayerIndexBottomSpinBox )->setValue( averagingMethod->startVerticalLevel() );
            pageIndex = 1;
          }
        }
        else
        {
          if ( averagingMethod->countedFromTop() )
          {
            // Multi Vertical Layer settings from top
            whileBlocking( mMultiTopVerticalLayerStartIndexSpinBox )->setValue( averagingMethod->startVerticalLevel() );
            whileBlocking( mMultiTopVerticalLayerEndIndexSpinBox )->setValue( averagingMethod->endVerticalLevel() );
            pageIndex = 2;
          }
          else
          {
            // Multi Vertical Layer settings from bottom
            whileBlocking( mMultiBottomVerticalLayerStartIndexSpinBox )->setValue( averagingMethod->startVerticalLevel() );
            whileBlocking( mMultiBottomVerticalLayerEndIndexSpinBox )->setValue( averagingMethod->endVerticalLevel() );
            pageIndex = 3;
          }
        }
        break;
      }
      case QgsMesh3DAveragingMethod::SigmaAveragingMethod:
      {
        const QgsMeshSigmaAveragingMethod *sigmaAveragingMethod = static_cast<const QgsMeshSigmaAveragingMethod *>( method );
        whileBlocking( mSigmaStartFractionSpinBox )->setValue( sigmaAveragingMethod->startFraction() );
        whileBlocking( mSigmaEndFractionSpinBox )->setValue( sigmaAveragingMethod->endFraction() );
        pageIndex = 4;
        break;
      }
      case QgsMesh3DAveragingMethod::RelativeHeightAveragingMethod:
      {
        const QgsMeshRelativeHeightAveragingMethod *averagingMethod = static_cast<const QgsMeshRelativeHeightAveragingMethod *>( method );
        if ( averagingMethod->countedFromTop() )
        {
          whileBlocking( mDepthStartSpinBox )->setValue( averagingMethod->startHeight() );
          whileBlocking( mDepthEndSpinBox )->setValue( averagingMethod->endHeight() );
          pageIndex = 5;
        }
        else
        {
          whileBlocking( mHeightStartSpinBox )->setValue( averagingMethod->startHeight() );
          whileBlocking( mHeightEndSpinBox )->setValue( averagingMethod->endHeight() );
          pageIndex = 6;
        }
        break;
      }
      case QgsMesh3DAveragingMethod::ElevationAveragingMethod:
      {
        const QgsMeshElevationAveragingMethod *elevationAveragingMethod = static_cast<const QgsMeshElevationAveragingMethod *>( method );
        whileBlocking( mElevationStartSpinBox )->setValue( elevationAveragingMethod->startElevation() );
        whileBlocking( mElevationEndSpinBox )->setValue( elevationAveragingMethod->endElevation() );
        pageIndex = 7;
        break;
      }
    }
    whileBlocking( mAveragingMethodComboBox )->setCurrentIndex( pageIndex );
    whileBlocking( mAveragingMethodStackedWidget )->setCurrentIndex( pageIndex );
  }
}

void QgsMeshRenderer3DAveragingWidget::onAveragingMethodChanged( int methodIndex )
{
  whileBlocking( mAveragingMethodStackedWidget )->setCurrentIndex( methodIndex );
  emit widgetChanged();
}

void QgsMeshRenderer3DAveragingWidget::updateGraphics()
{
  setLabelSvg( mSingleTopPngLabel, QStringLiteral( "SingleTop.svg" ) );
  mSingleTopGroup->adjustSize();

  setLabelSvg( mSingleBottomPngLabel, QStringLiteral( "SingleBottom.svg" ) );
  setLabelSvg( mMultiTopPngLabel, QStringLiteral( "MultiTop.svg" ) );
  setLabelSvg( mMultiBottomPngLabel, QStringLiteral( "MultiBottom.svg" ) );
  setLabelSvg( mSigmaPngLabel, QStringLiteral( "Sigma.svg" ) );
  setLabelSvg( mDepthPngLabel, QStringLiteral( "Depth.svg" ) );
  setLabelSvg( mHeightPngLabel, QStringLiteral( "Height.svg" ) );
  setLabelSvg( mElevationPngLabel, QStringLiteral( "Elevation.svg" ) );
}

void QgsMeshRenderer3DAveragingWidget::setLabelSvg( QLabel *imageLabel, const QString &imgName )
{
  const qreal dpi = mScreenHelper->screenDpi();
  const int desiredWidth = static_cast<int>( 100 * dpi / 25.4 );

  QSvgRenderer renderer( QStringLiteral( ":/images/themes/default/mesh/%1" ).arg( imgName ) );
  if ( renderer.isValid() )
  {
    const QSize defaultSvgSize = renderer.defaultSize();
    const int desiredHeight = defaultSvgSize.height() * desiredWidth / defaultSvgSize.width();

    QPixmap pixmap( QSize( desiredWidth, desiredHeight ) );
    pixmap.fill( Qt::transparent );
    QPainter painter;

    painter.begin( &pixmap );
    renderer.render( &painter );
    painter.end();
    imageLabel->setPixmap( pixmap );
  }
}

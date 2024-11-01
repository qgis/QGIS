/***************************************************************************
    qgsvectorrenderingeoptions.cpp
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

#include "qgsabstractgeometry.h"
#include "qgsvectorrenderingoptions.h"
#include "moc_qgsvectorrenderingoptions.cpp"
#include "qgssettings.h"
#include "qgsapplication.h"
#include "qgsvectorlayer.h"
#include "qgssettingsentryenumflag.h"

#include <QThread>
//
// QgsVectorRenderingOptionsWidget
//

QgsVectorRenderingOptionsWidget::QgsVectorRenderingOptionsWidget( QWidget *parent )
  : QgsOptionsPageWidget( parent )
{
  setupUi( this );

  QgsSettings settings;

  // Default simplify drawing configuration
  mSimplifyDrawingGroupBox->setChecked( settings.enumValue( QStringLiteral( "/qgis/simplifyDrawingHints" ), Qgis::VectorRenderingSimplificationFlag::GeometrySimplification ) != Qgis::VectorRenderingSimplificationFlag::NoSimplification );
  mSimplifyDrawingSpinBox->setValue( QgsVectorLayer::settingsSimplifyDrawingTol->value() );
  mSimplifyDrawingAtProvider->setChecked( !QgsVectorLayer::settingsSimplifyLocal->value() );

  //segmentation tolerance type
  mToleranceTypeComboBox->addItem( tr( "Maximum Angle" ), QgsAbstractGeometry::MaximumAngle );
  mToleranceTypeComboBox->addItem( tr( "Maximum Difference" ), QgsAbstractGeometry::MaximumDifference );
  QgsAbstractGeometry::SegmentationToleranceType toleranceType = settings.enumValue( QStringLiteral( "/qgis/segmentationToleranceType" ), QgsAbstractGeometry::MaximumAngle );
  int toleranceTypeIndex = mToleranceTypeComboBox->findData( toleranceType );
  if ( toleranceTypeIndex != -1 )
  {
    mToleranceTypeComboBox->setCurrentIndex( toleranceTypeIndex );
  }

  double tolerance = settings.value( QStringLiteral( "/qgis/segmentationTolerance" ), "0.01745" ).toDouble();
  if ( toleranceType == QgsAbstractGeometry::MaximumAngle )
  {
    tolerance = tolerance * 180.0 / M_PI; //value shown to the user is degree, not rad
  }
  mSegmentationToleranceSpinBox->setValue( tolerance );
  mSegmentationToleranceSpinBox->setClearValue( 1.0 );

  QStringList myScalesList = Qgis::defaultProjectScales().split( ',' );
  myScalesList.append( QStringLiteral( "1:1" ) );
  mSimplifyMaximumScaleComboBox->updateScales( myScalesList );
  mSimplifyMaximumScaleComboBox->setScale( QgsVectorLayer::settingsSimplifyMaxScale->value() );

  // Default local simplification algorithm
  mSimplifyAlgorithmComboBox->addItem( tr( "Distance" ), QVariant::fromValue( Qgis::VectorSimplificationAlgorithm::Distance ) );
  mSimplifyAlgorithmComboBox->addItem( tr( "SnapToGrid" ), QVariant::fromValue( Qgis::VectorSimplificationAlgorithm::SnapToGrid ) );
  mSimplifyAlgorithmComboBox->addItem( tr( "Visvalingam" ), QVariant::fromValue( Qgis::VectorSimplificationAlgorithm::Visvalingam ) );
  mSimplifyAlgorithmComboBox->setCurrentIndex( mSimplifyAlgorithmComboBox->findData( QVariant::fromValue( QgsVectorLayer::settingsSimplifyAlgorithm->value() ) ) );
}

QString QgsVectorRenderingOptionsWidget::helpKey() const
{
  return QStringLiteral( "introduction/qgis_configuration.html#vector-rendering-options" );
}

void QgsVectorRenderingOptionsWidget::apply()
{
  QgsSettings settings;

  // Default simplify drawing configuration
  Qgis::VectorRenderingSimplificationFlags simplifyHints = Qgis::VectorRenderingSimplificationFlag::NoSimplification;
  if ( mSimplifyDrawingGroupBox->isChecked() )
  {
    simplifyHints |= Qgis::VectorRenderingSimplificationFlag::GeometrySimplification;
    if ( mSimplifyDrawingSpinBox->value() > 1 )
      simplifyHints |= Qgis::VectorRenderingSimplificationFlag::AntialiasingSimplification;
  }
  QgsVectorLayer::settingsSimplifyDrawingHints->setValue( simplifyHints );
  QgsVectorLayer::settingsSimplifyAlgorithm->setValue( mSimplifyAlgorithmComboBox->currentData().value<Qgis::VectorSimplificationAlgorithm>() );
  QgsVectorLayer::settingsSimplifyDrawingTol->setValue( mSimplifyDrawingSpinBox->value() );
  QgsVectorLayer::settingsSimplifyLocal->setValue( !mSimplifyDrawingAtProvider->isChecked() );
  QgsVectorLayer::settingsSimplifyMaxScale->setValue( mSimplifyMaximumScaleComboBox->scale() );

  //curve segmentation
  QgsAbstractGeometry::SegmentationToleranceType segmentationType = ( QgsAbstractGeometry::SegmentationToleranceType ) mToleranceTypeComboBox->currentData().toInt();
  settings.setEnumValue( QStringLiteral( "/qgis/segmentationToleranceType" ), segmentationType );
  double segmentationTolerance = mSegmentationToleranceSpinBox->value();
  if ( segmentationType == QgsAbstractGeometry::MaximumAngle )
  {
    segmentationTolerance = segmentationTolerance / 180.0 * M_PI; //user sets angle tolerance in degrees, internal classes need value in rad
  }
  settings.setValue( QStringLiteral( "/qgis/segmentationTolerance" ), segmentationTolerance );
}


//
// QgsVectorRenderingOptionsFactory
//
QgsVectorRenderingOptionsFactory::QgsVectorRenderingOptionsFactory()
  : QgsOptionsWidgetFactory( tr( "Vector" ), QIcon(), QStringLiteral( "vector" ) )
{
}

QIcon QgsVectorRenderingOptionsFactory::icon() const
{
  return QgsApplication::getThemeIcon( QStringLiteral( "mIconVector.svg" ) );
}

QgsOptionsPageWidget *QgsVectorRenderingOptionsFactory::createWidget( QWidget *parent ) const
{
  return new QgsVectorRenderingOptionsWidget( parent );
}

QStringList QgsVectorRenderingOptionsFactory::path() const
{
  return { QStringLiteral( "rendering" ) };
}

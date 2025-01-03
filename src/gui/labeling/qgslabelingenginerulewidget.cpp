/***************************************************************************
    qgslabelingenginerulewidget.cpp
    ------------------------
    begin                : September 2024
    copyright            : (C) 2024 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgslabelingenginerulewidget.h"
#include "moc_qgslabelingenginerulewidget.cpp"
#include "qgslabelingenginerule_impl.h"
#include "qgsgui.h"
#include "qgshelp.h"

#include <QDialogButtonBox>
#include <QPushButton>

//
// QgsLabelingEngineRuleDialog
//

QgsLabelingEngineRuleDialog::QgsLabelingEngineRuleDialog( QgsLabelingEngineRuleWidget *widget, QWidget *parent, Qt::WindowFlags flags )
  : QDialog( parent, flags )
  , mWidget( widget )
{
  Q_ASSERT( mWidget );
  setWindowTitle( tr( "Configure Rule" ) );
  setObjectName( QStringLiteral( "QgsLabelingEngineRuleDialog" ) );

  QVBoxLayout *layout = new QVBoxLayout( this );
  layout->addWidget( mWidget );

  mButtonBox = new QDialogButtonBox( QDialogButtonBox::Ok | QDialogButtonBox::Cancel | QDialogButtonBox::Help, Qt::Horizontal, this );
  layout->addWidget( mButtonBox );

  setLayout( layout );
  QgsGui::enableAutoGeometryRestore( this );

  connect( mButtonBox->button( QDialogButtonBox::Ok ), &QAbstractButton::clicked, this, &QDialog::accept );
  connect( mButtonBox->button( QDialogButtonBox::Cancel ), &QAbstractButton::clicked, this, &QDialog::reject );
  connect( mButtonBox, &QDialogButtonBox::helpRequested, this, [=] {
    QgsHelp::openHelp( QStringLiteral( "working_with_vector/vector_properties.html#labeling-rules" ) );
  } );
}

void QgsLabelingEngineRuleDialog::setRule( const QgsAbstractLabelingEngineRule *rule )
{
  mWidget->setRule( rule );
}

QgsAbstractLabelingEngineRule *QgsLabelingEngineRuleDialog::rule()
{
  return mWidget->rule();
}

///@cond PRIVATE
//
// QgsLabelingEngineRuleAvoidLabelOverlapWithFeatureWidget
//

QgsLabelingEngineRuleAvoidLabelOverlapWithFeatureWidget::QgsLabelingEngineRuleAvoidLabelOverlapWithFeatureWidget( QWidget *parent )
  : QgsLabelingEngineRuleWidget( parent )
{
  setupUi( this );

  setWindowTitle( tr( "Prevent Labels Overlapping Features" ) );
  setPanelTitle( tr( "Configure Rule" ) );

  mComboLabeledLayer->setFilters( Qgis::LayerFilter::SpatialLayer );
  mComboTargetLayer->setFilters( Qgis::LayerFilter::HasGeometry );

  connect( mEditName, &QLineEdit::textChanged, this, &QgsLabelingEngineRuleAvoidLabelOverlapWithFeatureWidget::onChanged );
  connect( mComboLabeledLayer, &QgsMapLayerComboBox::layerChanged, this, &QgsLabelingEngineRuleAvoidLabelOverlapWithFeatureWidget::onChanged );
  connect( mComboTargetLayer, &QgsMapLayerComboBox::layerChanged, this, &QgsLabelingEngineRuleAvoidLabelOverlapWithFeatureWidget::onChanged );
}

void QgsLabelingEngineRuleAvoidLabelOverlapWithFeatureWidget::setRule( const QgsAbstractLabelingEngineRule *rule )
{
  const QgsLabelingEngineRuleAvoidLabelOverlapWithFeature *castRule = dynamic_cast<const QgsLabelingEngineRuleAvoidLabelOverlapWithFeature *>( rule );
  if ( !castRule )
    return;

  mBlockSignals = true;
  mEditName->setText( castRule->name() );
  mComboLabeledLayer->setLayer( const_cast<QgsLabelingEngineRuleAvoidLabelOverlapWithFeature *>( castRule )->labeledLayer() );
  mComboTargetLayer->setLayer( const_cast<QgsLabelingEngineRuleAvoidLabelOverlapWithFeature *>( castRule )->targetLayer() );

  mBlockSignals = false;
}

QgsAbstractLabelingEngineRule *QgsLabelingEngineRuleAvoidLabelOverlapWithFeatureWidget::rule()
{
  std::unique_ptr<QgsLabelingEngineRuleAvoidLabelOverlapWithFeature> res = std::make_unique<QgsLabelingEngineRuleAvoidLabelOverlapWithFeature>();
  res->setName( mEditName->text() );
  res->setLabeledLayer( mComboLabeledLayer->currentLayer() );
  res->setTargetLayer( qobject_cast<QgsVectorLayer *>( mComboTargetLayer->currentLayer() ) );
  return res.release();
}

void QgsLabelingEngineRuleAvoidLabelOverlapWithFeatureWidget::onChanged()
{
  if ( !mBlockSignals )
    emit changed();
}

//
// QgsLabelingEngineRuleMinimumDistanceLabelToFeatureWidget
//

QgsLabelingEngineRuleMinimumDistanceLabelToFeatureWidget::QgsLabelingEngineRuleMinimumDistanceLabelToFeatureWidget( QWidget *parent )
  : QgsLabelingEngineRuleWidget( parent )
{
  setupUi( this );

  setWindowTitle( tr( "Push Labels Away from Features" ) );
  setPanelTitle( tr( "Configure Rule" ) );

  mComboLabeledLayer->setFilters( Qgis::LayerFilter::SpatialLayer );
  mComboTargetLayer->setFilters( Qgis::LayerFilter::HasGeometry );

  mDistanceUnitWidget->setUnits( QgsUnitTypes::RenderUnitList() << Qgis::RenderUnit::Millimeters << Qgis::RenderUnit::MetersInMapUnits << Qgis::RenderUnit::MapUnits << Qgis::RenderUnit::Pixels << Qgis::RenderUnit::Points << Qgis::RenderUnit::Inches );

  connect( mEditName, &QLineEdit::textChanged, this, &QgsLabelingEngineRuleMinimumDistanceLabelToFeatureWidget::onChanged );
  connect( mComboLabeledLayer, &QgsMapLayerComboBox::layerChanged, this, &QgsLabelingEngineRuleMinimumDistanceLabelToFeatureWidget::onChanged );
  connect( mComboTargetLayer, &QgsMapLayerComboBox::layerChanged, this, &QgsLabelingEngineRuleMinimumDistanceLabelToFeatureWidget::onChanged );

  connect( mSpinDistance, qOverload<double>( &QgsDoubleSpinBox::valueChanged ), this, &QgsLabelingEngineRuleMinimumDistanceLabelToFeatureWidget::onChanged );
  connect( mDistanceUnitWidget, &QgsUnitSelectionWidget::changed, this, &QgsLabelingEngineRuleMinimumDistanceLabelToFeatureWidget::onChanged );
  connect( mCostSlider, &QSlider::valueChanged, this, &QgsLabelingEngineRuleMinimumDistanceLabelToFeatureWidget::onChanged );
}

void QgsLabelingEngineRuleMinimumDistanceLabelToFeatureWidget::setRule( const QgsAbstractLabelingEngineRule *rule )
{
  const QgsLabelingEngineRuleMinimumDistanceLabelToFeature *castRule = dynamic_cast<const QgsLabelingEngineRuleMinimumDistanceLabelToFeature *>( rule );
  if ( !castRule )
    return;

  mBlockSignals = true;
  mEditName->setText( castRule->name() );
  mComboLabeledLayer->setLayer( const_cast<QgsLabelingEngineRuleMinimumDistanceLabelToFeature *>( castRule )->labeledLayer() );
  mComboTargetLayer->setLayer( const_cast<QgsLabelingEngineRuleMinimumDistanceLabelToFeature *>( castRule )->targetLayer() );

  mSpinDistance->setValue( castRule->distance() );
  mDistanceUnitWidget->setUnit( castRule->distanceUnit() );
  mDistanceUnitWidget->setMapUnitScale( castRule->distanceUnitScale() );

  mCostSlider->setValue( static_cast<int>( castRule->cost() * 10 ) );

  mBlockSignals = false;
}

QgsAbstractLabelingEngineRule *QgsLabelingEngineRuleMinimumDistanceLabelToFeatureWidget::rule()
{
  std::unique_ptr<QgsLabelingEngineRuleMinimumDistanceLabelToFeature> res = std::make_unique<QgsLabelingEngineRuleMinimumDistanceLabelToFeature>();
  res->setName( mEditName->text() );
  res->setLabeledLayer( mComboLabeledLayer->currentLayer() );
  res->setTargetLayer( qobject_cast<QgsVectorLayer *>( mComboTargetLayer->currentLayer() ) );

  res->setDistance( mSpinDistance->value() );
  res->setDistanceUnit( mDistanceUnitWidget->unit() );
  res->setDistanceUnitScale( mDistanceUnitWidget->getMapUnitScale() );

  res->setCost( mCostSlider->value() / 10.0 );

  return res.release();
}

void QgsLabelingEngineRuleMinimumDistanceLabelToFeatureWidget::onChanged()
{
  if ( !mBlockSignals )
    emit changed();
}

//
// QgsLabelingEngineRuleMaximumDistanceLabelToFeatureWidget
//

QgsLabelingEngineRuleMaximumDistanceLabelToFeatureWidget::QgsLabelingEngineRuleMaximumDistanceLabelToFeatureWidget( QWidget *parent )
  : QgsLabelingEngineRuleWidget( parent )
{
  setupUi( this );

  setWindowTitle( tr( "Pull Labels toward Features" ) );
  setPanelTitle( tr( "Configure Rule" ) );

  mComboLabeledLayer->setFilters( Qgis::LayerFilter::SpatialLayer );
  mComboTargetLayer->setFilters( Qgis::LayerFilter::HasGeometry );

  mDistanceUnitWidget->setUnits( QgsUnitTypes::RenderUnitList() << Qgis::RenderUnit::Millimeters << Qgis::RenderUnit::MetersInMapUnits << Qgis::RenderUnit::MapUnits << Qgis::RenderUnit::Pixels << Qgis::RenderUnit::Points << Qgis::RenderUnit::Inches );

  connect( mEditName, &QLineEdit::textChanged, this, &QgsLabelingEngineRuleMaximumDistanceLabelToFeatureWidget::onChanged );
  connect( mComboLabeledLayer, &QgsMapLayerComboBox::layerChanged, this, &QgsLabelingEngineRuleMaximumDistanceLabelToFeatureWidget::onChanged );
  connect( mComboTargetLayer, &QgsMapLayerComboBox::layerChanged, this, &QgsLabelingEngineRuleMaximumDistanceLabelToFeatureWidget::onChanged );

  connect( mSpinDistance, qOverload<double>( &QgsDoubleSpinBox::valueChanged ), this, &QgsLabelingEngineRuleMaximumDistanceLabelToFeatureWidget::onChanged );
  connect( mDistanceUnitWidget, &QgsUnitSelectionWidget::changed, this, &QgsLabelingEngineRuleMaximumDistanceLabelToFeatureWidget::onChanged );
  connect( mCostSlider, &QSlider::valueChanged, this, &QgsLabelingEngineRuleMaximumDistanceLabelToFeatureWidget::onChanged );
}

void QgsLabelingEngineRuleMaximumDistanceLabelToFeatureWidget::setRule( const QgsAbstractLabelingEngineRule *rule )
{
  const QgsLabelingEngineRuleMaximumDistanceLabelToFeature *castRule = dynamic_cast<const QgsLabelingEngineRuleMaximumDistanceLabelToFeature *>( rule );
  if ( !castRule )
    return;

  mBlockSignals = true;
  mEditName->setText( castRule->name() );
  mComboLabeledLayer->setLayer( const_cast<QgsLabelingEngineRuleMaximumDistanceLabelToFeature *>( castRule )->labeledLayer() );
  mComboTargetLayer->setLayer( const_cast<QgsLabelingEngineRuleMaximumDistanceLabelToFeature *>( castRule )->targetLayer() );

  mSpinDistance->setValue( castRule->distance() );
  mDistanceUnitWidget->setUnit( castRule->distanceUnit() );
  mDistanceUnitWidget->setMapUnitScale( castRule->distanceUnitScale() );

  mCostSlider->setValue( static_cast<int>( castRule->cost() * 10 ) );

  mBlockSignals = false;
}

QgsAbstractLabelingEngineRule *QgsLabelingEngineRuleMaximumDistanceLabelToFeatureWidget::rule()
{
  std::unique_ptr<QgsLabelingEngineRuleMaximumDistanceLabelToFeature> res = std::make_unique<QgsLabelingEngineRuleMaximumDistanceLabelToFeature>();
  res->setName( mEditName->text() );
  res->setLabeledLayer( mComboLabeledLayer->currentLayer() );
  res->setTargetLayer( qobject_cast<QgsVectorLayer *>( mComboTargetLayer->currentLayer() ) );

  res->setDistance( mSpinDistance->value() );
  res->setDistanceUnit( mDistanceUnitWidget->unit() );
  res->setDistanceUnitScale( mDistanceUnitWidget->getMapUnitScale() );

  res->setCost( mCostSlider->value() / 10.0 );

  return res.release();
}

void QgsLabelingEngineRuleMaximumDistanceLabelToFeatureWidget::onChanged()
{
  if ( !mBlockSignals )
    emit changed();
}


//
// QgsLabelingEngineRuleMinimumDistanceLabelToLabelWidget
//

QgsLabelingEngineRuleMinimumDistanceLabelToLabelWidget::QgsLabelingEngineRuleMinimumDistanceLabelToLabelWidget( QWidget *parent )
  : QgsLabelingEngineRuleWidget( parent )
{
  setupUi( this );

  setWindowTitle( tr( "Pull Labels toward Features" ) );
  setPanelTitle( tr( "Configure Rule" ) );

  mComboLabeledLayer->setFilters( Qgis::LayerFilter::SpatialLayer );
  mComboTargetLayer->setFilters( Qgis::LayerFilter::SpatialLayer );

  mDistanceUnitWidget->setUnits( QgsUnitTypes::RenderUnitList() << Qgis::RenderUnit::Millimeters << Qgis::RenderUnit::MetersInMapUnits << Qgis::RenderUnit::MapUnits << Qgis::RenderUnit::Pixels << Qgis::RenderUnit::Points << Qgis::RenderUnit::Inches );

  connect( mEditName, &QLineEdit::textChanged, this, &QgsLabelingEngineRuleMinimumDistanceLabelToLabelWidget::onChanged );
  connect( mComboLabeledLayer, &QgsMapLayerComboBox::layerChanged, this, &QgsLabelingEngineRuleMinimumDistanceLabelToLabelWidget::onChanged );
  connect( mComboTargetLayer, &QgsMapLayerComboBox::layerChanged, this, &QgsLabelingEngineRuleMinimumDistanceLabelToLabelWidget::onChanged );

  connect( mSpinDistance, qOverload<double>( &QgsDoubleSpinBox::valueChanged ), this, &QgsLabelingEngineRuleMinimumDistanceLabelToLabelWidget::onChanged );
  connect( mDistanceUnitWidget, &QgsUnitSelectionWidget::changed, this, &QgsLabelingEngineRuleMinimumDistanceLabelToLabelWidget::onChanged );
}

void QgsLabelingEngineRuleMinimumDistanceLabelToLabelWidget::setRule( const QgsAbstractLabelingEngineRule *rule )
{
  const QgsLabelingEngineRuleMinimumDistanceLabelToLabel *castRule = dynamic_cast<const QgsLabelingEngineRuleMinimumDistanceLabelToLabel *>( rule );
  if ( !castRule )
    return;

  mBlockSignals = true;
  mEditName->setText( castRule->name() );
  mComboLabeledLayer->setLayer( const_cast<QgsLabelingEngineRuleMinimumDistanceLabelToLabel *>( castRule )->labeledLayer() );
  mComboTargetLayer->setLayer( const_cast<QgsLabelingEngineRuleMinimumDistanceLabelToLabel *>( castRule )->targetLayer() );

  mSpinDistance->setValue( castRule->distance() );
  mDistanceUnitWidget->setUnit( castRule->distanceUnit() );
  mDistanceUnitWidget->setMapUnitScale( castRule->distanceUnitScale() );

  mBlockSignals = false;
}

QgsAbstractLabelingEngineRule *QgsLabelingEngineRuleMinimumDistanceLabelToLabelWidget::rule()
{
  std::unique_ptr<QgsLabelingEngineRuleMinimumDistanceLabelToLabel> res = std::make_unique<QgsLabelingEngineRuleMinimumDistanceLabelToLabel>();
  res->setName( mEditName->text() );
  res->setLabeledLayer( mComboLabeledLayer->currentLayer() );
  res->setTargetLayer( mComboTargetLayer->currentLayer() );

  res->setDistance( mSpinDistance->value() );
  res->setDistanceUnit( mDistanceUnitWidget->unit() );
  res->setDistanceUnitScale( mDistanceUnitWidget->getMapUnitScale() );

  return res.release();
}

void QgsLabelingEngineRuleMinimumDistanceLabelToLabelWidget::onChanged()
{
  if ( !mBlockSignals )
    emit changed();
}


///@endcond

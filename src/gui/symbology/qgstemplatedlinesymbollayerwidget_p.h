/***************************************************************************
    qgstemplatedlinesymbollayerwidget_p.h
    ---------------------
    begin                : 2025/10/09
    copyright            : (C) 2025 by Julien Cabieces
    email                : julien dot cabieces at oslandia dot com
    ***************************************************************************
    *                                                                         *
    *   This program is free software; you can redistribute it and/or modify  *
    *   it under the terms of the GNU General Public License as published by  *
    *   the Free Software Foundation; either version 2 of the License, or     *
    *   (at your option) any later version.                                   *
    *                                                                         *
    ***************************************************************************/

#ifndef QGSTEMPLATEDLINESYMBOLLAYERWIDGET_P_H
#define QGSTEMPLATEDLINESYMBOLLAYERWIDGET_P_H

#define SIP_NO_FILE

#include "qgsapplication.h"
#include "qgslinesymbollayer.h"
#include "qgssymbollayerwidget.h"
#include "qgsunitselectionwidget.h"

/**
 * \ingroup gui
 * \class QgsTemplatedLineSymbolLayerWidget
 * \brief A widget for controlling common the properties of QgsMarkerLineSymbolLayer and QgsHashedLineSymbolLayer.
 * \since QGIS 4.0
 */
template<class Ui, class SL>
class GUI_EXPORT QgsTemplatedLineSymbolLayerWidget : public QgsSymbolLayerWidget, private Ui
{
  public:
    /**
   * Constructor
   * \param vl associated vector layer
   * \param parent parent widget
   */
    QgsTemplatedLineSymbolLayerWidget( QgsVectorLayer *vl, QWidget *parent = nullptr )
      : QgsSymbolLayerWidget( parent, vl )
    {
      mUi.setupUi( this );
      connect( mUi.mIntervalUnitWidget, &QgsUnitSelectionWidget::changed, this, &QgsTemplatedLineSymbolLayerWidget::mIntervalUnitWidget_changed );
      connect( mUi.mOffsetUnitWidget, &QgsUnitSelectionWidget::changed, this, &QgsTemplatedLineSymbolLayerWidget::mOffsetUnitWidget_changed );
      connect( mUi.mOffsetAlongLineUnitWidget, &QgsUnitSelectionWidget::changed, this, &QgsTemplatedLineSymbolLayerWidget::mOffsetAlongLineUnitWidget_changed );
      connect( mUi.mAverageAngleUnit, &QgsUnitSelectionWidget::changed, this, &QgsTemplatedLineSymbolLayerWidget::averageAngleUnitChanged );
      mUi.mIntervalUnitWidget->setUnits( QgsUnitTypes::RenderUnitList() << Qgis::RenderUnit::Millimeters << Qgis::RenderUnit::MetersInMapUnits << Qgis::RenderUnit::MapUnits << Qgis::RenderUnit::Pixels << Qgis::RenderUnit::Points << Qgis::RenderUnit::Inches );
      mUi.mOffsetUnitWidget->setUnits( QgsUnitTypes::RenderUnitList() << Qgis::RenderUnit::Millimeters << Qgis::RenderUnit::MetersInMapUnits << Qgis::RenderUnit::MapUnits << Qgis::RenderUnit::Pixels << Qgis::RenderUnit::Points << Qgis::RenderUnit::Inches );
      mUi.mOffsetAlongLineUnitWidget->setUnits( QgsUnitTypes::RenderUnitList() << Qgis::RenderUnit::Millimeters << Qgis::RenderUnit::MetersInMapUnits << Qgis::RenderUnit::MapUnits << Qgis::RenderUnit::Pixels << Qgis::RenderUnit::Points << Qgis::RenderUnit::Inches << Qgis::RenderUnit::Percentage );
      mUi.mAverageAngleUnit->setUnits( QgsUnitTypes::RenderUnitList() << Qgis::RenderUnit::Millimeters << Qgis::RenderUnit::MetersInMapUnits << Qgis::RenderUnit::MapUnits << Qgis::RenderUnit::Pixels << Qgis::RenderUnit::Points << Qgis::RenderUnit::Inches );

      mUi.mRingFilterComboBox->addItem( QgsApplication::getThemeIcon( QStringLiteral( "mIconAllRings.svg" ) ), tr( "All Rings" ), QgsLineSymbolLayer::AllRings );
      mUi.mRingFilterComboBox->addItem( QgsApplication::getThemeIcon( QStringLiteral( "mIconExteriorRing.svg" ) ), tr( "Exterior Ring Only" ), QgsLineSymbolLayer::ExteriorRingOnly );
      mUi.mRingFilterComboBox->addItem( QgsApplication::getThemeIcon( QStringLiteral( "mIconInteriorRings.svg" ) ), tr( "Interior Rings Only" ), QgsLineSymbolLayer::InteriorRingsOnly );
      connect( mUi.mRingFilterComboBox, qOverload<int>( &QComboBox::currentIndexChanged ), this, [this]( int ) {
        if ( mLayer )
        {
          mLayer->setRingFilter( static_cast<QgsLineSymbolLayer::RenderRingFilter>( mUi.mRingFilterComboBox->currentData().toInt() ) );
          emit changed();
        }
      } );

      mUi.spinOffset->setClearValue( 0.0 );
      mUi.mSpinOffsetAlongLine->setClearValue( 0.0 );
      mUi.mSpinAverageAngleLength->setClearValue( 4.0 );

      connect( mUi.spinInterval, static_cast<void ( QDoubleSpinBox::* )( double )>( &QDoubleSpinBox::valueChanged ), this, &QgsTemplatedLineSymbolLayerWidget::setInterval );
      connect( mUi.mSpinOffsetAlongLine, static_cast<void ( QDoubleSpinBox::* )( double )>( &QDoubleSpinBox::valueChanged ), this, &QgsTemplatedLineSymbolLayerWidget::setOffsetAlongLine );
      connect( mUi.chkRotateMarker, &QAbstractButton::clicked, this, &QgsTemplatedLineSymbolLayerWidget::setRotate );
      connect( mUi.spinOffset, static_cast<void ( QDoubleSpinBox::* )( double )>( &QDoubleSpinBox::valueChanged ), this, &QgsTemplatedLineSymbolLayerWidget::setOffset );
      connect( mUi.mSpinAverageAngleLength, static_cast<void ( QDoubleSpinBox::* )( double )>( &QDoubleSpinBox::valueChanged ), this, &QgsTemplatedLineSymbolLayerWidget::setAverageAngle );
      connect( mUi.mCheckInterval, &QCheckBox::toggled, this, &QgsTemplatedLineSymbolLayerWidget::setPlacement );
      connect( mUi.mCheckVertex, &QCheckBox::toggled, this, &QgsTemplatedLineSymbolLayerWidget::setPlacement );
      connect( mUi.mCheckVertexLast, &QCheckBox::toggled, this, &QgsTemplatedLineSymbolLayerWidget::setPlacement );
      connect( mUi.mCheckVertexFirst, &QCheckBox::toggled, this, &QgsTemplatedLineSymbolLayerWidget::setPlacement );
      connect( mUi.mCheckCentralPoint, &QCheckBox::toggled, this, &QgsTemplatedLineSymbolLayerWidget::setPlacement );
      connect( mUi.mCheckCurvePoint, &QCheckBox::toggled, this, &QgsTemplatedLineSymbolLayerWidget::setPlacement );
      connect( mUi.mCheckSegmentCentralPoint, &QCheckBox::toggled, this, &QgsTemplatedLineSymbolLayerWidget::setPlacement );
      connect( mUi.mCheckPlaceOnEveryPart, &QCheckBox::toggled, this, [this] {
        if ( mLayer )
        {
          mLayer->setPlaceOnEveryPart( mUi.mCheckPlaceOnEveryPart->isChecked() );
          emit changed();
        }
      } );
    }

    void setSymbolLayer( QgsSymbolLayer *layer ) override

    {
      if ( ( std::is_same<SL, QgsMarkerLineSymbolLayer>::value && layer->layerType() != QLatin1String( "MarkerLine" ) )
           || ( std::is_same<SL, QgsHashedLineSymbolLayer>::value && layer->layerType() != QLatin1String( "HashLine" ) ) )
        return;

      // layer type is correct, we can do the cast
      mLayer = static_cast<SL *>( layer );

      // set values
      mUi.spinInterval->blockSignals( true );
      mUi.spinInterval->setValue( mLayer->interval() );
      mUi.spinInterval->blockSignals( false );
      mUi.mSpinOffsetAlongLine->blockSignals( true );
      mUi.mSpinOffsetAlongLine->setValue( mLayer->offsetAlongLine() );
      mUi.mSpinOffsetAlongLine->blockSignals( false );
      mUi.chkRotateMarker->blockSignals( true );
      mUi.chkRotateMarker->setChecked( mLayer->rotateSymbols() );
      mUi.chkRotateMarker->blockSignals( false );
      mUi.spinOffset->blockSignals( true );
      mUi.spinOffset->setValue( mLayer->offset() );
      mUi.spinOffset->blockSignals( false );

      whileBlocking( mUi.mCheckInterval )->setChecked( mLayer->placements() & Qgis::MarkerLinePlacement::Interval );
      whileBlocking( mUi.mCheckVertex )->setChecked( mLayer->placements() & Qgis::MarkerLinePlacement::InnerVertices || mLayer->placements() & Qgis::MarkerLinePlacement::Vertex );
      whileBlocking( mUi.mCheckVertexFirst )->setChecked( mLayer->placements() & Qgis::MarkerLinePlacement::FirstVertex || mLayer->placements() & Qgis::MarkerLinePlacement::Vertex );
      whileBlocking( mUi.mCheckVertexLast )->setChecked( mLayer->placements() & Qgis::MarkerLinePlacement::LastVertex || mLayer->placements() & Qgis::MarkerLinePlacement::Vertex );
      whileBlocking( mUi.mCheckCentralPoint )->setChecked( mLayer->placements() & Qgis::MarkerLinePlacement::CentralPoint );
      whileBlocking( mUi.mCheckCurvePoint )->setChecked( mLayer->placements() & Qgis::MarkerLinePlacement::CurvePoint );
      whileBlocking( mUi.mCheckSegmentCentralPoint )->setChecked( mLayer->placements() & Qgis::MarkerLinePlacement::SegmentCenter );
      whileBlocking( mUi.mCheckPlaceOnEveryPart )->setChecked( mLayer->placeOnEveryPart() );

      // set units
      mUi.mIntervalUnitWidget->blockSignals( true );
      mUi.mIntervalUnitWidget->setUnit( mLayer->intervalUnit() );
      mUi.mIntervalUnitWidget->setMapUnitScale( mLayer->intervalMapUnitScale() );
      mUi.mIntervalUnitWidget->blockSignals( false );
      mUi.mOffsetUnitWidget->blockSignals( true );
      mUi.mOffsetUnitWidget->setUnit( mLayer->offsetUnit() );
      mUi.mOffsetUnitWidget->setMapUnitScale( mLayer->offsetMapUnitScale() );
      mUi.mOffsetUnitWidget->blockSignals( false );
      mUi.mOffsetAlongLineUnitWidget->blockSignals( true );
      mUi.mOffsetAlongLineUnitWidget->setUnit( mLayer->offsetAlongLineUnit() );
      mUi.mOffsetAlongLineUnitWidget->setMapUnitScale( mLayer->offsetAlongLineMapUnitScale() );
      mUi.mOffsetAlongLineUnitWidget->blockSignals( false );

      whileBlocking( mUi.mAverageAngleUnit )->setUnit( mLayer->averageAngleUnit() );
      whileBlocking( mUi.mAverageAngleUnit )->setMapUnitScale( mLayer->averageAngleMapUnitScale() );
      whileBlocking( mUi.mSpinAverageAngleLength )->setValue( mLayer->averageAngleLength() );

      whileBlocking( mUi.mRingFilterComboBox )->setCurrentIndex( mUi.mRingFilterComboBox->findData( mLayer->ringFilter() ) );

      setPlacement(); // update gui

      registerDataDefinedButton( mUi.mIntervalDDBtn, QgsSymbolLayer::Property::Interval );
      registerDataDefinedButton( mUi.mLineOffsetDDBtn, QgsSymbolLayer::Property::Offset );
      registerDataDefinedButton( mUi.mPlacementDDBtn, QgsSymbolLayer::Property::Placement );
      registerDataDefinedButton( mUi.mOffsetAlongLineDDBtn, QgsSymbolLayer::Property::OffsetAlongLine );
      registerDataDefinedButton( mUi.mAverageAngleDDBtn, QgsSymbolLayer::Property::AverageAngleLength );
    }

    QgsSymbolLayer *symbolLayer() override
    {
      return mLayer;
    }

    void setContext( const QgsSymbolWidgetContext &context ) override
    {
      QgsSymbolLayerWidget::setContext( context );

      switch ( context.symbolType() )
      {
        case Qgis::SymbolType::Marker:
        case Qgis::SymbolType::Line:
          //these settings only have an effect when the symbol layers is part of a fill symbol
          mUi.mRingFilterComboBox->hide();
          mUi.mRingsLabel->hide();
          break;

        case Qgis::SymbolType::Fill:
        case Qgis::SymbolType::Hybrid:
          break;
      }
    }

  private slots:

    void setInterval( double val )
    {
      mLayer->setInterval( val );
      emit changed();
    }

    void setOffsetAlongLine( double val )
    {
      mLayer->setOffsetAlongLine( val );
      emit changed();
    }

    void setRotate()
    {
      mUi.mSpinAverageAngleLength->setEnabled( mUi.chkRotateMarker->isChecked() && ( mUi.mCheckInterval->isChecked() || mUi.mCheckCentralPoint->isChecked() ) );
      mUi.mAverageAngleUnit->setEnabled( mUi.mSpinAverageAngleLength->isEnabled() );

      mLayer->setRotateSymbols( mUi.chkRotateMarker->isChecked() );
      emit changed();
    }

    void setOffset()
    {
      mLayer->setOffset( mUi.spinOffset->value() );
      emit changed();
    }

    void setPlacement()
    {
      const bool interval = mUi.mCheckInterval->isChecked();
      mUi.spinInterval->setEnabled( interval );
      mUi.mSpinOffsetAlongLine->setEnabled( mUi.mCheckInterval->isChecked() || mUi.mCheckVertexLast->isChecked() || mUi.mCheckVertexFirst->isChecked() );
      mUi.mOffsetAlongLineUnitWidget->setEnabled( mUi.mSpinOffsetAlongLine->isEnabled() );
      mUi.mSpinAverageAngleLength->setEnabled( mUi.chkRotateMarker->isChecked() && ( mUi.mCheckInterval->isChecked() || mUi.mCheckCentralPoint->isChecked() ) );
      mUi.mAverageAngleUnit->setEnabled( mUi.mSpinAverageAngleLength->isEnabled() );
      mUi.mCheckPlaceOnEveryPart->setEnabled( mUi.mCheckVertexLast->isChecked() || mUi.mCheckVertexFirst->isChecked() );

      Qgis::MarkerLinePlacements placements;
      if ( mUi.mCheckInterval->isChecked() )
        placements |= Qgis::MarkerLinePlacement::Interval;
      if ( mUi.mCheckVertex->isChecked() )
        placements |= Qgis::MarkerLinePlacement::InnerVertices;
      if ( mUi.mCheckVertexLast->isChecked() )
        placements |= Qgis::MarkerLinePlacement::LastVertex;
      if ( mUi.mCheckVertexFirst->isChecked() )
        placements |= Qgis::MarkerLinePlacement::FirstVertex;
      if ( mUi.mCheckCurvePoint->isChecked() )
        placements |= Qgis::MarkerLinePlacement::CurvePoint;
      if ( mUi.mCheckSegmentCentralPoint->isChecked() )
        placements |= Qgis::MarkerLinePlacement::SegmentCenter;
      if ( mUi.mCheckCentralPoint->isChecked() )
        placements |= Qgis::MarkerLinePlacement::CentralPoint;
      mLayer->setPlacements( placements );

      emit changed();
    }

    void mIntervalUnitWidget_changed()
    {
      if ( mLayer )
      {
        mLayer->setIntervalUnit( mUi.mIntervalUnitWidget->unit() );
        mLayer->setIntervalMapUnitScale( mUi.mIntervalUnitWidget->getMapUnitScale() );
        emit changed();
      }
    }

    void mOffsetUnitWidget_changed()
    {
      if ( mLayer )
      {
        mLayer->setOffsetUnit( mUi.mOffsetUnitWidget->unit() );
        mLayer->setOffsetMapUnitScale( mUi.mOffsetUnitWidget->getMapUnitScale() );
        emit changed();
      }
    }

    void mOffsetAlongLineUnitWidget_changed()
    {
      if ( mLayer )
      {
        mLayer->setOffsetAlongLineUnit( mUi.mOffsetAlongLineUnitWidget->unit() );
        mLayer->setOffsetAlongLineMapUnitScale( mUi.mOffsetAlongLineUnitWidget->getMapUnitScale() );
      }
      emit changed();
    }

    void averageAngleUnitChanged()
    {
      if ( mLayer )
      {
        mLayer->setAverageAngleUnit( mUi.mAverageAngleUnit->unit() );
        mLayer->setAverageAngleMapUnitScale( mUi.mAverageAngleUnit->getMapUnitScale() );
      }
      emit changed();
    }

    void setAverageAngle( double val )
    {
      if ( mLayer )
      {
        mLayer->setAverageAngleLength( val );
        emit changed();
      }
    }

  protected:
    Ui mUi;
    SL *mLayer = nullptr;
};


#endif

/***************************************************************************
                             qgslayoutpropertieswidget.cpp
                             -----------------------------
    begin                : July 2017
    copyright            : (C) 2017 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************/
/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgslayoutpropertieswidget.h"
#include "qgslayout.h"
#include "qgslayoutsnapper.h"

QgsLayoutPropertiesWidget::QgsLayoutPropertiesWidget( QWidget *parent, QgsLayout *layout )
  : QgsPanelWidget( parent )
  , mLayout( layout )
{
  setupUi( this );
  setPanelTitle( tr( "Layout properties" ) );
  blockSignals( true );

  updateSnappingElements();

  mGridSpacingUnitsCombo->linkToWidget( mGridResolutionSpinBox );
  mGridOffsetUnitsComboBox->linkToWidget( mOffsetXSpinBox );
  mGridOffsetUnitsComboBox->linkToWidget( mOffsetYSpinBox );

  blockSignals( false );

  connect( mSnapToleranceSpinBox, static_cast < void ( QSpinBox::* )( int ) > ( &QSpinBox::valueChanged ), this, &QgsLayoutPropertiesWidget::snapToleranceChanged );

  connect( mGridOffsetUnitsComboBox, &QgsLayoutUnitsComboBox::changed, this, &QgsLayoutPropertiesWidget::gridOffsetUnitsChanged );
  connect( mGridSpacingUnitsCombo, &QgsLayoutUnitsComboBox::changed, this, &QgsLayoutPropertiesWidget::gridResolutionUnitsChanged );
  connect( mGridResolutionSpinBox, static_cast < void ( QgsDoubleSpinBox::* )( double ) > ( &QgsDoubleSpinBox::valueChanged ), this, &QgsLayoutPropertiesWidget::gridResolutionChanged );
  connect( mOffsetXSpinBox, static_cast < void ( QgsDoubleSpinBox::* )( double ) > ( &QgsDoubleSpinBox::valueChanged ), this, &QgsLayoutPropertiesWidget::gridOffsetXChanged );
  connect( mOffsetYSpinBox, static_cast < void ( QgsDoubleSpinBox::* )( double ) > ( &QgsDoubleSpinBox::valueChanged ), this, &QgsLayoutPropertiesWidget::gridOffsetYChanged );
}

void QgsLayoutPropertiesWidget::updateSnappingElements()
{
  mSnapToleranceSpinBox->setValue( mLayout->snapper().snapTolerance() );

  mGridSpacingUnitsCombo->setUnit( mLayout->snapper().gridResolution().units() );
  mGridResolutionSpinBox->setValue( mLayout->snapper().gridResolution().length() );

  mGridOffsetUnitsComboBox->setUnit( mLayout->snapper().gridOffset().units() );
  mOffsetXSpinBox->setValue( mLayout->snapper().gridOffset().x() );
  mOffsetYSpinBox->setValue( mLayout->snapper().gridOffset().y() );
}

void QgsLayoutPropertiesWidget::gridResolutionChanged( double d )
{
  QgsLayoutMeasurement m = mLayout->snapper().gridResolution();
  m.setLength( d );
  mLayout->snapper().setGridResolution( m );
  mLayout->pageCollection()->redraw();
}

void QgsLayoutPropertiesWidget::gridResolutionUnitsChanged( QgsUnitTypes::LayoutUnit unit )
{
  QgsLayoutMeasurement m = mLayout->snapper().gridResolution();
  m.setUnits( unit );
  mLayout->snapper().setGridResolution( m );
  mLayout->pageCollection()->redraw();
}

void QgsLayoutPropertiesWidget::gridOffsetXChanged( double d )
{
  QgsLayoutPoint o = mLayout->snapper().gridOffset();
  o.setX( d );
  mLayout->snapper().setGridOffset( o );
  mLayout->pageCollection()->redraw();
}

void QgsLayoutPropertiesWidget::gridOffsetYChanged( double d )
{
  QgsLayoutPoint o = mLayout->snapper().gridOffset();
  o.setY( d );
  mLayout->snapper().setGridOffset( o );
  mLayout->pageCollection()->redraw();
}

void QgsLayoutPropertiesWidget::gridOffsetUnitsChanged( QgsUnitTypes::LayoutUnit unit )
{
  QgsLayoutPoint o = mLayout->snapper().gridOffset();
  o.setUnits( unit );
  mLayout->snapper().setGridOffset( o );
  mLayout->pageCollection()->redraw();
}

void QgsLayoutPropertiesWidget::snapToleranceChanged( int tolerance )
{
  mLayout->snapper().setSnapTolerance( tolerance );
}

void QgsLayoutPropertiesWidget::blockSignals( bool block )
{
  mGridResolutionSpinBox->blockSignals( block );
  mOffsetXSpinBox->blockSignals( block );
  mOffsetYSpinBox->blockSignals( block );
  mSnapToleranceSpinBox->blockSignals( block );
}


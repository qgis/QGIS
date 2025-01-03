/***************************************************************************
                             qgslayoutunitscombobox.cpp
                             --------------------------
    Date                 : July 2017
    Copyright            : (C) 2017 Nyall Dawson
    Email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgslayoutunitscombobox.h"
#include "moc_qgslayoutunitscombobox.cpp"
#include "qgslayoutmeasurementconverter.h"
#include "qgsunittypes.h"
#include "qgis.h"

QgsLayoutUnitsComboBox::QgsLayoutUnitsComboBox( QWidget *parent )
  : QComboBox( parent )
{
  QList<Qgis::LayoutUnit> units;
  units << Qgis::LayoutUnit::Millimeters
        << Qgis::LayoutUnit::Centimeters
        << Qgis::LayoutUnit::Meters
        << Qgis::LayoutUnit::Inches
        << Qgis::LayoutUnit::Feet
        << Qgis::LayoutUnit::Points
        << Qgis::LayoutUnit::Picas
        << Qgis::LayoutUnit::Pixels;

  const auto constUnits = units;
  for ( const Qgis::LayoutUnit u : constUnits )
  {
    addItem( QgsUnitTypes::toAbbreviatedString( u ), static_cast<int>( u ) );
    setItemData( count() - 1, QgsUnitTypes::toString( u ), Qt::ToolTipRole );
  }
  connect( this, static_cast<void ( QgsLayoutUnitsComboBox::* )( int )>( &QgsLayoutUnitsComboBox::currentIndexChanged ), this, &QgsLayoutUnitsComboBox::indexChanged );
}

Qgis::LayoutUnit QgsLayoutUnitsComboBox::unit() const
{
  return static_cast<Qgis::LayoutUnit>( currentData().toInt() );
}

void QgsLayoutUnitsComboBox::setUnit( Qgis::LayoutUnit unit )
{
  setCurrentIndex( findData( static_cast<int>( unit ) ) );
}

void QgsLayoutUnitsComboBox::linkToWidget( QDoubleSpinBox *widget )
{
  mLinkedSpinBoxes << widget;
}

void QgsLayoutUnitsComboBox::indexChanged( int )
{
  const Qgis::LayoutUnit newUnit = unit();
  if ( mConverter )
  {
    const auto constMLinkedSpinBoxes = mLinkedSpinBoxes;
    for ( const QPointer<QDoubleSpinBox> &widget : constMLinkedSpinBoxes )
    {
      if ( widget )
        whileBlocking( widget.data() )->setValue( mConverter->convert( QgsLayoutMeasurement( widget->value(), mOldUnit ), newUnit ).length() );
    }
  }
  emit unitChanged( newUnit );
  emit changed( static_cast<int>( newUnit ) );
  mOldUnit = newUnit;
}

QgsLayoutMeasurementConverter *QgsLayoutUnitsComboBox::converter() const
{
  return mConverter;
}

void QgsLayoutUnitsComboBox::setConverter( QgsLayoutMeasurementConverter *converter )
{
  mConverter = converter;
}

#include "qgslayoutunitscombobox.h"

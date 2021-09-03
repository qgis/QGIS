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
#include "qgslayoutmeasurementconverter.h"
#include "qgis.h"

QgsLayoutUnitsComboBox::QgsLayoutUnitsComboBox( QWidget *parent )
  : QComboBox( parent )
{
  QList< QgsUnitTypes::LayoutUnit > units;
  units << QgsUnitTypes::LayoutMillimeters
        << QgsUnitTypes::LayoutCentimeters
        << QgsUnitTypes::LayoutMeters
        << QgsUnitTypes::LayoutInches
        << QgsUnitTypes::LayoutFeet
        << QgsUnitTypes::LayoutPoints
        << QgsUnitTypes::LayoutPicas
        << QgsUnitTypes::LayoutPixels;

  const auto constUnits = units;
  for ( const QgsUnitTypes::LayoutUnit u : constUnits )
  {
    addItem( QgsUnitTypes::toAbbreviatedString( u ), u );
    setItemData( count() - 1, QgsUnitTypes::toString( u ), Qt::ToolTipRole );
  }
  connect( this, static_cast<void ( QgsLayoutUnitsComboBox::* )( int )>( &QgsLayoutUnitsComboBox::currentIndexChanged ), this, &QgsLayoutUnitsComboBox::indexChanged );
}

QgsUnitTypes::LayoutUnit QgsLayoutUnitsComboBox::unit() const
{
  return static_cast< QgsUnitTypes::LayoutUnit >( currentData().toInt() );
}

void QgsLayoutUnitsComboBox::setUnit( QgsUnitTypes::LayoutUnit unit )
{
  setCurrentIndex( findData( unit ) );
}

void QgsLayoutUnitsComboBox::linkToWidget( QDoubleSpinBox *widget )
{
  mLinkedSpinBoxes << widget;
}

void QgsLayoutUnitsComboBox::indexChanged( int )
{
  const QgsUnitTypes::LayoutUnit newUnit = unit();
  if ( mConverter )
  {
    const auto constMLinkedSpinBoxes = mLinkedSpinBoxes;
    for ( const QPointer< QDoubleSpinBox > &widget : constMLinkedSpinBoxes )
    {
      if ( widget )
        whileBlocking( widget.data() )->setValue( mConverter->convert( QgsLayoutMeasurement( widget->value(), mOldUnit ), newUnit ).length() );
    }
  }
  emit changed( newUnit );
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

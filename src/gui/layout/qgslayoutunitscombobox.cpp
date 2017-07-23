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

QgsLayoutUnitsComboBox::QgsLayoutUnitsComboBox( QWidget *parent )
  : QComboBox( parent )
{
  addItem( tr( "mm" ), QgsUnitTypes::LayoutMillimeters );
  setItemData( 0, tr( "Millimeters" ), Qt::ToolTipRole );
  addItem( tr( "cm" ), QgsUnitTypes::LayoutCentimeters );
  setItemData( 1, tr( "Centimeters" ), Qt::ToolTipRole );
  addItem( tr( "m" ), QgsUnitTypes::LayoutMeters );
  setItemData( 2, tr( "Meters" ), Qt::ToolTipRole );
  addItem( tr( "in" ), QgsUnitTypes::LayoutInches );
  setItemData( 3, tr( "Inches" ), Qt::ToolTipRole );
  addItem( tr( "ft" ), QgsUnitTypes::LayoutFeet );
  setItemData( 4, tr( "Feet" ), Qt::ToolTipRole );
  addItem( tr( "pt" ), QgsUnitTypes::LayoutPoints );
  setItemData( 5, tr( "Points" ), Qt::ToolTipRole );
  addItem( tr( "pica" ), QgsUnitTypes::LayoutPicas );
  setItemData( 6, tr( "Picas" ), Qt::ToolTipRole );
  addItem( tr( "px" ), QgsUnitTypes::LayoutPixels );
  setItemData( 7, tr( "Pixels" ), Qt::ToolTipRole );
  connect( this, static_cast<void ( QgsLayoutUnitsComboBox::* )( int )>( &QgsLayoutUnitsComboBox::currentIndexChanged ), this, [ = ]( int )
  {
    emit changed( unit() );
  } );
}

QgsUnitTypes::LayoutUnit QgsLayoutUnitsComboBox::unit() const
{
  return static_cast< QgsUnitTypes::LayoutUnit >( currentData().toInt() );
}

void QgsLayoutUnitsComboBox::setUnit( QgsUnitTypes::LayoutUnit unit )
{
  setCurrentIndex( findData( unit ) );
}

#include "qgslayoutunitscombobox.h"

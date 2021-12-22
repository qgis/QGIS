/***************************************************************************
                              qgsblendmodecombobox.cpp
                              ------------------------
  begin                : March 21, 2013
  copyright            : (C) 2013 by Nyall Dawson
  email                : nyall.dawson@gmail.com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgis.h"
#include "qgslogger.h"
#include "qgsblendmodecombobox.h"
#include "qgspainting.h"

#include <QAbstractItemView>
#include <QLocale>
#include <QSettings>
#include <QLineEdit>

QgsBlendModeComboBox::QgsBlendModeComboBox( QWidget *parent )
  : QComboBox( parent )
{
  setSizeAdjustPolicy( QComboBox::AdjustToMinimumContentsLengthWithIcon );
  updateModes();
}

void QgsBlendModeComboBox::updateModes()
{
  blockSignals( true );
  clear();

  // This list is designed to emulate GIMP's layer modes, where
  // blending modes are grouped by their effect (lightening, darkening, etc)

  addItem( tr( "Normal" ), static_cast< int >( QgsPainting::BlendMode::BlendNormal ) );
  insertSeparator( count() );
  addItem( tr( "Lighten" ), static_cast< int >( QgsPainting::BlendMode::BlendLighten ) );
  addItem( tr( "Screen" ), static_cast< int >( QgsPainting::BlendMode::BlendScreen ) );
  addItem( tr( "Dodge" ), static_cast< int >( QgsPainting::BlendMode::BlendDodge ) );
  addItem( tr( "Addition" ), static_cast< int >( QgsPainting::BlendMode::BlendAddition ) );
  insertSeparator( count() );
  addItem( tr( "Darken" ), static_cast< int >( QgsPainting::BlendMode::BlendDarken ) );
  addItem( tr( "Multiply" ), static_cast< int >( QgsPainting::BlendMode::BlendMultiply ) );
  addItem( tr( "Burn" ), static_cast< int >( QgsPainting::BlendMode::BlendBurn ) );
  insertSeparator( count() );
  addItem( tr( "Overlay" ), static_cast< int >( QgsPainting::BlendMode::BlendOverlay ) );
  addItem( tr( "Soft Light" ), static_cast< int >( QgsPainting::BlendMode::BlendSoftLight ) );
  addItem( tr( "Hard Light" ), static_cast< int >( QgsPainting::BlendMode::BlendHardLight ) );
  insertSeparator( count() );
  addItem( tr( "Difference" ), static_cast< int >( QgsPainting::BlendMode::BlendDifference ) );
  addItem( tr( "Subtract" ), static_cast< int >( QgsPainting::BlendMode::BlendSubtract ) );

  if ( mShowClipModes )
  {
    insertSeparator( count() );
    addItem( tr( "Masked By Below" ), static_cast< int >( QgsPainting::BlendMode::BlendSourceIn ) );
    addItem( tr( "Mask Below" ), static_cast< int >( QgsPainting::BlendMode::BlendDestinationIn ) );
    addItem( tr( "Inverse Masked By Below" ), static_cast< int >( QgsPainting::BlendMode::BlendSourceOut ) );
    addItem( tr( "Inverse Mask Below" ), static_cast< int >( QgsPainting::BlendMode::BlendDestinationOut ) );
    addItem( tr( "Paint Inside Below" ), static_cast< int >( QgsPainting::BlendMode::BlendSourceAtop ) );
    addItem( tr( "Paint Below Inside" ), static_cast< int >( QgsPainting::BlendMode::BlendDestinationAtop ) );
  }

  blockSignals( false );
}

QPainter::CompositionMode QgsBlendModeComboBox::blendMode()
{
  return QgsPainting::getCompositionMode( static_cast< QgsPainting::BlendMode >( currentData().toInt() ) );
}

void QgsBlendModeComboBox::setBlendMode( QPainter::CompositionMode blendMode )
{
  setCurrentIndex( findData( static_cast< int >( QgsPainting::getBlendModeEnum( blendMode ) ) ) );
}

void QgsBlendModeComboBox::setShowClippingModes( bool show )
{
  mShowClipModes = show;
  const QPainter::CompositionMode mode = blendMode();
  updateModes();

  setBlendMode( mode );
}

bool QgsBlendModeComboBox::showClippingModes() const
{
  return mShowClipModes;
}


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
#include "qgsblendmodecombobox.h"
#include "moc_qgsblendmodecombobox.cpp"
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

  addItem( tr( "Normal" ), static_cast<int>( Qgis::BlendMode::Normal ) );
  insertSeparator( count() );
  addItem( tr( "Lighten" ), static_cast<int>( Qgis::BlendMode::Lighten ) );
  addItem( tr( "Screen" ), static_cast<int>( Qgis::BlendMode::Screen ) );
  addItem( tr( "Dodge" ), static_cast<int>( Qgis::BlendMode::Dodge ) );
  addItem( tr( "Addition" ), static_cast<int>( Qgis::BlendMode::Addition ) );
  insertSeparator( count() );
  addItem( tr( "Darken" ), static_cast<int>( Qgis::BlendMode::Darken ) );
  addItem( tr( "Multiply" ), static_cast<int>( Qgis::BlendMode::Multiply ) );
  addItem( tr( "Burn" ), static_cast<int>( Qgis::BlendMode::Burn ) );
  insertSeparator( count() );
  addItem( tr( "Overlay" ), static_cast<int>( Qgis::BlendMode::Overlay ) );
  addItem( tr( "Soft Light" ), static_cast<int>( Qgis::BlendMode::SoftLight ) );
  addItem( tr( "Hard Light" ), static_cast<int>( Qgis::BlendMode::HardLight ) );
  insertSeparator( count() );
  addItem( tr( "Difference" ), static_cast<int>( Qgis::BlendMode::Difference ) );
  addItem( tr( "Subtract" ), static_cast<int>( Qgis::BlendMode::Subtract ) );

  if ( mShowClipModes )
  {
    insertSeparator( count() );
    addItem( tr( "Masked By Below" ), static_cast<int>( Qgis::BlendMode::SourceIn ) );
    addItem( tr( "Mask Below" ), static_cast<int>( Qgis::BlendMode::DestinationIn ) );
    addItem( tr( "Inverse Masked By Below" ), static_cast<int>( Qgis::BlendMode::SourceOut ) );
    addItem( tr( "Inverse Mask Below" ), static_cast<int>( Qgis::BlendMode::DestinationOut ) );
    addItem( tr( "Paint Inside Below" ), static_cast<int>( Qgis::BlendMode::SourceAtop ) );
    addItem( tr( "Paint Below Inside" ), static_cast<int>( Qgis::BlendMode::DestinationAtop ) );
  }

  blockSignals( false );
}

QPainter::CompositionMode QgsBlendModeComboBox::blendMode()
{
  return QgsPainting::getCompositionMode( static_cast<Qgis::BlendMode>( currentData().toInt() ) );
}

void QgsBlendModeComboBox::setBlendMode( QPainter::CompositionMode blendMode )
{
  setCurrentIndex( findData( static_cast<int>( QgsPainting::getBlendModeEnum( blendMode ) ) ) );
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

/***************************************************************************
    qgscolorrampcombobox.cpp
    ---------------------
    begin                : October 2010
    copyright            : (C) 2010 by Martin Dobias
    email                : wonder dot sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "qgscolorrampcombobox.h"

#include "qgssymbollayerutils.h"
#include "qgsvectorcolorramp.h"
#include "qgsstyle.h"
#include "qgsstylemanagerdialog.h"

#include "qgsvectorgradientcolorrampdialog.h"
#include "qgsvectorrandomcolorrampdialog.h"
#include "qgsvectorcolorbrewercolorrampdialog.h"
#include "qgscptcitycolorrampdialog.h"

QSize QgsColorRampComboBox::rampIconSize( 50, 16 );

QgsColorRampComboBox::QgsColorRampComboBox( QWidget *parent )
    : QComboBox( parent )
    , mStyle( nullptr )
    , mSourceColorRamp( nullptr )
    , mShowGradientOnly( false )
{
}

QgsColorRampComboBox::~QgsColorRampComboBox()
{
  delete mSourceColorRamp;
}

void QgsColorRampComboBox::populate( QgsStyle* style )
{
  if ( count() != 0 )
    return; // already populated!

  mStyle = style;

  setIconSize( rampIconSize );

  QStringList rampNames = mStyle->colorRampNames();
  for ( QStringList::iterator it = rampNames.begin(); it != rampNames.end(); ++it )
  {
    QgsVectorColorRamp* ramp = style->colorRamp( *it );

    if ( !mShowGradientOnly || ramp->type() == "gradient" )
    {
      QIcon icon = QgsSymbolLayerUtils::colorRampPreviewIcon( ramp, rampIconSize );

      addItem( icon, *it );
    }
    delete ramp;
  }

  if ( !mShowGradientOnly )
    addItem( tr( "Random colors" ) );
  addItem( tr( "New color ramp..." ) );
  connect( this, SIGNAL( activated( int ) ), SLOT( colorRampChanged( int ) ) );
}

QgsVectorColorRamp* QgsColorRampComboBox::currentColorRamp()
{
  QString rampName = currentText();

  if ( rampName == tr( "Random colors" ) )
  {
    return new QgsRandomColors();
  }
  else if ( rampName == "[source]" && mSourceColorRamp )
    return mSourceColorRamp->clone();
  else
    return mStyle->colorRamp( rampName );
}

bool QgsColorRampComboBox::createNewColorRampSelected() const
{
  int index = currentIndex();
  return index == count() - 1; //create new ramp is last item in combobox
}

void QgsColorRampComboBox::setSourceColorRamp( QgsVectorColorRamp* sourceRamp )
{
  delete mSourceColorRamp;
  mSourceColorRamp = sourceRamp->clone();

  QIcon icon = QgsSymbolLayerUtils::colorRampPreviewIcon( mSourceColorRamp, rampIconSize );
  if ( itemText( 0 ) == "[source]" )
    setItemIcon( 0, icon );
  else
    insertItem( 0, icon, "[source]" );
  setCurrentIndex( 0 );
}

void QgsColorRampComboBox::colorRampChanged( int index )
{
  if ( index != count() - 1 )
    return;

  // last item: "new color ramp..."
  QString rampName;
  if ( !mShowGradientOnly )
  {
    rampName = QgsStyleManagerDialog::addColorRampStatic( this, mStyle );
  }
  else
  {
    rampName = QgsStyleManagerDialog::addColorRampStatic( this, mStyle, "Gradient" );
  }
  if ( rampName.isEmpty() )
    return;

  // put newly added ramp into the combo
  QgsVectorColorRamp* ramp = mStyle->colorRamp( rampName );
  QIcon icon = QgsSymbolLayerUtils::colorRampPreviewIcon( ramp, rampIconSize );

  blockSignals( true ); // avoid calling this method again!
  insertItem( index, icon, rampName );
  blockSignals( false );

  delete ramp;

  // ... and set it as active
  setCurrentIndex( index );

  // make sure the color ramp is stored
  mStyle->save();
}

void QgsColorRampComboBox::editSourceRamp()
{
  QgsVectorColorRamp* currentRamp = currentColorRamp();
  if ( !currentRamp )
    return;

  QScopedPointer<QgsVectorColorRamp> newRamp( currentRamp->clone() );

  if ( newRamp->type() == "gradient" )
  {
    QgsVectorGradientColorRamp* gradRamp = static_cast<QgsVectorGradientColorRamp*>( newRamp.data() );
    QgsVectorGradientColorRampDialog dlg( gradRamp, this );
    if ( dlg.exec() && gradRamp )
    {
      setSourceColorRamp( gradRamp );
      emit sourceRampEdited();
    }
  }
  else if ( newRamp->type() == "random" )
  {
    QgsVectorRandomColorRamp* randRamp = static_cast<QgsVectorRandomColorRamp*>( newRamp.data() );
    QgsVectorRandomColorRampDialog dlg( randRamp, this );
    if ( dlg.exec() )
    {
      setSourceColorRamp( randRamp );
      emit sourceRampEdited();
    }
  }
  else if ( newRamp->type() == "colorbrewer" )
  {
    QgsVectorColorBrewerColorRamp* brewerRamp = static_cast<QgsVectorColorBrewerColorRamp*>( newRamp.data() );
    QgsVectorColorBrewerColorRampDialog dlg( brewerRamp, this );
    if ( dlg.exec() )
    {
      setSourceColorRamp( brewerRamp );
      emit sourceRampEdited();
    }
  }
  else if ( newRamp->type() == "cpt-city" )
  {
    QgsCptCityColorRamp* cptCityRamp = static_cast<QgsCptCityColorRamp*>( newRamp.data() );
    QgsCptCityColorRampDialog dlg( cptCityRamp, this );
    if ( dlg.exec() && cptCityRamp )
    {
      setSourceColorRamp( cptCityRamp );
      emit sourceRampEdited();
    }
  }
}

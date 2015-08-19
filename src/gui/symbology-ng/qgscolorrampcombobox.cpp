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

#include "qgssymbollayerv2utils.h"
#include "qgsvectorcolorrampv2.h"
#include "qgsstylev2.h"
#include "qgsstylev2managerdialog.h"

#include "qgsvectorgradientcolorrampv2dialog.h"
#include "qgsvectorrandomcolorrampv2dialog.h"
#include "qgsvectorcolorbrewercolorrampv2dialog.h"
#include "qgscptcitycolorrampv2dialog.h"

QSize QgsColorRampComboBox::rampIconSize( 50, 16 );

QgsColorRampComboBox::QgsColorRampComboBox( QWidget *parent ) :
    QComboBox( parent ), mStyle( NULL ), mSourceColorRamp( NULL ), mShowGradientOnly( false )
{
}

QgsColorRampComboBox::~QgsColorRampComboBox()
{
  delete mSourceColorRamp;
}

void QgsColorRampComboBox::populate( QgsStyleV2* style )
{
  if ( count() != 0 )
    return; // already populated!

  mStyle = style;

  setIconSize( rampIconSize );

  QStringList rampNames = mStyle->colorRampNames();
  for ( QStringList::iterator it = rampNames.begin(); it != rampNames.end(); ++it )
  {
    QgsVectorColorRampV2* ramp = style->colorRamp( *it );

    if ( !mShowGradientOnly || ramp->type() == "gradient" )
    {
      QIcon icon = QgsSymbolLayerV2Utils::colorRampPreviewIcon( ramp, rampIconSize );

      addItem( icon, *it );
    }
    delete ramp;
  }

  if ( !mShowGradientOnly )
    addItem( tr( "Random colors" ) );
  addItem( tr( "New color ramp..." ) );
  connect( this, SIGNAL( activated( int ) ), SLOT( colorRampChanged( int ) ) );
}

QgsVectorColorRampV2* QgsColorRampComboBox::currentColorRamp()
{
  QString rampName = currentText();

  if ( rampName == tr( "Random colors" ) )
  {
    return new QgsRandomColorsV2();
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

void QgsColorRampComboBox::setSourceColorRamp( QgsVectorColorRampV2* sourceRamp )
{
  mSourceColorRamp = sourceRamp->clone();

  QIcon icon = QgsSymbolLayerV2Utils::colorRampPreviewIcon( mSourceColorRamp, rampIconSize );
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
    rampName = QgsStyleV2ManagerDialog::addColorRampStatic( this, mStyle );
  }
  else
  {
    rampName = QgsStyleV2ManagerDialog::addColorRampStatic( this, mStyle, "Gradient" );
  }
  if ( rampName.isEmpty() )
    return;

  // put newly added ramp into the combo
  QgsVectorColorRampV2* ramp = mStyle->colorRamp( rampName );
  QIcon icon = QgsSymbolLayerV2Utils::colorRampPreviewIcon( ramp, rampIconSize );

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
  QgsVectorColorRampV2* currentRamp = currentColorRamp();
  if ( !currentRamp )
    return;

  QScopedPointer<QgsVectorColorRampV2> newRamp( currentRamp->clone() );

  if ( newRamp->type() == "gradient" )
  {
    QgsVectorGradientColorRampV2* gradRamp = static_cast<QgsVectorGradientColorRampV2*>( newRamp.data() );
    QgsVectorGradientColorRampV2Dialog dlg( gradRamp, this );
    if ( dlg.exec() && gradRamp )
    {
      setSourceColorRamp( gradRamp );
      emit sourceRampEdited();
    }
  }
  else if ( newRamp->type() == "random" )
  {
    QgsVectorRandomColorRampV2* randRamp = static_cast<QgsVectorRandomColorRampV2*>( newRamp.data() );
    QgsVectorRandomColorRampV2Dialog dlg( randRamp, this );
    if ( dlg.exec() )
    {
      setSourceColorRamp( randRamp );
      emit sourceRampEdited();
    }
  }
  else if ( newRamp->type() == "colorbrewer" )
  {
    QgsVectorColorBrewerColorRampV2* brewerRamp = static_cast<QgsVectorColorBrewerColorRampV2*>( newRamp.data() );
    QgsVectorColorBrewerColorRampV2Dialog dlg( brewerRamp, this );
    if ( dlg.exec() )
    {
      setSourceColorRamp( brewerRamp );
      emit sourceRampEdited();
    }
  }
  else if ( newRamp->type() == "cpt-city" )
  {
    QgsCptCityColorRampV2* cptCityRamp = static_cast<QgsCptCityColorRampV2*>( newRamp.data() );
    QgsCptCityColorRampV2Dialog dlg( cptCityRamp, this );
    if ( dlg.exec() && cptCityRamp )
    {
      setSourceColorRamp( cptCityRamp );
      emit sourceRampEdited();
    }
  }
}

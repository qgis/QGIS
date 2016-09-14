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
#include "qgscolorramp.h"
#include "qgsstyle.h"
#include "qgsstylemanagerdialog.h"

#include "qgsgradientcolorrampdialog.h"
#include "qgslimitedrandomcolorrampdialog.h"
#include "qgscolorbrewercolorrampdialog.h"
#include "qgscptcitycolorrampdialog.h"
#include "qgspresetcolorrampdialog.h"

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
    QScopedPointer< QgsColorRamp > ramp( style->colorRamp( *it ) );

    if ( !mShowGradientOnly || ramp->type() == "gradient" )
    {
      QIcon icon = QgsSymbolLayerUtils::colorRampPreviewIcon( ramp.data(), rampIconSize );

      addItem( icon, *it );
    }
  }

  if ( !mShowGradientOnly )
    addItem( tr( "Random colors" ) );
  addItem( tr( "New color ramp..." ) );
  connect( this, SIGNAL( activated( int ) ), SLOT( colorRampChanged( int ) ) );
}

QgsColorRamp* QgsColorRampComboBox::currentColorRamp() const
{
  QString rampName = currentText();

  if ( rampName == tr( "Random colors" ) )
  {
    return new QgsRandomColorRamp();
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

void QgsColorRampComboBox::setSourceColorRamp( QgsColorRamp* sourceRamp )
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
  QScopedPointer< QgsColorRamp > ramp( mStyle->colorRamp( rampName ) );
  QIcon icon = QgsSymbolLayerUtils::colorRampPreviewIcon( ramp.data(), rampIconSize );

  blockSignals( true ); // avoid calling this method again!
  insertItem( index, icon, rampName );
  blockSignals( false );

  // ... and set it as active
  setCurrentIndex( index );

  // make sure the color ramp is stored
  mStyle->save();
}

void QgsColorRampComboBox::editSourceRamp()
{
  QgsPanelWidget* panel = QgsPanelWidget::findParentPanel( this );
  bool panelMode = panel && panel->dockMode();

  QScopedPointer< QgsColorRamp > currentRamp( currentColorRamp() );
  if ( !currentRamp )
    return;

  if ( currentRamp->type() == "gradient" )
  {
    QgsGradientColorRamp* gradRamp = static_cast<QgsGradientColorRamp*>( currentRamp.data() );
    QgsGradientColorRampDialog dlg( *gradRamp, this );
    if ( dlg.exec() )
    {
      setSourceColorRamp( dlg.ramp().clone() );
      emit sourceRampEdited();
    }
  }
  else if ( currentRamp->type() == "random" )
  {
    QgsLimitedRandomColorRamp* randRamp = static_cast<QgsLimitedRandomColorRamp*>( currentRamp.data() );
    if ( panelMode )
    {
      QgsLimitedRandomColorRampWidget* widget = new QgsLimitedRandomColorRampWidget( *randRamp, this );
      widget->setPanelTitle( tr( "Edit ramp" ) );
      connect( widget, SIGNAL( changed() ), this, SLOT( rampWidgetUpdated() ) );
      panel->openPanel( widget );
    }
    else
    {
      QgsLimitedRandomColorRampDialog dlg( *randRamp, this );
      if ( dlg.exec() )
      {
        setSourceColorRamp( dlg.ramp().clone() );
        emit sourceRampEdited();
      }
    }
  }
  else if ( currentRamp->type() == "preset" )
  {
    QgsPresetSchemeColorRamp* presetRamp = static_cast<QgsPresetSchemeColorRamp*>( currentRamp.data() );
    if ( panelMode )
    {
      QgsPresetColorRampWidget* widget = new QgsPresetColorRampWidget( *presetRamp, this );
      widget->setPanelTitle( tr( "Edit ramp" ) );
      connect( widget, SIGNAL( changed() ), this, SLOT( rampWidgetUpdated() ) );
      panel->openPanel( widget );
    }
    else
    {
      QgsPresetColorRampDialog dlg( *presetRamp, this );
      if ( dlg.exec() )
      {
        setSourceColorRamp( dlg.ramp().clone() );
        emit sourceRampEdited();
      }
    }
  }
  else if ( currentRamp->type() == "colorbrewer" )
  {
    QgsColorBrewerColorRamp* brewerRamp = static_cast<QgsColorBrewerColorRamp*>( currentRamp.data() );
    if ( panelMode )
    {
      QgsColorBrewerColorRampWidget* widget = new QgsColorBrewerColorRampWidget( *brewerRamp, this );
      widget->setPanelTitle( tr( "Edit ramp" ) );
      connect( widget, SIGNAL( changed() ), this, SLOT( rampWidgetUpdated() ) );
      panel->openPanel( widget );
    }
    else
    {
      QgsColorBrewerColorRampDialog dlg( *brewerRamp, this );
      if ( dlg.exec() )
      {
        setSourceColorRamp( dlg.ramp().clone() );
        emit sourceRampEdited();
      }
    }
  }
  else if ( currentRamp->type() == "cpt-city" )
  {
    QgsCptCityColorRamp* cptCityRamp = static_cast<QgsCptCityColorRamp*>( currentRamp.data() );
    QgsCptCityColorRampDialog dlg( *cptCityRamp, this );
    if ( dlg.exec() )
    {
      if ( dlg.saveAsGradientRamp() )
      {
        setSourceColorRamp( dlg.ramp().cloneGradientRamp() );
      }
      else
      {
        setSourceColorRamp( dlg.ramp().clone() );
      }
      emit sourceRampEdited();
    }
  }
}

void QgsColorRampComboBox::rampWidgetUpdated()
{
  QgsLimitedRandomColorRampWidget* limitedRampWidget = qobject_cast< QgsLimitedRandomColorRampWidget* >( sender() );
  if ( limitedRampWidget )
  {
    setSourceColorRamp( limitedRampWidget->ramp().clone() );
    emit sourceRampEdited();
    return;
  }
  QgsColorBrewerColorRampWidget* colorBrewerRampWidget = qobject_cast< QgsColorBrewerColorRampWidget* >( sender() );
  if ( colorBrewerRampWidget )
  {
    setSourceColorRamp( colorBrewerRampWidget->ramp().clone() );
    emit sourceRampEdited();
    return;
  }
  QgsPresetColorRampWidget* presetRampWidget = qobject_cast< QgsPresetColorRampWidget* >( sender() );
  if ( presetRampWidget )
  {
    setSourceColorRamp( presetRampWidget->ramp().clone() );
    emit sourceRampEdited();
    return;
  }
}

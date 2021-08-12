/***************************************************************************
    qgsrasterbandcombobox.cpp
    -------------------------
    begin                : May 2017
    copyright            : (C) 2017 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsrasterbandcombobox.h"
#include "qgsrasterlayer.h"
#include "qgsrasterdataprovider.h"

QgsRasterBandComboBox::QgsRasterBandComboBox( QWidget *parent )
  : QComboBox( parent )
  , mNotSetString( tr( "Not set" ) )
{
  connect( this, static_cast<void ( QComboBox::* )( int )>( &QComboBox::currentIndexChanged ), this, [ = ]
  {
    if ( mLayer && mLayer->isValid() )
    {
      const int newBand = currentIndex() >= 0 ? currentData().toInt() : -1 ;
      if ( newBand != mPrevBand )
      {
        emit bandChanged( currentIndex() >= 0 ? currentData().toInt() : -1 );
        mPrevBand = newBand;
      }
    }
  } );

  connect( this, &QComboBox::currentTextChanged, this, [ = ]( const QString & value )
  {
    if ( !mLayer || !mLayer->isValid() )
    {
      bool ok = false;
      const int band = value.toInt( &ok );
      if ( ok && band != mPrevBand )
      {
        emit bandChanged( band );
        mPrevBand = band;
      }
      else if ( mShowNotSet && mPrevBand != -1 )
      {
        emit bandChanged( -1 );
        mPrevBand = -1;
      }
    }
  } );

  // default to editable, until a layer is set
  setEditable( true );
}

QgsRasterLayer *QgsRasterBandComboBox::layer() const
{
  return mLayer;
}

int QgsRasterBandComboBox::currentBand() const
{
  if ( !mLayer || !mLayer->dataProvider() || !mLayer->isValid() )
  {
    bool ok = false;
    const int band = currentText().toInt( &ok );
    if ( ok )
      return band;
    return -1;
  }
  else
  {
    if ( currentIndex() < 0 )
      return -1;

    return currentData().toInt();
  }
}

void QgsRasterBandComboBox::setLayer( QgsMapLayer *layer )
{
  const int oldBand = currentBand();

  QgsRasterLayer *rl = qobject_cast< QgsRasterLayer * >( layer );
  mLayer = rl;

  blockSignals( true );
  clear();

  if ( mShowNotSet )
    addItem( mNotSetString, -1 );

  if ( mLayer )
  {
    QgsRasterDataProvider *provider = mLayer->dataProvider();
    if ( provider && mLayer->isValid() )
    {
      setEditable( false );
      //fill available bands into combo box
      const int nBands = provider->bandCount();
      for ( int i = 1; i <= nBands; ++i ) //band numbering seem to start at 1
      {
        addItem( displayBandName( provider, i ), i );
      }
    }
    else
    {
      setEditable( true );
    }
  }
  else
  {
    setEditable( true );
  }

  if ( oldBand >= 0 )
    setBand( oldBand );
  else
    setCurrentIndex( 0 );

  blockSignals( false );
  const int newBand = currentBand();
  //if ( newBand != oldBand )
  //  emit bandChanged( newBand );
  mPrevBand = newBand;
}

void QgsRasterBandComboBox::setBand( int band )
{
  if ( !mLayer || !mLayer->dataProvider() || !mLayer->isValid() )
  {
    if ( band < 0 )
    {
      setCurrentIndex( -1 );
      if ( mPrevBand != -1 )
        emit bandChanged( -1 );
    }
    else
      setCurrentText( QString::number( band ) );
  }
  else
  {
    setCurrentIndex( findData( band ) );
  }
  mPrevBand = band;
}

bool QgsRasterBandComboBox::isShowingNotSetOption() const
{
  return mShowNotSet;
}

void QgsRasterBandComboBox::setShowNotSetOption( bool show, const QString &string )
{
  mShowNotSet = show;
  mNotSetString = string.isEmpty() ? tr( "Not set" ) : string;
  setLayer( mLayer );
}

QString QgsRasterBandComboBox::displayBandName( QgsRasterDataProvider *provider, int band )
{
  if ( !provider )
    return QString();

  return provider->displayBandName( band );
}

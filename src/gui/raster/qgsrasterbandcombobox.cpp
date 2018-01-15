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
    emit bandChanged( currentIndex() >= 0 ? currentData().toInt() : -1 );
  } );
}

QgsRasterLayer *QgsRasterBandComboBox::layer() const
{
  return mLayer;
}

int QgsRasterBandComboBox::currentBand() const
{
  return currentIndex() >= 0 ? currentData().toInt() : -1;
}

void QgsRasterBandComboBox::setLayer( QgsMapLayer *layer )
{
  QgsRasterLayer *rl = qobject_cast< QgsRasterLayer * >( layer );
  mLayer = rl;
  int oldBand = currentBand();

  blockSignals( true );
  clear();

  if ( mShowNotSet )
    addItem( mNotSetString, -1 );

  if ( mLayer )
  {
    QgsRasterDataProvider *provider = mLayer->dataProvider();
    if ( provider )
    {
      //fill available bands into combo box
      int nBands = provider->bandCount();
      for ( int i = 1; i <= nBands; ++i ) //band numbering seem to start at 1
      {
        addItem( displayBandName( provider, i ), i );
      }
    }
  }

  if ( count() > 0 )
    setCurrentIndex( findData( oldBand ) >= 0 ? findData( oldBand ) : 0 );

  blockSignals( false );
  emit bandChanged( currentIndex() >= 0 ? currentData().toInt() : -1 );
}

void QgsRasterBandComboBox::setBand( int band )
{
  setCurrentIndex( findData( band ) );
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

QString QgsRasterBandComboBox::displayBandName( QgsRasterDataProvider *provider, int band ) const
{
  if ( !provider )
    return QString();

  QString name = provider->generateBandName( band );

  QString colorInterp = provider->colorInterpretationName( band );
  if ( colorInterp != QLatin1String( "Undefined" ) )
  {
    name.append( QStringLiteral( " (%1)" ).arg( colorInterp ) );
  }
  return name;
}

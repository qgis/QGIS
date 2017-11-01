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

QgsBlendModeComboBox::QgsBlendModeComboBox( QWidget *parent ) : QComboBox( parent )
{
  updateModes();
}

QStringList QgsBlendModeComboBox::blendModesList() const
{
  return QStringList() << tr( "Normal" )
         << QStringLiteral( "-" )
         << tr( "Lighten" )
         << tr( "Screen" )
         << tr( "Dodge" )
         << tr( "Addition" )
         << QStringLiteral( "-" )
         << tr( "Darken" )
         << tr( "Multiply" )
         << tr( "Burn" )
         << QStringLiteral( "-" )
         << tr( "Overlay" )
         << tr( "Soft light" )
         << tr( "Hard light" )
         << QStringLiteral( "-" )
         << tr( "Difference" )
         << tr( "Subtract" );
}

void QgsBlendModeComboBox::updateModes()
{
  blockSignals( true );
  clear();

  QStringList myBlendModesList = blendModesList();
  QStringList::const_iterator blendModeIt = myBlendModesList.constBegin();

  mBlendModeToListIndex.resize( myBlendModesList.count() );
  mListIndexToBlendMode.resize( myBlendModesList.count() );

  // Loop through blend modes
  int index = 0;
  int blendModeIndex = 0;
  for ( ; blendModeIt != myBlendModesList.constEnd(); ++blendModeIt )
  {
    if ( *blendModeIt == QLatin1String( "-" ) )
    {
      // Add separator
      insertSeparator( index );
    }
    else
    {
      // Not a separator, so store indexes for translation
      // between blend modes and combo box item index
      addItem( *blendModeIt );
      mListIndexToBlendMode[ index ] = blendModeIndex;
      mBlendModeToListIndex[ blendModeIndex ] = index;
      blendModeIndex++;
    }
    index++;
  }

  blockSignals( false );
}

QPainter::CompositionMode QgsBlendModeComboBox::blendMode()
{
  return QgsPainting::getCompositionMode( ( QgsPainting::BlendMode ) mListIndexToBlendMode[ currentIndex()] );
}

void QgsBlendModeComboBox::setBlendMode( QPainter::CompositionMode blendMode )
{
  setCurrentIndex( mBlendModeToListIndex[( int ) QgsPainting::getBlendModeEnum( blendMode )] );
}


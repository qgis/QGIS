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

#include <QAbstractItemView>
#include <QLocale>
#include <QSettings>
#include <QLineEdit>

QgsBlendModeComboBox::QgsBlendModeComboBox( QWidget* parent ) : QComboBox( parent )
{
  updateModes();
}

QgsBlendModeComboBox::~QgsBlendModeComboBox()
{
}

/* Returns a QStringList of the translated blend modes
* "-" is used to indicate the position of a seperator in the list
* This list is designed to emulate GIMP's layer modes, where
* blending modes are grouped by their effect (lightening, darkening, etc)
*/
QStringList QgsBlendModeComboBox::blendModesList() const
{
  return QStringList() << tr( "Normal" )
         << "-"
         << tr( "Lighten" )
         << tr( "Screen" )
         << tr( "Dodge" )
         << tr( "Addition" )
         << "-"
         << tr( "Darken" )
         << tr( "Multiply" )
         << tr( "Burn" )
         << "-"
         << tr( "Overlay" )
         << tr( "Soft light" )
         << tr( "Hard light" )
         << "-"
         << tr( "Difference" )
         << tr( "Subtract" );
}

/* Populates the blend mode combo box, and sets up mapping for
* blend modes to combo box indexes
*/
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
    if ( *blendModeIt == "-" )
    {
      // Add seperator
      insertSeparator( index );
    }
    else
    {
      // Not a seperator, so store indexes for translation
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

//! Function to read the selected blend mode as int
int QgsBlendModeComboBox::blendMode()
{
  return mListIndexToBlendMode[ currentIndex()];
}

//! Function to set the selected blend mode from int
void QgsBlendModeComboBox::setBlendMode( int blendMode )
{
  setCurrentIndex( mBlendModeToListIndex[ blendMode ] );
}


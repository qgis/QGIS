/***************************************************************************
                              qgsscalecombobox.h
                              ------------------------
  begin                : January 7, 2012
  copyright            : (C) 2012 by Alexander Bruy
  email                : alexander dot bruy at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsscalecombobox.h"

#include <QAbstractItemView>

QgsScaleComboBox::QgsScaleComboBox( QWidget* parent ) : QComboBox( parent )
{
  // make combobox editable and populate with predefined scales
  setEditable( true );
  addItem( "1:1000000" );
  addItem( "1:500000" );
  addItem( "1:250000" );
  addItem( "1:100000" );
  addItem( "1:50000" );
  addItem( "1:25000" );
  addItem( "1:10000" );
  addItem( "1:5000" );
  addItem( "1:2500" );
  addItem( "1:1000" );
  addItem( "1:500" );

  setInsertPolicy( QComboBox::NoInsert );
}

QgsScaleComboBox::~QgsScaleComboBox()
{
}

void QgsScaleComboBox::showPopup()
{
  QComboBox::showPopup();

  QStringList parts = currentText().split( ':' );
  bool ok;
  int idx = 0;
  int min = 999999;
  long currScale = parts.at( 1 ).toLong( &ok );
  long nextScale, delta;
  for ( int i = 0; i < count(); i++ )
  {
    parts = itemText( i ).split( ':' );
    nextScale = parts.at( 1 ).toLong( &ok );
    delta = qAbs( currScale - nextScale );
    if( delta < min )
    {
      min = delta;
      idx = i;
    }
  }

  blockSignals( true );
  view()->setCurrentIndex( model()->index( idx, 0 ) );
  blockSignals( false );
}

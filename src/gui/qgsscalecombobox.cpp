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

#include "qgis.h"
#include "qgsscalecombobox.h"

#include <QAbstractItemView>
#include <QSettings>

QgsScaleComboBox::QgsScaleComboBox( QWidget* parent ) : QComboBox( parent )
{
  updateScales();

  setEditable( true );
  setInsertPolicy( QComboBox::NoInsert );
  setCompleter( 0 );
}

QgsScaleComboBox::~QgsScaleComboBox()
{
}

void QgsScaleComboBox::updateScales( const QStringList &scales )
{
  QStringList myScalesList;
  QString oldScale = currentText();

  if ( scales.isEmpty() )
  {
    QSettings settings;
    QString myScales = settings.value( "Map/scales", PROJECT_SCALES ).toString();
    if ( !myScales.isEmpty() )
    {
      myScalesList = myScales.split( "," );
      //~ QStringList::const_iterator scaleIt = myScalesList.constBegin();
      //~ for ( ; scaleIt != myScalesList.constEnd(); ++scaleIt )
      //~ {
        //~ addItem( *scaleIt );
      //~ }
    }
  }
  else
  {
  }

  blockSignals( true );
  clear();
  addItems( myScalesList );
  setEditText( oldScale );
  blockSignals( false );
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
    if ( delta < min )
    {
      min = delta;
      idx = i;
    }
  }

  blockSignals( true );
  view()->setCurrentIndex( model()->index( idx, 0 ) );
  blockSignals( false );
}

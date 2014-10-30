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
#include "qgslogger.h"
#include "qgsscalecombobox.h"

#include <QAbstractItemView>
#include <QLocale>
#include <QSettings>
#include <QLineEdit>

QgsScaleComboBox::QgsScaleComboBox( QWidget* parent ) : QComboBox( parent ), mScale( 1.0 )
{
  updateScales();

  setEditable( true );
  setInsertPolicy( QComboBox::NoInsert );
  setCompleter( 0 );
  connect( this, SIGNAL( activated( const QString & ) ), this, SLOT( fixupScale() ) );
  connect( lineEdit(), SIGNAL( editingFinished() ), this, SLOT( fixupScale() ) );
  fixupScale();
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
    }
  }
  else
  {
    QStringList::const_iterator scaleIt = scales.constBegin();
    for ( ; scaleIt != scales.constEnd(); ++scaleIt )
    {
      myScalesList.append( *scaleIt );
    }
  }

  QStringList parts;
  double denominator;
  bool ok;
  for ( int i = 0; i < myScalesList.size(); ++i )
  {
    parts = myScalesList[ i ] .split( ':' );
    denominator = QLocale::system().toDouble( parts[1], &ok );
    if ( ok )
    {
      myScalesList[ i ] = toString( 1.0 / denominator );
    }
  }

  blockSignals( true );
  clear();
  addItems( myScalesList );
  setScaleString( oldScale );
  blockSignals( false );
}

void QgsScaleComboBox::showPopup()
{
  QComboBox::showPopup();

  if ( !currentText().contains( ':' ) )
  {
    return;
  }
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

//! Function to read the selected scale as text
QString QgsScaleComboBox::scaleString()
{
  return toString( mScale );
}

//! Function to set the selected scale from text
bool QgsScaleComboBox::setScaleString( QString scaleTxt )
{
  bool ok;
  double newScale = toDouble( scaleTxt, &ok );
  if ( ! ok )
  {
    return false;
  }
  else
  {
    mScale = newScale;
    setEditText( toString( mScale ) );
    clearFocus();
    return true;
  }
}

//! Function to read the selected scale as double
double QgsScaleComboBox::scale()
{
  return mScale;
}

//! Function to set the selected scale from double
void QgsScaleComboBox::setScale( double scale )
{
  setScaleString( toString( scale ) );
}

//! Slot called when QComboBox has changed
void QgsScaleComboBox::fixupScale()
{
  double newScale;
  double oldScale = mScale;
  bool ok, userSetScale;
  QStringList txtList = currentText().split( ':' );
  userSetScale = txtList.size() != 2;

  // QgsDebugMsg( QString( "entered with oldScale: %1" ).arg( oldScale ) );
  newScale = toDouble( currentText(), &ok );

  // Valid string representation
  if ( ok && ( newScale != oldScale ) )
  {
    // if a user types scale = 2345, we transform to 1:2345
    if ( userSetScale && newScale >= 1.0 )
    {
      mScale = 1 / newScale;
    }
    else
    {
      mScale = newScale;
    }
    setScale( mScale );
    emit scaleChanged();
  }
  else
  {
    // Invalid string representation or same scale
    // Reset to the old
    setScale( mScale );
  }
}

QString QgsScaleComboBox::toString( double scale )
{
  if ( scale == 0 )
  {
    return "0";
  }
  else if ( scale > 1 )
  {
    return QString( "%1:1" ).arg( QLocale::system().toString( qRound( scale ) ) );
  }
  else
  {
    return QString( "1:%1" ).arg( QLocale::system().toString( qRound( 1.0 / scale ) ) );
  }
}

double QgsScaleComboBox::toDouble( QString scaleString, bool * returnOk )
{
  bool ok = false;
  QString scaleTxt( scaleString );

  double scale = QLocale::system().toDouble( scaleTxt, &ok );
  if ( ok )
  {
    // Create a text version and set that text and rescan
    // Idea is to get the same rounding.
    scaleTxt = toString( scale );
  }
  // It is now either X:Y or not valid
  ok = false;
  QStringList txtList = scaleTxt.split( ':' );
  if ( 2 == txtList.size() )
  {
    bool okX = false;
    bool okY = false;
    int x = QLocale::system().toInt( txtList[ 0 ], &okX );
    int y = QLocale::system().toInt( txtList[ 1 ], &okY );
    if ( okX && okY )
    {
      // Scale is fraction of x and y
      scale = ( double )x / ( double )y;
      ok = true;
    }
  }

  // Set up optional return flag
  if ( returnOk )
  {
    *returnOk = ok;
  }
  return scale;
}



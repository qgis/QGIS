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

QgsScaleComboBox::QgsScaleComboBox( QWidget* parent )
    : QComboBox( parent )
    , mScale( 1.0 )
    , mMinScale( 0.0 )
{
  updateScales();

  setEditable( true );
  setInsertPolicy( QComboBox::NoInsert );
  setCompleter( nullptr );
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
      myScalesList = myScales.split( ',' );
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
  view()->setMinimumWidth( view()->sizeHintForColumn( 0 ) );
}

//! Function to read the selected scale as text
QString QgsScaleComboBox::scaleString()
{
  return toString( mScale );
}

//! Function to set the selected scale from text
bool QgsScaleComboBox::setScaleString( const QString& scaleTxt )
{
  bool ok;
  double newScale = toDouble( scaleTxt, &ok );
  double oldScale = mScale;
  if ( newScale < mMinScale )
  {
    newScale = mMinScale;
  }
  if ( ! ok )
  {
    return false;
  }
  else
  {
    mScale = newScale;
    setEditText( toString( mScale ) );
    clearFocus();
    if ( mScale != oldScale )
    {
      emit scaleChanged( mScale );
    }
    return true;
  }
}

//! Function to read the selected scale as double
double QgsScaleComboBox::scale() const
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
  QStringList txtList = currentText().split( ':' );
  bool userSetScale = txtList.size() != 2;

  bool ok;
  double newScale = toDouble( currentText(), &ok );

  // Valid string representation
  if ( ok )
  {
    // if a user types scale = 2345, we transform to 1:2345
    if ( userSetScale && newScale >= 1.0 )
    {
      newScale = 1 / newScale;
    }
    setScale( newScale );
  }
  else
  {
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

double QgsScaleComboBox::toDouble( const QString& scaleString, bool * returnOk )
{
  bool ok = false;
  QString scaleTxt( scaleString );

  double scale = QGis::permissiveToDouble( scaleTxt, ok );
  if ( ok )
  {
    // Create a text version and set that text and rescan
    // Idea is to get the same rounding.
    scaleTxt = toString( scale );
  }
  else
  {
    // It is now either X:Y or not valid
    QStringList txtList = scaleTxt.split( ':' );
    if ( 2 == txtList.size() )
    {
      bool okX = false;
      bool okY = false;
      int x = QGis::permissiveToInt( txtList[ 0 ], okX );
      int y = QGis::permissiveToInt( txtList[ 1 ], okY );
      if ( okX && okY )
      {
        // Scale is fraction of x and y
        scale = ( double )x / ( double )y;
        ok = true;
      }
    }
  }

  // Set up optional return flag
  if ( returnOk )
  {
    *returnOk = ok;
  }
  return scale;
}

void QgsScaleComboBox::setMinScale( double scale )
{
  mMinScale = scale;
  if ( mScale < scale )
  {
    setScale( scale );
  }
}

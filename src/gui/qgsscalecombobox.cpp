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
#include "qgssettings.h"

#include <QAbstractItemView>
#include <QLocale>
#include <QLineEdit>

QgsScaleComboBox::QgsScaleComboBox( QWidget *parent )
  : QComboBox( parent )
{
  updateScales();

  setEditable( true );
  setInsertPolicy( QComboBox::NoInsert );
  setCompleter( nullptr );
  connect( this, static_cast<void ( QComboBox::* )( const QString & )>( &QComboBox::activated ), this, &QgsScaleComboBox::fixupScale );
  connect( lineEdit(), &QLineEdit::editingFinished, this, &QgsScaleComboBox::fixupScale );
  fixupScale();
}

void QgsScaleComboBox::updateScales( const QStringList &scales )
{
  QStringList myScalesList;
  QString oldScale = currentText();

  if ( scales.isEmpty() )
  {
    QgsSettings settings;
    QString myScales = settings.value( QStringLiteral( "Map/scales" ), PROJECT_SCALES ).toString();
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
      myScalesList[ i ] = toString( denominator );
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
    delta = std::labs( currScale - nextScale );
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

QString QgsScaleComboBox::scaleString() const
{
  return toString( mScale );
}

bool QgsScaleComboBox::setScaleString( const QString &string )
{
  bool ok;
  double newScale = toDouble( string, &ok );
  double oldScale = mScale;
  if ( newScale > mMinScale && newScale != 0 && mMinScale != 0 )
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

double QgsScaleComboBox::scale() const
{
  return mScale;
}

void QgsScaleComboBox::setScale( double scale )
{
  setScaleString( toString( scale ) );
}

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
    if ( userSetScale && newScale < 1.0 && !qgsDoubleNear( newScale, 0.0 ) )
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
    return QStringLiteral( "0" );
  }
  else if ( scale <= 1 )
  {
    return QStringLiteral( "%1:1" ).arg( QLocale::system().toString( static_cast< int >( std::round( 1.0 / scale ) ) ) );
  }
  else
  {
    return QStringLiteral( "1:%1" ).arg( QLocale::system().toString( static_cast< int >( std::round( scale ) ) ) );
  }
}

double QgsScaleComboBox::toDouble( const QString &scaleString, bool *returnOk )
{
  bool ok = false;
  QString scaleTxt( scaleString );

  double denominator = qgsPermissiveToDouble( scaleTxt, ok );
  double scale = !qgsDoubleNear( denominator, 0.0 ) ? 1.0 / denominator : 0.0;
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
      int x = qgsPermissiveToInt( txtList[ 0 ], okX );
      int y = qgsPermissiveToInt( txtList[ 1 ], okY );
      if ( okX && okY && x != 0 )
      {
        // Scale is fraction of x and y
        scale = static_cast<  double >( y ) / static_cast< double >( x );
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
  if ( mScale > mMinScale && mScale != 0 && mMinScale != 0 )
  {
    setScale( mMinScale );
  }
}

/***************************************************************************
    qgsalignmentcombobox.h
    ---------------------
    begin                : June 2019
    copyright            : (C) 2019 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsalignmentcombobox.h"

#include "qgsapplication.h"
#include "qgsguiutils.h"

QgsAlignmentComboBox::QgsAlignmentComboBox( QWidget *parent )
  : QComboBox( parent )
{
  populate();
  setCurrentIndex( 0 );
  connect( this, static_cast<void ( QComboBox::* )( int )>( &QComboBox::currentIndexChanged ), this, [ = ]
  {
    if ( !mBlockChanged )
      emit changed();
  } );
}

void QgsAlignmentComboBox::setAvailableAlignments( Qt::Alignment alignments )
{
  mAlignments = alignments;
  populate();
}

Qt::Alignment QgsAlignmentComboBox::currentAlignment() const
{
  return static_cast< Qt::Alignment >( currentData().toInt() );
}

void QgsAlignmentComboBox::setCurrentAlignment( Qt::Alignment alignment )
{
  const int index = findData( QVariant( alignment ) );
  if ( index >= 0 )
    setCurrentIndex( index );
}

void QgsAlignmentComboBox::customizeAlignmentDisplay( Qt::Alignment alignment, const QString &text, const QIcon &icon )
{
  const int index = findData( QVariant( alignment ) );
  if ( index >= 0 )
  {
    if ( !text.isEmpty() )
      setItemText( index, text );
    if ( !icon.isNull() )
      setItemIcon( index, icon );
  }
}

void QgsAlignmentComboBox::populate()
{
  const Qt::Alignment prevAlign = currentAlignment();

  mBlockChanged = true;
  clear();

  if ( mAlignments & Qt::AlignLeft )
    addItem( QgsApplication::getThemeIcon( QStringLiteral( "/mIconAlignLeft.svg" ) ), tr( "Left" ), Qt::AlignLeft );
  if ( mAlignments & Qt::AlignHCenter )
    addItem( QgsApplication::getThemeIcon( QStringLiteral( "/mIconAlignCenter.svg" ) ), tr( "Center" ), Qt::AlignHCenter );
  if ( mAlignments & Qt::AlignRight )
    addItem( QgsApplication::getThemeIcon( QStringLiteral( "/mIconAlignRight.svg" ) ), tr( "Right" ), Qt::AlignRight );
  if ( mAlignments & Qt::AlignJustify )
    addItem( QgsApplication::getThemeIcon( QStringLiteral( "/mIconAlignJustify.svg" ) ), tr( "Justify" ), Qt::AlignJustify );

  if ( mAlignments & Qt::AlignTop )
    addItem( QgsApplication::getThemeIcon( QStringLiteral( "/mIconAlignTop.svg" ) ), tr( "Top" ), Qt::AlignTop );
  if ( mAlignments & Qt::AlignVCenter )
    addItem( QgsApplication::getThemeIcon( QStringLiteral( "/mIconAlignVCenter.svg" ) ), tr( "Vertical Center" ), Qt::AlignVCenter );
  if ( mAlignments & Qt::AlignBottom )
    addItem( QgsApplication::getThemeIcon( QStringLiteral( "/mIconAlignBottom.svg" ) ), tr( "Bottom" ), Qt::AlignBottom );

  const int index = findData( QVariant( prevAlign ) );
  if ( index >= 0 )
    setCurrentIndex( index );

  mBlockChanged = false;
  if ( currentAlignment() != prevAlign )
    emit changed();
}


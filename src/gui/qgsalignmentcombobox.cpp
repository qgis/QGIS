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

#include "moc_qgsalignmentcombobox.cpp"

QgsAlignmentComboBox::QgsAlignmentComboBox( QWidget *parent )
  : QComboBox( parent )
{
  populate();
  setCurrentIndex( 0 );
  connect( this, static_cast<void ( QComboBox::* )( int )>( &QComboBox::currentIndexChanged ), this, [this] {
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
  return static_cast<Qt::Alignment>( currentData().toInt() );
}

void QgsAlignmentComboBox::setCurrentAlignment( Qt::Alignment alignment )
{
  const int index = findData( QVariant( alignment ) );
  if ( index >= 0 )
    setCurrentIndex( index );
}

Qgis::TextHorizontalAlignment QgsAlignmentComboBox::horizontalAlignment() const
{
  switch ( currentAlignment() )
  {
    case Qt::AlignLeft:
      return Qgis::TextHorizontalAlignment::Left;
    case Qt::AlignRight:
      return Qgis::TextHorizontalAlignment::Right;

    case Qt::AlignHCenter:
      return Qgis::TextHorizontalAlignment::Center;
    case Qt::AlignJustify:
      return Qgis::TextHorizontalAlignment::Justify;
    case Qt::AlignAbsolute:
      return Qgis::TextHorizontalAlignment::Left;

    default:
      break;
  }

  return Qgis::TextHorizontalAlignment::Left;
}

Qgis::TextVerticalAlignment QgsAlignmentComboBox::verticalAlignment() const
{
  switch ( currentAlignment() )
  {
    case Qt::AlignTop:
      return Qgis::TextVerticalAlignment::Top;
    case Qt::AlignBottom:
      return Qgis::TextVerticalAlignment::Bottom;
    case Qt::AlignVCenter:
      return Qgis::TextVerticalAlignment::VerticalCenter;
    case Qt::AlignBaseline:
      return Qgis::TextVerticalAlignment::Bottom; // not yet supported

    default:
      break;
  }

  return Qgis::TextVerticalAlignment::Bottom;
}

void QgsAlignmentComboBox::setCurrentAlignment( Qgis::TextHorizontalAlignment alignment )
{
  switch ( alignment )
  {
    case Qgis::TextHorizontalAlignment::Left:
      setCurrentAlignment( Qt::AlignmentFlag::AlignLeft );
      break;
    case Qgis::TextHorizontalAlignment::Center:
      setCurrentAlignment( Qt::AlignmentFlag::AlignHCenter );
      break;
    case Qgis::TextHorizontalAlignment::Right:
      setCurrentAlignment( Qt::AlignmentFlag::AlignRight );
      break;
    case Qgis::TextHorizontalAlignment::Justify:
      setCurrentAlignment( Qt::AlignmentFlag::AlignJustify );
      break;
  }
}

void QgsAlignmentComboBox::setCurrentAlignment( Qgis::TextVerticalAlignment alignment )
{
  switch ( alignment )
  {
    case Qgis::TextVerticalAlignment::Top:
      setCurrentAlignment( Qt::AlignmentFlag::AlignTop );
      break;
    case Qgis::TextVerticalAlignment::VerticalCenter:
      setCurrentAlignment( Qt::AlignmentFlag::AlignVCenter );
      break;
    case Qgis::TextVerticalAlignment::Bottom:
      setCurrentAlignment( Qt::AlignmentFlag::AlignBottom );
      break;
  }
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
    addItem( QgsApplication::getThemeIcon( u"/mIconAlignLeft.svg"_s ), tr( "Left" ), Qt::AlignLeft );
  if ( mAlignments & Qt::AlignHCenter )
    addItem( QgsApplication::getThemeIcon( u"/mIconAlignCenter.svg"_s ), tr( "Center" ), Qt::AlignHCenter );
  if ( mAlignments & Qt::AlignRight )
    addItem( QgsApplication::getThemeIcon( u"/mIconAlignRight.svg"_s ), tr( "Right" ), Qt::AlignRight );
  if ( mAlignments & Qt::AlignJustify )
    addItem( QgsApplication::getThemeIcon( u"/mIconAlignJustify.svg"_s ), tr( "Justify" ), Qt::AlignJustify );

  if ( mAlignments & Qt::AlignTop )
    addItem( QgsApplication::getThemeIcon( u"/mIconAlignTop.svg"_s ), tr( "Top" ), Qt::AlignTop );
  if ( mAlignments & Qt::AlignVCenter )
    addItem( QgsApplication::getThemeIcon( u"/mIconAlignVCenter.svg"_s ), tr( "Vertical Center" ), Qt::AlignVCenter );
  if ( mAlignments & Qt::AlignBottom )
    addItem( QgsApplication::getThemeIcon( u"/mIconAlignBottom.svg"_s ), tr( "Bottom" ), Qt::AlignBottom );

  const int index = findData( QVariant( prevAlign ) );
  if ( index >= 0 )
    setCurrentIndex( index );

  mBlockChanged = false;
  if ( currentAlignment() != prevAlign )
    emit changed();
}

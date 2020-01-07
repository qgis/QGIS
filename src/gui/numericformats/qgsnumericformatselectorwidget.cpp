/***************************************************************************
    qgsnumericformatselectorwidget.cpp
    ----------------------------------
    begin                : January 2020
    copyright            : (C) 2020 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsnumericformatselectorwidget.h"
#include "qgsapplication.h"
#include "qgsnumericformatregistry.h"
#include "qgsnumericformat.h"
#include "qgis.h"
#include <mutex>


static void _initWidgetFunctions()
{
  static std::once_flag initialized;
  std::call_once( initialized, [ = ]
  {

  } );
}

QgsNumericFormatSelectorWidget::QgsNumericFormatSelectorWidget( QWidget *parent )
  : QgsPanelWidget( parent )
{
  setupUi( this );
  _initWidgetFunctions();

  populateTypes();
  mCategoryCombo->setCurrentIndex( 0 );
}

void QgsNumericFormatSelectorWidget::setFormat( const QgsNumericFormat *format )
{
  if ( !format )
    return;

  const QString id = format->id();
  const int index = mCategoryCombo->findData( id );
  const QString prevId = mCategoryCombo->currentData().toString();
  if ( index < 0 )
    mCategoryCombo->setCurrentIndex( mCategoryCombo->findData( QStringLiteral( "fallback" ) ) );
  else
    mCategoryCombo->setCurrentIndex( index );

  if ( prevId != id )
    emit changed();
}

QgsNumericFormat *QgsNumericFormatSelectorWidget::format() const
{
  return QgsApplication::numericFormatRegistry()->format( mCategoryCombo->currentData().toString() );
}

void QgsNumericFormatSelectorWidget::populateTypes()
{
  QStringList ids = QgsApplication::numericFormatRegistry()->formats();

  std::sort( ids.begin(), ids.end(), [ = ]( const QString & a, const QString & b )->bool
  {
    if ( QgsApplication::numericFormatRegistry()->sortKey( a ) < QgsApplication::numericFormatRegistry()->sortKey( b ) )
      return true;
    else if ( QgsApplication::numericFormatRegistry()->sortKey( a ) > QgsApplication::numericFormatRegistry()->sortKey( b ) )
      return false;
    else
    {
      int res = QString::localeAwareCompare( QgsApplication::numericFormatRegistry()->visibleName( a ), QgsApplication::numericFormatRegistry()->visibleName( b ) );
      if ( res < 0 )
        return true;
      else if ( res > 0 )
        return false;
    }
    return false;
  } );

  for ( const QString &id : qgis::as_const( ids ) )
    mCategoryCombo->addItem( QgsApplication::numericFormatRegistry()->visibleName( id ), id );
}

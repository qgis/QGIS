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

static void _initWidgetFunctions()
{
  static bool sInitialized = false;
  if ( sInitialized )
    return;

  sInitialized = true;
}


QgsNumericFormatSelectorWidget::QgsNumericFormatSelectorWidget( QWidget *parent )
  : QgsPanelWidget( parent )
{
  setupUi( this );
  _initWidgetFunctions();

  populateTypes();
}

void QgsNumericFormatSelectorWidget::setFormat( const QgsNumericFormat *format )
{
  if ( !format )
    return;

  const QString id = format->id();
  const int index = mCategoryCombo->findData( id );
  if ( index < 0 )
    mCategoryCombo->setCurrentIndex( mCategoryCombo->findData( QStringLiteral( "fallback" ) ) );
  else
    mCategoryCombo->setCurrentIndex( index );
}

QgsNumericFormat *QgsNumericFormatSelectorWidget::format() const
{
  return QgsApplication::numericFormatRegistry()->format( mCategoryCombo->currentData().toString() );
}

void QgsNumericFormatSelectorWidget::populateTypes()
{
  const QStringList ids = QgsApplication::numericFormatRegistry()->formats();

  for ( const QString &id : ids )
    mCategoryCombo->addItem( QgsApplication::numericFormatRegistry()->visibleName( id ), id );
}

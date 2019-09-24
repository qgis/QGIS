/***************************************************************************
  qgscheckboxfieldformatter.cpp - QgsCheckBoxFieldFormatter

 ---------------------
 begin                : 23.09.2019
 copyright            : (C) 2019 by Denis Rouzaud
 email                : denis@opengis.ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <QObject>

#include "qgscheckboxfieldformatter.h"
#include "qgsapplication.h"
#include "qgsvectorlayer.h"


QString QgsCheckBoxFieldFormatter::id() const
{
  return QStringLiteral( "CheckBox" );
}

QString QgsCheckBoxFieldFormatter::representValue( QgsVectorLayer *layer, int fieldIndex, const QVariantMap &config, const QVariant &cache, const QVariant &value ) const
{
  Q_UNUSED( cache )


  enum BoolValue {_FALSE, _TRUE, _NULL};
  BoolValue boolValue = _NULL;

  const QVariant::Type fieldType = layer->fields().at( fieldIndex ).type();
  if ( fieldType == QVariant::Bool )
  {
    if ( value.toBool() )
      boolValue = _TRUE;
    else
      boolValue = _FALSE;
  }
  else
  {
    if ( config.contains( QStringLiteral( "CheckedState" ) ) && value == config[ QStringLiteral( "CheckedState" ) ] )
      boolValue = _TRUE;
    else if ( config.contains( QStringLiteral( "UncheckedState" ) ) && value == config[ QStringLiteral( "UncheckedState" ) ] )
      boolValue = _FALSE;
    else
      boolValue = _NULL;
  }

  switch ( boolValue )
  {
    case _NULL:
      return QgsApplication::nullRepresentation();
    case _TRUE:
      return QObject::tr( "true" );
    case _FALSE:
      return QObject::tr( "false" );
  }
}

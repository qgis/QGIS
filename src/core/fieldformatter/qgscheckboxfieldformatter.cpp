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
#include "qgsvectorlayer.h"
#include "qgsapplication.h"


QString QgsCheckBoxFieldFormatter::id() const
{
  return QStringLiteral( "CheckBox" );
}

QString QgsCheckBoxFieldFormatter::representValue( QgsVectorLayer *layer, int fieldIndex, const QVariantMap &config, const QVariant &cache, const QVariant &value ) const
{
  Q_UNUSED( cache )

  /*
  This follows this logic:

  if field type is bool:
    NULL => nullRepresentation
    true => tr("true")
    false => tr("false")
  else
    if cannot convert to string (like json integer list) => (invalid)
    if == checkedstate => tr("true")
    if == uncheckedstate => tr("false")
    else (value.toString)
  */

  bool isNull = value.isNull();
  bool boolValue = false;
  QString textValue = QgsApplication::nullRepresentation();

  const QVariant::Type fieldType = layer->fields().at( fieldIndex ).type();
  if ( fieldType == QVariant::Bool )
  {
    boolValue = value.toBool();
  }
  else
  {
    if ( !value.canConvert<QString>() )
    {
      isNull = true;
      textValue = QObject::tr( "(invalid)" );
    }
    else
    {
      if ( config.contains( QStringLiteral( "CheckedState" ) ) && value.toString() == config[ QStringLiteral( "CheckedState" ) ].toString() )
      {
        boolValue = true;
      }
      else if ( config.contains( QStringLiteral( "UncheckedState" ) ) && value.toString() == config[ QStringLiteral( "UncheckedState" ) ].toString() )
      {
        boolValue = false;
      }
      else
      {
        isNull = true;
        textValue = QStringLiteral( "(%1)" ).arg( value.toString() );
      }
    }
  }

  if ( isNull )
  {
    return textValue;
  }
  if ( boolValue )
    return QObject::tr( "true" );
  else
    return QObject::tr( "false" );
}

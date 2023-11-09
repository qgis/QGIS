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
#include "qgsvariantutils.h"

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

  bool isNull = QgsVariantUtils::isNull( value );
  bool boolValue = false;
  QString textValue = QgsApplication::nullRepresentation();

  const QVariant::Type fieldType = layer->fields().at( fieldIndex ).type();
  if ( fieldType == QVariant::Bool )
  {
    boolValue = value.toBool();
    textValue = boolValue ? QObject::tr( "true" ) : QObject::tr( "false" );
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
      textValue = value.toString();
      if ( config.contains( QStringLiteral( "CheckedState" ) ) && textValue == config[ QStringLiteral( "CheckedState" ) ].toString() )
      {
        boolValue = true;
      }
      else if ( config.contains( QStringLiteral( "UncheckedState" ) ) && textValue == config[ QStringLiteral( "UncheckedState" ) ].toString() )
      {
        boolValue = false;
      }
      else
      {
        isNull = true;
        textValue = QStringLiteral( "(%1)" ).arg( textValue );
      }
    }
  }

  if ( isNull )
  {
    return textValue;
  }

  const TextDisplayMethod displayMethod = static_cast< TextDisplayMethod >( config.value( QStringLiteral( "TextDisplayMethod" ), QStringLiteral( "0" ) ).toInt() );
  switch ( displayMethod )
  {
    case QgsCheckBoxFieldFormatter::ShowTrueFalse:
      if ( boolValue )
        return QObject::tr( "true" );
      else
        return QObject::tr( "false" );

    case QgsCheckBoxFieldFormatter::ShowStoredValues:
      return textValue;
  }
  return QString();
}

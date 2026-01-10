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

#include "qgscheckboxfieldformatter.h"

#include "qgsapplication.h"
#include "qgsvariantutils.h"
#include "qgsvectorlayer.h"

#include <QObject>

QString QgsCheckBoxFieldFormatter::id() const
{
  return u"CheckBox"_s;
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

  const QMetaType::Type fieldType = layer->fields().at( fieldIndex ).type();
  if ( fieldType == QMetaType::Type::Bool )
  {
    if ( ! isNull )
    {
      boolValue = value.toBool();
      textValue = boolValue ? QObject::tr( "true" ) : QObject::tr( "false" );
    }
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
      if ( config.contains( u"CheckedState"_s ) && textValue == config[ u"CheckedState"_s ].toString() )
      {
        boolValue = true;
      }
      else if ( config.contains( u"UncheckedState"_s ) && textValue == config[ u"UncheckedState"_s ].toString() )
      {
        boolValue = false;
      }
      else
      {
        isNull = true;
        textValue = u"(%1)"_s.arg( textValue );
      }
    }
  }

  if ( isNull )
  {
    return textValue;
  }

  const TextDisplayMethod displayMethod = static_cast< TextDisplayMethod >( config.value( u"TextDisplayMethod"_s, u"0"_s ).toInt() );
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

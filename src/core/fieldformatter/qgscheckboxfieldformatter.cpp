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


QString QgsCheckBoxFieldFormatter::id() const
{
  return QStringLiteral( "CheckBox" );
}

QString QgsCheckBoxFieldFormatter::representValue( QgsVectorLayer *layer, int fieldIndex, const QVariantMap &config, const QVariant &cache, const QVariant &value ) const
{
  Q_UNUSED( layer )
  Q_UNUSED( fieldIndex )
  Q_UNUSED( cache )
  Q_UNUSED( config )


  if ( value.isNull() || !value.canConvert<bool>() )
  {
    return QgsApplication::nullRepresentation();
  }

  const bool boolValue = value.toBool();

  if ( boolValue )
    return QObject::tr( "true" );
  else
    return QObject::tr( "false" );
}

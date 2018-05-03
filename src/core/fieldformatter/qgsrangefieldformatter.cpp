/***************************************************************************
  qgsrangefieldformatter.cpp - QgsRangeFieldFormatter

 ---------------------
 begin                : 01/02/2018
 copyright            : (C) 2018 by Alessandro Pasotti
 email                : elpaso at itopen dot it
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <QLocale>

#include "qgsrangefieldformatter.h"

#include "qgssettings.h"
#include "qgsfield.h"
#include "qgsvectorlayer.h"


QString QgsRangeFieldFormatter::id() const
{
  return QStringLiteral( "Range" );
}

QString QgsRangeFieldFormatter::representValue( QgsVectorLayer *layer, int fieldIndex, const QVariantMap &config, const QVariant &cache, const QVariant &value ) const
{
  Q_UNUSED( cache )
  Q_UNUSED( config )

  if ( value.isNull() )
  {
    return QgsApplication::nullRepresentation();
  }

  QString result;

  const QgsField field = layer->fields().at( fieldIndex );

  if ( field.type() == QVariant::Double &&
       config.contains( QStringLiteral( "Precision" ) ) &&
       value.isValid( ) )
  {
    bool ok;
    double val( value.toDouble( &ok ) );
    if ( ok )
    {
      int precision( config[ QStringLiteral( "Precision" ) ].toInt( &ok ) );
      if ( ok )
      {
        // TODO: make the format configurable!
        result = QLocale().toString( val, 'f', precision );
      }
    }
  }
  else if ( ( field.type() == QVariant::Int ) &&
            value.isValid( ) )
  {
    bool ok;
    double val( value.toInt( &ok ) );
    if ( ok )
    {
      result =  QLocale().toString( val, 'f', 0 );
    }
  }
  else if ( ( field.type() == QVariant::LongLong ) &&
            value.isValid( ) )
  {
    bool ok;
    double val( value.toLongLong( &ok ) );
    if ( ok )
    {
      result =  QLocale().toString( val, 'f', 0 );
    }
  }
  else
  {
    result = value.toString();
  }
  return result;
}

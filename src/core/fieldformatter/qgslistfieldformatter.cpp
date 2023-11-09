/***************************************************************************
  qgslistfieldformatter.cpp - QgsListFieldFormatter

 ---------------------
 begin                : 3.12.2016
 copyright            : (C) 2016 by Matthias Kuhn
 email                : matthias@opengis.ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "qgslistfieldformatter.h"
#include "qgsapplication.h"
#include "qgsvariantutils.h"
#include <QSettings>

QString QgsListFieldFormatter::id() const
{
  return QStringLiteral( "List" );
}

QString QgsListFieldFormatter::representValue( QgsVectorLayer *layer, int fieldIndex, const QVariantMap &config, const QVariant &cache, const QVariant &value ) const
{
  Q_UNUSED( layer )
  Q_UNUSED( fieldIndex )
  Q_UNUSED( config )
  Q_UNUSED( cache )

  if ( QgsVariantUtils::isNull( value ) )
  {
    return QgsApplication::nullRepresentation();
  }

  QString result;
  const QVariantList list = value.toList();
  for ( const QVariant &val : list )
  {
    if ( !result.isEmpty() )
      result.append( ", " );
    result.append( val.toString() );
  }
  return result;
}

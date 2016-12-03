/***************************************************************************
  qgslistfieldkit.cpp - QgsListFieldKit

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
#include "qgslistfieldkit.h"

#include <QSettings>

QgsListFieldKit::QgsListFieldKit()
{

}

QString QgsListFieldKit::representValue( QgsVectorLayer* vl, int fieldIdx, const QVariantMap& config, const QVariant& cache, const QVariant& value ) const
{
  Q_UNUSED( vl );
  Q_UNUSED( fieldIdx );
  Q_UNUSED( config );
  Q_UNUSED( cache );

  if ( value.isNull() )
  {
    QSettings settings;
    return settings.value( QStringLiteral( "qgis/nullValue" ), "NULL" ).toString();
  }

  QString result;
  const QVariantList list = value.toList();
  for ( QVariantList::const_iterator i = list.constBegin(); i != list.constEnd(); ++i )
  {
    if ( !result.isEmpty() ) result.append( ", " );
    result.append( i->toString() );
  }
  return result;
}

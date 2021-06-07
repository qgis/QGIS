/***************************************************************************
  qgslistfieldformatter.cpp - QgsListFieldFormatter

 ---------------------
 begin                : 3.12.2016
 copyright            : (C) 2016 by Matthias Kuhn
 email                : matthias@opengis.ch
 ***************************************************************************
 *                                                                         *
 *   *
 *  
 *        *
 *                                     *
 *                                                                         *
 ***************************************************************************/
#include "qgslistfieldformatter.h"
#include "qgsapplication.h"
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

  if ( value.isNull() )
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

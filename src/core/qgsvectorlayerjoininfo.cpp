/***************************************************************************
                          qgsvectorlayerjoininfo.cpp
                          --------------------------
    begin                : Jun 29, 2017
    copyright            : (C) 2017 by Paul Blottiere
    email                : paul.blottiere@oslandia.com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsvectorlayerjoininfo.h"
#include "qgsvectorlayer.h"

QString QgsVectorLayerJoinInfo::prefixedFieldName( const QgsField &f ) const
{
  QString name;

  if ( joinLayer() )
  {
    if ( prefix().isNull() )
      name = joinLayer()->name() + '_';
    else
      name = prefix();

    name += f.name();
  }

  return name;
}

void QgsVectorLayerJoinInfo::setEditable( bool enabled )
{
  mEditable = enabled;

  if ( ! mEditable )
  {
    setCascadedDelete( false );
    setUpsertOnEdit( false );
  }
}

QgsFeature QgsVectorLayerJoinInfo::extractJoinedFeature( const QgsFeature &feature ) const
{
  QgsFeature joinFeature;

  if ( joinLayer() )
  {
    const QVariant idFieldValue = feature.attribute( targetFieldName() );
    joinFeature.initAttributes( joinLayer()->fields().count() );
    joinFeature.setFields( joinLayer()->fields() );
    joinFeature.setAttribute( joinFieldName(), idFieldValue );

    const QgsFields joinFields = joinFeature.fields();
    for ( const auto &field : joinFields )
    {
      const QString prefixedName = prefixedFieldName( field );

      if ( feature.fieldNameIndex( prefixedName ) != -1 )
        joinFeature.setAttribute( field.name(), feature.attribute( prefixedName ) );
    }
  }

  return joinFeature;
}

QStringList QgsVectorLayerJoinInfo::joinFieldNamesSubset( const QgsVectorLayerJoinInfo &info, bool blacklisted )
{
  QStringList fieldNames;

  if ( blacklisted && !info.joinFieldNamesBlackList().isEmpty() )
  {
    QStringList *lst = info.joinFieldNamesSubset();
    if ( lst )
    {
      for ( const QString &s : qgis::as_const( *lst ) )
      {
        if ( !info.joinFieldNamesBlackList().contains( s ) )
          fieldNames.append( s );
      }
    }
    else
    {
      for ( const QgsField &f : info.joinLayer()->fields() )
      {
        if ( !info.joinFieldNamesBlackList().contains( f.name() )
             && f.name() != info.joinFieldName() )
          fieldNames.append( f.name() );
      }
    }
  }
  else
  {
    QStringList *lst = info.joinFieldNamesSubset();
    if ( lst )
    {
      fieldNames = *lst;
    }
  }

  return fieldNames;
}

bool QgsVectorLayerJoinInfo::hasSubset( bool blacklisted ) const
{
  bool subset = joinFieldNamesSubset();

  if ( blacklisted )
    subset |= !joinFieldNamesBlackList().isEmpty();

  return subset;
}

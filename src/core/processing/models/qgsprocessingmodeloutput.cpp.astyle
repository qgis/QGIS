/***************************************************************************
                         qgsprocessingmodeloutput.cpp
                         ----------------------------
    begin                : June 2017
    copyright            : (C) 2017 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsprocessingmodeloutput.h"

///@cond NOT_STABLE

QgsProcessingModelOutput::QgsProcessingModelOutput( const QString &name, const QString &description )
  : QgsProcessingModelComponent( description )
  , mName( name )
{}

QVariant QgsProcessingModelOutput::toVariant() const
{
  QVariantMap map;
  map.insert( QStringLiteral( "name" ), mName );
  map.insert( QStringLiteral( "child_id" ), mChildId );
  map.insert( QStringLiteral( "output_name" ), mOutputName );
  saveCommonProperties( map );
  return map;
}

bool QgsProcessingModelOutput::loadVariant( const QVariantMap &map )
{
  mName = map.value( QStringLiteral( "name" ) ).toString();
  mChildId = map.value( QStringLiteral( "child_id" ) ).toString();
  mOutputName = map.value( QStringLiteral( "output_name" ) ).toString();
  restoreCommonProperties( map );
  return true;
}


///@endcond

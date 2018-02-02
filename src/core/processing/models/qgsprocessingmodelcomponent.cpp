/***************************************************************************
                         qgsprocessingmodelcomponent.cpp
                         -------------------------------
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

#include "qgsprocessingmodelcomponent.h"

///@cond NOT_STABLE

QgsProcessingModelComponent::QgsProcessingModelComponent( const QString &description )
  : mDescription( description )
{}

QString QgsProcessingModelComponent::description() const
{
  return mDescription;
}

void QgsProcessingModelComponent::setDescription( const QString &description )
{
  mDescription = description;
}

QPointF QgsProcessingModelComponent::position() const
{
  return mPosition;
}

void QgsProcessingModelComponent::setPosition( QPointF position )
{
  mPosition = position;
}

void QgsProcessingModelComponent::saveCommonProperties( QVariantMap &map ) const
{
  map.insert( QStringLiteral( "component_pos_x" ), mPosition.x() );
  map.insert( QStringLiteral( "component_pos_y" ), mPosition.y() );
  map.insert( QStringLiteral( "component_description" ), mDescription );
}

void QgsProcessingModelComponent::restoreCommonProperties( const QVariantMap &map )
{
  QPointF pos;
  pos.setX( map.value( QStringLiteral( "component_pos_x" ) ).toDouble() );
  pos.setY( map.value( QStringLiteral( "component_pos_y" ) ).toDouble() );
  mPosition = pos;
  mDescription = map.value( QStringLiteral( "component_description" ) ).toString();
}

///@endcond

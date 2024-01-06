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
#include "qgsprocessingmodelcomment.h"
#include "qgscolorutils.h"

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

QSizeF QgsProcessingModelComponent::size() const
{
  return mSize;
}

void QgsProcessingModelComponent::setSize( QSizeF size )
{
  mSize = size;
}

QColor QgsProcessingModelComponent::color() const
{
  return mColor;
}

void QgsProcessingModelComponent::setColor( const QColor &color )
{
  mColor = color;
}

bool QgsProcessingModelComponent::linksCollapsed( Qt::Edge edge ) const
{
  switch ( edge )
  {
    case Qt::TopEdge:
      return mTopEdgeLinksCollapsed;

    case Qt::BottomEdge:
      return mBottomEdgeLinksCollapsed;

    case Qt::LeftEdge:
    case Qt::RightEdge:
      return false;
  }
  return false;
}

void QgsProcessingModelComponent::setLinksCollapsed( Qt::Edge edge, bool collapsed )
{
  switch ( edge )
  {
    case Qt::TopEdge:
      mTopEdgeLinksCollapsed = collapsed;
      break;

    case Qt::BottomEdge:
      mBottomEdgeLinksCollapsed = collapsed;
      break;

    case Qt::LeftEdge:
    case Qt::RightEdge:
      break;
  }
}

void QgsProcessingModelComponent::setComment( const QgsProcessingModelComment & )
{

}

void QgsProcessingModelComponent::saveCommonProperties( QVariantMap &map ) const
{
  map.insert( QStringLiteral( "component_pos_x" ), mPosition.x() );
  map.insert( QStringLiteral( "component_pos_y" ), mPosition.y() );
  map.insert( QStringLiteral( "component_description" ), mDescription );
  map.insert( QStringLiteral( "component_width" ), mSize.width() );
  map.insert( QStringLiteral( "component_height" ), mSize.height() );
  map.insert( QStringLiteral( "parameters_collapsed" ), mTopEdgeLinksCollapsed );
  map.insert( QStringLiteral( "outputs_collapsed" ), mBottomEdgeLinksCollapsed );
  map.insert( QStringLiteral( "color" ), mColor.isValid() ? QgsColorUtils::colorToString( mColor ) : QString() );
  const QgsProcessingModelComment *thisComment = comment();
  if ( thisComment )
    map.insert( QStringLiteral( "comment" ), thisComment->toVariant() );
}

void QgsProcessingModelComponent::restoreCommonProperties( const QVariantMap &map )
{
  QPointF pos;
  pos.setX( map.value( QStringLiteral( "component_pos_x" ) ).toDouble() );
  pos.setY( map.value( QStringLiteral( "component_pos_y" ) ).toDouble() );
  mPosition = pos;
  mDescription = map.value( QStringLiteral( "component_description" ) ).toString();
  mSize.setWidth( map.value( QStringLiteral( "component_width" ), QString::number( DEFAULT_COMPONENT_WIDTH ) ).toDouble() );
  mSize.setHeight( map.value( QStringLiteral( "component_height" ), QString::number( DEFAULT_COMPONENT_HEIGHT ) ).toDouble() );
  mColor = map.value( QStringLiteral( "color" ) ).toString().isEmpty() ? QColor() : QgsColorUtils::colorFromString( map.value( QStringLiteral( "color" ) ).toString() );
  mTopEdgeLinksCollapsed = map.value( QStringLiteral( "parameters_collapsed" ) ).toBool();
  mBottomEdgeLinksCollapsed = map.value( QStringLiteral( "outputs_collapsed" ) ).toBool();
  QgsProcessingModelComment *thisComment = comment();
  if ( thisComment )
    thisComment->loadVariant( map.value( QStringLiteral( "comment" ) ).toMap() );
}

void QgsProcessingModelComponent::copyNonDefinitionProperties( const QgsProcessingModelComponent &other )
{
  setPosition( other.position() );
  setSize( other.size() );
  setLinksCollapsed( Qt::TopEdge, other.linksCollapsed( Qt::TopEdge ) );
  setLinksCollapsed( Qt::BottomEdge, other.linksCollapsed( Qt::BottomEdge ) );
  QgsProcessingModelComment *thisComment = comment();
  const QgsProcessingModelComment *otherComment = other.comment();
  if ( thisComment && otherComment )
  {
    if ( !otherComment->position().isNull() )
      thisComment->setPosition( otherComment->position() );
    else
      thisComment->setPosition( other.position() + QPointF( size().width(), -1.5 * size().height() ) );
    thisComment->setSize( otherComment->size() );
  }
}

///@endcond

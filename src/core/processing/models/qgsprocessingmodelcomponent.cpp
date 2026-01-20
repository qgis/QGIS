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

#include "qgscolorutils.h"
#include "qgsprocessingmodelcomment.h"

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
  map.insert( u"component_pos_x"_s, mPosition.x() );
  map.insert( u"component_pos_y"_s, mPosition.y() );
  map.insert( u"component_description"_s, mDescription );
  map.insert( u"component_width"_s, mSize.width() );
  map.insert( u"component_height"_s, mSize.height() );
  map.insert( u"parameters_collapsed"_s, mTopEdgeLinksCollapsed );
  map.insert( u"outputs_collapsed"_s, mBottomEdgeLinksCollapsed );
  map.insert( u"color"_s, mColor.isValid() ? QgsColorUtils::colorToString( mColor ) : QString() );
  const QgsProcessingModelComment *thisComment = comment();
  if ( thisComment )
    map.insert( u"comment"_s, thisComment->toVariant() );
}

void QgsProcessingModelComponent::restoreCommonProperties( const QVariantMap &map )
{
  QPointF pos;
  pos.setX( map.value( u"component_pos_x"_s ).toDouble() );
  pos.setY( map.value( u"component_pos_y"_s ).toDouble() );
  mPosition = pos;
  mDescription = map.value( u"component_description"_s ).toString();
  mSize.setWidth( map.value( u"component_width"_s, QString::number( DEFAULT_COMPONENT_WIDTH ) ).toDouble() );
  mSize.setHeight( map.value( u"component_height"_s, QString::number( DEFAULT_COMPONENT_HEIGHT ) ).toDouble() );
  mColor = map.value( u"color"_s ).toString().isEmpty() ? QColor() : QgsColorUtils::colorFromString( map.value( u"color"_s ).toString() );
  mTopEdgeLinksCollapsed = map.value( u"parameters_collapsed"_s ).toBool();
  mBottomEdgeLinksCollapsed = map.value( u"outputs_collapsed"_s ).toBool();
  QgsProcessingModelComment *thisComment = comment();
  if ( thisComment )
    thisComment->loadVariant( map.value( u"comment"_s ).toMap() );
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

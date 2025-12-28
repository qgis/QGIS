/***************************************************************************
                             qgsmodelgraphicitem.cpp
                             ----------------------------------
    Date                 : February 2020
    Copyright            : (C) 2020 Nyall Dawson
    Email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsmodelgraphicitem.h"

#include "qgsapplication.h"
#include "qgsmodelcomponentgraphicitem.h"
#include "qgsmodelgraphicsscene.h"
#include "qgsmodelgraphicsview.h"
#include "qgsmodelviewmouseevent.h"
#include "qgsmodelviewtool.h"
#include "qgsprocessingalgorithm.h"
#include "qgsprocessingmodelchildalgorithm.h"
#include "qgsprocessingmodelcomponent.h"
#include "qgsprocessingoutputs.h"
#include "qgsprocessingparameters.h"

#include <QGraphicsSceneMouseEvent>
#include <QPainter>
#include <QSvgRenderer>

#include "moc_qgsmodelgraphicitem.cpp"

///@cond NOT_STABLE

QgsModelDesignerFlatButtonGraphicItem::QgsModelDesignerFlatButtonGraphicItem( QGraphicsItem *parent, const QPicture &picture, const QPointF &position, const QSizeF &size )
  : QGraphicsObject( parent )
  , mPicture( picture )
  , mPosition( position )
  , mSize( size )
{
  setAcceptHoverEvents( true );
  setFlag( QGraphicsItem::ItemIsMovable, false );
  setCacheMode( QGraphicsItem::DeviceCoordinateCache );
}

void QgsModelDesignerFlatButtonGraphicItem::paint( QPainter *painter, const QStyleOptionGraphicsItem *, QWidget * )
{
  if ( QgsModelGraphicsScene *modelScene = qobject_cast<QgsModelGraphicsScene *>( scene() ) )
  {
    if ( modelScene->flags() & QgsModelGraphicsScene::FlagHideControls )
      return;
  }

  if ( mHoverState )
  {
    painter->setPen( QPen( Qt::transparent, 1.0 ) );
    painter->setBrush( QBrush( QColor( 55, 55, 55, 33 ), Qt::SolidPattern ) );
  }
  else
  {
    painter->setPen( QPen( Qt::transparent, 1.0 ) );
    painter->setBrush( QBrush( Qt::transparent, Qt::SolidPattern ) );
  }
  const QPointF topLeft = mPosition - QPointF( std::floor( mSize.width() / 2 ), std::floor( mSize.height() / 2 ) );
  const QRectF rect = QRectF( topLeft.x(), topLeft.y(), mSize.width(), mSize.height() );
  painter->drawRect( rect );
  painter->drawPicture( topLeft.x(), topLeft.y(), mPicture );
}

QRectF QgsModelDesignerFlatButtonGraphicItem::boundingRect() const
{
  return QRectF( mPosition.x() - std::floor( mSize.width() / 2 ), mPosition.y() - std::floor( mSize.height() / 2 ), mSize.width(), mSize.height() );
}

void QgsModelDesignerFlatButtonGraphicItem::hoverEnterEvent( QGraphicsSceneHoverEvent * )
{
  if ( view()->tool() && !view()->tool()->allowItemInteraction() )
    mHoverState = false;
  else
    mHoverState = true;
  update();
}

void QgsModelDesignerFlatButtonGraphicItem::hoverLeaveEvent( QGraphicsSceneHoverEvent * )
{
  mHoverState = false;
  update();
}

void QgsModelDesignerFlatButtonGraphicItem::mousePressEvent( QGraphicsSceneMouseEvent * )
{
  if ( view()->tool() && view()->tool()->allowItemInteraction() )
    emit clicked();
}

void QgsModelDesignerFlatButtonGraphicItem::modelHoverEnterEvent( QgsModelViewMouseEvent * )
{
  if ( view()->tool() && !view()->tool()->allowItemInteraction() )
    mHoverState = false;
  else
    mHoverState = true;
  update();
}

void QgsModelDesignerFlatButtonGraphicItem::modelHoverLeaveEvent( QgsModelViewMouseEvent * )
{
  mHoverState = false;
  update();
}

void QgsModelDesignerFlatButtonGraphicItem::modelPressEvent( QgsModelViewMouseEvent *event )
{
  if ( view()->tool() && view()->tool()->allowItemInteraction() && event->button() == Qt::LeftButton )
  {
    QMetaObject::invokeMethod( this, "clicked", Qt::QueuedConnection );
    mHoverState = false;
    update();
  }
}

void QgsModelDesignerFlatButtonGraphicItem::setPosition( const QPointF &position )
{
  mPosition = position;
  prepareGeometryChange();
  update();
}

QgsModelGraphicsView *QgsModelDesignerFlatButtonGraphicItem::view()
{
  return qobject_cast<QgsModelGraphicsView *>( scene()->views().first() );
}

void QgsModelDesignerFlatButtonGraphicItem::setPicture( const QPicture &picture )
{
  mPicture = picture;
  update();
}

//
// QgsModelDesignerFoldButtonGraphicItem
//

QgsModelDesignerFoldButtonGraphicItem::QgsModelDesignerFoldButtonGraphicItem( QGraphicsItem *parent, bool folded, const QPointF &position, const QSizeF &size )
  : QgsModelDesignerFlatButtonGraphicItem( parent, QPicture(), position, size )
  , mFolded( folded )
{
  QSvgRenderer svg( QgsApplication::iconPath( u"mIconModelerExpand.svg"_s ) );
  QPainter painter( &mPlusPicture );
  svg.render( &painter );
  painter.end();

  QSvgRenderer svg2( QgsApplication::iconPath( u"mIconModelerCollapse.svg"_s ) );
  painter.begin( &mMinusPicture );
  svg2.render( &painter );
  painter.end();

  setPicture( mFolded ? mPlusPicture : mMinusPicture );
}

void QgsModelDesignerFoldButtonGraphicItem::mousePressEvent( QGraphicsSceneMouseEvent *event )
{
  mFolded = !mFolded;
  setPicture( mFolded ? mPlusPicture : mMinusPicture );
  emit folded( mFolded );
  QgsModelDesignerFlatButtonGraphicItem::mousePressEvent( event );
}

void QgsModelDesignerFoldButtonGraphicItem::modelPressEvent( QgsModelViewMouseEvent *event )
{
  mFolded = !mFolded;
  setPicture( mFolded ? mPlusPicture : mMinusPicture );
  emit folded( mFolded );
  QgsModelDesignerFlatButtonGraphicItem::modelPressEvent( event );
}


QgsModelDesignerSocketGraphicItem::QgsModelDesignerSocketGraphicItem( QgsModelComponentGraphicItem *parent, QgsProcessingModelComponent *component, int index, const QPointF &position, Qt::Edge edge, const QSizeF &size )
  : QgsModelDesignerFlatButtonGraphicItem( parent, QPicture(), position, size )
  , mComponentItem( parent )
  , mComponent( component )
  , mIndex( index )
  , mEdge( edge )
{
}

void QgsModelDesignerSocketGraphicItem::paint( QPainter *painter, const QStyleOptionGraphicsItem *, QWidget * )
{
  QColor outlineColor = socketColor();
  QColor fillColor = QColor( outlineColor );

  if ( isInput() )
  {
    fillColor.setAlpha( isDefaultParameterValue() ? 30 : 255 );
  }
  else
  {
    // outputs are always filled sockets
    fillColor.setAlpha( 255 );
  }

  // Outline style
  painter->setPen( QPen( outlineColor, mHoverState ? mSocketOutlineWidth * 2 : mSocketOutlineWidth ) );

  // Fill style
  painter->setBrush( QBrush( fillColor, Qt::SolidPattern ) );

  painter->setRenderHint( QPainter::Antialiasing );

  // Radius of the socket circle
  constexpr float DISPLAY_SIZE = 4;

  // Offset of the socket to separate from the label
  constexpr float ELLIPSE_OFFSET = 0.4;
  QPointF ellipsePosition = QPointF( position().x() + ELLIPSE_OFFSET, position().y() + ELLIPSE_OFFSET );
  painter->drawEllipse( ellipsePosition, DISPLAY_SIZE, DISPLAY_SIZE );

  /* Uncomment to display bounding box */
#if 0
  painter->save();
  painter->setPen( QPen() );
  painter->setBrush( QBrush() );
  painter->drawRect( boundingRect() );
  painter->restore();
#endif
}


QColor QgsModelDesignerSocketGraphicItem::socketColor() const
{
  return mComponentItem->linkColor( mEdge, mIndex );
}


bool QgsModelDesignerSocketGraphicItem::isDefaultParameterValue() const
{
  if ( !mComponent )
  {
    return false;
  }

  const QgsProcessingModelChildAlgorithm *child = dynamic_cast<const QgsProcessingModelChildAlgorithm *>( mComponent );

  if ( !child )
  {
    return false;
  }

  bool isDefaultValue = true;

  // We can only know if the socket should be filled if the algorithm is non null
  if ( child->algorithm() )
  {
    switch ( mEdge )
    {
      // Input params
      case Qt::TopEdge:
      {
        const QgsProcessingParameterDefinitions params = child->algorithm()->parameterDefinitions();
        const QgsProcessingParameterDefinition *param = params.value( mIndex );
        if ( !param )
          break;

        const QString name = param->name();

        QgsProcessingModelChildParameterSources paramSources = child->parameterSources().value( name );
        if ( paramSources.empty() )
        {
          break;
        }

        // The default value can only happen in the case of the parameter uses a static value
        if ( paramSources[0].source() != Qgis::ProcessingModelChildParameterSource::StaticValue )
        {
          isDefaultValue = false;
          break;
        }

        isDefaultValue = paramSources[0].staticValue() == param->defaultValue();
        break;
      }

      // Outputs
      case Qt::BottomEdge:
      case Qt::LeftEdge:
      case Qt::RightEdge:
        break;
    }
  }

  return isDefaultValue;
}

///@endcond

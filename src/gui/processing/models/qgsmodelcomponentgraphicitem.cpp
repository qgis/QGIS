/***************************************************************************
                             qgsmodelcomponentgraphicitem.cpp
                             ----------------------------------
    Date                 : March 2020
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

#include "qgsmodelcomponentgraphicitem.h"
#include "qgsprocessingmodelcomponent.h"
#include "qgsprocessingmodelparameter.h"
#include "qgsprocessingmodelchildalgorithm.h"
#include "qgsprocessingmodeloutput.h"
#include "qgsmodelgraphicsscene.h"
#include "qgsapplication.h"
#include "qgsmodelgraphicitem.h"
#include <QSvgRenderer>
#include <QPicture>
#include <QPainter>
#include <QGraphicsSceneHoverEvent>

///@cond NOT_STABLE

QgsModelComponentGraphicItem::QgsModelComponentGraphicItem( QgsProcessingModelComponent *component, QgsProcessingModelAlgorithm *model, QGraphicsItem *parent )
  : QGraphicsObject( parent )
  , mComponent( component )
  , mModel( model )
{
  setAcceptHoverEvents( true );
  setFlag( QGraphicsItem::ItemIsMovable, true );
  setFlag( QGraphicsItem::ItemIsSelectable, true );
  setFlag( QGraphicsItem::ItemSendsGeometryChanges, true );
  setZValue( QgsModelGraphicsScene::ZValues::ModelComponent );

  mFont.setPixelSize( 12 );

  QSvgRenderer svg( QgsApplication::iconPath( QStringLiteral( "mActionEditModelComponent.svg" ) ) );
  QPicture editPicture;
  QPainter painter( &editPicture );
  svg.render( &painter );
  painter.end();
  mEditButton = new QgsModelDesignerFlatButtonGraphicItem( this, editPicture,
      QPointF( component->size().width() / 2.0 - mButtonSize.width() / 2.0,
               component->size().height() / 2.0 - mButtonSize.height() / 2.0 ) );
  connect( mEditButton, &QgsModelDesignerFlatButtonGraphicItem::clicked, this, &QgsModelComponentGraphicItem::editComponent );

  QSvgRenderer svg2( QgsApplication::iconPath( QStringLiteral( "mActionDeleteModelComponent.svg" ) ) );
  QPicture deletePicture;
  painter.begin( &deletePicture );
  svg2.render( &painter );
  painter.end();
  mDeleteButton = new QgsModelDesignerFlatButtonGraphicItem( this, deletePicture,
      QPointF( component->size().width() / 2.0 - mButtonSize.width() / 2.0,
               mButtonSize.height() / 2.0 - component->size().height() / 2.0 ) );
  connect( mDeleteButton, &QgsModelDesignerFlatButtonGraphicItem::clicked, this, &QgsModelComponentGraphicItem::deleteComponent );
}

QgsProcessingModelComponent *QgsModelComponentGraphicItem::component()
{
  return mComponent.get();
}

QgsProcessingModelAlgorithm *QgsModelComponentGraphicItem::model()
{
  return mModel;
}

QFont QgsModelComponentGraphicItem::font() const
{
  return mFont;
}

void QgsModelComponentGraphicItem::setFont( const QFont &font )
{
  mFont = font;
  update();
}

void QgsModelComponentGraphicItem::mouseDoubleClickEvent( QGraphicsSceneMouseEvent * )
{
  editComponent();
}

void QgsModelComponentGraphicItem::hoverEnterEvent( QGraphicsSceneHoverEvent *event )
{
  updateToolTip( event->pos() );
}

void QgsModelComponentGraphicItem::hoverMoveEvent( QGraphicsSceneHoverEvent *event )
{
  updateToolTip( event->pos() );
}

void QgsModelComponentGraphicItem::hoverLeaveEvent( QGraphicsSceneHoverEvent * )
{
  setToolTip( QString() );
  if ( mIsHovering )
  {
    mIsHovering = false;
    update();
    emit repaintArrows();
  }
}

QRectF QgsModelComponentGraphicItem::itemRect() const
{
  return QRectF( -( mComponent->size().width() + 2 ) / 2.0,
                 -( mComponent->size().height() + 2 ) / 2.0,
                 mComponent->size().width() + 2,
                 mComponent->size().height() + 2 );
}

QString QgsModelComponentGraphicItem::truncatedTextForItem( const QString &text ) const
{
  QFontMetricsF fm( mFont );
  double width = fm.boundingRect( text ).width();
  if ( width < mComponent->size().width() - 25 - mButtonSize.width() )
    return text;

  QString t = text;
  t = t.left( t.length() - 3 ) + QChar( 0x2026 );
  width = fm.boundingRect( t ).width();
  while ( width > mComponent->size().width() - 25 - mButtonSize.width() )
  {
    t = t.left( t.length() - 4 ) + QChar( 0x2026 );
    width = fm.boundingRect( t ).width();
  }
  return t;
}

void QgsModelComponentGraphicItem::updateToolTip( const QPointF &pos )
{
  const bool prevHoverStatus = mIsHovering;
  if ( itemRect().contains( pos ) )
  {
    setToolTip( mLabel );
    mIsHovering = true;
  }
  else
  {
    setToolTip( QString() );
    mIsHovering = false;
  }
  if ( mIsHovering != prevHoverStatus )
  {
    update();
    emit repaintArrows();
  }
}

QString QgsModelComponentGraphicItem::label() const
{
  return mLabel;
}

void QgsModelComponentGraphicItem::setLabel( const QString &label )
{
  mLabel = label;
  update();
}

QgsModelComponentGraphicItem::State QgsModelComponentGraphicItem::state() const
{
  if ( isSelected() )
    return Selected;
  else if ( mIsHovering )
    return Hover;
  else
    return Normal;
}


QgsModelParameterGraphicItem::QgsModelParameterGraphicItem( QgsProcessingModelParameter *parameter, QgsProcessingModelAlgorithm *model, QGraphicsItem *parent )
  : QgsModelComponentGraphicItem( parameter, model, parent )
{

}



QgsModelChildAlgorithmGraphicItem::QgsModelChildAlgorithmGraphicItem( QgsProcessingModelChildAlgorithm *child, QgsProcessingModelAlgorithm *model, QGraphicsItem *parent )
  : QgsModelComponentGraphicItem( child, model, parent )
{

}


QgsModelOutputGraphicItem::QgsModelOutputGraphicItem( QgsProcessingModelOutput *output, QgsProcessingModelAlgorithm *model, QGraphicsItem *parent )
  : QgsModelComponentGraphicItem( output, model, parent )
{
}

///@endcond

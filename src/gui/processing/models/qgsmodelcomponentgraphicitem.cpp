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
#include "qgsprocessingmodelalgorithm.h"
#include <QSvgRenderer>
#include <QPicture>
#include <QPainter>
#include <QGraphicsSceneHoverEvent>
#include <QApplication>
#include <QPalette>

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

QgsModelComponentGraphicItem::~QgsModelComponentGraphicItem() = default;

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

QVariant QgsModelComponentGraphicItem::itemChange( QGraphicsItem::GraphicsItemChange change, const QVariant &value )
{
  switch ( change )
  {
    case QGraphicsItem::ItemPositionHasChanged:
    {
      emit updateArrowPaths();
      mComponent->setPosition( pos() );

      // also need to update the model's stored component's position
      // TODO - this is not so nice, consider moving this to model class
      if ( QgsProcessingModelChildAlgorithm *child = dynamic_cast< QgsProcessingModelChildAlgorithm * >( mComponent.get() ) )
        mModel->childAlgorithm( child->childId() ).setPosition( pos() );
      else if ( QgsProcessingModelParameter *param = dynamic_cast< QgsProcessingModelParameter * >( mComponent.get() ) )
        mModel->parameterComponent( param->parameterName() ).setPosition( pos() );
      else if ( QgsProcessingModelOutput *output = dynamic_cast< QgsProcessingModelOutput * >( mComponent.get() ) )
        mModel->childAlgorithm( output->childId() ).modelOutput( output->name() ).setPosition( pos() );

      break;
    }
    case QGraphicsItem::ItemSelectedChange:
    {
      emit repaintArrows();
      break;
    }

    case QGraphicsItem::ItemSceneChange:
    {
      if ( !mInitialized )
      {
        // ideally would be in constructor, but cannot call virtual methods from that...
        if ( linkPointCount( Qt::TopEdge ) )
        {
          QPointF pt = linkPoint( Qt::TopEdge, -1 );
          pt = QPointF( 0, pt.y() );
          mExpandTopButton = new QgsModelDesignerFoldButtonGraphicItem( this, mComponent->linksCollapsed( Qt::TopEdge ), pt );
          connect( mExpandTopButton, &QgsModelDesignerFoldButtonGraphicItem::folded, this, [ = ]( bool folded ) { fold( Qt::TopEdge, folded ); } );
        }
        if ( linkPointCount( Qt::BottomEdge ) )
        {
          QPointF pt = linkPoint( Qt::BottomEdge, -1 );
          pt = QPointF( 0, pt.y() );
          mExpandBottomButton = new QgsModelDesignerFoldButtonGraphicItem( this, mComponent->linksCollapsed( Qt::BottomEdge ), pt );
          connect( mExpandBottomButton, &QgsModelDesignerFoldButtonGraphicItem::folded, this, [ = ]( bool folded ) { fold( Qt::BottomEdge, folded ); } );
        }
      }
      break;
    }

    default:
      break;
  }

  return QGraphicsObject::itemChange( change, value );
}

QRectF QgsModelComponentGraphicItem::boundingRect() const
{
  QFontMetricsF fm( mFont );
  const int linksAbove = mComponent->linksCollapsed( Qt::TopEdge ) ? 0 : linkPointCount( Qt::TopEdge );
  const int linksBelow = mComponent->linksCollapsed( Qt::BottomEdge ) ? 0 : linkPointCount( Qt::BottomEdge );

  const double hUp = fm.height() * 1.2 * ( linksAbove + 2 );
  const double hDown = fm.height() * 1.2 * ( linksBelow + 2 );
  return QRectF( -( mComponent->size().width() + 2 ) / 2,
                 -( mComponent->size().height() + 2 ) / 2 - hUp,
                 mComponent->size().width() + 2,
                 mComponent->size().height() + hDown + hUp );
}

void QgsModelComponentGraphicItem::paint( QPainter *painter, const QStyleOptionGraphicsItem *, QWidget * )
{
  const QRectF rect = itemRect();
  QColor color = fillColor( state() );
  QColor stroke = strokeColor( state() );

  painter->setPen( QPen( stroke, 0 ) ); // 0 width "cosmetic" pen
  painter->setBrush( QBrush( color, Qt::SolidPattern ) );
  painter->drawRect( rect );
  painter->setFont( font() );
  painter->setPen( QPen( textColor( state() ) ) );

  QString text = truncatedTextForItem( label() );

  const QSizeF componentSize = mComponent->size();

  QFontMetricsF fm( font() );
  double h = fm.ascent();
  QPointF pt( -componentSize.width() / 2 + 25, componentSize.height() / 2.0 - h + 1 );
  painter->drawText( pt, text );
  painter->setPen( QPen( QApplication::palette().color( QPalette::WindowText ) ) );

  if ( linkPointCount( Qt::TopEdge ) || linkPointCount( Qt::BottomEdge ) )
  {
    h = -( fm.height() * 1.2 );
    h = h - componentSize.height() / 2.0 + 5;
    pt = QPointF( -componentSize.width() / 2 + 25, h );
    painter->drawText( pt, QObject::tr( "In" ) );
    int i = 1;
    if ( !mComponent->linksCollapsed( Qt::TopEdge ) )
    {
      for ( int idx = 0; idx < linkPointCount( Qt::TopEdge ); ++idx )
      {
        text = linkPointText( Qt::TopEdge, idx );
        h = -( fm.height() * 1.2 ) * ( i + 1 );
        h = h - componentSize.height() / 2.0 + 5;
        pt = QPointF( -componentSize.width() / 2 + 33, h );
        painter->drawText( pt, text );
        i += 1;
      }
    }

    h = fm.height() * 1.1;
    h = h + componentSize.height() / 2.0;
    pt = QPointF( -componentSize.width() / 2 + 25, h );
    painter->drawText( pt, QObject::tr( "Out" ) );
    if ( !mComponent->linksCollapsed( Qt::BottomEdge ) )
    {
      for ( int idx = 0; idx < linkPointCount( Qt::BottomEdge ); ++idx )
      {
        text = linkPointText( Qt::BottomEdge, idx );
        h = fm.height() * 1.2 * ( idx + 2 );
        h = h + componentSize.height() / 2.0;
        pt = QPointF( -componentSize.width() / 2 + 33, h );
        painter->drawText( pt, text );
      }
    }
  }

  const QPixmap px = iconPixmap();
  if ( !px.isNull() )
  {
    painter->drawPixmap( -( componentSize.width() / 2.0 ) + 3, -8, px );
  }
  else
  {
    const QPicture pic = iconPicture();
    if ( !pic.isNull() )
    {
      painter->drawPicture( -( componentSize.width() / 2.0 ) + 3, -8, pic );
    }
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

QPicture QgsModelComponentGraphicItem::iconPicture() const
{
  return QPicture();
}

QPixmap QgsModelComponentGraphicItem::iconPixmap() const
{
  return QPixmap();
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

void QgsModelComponentGraphicItem::fold( Qt::Edge edge, bool folded )
{
  mComponent->setLinksCollapsed( edge, folded );
  // also need to update the model's stored component

  // TODO - this is not so nice, consider moving this to model class
  if ( QgsProcessingModelChildAlgorithm *child = dynamic_cast< QgsProcessingModelChildAlgorithm * >( mComponent.get() ) )
    mModel->childAlgorithm( child->childId() ).setLinksCollapsed( edge, folded );
  else if ( QgsProcessingModelParameter *param = dynamic_cast< QgsProcessingModelParameter * >( mComponent.get() ) )
    mModel->parameterComponent( param->parameterName() ).setLinksCollapsed( edge, folded );
  else if ( QgsProcessingModelOutput *output = dynamic_cast< QgsProcessingModelOutput * >( mComponent.get() ) )
    mModel->childAlgorithm( output->childId() ).modelOutput( output->name() ).setLinksCollapsed( edge, folded );

  prepareGeometryChange();
  emit updateArrowPaths();
  update();
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

int QgsModelComponentGraphicItem::linkPointCount( Qt::Edge ) const
{
  return 0;
}

QString QgsModelComponentGraphicItem::linkPointText( Qt::Edge, int ) const
{
  return QString();
}

QPointF QgsModelComponentGraphicItem::linkPoint( Qt::Edge edge, int index ) const
{
  switch ( edge )
  {
    case Qt::BottomEdge:
    {
      if ( linkPointCount( Qt::BottomEdge ) )
      {
        const int pointIndex = !mComponent->linksCollapsed( Qt::BottomEdge ) ? index : -1;
        const QString text = truncatedTextForItem( linkPointText( Qt::BottomEdge, index ) );
        QFontMetricsF fm( mFont );
        const double w = fm.boundingRect( text ).width();
        const double h = fm.height() * 1.2 * ( pointIndex + 1 ) + fm.height() / 2.0;
        const double y = h + mComponent->size().height() / 2.0 + 5;
        const double x = !mComponent->linksCollapsed( Qt::BottomEdge ) ? ( -mComponent->size().width() / 2 + 33 + w + 5 ) : 10;
        return QPointF( x, y );
      }
      break;
    }

    case Qt::TopEdge:
    {
      if ( linkPointCount( Qt::TopEdge ) )
      {
        double offsetX = 25;
        int paramIndex = index;
        if ( mComponent->linksCollapsed( Qt::TopEdge ) )
        {
          paramIndex = -1;
          offsetX = 17;
        }
        QFontMetricsF fm( mFont );
        double h = -( fm.height() * 1.2 ) * ( paramIndex + 2 ) - fm.height() / 2.0 + 8;
        h = h - mComponent->size().height() / 2.0;
        return QPointF( -mComponent->size().width() / 2 + offsetX, h );
      }
      break;
    }
    case Qt::LeftEdge:
    case Qt::RightEdge:
      break;
  }

  return QPointF();
}

QPointF QgsModelComponentGraphicItem::calculateAutomaticLinkPoint( QgsModelComponentGraphicItem *other, Qt::Edge &edge ) const
{
  // find closest edge to other item
  const QgsRectangle otherRect( other->itemRect().translated( other->pos() ) );

  const QPointF leftPoint = pos() + QPointF( -mComponent->size().width() / 2.0, 0 );
  const double distLeft = otherRect.distance( QgsPointXY( leftPoint ) );

  const QPointF rightPoint = pos() + QPointF( mComponent->size().width() / 2.0, 0 );
  const double distRight = otherRect.distance( QgsPointXY( rightPoint ) );

  const QPointF topPoint = pos() + QPointF( 0, -mComponent->size().height() / 2.0 );
  const double distTop = otherRect.distance( QgsPointXY( topPoint ) );

  const QPointF bottomPoint = pos() + QPointF( 0, mComponent->size().height() / 2.0 );
  const double distBottom = otherRect.distance( QgsPointXY( bottomPoint ) );

  if ( distLeft <= distRight && distLeft <= distTop && distLeft <= distBottom )
  {
    edge = Qt::LeftEdge;
    return leftPoint;
  }
  else if ( distRight <= distTop && distRight <= distBottom )
  {
    edge = Qt::RightEdge;
    return rightPoint;
  }
  else if ( distBottom <= distTop )
  {
    edge = Qt::BottomEdge;
    return bottomPoint;
  }
  else
  {
    edge = Qt::TopEdge;
    return topPoint;
  }
}

QPointF QgsModelComponentGraphicItem::calculateAutomaticLinkPoint( const QPointF &point, Qt::Edge &edge ) const
{
  // find closest edge to other point
  const QgsPointXY otherPt( point );
  const QPointF leftPoint = pos() + QPointF( -mComponent->size().width() / 2.0, 0 );
  const double distLeft = otherPt.distance( QgsPointXY( leftPoint ) );

  const QPointF rightPoint = pos() + QPointF( mComponent->size().width() / 2.0, 0 );
  const double distRight = otherPt.distance( QgsPointXY( rightPoint ) );

  const QPointF topPoint = pos() + QPointF( 0, -mComponent->size().height() / 2.0 );
  const double distTop = otherPt.distance( QgsPointXY( topPoint ) );

  const QPointF bottomPoint = pos() + QPointF( 0, mComponent->size().height() / 2.0 );
  const double distBottom = otherPt.distance( QgsPointXY( bottomPoint ) );

  if ( distLeft <= distRight && distLeft <= distTop && distLeft <= distBottom )
  {
    edge = Qt::LeftEdge;
    return leftPoint;
  }
  else if ( distRight <= distTop && distRight <= distBottom )
  {
    edge = Qt::RightEdge;
    return rightPoint;
  }
  else if ( distBottom <= distTop )
  {
    edge = Qt::BottomEdge;
    return bottomPoint;
  }
  else
  {
    edge = Qt::TopEdge;
    return topPoint;
  }
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

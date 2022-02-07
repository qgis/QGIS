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
#include "qgsprocessingmodelgroupbox.h"
#include "qgsmodelgraphicsscene.h"
#include "qgsapplication.h"
#include "qgsmodelgraphicitem.h"
#include "qgsprocessingmodelalgorithm.h"
#include "qgsmodelgraphicsview.h"
#include "qgsmodelviewtool.h"
#include "qgsmodelviewmouseevent.h"
#include "qgsmodelgroupboxdefinitionwidget.h"

#include <QSvgRenderer>
#include <QPicture>
#include <QPainter>
#include <QGraphicsSceneHoverEvent>
#include <QApplication>
#include <QPalette>
#include <QMessageBox>
#include <QMenu>

///@cond NOT_STABLE

QgsModelComponentGraphicItem::QgsModelComponentGraphicItem( QgsProcessingModelComponent *component, QgsProcessingModelAlgorithm *model, QGraphicsItem *parent )
  : QGraphicsObject( parent )
  , mComponent( component )
  , mModel( model )
{
  setAcceptHoverEvents( true );
  setFlag( QGraphicsItem::ItemIsSelectable, true );
  setFlag( QGraphicsItem::ItemSendsGeometryChanges, true );
  setZValue( QgsModelGraphicsScene::ZValues::ModelComponent );

  mFont.setPixelSize( 12 );

  QSvgRenderer svg( QgsApplication::iconPath( QStringLiteral( "mActionEditModelComponent.svg" ) ) );
  QPicture editPicture;
  QPainter painter( &editPicture );
  svg.render( &painter );
  painter.end();
  mEditButton = new QgsModelDesignerFlatButtonGraphicItem( this, editPicture, QPointF( 0, 0 ) );
  connect( mEditButton, &QgsModelDesignerFlatButtonGraphicItem::clicked, this, &QgsModelComponentGraphicItem::editComponent );

  QSvgRenderer svg2( QgsApplication::iconPath( QStringLiteral( "mActionDeleteModelComponent.svg" ) ) );
  QPicture deletePicture;
  painter.begin( &deletePicture );
  svg2.render( &painter );
  painter.end();
  mDeleteButton = new QgsModelDesignerFlatButtonGraphicItem( this, deletePicture, QPointF( 0, 0 ) );
  connect( mDeleteButton, &QgsModelDesignerFlatButtonGraphicItem::clicked, this, &QgsModelComponentGraphicItem::deleteComponent );

  updateButtonPositions();
}

QgsModelComponentGraphicItem::Flags QgsModelComponentGraphicItem::flags() const
{
  return QgsModelComponentGraphicItem::Flags();
}

QgsModelComponentGraphicItem::~QgsModelComponentGraphicItem() = default;

QgsProcessingModelComponent *QgsModelComponentGraphicItem::component()
{
  return mComponent.get();
}

const QgsProcessingModelComponent *QgsModelComponentGraphicItem::component() const
{
  return mComponent.get();
}

QgsProcessingModelAlgorithm *QgsModelComponentGraphicItem::model()
{
  return mModel;
}

QgsModelGraphicsView *QgsModelComponentGraphicItem::view()
{
  if ( scene()->views().isEmpty() )
    return nullptr;

  return qobject_cast< QgsModelGraphicsView * >( scene()->views().first() );
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

void QgsModelComponentGraphicItem::moveComponentBy( qreal dx, qreal dy )
{
  setPos( mComponent->position().x() + dx, mComponent->position().y() + dy );
  mComponent->setPosition( pos() );

  emit aboutToChange( tr( "Move %1" ).arg( mComponent->description() ) );
  updateStoredComponentPosition( pos(), mComponent->size() );
  emit changed();

  emit sizePositionChanged();
  emit updateArrowPaths();
}

void QgsModelComponentGraphicItem::previewItemMove( qreal dx, qreal dy )
{
  setPos( mComponent->position().x() + dx, mComponent->position().y() + dy );
  emit updateArrowPaths();
}

void QgsModelComponentGraphicItem::setItemRect( QRectF rect )
{
  rect = rect.normalized();

  if ( rect.width() < MIN_COMPONENT_WIDTH )
    rect.setWidth( MIN_COMPONENT_WIDTH );
  if ( rect.height() < MIN_COMPONENT_HEIGHT )
    rect.setHeight( MIN_COMPONENT_HEIGHT );

  setPos( rect.center() );
  prepareGeometryChange();

  emit aboutToChange( tr( "Resize %1" ).arg( mComponent->description() ) );

  mComponent->setPosition( pos() );
  mComponent->setSize( rect.size() );
  updateStoredComponentPosition( pos(), mComponent->size() );

  updateButtonPositions();
  emit changed();

  emit updateArrowPaths();
  emit sizePositionChanged();
}

QRectF QgsModelComponentGraphicItem::previewItemRectChange( QRectF rect )
{
  rect = rect.normalized();

  if ( rect.width() < MIN_COMPONENT_WIDTH )
    rect.setWidth( MIN_COMPONENT_WIDTH );
  if ( rect.height() < MIN_COMPONENT_HEIGHT )
    rect.setHeight( MIN_COMPONENT_HEIGHT );

  setPos( rect.center() );
  prepareGeometryChange();

  mTempSize = rect.size();

  updateButtonPositions();
  emit updateArrowPaths();

  return rect;
}

void QgsModelComponentGraphicItem::finalizePreviewedItemRectChange( QRectF )
{
  mComponent->setPosition( pos() );
  prepareGeometryChange();
  mComponent->setSize( mTempSize );
  mTempSize = QSizeF();

  emit aboutToChange( tr( "Resize %1" ).arg( mComponent->description() ) );
  updateStoredComponentPosition( pos(), mComponent->size() );

  updateButtonPositions();

  emit changed();

  emit sizePositionChanged();
  emit updateArrowPaths();
}

void QgsModelComponentGraphicItem::modelHoverEnterEvent( QgsModelViewMouseEvent *event )
{
  if ( view() && view()->tool() && view()->tool()->allowItemInteraction() )
    updateToolTip( mapFromScene( event->modelPoint() ) );
}

void QgsModelComponentGraphicItem::modelHoverMoveEvent( QgsModelViewMouseEvent *event )
{
  if ( view() && view()->tool() && view()->tool()->allowItemInteraction() )
    updateToolTip( mapFromScene( event->modelPoint() ) );
}

void QgsModelComponentGraphicItem::modelHoverLeaveEvent( QgsModelViewMouseEvent * )
{
  if ( view() && view()->tool() && view()->tool()->allowItemInteraction() )
  {
    setToolTip( QString() );
    if ( mIsHovering )
    {
      mIsHovering = false;
      update();
      emit repaintArrows();
    }
  }
}

void QgsModelComponentGraphicItem::modelDoubleClickEvent( QgsModelViewMouseEvent * )
{
  if ( view() && view()->tool() && view()->tool()->allowItemInteraction() )
    editComponent();
}

void QgsModelComponentGraphicItem::mouseDoubleClickEvent( QGraphicsSceneMouseEvent * )
{
  if ( view() && view()->tool() && view()->tool()->allowItemInteraction() )
    editComponent();
}

void QgsModelComponentGraphicItem::hoverEnterEvent( QGraphicsSceneHoverEvent *event )
{
  if ( view() && view()->tool() && view()->tool()->allowItemInteraction() )
    updateToolTip( event->pos() );
}

void QgsModelComponentGraphicItem::hoverMoveEvent( QGraphicsSceneHoverEvent *event )
{
  if ( view() && view()->tool() && view()->tool()->allowItemInteraction() )
    updateToolTip( event->pos() );
}

void QgsModelComponentGraphicItem::hoverLeaveEvent( QGraphicsSceneHoverEvent * )
{
  modelHoverLeaveEvent( nullptr );
}

QVariant QgsModelComponentGraphicItem::itemChange( QGraphicsItem::GraphicsItemChange change, const QVariant &value )
{
  switch ( change )
  {
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
          mExpandTopButton = new QgsModelDesignerFoldButtonGraphicItem( this, mComponent->linksCollapsed( Qt::TopEdge ), QPointF( 0, 0 ) );
          connect( mExpandTopButton, &QgsModelDesignerFoldButtonGraphicItem::folded, this, [ = ]( bool folded ) { fold( Qt::TopEdge, folded ); } );
        }
        if ( linkPointCount( Qt::BottomEdge ) )
        {
          mExpandBottomButton = new QgsModelDesignerFoldButtonGraphicItem( this, mComponent->linksCollapsed( Qt::BottomEdge ), QPointF( 0, 0 ) );
          connect( mExpandBottomButton, &QgsModelDesignerFoldButtonGraphicItem::folded, this, [ = ]( bool folded ) { fold( Qt::BottomEdge, folded ); } );
        }
        mInitialized = true;
        updateButtonPositions();
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
  const QFontMetricsF fm( mFont );
  const int linksAbove = linkPointCount( Qt::TopEdge );
  const int linksBelow = linkPointCount( Qt::BottomEdge );

  const double hUp = linksAbove == 0 ? 0 :
                     fm.height() * 1.2 * ( ( mComponent->linksCollapsed( Qt::TopEdge ) ? 0 : linksAbove ) + 2 );
  const double hDown = linksBelow == 0 ? 0 :
                       fm.height() * 1.2 * ( ( mComponent->linksCollapsed( Qt::BottomEdge ) ? 0 : linksBelow ) + 2 );
  return QRectF( -( itemSize().width() ) / 2 - RECT_PEN_SIZE,
                 -( itemSize().height() ) / 2 - hUp - RECT_PEN_SIZE,
                 itemSize().width() + 2 * RECT_PEN_SIZE,
                 itemSize().height() + hDown + hUp + 2 * RECT_PEN_SIZE );
}

bool QgsModelComponentGraphicItem::contains( const QPointF &point ) const
{
  const QRectF paintingBounds = boundingRect();
  if ( point.x() < paintingBounds.left() + RECT_PEN_SIZE )
    return false;
  if ( point.x() > paintingBounds.right() - RECT_PEN_SIZE )
    return false;
  if ( point.y() < paintingBounds.top() + RECT_PEN_SIZE )
    return false;
  if ( point.y() > paintingBounds.bottom() - RECT_PEN_SIZE )
    return false;

  return true;
}

void QgsModelComponentGraphicItem::paint( QPainter *painter, const QStyleOptionGraphicsItem *, QWidget * )
{
  const QRectF rect = itemRect();
  QColor color;
  QColor stroke;
  QColor foreColor;
  if ( mComponent->color().isValid() )
  {
    color = mComponent->color();
    switch ( state() )
    {
      case Selected:
        color = color.darker( 110 );
        break;
      case Hover:
        color = color.darker( 105 );
        break;

      case Normal:
        break;
    }
    stroke = color.darker( 110 );
    foreColor = color.lightness() > 150 ? QColor( 0, 0, 0 ) : QColor( 255, 255, 255 );
  }
  else
  {
    color = fillColor( state() );
    stroke = strokeColor( state() );
    foreColor = textColor( state() );
  }

  QPen strokePen = QPen( stroke, 0 ) ; // 0 width "cosmetic" pen
  strokePen.setStyle( strokeStyle( state() ) );
  painter->setPen( strokePen );
  painter->setBrush( QBrush( color, Qt::SolidPattern ) );
  painter->drawRect( rect );
  painter->setFont( font() );
  painter->setPen( QPen( foreColor ) );

  QString text;

  const QSizeF componentSize = itemSize();

  const QFontMetricsF fm( font() );
  double h = fm.ascent();
  QPointF pt( -componentSize.width() / 2 + 25, componentSize.height() / 2.0 - h + 1 );

  if ( iconPicture().isNull() && iconPixmap().isNull() )
  {
    const QRectF labelRect = QRectF( rect.left() + TEXT_MARGIN, rect.top() + TEXT_MARGIN, rect.width() - 2 * TEXT_MARGIN - mButtonSize.width() - BUTTON_MARGIN, rect.height() - 2 * TEXT_MARGIN );
    text = label();
    painter->drawText( labelRect, Qt::TextWordWrap | titleAlignment(), text );
  }
  else
  {
    const QRectF labelRect = QRectF( rect.left() + 21 + TEXT_MARGIN, rect.top() + TEXT_MARGIN,
                                     rect.width() - 2 * TEXT_MARGIN - mButtonSize.width() - BUTTON_MARGIN - 21, rect.height() - 2 * TEXT_MARGIN );
    text = label();
    painter->drawText( labelRect, Qt::TextWordWrap | Qt::AlignVCenter, text );
  }

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
    painter->drawPixmap( QPointF( -( componentSize.width() / 2.0 ) + 3, -8 ), px );
  }
  else
  {
    const QPicture pic = iconPicture();
    if ( !pic.isNull() )
    {
      painter->drawPicture( QPointF( -( componentSize.width() / 2.0 ) + 3, -8 ), pic );
    }
  }
}

QRectF QgsModelComponentGraphicItem::itemRect( bool storedRect ) const
{
  if ( storedRect )
  {
    return QRectF( mComponent->position().x() - ( mComponent->size().width() ) / 2.0,
                   mComponent->position().y()  - ( mComponent->size().height() ) / 2.0,
                   mComponent->size().width(),
                   mComponent->size().height() );
  }
  else
    return QRectF( -( itemSize().width() ) / 2.0,
                   -( itemSize().height() ) / 2.0,
                   itemSize().width(),
                   itemSize().height() );
}

QString QgsModelComponentGraphicItem::truncatedTextForItem( const QString &text ) const
{
  const QFontMetricsF fm( mFont );
  double width = fm.boundingRect( text ).width();
  if ( width < itemSize().width() - 25 - mButtonSize.width() )
    return text;

  QString t = text;
  t = t.left( t.length() - 3 ) + QChar( 0x2026 );
  width = fm.boundingRect( t ).width();
  while ( width > itemSize().width() - 25 - mButtonSize.width() )
  {
    if ( t.length() < 5 )
      break;

    t = t.left( t.length() - 4 ) + QChar( 0x2026 );
    width = fm.boundingRect( t ).width();
  }
  return t;
}

Qt::PenStyle QgsModelComponentGraphicItem::strokeStyle( QgsModelComponentGraphicItem::State ) const
{
  return Qt::SolidLine;
}

Qt::Alignment QgsModelComponentGraphicItem::titleAlignment() const
{
  return Qt::AlignLeft;
}

QPicture QgsModelComponentGraphicItem::iconPicture() const
{
  return QPicture();
}

QPixmap QgsModelComponentGraphicItem::iconPixmap() const
{
  return QPixmap();
}

void QgsModelComponentGraphicItem::updateButtonPositions()
{
  mEditButton->setPosition( QPointF( itemSize().width() / 2.0 - mButtonSize.width() / 2.0 - BUTTON_MARGIN,
                                     itemSize().height() / 2.0 - mButtonSize.height() / 2.0 - BUTTON_MARGIN ) );
  mDeleteButton->setPosition( QPointF( itemSize().width() / 2.0 - mButtonSize.width() / 2.0 - BUTTON_MARGIN,
                                       mButtonSize.height() / 2.0 - itemSize().height() / 2.0 + BUTTON_MARGIN ) );

  if ( mExpandTopButton )
  {
    const QPointF pt = linkPoint( Qt::TopEdge, -1, true );
    mExpandTopButton->setPosition( QPointF( 0, pt.y() ) );
  }
  if ( mExpandBottomButton )
  {
    const QPointF pt = linkPoint( Qt::BottomEdge, -1, false );
    mExpandBottomButton->setPosition( QPointF( 0, pt.y() ) );
  }
}

QSizeF QgsModelComponentGraphicItem::itemSize() const
{
  return !mTempSize.isValid() ? mComponent->size() : mTempSize;
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
  emit aboutToChange( !folded ? tr( "Expand Item" ) : tr( "Collapse Item" ) );
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
  emit changed();
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

QPointF QgsModelComponentGraphicItem::linkPoint( Qt::Edge edge, int index, bool incoming ) const
{
  switch ( edge )
  {
    case Qt::BottomEdge:
    {
      if ( linkPointCount( Qt::BottomEdge ) )
      {
        double offsetX = 25;
        if ( mComponent->linksCollapsed( Qt::BottomEdge ) )
        {
          offsetX = 17;
        }
        const int pointIndex = !mComponent->linksCollapsed( Qt::BottomEdge ) ? index : -1;
        const QString text = truncatedTextForItem( linkPointText( Qt::BottomEdge, index ) );
        const QFontMetricsF fm( mFont );
        const double w = fm.boundingRect( text ).width();
        const double h = fm.height() * 1.2 * ( pointIndex + 1 ) + fm.height() / 2.0;
        const double y = h + itemSize().height() / 2.0 + 5;
        const double x = !mComponent->linksCollapsed( Qt::BottomEdge ) ? ( -itemSize().width() / 2 + 33 + w + 5 ) : 10;
        return QPointF( incoming ? -itemSize().width() / 2 + offsetX
                        :  x,
                        y );
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
        const QFontMetricsF fm( mFont );
        const QString text = truncatedTextForItem( linkPointText( Qt::TopEdge, index ) );
        const double w = fm.boundingRect( text ).width();
        double h = -( fm.height() * 1.2 ) * ( paramIndex + 2 ) - fm.height() / 2.0 + 8;
        h = h - itemSize().height() / 2.0;
        return QPointF( incoming ? -itemSize().width() / 2 + offsetX
                        : ( !mComponent->linksCollapsed( Qt::TopEdge ) ? ( -itemSize().width() / 2 + 33 + w + 5 ) : 10 ),
                        h );
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

  const QPointF leftPoint = pos() + QPointF( -itemSize().width() / 2.0, 0 );
  const double distLeft = otherRect.distance( QgsPointXY( leftPoint ) );

  const QPointF rightPoint = pos() + QPointF( itemSize().width() / 2.0, 0 );
  const double distRight = otherRect.distance( QgsPointXY( rightPoint ) );

  const QPointF topPoint = pos() + QPointF( 0, -itemSize().height() / 2.0 );
  const double distTop = otherRect.distance( QgsPointXY( topPoint ) );

  const QPointF bottomPoint = pos() + QPointF( 0, itemSize().height() / 2.0 );
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
  const QPointF leftPoint = pos() + QPointF( -itemSize().width() / 2.0, 0 );
  const double distLeft = otherPt.distance( QgsPointXY( leftPoint ) );

  const QPointF rightPoint = pos() + QPointF( itemSize().width() / 2.0, 0 );
  const double distRight = otherPt.distance( QgsPointXY( rightPoint ) );

  const QPointF topPoint = pos() + QPointF( 0, -itemSize().height() / 2.0 );
  const double distTop = otherPt.distance( QgsPointXY( topPoint ) );

  const QPointF bottomPoint = pos() + QPointF( 0, itemSize().height() / 2.0 );
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
  QSvgRenderer svg( QgsApplication::iconPath( QStringLiteral( "mIconModelInput.svg" ) ) );
  QPainter painter( &mPicture );
  svg.render( &painter );
  painter.end();

  if ( const QgsProcessingParameterDefinition *paramDef = model->parameterDefinition( parameter->parameterName() ) )
    setLabel( paramDef->description() );
  else
    setLabel( QObject::tr( "Error (%1)" ).arg( parameter->parameterName() ) );
}

void QgsModelParameterGraphicItem::contextMenuEvent( QGraphicsSceneContextMenuEvent *event )
{
  QMenu *popupmenu = new QMenu( event->widget() );
  QAction *removeAction = popupmenu->addAction( QObject::tr( "Remove" ) );
  connect( removeAction, &QAction::triggered, this, &QgsModelParameterGraphicItem::deleteComponent );
  QAction *editAction = popupmenu->addAction( QObject::tr( "Edit…" ) );
  connect( editAction, &QAction::triggered, this, &QgsModelParameterGraphicItem::editComponent );
  QAction *editCommentAction = popupmenu->addAction( component()->comment()->description().isEmpty() ? QObject::tr( "Add Comment…" ) : QObject::tr( "Edit Comment…" ) );
  connect( editCommentAction, &QAction::triggered, this, &QgsModelParameterGraphicItem::editComment );

  popupmenu->exec( event->screenPos() );
}

QColor QgsModelParameterGraphicItem::fillColor( QgsModelComponentGraphicItem::State state ) const
{
  QColor c( 238, 242, 131 );
  switch ( state )
  {
    case Selected:
      c = c.darker( 110 );
      break;
    case Hover:
      c = c.darker( 105 );
      break;

    case Normal:
      break;
  }
  return c;
}

QColor QgsModelParameterGraphicItem::strokeColor( QgsModelComponentGraphicItem::State state ) const
{
  switch ( state )
  {
    case Selected:
      return QColor( 116, 113, 68 );
    case Hover:
    case Normal:
      return QColor( 234, 226, 118 );
  }
  return QColor();
}

QColor QgsModelParameterGraphicItem::textColor( QgsModelComponentGraphicItem::State ) const
{
  return Qt::black;
}

QPicture QgsModelParameterGraphicItem::iconPicture() const
{
  return mPicture;
}

void QgsModelParameterGraphicItem::updateStoredComponentPosition( const QPointF &pos, const QSizeF &size )
{
  if ( QgsProcessingModelParameter *param = dynamic_cast< QgsProcessingModelParameter * >( component() ) )
  {
    model()->parameterComponent( param->parameterName() ).setPosition( pos );
    model()->parameterComponent( param->parameterName() ).setSize( size );
  }
}

bool QgsModelParameterGraphicItem::canDeleteComponent()
{
  if ( const QgsProcessingModelParameter *param = dynamic_cast< const QgsProcessingModelParameter * >( component() ) )
  {
    if ( model()->childAlgorithmsDependOnParameter( param->parameterName() ) )
    {
      return false;
    }
    else if ( model()->otherParametersDependOnParameter( param->parameterName() ) )
    {
      return false;
    }
    else
    {
      return true;
    }
  }
  return false;
}

void QgsModelParameterGraphicItem::deleteComponent()
{
  if ( const QgsProcessingModelParameter *param = dynamic_cast< const QgsProcessingModelParameter * >( component() ) )
  {
    if ( model()->childAlgorithmsDependOnParameter( param->parameterName() ) )
    {
      QMessageBox::warning( nullptr, QObject::tr( "Could not remove input" ),
                            QObject::tr( "Algorithms depend on the selected input.\n"
                                         "Remove them before trying to remove it." ) );
    }
    else if ( model()->otherParametersDependOnParameter( param->parameterName() ) )
    {
      QMessageBox::warning( nullptr, QObject::tr( "Could not remove input" ),
                            QObject::tr( "Other inputs depend on the selected input.\n"
                                         "Remove them before trying to remove it." ) );
    }
    else
    {
      emit aboutToChange( tr( "Delete Input %1" ).arg( param->description() ) );
      model()->removeModelParameter( param->parameterName() );
      emit changed();
      emit requestModelRepaint();
    }
  }
}



QgsModelChildAlgorithmGraphicItem::QgsModelChildAlgorithmGraphicItem( QgsProcessingModelChildAlgorithm *child, QgsProcessingModelAlgorithm *model, QGraphicsItem *parent )
  : QgsModelComponentGraphicItem( child, model, parent )
{
  if ( child->algorithm() && !child->algorithm()->svgIconPath().isEmpty() )
  {
    QSvgRenderer svg( child->algorithm()->svgIconPath() );
    const QSizeF size = svg.defaultSize();
    QPainter painter( &mPicture );
    painter.scale( 16.0 / size.width(), 16.0 / size.width() );
    svg.render( &painter );
    painter.end();
  }
  else if ( child->algorithm() )
  {
    mPixmap = child->algorithm()->icon().pixmap( 15, 15 );
  }

  setLabel( child->description() );

  QStringList issues;
  mIsValid = model->validateChildAlgorithm( child->childId(), issues );
}

void QgsModelChildAlgorithmGraphicItem::contextMenuEvent( QGraphicsSceneContextMenuEvent *event )
{
  QMenu *popupmenu = new QMenu( event->widget() );
  QAction *removeAction = popupmenu->addAction( QObject::tr( "Remove" ) );
  connect( removeAction, &QAction::triggered, this, &QgsModelChildAlgorithmGraphicItem::deleteComponent );
  QAction *editAction = popupmenu->addAction( QObject::tr( "Edit…" ) );
  connect( editAction, &QAction::triggered, this, &QgsModelChildAlgorithmGraphicItem::editComponent );
  QAction *editCommentAction = popupmenu->addAction( component()->comment()->description().isEmpty() ? QObject::tr( "Add Comment…" ) : QObject::tr( "Edit Comment…" ) );
  connect( editCommentAction, &QAction::triggered, this, &QgsModelParameterGraphicItem::editComment );
  popupmenu->addSeparator();

  if ( const QgsProcessingModelChildAlgorithm *child = dynamic_cast< const QgsProcessingModelChildAlgorithm * >( component() ) )
  {
    if ( !child->isActive() )
    {
      QAction *activateAction = popupmenu->addAction( QObject::tr( "Activate" ) );
      connect( activateAction, &QAction::triggered, this, &QgsModelChildAlgorithmGraphicItem::activateAlgorithm );
    }
    else
    {
      QAction *deactivateAction = popupmenu->addAction( QObject::tr( "Deactivate" ) );
      connect( deactivateAction, &QAction::triggered, this, &QgsModelChildAlgorithmGraphicItem::deactivateAlgorithm );
    }
  }

  popupmenu->exec( event->screenPos() );
}

QColor QgsModelChildAlgorithmGraphicItem::fillColor( QgsModelComponentGraphicItem::State state ) const
{
  QColor c;

  if ( mIsValid )
    c = QColor( 255, 255, 255 );
  else
    c = QColor( 208, 0, 0 );

  switch ( state )
  {
    case Selected:
      c = c.darker( 110 );
      break;
    case Hover:
      c = c.darker( 105 );
      break;

    case Normal:
      break;
  }
  return c;
}

QColor QgsModelChildAlgorithmGraphicItem::strokeColor( QgsModelComponentGraphicItem::State state ) const
{
  switch ( state )
  {
    case Selected:
      return mIsValid ? QColor( 50, 50, 50 ) : QColor( 80, 0, 0 );
    case Hover:
    case Normal:
      return mIsValid ? Qt::gray : QColor( 134, 0, 0 );
  }
  return QColor();
}

QColor QgsModelChildAlgorithmGraphicItem::textColor( QgsModelComponentGraphicItem::State ) const
{
  return mIsValid ? ( qgis::down_cast< const QgsProcessingModelChildAlgorithm * >( component() )->isActive() ? Qt::black : Qt::gray ) : QColor( 255, 255, 255 );
}

QPixmap QgsModelChildAlgorithmGraphicItem::iconPixmap() const
{
  return mPixmap;
}

QPicture QgsModelChildAlgorithmGraphicItem::iconPicture() const
{
  return mPicture;
}

int QgsModelChildAlgorithmGraphicItem::linkPointCount( Qt::Edge edge ) const
{
  if ( const QgsProcessingModelChildAlgorithm *child = dynamic_cast< const QgsProcessingModelChildAlgorithm * >( component() ) )
  {
    if ( !child->algorithm() )
      return 0;

    switch ( edge )
    {
      case Qt::BottomEdge:
        return child->algorithm()->outputDefinitions().size();
      case Qt::TopEdge:
      {
        QgsProcessingParameterDefinitions params = child->algorithm()->parameterDefinitions();
        params.erase( std::remove_if( params.begin(), params.end(), []( const QgsProcessingParameterDefinition * param )
        {
          return param->flags() & QgsProcessingParameterDefinition::FlagHidden || param->isDestination();
        } ), params.end() );
        return params.size();
      }

      case Qt::LeftEdge:
      case Qt::RightEdge:
        break;
    }
  }
  return 0;
}

QString QgsModelChildAlgorithmGraphicItem::linkPointText( Qt::Edge edge, int index ) const
{
  if ( index < 0 )
    return QString();

  if ( const QgsProcessingModelChildAlgorithm *child = dynamic_cast< const QgsProcessingModelChildAlgorithm * >( component() ) )
  {
    if ( !child->algorithm() )
      return QString();

    switch ( edge )
    {
      case Qt::BottomEdge:
      {
        if ( index >= child->algorithm()->outputDefinitions().length() )
        {
          // something goes wrong and tried to link to an not existing output
          QgsMessageLog::logMessage(
            tr( "Cannot link output for child: %1" ).arg( child->algorithm()->name() ),
            "QgsModelChildAlgorithmGraphicItem", Qgis::MessageLevel::Warning, true );
          return QString();
        }

        const QgsProcessingOutputDefinition *output = child->algorithm()->outputDefinitions().at( index );
        QString title = output->description();
        if ( mResults.contains( output->name() ) )
        {
          title += QStringLiteral( ": %1" ).arg( mResults.value( output->name() ).toString() );
        }
        return truncatedTextForItem( title );
      }

      case Qt::TopEdge:
      {
        QgsProcessingParameterDefinitions params = child->algorithm()->parameterDefinitions();
        params.erase( std::remove_if( params.begin(), params.end(), []( const QgsProcessingParameterDefinition * param )
        {
          return param->flags() & QgsProcessingParameterDefinition::FlagHidden || param->isDestination();
        } ), params.end() );

        if ( index >= params.length() )
        {
          // something goes wrong and tried to link to an not existing source parameter
          QgsMessageLog::logMessage(
            tr( "Cannot link source for child: %1" ).arg( child->algorithm()->name() ),
            "QgsModelChildAlgorithmGraphicItem", Qgis::MessageLevel::Warning, true );
          return QString();
        }

        QString title = params.at( index )->description();
        if ( !mInputs.value( params.at( index )->name() ).toString().isEmpty() )
          title +=  QStringLiteral( ": %1" ).arg( mInputs.value( params.at( index )->name() ).toString() );
        return truncatedTextForItem( title );
      }

      case Qt::LeftEdge:
      case Qt::RightEdge:
        break;
    }
  }
  return QString();
}

void QgsModelChildAlgorithmGraphicItem::updateStoredComponentPosition( const QPointF &pos, const QSizeF &size )
{
  if ( QgsProcessingModelChildAlgorithm *child = dynamic_cast< QgsProcessingModelChildAlgorithm * >( component() ) )
  {
    model()->childAlgorithm( child->childId() ).setPosition( pos );
    model()->childAlgorithm( child->childId() ).setSize( size );
  }
}

bool QgsModelChildAlgorithmGraphicItem::canDeleteComponent()
{
  if ( const QgsProcessingModelChildAlgorithm *child = dynamic_cast< const QgsProcessingModelChildAlgorithm * >( component() ) )
  {
    return model()->dependentChildAlgorithms( child->childId() ).empty();
  }
  return false;
}

void QgsModelChildAlgorithmGraphicItem::setResults( const QVariantMap &results )
{
  if ( mResults == results )
    return;

  mResults = results;
  update();
  emit updateArrowPaths();
}

void QgsModelChildAlgorithmGraphicItem::setInputs( const QVariantMap &inputs )
{
  if ( mInputs == inputs )
    return;

  mInputs = inputs;
  update();
  emit updateArrowPaths();
}

void QgsModelChildAlgorithmGraphicItem::deleteComponent()
{
  if ( const QgsProcessingModelChildAlgorithm *child = dynamic_cast< const QgsProcessingModelChildAlgorithm * >( component() ) )
  {
    emit aboutToChange( tr( "Remove %1" ).arg( child->algorithm() ? child->algorithm()->displayName() : tr( "Algorithm" ) ) );
    if ( !model()->removeChildAlgorithm( child->childId() ) )
    {
      QMessageBox::warning( nullptr, QObject::tr( "Could not remove algorithm" ),
                            QObject::tr( "Other algorithms depend on the selected one.\n"
                                         "Remove them before trying to remove it." ) );
    }
    else
    {
      emit changed();
      emit requestModelRepaint();
    }
  }
}

void QgsModelChildAlgorithmGraphicItem::deactivateAlgorithm()
{
  if ( const QgsProcessingModelChildAlgorithm *child = dynamic_cast< const QgsProcessingModelChildAlgorithm * >( component() ) )
  {
    model()->deactivateChildAlgorithm( child->childId() );
    emit requestModelRepaint();
  }
}

void QgsModelChildAlgorithmGraphicItem::activateAlgorithm()
{
  if ( const QgsProcessingModelChildAlgorithm *child = dynamic_cast< const QgsProcessingModelChildAlgorithm * >( component() ) )
  {
    if ( model()->activateChildAlgorithm( child->childId() ) )
    {
      emit requestModelRepaint();
    }
    else
    {
      QMessageBox::warning( nullptr, QObject::tr( "Could not activate algorithm" ),
                            QObject::tr( "The selected algorithm depends on other currently non-active algorithms.\n"
                                         "Activate them them before trying to activate it.." ) );
    }
  }
}


QgsModelOutputGraphicItem::QgsModelOutputGraphicItem( QgsProcessingModelOutput *output, QgsProcessingModelAlgorithm *model, QGraphicsItem *parent )
  : QgsModelComponentGraphicItem( output, model, parent )
{
  QSvgRenderer svg( QgsApplication::iconPath( QStringLiteral( "mIconModelOutput.svg" ) ) );
  QPainter painter( &mPicture );
  svg.render( &painter );
  painter.end();
  setLabel( output->description() );
}

QColor QgsModelOutputGraphicItem::fillColor( QgsModelComponentGraphicItem::State state ) const
{
  QColor c( 172, 196, 114 );
  switch ( state )
  {
    case Selected:
      c = c.darker( 110 );
      break;
    case Hover:
      c = c.darker( 105 );
      break;

    case Normal:
      break;
  }
  return c;
}

QColor QgsModelOutputGraphicItem::strokeColor( QgsModelComponentGraphicItem::State state ) const
{
  switch ( state )
  {
    case Selected:
      return QColor( 42, 65, 42 );
    case Hover:
    case Normal:
      return QColor( 90, 140, 90 );
  }
  return QColor();
}

QColor QgsModelOutputGraphicItem::textColor( QgsModelComponentGraphicItem::State ) const
{
  return Qt::black;
}

QPicture QgsModelOutputGraphicItem::iconPicture() const
{
  return mPicture;
}

void QgsModelOutputGraphicItem::updateStoredComponentPosition( const QPointF &pos, const QSizeF &size )
{
  if ( QgsProcessingModelOutput *output = dynamic_cast< QgsProcessingModelOutput * >( component() ) )
  {
    model()->childAlgorithm( output->childId() ).modelOutput( output->name() ).setPosition( pos );
    model()->childAlgorithm( output->childId() ).modelOutput( output->name() ).setSize( size );
  }
}

bool QgsModelOutputGraphicItem::canDeleteComponent()
{
  if ( dynamic_cast< const QgsProcessingModelOutput * >( component() ) )
  {
    return true;
  }
  return false;
}

void QgsModelOutputGraphicItem::deleteComponent()
{
  if ( const QgsProcessingModelOutput *output = dynamic_cast< const QgsProcessingModelOutput * >( component() ) )
  {
    emit aboutToChange( tr( "Delete Output %1" ).arg( output->description() ) );
    model()->childAlgorithm( output->childId() ).removeModelOutput( output->name() );
    model()->updateDestinationParameters();
    emit changed();
    emit requestModelRepaint();
  }
}


//
// QgsModelGroupBoxGraphicItem
//

QgsModelGroupBoxGraphicItem::QgsModelGroupBoxGraphicItem( QgsProcessingModelGroupBox *box, QgsProcessingModelAlgorithm *model, QGraphicsItem *parent )
  : QgsModelComponentGraphicItem( box, model, parent )
{
  setZValue( QgsModelGraphicsScene::ZValues::GroupBox );
  setLabel( box->description() );

  QFont f = font();
  f.setBold( true );
  f.setPixelSize( 14 );
  setFont( f );
}

void QgsModelGroupBoxGraphicItem::contextMenuEvent( QGraphicsSceneContextMenuEvent *event )
{
  QMenu *popupmenu = new QMenu( event->widget() );
  QAction *removeAction = popupmenu->addAction( QObject::tr( "Remove" ) );
  connect( removeAction, &QAction::triggered, this, &QgsModelGroupBoxGraphicItem::deleteComponent );
  QAction *editAction = popupmenu->addAction( QObject::tr( "Edit…" ) );
  connect( editAction, &QAction::triggered, this, &QgsModelGroupBoxGraphicItem::editComponent );
  popupmenu->exec( event->screenPos() );
}

QgsModelGroupBoxGraphicItem::~QgsModelGroupBoxGraphicItem() = default;

QColor QgsModelGroupBoxGraphicItem::fillColor( QgsModelComponentGraphicItem::State state ) const
{
  QColor c( 230, 230, 230 );
  switch ( state )
  {
    case Selected:
      c = c.darker( 110 );
      break;
    case Hover:
      c = c.darker( 105 );
      break;

    case Normal:
      break;
  }
  return c;
}

QColor QgsModelGroupBoxGraphicItem::strokeColor( QgsModelComponentGraphicItem::State state ) const
{
  switch ( state )
  {
    case Selected:
      return QColor( 50, 50, 50 );
    case Hover:
    case Normal:
      return QColor( 150, 150, 150 );
  }
  return QColor();
}

QColor QgsModelGroupBoxGraphicItem::textColor( QgsModelComponentGraphicItem::State ) const
{
  return QColor( 100, 100, 100 );
}

Qt::PenStyle QgsModelGroupBoxGraphicItem::strokeStyle( QgsModelComponentGraphicItem::State ) const
{
  return Qt::DotLine;
}

Qt::Alignment QgsModelGroupBoxGraphicItem::titleAlignment() const
{
  return Qt::AlignHCenter;
}

void QgsModelGroupBoxGraphicItem::updateStoredComponentPosition( const QPointF &pos, const QSizeF &size )
{
  if ( QgsProcessingModelGroupBox *box = dynamic_cast< QgsProcessingModelGroupBox * >( component() ) )
  {
    box->setPosition( pos );
    box->setSize( size );
    model()->addGroupBox( *box );
  }
}

bool QgsModelGroupBoxGraphicItem::canDeleteComponent()
{
  if ( dynamic_cast< QgsProcessingModelGroupBox * >( component() ) )
  {
    return true;
  }
  return false;
}

void QgsModelGroupBoxGraphicItem::deleteComponent()
{
  if ( const QgsProcessingModelGroupBox *box = dynamic_cast< const QgsProcessingModelGroupBox * >( component() ) )
  {
    emit aboutToChange( tr( "Delete Group Box" ) );
    model()->removeGroupBox( box->uuid() );
    emit changed();
    emit requestModelRepaint();
  }
}

void QgsModelGroupBoxGraphicItem::editComponent()
{
  if ( const QgsProcessingModelGroupBox *box = dynamic_cast< const QgsProcessingModelGroupBox * >( component() ) )
  {
    QgsModelGroupBoxDefinitionDialog dlg( *box, this->scene()->views().at( 0 ) );

    if ( dlg.exec() )
    {
      emit aboutToChange( tr( "Edit Group Box" ) );
      model()->addGroupBox( dlg.groupBox() );
      emit changed();
      emit requestModelRepaint();
    }
  }
}

//
// QgsModelCommentGraphicItem
//

QgsModelCommentGraphicItem::QgsModelCommentGraphicItem( QgsProcessingModelComment *comment, QgsModelComponentGraphicItem *parentItem, QgsProcessingModelAlgorithm *model, QGraphicsItem *parent )
  : QgsModelComponentGraphicItem( comment, model, parent )
  , mParentComponent( parentItem->component()->clone() )
  , mParentItem( parentItem )
{
  setLabel( comment->description() );

  QFont f = font();
  f.setPixelSize( 9 );
  setFont( f );
}

void QgsModelCommentGraphicItem::contextMenuEvent( QGraphicsSceneContextMenuEvent *event )
{
  QMenu *popupmenu = new QMenu( event->widget() );
  QAction *removeAction = popupmenu->addAction( QObject::tr( "Remove" ) );
  connect( removeAction, &QAction::triggered, this, &QgsModelCommentGraphicItem::deleteComponent );
  QAction *editAction = popupmenu->addAction( QObject::tr( "Edit…" ) );
  connect( editAction, &QAction::triggered, this, &QgsModelCommentGraphicItem::editComponent );
  popupmenu->exec( event->screenPos() );
}

QgsModelCommentGraphicItem::~QgsModelCommentGraphicItem() = default;

QColor QgsModelCommentGraphicItem::fillColor( QgsModelComponentGraphicItem::State state ) const
{
  QColor c( 230, 230, 230 );
  switch ( state )
  {
    case Selected:
      c = c.darker( 110 );
      break;
    case Hover:
      c = c.darker( 105 );
      break;

    case Normal:
      break;
  }
  return c;
}

QColor QgsModelCommentGraphicItem::strokeColor( QgsModelComponentGraphicItem::State state ) const
{
  switch ( state )
  {
    case Selected:
      return QColor( 50, 50, 50 );
    case Hover:
    case Normal:
      return QColor( 150, 150, 150 );
  }
  return QColor();
}

QColor QgsModelCommentGraphicItem::textColor( QgsModelComponentGraphicItem::State ) const
{
  return QColor( 100, 100, 100 );
}

Qt::PenStyle QgsModelCommentGraphicItem::strokeStyle( QgsModelComponentGraphicItem::State ) const
{
  return Qt::DotLine;
}

void QgsModelCommentGraphicItem::updateStoredComponentPosition( const QPointF &pos, const QSizeF &size )
{
  if ( QgsProcessingModelComment *comment = modelComponent() )
  {
    comment->setPosition( pos );
    comment->setSize( size );
  }
}

bool QgsModelCommentGraphicItem::canDeleteComponent()
{
  if ( modelComponent() )
  {
    return true;
  }
  return false;
}

void QgsModelCommentGraphicItem::deleteComponent()
{
  if ( QgsProcessingModelComment *comment = modelComponent() )
  {
    emit aboutToChange( tr( "Delete Comment" ) );
    comment->setDescription( QString() );
    emit changed();
    emit requestModelRepaint();
  }
}

void QgsModelCommentGraphicItem::editComponent()
{
  if ( mParentItem )
  {
    mParentItem->editComment();
  }
}

QgsProcessingModelComment *QgsModelCommentGraphicItem::modelComponent()
{
  if ( const QgsProcessingModelChildAlgorithm *child = dynamic_cast< const QgsProcessingModelChildAlgorithm * >( mParentComponent.get() ) )
  {
    return model()->childAlgorithm( child->childId() ).comment();
  }
  else if ( const QgsProcessingModelParameter *param = dynamic_cast< const QgsProcessingModelParameter * >( mParentComponent.get() ) )
  {
    return model()->parameterComponent( param->parameterName() ).comment();
  }
  else if ( const QgsProcessingModelOutput *output = dynamic_cast< const QgsProcessingModelOutput * >( mParentComponent.get() ) )
  {
    return model()->childAlgorithm( output->childId() ).modelOutput( output->name() ).comment();
  }
  return nullptr;
}

QgsModelComponentGraphicItem *QgsModelCommentGraphicItem::parentComponentItem() const
{
  return mParentItem;
}


///@endcond

/***************************************************************************
                             qgsmodelarrowitem.cpp
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

#include "qgsmodelarrowitem.h"

#include <math.h>

#include "qgsapplication.h"
#include "qgsmodelcomponentgraphicitem.h"
#include "qgsmodelgraphicitem.h"
#include "qgsmodelgraphicsscene.h"

#include <QApplication>
#include <QPainter>
#include <QPalette>

#include "moc_qgsmodelarrowitem.cpp"

///@cond NOT_STABLE

//
// QgsModelDesignerArrowBadgeItem
//

QgsModelDesignerArrowBadgeItem::QgsModelDesignerArrowBadgeItem( QgsModelArrowItem *link )
  : QGraphicsRectItem( link )
{
  setZValue( QgsModelGraphicsScene::ZValues::ArrowDecoration );
}

void QgsModelDesignerArrowBadgeItem::setCenter( const QPointF &center )
{
  const double width = rect().width();
  const double height = rect().height();
  setRect( center.x() - width * 0.5, center.y() - height * 0.5, width, height );
}

QgsModelArrowItem *QgsModelDesignerArrowBadgeItem::arrow()
{
  return dynamic_cast< QgsModelArrowItem * >( parentItem() );
}

void QgsModelDesignerArrowBadgeItem::paint( QPainter *painter, const QStyleOptionGraphicsItem *, QWidget * )
{
  QgsModelArrowItem *arrow = QgsModelDesignerArrowBadgeItem::arrow();
  if ( !arrow )
    return;

  // find mid-point color for arrow, and match badge to mid-point color
  const QColor startColor = arrow->startItem()->linkColor( arrow->startEdge(), arrow->startIndex() );
  const QColor endColor = arrow->endItem()->linkColor( arrow->endEdge(), arrow->endIndex() );
  const QColor backgroundColor = QColor::fromRgbF( 0.5f * ( startColor.redF() + endColor.redF() ), 0.5f * ( startColor.greenF() + endColor.greenF() ), 0.5f * ( startColor.blueF() + endColor.blueF() ) );
  const QColor strokeColor = backgroundColor.darker( 150 );

  const bool hadAntialiasing = painter->testRenderHint( QPainter::Antialiasing );
  painter->setRenderHint( QPainter::Antialiasing, true );
  // First draw a rounded rectangle as background
  painter->setBrush( QBrush( backgroundColor ) );
  QPen pen( strokeColor );
  pen.setCosmetic( true );
  pen.setWidth( 1 );
  painter->setPen( pen );
  painter->drawRoundedRect( rect(), BORDER_RADIUS, BORDER_RADIUS );

  const bool isDarkBackground = backgroundColor.lightness() < 128;

  // And finally draw the text on top
  QFont font;
  font.setPointSize( FONT_SIZE );
  font.setBold( true );
  painter->setFont( font );
  painter->setPen( QPen( isDarkBackground ? QColor( 255, 255, 255 ) : QColor( 0, 0, 0 ) ) );
  painter->setBrush( Qt::NoBrush );

  painter->drawText( rect(), Qt::AlignCenter, textForValue( mValue ) );

  painter->setRenderHint( QPainter::Antialiasing, hadAntialiasing );
}

void QgsModelDesignerArrowBadgeItem::setValue( const QVariant &value )
{
  mValue = value;
  resizeToContents();
  update();
}

QVariant QgsModelDesignerArrowBadgeItem::value() const
{
  return mValue;
}

QString QgsModelDesignerArrowBadgeItem::textForValue( const QVariant &value )
{
  if ( QgsVariantUtils::isNull( value ) )
    return QString();

  if ( QgsVariantUtils::isNumericType( static_cast< QMetaType::Type>( value.userType() ) ) )
  {
    return value.toString();
  }

  // limit size of badge
  const QString stringValue = value.toString();
  return QgsStringUtils::truncateMiddleOfString( stringValue, 10 );
}

void QgsModelDesignerArrowBadgeItem::resizeToContents()
{
  QFont font;
  font.setPointSize( FONT_SIZE );
  font.setBold( true );
  QFontMetrics fm( font );
  const QRectF boundingRect = fm.boundingRect( textForValue( mValue ) );
  const double width = boundingRect.width() + 2 * BORDER_RADIUS + CONTENTS_MARGIN * 2;
  const double height = boundingRect.height() + 2 * BORDER_RADIUS + CONTENTS_MARGIN * 2;

  const QPointF center = rect().center();
  setRect( center.x() - width * 0.5, center.y() - height * 0.5, width, height );
}


//
// QgsModelArrowItem
//

QgsModelArrowItem::QgsModelArrowItem(
  QgsModelComponentGraphicItem *startItem,
  Qt::Edge startEdge,
  int startIndex,
  bool startIsOutgoing,
  Marker startMarker,
  QgsModelComponentGraphicItem *endItem,
  Qt::Edge endEdge,
  int endIndex,
  bool endIsIncoming,
  Marker endMarker
)
  : QObject( nullptr )
  , mStartItem( startItem )
  , mStartEdge( startEdge )
  , mStartIndex( startIndex )
  , mStartIsOutgoing( startIsOutgoing )
  , mStartMarker( startMarker )
  , mEndItem( endItem )
  , mEndEdge( endEdge )
  , mEndIndex( endIndex )
  , mEndIsIncoming( endIsIncoming )
  , mEndMarker( endMarker )
{
  setCacheMode( QGraphicsItem::DeviceCoordinateCache );
  setFlag( QGraphicsItem::ItemIsSelectable, false );
  mColor = QApplication::palette().color( QPalette::Text );
  mColor.setAlpha( 150 );
  setPen( QPen( mColor, 8, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin ) );
  setZValue( QgsModelGraphicsScene::ArrowLink );
  updatePath();

  connect( mStartItem, &QgsModelComponentGraphicItem::updateArrowPaths, this, &QgsModelArrowItem::updatePath );
  connect( mStartItem, &QgsModelComponentGraphicItem::repaintArrows, this, [this] { update(); } );
  connect( mEndItem, &QgsModelComponentGraphicItem::updateArrowPaths, this, &QgsModelArrowItem::updatePath );
  connect( mEndItem, &QgsModelComponentGraphicItem::repaintArrows, this, [this] { update(); } );
}

QgsModelArrowItem::QgsModelArrowItem( QgsModelComponentGraphicItem *startItem, Qt::Edge startEdge, int startIndex, Marker startMarker, QgsModelComponentGraphicItem *endItem, Marker endMarker )
  : QgsModelArrowItem( startItem, startEdge, startIndex, true, startMarker, endItem, Qt::LeftEdge, -1, true, endMarker )
{}

QgsModelArrowItem::QgsModelArrowItem( QgsModelComponentGraphicItem *startItem, Marker startMarker, QgsModelComponentGraphicItem *endItem, Qt::Edge endEdge, int endIndex, Marker endMarker )
  : QgsModelArrowItem( startItem, Qt::LeftEdge, -1, true, startMarker, endItem, endEdge, endIndex, true, endMarker )
{}

QgsModelArrowItem::QgsModelArrowItem( QgsModelComponentGraphicItem *startItem, Marker startMarker, QgsModelComponentGraphicItem *endItem, Marker endMarker )
  : QgsModelArrowItem( startItem, Qt::LeftEdge, -1, true, startMarker, endItem, Qt::LeftEdge, -1, true, endMarker )
{}


void QgsModelArrowItem::paint( QPainter *painter, const QStyleOptionGraphicsItem *, QWidget * )
{
  QColor color = mStartItem->linkColor( mStartEdge, mStartIndex );

  if ( mStartItem->state() == QgsModelComponentGraphicItem::Selected || mEndItem->state() == QgsModelComponentGraphicItem::Selected )
    color.setAlpha( 220 );
  else if ( mStartItem->state() == QgsModelComponentGraphicItem::Hover || mEndItem->state() == QgsModelComponentGraphicItem::Hover )
    color.setAlpha( 150 );
  else
    color.setAlpha( 80 );

  //
  QPen p = pen();
  p.setColor( color );
  p.setWidth( 0 );
  painter->setPen( p );

  painter->setBrush( color );
  painter->setRenderHint( QPainter::Antialiasing );

  switch ( mStartMarker )
  {
    case Marker::ArrowHead:
      drawArrowHead( painter, mStartPoint, path().pointAtPercent( 0.0 ) - path().pointAtPercent( 0.05 ) );
      break;

    // start marker are no longer drawn
    case Marker::Circle:
    case Marker::NoMarker:
      break;
  }

  switch ( mEndMarker )
  {
    case Marker::Circle:
      painter->drawEllipse( mEndPoint, 3.0, 3.0 );
      break;
    case Marker::ArrowHead:
      drawArrowHead( painter, mEndPoint, path().pointAtPercent( 1.0 ) - path().pointAtPercent( 0.95 ) );
      break;
    case Marker::NoMarker:
      break;
  }

  painter->setBrush( color );
  painter->setRenderHint( QPainter::Antialiasing );
  painter->setBrush( Qt::NoBrush );

  // Set the painter back to regular stroke thickness
  p = pen();
  QColor endColor = mEndItem->linkColor( mEndEdge, mEndIndex );
  color.setAlpha( 255 );

  QLinearGradient gradient;
  QPointF startPoint = path().pointAtPercent( 0.3 );
  QPointF endPoint = path().pointAtPercent( 0.7 );
  gradient.setStart( startPoint );
  gradient.setFinalStop( endPoint );
  gradient.setColorAt( 0, color );
  gradient.setColorAt( 1, endColor );

  p.setBrush( QBrush( gradient ) );
  p.setWidth( 2 );
  painter->setPen( p );
  painter->drawPath( path() );
}

void QgsModelArrowItem::drawArrowHead( QPainter *painter, const QPointF &position, const QPointF &vector )
{
  const float angle = atan2( vector.y(), vector.x() ) * 180.0 / M_PI;
  painter->translate( position );
  painter->rotate( angle );
  QPolygonF arrowHead;
  arrowHead << QPointF( 0, 0 ) << QPointF( -6, 4 ) << QPointF( -6, -4 ) << QPointF( 0, 0 );
  painter->drawPolygon( arrowHead );
  painter->rotate( -angle );
  painter->translate( -position );
}

void QgsModelArrowItem::setPenStyle( Qt::PenStyle style )
{
  QPen p = pen();
  p.setStyle( style );
  setPen( p );
  update();
}

QgsModelComponentGraphicItem *QgsModelArrowItem::startItem()
{
  return mStartItem;
}

QgsModelComponentGraphicItem *QgsModelArrowItem::endItem()
{
  return mEndItem;
}

QgsModelDesignerArrowBadgeItem *QgsModelArrowItem::badgeItem()
{
  return mBadgeItem;
}

void QgsModelArrowItem::setShowBadge( bool visible )
{
  if ( visible && !mBadgeItem )
  {
    mBadgeItem = new QgsModelDesignerArrowBadgeItem( this );
    mBadgeItem->setCenter( path().pointAtPercent( 0.5 ) );
  }
  else if ( !visible && mBadgeItem )
  {
    scene()->removeItem( mBadgeItem );
    delete mBadgeItem;
    mBadgeItem = nullptr;
  }
}

void QgsModelArrowItem::setDataViewerButton( const QString &childId, const QString &paramOrOutputName )
{
  if ( mDataViewerButton )
  {
    scene()->removeItem( mDataViewerButton );
    delete mDataViewerButton;
    mDataViewerButton = nullptr;
  }

  QgsModelDesignerDataViewerButtonGraphicItem *openDataViewerButton = new QgsModelDesignerDataViewerButtonGraphicItem( this );
  mDataViewerButton = openDataViewerButton;
  if ( mDataViewerButton )
  {
    mDataViewerButton->setPosition();
    connect( openDataViewerButton, &QgsModelDesignerDataViewerButtonGraphicItem::clicked, this, [this, childId = childId, paramOrOutputName = paramOrOutputName] {
      emit showDataViewerDock( childId, paramOrOutputName );
    } );
  }
}


void QgsModelArrowItem::updatePath()
{
  QList<QPointF> controlPoints;

  // is there a fixed start or end point?
  QPointF startPt;
  bool hasStartPt = false;

  // usually arrows attached to an algorithm have a concept of directional flow -- they are either
  // "inputs" to the item or "outputs". In this case we need to reflect this in how we draw the linking
  // arrows, because we always have "inputs" on the left/top side and "outputs" on the right/bottom
  bool startHasSpecificDirectionalFlow = qobject_cast<QgsModelChildAlgorithmGraphicItem *>( mStartItem );
  bool endHasSpecificDirectionalFlow = qobject_cast<QgsModelChildAlgorithmGraphicItem *>( mEndItem );

  // some specific exceptions to the above
  if ( qobject_cast<QgsModelCommentGraphicItem *>( mStartItem ) || qobject_cast<QgsModelCommentGraphicItem *>( mEndItem ) )
  {
    // comments can be freely attached to any side of an algorithm item without directional flow
    startHasSpecificDirectionalFlow = false;
    endHasSpecificDirectionalFlow = false;
  }

  if ( mStartIndex != -1 )
  {
    startPt = mStartItem->linkPoint( mStartEdge, mStartIndex, !mStartIsOutgoing );
    hasStartPt = true;
  }
  QPointF endPt;
  bool hasEndPt = false;
  if ( mEndIndex != -1 )
  {
    endPt = mEndItem->linkPoint( mEndEdge, mEndIndex, mEndIsIncoming );
    hasEndPt = true;
  }

  if ( !hasStartPt )
  {
    Qt::Edge startEdge;
    QPointF pt;
    if ( !hasEndPt )
      pt = mStartItem->calculateAutomaticLinkPoint( mEndItem, startEdge );
    else
      pt = mStartItem->calculateAutomaticLinkPoint( endPt + mEndItem->pos(), startEdge );

    controlPoints.append( pt );
    mStartPoint = pt;
    controlPoints.append( bezierPointForCurve( pt, startEdge, !mStartIsOutgoing, startHasSpecificDirectionalFlow ) );
  }
  else
  {
    mStartPoint = mStartItem->pos() + startPt;
    controlPoints.append( mStartItem->pos() + startPt );
    controlPoints.append( bezierPointForCurve( mStartItem->pos() + startPt, mStartEdge == Qt::BottomEdge ? Qt::RightEdge : Qt::LeftEdge, !mStartIsOutgoing, startHasSpecificDirectionalFlow ) );
  }

  if ( !hasEndPt )
  {
    Qt::Edge endEdge;
    QPointF pt;
    if ( !hasStartPt )
      pt = mEndItem->calculateAutomaticLinkPoint( mStartItem, endEdge );
    else
      pt = mEndItem->calculateAutomaticLinkPoint( startPt + mStartItem->pos(), endEdge );

    controlPoints.append( bezierPointForCurve( pt, endEdge, mEndIsIncoming, endHasSpecificDirectionalFlow ) );
    controlPoints.append( pt );
    mEndPoint = pt;
  }
  else
  {
    mEndPoint = mEndItem->pos() + endPt;
    controlPoints.append( bezierPointForCurve( mEndItem->pos() + endPt, mEndEdge == Qt::BottomEdge ? Qt::RightEdge : Qt::LeftEdge, mEndIsIncoming, endHasSpecificDirectionalFlow ) );
    controlPoints.append( mEndItem->pos() + endPt );
  }

  QPainterPath path;
  path.moveTo( controlPoints.at( 0 ) );
  path.cubicTo( controlPoints.at( 1 ), controlPoints.at( 2 ), controlPoints.at( 3 ) );
  setPath( path );
  if ( mBadgeItem )
  {
    mBadgeItem->setCenter( path.pointAtPercent( 0.5 ) );
  }

  if ( mDataViewerButton )
  {
    mDataViewerButton->setPosition();
  }
}

QPointF QgsModelArrowItem::bezierPointForCurve( const QPointF &point, Qt::Edge edge, bool incoming, bool hasSpecificDirectionalFlow ) const
{
  switch ( edge )
  {
    case Qt::LeftEdge:
      return point + QPointF( hasSpecificDirectionalFlow ? ( incoming ? -50 : 50 ) : -50, 0 );

    case Qt::RightEdge:
      return point + QPointF( hasSpecificDirectionalFlow ? ( incoming ? -50 : 50 ) : 50, 0 );

    case Qt::TopEdge:
      return point + QPointF( 0, hasSpecificDirectionalFlow ? ( incoming ? -30 : 30 ) : -30 );

    case Qt::BottomEdge:
      return point + QPointF( 0, hasSpecificDirectionalFlow ? ( incoming ? -30 : 30 ) : 30 );
  }
  return QPointF();
}


///@endcond

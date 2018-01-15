/***************************************************************************
                         qgscomposerarrow.cpp
                         ----------------------
    begin                : November 2009
    copyright            : (C) 2009 by Marco Hugentobler
    email                : marco@hugis.net
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgscomposerarrow.h"
#include "qgscomposition.h"
#include "qgscomposerutils.h"
#include "qgssymbollayerutils.h"
#include "qgssvgcache.h"
#include "qgsmapsettings.h"
#include "qgspathresolver.h"
#include "qgsreadwritecontext.h"
#include <QPainter>
#include <QSvgRenderer>
#include <QVector2D>

#include <cmath>

QgsComposerArrow::QgsComposerArrow( QgsComposition *c )
  : QgsComposerItem( c )
  , mStartPoint( 0, 0 )
  , mStopPoint( 0, 0 )
  , mStartXIdx( 0 )
  , mStartYIdx( 0 )
  , mMarkerMode( DefaultMarker )
  , mArrowHeadStrokeWidth( 1.0 )
  , mArrowHeadStrokeColor( Qt::black )
  , mArrowHeadFillColor( Qt::black )
  , mBoundsBehavior( 24 )

{
  init();
}

QgsComposerArrow::QgsComposerArrow( QPointF startPoint, QPointF stopPoint, QgsComposition *c )
  : QgsComposerItem( c )
  , mStartPoint( startPoint )
  , mStopPoint( stopPoint )
  , mMarkerMode( DefaultMarker )
  , mArrowHeadStrokeWidth( 1.0 )
  , mArrowHeadStrokeColor( Qt::black )
  , mArrowHeadFillColor( Qt::black )
  , mBoundsBehavior( 24 )

{
  mStartXIdx = mStopPoint.x() < mStartPoint.x();
  mStartYIdx = mStopPoint.y() < mStartPoint.y();
  init();
  adaptItemSceneRect();
}

QgsComposerArrow::~QgsComposerArrow()
{
  delete mLineSymbol;
}

void QgsComposerArrow::init()
{
  setArrowHeadWidth( 4 );
  mPen.setColor( mArrowHeadStrokeColor );
  mPen.setWidthF( 1 );
  mBrush.setColor( mArrowHeadFillColor );
  createDefaultLineSymbol();

  //default to no background
  setBackgroundEnabled( false );
}


void QgsComposerArrow::createDefaultLineSymbol()
{
  delete mLineSymbol;
  QgsStringMap properties;
  properties.insert( QStringLiteral( "color" ), QStringLiteral( "0,0,0,255" ) );
  properties.insert( QStringLiteral( "width" ), QStringLiteral( "1" ) );
  properties.insert( QStringLiteral( "capstyle" ), QStringLiteral( "square" ) );
  mLineSymbol = QgsLineSymbol::createSimple( properties );
}

void QgsComposerArrow::paint( QPainter *painter, const QStyleOptionGraphicsItem *itemStyle, QWidget *pWidget )
{
  Q_UNUSED( itemStyle );
  Q_UNUSED( pWidget );
  if ( !painter || !painter->device() )
  {
    return;
  }
  if ( !shouldDrawItem() )
  {
    return;
  }

  drawBackground( painter );

  painter->save();
  //antialiasing on
  painter->setRenderHint( QPainter::Antialiasing, true );

  //draw line section
  drawLine( painter );

  //draw arrowhead if required
  if ( mMarkerMode != NoMarker )
  {
    painter->setBrush( mBrush );
    painter->setPen( mPen );

    if ( mMarkerMode == DefaultMarker )
    {
      drawHardcodedMarker( painter, EndMarker );
    }
    else if ( mMarkerMode == SVGMarker )
    {
      drawSVGMarker( painter, StartMarker, mStartMarkerFile );
      drawSVGMarker( painter, EndMarker, mEndMarkerFile );
    }
  }

  painter->restore();

  drawFrame( painter );
  if ( isSelected() )
  {
    drawSelectionBoxes( painter );
  }
}

void QgsComposerArrow::setSceneRect( const QRectF &rectangle )
{
  //update rect for data defined size and position
  QRectF evaluatedRect = evalItemRect( rectangle );

  if ( evaluatedRect.width() < 0 )
  {
    mStartXIdx = 1 - mStartXIdx;
  }
  if ( evaluatedRect.height() < 0 )
  {
    mStartYIdx = 1 - mStartYIdx;
  }

  double margin = computeMarkerMargin();

  // Ensure the rectangle is at least as large as needed to include the markers
  QRectF rect = rectangle.united( QRectF( evaluatedRect.x(), evaluatedRect.y(), 2. * margin, 2. * margin ) );

  // Compute new start and stop positions
  double x[2] = {rect.x(), rect.x() + rect.width()};
  double y[2] = {rect.y(), rect.y() + rect.height()};

  double xsign = x[mStartXIdx] < x[1 - mStartXIdx] ? 1.0 : -1.0;
  double ysign = y[mStartYIdx] < y[1 - mStartYIdx] ? 1.0 : -1.0;

  mStartPoint = QPointF( x[mStartXIdx] + xsign * margin, y[mStartYIdx] + ysign * margin );
  mStopPoint = QPointF( x[1 - mStartXIdx] - xsign * margin, y[1 - mStartYIdx] - ysign * margin );

  QgsComposerItem::setSceneRect( rect );
}

void QgsComposerArrow::drawLine( QPainter *painter )
{
  if ( ! mLineSymbol || ! mComposition )
  {
    return;
  }

  QPaintDevice *paintDevice = painter->device();
  painter->save();
  //setup painter scaling to dots so that raster symbology is drawn to scale
  double dotsPerMM = paintDevice->logicalDpiX() / 25.4;
  painter->scale( 1 / dotsPerMM, 1 / dotsPerMM ); //scale painter from mm to dots

  //setup render context
  QgsRenderContext context = QgsComposerUtils::createRenderContextForComposition( mComposition, painter );
  context.setForceVectorOutput( true );

  QgsExpressionContext expressionContext = createExpressionContext();
  context.setExpressionContext( expressionContext );

  //line scaled to dots
  QPolygonF line;
  line << QPointF( mStartPoint.x() - pos().x(), mStartPoint.y() - pos().y() ) * dotsPerMM
       << QPointF( mStopPoint.x() - pos().x(), mStopPoint.y() - pos().y() ) * dotsPerMM;

  mLineSymbol->startRender( context );
  mLineSymbol->renderPolyline( line, nullptr, context );
  mLineSymbol->stopRender( context );
  painter->restore();

}

void QgsComposerArrow::drawHardcodedMarker( QPainter *p, MarkerType type )
{
  Q_UNUSED( type );
  if ( mBoundsBehavior == 22 )
  {
    //if arrow was created in versions prior to 2.4, use the old rendering style
    QgsComposerUtils::drawArrowHead( p, mStopPoint.x() - pos().x(), mStopPoint.y() - pos().y(), QgsComposerUtils::angle( mStartPoint, mStopPoint ), mArrowHeadWidth );
  }
  else
  {
    QVector2D dir = QVector2D( mStopPoint - mStartPoint ).normalized();
    QPointF stop = mStopPoint + ( dir * 0.5 * mArrowHeadWidth ).toPointF();
    QgsComposerUtils::drawArrowHead( p, stop.x() - pos().x(), stop.y() - pos().y(), QgsComposerUtils::angle( mStartPoint, stop ), mArrowHeadWidth );
  }
}

void QgsComposerArrow::drawSVGMarker( QPainter *p, MarkerType type, const QString &markerPath )
{
  Q_UNUSED( markerPath );
  double angle = QgsComposerUtils::angle( mStartPoint, mStopPoint );

  double arrowHeadHeight;
  if ( type == StartMarker )
  {
    arrowHeadHeight = mStartArrowHeadHeight;
  }
  else
  {
    arrowHeadHeight = mStopArrowHeadHeight;
  }
  if ( mArrowHeadWidth <= 0 || arrowHeadHeight <= 0 )
  {
    //bad image size
    return;
  }

  QPointF imageFixPoint;
  imageFixPoint.setX( mArrowHeadWidth / 2.0 );
  QPointF canvasPoint;
  if ( type == StartMarker )
  {
    canvasPoint = QPointF( mStartPoint.x() - pos().x(), mStartPoint.y() - pos().y() );
    imageFixPoint.setY( mStartArrowHeadHeight );
  }
  else //end marker
  {
    canvasPoint = QPointF( mStopPoint.x() - pos().x(), mStopPoint.y() - pos().y() );
    imageFixPoint.setY( 0 );
  }

  QString svgFileName = ( type == StartMarker ? mStartMarkerFile : mEndMarkerFile );
  if ( svgFileName.isEmpty() )
    return;

  QSvgRenderer r;
  const QByteArray &svgContent = QgsApplication::svgCache()->svgContent( svgFileName, mArrowHeadWidth, mArrowHeadFillColor, mArrowHeadStrokeColor, mArrowHeadStrokeWidth,
                                 1.0 );
  r.load( svgContent );

  p->save();
  p->setRenderHint( QPainter::Antialiasing );
  if ( mBoundsBehavior == 22 )
  {
    //if arrow was created in versions prior to 2.4, use the old rendering style
    //rotate image fix point for backtransform
    QPointF fixPoint;
    if ( type == StartMarker )
    {
      fixPoint.setX( 0 );
      fixPoint.setY( arrowHeadHeight / 2.0 );
    }
    else
    {
      fixPoint.setX( 0 );
      fixPoint.setY( -arrowHeadHeight / 2.0 );
    }
    QPointF rotatedFixPoint;
    double angleRad = angle / 180 * M_PI;
    rotatedFixPoint.setX( fixPoint.x() * std::cos( angleRad ) + fixPoint.y() * -sin( angleRad ) );
    rotatedFixPoint.setY( fixPoint.x() * std::sin( angleRad ) + fixPoint.y() * std::cos( angleRad ) );
    p->translate( canvasPoint.x() - rotatedFixPoint.x(), canvasPoint.y() - rotatedFixPoint.y() );
  }
  else
  {
    p->translate( canvasPoint.x(), canvasPoint.y() );
  }

  p->rotate( angle );
  p->translate( -mArrowHeadWidth / 2.0, -arrowHeadHeight / 2.0 );
  r.render( p, QRectF( 0, 0, mArrowHeadWidth, arrowHeadHeight ) );
  p->restore();
}

void QgsComposerArrow::setStartMarker( const QString &svgPath )
{
  QSvgRenderer r;
  mStartMarkerFile = svgPath;
  if ( svgPath.isEmpty() || !r.load( svgPath ) )
  {
    mStartArrowHeadHeight = 0;
  }
  else
  {
    //calculate mArrowHeadHeight from svg file and mArrowHeadWidth
    QRect viewBox = r.viewBox();
    mStartArrowHeadHeight = mArrowHeadWidth / viewBox.width() * viewBox.height();
  }
  adaptItemSceneRect();
}

void QgsComposerArrow::setEndMarker( const QString &svgPath )
{
  QSvgRenderer r;
  mEndMarkerFile = svgPath;
  if ( svgPath.isEmpty() || !r.load( svgPath ) )
  {
    mStopArrowHeadHeight = 0;
  }
  else
  {
    //calculate mArrowHeadHeight from svg file and mArrowHeadWidth
    QRect viewBox = r.viewBox();
    mStopArrowHeadHeight = mArrowHeadWidth / viewBox.width() * viewBox.height();
  }
  adaptItemSceneRect();
}

void QgsComposerArrow::setArrowHeadStrokeColor( const QColor &color )
{
  mArrowHeadStrokeColor = color;
  mPen.setColor( color );
}

void QgsComposerArrow::setArrowHeadFillColor( const QColor &color )
{
  mArrowHeadFillColor = color;
  mBrush.setColor( color );
}

void QgsComposerArrow::setArrowHeadStrokeWidth( const double width )
{
  mArrowHeadStrokeWidth = width;
  mPen.setWidthF( mArrowHeadStrokeWidth );

  adaptItemSceneRect();
}

void QgsComposerArrow::setLineSymbol( QgsLineSymbol *symbol )
{
  delete mLineSymbol;
  mLineSymbol = symbol;
}

void QgsComposerArrow::setArrowHeadWidth( double width )
{
  mArrowHeadWidth = width;
  setStartMarker( mStartMarkerFile );
  setEndMarker( mEndMarkerFile );
  adaptItemSceneRect();
}

double QgsComposerArrow::computeMarkerMargin() const
{
  double margin = 0;

  if ( mBoundsBehavior == 22 )
  {
    //if arrow was created in versions prior to 2.4, use the old rendering style
    if ( mMarkerMode == DefaultMarker )
    {
      margin = mPen.widthF() / 2.0 + mArrowHeadWidth / 2.0;
    }
    else if ( mMarkerMode == NoMarker )
    {
      margin = mPen.widthF() / 2.0;
    }
    else if ( mMarkerMode == SVGMarker )
    {
      double maxArrowHeight = std::max( mStartArrowHeadHeight, mStopArrowHeadHeight );
      margin = mPen.widthF() / 2 + std::max( mArrowHeadWidth / 2.0, maxArrowHeight / 2.0 );
    }
  }
  else
  {
    if ( mMarkerMode == DefaultMarker )
    {
      margin = mPen.widthF() * M_SQRT1_2 + mArrowHeadWidth / 2.0;
    }
    else if ( mMarkerMode == NoMarker )
    {
      margin = mPen.widthF() * M_SQRT1_2;
    }
    else if ( mMarkerMode == SVGMarker )
    {
      double startMarkerMargin = std::sqrt( 0.25 * ( mStartArrowHeadHeight * mStartArrowHeadHeight + mArrowHeadWidth * mArrowHeadWidth ) );
      double stopMarkerMargin = std::sqrt( 0.25 * ( mStopArrowHeadHeight * mStopArrowHeadHeight + mArrowHeadWidth * mArrowHeadWidth ) );
      double markerMargin = std::max( startMarkerMargin, stopMarkerMargin );
      margin = std::max( mPen.widthF() * M_SQRT1_2, markerMargin );
    }
  }
  return margin;
}

void QgsComposerArrow::adaptItemSceneRect()
{
  //rectangle containing start and end point
  QRectF rect = QRectF( std::min( mStartPoint.x(), mStopPoint.x() ), std::min( mStartPoint.y(), mStopPoint.y() ),
                        std::fabs( mStopPoint.x() - mStartPoint.x() ), std::fabs( mStopPoint.y() - mStartPoint.y() ) );
  double enlarge = computeMarkerMargin();
  rect.adjust( -enlarge, -enlarge, enlarge, enlarge );
  QgsComposerItem::setSceneRect( rect );
}

void QgsComposerArrow::setMarkerMode( MarkerMode mode )
{
  mMarkerMode = mode;
  adaptItemSceneRect();
}

bool QgsComposerArrow::writeXml( QDomElement &elem, QDomDocument &doc ) const
{
  QgsPathResolver pathResolver;
  if ( mComposition )
    pathResolver = mComposition->project()->pathResolver();

  QgsReadWriteContext context;
  context.setPathResolver( pathResolver );
  // absolute paths to relative
  QString startMarkerPath = QgsSymbolLayerUtils::svgSymbolPathToName( mStartMarkerFile, pathResolver );
  QString endMarkerPath = QgsSymbolLayerUtils::svgSymbolPathToName( mEndMarkerFile, pathResolver );

  QDomElement composerArrowElem = doc.createElement( QStringLiteral( "ComposerArrow" ) );
  composerArrowElem.setAttribute( QStringLiteral( "arrowHeadWidth" ), QString::number( mArrowHeadWidth ) );
  composerArrowElem.setAttribute( QStringLiteral( "arrowHeadFillColor" ), QgsSymbolLayerUtils::encodeColor( mArrowHeadFillColor ) );
  composerArrowElem.setAttribute( QStringLiteral( "arrowHeadOutlineColor" ), QgsSymbolLayerUtils::encodeColor( mArrowHeadStrokeColor ) );
  composerArrowElem.setAttribute( QStringLiteral( "outlineWidth" ), QString::number( mArrowHeadStrokeWidth ) );
  composerArrowElem.setAttribute( QStringLiteral( "markerMode" ), mMarkerMode );
  composerArrowElem.setAttribute( QStringLiteral( "startMarkerFile" ), startMarkerPath );
  composerArrowElem.setAttribute( QStringLiteral( "endMarkerFile" ), endMarkerPath );
  composerArrowElem.setAttribute( QStringLiteral( "boundsBehaviorVersion" ), QString::number( mBoundsBehavior ) );

  QDomElement styleElem = doc.createElement( QStringLiteral( "lineStyle" ) );
  QDomElement lineStyleElem = QgsSymbolLayerUtils::saveSymbol( QString(), mLineSymbol, doc, context );
  styleElem.appendChild( lineStyleElem );
  composerArrowElem.appendChild( styleElem );

  //start point
  QDomElement startPointElem = doc.createElement( QStringLiteral( "StartPoint" ) );
  startPointElem.setAttribute( QStringLiteral( "x" ), QString::number( mStartPoint.x() ) );
  startPointElem.setAttribute( QStringLiteral( "y" ), QString::number( mStartPoint.y() ) );
  composerArrowElem.appendChild( startPointElem );

  //stop point
  QDomElement stopPointElem = doc.createElement( QStringLiteral( "StopPoint" ) );
  stopPointElem.setAttribute( QStringLiteral( "x" ), QString::number( mStopPoint.x() ) );
  stopPointElem.setAttribute( QStringLiteral( "y" ), QString::number( mStopPoint.y() ) );
  composerArrowElem.appendChild( stopPointElem );

  elem.appendChild( composerArrowElem );
  return _writeXml( composerArrowElem, doc );
}

bool QgsComposerArrow::readXml( const QDomElement &itemElem, const QDomDocument &doc )
{
  QgsPathResolver pathResolver;
  if ( mComposition )
    pathResolver = mComposition->project()->pathResolver();

  QgsReadWriteContext context;
  context.setPathResolver( pathResolver );

  mArrowHeadWidth = itemElem.attribute( QStringLiteral( "arrowHeadWidth" ), QStringLiteral( "2.0" ) ).toDouble();
  mArrowHeadFillColor = QgsSymbolLayerUtils::decodeColor( itemElem.attribute( QStringLiteral( "arrowHeadFillColor" ), QStringLiteral( "0,0,0,255" ) ) );
  mArrowHeadStrokeColor = QgsSymbolLayerUtils::decodeColor( itemElem.attribute( QStringLiteral( "arrowHeadOutlineColor" ), QStringLiteral( "0,0,0,255" ) ) );
  mArrowHeadStrokeWidth = itemElem.attribute( QStringLiteral( "outlineWidth" ), QStringLiteral( "1.0" ) ).toDouble();
  // relative paths to absolute
  QString startMarkerPath = itemElem.attribute( QStringLiteral( "startMarkerFile" ), QLatin1String( "" ) );
  QString endMarkerPath = itemElem.attribute( QStringLiteral( "endMarkerFile" ), QLatin1String( "" ) );
  setStartMarker( QgsSymbolLayerUtils::svgSymbolNameToPath( startMarkerPath, pathResolver ) );
  setEndMarker( QgsSymbolLayerUtils::svgSymbolNameToPath( endMarkerPath, pathResolver ) );
  mMarkerMode = QgsComposerArrow::MarkerMode( itemElem.attribute( QStringLiteral( "markerMode" ), QStringLiteral( "0" ) ).toInt() );
  //if bounds behavior version is not set, default to 2.2 behavior
  mBoundsBehavior = itemElem.attribute( QStringLiteral( "boundsBehaviorVersion" ), QStringLiteral( "22" ) ).toInt();

  //arrow style
  QDomElement styleElem = itemElem.firstChildElement( QStringLiteral( "lineStyle" ) );
  if ( !styleElem.isNull() )
  {
    QDomElement lineStyleElem = styleElem.firstChildElement( QStringLiteral( "symbol" ) );
    if ( !lineStyleElem.isNull() )
    {
      delete mLineSymbol;
      mLineSymbol = QgsSymbolLayerUtils::loadSymbol<QgsLineSymbol>( lineStyleElem, context );
    }
  }
  else
  {
    //old project file, read arrow width and color
    delete mLineSymbol;

    QgsStringMap properties;
    properties.insert( QStringLiteral( "width" ), itemElem.attribute( QStringLiteral( "outlineWidth" ), QStringLiteral( "1.0" ) ) );

    if ( mBoundsBehavior == 22 )
    {
      //if arrow was created in versions prior to 2.4, use the old rendering style
      properties.insert( QStringLiteral( "capstyle" ), QStringLiteral( "flat" ) );
    }
    else
    {
      properties.insert( QStringLiteral( "capstyle" ), QStringLiteral( "square" ) );
    }
    int red = 0;
    int blue = 0;
    int green = 0;
    int alpha = 255;

    QDomNodeList arrowColorList = itemElem.elementsByTagName( QStringLiteral( "ArrowColor" ) );
    if ( !arrowColorList.isEmpty() )
    {
      QDomElement arrowColorElem = arrowColorList.at( 0 ).toElement();
      red = arrowColorElem.attribute( QStringLiteral( "red" ), QStringLiteral( "0" ) ).toInt();
      green = arrowColorElem.attribute( QStringLiteral( "green" ), QStringLiteral( "0" ) ).toInt();
      blue = arrowColorElem.attribute( QStringLiteral( "blue" ), QStringLiteral( "0" ) ).toInt();
      alpha = arrowColorElem.attribute( QStringLiteral( "alpha" ), QStringLiteral( "255" ) ).toInt();
      mArrowHeadFillColor = QColor( red, green, blue, alpha );
      mArrowHeadStrokeColor = QColor( red, green, blue, alpha );
    }
    properties.insert( QStringLiteral( "color" ), QStringLiteral( "%1,%2,%3,%4" ).arg( red ).arg( green ).arg( blue ).arg( alpha ) );
    mLineSymbol = QgsLineSymbol::createSimple( properties );
  }

  mPen.setColor( mArrowHeadStrokeColor );
  mPen.setWidthF( mArrowHeadStrokeWidth );
  mBrush.setColor( mArrowHeadFillColor );

  //restore general composer item properties
  //needs to be before start point / stop point because setSceneRect()
  QDomNodeList composerItemList = itemElem.elementsByTagName( QStringLiteral( "ComposerItem" ) );
  if ( !composerItemList.isEmpty() )
  {
    QDomElement composerItemElem = composerItemList.at( 0 ).toElement();
    _readXml( composerItemElem, doc );
  }

  //start point
  QDomNodeList startPointList = itemElem.elementsByTagName( QStringLiteral( "StartPoint" ) );
  if ( !startPointList.isEmpty() )
  {
    QDomElement startPointElem = startPointList.at( 0 ).toElement();
    mStartPoint.setX( startPointElem.attribute( QStringLiteral( "x" ), QStringLiteral( "0.0" ) ).toDouble() );
    mStartPoint.setY( startPointElem.attribute( QStringLiteral( "y" ), QStringLiteral( "0.0" ) ).toDouble() );
  }

  //stop point
  QDomNodeList stopPointList = itemElem.elementsByTagName( QStringLiteral( "StopPoint" ) );
  if ( !stopPointList.isEmpty() )
  {
    QDomElement stopPointElem = stopPointList.at( 0 ).toElement();
    mStopPoint.setX( stopPointElem.attribute( QStringLiteral( "x" ), QStringLiteral( "0.0" ) ).toDouble() );
    mStopPoint.setY( stopPointElem.attribute( QStringLiteral( "y" ), QStringLiteral( "0.0" ) ).toDouble() );
  }

  mStartXIdx = mStopPoint.x() < mStartPoint.x();
  mStartYIdx = mStopPoint.y() < mStartPoint.y();

  adaptItemSceneRect();
  emit itemChanged();
  return true;
}



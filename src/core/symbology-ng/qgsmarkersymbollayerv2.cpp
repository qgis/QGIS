/***************************************************************************
    qgsmarkersymbollayerv2.cpp
    ---------------------
    begin                : November 2009
    copyright            : (C) 2009 by Martin Dobias
    email                : wonder dot sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsmarkersymbollayerv2.h"
#include "qgssymbollayerv2utils.h"

#include "qgsexpression.h"
#include "qgsrendercontext.h"
#include "qgslogger.h"
#include "qgssvgcache.h"

#include <QPainter>
#include <QSvgRenderer>
#include <QFileInfo>
#include <QDir>
#include <QDomDocument>
#include <QDomElement>

#include <cmath>

// MSVC compiler doesn't have defined M_PI in math.h
#ifndef M_PI
#define M_PI          3.14159265358979323846
#endif

#define DEG2RAD(x)    ((x)*M_PI/180)


static QPointF _rotatedOffset( const QPointF& offset, double angle )
{
  angle = DEG2RAD( angle );
  double c = cos( angle ), s = sin( angle );
  return QPointF( offset.x() * c - offset.y() * s, offset.x() * s + offset.y() * c );
}

//////

QgsSimpleMarkerSymbolLayerV2::QgsSimpleMarkerSymbolLayerV2( QString name, QColor color, QColor borderColor, double size, double angle, QgsSymbolV2::ScaleMethod scaleMethod )
    : mOutlineWidth( 0 ), mOutlineWidthUnit( QgsSymbolV2::MM ), mNameExpression( 0 ), mColorExpression( 0 ), mColorBorderExpression( 0 ), mOutlineWidthExpression( 0 ),
    mSizeExpression( 0 ), mAngleExpression( 0 ), mOffsetExpression( 0 )
{
  mName = name;
  mColor = color;
  mBorderColor = borderColor;
  mSize = size;
  mAngle = angle;
  mOffset = QPointF( 0, 0 );
  mScaleMethod = scaleMethod;
  mSizeUnit = QgsSymbolV2::MM;
  mOffsetUnit = QgsSymbolV2::MM;
}

QgsSymbolLayerV2* QgsSimpleMarkerSymbolLayerV2::create( const QgsStringMap& props )
{
  QString name = DEFAULT_SIMPLEMARKER_NAME;
  QColor color = DEFAULT_SIMPLEMARKER_COLOR;
  QColor borderColor = DEFAULT_SIMPLEMARKER_BORDERCOLOR;
  double size = DEFAULT_SIMPLEMARKER_SIZE;
  double angle = DEFAULT_SIMPLEMARKER_ANGLE;
  QgsSymbolV2::ScaleMethod scaleMethod = DEFAULT_SCALE_METHOD;

  if ( props.contains( "name" ) )
    name = props["name"];
  if ( props.contains( "color" ) )
    color = QgsSymbolLayerV2Utils::decodeColor( props["color"] );
  if ( props.contains( "color_border" ) )
    borderColor = QgsSymbolLayerV2Utils::decodeColor( props["color_border"] );
  if ( props.contains( "size" ) )
    size = props["size"].toDouble();
  if ( props.contains( "angle" ) )
    angle = props["angle"].toDouble();
  if ( props.contains( "scale_method" ) )
    scaleMethod = QgsSymbolLayerV2Utils::decodeScaleMethod( props["scale_method"] );

  QgsSimpleMarkerSymbolLayerV2* m = new QgsSimpleMarkerSymbolLayerV2( name, color, borderColor, size, angle, scaleMethod );
  if ( props.contains( "offset" ) )
    m->setOffset( QgsSymbolLayerV2Utils::decodePoint( props["offset"] ) );
  if ( props.contains( "offset_unit" ) )
    m->setOffsetUnit( QgsSymbolLayerV2Utils::decodeOutputUnit( props["offset_unit"] ) );
  if ( props.contains( "size_unit" ) )
    m->setSizeUnit( QgsSymbolLayerV2Utils::decodeOutputUnit( props["size_unit"] ) );

  if ( props.contains( "outline_width" ) )
  {
    m->setOutlineWidth( props["outline_width"].toDouble() );
  }
  if ( props.contains( "outline_width_unit" ) )
  {
    m->setOutlineWidthUnit( QgsSymbolLayerV2Utils::decodeOutputUnit( props["outline_width_unit"] ) );
  }

  //data defined properties
  if ( props.contains( "name_expression" ) )
  {
    m->setDataDefinedProperty( "name", props["name_expression"] );
  }
  if ( props.contains( "color_expression" ) )
  {
    m->setDataDefinedProperty( "color", props["color_expression"] );
  }
  if ( props.contains( "color_border_expression" ) )
  {
    m->setDataDefinedProperty( "color_border", props["color_border_expression"] );
  }
  if ( props.contains( "outline_width_expression" ) )
  {
    m->setDataDefinedProperty( "outline_width", props["outline_width_expression"] );
  }
  if ( props.contains( "size_expression" ) )
  {
    m->setDataDefinedProperty( "size", props["size_expression"] );
  }
  if ( props.contains( "angle_expression" ) )
  {
    m->setDataDefinedProperty( "angle", props["angle_expression"] );
  }
  if ( props.contains( "offset_expression" ) )
  {
    m->setDataDefinedProperty( "offset", props["offset_expression"] );
  }
  return m;
}


QString QgsSimpleMarkerSymbolLayerV2::layerType() const
{
  return "SimpleMarker";
}

void QgsSimpleMarkerSymbolLayerV2::startRender( QgsSymbolV2RenderContext& context )
{
  QColor brushColor = mColor;
  QColor penColor = mBorderColor;

  brushColor.setAlphaF( mColor.alphaF() * context.alpha() );
  penColor.setAlphaF( mBorderColor.alphaF() * context.alpha() );

  mBrush = QBrush( brushColor );
  mPen = QPen( penColor );
  mPen.setWidthF( mOutlineWidth * QgsSymbolLayerV2Utils::lineWidthScaleFactor( context.renderContext(), mOutlineWidthUnit ) );

  QColor selBrushColor = context.selectionColor();
  QColor selPenColor = selBrushColor == mColor ? selBrushColor : mBorderColor;
  if ( context.alpha() < 1 )
  {
    selBrushColor.setAlphaF( context.alpha() );
    selPenColor.setAlphaF( context.alpha() );
  }
  mSelBrush = QBrush( selBrushColor );
  mSelPen = QPen( selPenColor );
  mSelPen.setWidthF( mOutlineWidth * QgsSymbolLayerV2Utils::lineWidthScaleFactor( context.renderContext(), mOutlineWidthUnit ) );

  bool hasDataDefinedRotation = context.renderHints() & QgsSymbolV2::DataDefinedRotation;
  bool hasDataDefinedSize = context.renderHints() & QgsSymbolV2::DataDefinedSizeScale || mSizeExpression;

  // use caching only when:
  // - size, rotation, shape, color, border color is not data-defined
  // - drawing to screen (not printer)
  mUsingCache = !hasDataDefinedRotation && !hasDataDefinedSize && !context.renderContext().forceVectorOutput()
                && !mNameExpression && !mColorExpression && !mColorBorderExpression && !mOutlineWidthExpression &&
                !mSizeExpression && !mAngleExpression;

  // use either QPolygonF or QPainterPath for drawing
  // TODO: find out whether drawing directly doesn't bring overhead - if not, use it for all shapes
  if ( !prepareShape() ) // drawing as a polygon
  {
    if ( preparePath() ) // drawing as a painter path
    {
      // some markers can't be drawn as a polygon (circle, cross)
      // For these set the selected border color to the selected color

      if ( mName != "circle" )
        mSelPen.setColor( selBrushColor );
    }
    else
    {
      QgsDebugMsg( "unknown symbol" );
      return;
    }
  }

  QMatrix transform;

  // scale the shape (if the size is not going to be modified)
  if ( !hasDataDefinedSize )
  {
    double scaledSize = mSize * QgsSymbolLayerV2Utils::lineWidthScaleFactor( context.renderContext(), mSizeUnit );
    if ( mUsingCache )
      scaledSize *= context.renderContext().rasterScaleFactor();
    double half = scaledSize / 2.0;
    transform.scale( half, half );
  }

  // rotate if the rotation is not going to be changed during the rendering
  if ( !hasDataDefinedRotation && mAngle != 0 )
  {
    transform.rotate( mAngle );
  }

  if ( !mPolygon.isEmpty() )
    mPolygon = transform.map( mPolygon );
  else
    mPath = transform.map( mPath );

  if ( mUsingCache )
  {
    prepareCache( context );
  }
  else
  {
    mCache = QImage();
    mSelCache = QImage();
  }

  prepareExpressions( context.layer() );
}


void QgsSimpleMarkerSymbolLayerV2::prepareCache( QgsSymbolV2RenderContext& context )
{
  double scaledSize = mSize * QgsSymbolLayerV2Utils::lineWidthScaleFactor( context.renderContext(), mSizeUnit );

  // calculate necessary image size for the cache
  double pw = (( mPen.widthF() == 0 ? 1 : mPen.widthF() ) + 1 ) / 2 * 2; // make even (round up); handle cosmetic pen
  int imageSize = (( int ) scaledSize + pw ) / 2 * 2 + 1; //  make image width, height odd; account for pen width
  double center = imageSize / 2.0;

  mCache = QImage( QSize( imageSize, imageSize ), QImage::Format_ARGB32_Premultiplied );
  mCache.fill( 0 );

  QPainter p;
  p.begin( &mCache );
  p.setRenderHint( QPainter::Antialiasing );
  p.setBrush( mBrush );
  p.setPen( mPen );
  p.translate( QPointF( center, center ) );
  drawMarker( &p, context );
  p.end();

  // Construct the selected version of the Cache

  QColor selColor = context.selectionColor();

  mSelCache = QImage( QSize( imageSize, imageSize ), QImage::Format_ARGB32_Premultiplied );
  mSelCache.fill( 0 );

  p.begin( &mSelCache );
  p.setRenderHint( QPainter::Antialiasing );
  p.setBrush( mSelBrush );
  p.setPen( mSelPen );
  p.translate( QPointF( center, center ) );
  drawMarker( &p, context );
  p.end();

  // Check that the selected version is different.  If not, then re-render,
  // filling the background with the selection color and using the normal
  // colors for the symbol .. could be ugly!

  if ( mSelCache == mCache )
  {
    p.begin( &mSelCache );
    p.setRenderHint( QPainter::Antialiasing );
    p.fillRect( 0, 0, imageSize, imageSize, selColor );
    p.setBrush( mBrush );
    p.setPen( mPen );
    p.translate( QPointF( center, center ) );
    drawMarker( &p, context );
    p.end();
  }
}

void QgsSimpleMarkerSymbolLayerV2::stopRender( QgsSymbolV2RenderContext& context )
{
  Q_UNUSED( context );
}

bool QgsSimpleMarkerSymbolLayerV2::prepareShape( QString name )
{
  mPolygon.clear();

  if ( name.isNull() )
  {
    name = mName;
  }

  if ( name == "square" || name == "rectangle" )
  {
    mPolygon = QPolygonF( QRectF( QPointF( -1, -1 ), QPointF( 1, 1 ) ) );
    return true;
  }
  else if ( name == "diamond" )
  {
    mPolygon << QPointF( -1, 0 ) << QPointF( 0, 1 )
    << QPointF( 1, 0 ) << QPointF( 0, -1 );
    return true;
  }
  else if ( name == "pentagon" )
  {
    mPolygon << QPointF( sin( DEG2RAD( 288.0 ) ), - cos( DEG2RAD( 288.0 ) ) )
    << QPointF( sin( DEG2RAD( 216.0 ) ), - cos( DEG2RAD( 216.0 ) ) )
    << QPointF( sin( DEG2RAD( 144.0 ) ), - cos( DEG2RAD( 144.0 ) ) )
    << QPointF( sin( DEG2RAD( 72.0 ) ), - cos( DEG2RAD( 72.0 ) ) )
    << QPointF( 0, -1 );
    return true;
  }
  else if ( name == "triangle" )
  {
    mPolygon << QPointF( -1, 1 ) << QPointF( 1, 1 ) << QPointF( 0, -1 );
    return true;
  }
  else if ( name == "equilateral_triangle" )
  {
    mPolygon << QPointF( sin( DEG2RAD( 240.0 ) ), - cos( DEG2RAD( 240.0 ) ) )
    << QPointF( sin( DEG2RAD( 120.0 ) ), - cos( DEG2RAD( 120.0 ) ) )
    << QPointF( 0, -1 );
    return true;
  }
  else if ( name == "star" )
  {
    double sixth = 1.0 / 3;

    mPolygon << QPointF( 0, -1 )
    << QPointF( -sixth, -sixth )
    << QPointF( -1, -sixth )
    << QPointF( -sixth, 0 )
    << QPointF( -1, 1 )
    << QPointF( 0, + sixth )
    << QPointF( 1, 1 )
    << QPointF( + sixth, 0 )
    << QPointF( 1, -sixth )
    << QPointF( + sixth, -sixth );
    return true;
  }
  else if ( name == "regular_star" )
  {
    double inner_r = cos( DEG2RAD( 72.0 ) ) / cos( DEG2RAD( 36.0 ) );

    mPolygon << QPointF( inner_r * sin( DEG2RAD( 324.0 ) ), - inner_r * cos( DEG2RAD( 324.0 ) ) )  // 324
    << QPointF( sin( DEG2RAD( 288.0 ) ) , - cos( DEG2RAD( 288 ) ) )    // 288
    << QPointF( inner_r * sin( DEG2RAD( 252.0 ) ), - inner_r * cos( DEG2RAD( 252.0 ) ) )   // 252
    << QPointF( sin( DEG2RAD( 216.0 ) ) , - cos( DEG2RAD( 216.0 ) ) )   // 216
    << QPointF( 0, inner_r )         // 180
    << QPointF( sin( DEG2RAD( 144.0 ) ) , - cos( DEG2RAD( 144.0 ) ) )   // 144
    << QPointF( inner_r * sin( DEG2RAD( 108.0 ) ), - inner_r * cos( DEG2RAD( 108.0 ) ) )   // 108
    << QPointF( sin( DEG2RAD( 72.0 ) ) , - cos( DEG2RAD( 72.0 ) ) )    //  72
    << QPointF( inner_r * sin( DEG2RAD( 36.0 ) ), - inner_r * cos( DEG2RAD( 36.0 ) ) )   //  36
    << QPointF( 0, -1 );          //   0
    return true;
  }
  else if ( name == "arrow" )
  {
    mPolygon
    << QPointF( 0, -1 )
    << QPointF( 0.5,  -0.5 )
    << QPointF( 0.25, -0.25 )
    << QPointF( 0.25,  1 )
    << QPointF( -0.25,  1 )
    << QPointF( -0.25, -0.5 )
    << QPointF( -0.5,  -0.5 );
    return true;
  }
  else if ( name == "filled_arrowhead" )
  {
    mPolygon << QPointF( 0, 0 ) << QPointF( -1, 1 ) << QPointF( -1, -1 );
    return true;
  }

  return false;
}

bool QgsSimpleMarkerSymbolLayerV2::preparePath( QString name )
{
  mPath = QPainterPath();
  if ( name.isNull() )
  {
    name = mName;
  }

  if ( name == "circle" )
  {
    mPath.addEllipse( QRectF( -1, -1, 2, 2 ) ); // x,y,w,h
    return true;
  }
  else if ( name == "cross" )
  {
    mPath.moveTo( -1, 0 );
    mPath.lineTo( 1, 0 ); // horizontal
    mPath.moveTo( 0, -1 );
    mPath.lineTo( 0, 1 ); // vertical
    return true;
  }
  else if ( name == "x" || name == "cross2" )
  {
    mPath.moveTo( -1, -1 );
    mPath.lineTo( 1, 1 );
    mPath.moveTo( 1, -1 );
    mPath.lineTo( -1, 1 );
    return true;
  }
  else if ( name == "line" )
  {
    mPath.moveTo( 0, -1 );
    mPath.lineTo( 0, 1 ); // vertical line
    return true;
  }
  else if ( name == "arrowhead" )
  {
    mPath.moveTo( 0, 0 );
    mPath.lineTo( -1, -1 );
    mPath.moveTo( 0, 0 );
    mPath.lineTo( -1, 1 );
    return true;
  }

  return false;
}

void QgsSimpleMarkerSymbolLayerV2::markerOffset( QgsSymbolV2RenderContext& context, double& offsetX, double& offsetY )
{
  offsetX = mOffset.x();
  offsetY = mOffset.y();

  if ( mOffsetExpression )
  {
    QPointF offset = QgsSymbolLayerV2Utils::decodePoint( mOffsetExpression->evaluate( const_cast<QgsFeature*>( context.feature() ) ).toString() );
    offsetX = offset.x();
    offsetY = offset.y();
  }

  offsetX *= QgsSymbolLayerV2Utils::lineWidthScaleFactor( context.renderContext(), mOffsetUnit );
  offsetY *= QgsSymbolLayerV2Utils::lineWidthScaleFactor( context.renderContext(), mOffsetUnit );
}

void QgsSimpleMarkerSymbolLayerV2::renderPoint( const QPointF& point, QgsSymbolV2RenderContext& context )
{
  QgsRenderContext& rc = context.renderContext();
  QPainter* p = rc.painter();
  if ( !p )
  {
    return;
  }

  //offset
  double offsetX = 0;
  double offsetY = 0;
  markerOffset( context, offsetX, offsetY );
  QPointF off( offsetX, offsetY );

  //angle
  double angle = mAngle;
  if ( mAngleExpression )
  {
    angle = mAngleExpression->evaluate( const_cast<QgsFeature*>( context.feature() ) ).toDouble();
  }
  if ( angle )
    off = _rotatedOffset( off, angle );

  //data defined shape?
  if ( mNameExpression )
  {
    QString name = mNameExpression->evaluate( const_cast<QgsFeature*>( context.feature() ) ).toString();
    if ( !prepareShape( name ) ) // drawing as a polygon
    {
      preparePath( name ); // drawing as a painter path
    }
  }

  if ( mUsingCache )
  {
    // we will use cached image
    QImage &img = context.selected() ? mSelCache : mCache;
    double s = img.width() / context.renderContext().rasterScaleFactor();
    p->drawImage( QRectF( point.x() - s / 2.0 + off.x(),
                          point.y() - s / 2.0 + off.y(),
                          s, s ), img );
  }
  else
  {
    QMatrix transform;

    bool hasDataDefinedRotation = context.renderHints() & QgsSymbolV2::DataDefinedRotation || mAngleExpression;
    bool hasDataDefinedSize = context.renderHints() & QgsSymbolV2::DataDefinedSizeScale || mSizeExpression;

    // move to the desired position
    transform.translate( point.x() + off.x(), point.y() + off.y() );

    // resize if necessary
    if ( hasDataDefinedSize )
    {
      double scaledSize = mSize;
      if ( mSizeExpression )
      {
        scaledSize = mSizeExpression->evaluate( const_cast<QgsFeature*>( context.feature() ) ).toDouble();
      }
      scaledSize *= QgsSymbolLayerV2Utils::lineWidthScaleFactor( context.renderContext(), mSizeUnit );

      switch ( mScaleMethod )
      {
        case QgsSymbolV2::ScaleArea:
          scaledSize = sqrt( scaledSize );
          break;
        case QgsSymbolV2::ScaleDiameter:
          break;
      }

      double half = scaledSize / 2.0;
      transform.scale( half, half );
    }

    // rotate if necessary
    if ( angle != 0 && hasDataDefinedRotation )
    {
      transform.rotate( angle );
    }

    if ( mColorExpression )
    {
      mBrush.setColor( QgsSymbolLayerV2Utils::decodeColor( mColorExpression->evaluate( const_cast<QgsFeature*>( context.feature() ) ).toString() ) );
    }
    if ( mColorBorderExpression )
    {
      mPen.setColor( QgsSymbolLayerV2Utils::decodeColor( mColorBorderExpression->evaluate( const_cast<QgsFeature*>( context.feature() ) ).toString() ) );
      mSelPen.setColor( QgsSymbolLayerV2Utils::decodeColor( mColorBorderExpression->evaluate( const_cast<QgsFeature*>( context.feature() ) ).toString() ) );
    }
    if ( mOutlineWidthExpression )
    {
      double outlineWidth = mOutlineWidthExpression->evaluate( const_cast<QgsFeature*>( context.feature() ) ).toDouble();
      mPen.setWidthF( outlineWidth * QgsSymbolLayerV2Utils::lineWidthScaleFactor( context.renderContext(), mOutlineWidthUnit ) );
      mSelPen.setWidthF( outlineWidth * QgsSymbolLayerV2Utils::lineWidthScaleFactor( context.renderContext(), mOutlineWidthUnit ) );
    }

    p->setBrush( context.selected() ? mSelBrush : mBrush );
    p->setPen( context.selected() ? mSelPen : mPen );

    if ( !mPolygon.isEmpty() )
      p->drawPolygon( transform.map( mPolygon ) );
    else
      p->drawPath( transform.map( mPath ) );
  }
}


QgsStringMap QgsSimpleMarkerSymbolLayerV2::properties() const
{
  QgsStringMap map;
  map["name"] = mName;
  map["color"] = QgsSymbolLayerV2Utils::encodeColor( mColor );
  map["color_border"] = QgsSymbolLayerV2Utils::encodeColor( mBorderColor );
  map["size"] = QString::number( mSize );
  map["size_unit"] = QgsSymbolLayerV2Utils::encodeOutputUnit( mSizeUnit );
  map["angle"] = QString::number( mAngle );
  map["offset"] = QgsSymbolLayerV2Utils::encodePoint( mOffset );
  map["offset_unit"] = QgsSymbolLayerV2Utils::encodeOutputUnit( mOffsetUnit );
  map["scale_method"] = QgsSymbolLayerV2Utils::encodeScaleMethod( mScaleMethod );
  map["outline_width"] = QString::number( mOutlineWidth );
  map["outline_width_unit"] = QgsSymbolLayerV2Utils::encodeOutputUnit( mOutlineWidthUnit );

  //data define properties
  if ( mNameExpression )
  {
    map["name_expression"] = mNameExpression->dump();
  }
  if ( mColorExpression )
  {
    map["color_expression"] = mColorExpression->dump();
  }
  if ( mColorBorderExpression )
  {
    map["color_border_expression"] = mColorBorderExpression->dump();
  }
  if ( mOutlineWidthExpression )
  {
    map["outline_width_expression"] = mOutlineWidthExpression->dump();
  }
  if ( mSizeExpression )
  {
    map["size_expression"] = mSizeExpression->dump();
  }
  if ( mAngleExpression )
  {
    map["angle_expression"] = mAngleExpression->dump();
  }
  if ( mOffsetExpression )
  {
    map["offset_expression"] = mOffsetExpression->dump();
  }
  return map;
}

QgsSymbolLayerV2* QgsSimpleMarkerSymbolLayerV2::clone() const
{
  QgsSimpleMarkerSymbolLayerV2* m = new QgsSimpleMarkerSymbolLayerV2( mName, mColor, mBorderColor, mSize, mAngle, mScaleMethod );
  m->setOffset( mOffset );
  m->setSizeUnit( mSizeUnit );
  m->setOffsetUnit( mOffsetUnit );
  m->setOutlineWidth( mOutlineWidth );
  m->setOutlineWidthUnit( mOutlineWidthUnit );

  //data defined properties
  if ( mNameExpression )
  {
    m->setDataDefinedProperty( "name", mNameExpression->dump() );
  }
  if ( mColorExpression )
  {
    m->setDataDefinedProperty( "color", mColorExpression->dump() );
  }
  if ( mColorBorderExpression )
  {
    m->setDataDefinedProperty( "color_border", mColorBorderExpression->dump() );
  }
  if ( mOutlineWidthExpression )
  {
    m->setDataDefinedProperty( "outline_width", mOutlineWidthExpression->dump() );
  }
  if ( mSizeExpression )
  {
    m->setDataDefinedProperty( "size", mSizeExpression->dump() );
  }
  if ( mAngleExpression )
  {
    m->setDataDefinedProperty( "angle", mAngleExpression->dump() );
  }
  if ( mOffsetExpression )
  {
    m->setDataDefinedProperty( "offset", mOffsetExpression->dump() );
  }
  return m;
}

void QgsSimpleMarkerSymbolLayerV2::writeSldMarker( QDomDocument &doc, QDomElement &element, QgsStringMap props ) const
{
  // <Graphic>
  QDomElement graphicElem = doc.createElement( "se:Graphic" );
  element.appendChild( graphicElem );

  QgsSymbolLayerV2Utils::wellKnownMarkerToSld( doc, graphicElem, mName, mColor, mBorderColor, -1, mSize );

  // <Rotation>
  QString angleFunc;
  bool ok;
  double angle = props.value( "angle", "0" ).toDouble( &ok );
  if ( !ok )
  {
    angleFunc = QString( "%1 + %2" ).arg( props.value( "angle", "0" ) ).arg( mAngle );
  }
  else if ( angle + mAngle != 0 )
  {
    angleFunc = QString::number( angle + mAngle );
  }
  QgsSymbolLayerV2Utils::createRotationElement( doc, graphicElem, angleFunc );

  // <Displacement>
  QgsSymbolLayerV2Utils::createDisplacementElement( doc, graphicElem, mOffset );
}

QString QgsSimpleMarkerSymbolLayerV2::ogrFeatureStyle( double mmScaleFactor, double mapUnitScaleFactor ) const
{
  Q_UNUSED( mmScaleFactor );
  Q_UNUSED( mapUnitScaleFactor );
#if 0
  QString ogrType = "3"; //default is circle
  if ( mName == "square" )
  {
    ogrType = "5";
  }
  else if ( mName == "triangle" )
  {
    ogrType = "7";
  }
  else if ( mName == "star" )
  {
    ogrType = "9";
  }
  else if ( mName == "circle" )
  {
    ogrType = "3";
  }
  else if ( mName == "cross" )
  {
    ogrType = "0";
  }
  else if ( mName == "x" || mName == "cross2" )
  {
    ogrType = "1";
  }
  else if ( mName == "line" )
  {
    ogrType = "10";
  }

  QString ogrString;
  ogrString.append( "SYMBOL(" );
  ogrString.append( "id:" );
  ogrString.append( "\"" );
  ogrString.append( "ogr-sym-" );
  ogrString.append( ogrType );
  ogrString.append( "\"" );
  ogrString.append( ",c:" );
  ogrString.append( mColor.name() );
  ogrString.append( ",o:" );
  ogrString.append( mBorderColor.name() );
  ogrString.append( QString( ",s:%1mm" ).arg( mSize ) );
  ogrString.append( ")" );
  return ogrString;
#endif //0

  QString ogrString;
  ogrString.append( "PEN(" );
  ogrString.append( "c:" );
  ogrString.append( mColor.name() );
  ogrString.append( ",w:" );
  ogrString.append( QString::number( mSize ) );
  ogrString.append( "mm" );
  ogrString.append( ")" );
  return ogrString;
}

const QgsExpression* QgsSimpleMarkerSymbolLayerV2::dataDefinedProperty( const QString& property ) const
{
  if ( property == "name" )
  {
    return mNameExpression;
  }
  else if ( property == "color" )
  {
    return mColorExpression;
  }
  else if ( property == "color_border" )
  {
    return mColorBorderExpression;
  }
  else if ( property == "outline_width" )
  {
    return mOutlineWidthExpression;
  }
  else if ( property == "size" )
  {
    return mSizeExpression;
  }
  else if ( property == "angle" )
  {
    return mAngleExpression;
  }
  else if ( property == "offset" )
  {
    return mOffsetExpression;
  }
  return 0;
}

QString QgsSimpleMarkerSymbolLayerV2::dataDefinedPropertyString( const QString& property ) const
{
  const QgsExpression* ex = dataDefinedProperty( property );
  return ex ? ex->dump() : QString();
}

void QgsSimpleMarkerSymbolLayerV2::setDataDefinedProperty( const QString& property, const QString& expressionString )
{
  if ( property == "name" )
  {
    delete mNameExpression; mNameExpression = new QgsExpression( expressionString );
  }
  else if ( property == "color" )
  {
    delete mColorExpression; mColorExpression = new QgsExpression( expressionString );
  }
  else if ( property == "color_border" )
  {
    delete mColorBorderExpression; mColorBorderExpression = new QgsExpression( expressionString );
  }
  else if ( property == "outline_width" )
  {
    delete mOutlineWidthExpression; mOutlineWidthExpression = new QgsExpression( expressionString );
  }
  else if ( property == "size" )
  {
    delete mSizeExpression; mSizeExpression = new QgsExpression( expressionString );
  }
  else if ( property == "angle" )
  {
    delete  mAngleExpression; mAngleExpression = new QgsExpression( expressionString );
  }
  else if ( property == "offset" )
  {
    delete mOffsetExpression; mOffsetExpression = new QgsExpression( expressionString );
  }
}

void QgsSimpleMarkerSymbolLayerV2::removeDataDefinedProperty( const QString& property )
{
  if ( property == "name" )
  {
    delete mNameExpression; mNameExpression = 0;
  }
  else if ( property == "color" )
  {
    delete mColorExpression; mColorExpression = 0;
  }
  else if ( property == "color_border" )
  {
    delete mColorBorderExpression; mColorBorderExpression = 0;
  }
  else if ( property == "outline_width" )
  {
    delete mOutlineWidthExpression; mOutlineWidthExpression = 0;
  }
  else if ( property == "size" )
  {
    delete mSizeExpression; mSizeExpression = 0;
  }
  else if ( property == "angle" )
  {
    delete  mAngleExpression; mAngleExpression = 0;
  }
  else if ( property == "offset" )
  {
    delete mOffsetExpression; mOffsetExpression = 0;
  }
}

void QgsSimpleMarkerSymbolLayerV2::removeDataDefinedProperties()
{
  delete mNameExpression; mNameExpression = 0;
  delete mColorExpression; mColorExpression = 0;
  delete mColorBorderExpression; mColorBorderExpression = 0;
  delete mOutlineWidthExpression; mOutlineWidthExpression = 0;
  delete mSizeExpression; mSizeExpression = 0;
  delete mAngleExpression; mAngleExpression = 0;
  delete mOffsetExpression; mOffsetExpression = 0;
}

QSet<QString> QgsSimpleMarkerSymbolLayerV2::usedAttributes() const
{
  QSet<QString> attributes;

  //add data defined attributes
  QStringList columns;
  if ( mNameExpression )
    columns.append( mNameExpression->referencedColumns() );
  if ( mColorExpression )
    columns.append( mColorExpression->referencedColumns() );
  if ( mColorBorderExpression )
    columns.append( mColorBorderExpression->referencedColumns() );
  if ( mOutlineWidthExpression )
    columns.append( mOutlineWidthExpression->referencedColumns() );
  if ( mSizeExpression )
    columns.append( mSizeExpression->referencedColumns() );
  if ( mAngleExpression )
    columns.append( mAngleExpression->referencedColumns() );
  if ( mOffsetExpression )
    columns.append( mOffsetExpression->referencedColumns() );

  QStringList::const_iterator it = columns.constBegin();
  for ( ; it != columns.constEnd(); ++it )
  {
    attributes.insert( *it );
  }
  return attributes;
}

QgsSymbolLayerV2* QgsSimpleMarkerSymbolLayerV2::createFromSld( QDomElement &element )
{
  QgsDebugMsg( "Entered." );

  QDomElement graphicElem = element.firstChildElement( "Graphic" );
  if ( graphicElem.isNull() )
    return NULL;

  QString name = "square";
  QColor color, borderColor;
  double borderWidth, size;

  if ( !QgsSymbolLayerV2Utils::wellKnownMarkerFromSld( graphicElem, name, color, borderColor, borderWidth, size ) )
    return NULL;

  double angle = 0.0;
  QString angleFunc;
  if ( QgsSymbolLayerV2Utils::rotationFromSldElement( graphicElem, angleFunc ) )
  {
    bool ok;
    double d = angleFunc.toDouble( &ok );
    if ( ok )
      angle = d;
  }

  QPointF offset;
  QgsSymbolLayerV2Utils::displacementFromSldElement( graphicElem, offset );

  QgsMarkerSymbolLayerV2 *m = new QgsSimpleMarkerSymbolLayerV2( name, color, borderColor, size );
  m->setAngle( angle );
  m->setOffset( offset );
  return m;
}

void QgsSimpleMarkerSymbolLayerV2::drawMarker( QPainter* p, QgsSymbolV2RenderContext& context )
{
  Q_UNUSED( context );

  if ( mPolygon.count() != 0 )
  {
    p->drawPolygon( mPolygon );
  }
  else
  {
    p->drawPath( mPath );
  }
}

void QgsSimpleMarkerSymbolLayerV2::prepareExpressions( const QgsVectorLayer* vl )
{
  if ( !vl )
  {
    return;
  }

  const QgsFields& fields = vl->pendingFields();
  if ( mNameExpression )
    mNameExpression->prepare( fields );
  if ( mColorExpression )
    mColorExpression->prepare( fields );
  if ( mColorBorderExpression )
    mColorBorderExpression->prepare( fields );
  if ( mOutlineWidthExpression )
    mOutlineWidthExpression->prepare( fields );
  if ( mSizeExpression )
    mSizeExpression->prepare( fields );
  if ( mAngleExpression )
    mAngleExpression->prepare( fields );
  if ( mOffsetExpression )
    mOffsetExpression->prepare( fields );
}

//////////


QgsSvgMarkerSymbolLayerV2::QgsSvgMarkerSymbolLayerV2( QString name, double size, double angle ): mSizeExpression( 0 ),
    mOutlineWidthExpression( 0 ), mAngleExpression( 0 ), mOffsetExpression( 0 ), mNameExpression( 0 ), mFillExpression( 0 ),
    mOutlineExpression( 0 )
{
  mPath = QgsSymbolLayerV2Utils::symbolNameToPath( name );
  mSize = size;
  mAngle = angle;
  mOffset = QPointF( 0, 0 );
  mOutlineWidth = 1.0;
  mOutlineWidthUnit = QgsSymbolV2::MM;
  mFillColor = QColor( Qt::black );
  mOutlineColor = QColor( Qt::black );
}


QgsSymbolLayerV2* QgsSvgMarkerSymbolLayerV2::create( const QgsStringMap& props )
{
  QString name = DEFAULT_SVGMARKER_NAME;
  double size = DEFAULT_SVGMARKER_SIZE;
  double angle = DEFAULT_SVGMARKER_ANGLE;

  if ( props.contains( "name" ) )
    name = props["name"];
  if ( props.contains( "size" ) )
    size = props["size"].toDouble();
  if ( props.contains( "angle" ) )
    angle = props["angle"].toDouble();

  QgsSvgMarkerSymbolLayerV2* m = new QgsSvgMarkerSymbolLayerV2( name, size, angle );

  //we only check the svg default parameters if necessary, since it could be expensive
  if ( !props.contains( "fill" ) && !props.contains( "outline" ) && !props.contains( "outline-width" ) )
  {
    QColor fillColor, outlineColor;
    double outlineWidth;
    bool hasFillParam, hasOutlineParam, hasOutlineWidthParam;
    QgsSvgCache::instance()->containsParams( name, hasFillParam, fillColor, hasOutlineParam, outlineColor, hasOutlineWidthParam, outlineWidth );
    if ( hasFillParam )
    {
      m->setFillColor( fillColor );
    }
    if ( hasOutlineParam )
    {
      m->setOutlineColor( outlineColor );
    }
    if ( hasOutlineWidthParam )
    {
      m->setOutlineWidth( outlineWidth );
    }
  }

  if ( props.contains( "size_unit" ) )
    m->setSizeUnit( QgsSymbolLayerV2Utils::decodeOutputUnit( "size_unit" ) );
  if ( props.contains( "offset" ) )
    m->setOffset( QgsSymbolLayerV2Utils::decodePoint( props["offset"] ) );
  if ( props.contains( "offset_unit" ) )
    m->setOffsetUnit( QgsSymbolLayerV2Utils::decodeOutputUnit( props["offset_unit"] ) );
  if ( props.contains( "fill" ) )
    m->setFillColor( QColor( props["fill"] ) );
  if ( props.contains( "outline" ) )
    m->setOutlineColor( QColor( props["outline"] ) );
  if ( props.contains( "outline-width" ) )
    m->setOutlineWidth( props["outline-width"].toDouble() );
  if ( props.contains( "outline_width_unit" ) )
    m->setOutlineWidthUnit( QgsSymbolLayerV2Utils::decodeOutputUnit( props["outline_width_unit"] ) );

  //data defined properties
  if ( props.contains( "size_expression" ) )
  {
    m->setDataDefinedProperty( "size", props["size_expression"] );
  }
  if ( props.contains( "outline-width_expression" ) )
  {
    m->setDataDefinedProperty( "outline-width", props["outline-width_expression"] );
  }
  if ( props.contains( "angle_expression" ) )
  {
    m->setDataDefinedProperty( "angle", props["angle_expression"] );
  }
  if ( props.contains( "offset_expression" ) )
  {
    m->setDataDefinedProperty( "offset", props["offset_expression"] );
  }
  if ( props.contains( "name_expression" ) )
  {
    m->setDataDefinedProperty( "name", props["name_expression"] );
  }
  if ( props.contains( "fill_expression" ) )
  {
    m->setDataDefinedProperty( "fill", props["fill_expression"] );
  }
  if ( props.contains( "outline_expression" ) )
  {
    m->setDataDefinedProperty( "outline", props["outline_expression"] );
  }
  return m;
}

void QgsSvgMarkerSymbolLayerV2::setPath( QString path )
{
  mPath = path;
  QColor fillColor, outlineColor;
  double outlineWidth;
  bool hasFillParam, hasOutlineParam, hasOutlineWidthParam;
  QgsSvgCache::instance()->containsParams( path, hasFillParam, fillColor, hasOutlineParam, outlineColor, hasOutlineWidthParam, outlineWidth );
  if ( hasFillParam )
  {
    setFillColor( fillColor );
  }
  if ( hasOutlineParam )
  {
    setOutlineColor( outlineColor );
  }
  if ( hasOutlineWidthParam )
  {
    setOutlineWidth( outlineWidth );
  }
}


QString QgsSvgMarkerSymbolLayerV2::layerType() const
{
  return "SvgMarker";
}

void QgsSvgMarkerSymbolLayerV2::startRender( QgsSymbolV2RenderContext& context )
{
  mOrigSize = mSize; // save in case the size would be data defined
  Q_UNUSED( context );
  prepareExpressions( context.layer() );
}

void QgsSvgMarkerSymbolLayerV2::stopRender( QgsSymbolV2RenderContext& context )
{
  Q_UNUSED( context );
}

void QgsSvgMarkerSymbolLayerV2::renderPoint( const QPointF& point, QgsSymbolV2RenderContext& context )
{
  QPainter* p = context.renderContext().painter();
  if ( !p )
  {
    return;
  }

  double size = mSize;
  if ( mSizeExpression )
  {
    size = mSizeExpression->evaluate( const_cast<QgsFeature*>( context.feature() ) ).toDouble();
  }
  size *= QgsSymbolLayerV2Utils::lineWidthScaleFactor( context.renderContext(), mSizeUnit );
  //don't render symbols with size below one or above 10,000 pixels
  if (( int )size < 1 || 10000.0 < size )
  {
    return;
  }

  p->save();

  QPointF offset = mOffset;
  if ( mOffsetExpression )
  {
    QString offsetString =  mOffsetExpression->evaluate( const_cast<QgsFeature*>( context.feature() ) ).toString();
    offset = QgsSymbolLayerV2Utils::decodePoint( offsetString );
  }
  double offsetX = offset.x() * QgsSymbolLayerV2Utils::lineWidthScaleFactor( context.renderContext(), mOffsetUnit );
  double offsetY = offset.y() * QgsSymbolLayerV2Utils::lineWidthScaleFactor( context.renderContext(), mOffsetUnit );
  QPointF outputOffset( offsetX, offsetY );

  double angle = mAngle;
  if ( mAngleExpression )
  {
    angle = mAngleExpression->evaluate( const_cast<QgsFeature*>( context.feature() ) ).toDouble();
  }
  if ( angle )
    outputOffset = _rotatedOffset( outputOffset, angle );
  p->translate( point + outputOffset );

  bool rotated = !doubleNear( angle, 0 );
  bool drawOnScreen = doubleNear( context.renderContext().rasterScaleFactor(), 1.0, 0.1 );
  if ( rotated )
    p->rotate( angle );

  QString path = mPath;
  if ( mNameExpression )
  {
    path = mNameExpression->evaluate( const_cast<QgsFeature*>( context.feature() ) ).toString();
  }

  double outlineWidth = mOutlineWidth;
  if ( mOutlineWidthExpression )
  {
    outlineWidth = mOutlineWidthExpression->evaluate( const_cast<QgsFeature*>( context.feature() ) ).toDouble();
  }

  QColor fillColor = mFillColor;
  if ( mFillExpression )
  {
    fillColor = QgsSymbolLayerV2Utils::decodeColor( mFillExpression->evaluate( const_cast<QgsFeature*>( context.feature() ) ).toString() );
  }

  QColor outlineColor = mOutlineColor;
  if ( mOutlineExpression )
  {
    outlineColor = QgsSymbolLayerV2Utils::decodeColor( mOutlineExpression->evaluate( const_cast<QgsFeature*>( context.feature() ) ).toString() );
  }


  bool fitsInCache = true;
  bool usePict = true;
  double hwRatio = 1.0;
  if ( drawOnScreen && !rotated )
  {
    usePict = false;
    const QImage& img = QgsSvgCache::instance()->svgAsImage( path, size, fillColor, outlineColor, outlineWidth,
                        context.renderContext().scaleFactor(), context.renderContext().rasterScaleFactor(), fitsInCache );
    if ( fitsInCache && img.width() > 1 )
    {
      //consider transparency
      if ( !doubleNear( context.alpha(), 1.0 ) )
      {
        QImage transparentImage = img.copy();
        QgsSymbolLayerV2Utils::multiplyImageOpacity( &transparentImage, context.alpha() );
        p->drawImage( -transparentImage.width() / 2.0, -transparentImage.height() / 2.0, transparentImage );
        hwRatio = ( double )transparentImage.height() / ( double )transparentImage.width();
      }
      else
      {
        p->drawImage( -img.width() / 2.0, -img.height() / 2.0, img );
        hwRatio = ( double )img.height() / ( double )img.width();
      }
    }
  }

  if ( usePict || !fitsInCache )
  {
    p->setOpacity( context.alpha() );
    const QPicture& pct = QgsSvgCache::instance()->svgAsPicture( path, size, fillColor, outlineColor, outlineWidth,
                          context.renderContext().scaleFactor(), context.renderContext().rasterScaleFactor() );

    if ( pct.width() > 1 )
    {
      p->drawPicture( 0, 0, pct );
      hwRatio = ( double )pct.height() / ( double )pct.width();
    }
  }

  if ( context.selected() )
  {
    QPen pen( context.selectionColor() );
    double penWidth = QgsSymbolLayerV2Utils::lineWidthScaleFactor( context.renderContext(), QgsSymbolV2::MM );
    if ( penWidth > size / 20 )
    {
      // keep the pen width from covering symbol
      penWidth = size / 20;
    }
    double penOffset = penWidth / 2;
    pen.setWidth( penWidth );
    p->setPen( pen );
    p->setBrush( Qt::NoBrush );
    double wSize = size + penOffset;
    double hSize = size * hwRatio + penOffset;
    p->drawRect( QRectF( -wSize / 2.0, -hSize / 2.0, wSize, hSize ) );
  }

  p->restore();
}


QgsStringMap QgsSvgMarkerSymbolLayerV2::properties() const
{
  QgsStringMap map;
  map["name"] = QgsSymbolLayerV2Utils::symbolPathToName( mPath );
  map["size"] = QString::number( mSize );
  map["size_unit"] = QgsSymbolLayerV2Utils::encodeOutputUnit( mSizeUnit );
  map["angle"] = QString::number( mAngle );
  map["offset"] = QgsSymbolLayerV2Utils::encodePoint( mOffset );
  map["offset_unit"] = QgsSymbolLayerV2Utils::encodeOutputUnit( mOffsetUnit );
  map["fill"] = mFillColor.name();
  map["outline"] = mOutlineColor.name();
  map["outline-width"] = QString::number( mOutlineWidth );
  map["outline_width_unit"] = QgsSymbolLayerV2Utils::encodeOutputUnit( mOutlineWidthUnit );

  //data defined properties
  if ( mSizeExpression )
  {
    map["size_expression"] = mSizeExpression->dump();
  }
  if ( mOutlineWidthExpression )
  {
    map["outline-width_expression"] = mOutlineWidthExpression->dump();
  }
  if ( mAngleExpression )
  {
    map["angle_expression"] = mAngleExpression->dump();
  }
  if ( mOffsetExpression )
  {
    map["offset_expression"] = mOffsetExpression->dump();
  }
  if ( mNameExpression )
  {
    map["name_expression"] = mNameExpression->dump();
  }
  if ( mFillExpression )
  {
    map["fill_expression"] = mFillExpression->dump();
  }
  if ( mOutlineExpression )
  {
    map["outline_expression"] = mOutlineExpression->dump();
  }
  return map;
}

QgsSymbolLayerV2* QgsSvgMarkerSymbolLayerV2::clone() const
{
  QgsSvgMarkerSymbolLayerV2* m = new QgsSvgMarkerSymbolLayerV2( mPath, mSize, mAngle );
  m->setFillColor( mFillColor );
  m->setOutlineColor( mOutlineColor );
  m->setOutlineWidth( mOutlineWidth );
  m->setOutlineWidthUnit( mOutlineWidthUnit );
  m->setOffset( mOffset );
  m->setOffsetUnit( mOffsetUnit );
  m->setSizeUnit( mSizeUnit );

  //data defined properties
  if ( mSizeExpression )
  {
    m->setDataDefinedProperty( "size", mSizeExpression->dump() );
  }
  if ( mOutlineWidthExpression )
  {
    m->setDataDefinedProperty( "outline-width", mOutlineWidthExpression->dump() );
  }
  if ( mAngleExpression )
  {
    m->setDataDefinedProperty( "angle", mAngleExpression->dump() );
  }
  if ( mOffsetExpression )
  {
    m->setDataDefinedProperty( "offset", mOffsetExpression->dump() );
  }
  if ( mNameExpression )
  {
    m->setDataDefinedProperty( "name", mNameExpression->dump() );
  }
  if ( mFillExpression )
  {
    m->setDataDefinedProperty( "fill", mFillExpression->dump() );
  }
  if ( mOutlineExpression )
  {
    m->setDataDefinedProperty( "outline", mOutlineExpression->dump() );
  }

  return m;
}

const QgsExpression* QgsSvgMarkerSymbolLayerV2::dataDefinedProperty( const QString& property ) const
{
  if ( property == "size" )
  {
    return mSizeExpression;
  }
  else if ( property == "outline-width" )
  {
    return mOutlineWidthExpression;
  }
  else if ( property == "angle" )
  {
    return mAngleExpression;
  }
  else if ( property == "offset" )
  {
    return mOffsetExpression;
  }
  else if ( property == "name" )
  {
    return mNameExpression;
  }
  else if ( property == "fill" )
  {
    return mFillExpression;
  }
  else if ( property == "outline" )
  {
    return mOutlineExpression;
  }
  return 0;
}

QString QgsSvgMarkerSymbolLayerV2::dataDefinedPropertyString( const QString& property ) const
{
  const QgsExpression* ex = dataDefinedProperty( property );
  return ( ex ? ex->dump() : QString() );
}

void QgsSvgMarkerSymbolLayerV2::setDataDefinedProperty( const QString& property, const QString& expressionString )
{
  if ( property == "size" )
  {
    delete mSizeExpression; mSizeExpression = new QgsExpression( expressionString );
  }
  else if ( property == "outline-width" )
  {
    delete mOutlineWidthExpression; mOutlineWidthExpression = new QgsExpression( expressionString );
  }
  else if ( property == "angle" )
  {
    delete mAngleExpression; mAngleExpression = new QgsExpression( expressionString );
  }
  else if ( property == "offset" )
  {
    delete mOffsetExpression; mOffsetExpression = new QgsExpression( expressionString );
  }
  else if ( property == "name" )
  {
    delete mNameExpression; mNameExpression = new QgsExpression( expressionString );
  }
  else if ( property == "fill" )
  {
    delete mFillExpression; mFillExpression = new QgsExpression( expressionString );
  }
  else if ( property == "outline" )
  {
    delete mOutlineExpression; mOutlineExpression = new QgsExpression( expressionString );
  }
}

void QgsSvgMarkerSymbolLayerV2::removeDataDefinedProperty( const QString& property )
{
  if ( property == "size" )
  {
    delete mSizeExpression; mSizeExpression = 0;
  }
  else if ( property == "outline-width" )
  {
    delete mOutlineWidthExpression; mOutlineWidthExpression = 0;
  }
  else if ( property == "angle" )
  {
    delete mAngleExpression; mAngleExpression = 0;
  }
  else if ( property == "offset" )
  {
    delete mOffsetExpression; mOffsetExpression = 0;
  }
  else if ( property == "name" )
  {
    delete mNameExpression; mNameExpression = 0;
  }
  else if ( property == "fill" )
  {
    delete mFillExpression; mFillExpression = 0;
  }
  else if ( property == "outline" )
  {
    delete mOutlineExpression; mOutlineExpression = 0;
  }
}

void QgsSvgMarkerSymbolLayerV2::removeDataDefinedProperties()
{
  delete mSizeExpression; mSizeExpression = 0;
  delete mOutlineWidthExpression; mOutlineWidthExpression = 0;
  delete mAngleExpression; mAngleExpression = 0;
  delete mOffsetExpression; mOffsetExpression = 0;
  delete mNameExpression; mNameExpression = 0;
  delete mFillExpression; mFillExpression = 0;
  delete mOutlineExpression; mOutlineExpression = 0;
}

QSet<QString> QgsSvgMarkerSymbolLayerV2::usedAttributes() const
{
  QSet<QString> attributes;

  //add data defined attributes
  QStringList columns;
  if ( mSizeExpression )
    columns.append( mSizeExpression->referencedColumns() );
  if ( mOutlineWidthExpression )
    columns.append( mOutlineWidthExpression->referencedColumns() );
  if ( mAngleExpression )
    columns.append( mAngleExpression->referencedColumns() );
  if ( mOffsetExpression )
    columns.append( mOffsetExpression->referencedColumns() );
  if ( mNameExpression )
    columns.append( mNameExpression->referencedColumns() );
  if ( mFillExpression )
    columns.append( mFillExpression->referencedColumns() );
  if ( mOutlineExpression )
    columns.append( mOutlineExpression->referencedColumns() );

  QStringList::const_iterator it = columns.constBegin();
  for ( ; it != columns.constEnd(); ++it )
  {
    attributes.insert( *it );
  }
  return attributes;
}

void QgsSvgMarkerSymbolLayerV2::prepareExpressions( const QgsVectorLayer* vl )
{
  if ( !vl )
  {
    return;
  }

  const QgsFields& fields = vl->pendingFields();
  if ( mSizeExpression )
    mSizeExpression->prepare( fields );
  if ( mOutlineWidthExpression )
    mOutlineWidthExpression->prepare( fields );
  if ( mAngleExpression )
    mAngleExpression->prepare( fields );
  if ( mOffsetExpression )
    mOffsetExpression->prepare( fields );
  if ( mNameExpression )
    mNameExpression->prepare( fields );
  if ( mFillExpression )
    mFillExpression->prepare( fields );
}

void QgsSvgMarkerSymbolLayerV2::setOutputUnit( QgsSymbolV2::OutputUnit unit )
{
  mSizeUnit = unit;
  mOffsetUnit = unit;
  mOutlineWidthUnit = unit;
}

QgsSymbolV2::OutputUnit QgsSvgMarkerSymbolLayerV2::outputUnit() const
{
  QgsSymbolV2::OutputUnit unit = mSizeUnit;
  if ( unit != mOffsetUnit || unit != mOutlineWidthUnit )
  {
    return QgsSymbolV2::Mixed;
  }
  return unit;
}

void QgsSvgMarkerSymbolLayerV2::writeSldMarker( QDomDocument &doc, QDomElement &element, QgsStringMap props ) const
{
  // <Graphic>
  QDomElement graphicElem = doc.createElement( "se:Graphic" );
  element.appendChild( graphicElem );

  QgsSymbolLayerV2Utils::externalGraphicToSld( doc, graphicElem, mPath, "image/svg+xml", mFillColor, mSize );

  // <Rotation>
  QString angleFunc;
  bool ok;
  double angle = props.value( "angle", "0" ).toDouble( &ok );
  if ( !ok )
  {
    angleFunc = QString( "%1 + %2" ).arg( props.value( "angle", "0" ) ).arg( mAngle );
  }
  else if ( angle + mAngle != 0 )
  {
    angleFunc = QString::number( angle + mAngle );
  }

  QgsSymbolLayerV2Utils::createRotationElement( doc, graphicElem, angleFunc );

  // <Displacement>
  QgsSymbolLayerV2Utils::createDisplacementElement( doc, graphicElem, mOffset );
}

QgsSymbolLayerV2* QgsSvgMarkerSymbolLayerV2::createFromSld( QDomElement &element )
{
  QgsDebugMsg( "Entered." );

  QDomElement graphicElem = element.firstChildElement( "Graphic" );
  if ( graphicElem.isNull() )
    return NULL;

  QString path, mimeType;
  QColor fillColor;
  double size;

  if ( !QgsSymbolLayerV2Utils::externalGraphicFromSld( graphicElem, path, mimeType, fillColor, size ) )
    return NULL;

  if ( mimeType != "image/svg+xml" )
    return NULL;

  double angle = 0.0;
  QString angleFunc;
  if ( QgsSymbolLayerV2Utils::rotationFromSldElement( graphicElem, angleFunc ) )
  {
    bool ok;
    double d = angleFunc.toDouble( &ok );
    if ( ok )
      angle = d;
  }

  QPointF offset;
  QgsSymbolLayerV2Utils::displacementFromSldElement( graphicElem, offset );

  QgsSvgMarkerSymbolLayerV2* m = new QgsSvgMarkerSymbolLayerV2( path, size );
  m->setFillColor( fillColor );
  //m->setOutlineColor( outlineColor );
  //m->setOutlineWidth( outlineWidth );
  m->setAngle( angle );
  m->setOffset( offset );
  return m;
}

//////////

QgsFontMarkerSymbolLayerV2::QgsFontMarkerSymbolLayerV2( QString fontFamily, QChar chr, double pointSize, QColor color, double angle )
{
  mFontFamily = fontFamily;
  mChr = chr;
  mColor = color;
  mAngle = angle;
  mSize = pointSize;
  mSizeUnit = QgsSymbolV2::MM;
  mOffset = QPointF( 0, 0 );
  mOffsetUnit = QgsSymbolV2::MM;
}

QgsSymbolLayerV2* QgsFontMarkerSymbolLayerV2::create( const QgsStringMap& props )
{
  QString fontFamily = DEFAULT_FONTMARKER_FONT;
  QChar chr = DEFAULT_FONTMARKER_CHR;
  double pointSize = DEFAULT_FONTMARKER_SIZE;
  QColor color = DEFAULT_FONTMARKER_COLOR;
  double angle = DEFAULT_FONTMARKER_ANGLE;

  if ( props.contains( "font" ) )
    fontFamily = props["font"];
  if ( props.contains( "chr" ) && props["chr"].length() > 0 )
    chr = props["chr"].at( 0 );
  if ( props.contains( "size" ) )
    pointSize = props["size"].toDouble();
  if ( props.contains( "color" ) )
    color = QgsSymbolLayerV2Utils::decodeColor( props["color"] );
  if ( props.contains( "angle" ) )
    angle = props["angle"].toDouble();

  QgsFontMarkerSymbolLayerV2* m = new QgsFontMarkerSymbolLayerV2( fontFamily, chr, pointSize, color, angle );
  if ( props.contains( "offset" ) )
    m->setOffset( QgsSymbolLayerV2Utils::decodePoint( props["offset"] ) );
  if ( props.contains( "offset_unit" ) )
    m->setOffsetUnit( QgsSymbolLayerV2Utils::decodeOutputUnit( props["offset_unit" ] ) );
  if ( props.contains( "size_unit" ) )
    m->setSizeUnit( QgsSymbolLayerV2Utils::decodeOutputUnit( props["size_unit"] ) );
  return m;
}

QString QgsFontMarkerSymbolLayerV2::layerType() const
{
  return "FontMarker";
}

void QgsFontMarkerSymbolLayerV2::startRender( QgsSymbolV2RenderContext& context )
{
  mFont = QFont( mFontFamily );
  mFont.setPixelSize( mSize * QgsSymbolLayerV2Utils::lineWidthScaleFactor( context.renderContext(), mSizeUnit ) );
  QFontMetrics fm( mFont );
  mChrOffset = QPointF( fm.width( mChr ) / 2, -fm.ascent() / 2 );

  mOrigSize = mSize; // save in case the size would be data defined
}

void QgsFontMarkerSymbolLayerV2::stopRender( QgsSymbolV2RenderContext& context )
{
  Q_UNUSED( context );
}

void QgsFontMarkerSymbolLayerV2::renderPoint( const QPointF& point, QgsSymbolV2RenderContext& context )
{
  QPainter* p = context.renderContext().painter();
  QColor penColor = context.selected() ? context.selectionColor() : mColor;
  penColor.setAlphaF( mColor.alphaF() * context.alpha() );
  p->setPen( penColor );
  p->setFont( mFont );


  p->save();
  double offsetX = mOffset.x() * QgsSymbolLayerV2Utils::lineWidthScaleFactor( context.renderContext(), mOffsetUnit );
  double offsetY = mOffset.y() * QgsSymbolLayerV2Utils::lineWidthScaleFactor( context.renderContext(), mOffsetUnit );
  QPointF outputOffset( offsetX, offsetY );
  if ( mAngle )
    outputOffset = _rotatedOffset( outputOffset, mAngle );
  p->translate( point + outputOffset );

  if ( context.renderHints() & QgsSymbolV2::DataDefinedSizeScale )
  {
    double s = mSize / mOrigSize;
    p->scale( s, s );
  }

  if ( mAngle != 0 )
    p->rotate( mAngle );

  p->drawText( -mChrOffset, mChr );
  p->restore();
}

QgsStringMap QgsFontMarkerSymbolLayerV2::properties() const
{
  QgsStringMap props;
  props["font"] = mFontFamily;
  props["chr"] = mChr;
  props["size"] = QString::number( mSize );
  props["size_unit"] = QgsSymbolLayerV2Utils::encodeOutputUnit( mSizeUnit );
  props["color"] = QgsSymbolLayerV2Utils::encodeColor( mColor );
  props["angle"] = QString::number( mAngle );
  props["offset"] = QgsSymbolLayerV2Utils::encodePoint( mOffset );
  props["offset_unit"] = QgsSymbolLayerV2Utils::encodeOutputUnit( mOffsetUnit );
  return props;
}

QgsSymbolLayerV2* QgsFontMarkerSymbolLayerV2::clone() const
{
  QgsFontMarkerSymbolLayerV2* m = new QgsFontMarkerSymbolLayerV2( mFontFamily, mChr, mSize, mColor, mAngle );
  m->setOffset( mOffset );
  m->setOffsetUnit( mOffsetUnit );
  m->setSizeUnit( mSizeUnit );
  return m;
}

void QgsFontMarkerSymbolLayerV2::writeSldMarker( QDomDocument &doc, QDomElement &element, QgsStringMap props ) const
{
  // <Graphic>
  QDomElement graphicElem = doc.createElement( "se:Graphic" );
  element.appendChild( graphicElem );

  QString fontPath = QString( "ttf://%1" ).arg( mFontFamily );
  int markIndex = mChr.unicode();
  QgsSymbolLayerV2Utils::externalMarkerToSld( doc, graphicElem, fontPath, "ttf", &markIndex, mColor, mSize );

  // <Rotation>
  QString angleFunc;
  bool ok;
  double angle = props.value( "angle", "0" ).toDouble( &ok );
  if ( !ok )
  {
    angleFunc = QString( "%1 + %2" ).arg( props.value( "angle", "0" ) ).arg( mAngle );
  }
  else if ( angle + mAngle != 0 )
  {
    angleFunc = QString::number( angle + mAngle );
  }
  QgsSymbolLayerV2Utils::createRotationElement( doc, graphicElem, angleFunc );

  // <Displacement>
  QgsSymbolLayerV2Utils::createDisplacementElement( doc, graphicElem, mOffset );
}

QgsSymbolLayerV2* QgsFontMarkerSymbolLayerV2::createFromSld( QDomElement &element )
{
  QgsDebugMsg( "Entered." );

  QDomElement graphicElem = element.firstChildElement( "Graphic" );
  if ( graphicElem.isNull() )
    return NULL;

  QString name, format;
  QColor color;
  double size;
  int chr;

  if ( !QgsSymbolLayerV2Utils::externalMarkerFromSld( graphicElem, name, format, chr, color, size ) )
    return NULL;

  if ( !name.startsWith( "ttf://" ) || format != "ttf" )
    return NULL;

  QString fontFamily = name.mid( 6 );

  double angle = 0.0;
  QString angleFunc;
  if ( QgsSymbolLayerV2Utils::rotationFromSldElement( graphicElem, angleFunc ) )
  {
    bool ok;
    double d = angleFunc.toDouble( &ok );
    if ( ok )
      angle = d;
  }

  QPointF offset;
  QgsSymbolLayerV2Utils::displacementFromSldElement( graphicElem, offset );

  QgsMarkerSymbolLayerV2 *m = new QgsFontMarkerSymbolLayerV2( fontFamily, chr, size, color );
  m->setAngle( angle );
  m->setOffset( offset );
  return m;
}

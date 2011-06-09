#include "qgsellipsesymbollayerv2.h"
#include "qgsfeature.h"
#include "qgsrendercontext.h"
#include <QPainter>
#include <QSet>

QgsEllipseSymbolLayerV2::QgsEllipseSymbolLayerV2(): mDataDefinedWidth(-1), mDataDefinedHeight(-1),
  mDataDefinedOutlineWidth(-1), mFillColor( Qt::black ), mDataDefinedFillColor(-1), mOutlineColor( Qt::white ), mDataDefinedOutlineColor(-1)
{
  mSymbolName = "circle";
  mPen.setColor( mOutlineColor );
  mPen.setWidth( 1.0 );
  mPen.setJoinStyle( Qt::MiterJoin );
  mBrush.setColor( mFillColor );
  mBrush.setStyle( Qt::SolidPattern );
}

QgsEllipseSymbolLayerV2::~QgsEllipseSymbolLayerV2()
{
}

QgsSymbolLayerV2* QgsEllipseSymbolLayerV2::create( const QgsStringMap& properties )
{
  QgsEllipseSymbolLayerV2* layer = new QgsEllipseSymbolLayerV2();
  if( properties.contains( "symbol_name" ) )
  {
    layer->setSymbolName( properties[ "symbol_name" ] );
  }
  if( properties.contains( "symbol_width" ) )
  {
    layer->setSymbolWidth( properties["symbol_width"].toDouble() );
  }
  if( properties.contains( "data_defined_width" ) )
  {
    layer->setDataDefinedWidth( properties["data_defined_width"].toInt() );
  }
  if( properties.contains("symbol_height") )
  {
    layer->setSymbolHeight( properties["symbol_height"].toDouble() );
  }
  if( properties.contains( "data_defined_height" ) )
  {
    layer->setDataDefinedHeight( properties["data_defined_height"].toInt() );
  }
  if( properties.contains( "outline_width" ) )
  {
    layer->setOutlineWidth( properties["outline_width"].toDouble() );
  }
  if( properties.contains( "data_defined_outline_width" ) )
  {
    layer->setDataDefinedOutlineWidth( properties["data_defined_outline_width"].toInt() );
  }
  if( properties.contains( "fill_color" ) )
  {
    layer->setFillColor( QgsSymbolLayerV2Utils::decodeColor( properties["fill_color"] ) );
  }
  if( properties.contains( "data_defined_fill_color" ) )
  {
    layer->setDataDefinedFillColor( properties["data_defined_fill_color"].toInt() );
  }
  if( properties.contains( "outline_color" ) )
  {
    layer->setOutlineColor( QgsSymbolLayerV2Utils::decodeColor( properties["outline_color"] ) );
  }
  if( properties.contains( "data_defined_outline_color" ) )
  {
    layer->setDataDefinedOutlineColor( properties[ "data_defined_outline_color" ].toInt() );
  }
  return layer;
}

void QgsEllipseSymbolLayerV2::renderPoint( const QPointF& point, QgsSymbolV2RenderContext& context )
{
  const QgsFeature* f = context.feature();

  if( f )
  {
    if( mDataDefinedOutlineWidth != -1 )
    {
      double width = context.outputLineWidth( f->attributeMap()[mDataDefinedOutlineWidth].toDouble() );
      mPen.setWidth( width );
    }
    if( mDataDefinedFillColor != -1 )
    {
      mBrush.setColor( QColor( f->attributeMap()[mDataDefinedFillColor].toString() ) );
    }
    if( mDataDefinedOutlineColor != -1 )
    {
      mPen.setColor( QColor( f->attributeMap()[mDataDefinedOutlineColor].toString() ) );
    }
    if( mDataDefinedWidth != -1 || mDataDefinedHeight != -1 )
    {
      preparePath( context, f );
    }
  }

  QPainter* p = context.renderContext().painter();
  if( !p )
  {
    return;
  }

  QPointF off( context.outputLineWidth( mOffset.x() ), context.outputLineWidth( mOffset.y() ) );

  QMatrix transform;
  transform.translate( point.x() + off.x(), point.y() + off.y() );

  p->setPen( mPen );
  p->setBrush( mBrush );
  p->drawPath( transform.map( mPainterPath ) );
}

QString QgsEllipseSymbolLayerV2::layerType() const
{
  return "Ellipse";
}

void QgsEllipseSymbolLayerV2::startRender( QgsSymbolV2RenderContext& context )
{
  if( !hasDataDefinedProperty() )
  {
    preparePath( context );
  }
  mPen.setColor( mOutlineColor );
  mPen.setWidth( context.outputLineWidth( mOutlineWidth ) );
  mBrush.setColor( mFillColor );
}

void QgsEllipseSymbolLayerV2::stopRender( QgsSymbolV2RenderContext& context )
{
}

QgsSymbolLayerV2* QgsEllipseSymbolLayerV2::clone() const
{
  return QgsEllipseSymbolLayerV2::create( properties() );
}

QgsStringMap QgsEllipseSymbolLayerV2::properties() const
{
  QgsStringMap map;
  map["symbol_name"] = mSymbolName;
  map["symbol_width"] = QString::number( mSymbolWidth );
  map["data_defined_width"] = QString::number( mDataDefinedWidth );
  map["symbol_height"] = QString::number( mSymbolHeight );
  map["data_defined_height"] = QString::number( mDataDefinedHeight );
  map["outline_width"] = QString::number( mOutlineWidth );
  map["data_defined_outline_width"] = QString::number( mDataDefinedOutlineWidth );
  map["fill_color"] = QgsSymbolLayerV2Utils::encodeColor( mFillColor );
  map["data_defined_fill_color"] = QString::number( mDataDefinedFillColor );
  map["outline_color"] = QgsSymbolLayerV2Utils::encodeColor( mOutlineColor );
  map["data_defined_outline_color"] = QString::number( mDataDefinedOutlineColor );
  return map;
}

bool QgsEllipseSymbolLayerV2::hasDataDefinedProperty() const
{
  return  ( mDataDefinedWidth != -1 || mDataDefinedHeight != -1 || mDataDefinedOutlineWidth != -1
      || mDataDefinedFillColor != -1 || mDataDefinedOutlineColor != -1 );
}

void QgsEllipseSymbolLayerV2::preparePath( QgsSymbolV2RenderContext& context, const QgsFeature* f )
{
  mPainterPath = QPainterPath();

  double width = 0;
  if( f && mDataDefinedOutlineWidth != -1 )
  {
    width = context.outputLineWidth( f->attributeMap()[mDataDefinedOutlineWidth].toDouble() );
  }
  else
  {
    width = context.outputLineWidth( mSymbolWidth );
  }

  double height = 0;
  if( f && mDataDefinedHeight != -1 )
  {
    height = context.outputLineWidth( f->attributeMap()[mDataDefinedHeight].toDouble() );
  }
  else
  {
    height = context.outputLineWidth( mSymbolHeight );
  }

  if( mSymbolName == "circle" )
  {
    mPainterPath.addEllipse( QRectF( -width / 2.0, -height / 2.0, width, height ) );
  }
  else if( mSymbolName == "rectangle" )
  {
    mPainterPath.addRect( QRectF( -width / 2.0, -height / 2.0, width, height ) );
  }
  else if( mSymbolName == "cross" )
  {
    mPainterPath.moveTo( 0, -height / 2.0 );
    mPainterPath.lineTo( 0, height / 2.0 );
    mPainterPath.moveTo( -width / 2.0, 0 );
    mPainterPath.lineTo( width / 2.0, 0 );
  }
  else if( mSymbolName == "triangle" )
  {
    mPainterPath.moveTo( 0, -height / 2.0 );
    mPainterPath.lineTo( -width / 2.0, height / 2.0 );
    mPainterPath.lineTo( width / 2.0, height / 2.0 );
    mPainterPath.lineTo( 0, -height / 2.0 );
  }
}

QSet<QString> QgsEllipseSymbolLayerV2::usedAttributes() const
{
  QSet<QString> dataDefinedAttributes;
  /*dataDefinedAttributes.insert( mDataDefinedWidth );
  dataDefinedAttributes.insert( mDataDefinedHeight );
  dataDefinedAttributes.insert( mDataDefinedOutlineWidth );
  dataDefinedAttributes.insert( mDataDefinedFillColor );
  dataDefinedAttributes.insert( mDataDefinedOutlineColor );
  dataDefinedAttributes.remove( -1 );*/
  return dataDefinedAttributes;
}

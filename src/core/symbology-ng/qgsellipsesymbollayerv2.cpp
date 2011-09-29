#include "qgsellipsesymbollayerv2.h"
#include "qgsfeature.h"
#include "qgsrendercontext.h"
#include <QPainter>
#include <QSet>

QgsEllipseSymbolLayerV2::QgsEllipseSymbolLayerV2(): mSymbolName( "circle" ), mSymbolWidth( 4 ), mSymbolHeight( 3 ),
    mFillColor( Qt::black ), mOutlineColor( Qt::white ), mOutlineWidth( 0 )
{
  mPen.setColor( mOutlineColor );
  mPen.setWidth( 1.0 );
  mPen.setJoinStyle( Qt::MiterJoin );
  mBrush.setColor( mFillColor );
  mBrush.setStyle( Qt::SolidPattern );

  mAngle = 0;
  mWidthField.first = -1;
  mHeightField.first = -1;
  mRotationField.first = -1;
  mOutlineWidthField.first = -1;
  mFillColorField.first = -1;
  mOutlineColorField.first = -1;
  mSymbolNameField.first = -1;
}

QgsEllipseSymbolLayerV2::~QgsEllipseSymbolLayerV2()
{
}

QgsSymbolLayerV2* QgsEllipseSymbolLayerV2::create( const QgsStringMap& properties )
{
  QgsEllipseSymbolLayerV2* layer = new QgsEllipseSymbolLayerV2();
  if ( properties.contains( "symbol_name" ) )
  {
    layer->setSymbolName( properties[ "symbol_name" ] );
  }
  if ( properties.contains( "symbol_width" ) )
  {
    layer->setSymbolWidth( properties["symbol_width"].toDouble() );
  }
  if ( properties.contains( "symbol_height" ) )
  {
    layer->setSymbolHeight( properties["symbol_height"].toDouble() );
  }
  if ( properties.contains( "angle" ) )
  {
    layer->setAngle( properties["angle"].toDouble() );
  }
  if ( properties.contains( "outline_width" ) )
  {
    layer->setOutlineWidth( properties["outline_width"].toDouble() );
  }
  if ( properties.contains( "fill_color" ) )
  {
    layer->setFillColor( QgsSymbolLayerV2Utils::decodeColor( properties["fill_color"] ) );
  }
  if ( properties.contains( "outline_color" ) )
  {
    layer->setOutlineColor( QgsSymbolLayerV2Utils::decodeColor( properties["outline_color"] ) );
  }

  //data defined properties
  if ( properties.contains( "height_index" ) && properties.contains( "height_field" ) )
  {
    layer->setHeightField( properties["height_index"].toInt(), properties["height_field"] );
  }
  if ( properties.contains( "width_index" ) && properties.contains( "width_field" ) )
  {
    layer->setWidthField( properties["width_index"].toInt(), properties["width_field"] );
  }
  if ( properties.contains( "rotation_index" ) && properties.contains( "rotation_field" ) )
  {
    layer->setRotationField( properties["rotation_index"].toInt(), properties["rotation_field"] );
  }
  if ( properties.contains( "outline_width_index" ) && properties.contains( "outline_width_field" ) )
  {
    layer->setOutlineWidthField( properties["outline_width_index"].toInt(), properties["outline_width_field"] );
  }
  if ( properties.contains( "fill_color_index" ) && properties.contains( "fill_color_field" ) )
  {
    layer->setFillColorField( properties["fill_color_index"].toInt(), properties["fill_color_field"] );
  }
  if ( properties.contains( "outline_color_index" ) && properties.contains( "outline_color_field" ) )
  {
    layer->setOutlineColorField( properties["outline_color_index"].toInt(), properties["outline_color_field"] );
  }
  if ( properties.contains( "symbol_name_index" ) && properties.contains( "symbol_name_field" ) )
  {
    layer->setSymbolNameField( properties["symbol_name_index"].toInt(), properties["symbol_name_field"] );
  }

  return layer;
}

void QgsEllipseSymbolLayerV2::renderPoint( const QPointF& point, QgsSymbolV2RenderContext& context )
{
  const QgsFeature* f = context.feature();

  if ( f )
  {
    if ( mOutlineWidthField.first != -1 )
    {
      double width = context.outputLineWidth( f->attributeMap()[mOutlineWidthField.first].toDouble() );
      mPen.setWidthF( width );
    }
    if ( mFillColorField.first != -1 )
    {
      mBrush.setColor( QColor( f->attributeMap()[mFillColorField.first].toString() ) );
    }
    if ( mOutlineColorField.first != -1 )
    {
      mPen.setColor( QColor( f->attributeMap()[mOutlineColorField.first].toString() ) );
    }

    if ( mWidthField.first != -1 || mHeightField.first != -1 || mSymbolNameField.first != -1 )
    {
      QString symbolName = ( mSymbolNameField.first == -1 ) ? mSymbolName : f->attributeMap()[mSymbolNameField.first].toString();
      preparePath( symbolName, context, f );
    }
  }

  QPainter* p = context.renderContext().painter();
  if ( !p )
  {
    return;
  }

  //priority for rotation: 1. data defined symbol level, 2. symbol layer rotation (mAngle)
  double rotation = 0.0;
  if ( f && mRotationField.first != -1 )
  {
    rotation = f->attributeMap()[mRotationField.first].toDouble();
  }
  else if ( !doubleNear( mAngle, 0.0 ) )
  {
    rotation = mAngle;
  }

  QMatrix transform;
  transform.translate( point.x(), point.y() );
  if ( !doubleNear( rotation, 0.0 ) )
  {
    transform.rotate( rotation );
  }

  p->setPen( mPen );
  p->setBrush( mBrush );
  p->drawPath( transform.map( mPainterPath ) );
}

QString QgsEllipseSymbolLayerV2::layerType() const
{
  return "EllipseMarker";
}

void QgsEllipseSymbolLayerV2::startRender( QgsSymbolV2RenderContext& context )
{
  if ( !context.feature() || !hasDataDefinedProperty() )
  {
    preparePath( mSymbolName, context );
  }
  mPen.setColor( mOutlineColor );
  mPen.setWidthF( context.outputLineWidth( mOutlineWidth ) );
  mBrush.setColor( mFillColor );
}

void QgsEllipseSymbolLayerV2::stopRender( QgsSymbolV2RenderContext & )
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
  map["width_index"] = QString::number( mWidthField.first );
  map["width_field"] = mWidthField.second;
  map["symbol_height"] = QString::number( mSymbolHeight );
  map["height_index"] = QString::number( mHeightField.first );
  map["height_field"] = mHeightField.second;
  map["angle"] = QString::number( mAngle );
  map["rotation_index"] = QString::number( mRotationField.first );
  map["rotation_field"] = mRotationField.second;
  map["outline_width"] = QString::number( mOutlineWidth );
  map["outline_width_index"] = QString::number( mOutlineWidthField.first );
  map["outline_width_field"] = mOutlineWidthField.second;
  map["fill_color"] = QgsSymbolLayerV2Utils::encodeColor( mFillColor );
  map["fill_color_index"] = QString::number( mFillColorField.first );
  map["fill_color_field"] = mFillColorField.second;
  map["outline_color"] = QgsSymbolLayerV2Utils::encodeColor( mOutlineColor );
  map["outline_color_index"] = QString::number( mOutlineColorField.first );
  map["outline_color_field"] = mOutlineColorField.second;
  map["symbol_name_index"] = QString::number( mSymbolNameField.first );
  map["symbol_name_field"] = mSymbolNameField.second;
  return map;
}

bool QgsEllipseSymbolLayerV2::hasDataDefinedProperty() const
{
  return ( mWidthField.first != -1 || mHeightField.first != -1 || mOutlineWidthField.first != -1
           || mFillColorField.first != -1 || mOutlineColorField.first != -1 );
}

void QgsEllipseSymbolLayerV2::preparePath( const QString& symbolName, QgsSymbolV2RenderContext& context, const QgsFeature* f )
{
  mPainterPath = QPainterPath();

  double width = 0;

  if ( f && mWidthField.first != -1 ) //1. priority: data defined setting on symbol layer level
  {
    width = context.outputLineWidth( f->attributeMap()[mWidthField.first].toDouble() );
  }
  else if( context.renderHints() & QgsSymbolV2::DataDefinedSizeScale ) //2. priority: is data defined size on symbol level
  {
      width = context.outputLineWidth( mSize );
  }
  else //3. priority: global width setting
  {
    width = context.outputLineWidth( mSymbolWidth );
  }

  double height = 0;
  if ( f && mHeightField.first != -1 ) //1. priority: data defined setting on symbol layer level
  {
    height = context.outputLineWidth( f->attributeMap()[mHeightField.first].toDouble() );
  }
  else if( context.renderHints() & QgsSymbolV2::DataDefinedSizeScale ) //2. priority: is data defined size on symbol level
  {
      height = context.outputLineWidth( mSize );
  }
  else //3. priority: global height setting
  {
    height = context.outputLineWidth( mSymbolHeight );
  }

  if ( symbolName == "circle" )
  {
    mPainterPath.addEllipse( QRectF( -width / 2.0, -height / 2.0, width, height ) );
  }
  else if ( symbolName == "rectangle" )
  {
    mPainterPath.addRect( QRectF( -width / 2.0, -height / 2.0, width, height ) );
  }
  else if ( symbolName == "cross" )
  {
    mPainterPath.moveTo( 0, -height / 2.0 );
    mPainterPath.lineTo( 0, height / 2.0 );
    mPainterPath.moveTo( -width / 2.0, 0 );
    mPainterPath.lineTo( width / 2.0, 0 );
  }
  else if ( symbolName == "triangle" )
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
  if ( mWidthField.first != -1 )
  {
    dataDefinedAttributes.insert( mWidthField.second );
  }
  if ( mHeightField.first != -1 )
  {
    dataDefinedAttributes.insert( mHeightField.second );
  }
  if ( mRotationField.first != -1 )
  {
    dataDefinedAttributes.insert( mRotationField.second );
  }
  if ( mOutlineWidthField.first != -1 )
  {
    dataDefinedAttributes.insert( mOutlineWidthField.second );
  }
  if ( mFillColorField.first != -1 )
  {
    dataDefinedAttributes.insert( mFillColorField.second );
  }
  if ( mOutlineColorField.first != -1 )
  {
    dataDefinedAttributes.insert( mOutlineColorField.second );
  }
  if ( mSymbolNameField.first != -1 )
  {
    dataDefinedAttributes.insert( mSymbolNameField.second );
  }
  return dataDefinedAttributes;
}

void QgsEllipseSymbolLayerV2::setSymbolNameField( int index, const QString& field )
{
  mSymbolNameField.first = index;
  mSymbolNameField.second = field;
}

void QgsEllipseSymbolLayerV2::setWidthField( int index, const QString& field )
{
  mWidthField.first = index;
  mWidthField.second = field;
}

void QgsEllipseSymbolLayerV2::setHeightField( int index, const QString& field )
{
  mHeightField.first = index;
  mHeightField.second = field;
}

void QgsEllipseSymbolLayerV2::setRotationField( int index, const QString& field )
{
  mRotationField.first = index;
  mRotationField.second = field;
}

void QgsEllipseSymbolLayerV2::setOutlineWidthField( int index, const QString& field )
{
  mOutlineWidthField.first = index;
  mOutlineWidthField.second = field;
}

void QgsEllipseSymbolLayerV2::setFillColorField( int index, const QString& field )
{
  mFillColorField.first = index;
  mFillColorField.second = field;
}

void QgsEllipseSymbolLayerV2::setOutlineColorField( int index, const QString& field )
{
  mOutlineColorField.first = index;
  mOutlineColorField.second = field;
}

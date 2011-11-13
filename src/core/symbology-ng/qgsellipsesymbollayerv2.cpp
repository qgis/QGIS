#include "qgsellipsesymbollayerv2.h"
#include "qgsfeature.h"
#include "qgsrendercontext.h"
#include "qgsvectorlayer.h"
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
  mWidthIndex = -1;
  mHeightIndex = -1;
  mRotationIndex = -1;
  mOutlineWidthIndex = -1;
  mFillColorIndex = -1;
  mOutlineColorIndex = -1;
  mSymbolNameIndex = -1;
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
  if ( properties.contains( "height_field" ) )
  {
    layer->setHeightField( properties["height_field"] );
  }
  if ( properties.contains( "width_field" ) )
  {
    layer->setWidthField( properties["width_field"] );
  }
  if ( properties.contains( "rotation_field" ) )
  {
    layer->setRotationField( properties["rotation_field"] );
  }
  if ( properties.contains( "outline_width_field" ) )
  {
    layer->setOutlineWidthField( properties["outline_width_field"] );
  }
  if ( properties.contains( "fill_color_field" ) )
  {
    layer->setFillColorField( properties["fill_color_field"] );
  }
  if ( properties.contains( "outline_color_field" ) )
  {
    layer->setOutlineColorField( properties["outline_color_field"] );
  }
  if ( properties.contains( "symbol_name_field" ) )
  {
    layer->setSymbolNameField( properties["symbol_name_field"] );
  }

  return layer;
}

void QgsEllipseSymbolLayerV2::renderPoint( const QPointF& point, QgsSymbolV2RenderContext& context )
{
  const QgsFeature* f = context.feature();

  if ( f )
  {
    if ( mOutlineWidthIndex != -1 )
    {
      double width = context.outputLineWidth( f->attributeMap()[mOutlineWidthIndex].toDouble() );
      mPen.setWidthF( width );
    }
    if ( mFillColorIndex != -1 )
    {
      mBrush.setColor( QColor( f->attributeMap()[mFillColorIndex].toString() ) );
    }
    if ( mOutlineColorIndex != -1 )
    {
      mPen.setColor( QColor( f->attributeMap()[mOutlineColorIndex].toString() ) );
    }

    if ( mWidthIndex != -1 || mHeightIndex != -1 || mSymbolNameIndex != -1 )
    {
      QString symbolName = ( mSymbolNameIndex == -1 ) ? mSymbolName : f->attributeMap()[mSymbolNameIndex].toString();
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
  if ( f && mRotationIndex != -1 )
  {
    rotation = f->attributeMap()[mRotationIndex].toDouble();
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

  //resolve data defined attribute indices
  const QgsVectorLayer* vlayer = context.layer();
  if ( vlayer )
  {
    mWidthIndex = vlayer->fieldNameIndex( mWidthField );
    mHeightIndex = vlayer->fieldNameIndex( mHeightField );
    mRotationIndex = vlayer->fieldNameIndex( mRotationField );
    mOutlineWidthIndex = vlayer->fieldNameIndex( mOutlineWidthField );
    mFillColorIndex = vlayer->fieldNameIndex( mFillColorField );
    mOutlineColorIndex = vlayer->fieldNameIndex( mOutlineColorField );
    mSymbolNameIndex = vlayer->fieldNameIndex( mSymbolNameField );
  }
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
  map["width_field"] = mWidthField;
  map["symbol_height"] = QString::number( mSymbolHeight );
  map["height_field"] = mHeightField;
  map["angle"] = QString::number( mAngle );
  map["rotation_field"] = mRotationField;
  map["outline_width"] = QString::number( mOutlineWidth );
  map["outline_width_field"] = mOutlineWidthField;
  map["fill_color"] = QgsSymbolLayerV2Utils::encodeColor( mFillColor );
  map["fill_color_field"] = mFillColorField;
  map["outline_color"] = QgsSymbolLayerV2Utils::encodeColor( mOutlineColor );
  map["outline_color_field"] = mOutlineColorField;
  map["symbol_name_field"] = mSymbolNameField;
  return map;
}

bool QgsEllipseSymbolLayerV2::hasDataDefinedProperty() const
{
  return ( mWidthIndex != -1 || mHeightIndex != -1 || mOutlineWidthIndex != -1
           || mFillColorIndex != -1 || mOutlineColorIndex != -1 );
}

void QgsEllipseSymbolLayerV2::preparePath( const QString& symbolName, QgsSymbolV2RenderContext& context, const QgsFeature* f )
{
  mPainterPath = QPainterPath();

  double width = 0;

  if ( f && mWidthIndex != -1 ) //1. priority: data defined setting on symbol layer level
  {
    width = context.outputLineWidth( f->attributeMap()[mWidthIndex].toDouble() );
  }
  else if ( context.renderHints() & QgsSymbolV2::DataDefinedSizeScale ) //2. priority: is data defined size on symbol level
  {
    width = context.outputLineWidth( mSize );
  }
  else //3. priority: global width setting
  {
    width = context.outputLineWidth( mSymbolWidth );
  }

  double height = 0;
  if ( f && mHeightIndex != -1 ) //1. priority: data defined setting on symbol layer level
  {
    height = context.outputLineWidth( f->attributeMap()[mHeightIndex].toDouble() );
  }
  else if ( context.renderHints() & QgsSymbolV2::DataDefinedSizeScale ) //2. priority: is data defined size on symbol level
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
  if ( !mWidthField.isEmpty() )
  {
    dataDefinedAttributes.insert( mWidthField );
  }
  if ( !mHeightField.isEmpty() )
  {
    dataDefinedAttributes.insert( mHeightField );
  }
  if ( !mRotationField.isEmpty() )
  {
    dataDefinedAttributes.insert( mRotationField );
  }
  if ( !mOutlineWidthField.isEmpty() )
  {
    dataDefinedAttributes.insert( mOutlineWidthField );
  }
  if ( !mFillColorField.isEmpty() )
  {
    dataDefinedAttributes.insert( mFillColorField );
  }
  if ( !mOutlineColorField.isEmpty() )
  {
    dataDefinedAttributes.insert( mOutlineColorField );
  }
  if ( !mSymbolNameField.isEmpty() )
  {
    dataDefinedAttributes.insert( mSymbolNameField );
  }
  return dataDefinedAttributes;
}

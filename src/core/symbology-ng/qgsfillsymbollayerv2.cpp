
#include "qgsfillsymbollayerv2.h"
#include "qgssymbollayerv2utils.h"

#include "qgsrendercontext.h"

#include <QPainter>

QgsSimpleFillSymbolLayerV2::QgsSimpleFillSymbolLayerV2( QColor color, Qt::BrushStyle style, QColor borderColor, Qt::PenStyle borderStyle, double borderWidth )
    : mBrushStyle( style ), mBorderColor( borderColor ), mBorderStyle( borderStyle ), mBorderWidth( borderWidth )
{
  mColor = color;
}


QgsSymbolLayerV2* QgsSimpleFillSymbolLayerV2::create( const QgsStringMap& props )
{
  QColor color = DEFAULT_SIMPLEFILL_COLOR;
  Qt::BrushStyle style = DEFAULT_SIMPLEFILL_STYLE;
  QColor borderColor = DEFAULT_SIMPLEFILL_BORDERCOLOR;
  Qt::PenStyle borderStyle = DEFAULT_SIMPLEFILL_BORDERSTYLE;
  double borderWidth = DEFAULT_SIMPLEFILL_BORDERWIDTH;

  if ( props.contains( "color" ) )
    color = QgsSymbolLayerV2Utils::decodeColor( props["color"] );
  if ( props.contains( "style" ) )
    style = QgsSymbolLayerV2Utils::decodeBrushStyle( props["style"] );
  if ( props.contains( "color_border" ) )
    borderColor = QgsSymbolLayerV2Utils::decodeColor( props["color_border"] );
  if ( props.contains( "style_border" ) )
    borderStyle = QgsSymbolLayerV2Utils::decodePenStyle( props["style_border"] );
  if ( props.contains( "width_border" ) )
    borderWidth = props["width_border"].toDouble();

  return new QgsSimpleFillSymbolLayerV2( color, style, borderColor, borderStyle, borderWidth );
}


QString QgsSimpleFillSymbolLayerV2::layerType() const
{
  return "SimpleFill";
}

void QgsSimpleFillSymbolLayerV2::startRender( QgsSymbolV2RenderContext& context )
{
  mBrush = QBrush( mColor, mBrushStyle );
  mPen = QPen( mBorderColor );
  mPen.setStyle( mBorderStyle );
  mPen.setWidthF( context.outputLineWidth( mBorderWidth ) );
}

void QgsSimpleFillSymbolLayerV2::stopRender( QgsSymbolV2RenderContext& context )
{
}

void QgsSimpleFillSymbolLayerV2::renderPolygon( const QPolygonF& points, QList<QPolygonF>* rings, QgsSymbolV2RenderContext& context )
{
  QPainter* p = context.renderContext().painter();
  if ( !p )
  {
    return;
  }

  p->setBrush( mBrush );
  p->setPen( mPen );

  _renderPolygon( p, points, rings );
}

QgsStringMap QgsSimpleFillSymbolLayerV2::properties() const
{
  QgsStringMap map;
  map["color"] = QgsSymbolLayerV2Utils::encodeColor( mColor );
  map["style"] = QgsSymbolLayerV2Utils::encodeBrushStyle( mBrushStyle );
  map["color_border"] = QgsSymbolLayerV2Utils::encodeColor( mBorderColor );
  map["style_border"] = QgsSymbolLayerV2Utils::encodePenStyle( mBorderStyle );
  map["width_border"] = QString::number( mBorderWidth );
  return map;
}

QgsSymbolLayerV2* QgsSimpleFillSymbolLayerV2::clone() const
{
  return new QgsSimpleFillSymbolLayerV2( mColor, mBrushStyle, mBorderColor, mBorderStyle, mBorderWidth );
}

//QgsSVGFillSymbolLayer
#include <QFile>
#include <QSvgRenderer>
#include "qgsproject.h" //for absolute/relative file paths

QgsSVGFillSymbolLayer::QgsSVGFillSymbolLayer( const QString& svgFilePath, double width ): mPatternWidth( width ), mOutline( 0 )
{
  setSvgFilePath( svgFilePath );
  mOutlineWidth = 0.3;
  setSubSymbol( new QgsLineSymbolV2() );
}

QgsSVGFillSymbolLayer::QgsSVGFillSymbolLayer( const QByteArray& svgData, double width ): mSvgData( svgData ), mPatternWidth( width ), mOutline( 0 )
{
  storeViewBox();
  mOutlineWidth = 0.3;
  setSubSymbol( new QgsLineSymbolV2() );
}

QgsSVGFillSymbolLayer::~QgsSVGFillSymbolLayer()
{
  delete mOutline;
}

void QgsSVGFillSymbolLayer::setSvgFilePath( const QString& svgPath )
{
  QFile svgFile( svgPath );
  if ( svgFile.open( QFile::ReadOnly ) )
  {
    mSvgData = svgFile.readAll();
    storeViewBox();
  }
  mSvgFilePath = svgPath;
}

QgsSymbolLayerV2* QgsSVGFillSymbolLayer::create( const QgsStringMap& properties )
{
  QByteArray data;
  double width = 20;
  QString svgFilePath;


  if ( properties.contains( "width" ) )
  {
    width = properties["width"].toDouble();
  }
  if ( properties.contains( "svgFile" ) )
  {
    svgFilePath = QgsProject::instance()->readPath( properties["svgFile"] );
  }

  if ( !svgFilePath.isEmpty() )
  {
    return new QgsSVGFillSymbolLayer( svgFilePath, width );
  }
  else
  {
    if ( properties.contains( "data" ) )
    {
      data = QByteArray::fromHex( properties["data"].toLocal8Bit() );
    }

    return new QgsSVGFillSymbolLayer( data, width );
  }
}

QString QgsSVGFillSymbolLayer::layerType() const
{
  return "SVGFill";
}

void QgsSVGFillSymbolLayer::startRender( QgsSymbolV2RenderContext& context )
{
  if ( mSvgViewBox.isNull() )
  {
    return;
  }

  //create QImage with appropriate dimensions
  int pixelWidth = context.outputPixelSize( mPatternWidth );//mPatternWidth.value( context, QgsOutputUnit::Pixel );
  int pixelHeight = pixelWidth / mSvgViewBox.width() * mSvgViewBox.height();

  QImage textureImage( pixelWidth, pixelHeight, QImage::Format_ARGB32_Premultiplied );
  textureImage.fill( QColor( 255, 255, 255, 0 ).rgba() );

  //rasterise byte array to image
  QPainter p( &textureImage );
  QSvgRenderer r( mSvgData );
  if ( !r.isValid() )
  {
    return;
  }
  r.render( &p );

  QTransform brushTransform;
  brushTransform.scale( 1.0 / context.renderContext().rasterScaleFactor(), 1.0 / context.renderContext().rasterScaleFactor() );
  mBrush.setTextureImage( textureImage );
  mBrush.setTransform( brushTransform );

  if ( mOutline )
  {
    mOutline->startRender( context.renderContext() );
  }
}

void QgsSVGFillSymbolLayer::stopRender( QgsSymbolV2RenderContext& context )
{
  if ( mOutline )
  {
    mOutline->stopRender( context.renderContext() );
  }
}

void QgsSVGFillSymbolLayer::renderPolygon( const QPolygonF& points, QList<QPolygonF>* rings, QgsSymbolV2RenderContext& context )
{
  QPainter* p = context.renderContext().painter();
  if ( !p )
  {
    return;
  }
  p->setBrush( mBrush );
  p->setPen( QPen( Qt::NoPen ) );
  _renderPolygon( p, points, rings );
  if ( mOutline )
  {
    mOutline->renderPolyline( points, context.renderContext() );
    if ( rings )
    {
      QList<QPolygonF>::const_iterator ringIt = rings->constBegin();
      for ( ; ringIt != rings->constEnd(); ++ringIt )
      {
        mOutline->renderPolyline( *ringIt, context.renderContext() );
      }
    }
  }
}

QgsStringMap QgsSVGFillSymbolLayer::properties() const
{
  QgsStringMap map;
  if ( !mSvgFilePath.isEmpty() )
  {
    map.insert( "svgFile", QgsProject::instance()->writePath( mSvgFilePath ) );
  }
  else
  {
    map.insert( "data", QString( mSvgData.toHex() ) );
  }

  map.insert( "width", QString::number( mPatternWidth ) );
  return map;
}

QgsSymbolLayerV2* QgsSVGFillSymbolLayer::clone() const
{
  QgsSymbolLayerV2* clonedLayer = 0;
  if ( !mSvgFilePath.isEmpty() )
  {
    clonedLayer = new QgsSVGFillSymbolLayer( mSvgFilePath, mPatternWidth );
  }
  else
  {
    clonedLayer = new QgsSVGFillSymbolLayer( mSvgData, mPatternWidth );
  }

  if ( mOutline )
  {
    clonedLayer->setSubSymbol( mOutline->clone() );
  }
  return clonedLayer;
}

void QgsSVGFillSymbolLayer::storeViewBox()
{
  if ( !mSvgData.isEmpty() )
  {
    QSvgRenderer r( mSvgData );
    if ( r.isValid() )
    {
      mSvgViewBox = r.viewBoxF();
      return;
    }
  }

  mSvgViewBox = QRectF();
  return;
}

bool QgsSVGFillSymbolLayer::setSubSymbol( QgsSymbolV2* symbol )
{

  if ( !symbol || symbol->type() != QgsSymbolV2::Line )
  {
    delete symbol;
    return false;
  }


  QgsLineSymbolV2* lineSymbol = dynamic_cast<QgsLineSymbolV2*>( symbol );
  if ( lineSymbol )
  {
    delete mOutline;
    mOutline = lineSymbol;
    return true;
  }

  delete symbol;
  return false;
}

#include "qgsfillsymbollayerv2.h"
#include "qgsmarkersymbollayerv2.h"
#include "qgssymbollayerv2utils.h"

#include "qgsrendercontext.h"
#include "qgsproject.h"
#include "qgssvgcache.h"

#include <QPainter>
#include <QFile>
#include <QSvgRenderer>

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
  QPointF offset;

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
  if ( props.contains( "offset" ) )
    offset = QgsSymbolLayerV2Utils::decodePoint( props["offset"] );

  QgsSimpleFillSymbolLayerV2* sl = new QgsSimpleFillSymbolLayerV2( color, style, borderColor, borderStyle, borderWidth );
  sl->setOffset( offset );
  return sl;
}


QString QgsSimpleFillSymbolLayerV2::layerType() const
{
  return "SimpleFill";
}

void QgsSimpleFillSymbolLayerV2::startRender( QgsSymbolV2RenderContext& context )
{
  mColor.setAlphaF( context.alpha() );
  mBrush = QBrush( mColor, mBrushStyle );

  // scale brush content for printout
  double rasterScaleFactor = context.renderContext().rasterScaleFactor();
  if ( rasterScaleFactor != 1.0 )
  {
    mBrush.setMatrix( QMatrix().scale( 1.0 / rasterScaleFactor, 1.0 / rasterScaleFactor ) );
  }

  QColor selColor = context.selectionColor();
  // selColor.setAlphaF( context.alpha() );
  mSelBrush = QBrush( selColor );
  if ( selectFillStyle )
    mSelBrush.setStyle( mBrushStyle );
  mBorderColor.setAlphaF( context.alpha() );
  mPen = QPen( mBorderColor );
  mPen.setStyle( mBorderStyle );
  mPen.setWidthF( context.outputLineWidth( mBorderWidth ) );
}

void QgsSimpleFillSymbolLayerV2::stopRender( QgsSymbolV2RenderContext& context )
{
  Q_UNUSED( context );
}

void QgsSimpleFillSymbolLayerV2::renderPolygon( const QPolygonF& points, QList<QPolygonF>* rings, QgsSymbolV2RenderContext& context )
{
  QPainter* p = context.renderContext().painter();
  if ( !p )
  {
    return;
  }

  p->setBrush( context.selected() ? mSelBrush : mBrush );
  p->setPen( mPen );

  if ( !mOffset.isNull() )
    p->translate( mOffset );

  _renderPolygon( p, points, rings );

  if ( !mOffset.isNull() )
    p->translate( -mOffset );
}

QgsStringMap QgsSimpleFillSymbolLayerV2::properties() const
{
  QgsStringMap map;
  map["color"] = QgsSymbolLayerV2Utils::encodeColor( mColor );
  map["style"] = QgsSymbolLayerV2Utils::encodeBrushStyle( mBrushStyle );
  map["color_border"] = QgsSymbolLayerV2Utils::encodeColor( mBorderColor );
  map["style_border"] = QgsSymbolLayerV2Utils::encodePenStyle( mBorderStyle );
  map["width_border"] = QString::number( mBorderWidth );
  map["offset"] = QgsSymbolLayerV2Utils::encodePoint( mOffset );
  return map;
}

QgsSymbolLayerV2* QgsSimpleFillSymbolLayerV2::clone() const
{
  QgsSimpleFillSymbolLayerV2* sl = new QgsSimpleFillSymbolLayerV2( mColor, mBrushStyle, mBorderColor, mBorderStyle, mBorderWidth );
  sl->setOffset( mOffset );
  return sl;
}

//QgsImageFillSymbolLayer

QgsImageFillSymbolLayer::QgsImageFillSymbolLayer(): mOutlineWidth( 0.0 ), mOutline( 0 )
{
  setSubSymbol( new QgsLineSymbolV2() );
}

QgsImageFillSymbolLayer::~QgsImageFillSymbolLayer()
{
}

void QgsImageFillSymbolLayer::renderPolygon( const QPolygonF& points, QList<QPolygonF>* rings, QgsSymbolV2RenderContext& context )
{
  QPainter* p = context.renderContext().painter();
  if ( !p )
  {
    return;
  }
  p->setPen( QPen( Qt::NoPen ) );
  if ( context.selected() )
  {
    QColor selColor = context.selectionColor();
    if ( ! selectionIsOpaque )
      selColor.setAlphaF( context.alpha() );
    p->setBrush( QBrush( selColor ) );
    _renderPolygon( p, points, rings );
  }

  if ( doubleNear( mAngle, 0.0 ) )
  {
    p->setBrush( mBrush );
  }
  else
  {
    QTransform t = mBrush.transform();
    t.rotate( mAngle );
    QBrush rotatedBrush = mBrush;
    rotatedBrush.setTransform( t );
    p->setBrush( rotatedBrush );
  }
  _renderPolygon( p, points, rings );
  if ( mOutline )
  {
    mOutline->renderPolyline( points, context.feature(), context.renderContext(), -1, selectFillBorder && context.selected() );
    if ( rings )
    {
      QList<QPolygonF>::const_iterator ringIt = rings->constBegin();
      for ( ; ringIt != rings->constEnd(); ++ringIt )
      {
        mOutline->renderPolyline( *ringIt, context.feature(), context.renderContext(), -1, selectFillBorder && context.selected() );
      }
    }
  }
}

bool QgsImageFillSymbolLayer::setSubSymbol( QgsSymbolV2* symbol )
{
  if( !symbol ) //unset current outline
  {
    delete mOutline;
    mOutline = 0;
    return true;
  }

  if ( symbol->type() != QgsSymbolV2::Line )
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

//QgsSVGFillSymbolLayer

QgsSVGFillSymbolLayer::QgsSVGFillSymbolLayer( const QString& svgFilePath, double width, double angle ): QgsImageFillSymbolLayer(), mPatternWidth( width )
{
  setSvgFilePath( svgFilePath );
  mOutlineWidth = 0.3;
  mAngle = angle;
}

QgsSVGFillSymbolLayer::QgsSVGFillSymbolLayer( const QByteArray& svgData, double width, double angle ): QgsImageFillSymbolLayer(), mPatternWidth( width ),
    mSvgData( svgData )
{
  storeViewBox();
  mOutlineWidth = 0.3;
  mAngle = angle;
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
  double angle = 0.0;


  if ( properties.contains( "width" ) )
  {
    width = properties["width"].toDouble();
  }
  if ( properties.contains( "svgFile" ) )
  {
    QString svgName = properties["svgFile"];
    QString savePath = QgsSvgMarkerSymbolLayerV2::symbolNameToPath( svgName );
    svgFilePath = ( savePath.isEmpty() ? svgName : savePath );
  }
  if ( properties.contains( "angle" ) )
  {
    angle = properties["angle"].toDouble();
  }

  if ( !svgFilePath.isEmpty() )
  {
    return new QgsSVGFillSymbolLayer( svgFilePath, width, angle );
  }
  else
  {
    if ( properties.contains( "data" ) )
    {
      data = QByteArray::fromHex( properties["data"].toLocal8Bit() );
    }

    return new QgsSVGFillSymbolLayer( data, width, angle );
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

  QColor fillColor( Qt::black );
  QColor outlineColor( Qt::black );
  double outlineWidth = 1;
  int size = context.outputPixelSize( mPatternWidth );
  const QImage& patternImage = QgsSvgCache::instance()->svgAsImage( mSvgFilePath, size, fillColor, outlineColor, outlineWidth,
                               context.renderContext().scaleFactor(), context.renderContext().rasterScaleFactor() );
  QTransform brushTransform;
  brushTransform.scale( 1.0 / context.renderContext().rasterScaleFactor(), 1.0 / context.renderContext().rasterScaleFactor() );
  if ( !doubleNear( context.alpha(), 1.0 ) )
  {
    QImage transparentImage = patternImage.copy();
    QgsSymbolLayerV2Utils::multiplyImageOpacity( &transparentImage, context.alpha() );
    mBrush.setTextureImage( transparentImage );
  }
  else
  {
    mBrush.setTextureImage( patternImage );
  }
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

QgsStringMap QgsSVGFillSymbolLayer::properties() const
{
  QgsStringMap map;
  if ( !mSvgFilePath.isEmpty() )
  {
    map.insert( "svgFile", QgsSvgMarkerSymbolLayerV2::symbolPathToName( mSvgFilePath ) );
  }
  else
  {
    map.insert( "data", QString( mSvgData.toHex() ) );
  }

  map.insert( "width", QString::number( mPatternWidth ) );
  map.insert( "angle", QString::number( mAngle ) );
  return map;
}

QgsSymbolLayerV2* QgsSVGFillSymbolLayer::clone() const
{
  QgsSymbolLayerV2* clonedLayer = 0;
  if ( !mSvgFilePath.isEmpty() )
  {
    clonedLayer = new QgsSVGFillSymbolLayer( mSvgFilePath, mPatternWidth, mAngle );
  }
  else
  {
    clonedLayer = new QgsSVGFillSymbolLayer( mSvgData, mPatternWidth, mAngle );
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

QgsLinePatternFillSymbolLayer::QgsLinePatternFillSymbolLayer(): QgsImageFillSymbolLayer()
{
}

QgsLinePatternFillSymbolLayer::~QgsLinePatternFillSymbolLayer()
{
}

QgsSymbolLayerV2* QgsLinePatternFillSymbolLayer::create( const QgsStringMap& properties )
{
  QgsLinePatternFillSymbolLayer* patternLayer = new QgsLinePatternFillSymbolLayer();

  //default values
  double angle = 45;
  double distance = 5;
  double lineWidth = 0.5;
  QColor color( Qt::black );

  if ( properties.contains( "angle" ) )
  {
    angle = properties["angle"].toDouble();
  }
  patternLayer->setAngle( angle );

  if ( properties.contains( "distance" ) )
  {
    distance = properties["distance"].toDouble();
  }
  patternLayer->setDistance( distance );

  if ( properties.contains( "linewidth" ) )
  {
    lineWidth = properties["linewidth"].toDouble();
  }
  patternLayer->setLineWidth( lineWidth );

  if ( properties.contains( "color" ) )
  {
    color = QgsSymbolLayerV2Utils::decodeColor( properties["color"] );
  }
  patternLayer->setColor( color );
  return patternLayer;
}

QString QgsLinePatternFillSymbolLayer::layerType() const
{
  return "LinePatternFill";
}

void QgsLinePatternFillSymbolLayer::startRender( QgsSymbolV2RenderContext& context )
{
  //create image
  int height, width;
  if ( doubleNear( mAngle, 0 ) || doubleNear( mAngle, 360 ) || doubleNear( mAngle, 90 ) || doubleNear( mAngle, 180 ) || doubleNear( mAngle, 270 ) )
  {
    height = context.outputPixelSize( mDistance );
    width = height; //width can be set to arbitrary value
  }
  else
  {
    height = fabs( context.outputPixelSize( mDistance ) / cos( mAngle * M_PI / 180 ) ); //keep perpendicular distance between lines constant
    width = fabs( height / tan( mAngle * M_PI / 180 ) );
  }

  double outlinePixelWidth = context.outputPixelSize( mLineWidth );

  //find a suitable multiple of width and heigh

  QImage patternImage( fabs( width ), height, QImage::Format_ARGB32 );
  patternImage.fill( 0 );
  QPainter p( &patternImage );
  p.setRenderHint( QPainter::Antialiasing, true );
  QPen pen( mColor );
  pen.setWidthF( outlinePixelWidth );
  pen.setCapStyle( Qt::FlatCap );
  p.setPen( pen );

  //draw line and dots in the border
  if ( doubleNear( mAngle, 0.0 ) || doubleNear( mAngle, 360.0 ) || doubleNear( mAngle, 180.0 ) )
  {
    p.drawLine( QPointF( 0, height / 2.0 ), QPointF( width, height / 2.0 ) );
  }
  else if ( doubleNear( mAngle, 90.0 ) || doubleNear( mAngle, 270.0 ) )
  {
    p.drawLine( QPointF( width / 2.0, 0 ), QPointF( width / 2.0, height ) );
  }
  else if (( mAngle > 0 && mAngle < 90 ) || ( mAngle > 180 && mAngle < 270 ) )
  {
    p.drawLine( QPointF( 0, height ), QPointF( width, 0 ) );
  }
  else if (( mAngle < 180 ) || ( mAngle > 270 && mAngle < 360 ) )
  {
    p.drawLine( QPointF( width, height ), QPointF( 0, 0 ) );
  }

  //todo: calculate triangles more accurately
  double d1 = 0;
  double d2 = 0;
  QPolygonF triangle1, triangle2;
  if ( mAngle > 0 && mAngle < 90 )
  {
    d1 = ( outlinePixelWidth / 2.0 ) / cos( mAngle * M_PI / 180 );
    d2 = ( outlinePixelWidth / 2.0 ) / cos(( 90 - mAngle ) * M_PI / 180 );
    triangle1 << QPointF( 0, 0 ) << QPointF( 0, d1 ) << QPointF( d2, 0 ) << QPointF( 0, 0 );
    triangle2 << QPointF( width, height ) << QPointF( width - d2, height ) << QPointF( width, height - d1 ) << QPointF( width, height );
  }
  else if ( mAngle > 90 && mAngle < 180 )
  {
    d1 = ( outlinePixelWidth / 2.0 ) / cos(( mAngle - 90 ) * M_PI / 180 );
    d2 = ( outlinePixelWidth / 2.0 ) / cos(( 180 - mAngle ) * M_PI / 180 );
    triangle1 << QPointF( width, 0 ) << QPointF( width - d1, 0 ) << QPointF( width, d2 ) << QPointF( width, 0 );
    triangle2 << QPointF( 0, height ) << QPointF( 0, height - d2 ) << QPointF( d1, height ) << QPointF( 0, height );
  }
  else if ( mAngle > 180 && mAngle < 270 )
  {
    d1 = ( outlinePixelWidth / 2.0 ) / cos(( mAngle - 180 ) * M_PI / 180 );
    d2 = ( outlinePixelWidth / 2.0 ) / cos(( 270 - mAngle ) * M_PI / 180 );
    triangle1 << QPointF( 0, 0 ) << QPointF( 0, d1 ) << QPointF( d2, 0 ) << QPointF( 0, 0 );
    triangle2 << QPointF( width, height ) << QPointF( width - d2, height ) << QPointF( width, height - d1 ) << QPointF( width, height );
  }
  else if ( mAngle > 270 && mAngle < 360 )
  {
    d1 = ( outlinePixelWidth / 2.0 ) / cos(( mAngle - 270 ) * M_PI / 180 );
    d2 = ( outlinePixelWidth / 2.0 ) / cos(( 360 - mAngle ) * M_PI / 180 );
    triangle1 << QPointF( width, 0 ) << QPointF( width - d1, 0 ) << QPointF( width, d2 ) << QPointF( width, 0 );
    triangle2 << QPointF( 0, height ) << QPointF( 0, height - d2 ) << QPointF( d1, height ) << QPointF( 0, height );
  }

  p.setPen( QPen( Qt::NoPen ) );
  p.setBrush( QBrush( mColor ) );
  p.drawPolygon( triangle1 );
  p.drawPolygon( triangle2 );
  p.end();

  //set image to mBrush
  if ( !doubleNear( context.alpha(), 1.0 ) )
  {
    QImage transparentImage = patternImage.copy();
    QgsSymbolLayerV2Utils::multiplyImageOpacity( &transparentImage, context.alpha() );
    mBrush.setTextureImage( transparentImage );
  }
  else
  {
    mBrush.setTextureImage( patternImage );
  }

  QTransform brushTransform;
  brushTransform.scale( 1.0 / context.renderContext().rasterScaleFactor(), 1.0 / context.renderContext().rasterScaleFactor() );
  mBrush.setTransform( brushTransform );

  if ( mOutline )
  {
    mOutline->startRender( context.renderContext() );
  }
}

void QgsLinePatternFillSymbolLayer::stopRender( QgsSymbolV2RenderContext & )
{
}

QgsStringMap QgsLinePatternFillSymbolLayer::properties() const
{
  QgsStringMap map;
  map.insert( "angle", QString::number( mAngle ) );
  map.insert( "distance", QString::number( mDistance ) );
  map.insert( "linewidth", QString::number( mLineWidth ) );
  map.insert( "color", QgsSymbolLayerV2Utils::encodeColor( mColor ) );
  return map;
}

QgsSymbolLayerV2* QgsLinePatternFillSymbolLayer::clone() const
{
  QgsSymbolLayerV2* clonedLayer = QgsLinePatternFillSymbolLayer::create( properties() );
  if ( mOutline )
  {
    clonedLayer->setSubSymbol( mOutline->clone() );
  }
  return clonedLayer;
}

////////////////////////

QgsPointPatternFillSymbolLayer::QgsPointPatternFillSymbolLayer(): QgsImageFillSymbolLayer(), mMarkerSymbol( 0 ), mDistanceX( 15 ),
  mDistanceY( 15 ), mDisplacementX( 0 ), mDisplacementY( 0 )
{
  mDistanceX = 15;
  mDistanceY = 15;
  mDisplacementX = 0;
  mDisplacementY = 0;
  setSubSymbol( new QgsMarkerSymbolV2() );
  QgsImageFillSymbolLayer::setSubSymbol( 0 ); //no outline
}

QgsPointPatternFillSymbolLayer::~QgsPointPatternFillSymbolLayer()
{
}

QgsSymbolLayerV2* QgsPointPatternFillSymbolLayer::create( const QgsStringMap& properties )
{
  QgsPointPatternFillSymbolLayer* layer = new QgsPointPatternFillSymbolLayer();
  if( properties.contains("distance_x") )
  {
    layer->setDistanceX( properties["distance_x"].toDouble() );
  }
  if( properties.contains("distance_y") )
  {
    layer->setDistanceY( properties["distance_y"].toDouble() );
  }
  if( properties.contains("displacement_x") )
  {
    layer->setDisplacementX( properties["displacement_x"].toDouble() );
  }
  if( properties.contains("displacement_y") )
  {
    layer->setDisplacementY( properties["displacement_y"].toDouble() );
  }
  return layer;
}

QString QgsPointPatternFillSymbolLayer::layerType() const
{
  return "PointPatternFill";
}

void QgsPointPatternFillSymbolLayer::startRender( QgsSymbolV2RenderContext& context )
{
  //render 3 rows and columns in one go to easily incorporate displacement
  double width = context.outputPixelSize( mDistanceX ) * 2.0;
  double height = context.outputPixelSize( mDistanceY ) * 2.0;

  QImage patternImage( width, height, QImage::Format_ARGB32 );
  patternImage.fill( 0 );

  if( mMarkerSymbol )
  {
    QPainter p( &patternImage );

    //marker rendering needs context for drawing on patternImage
    QgsRenderContext pointRenderContext;
    pointRenderContext.setPainter( &p );
    pointRenderContext.setRasterScaleFactor( 1.0 );
    pointRenderContext.setScaleFactor( context.renderContext().scaleFactor() * context.renderContext().rasterScaleFactor() );
    QgsMapToPixel mtp( context.renderContext().mapToPixel().mapUnitsPerPixel() / context.renderContext().rasterScaleFactor() );
    pointRenderContext.setMapToPixel( mtp );
    pointRenderContext.setForceVectorOutput( false );

    mMarkerSymbol->setOutputUnit( context.outputUnit() );
    mMarkerSymbol->startRender( pointRenderContext );

    //render corner points
    mMarkerSymbol->renderPoint( QPointF( 0, 0 ), context.feature(), pointRenderContext );
    mMarkerSymbol->renderPoint( QPointF( width, 0 ), context.feature(), pointRenderContext );
    mMarkerSymbol->renderPoint( QPointF( 0, height ), context.feature(), pointRenderContext );
    mMarkerSymbol->renderPoint( QPointF( width, height ), context.feature(), pointRenderContext );

    //render displaced points
    double displacementPixelX = context.outputPixelSize( mDisplacementX );
    double displacementPixelY = context.outputPixelSize( mDisplacementY );
    mMarkerSymbol->renderPoint( QPointF( width / 2.0, -displacementPixelY ), context.feature(), pointRenderContext );
    mMarkerSymbol->renderPoint( QPointF( displacementPixelX, height / 2.0 ), context.feature(), pointRenderContext );
    mMarkerSymbol->renderPoint( QPointF( width / 2.0 + displacementPixelX, height / 2.0 - displacementPixelY ), context.feature(), pointRenderContext );
    mMarkerSymbol->renderPoint( QPointF( width + displacementPixelX, height / 2.0 ), context.feature(), pointRenderContext );
    mMarkerSymbol->renderPoint( QPointF( width / 2.0, height - displacementPixelY), context.feature(), pointRenderContext );

    mMarkerSymbol->stopRender( pointRenderContext );
  }

  if ( !doubleNear( context.alpha(), 1.0 ) )
  {
    QImage transparentImage = patternImage.copy();
    QgsSymbolLayerV2Utils::multiplyImageOpacity( &transparentImage, context.alpha() );
    mBrush.setTextureImage( transparentImage );
  }
  else
  {
    mBrush.setTextureImage( patternImage );
  }
  QTransform brushTransform;
  brushTransform.scale( 1.0 / context.renderContext().rasterScaleFactor(), 1.0 / context.renderContext().rasterScaleFactor() );
  mBrush.setTransform( brushTransform );

  if ( mOutline )
  {
    mOutline->startRender( context.renderContext() );
  }
}

void QgsPointPatternFillSymbolLayer::stopRender( QgsSymbolV2RenderContext& context )
{
  if ( mOutline )
  {
    mOutline->stopRender( context.renderContext() );
  }
}

QgsStringMap QgsPointPatternFillSymbolLayer::properties() const
{
  QgsStringMap propertyMap;
  propertyMap["distance_x"] = QString::number( mDistanceX );
  propertyMap["distance_y"] = QString::number( mDistanceY );
  propertyMap["displacement_x"] = QString::number( mDisplacementX );
  propertyMap["displacement_y"] = QString::number( mDisplacementY );
  return propertyMap;
}

QgsSymbolLayerV2* QgsPointPatternFillSymbolLayer::clone() const
{
  QgsSymbolLayerV2* clonedLayer = QgsPointPatternFillSymbolLayer::create( properties() );
  if( mMarkerSymbol )
  {
    clonedLayer->setSubSymbol( mMarkerSymbol->clone() );
  }
  return clonedLayer;
}

bool QgsPointPatternFillSymbolLayer::setSubSymbol( QgsSymbolV2* symbol )
{
  if( !symbol )
  {
    return false;
  }

  if(symbol->type() == QgsSymbolV2::Marker )
  {
    QgsMarkerSymbolV2* markerSymbol = static_cast<QgsMarkerSymbolV2*>( symbol );
    delete mMarkerSymbol;
    mMarkerSymbol = markerSymbol;
  }
  return true;
}

//////////////


QgsCentroidFillSymbolLayerV2::QgsCentroidFillSymbolLayerV2()
{
  mMarker = NULL;
  setSubSymbol( new QgsMarkerSymbolV2() );
}

QgsCentroidFillSymbolLayerV2::~QgsCentroidFillSymbolLayerV2()
{
  delete mMarker;
}

QgsSymbolLayerV2* QgsCentroidFillSymbolLayerV2::create( const QgsStringMap& /*properties*/ )
{
  return new QgsCentroidFillSymbolLayerV2();
}

QString QgsCentroidFillSymbolLayerV2::layerType() const
{
  return "CentroidFill";
}

void QgsCentroidFillSymbolLayerV2::setColor( const QColor& color )
{
  mMarker->setColor( color );
  mColor = color;
}

void QgsCentroidFillSymbolLayerV2::startRender( QgsSymbolV2RenderContext& context )
{
  mMarker->setAlpha( context.alpha() );
  mMarker->setOutputUnit( context.outputUnit() );

  mMarker->startRender( context.renderContext() );
}

void QgsCentroidFillSymbolLayerV2::stopRender( QgsSymbolV2RenderContext& context )
{
  mMarker->stopRender( context.renderContext() );
}

void QgsCentroidFillSymbolLayerV2::renderPolygon( const QPolygonF& points, QList<QPolygonF>* rings, QgsSymbolV2RenderContext& context )
{
  Q_UNUSED( rings );

  // calculate centroid
  double cx = 0, cy = 0;
  double area, sum = 0;
  for ( int i = points.count() - 1, j = 0; j < points.count(); i = j++ )
  {
    const QPointF& p1 = points[i];
    const QPointF& p2 = points[j];
    area = p1.x() * p2.y() - p1.y() * p2.x();
    sum += area;
    cx += ( p1.x() + p2.x() ) * area;
    cy += ( p1.y() + p2.y() ) * area;
  }
  sum *= 3.0;
  cx /= sum;
  cy /= sum;

  mMarker->renderPoint( QPointF( cx, cy ), context.feature(), context.renderContext(), -1, context.selected() );
}

QgsStringMap QgsCentroidFillSymbolLayerV2::properties() const
{
  return QgsStringMap();
}

QgsSymbolLayerV2* QgsCentroidFillSymbolLayerV2::clone() const
{
  QgsCentroidFillSymbolLayerV2* x = new QgsCentroidFillSymbolLayerV2();
  x->setSubSymbol( mMarker->clone() );
  return x;
}


QgsSymbolV2* QgsCentroidFillSymbolLayerV2::subSymbol()
{
  return mMarker;
}

bool QgsCentroidFillSymbolLayerV2::setSubSymbol( QgsSymbolV2* symbol )
{
  if ( symbol == NULL || symbol->type() != QgsSymbolV2::Marker )
  {
    delete symbol;
    return false;
  }

  delete mMarker;
  mMarker = static_cast<QgsMarkerSymbolV2*>( symbol );
  mColor = mMarker->color();
  return true;
}

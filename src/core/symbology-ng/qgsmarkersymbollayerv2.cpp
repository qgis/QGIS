
#include "qgsmarkersymbollayerv2.h"
#include "qgssymbollayerv2utils.h"

#include "qgsrendercontext.h"
#include "qgsapplication.h"
#include "qgslogger.h"
#include "qgsproject.h"

#include <QPainter>
#include <QSvgRenderer>
#include <QFileInfo>
#include <QDir>

#include <cmath>

// MSVC compiler doesn't have defined M_PI in math.h
#ifndef M_PI
#define M_PI          3.14159265358979323846
#endif

#define DEG2RAD(x)    ((x)*M_PI/180)


QgsSimpleMarkerSymbolLayerV2::QgsSimpleMarkerSymbolLayerV2( QString name, QColor color, QColor borderColor, double size, double angle )
{
  mName = name;
  mColor = color;
  mBorderColor = borderColor;
  mSize = size;
  mAngle = angle;
  mOffset = QPointF( 0, 0 );
}

QgsSymbolLayerV2* QgsSimpleMarkerSymbolLayerV2::create( const QgsStringMap& props )
{
  QString name = DEFAULT_SIMPLEMARKER_NAME;
  QColor color = DEFAULT_SIMPLEMARKER_COLOR;
  QColor borderColor = DEFAULT_SIMPLEMARKER_BORDERCOLOR;
  double size = DEFAULT_SIMPLEMARKER_SIZE;
  double angle = DEFAULT_SIMPLEMARKER_ANGLE;

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

  QgsSimpleMarkerSymbolLayerV2* m = new QgsSimpleMarkerSymbolLayerV2( name, color, borderColor, size, angle );
  if ( props.contains( "offset" ) )
    m->setOffset( QgsSymbolLayerV2Utils::decodePoint( props["offset"] ) );
  return m;
}


QString QgsSimpleMarkerSymbolLayerV2::layerType() const
{
  return "SimpleMarker";
}

void QgsSimpleMarkerSymbolLayerV2::startRender( QgsSymbolV2RenderContext& context )
{
  mBrush = QBrush( mColor );
  mPen = QPen( mBorderColor );
  mPen.setWidthF( context.outputLineWidth( mPen.widthF() ) );

  QColor selColor = context.selectionColor();
  mSelBrush = QBrush( selColor );
  mSelPen = QPen( selColor == mColor ? selColor : mBorderColor );
  mSelPen.setWidthF( mPen.widthF() );

  bool hasDataDefinedRotation = context.renderHints() & QgsSymbolV2::DataDefinedRotation;
  bool hasDataDefinedSize = context.renderHints() & QgsSymbolV2::DataDefinedSizeScale;

  // use either QPolygonF or QPainterPath for drawing
  // TODO: find out whether drawing directly doesn't bring overhead - if not, use it for all shapes
  if ( !prepareShape() ) // drawing as a polygon
  {
    if ( preparePath() ) // drawing as a painter path
    {
      // some markers can't be drawn as a polygon (circle, cross)
      // For these set the selected border color to the selected color

      if ( mName != "circle" )
        mSelPen.setColor( selColor );
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
    double scaledSize = context.outputPixelSize( mSize );
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

  if ( !hasDataDefinedRotation && !hasDataDefinedSize )
  {
    // we can use the cached marker
    // TODO: use caching only when drawing to screen (not printer)
    prepareCache( context );
  }
  else
  {
    mCache = QImage();
    mSelCache = QImage();
  }
}


void QgsSimpleMarkerSymbolLayerV2::prepareCache( QgsSymbolV2RenderContext& context )
{
  double scaledSize = context.outputPixelSize( mSize );

  // calculate necessary image size for the cache
  double pw = (( mPen.widthF() == 0 ? 1 : mPen.widthF() ) + 1 ) / 2 * 2; // make even (round up); handle cosmetic pen
  int imageSize = (( int ) scaledSize + pw ) / 2 * 2 + 1; //  make image width, height odd; account for pen width

  double center = (( double ) imageSize / 2 ) + 0.5; // add 1/2 pixel for proper rounding when the figure's coordinates are added

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
  // filling the background with the selection colour and using the normal
  // colours for the symbol .. could be ugly!

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

  //opacity
  if ( context.alpha() < 1.0 )
  {
    QgsSymbolLayerV2Utils::multiplyImageOpacity( &mCache, context.alpha() );
    if ( ! selectionIsOpaque ) QgsSymbolLayerV2Utils::multiplyImageOpacity( &mSelCache, context.alpha() );
  }
}

void QgsSimpleMarkerSymbolLayerV2::stopRender( QgsSymbolV2RenderContext& context )
{
}

bool QgsSimpleMarkerSymbolLayerV2::prepareShape()
{
  mPolygon.clear();

  if ( mName == "rectangle" )
  {
    mPolygon = QPolygonF( QRectF( QPointF( -1, -1 ), QPointF( 1, 1 ) ) );
    return true;
  }
  else if ( mName == "diamond" )
  {
    mPolygon << QPointF( -1, 0 ) << QPointF( 0, 1 )
    << QPointF( 1, 0 ) << QPointF( 0, -1 );
    return true;
  }
  else if ( mName == "pentagon" )
  {
    mPolygon << QPointF( sin( DEG2RAD( 288.0 ) ), - cos( DEG2RAD( 288.0 ) ) )
    << QPointF( sin( DEG2RAD( 216.0 ) ), - cos( DEG2RAD( 216.0 ) ) )
    << QPointF( sin( DEG2RAD( 144.0 ) ), - cos( DEG2RAD( 144.0 ) ) )
    << QPointF( sin( DEG2RAD( 72.0 ) ), - cos( DEG2RAD( 72.0 ) ) )
    << QPointF( 0, -1 );
    return true;
  }
  else if ( mName == "triangle" )
  {
    mPolygon << QPointF( -1, 1 ) << QPointF( 1, 1 ) << QPointF( 0, -1 );
    return true;
  }
  else if ( mName == "equilateral_triangle" )
  {
    mPolygon << QPointF( sin( DEG2RAD( 240.0 ) ), - cos( DEG2RAD( 240.0 ) ) )
    << QPointF( sin( DEG2RAD( 120.0 ) ), - cos( DEG2RAD( 120.0 ) ) )
    << QPointF( 0, -1 );
    return true;
  }
  else if ( mName == "star" )
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
  else if ( mName == "regular_star" )
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
  else if ( mName == "arrow" )
  {
    double eight = 1.0 / 4;
    double quarter = 1.0 / 2;

    mPolygon << QPointF( 0, -1 )
    << QPointF( 0.5,  -0.5 )
    << QPointF( 0.25, -0.25 )
    << QPointF( 0.25,  1 )
    << QPointF( -0.25,  1 )
    << QPointF( -0.25, -0.5 )
    << QPointF( -0.5,  -0.5 );
    return true;
  }

  return false;
}

bool QgsSimpleMarkerSymbolLayerV2::preparePath()
{
  mPath = QPainterPath();

  if ( mName == "circle" )
  {
    mPath.addEllipse( QRectF( -1, -1, 2, 2 ) ); // x,y,w,h
    return true;
  }
  else if ( mName == "cross" )
  {
    mPath.moveTo( -1, 0 );
    mPath.lineTo( 1, 0 ); // horizontal
    mPath.moveTo( 0, -1 );
    mPath.lineTo( 0, 1 ); // vertical
    return true;
  }
  else if ( mName == "cross2" )
  {
    mPath.moveTo( -1, -1 );
    mPath.lineTo( 1, 1 );
    mPath.moveTo( 1, -1 );
    mPath.lineTo( -1, 1 );
    return true;
  }
  else if ( mName == "line" )
  {
    mPath.moveTo( 0, -1 );
    mPath.lineTo( 0, 1 ); // vertical line
    return true;
  }

  return false;
}

void QgsSimpleMarkerSymbolLayerV2::renderPoint( const QPointF& point, QgsSymbolV2RenderContext& context )
{
  QgsRenderContext& rc = context.renderContext();
  QPainter* p = rc.painter();
  if ( !p )
  {
    return;
  }

  bool hasDataDefinedRotation = context.renderHints() & QgsSymbolV2::DataDefinedRotation;
  bool hasDataDefinedSize = context.renderHints() & QgsSymbolV2::DataDefinedSizeScale;

  if ( !hasDataDefinedRotation && !hasDataDefinedSize )
  {
    // we will use cached image
    QImage &img = context.selected() ? mSelCache : mCache;
    double s = img.width() / context.renderContext().rasterScaleFactor();
    p->drawImage( QRectF( point.x() - s / 2.0 + context.outputLineWidth( mOffset.x() ),
                          point.y() - s / 2.0 + context.outputLineWidth( mOffset.y() ),
                          s, s ), img );
  }
  else
  {
    QMatrix transform;

    // move to the desired position
    transform.translate( point.x() + context.outputLineWidth( mOffset.x() ),
                         point.y() + context.outputLineWidth( mOffset.y() ) );

    // resize if necessary
    if ( hasDataDefinedSize )
    {
      double scaledSize = context.outputPixelSize( mSize );
      double half = scaledSize / 2.0;
      transform.scale( half, half );
    }

    // rotate if necessary
    if ( mAngle != 0 )
    {
      transform.rotate( mAngle );
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
  map["angle"] = QString::number( mAngle );
  map["offset"] = QgsSymbolLayerV2Utils::encodePoint( mOffset );
  return map;
}

QgsSymbolLayerV2* QgsSimpleMarkerSymbolLayerV2::clone() const
{
  QgsSimpleMarkerSymbolLayerV2* m = new QgsSimpleMarkerSymbolLayerV2( mName, mColor, mBorderColor, mSize, mAngle );
  m->setOffset( mOffset );
  return m;
}

void QgsSimpleMarkerSymbolLayerV2::drawMarker( QPainter* p, QgsSymbolV2RenderContext& context )
{
  if ( mPolygon.count() != 0 )
  {
    p->drawPolygon( mPolygon );
  }
  else
  {
    p->drawPath( mPath );
  }
}


//////////


QgsSvgMarkerSymbolLayerV2::QgsSvgMarkerSymbolLayerV2( QString name, double size, double angle )
{
  mPath = symbolNameToPath( name );
  mSize = size;
  mAngle = angle;
  mOffset = QPointF( 0, 0 );
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
  if ( props.contains( "offset" ) )
    m->setOffset( QgsSymbolLayerV2Utils::decodePoint( props["offset"] ) );
  return m;
}


QString QgsSvgMarkerSymbolLayerV2::layerType() const
{
  return "SvgMarker";
}

void QgsSvgMarkerSymbolLayerV2::startRender( QgsSymbolV2RenderContext& context )
{
  double pictureSize = 0;
  QgsRenderContext& rc = context.renderContext();

  if ( rc.painter() && rc.painter()->device() )
  {
    //correct QPictures DPI correction
    pictureSize = context.outputLineWidth( mSize ) / rc.painter()->device()->logicalDpiX() * mPicture.logicalDpiX();
  }
  else
  {
    pictureSize = context.outputLineWidth( mSize );
  }
  QRectF rect( QPointF( -pictureSize / 2.0, -pictureSize / 2.0 ), QSizeF( pictureSize, pictureSize ) );
  QSvgRenderer renderer( mPath );
  QPainter painter( &mPicture );
  renderer.render( &painter, rect );
  QPainter selPainter( &mSelPicture );
  selPainter.setRenderHint( QPainter::Antialiasing );
  selPainter.setBrush( QBrush( context.selectionColor() ) );
  selPainter.setPen( Qt::NoPen );
  selPainter.drawEllipse( QPointF( 0, 0 ), pictureSize*0.6, pictureSize*0.6 );
  renderer.render( &selPainter, rect );

  mOrigSize = mSize; // save in case the size would be data defined
}

void QgsSvgMarkerSymbolLayerV2::stopRender( QgsSymbolV2RenderContext& context )
{
}


void QgsSvgMarkerSymbolLayerV2::renderPoint( const QPointF& point, QgsSymbolV2RenderContext& context )
{
  QPainter* p = context.renderContext().painter();
  if ( !p )
  {
    return;
  }

  p->save();
  QPointF outputOffset = QPointF( context.outputLineWidth( mOffset.x() ), context.outputLineWidth( mOffset.y() ) );
  p->translate( point + outputOffset );

  if ( context.renderHints() & QgsSymbolV2::DataDefinedSizeScale )
  {
    double s = mSize / mOrigSize;
    p->scale( s, s );
  }

  if ( mAngle != 0 )
    p->rotate( mAngle );

  QPicture &pct = context.selected() ? mSelPicture : mPicture;
  p->drawPicture( 0, 0, pct );

  p->restore();
}


QgsStringMap QgsSvgMarkerSymbolLayerV2::properties() const
{
  QgsStringMap map;
  map["name"] = symbolPathToName( mPath );
  map["size"] = QString::number( mSize );
  map["angle"] = QString::number( mAngle );
  map["offset"] = QgsSymbolLayerV2Utils::encodePoint( mOffset );
  return map;
}

QgsSymbolLayerV2* QgsSvgMarkerSymbolLayerV2::clone() const
{
  QgsSvgMarkerSymbolLayerV2* m = new QgsSvgMarkerSymbolLayerV2( mPath, mSize, mAngle );
  m->setOffset( mOffset );
  return m;
}


QStringList QgsSvgMarkerSymbolLayerV2::listSvgFiles()
{
  // copied from QgsMarkerCatalogue - TODO: unify
  QStringList list;
  QStringList svgPaths = QgsApplication::svgPaths();

  for ( int i = 0; i < svgPaths.size(); i++ )
  {
    QDir dir( svgPaths[i] );
    foreach( QString item, dir.entryList( QDir::Dirs | QDir::NoDotAndDotDot ) )
    {
      svgPaths.insert( i + 1, dir.path() + "/" + item );
    }

    foreach( QString item, dir.entryList( QStringList( "*.svg" ), QDir::Files ) )
    {
      // TODO test if it is correct SVG
      list.append( dir.path() + "/" + item );
    }
  }
  return list;
}

QString QgsSvgMarkerSymbolLayerV2::symbolNameToPath( QString name )
{
  // copied from QgsSymbol::setNamedPointSymbol - TODO: unify

  // we might have a full path...
  if ( QFile( name ).exists() )
    return QFileInfo( name ).canonicalFilePath();

  // SVG symbol not found - probably a relative path was used

  QStringList svgPaths = QgsApplication::svgPaths();
  for ( int i = 0; i < svgPaths.size(); i++ )
  {
    QgsDebugMsg( "SvgPath: " + svgPaths[i] );
    QFileInfo myInfo( name );
    QString myFileName = myInfo.fileName(); // foo.svg
    QString myLowestDir = myInfo.dir().dirName();
    QString myLocalPath = svgPaths[i] + "/" + myLowestDir + "/" + myFileName;

    QgsDebugMsg( "Alternative svg path: " + myLocalPath );
    if ( QFile( myLocalPath ).exists() )
    {
      QgsDebugMsg( "Svg found in alternative path" );
      return QFileInfo( myLocalPath ).canonicalFilePath();
    }
    else if ( myInfo.isRelative() )
    {
      QFileInfo pfi( QgsProject::instance()->fileName() );
      QString alternatePath = pfi.canonicalPath() + QDir::separator() + name;
      if ( pfi.exists() && QFile( alternatePath ).exists() )
      {
        QgsDebugMsg( "Svg found in alternative path" );
        return QFileInfo( alternatePath ).canonicalFilePath();
      }
      else
      {
        QgsDebugMsg( "Svg not found in project path" );
      }
    }
    else
    {
      //couldnt find the file, no happy ending :-(
      QgsDebugMsg( "Computed alternate path but no svg there either" );
    }
  }
  return QString();
}

QString QgsSvgMarkerSymbolLayerV2::symbolPathToName( QString path )
{
  // copied from QgsSymbol::writeXML

  QFileInfo fi( path );
  if ( !fi.exists() )
    return path;

  path = fi.canonicalFilePath();

  QStringList svgPaths = QgsApplication::svgPaths();

  for ( int i = 0; i < svgPaths.size(); i++ )
  {
    QString dir = QFileInfo( svgPaths[i] ).canonicalFilePath();

    if ( !dir.isEmpty() && path.startsWith( dir ) )
    {
      path = path.mid( dir.size() );
      break;
    }
  }

  return path;
}


//////////

QgsFontMarkerSymbolLayerV2::QgsFontMarkerSymbolLayerV2( QString fontFamily, QChar chr, double pointSize, QColor color, double angle )
{
  mFontFamily = fontFamily;
  mChr = chr;
  mColor = color;
  mAngle = angle;
  mSize = pointSize;
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

  return new QgsFontMarkerSymbolLayerV2( fontFamily, chr, pointSize, color, angle );
}

QString QgsFontMarkerSymbolLayerV2::layerType() const
{
  return "FontMarker";
}

void QgsFontMarkerSymbolLayerV2::startRender( QgsSymbolV2RenderContext& context )
{
  mFont = QFont( mFontFamily, MM2POINT( mSize ) / context.renderContext().rasterScaleFactor() );
  QFontMetrics fm( mFont );
  mChrOffset = QPointF( fm.width( mChr ) / 2, -fm.ascent() / 2 );

  mOrigSize = mSize; // save in case the size would be data defined
}

void QgsFontMarkerSymbolLayerV2::stopRender( QgsSymbolV2RenderContext& context )
{
}

void QgsFontMarkerSymbolLayerV2::renderPoint( const QPointF& point, QgsSymbolV2RenderContext& context )
{
  QPainter* p = context.renderContext().painter();
  QColor penColor = context.selected() ? context.selectionColor() : mColor;
  penColor.setAlphaF( context.alpha() );
  p->setPen( penColor );
  p->setFont( mFont );

  p->save();
  p->translate( point );

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
  props["color"] = QgsSymbolLayerV2Utils::encodeColor( mColor );
  props["angle"] = QString::number( mAngle );
  return props;
}

QgsSymbolLayerV2* QgsFontMarkerSymbolLayerV2::clone() const
{
  return new QgsFontMarkerSymbolLayerV2( mFontFamily, mChr, mSize, mColor, mAngle );
}

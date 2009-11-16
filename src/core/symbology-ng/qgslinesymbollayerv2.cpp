
#include "qgslinesymbollayerv2.h"
#include "qgssymbollayerv2utils.h"

#include "qgsrendercontext.h"

#include <QPainter>

#include <cmath>

QgsSimpleLineSymbolLayerV2::QgsSimpleLineSymbolLayerV2( QColor color, double width, Qt::PenStyle penStyle )
    : mPenStyle( penStyle ), mPenJoinStyle( DEFAULT_SIMPLELINE_JOINSTYLE ), mPenCapStyle( DEFAULT_SIMPLELINE_CAPSTYLE ), mOffset( 0 )
{
  mColor = color;
  mWidth = width;
}


QgsSymbolLayerV2* QgsSimpleLineSymbolLayerV2::create( const QgsStringMap& props )
{
  QColor color = DEFAULT_SIMPLELINE_COLOR;
  double width = DEFAULT_SIMPLELINE_WIDTH;
  Qt::PenStyle penStyle = DEFAULT_SIMPLELINE_PENSTYLE;

  if ( props.contains( "color" ) )
    color = QgsSymbolLayerV2Utils::decodeColor( props["color"] );
  if ( props.contains( "width" ) )
    width = props["width"].toDouble();
  if ( props.contains( "penstyle" ) )
    penStyle = QgsSymbolLayerV2Utils::decodePenStyle( props["penstyle"] );


  QgsSimpleLineSymbolLayerV2* l = new QgsSimpleLineSymbolLayerV2( color, width, penStyle );
  if ( props.contains( "offset" ) )
    l->setOffset( props["offset"].toDouble() );
  if ( props.contains( "joinstyle" ) )
    l->setPenJoinStyle( QgsSymbolLayerV2Utils::decodePenJoinStyle( props["joinstyle"] ) );
  if ( props.contains( "capstyle" ) )
    l->setPenCapStyle( QgsSymbolLayerV2Utils::decodePenCapStyle( props["capstyle"] ) );
  return l;
}


QString QgsSimpleLineSymbolLayerV2::layerType() const
{
  return "SimpleLine";
}


void QgsSimpleLineSymbolLayerV2::startRender( QgsRenderContext& context )
{
  mPen.setColor( mColor );
  mPen.setWidth( mWidth );
  mPen.setStyle( mPenStyle );
  mPen.setJoinStyle( mPenJoinStyle );
  mPen.setCapStyle( mPenCapStyle );
}

void QgsSimpleLineSymbolLayerV2::stopRender( QgsRenderContext& context )
{
}

void QgsSimpleLineSymbolLayerV2::renderPolyline( const QPolygonF& points, QgsRenderContext& context )
{
  context.painter()->setPen( mPen );
  if ( mOffset == 0 )
  {
    context.painter()->drawPolyline( points );
  }
  else
  {
    context.painter()->drawPolyline( ::offsetLine( points, mOffset ) );
  }
}

QgsStringMap QgsSimpleLineSymbolLayerV2::properties() const
{
  QgsStringMap map;
  map["color"] = QgsSymbolLayerV2Utils::encodeColor( mColor );
  map["width"] = QString::number( mWidth );
  map["penstyle"] = QgsSymbolLayerV2Utils::encodePenStyle( mPenStyle );
  map["joinstyle"] = QgsSymbolLayerV2Utils::encodePenJoinStyle( mPenJoinStyle );
  map["capstyle"] = QgsSymbolLayerV2Utils::encodePenCapStyle( mPenCapStyle );
  map["offset"] = QString::number( mOffset );
  return map;
}

QgsSymbolLayerV2* QgsSimpleLineSymbolLayerV2::clone() const
{
  QgsSimpleLineSymbolLayerV2* l = new QgsSimpleLineSymbolLayerV2( mColor, mWidth, mPenStyle );
  l->setOffset( mOffset );
  l->setPenJoinStyle( mPenJoinStyle );
  l->setPenCapStyle( mPenCapStyle );
  return l;
}


/////////


class MyLine
{
  public:
    MyLine( QPointF p1, QPointF p2 ) : mVertical(false), mIncreasing(false), mT(0.0), mLength(0.0)
    {
      if ( p1 == p2 )
        return; // invalid

      // tangent and direction
      if ( p1.x() == p2.x() )
      {
        // vertical line - tangent undefined
        mVertical = true;
        mIncreasing = ( p2.y() > p1.y() );
      }
      else
      {
        mVertical = false;
        mT = float( p2.y() - p1.y() ) / ( p2.x() - p1.x() );
        mIncreasing = ( p2.x() > p1.x() );
      }

      // length
      double x = ( p2.x() - p1.x() );
      double y = ( p2.y() - p1.y() );
      mLength = sqrt( x * x + y * y );
    }

    // return angle in radians
    double angle()
    {
      double a = ( mVertical ? M_PI / 2 : atan( mT ) );

      if ( !mIncreasing )
        a += M_PI;
      return a;
    }

    // return difference for x,y when going along the line with specified interval
    QPointF diffForInterval( double interval )
    {
      if ( mVertical )
        return ( mIncreasing ? QPointF( 0, interval ) : QPointF( 0, -interval ) );

      double alpha = atan( mT );
      double dx = cos( alpha ) * interval;
      double dy = sin( alpha ) * interval;
      return ( mIncreasing ? QPointF( dx, dy ) : QPointF( -dx, -dy ) );
    }

    double length() { return mLength; }

  protected:
    bool mVertical;
    bool mIncreasing;
    double mT;
    double mLength;
};


QgsMarkerLineSymbolLayerV2::QgsMarkerLineSymbolLayerV2( bool rotateMarker, double interval )
{
  mRotateMarker = rotateMarker;
  mInterval = interval;
  mMarker = NULL;
  mOffset = 0;

  setSubSymbol( new QgsMarkerSymbolV2() );
}

QgsMarkerLineSymbolLayerV2::~QgsMarkerLineSymbolLayerV2()
{
  delete mMarker;
}

QgsSymbolLayerV2* QgsMarkerLineSymbolLayerV2::create( const QgsStringMap& props )
{
  bool rotate = DEFAULT_MARKERLINE_ROTATE;
  double interval = DEFAULT_MARKERLINE_INTERVAL;

  if ( props.contains( "interval" ) )
    interval = props["interval"].toDouble();
  if ( props.contains( "rotate" ) )
    rotate = ( props["rotate"] == "1" );

  QgsMarkerLineSymbolLayerV2* x = new QgsMarkerLineSymbolLayerV2( rotate, interval );
  if ( props.contains( "offset" ) )
    x->setOffset( props["offset"].toDouble() );
  return x;
}

QString QgsMarkerLineSymbolLayerV2::layerType() const
{
  return "MarkerLine";
}

void QgsMarkerLineSymbolLayerV2::setColor( QColor color )
{
  mMarker->setColor( color );
  mColor = color;
}

void QgsMarkerLineSymbolLayerV2::startRender( QgsRenderContext& context )
{
  // if being rotated, it gets initialized with every line segment
  if ( !mRotateMarker )
    mMarker->startRender( context );
}

void QgsMarkerLineSymbolLayerV2::stopRender( QgsRenderContext& context )
{
  if ( !mRotateMarker )
    mMarker->stopRender( context );
}

void QgsMarkerLineSymbolLayerV2::renderPolyline( const QPolygonF& points, QgsRenderContext& context )
{
  if ( mOffset == 0 )
  {
    renderPolylineNoOffset( points, context );
  }
  else
  {
    QPolygonF points2 = ::offsetLine( points, mOffset );
    renderPolylineNoOffset( points2, context );
  }
}

void QgsMarkerLineSymbolLayerV2::renderPolylineNoOffset( const QPolygonF& points, QgsRenderContext& context )
{
  QPointF lastPt = points[0];
  double lengthLeft = 0; // how much is left until next marker
  bool first = true;

  for ( int i = 1; i < points.count(); ++i )
  {
    const QPointF& pt = points[i];

    if ( lastPt == pt ) // must not be equal!
      continue;

    // for each line, find out dx and dy, and length
    MyLine l( lastPt, pt );
    QPointF diff = l.diffForInterval( mInterval );

    // if there's some length left from previous line
    // use only the rest for the first point in new line segment
    double c = 1 - lengthLeft / mInterval;

    lengthLeft += l.length();

    // rotate marker (if desired)
    if ( mRotateMarker )
    {
      mMarker->setAngle( l.angle() * 180 / M_PI );
      mMarker->startRender( context );
    }

    // draw first marker
    if ( first )
    {
      mMarker->renderPoint( lastPt, context );
      first = false;
    }

    // while we're not at the end of line segment, draw!
    while ( lengthLeft > mInterval )
    {
      // "c" is 1 for regular point or in interval (0,1] for begin of line segment
      lastPt += c * diff;
      lengthLeft -= mInterval;
      mMarker->renderPoint( lastPt, context );
      c = 1; // reset c (if wasn't 1 already)
    }

    lastPt = pt;

    if ( mRotateMarker )
      mMarker->stopRender( context );
  }

}

QgsStringMap QgsMarkerLineSymbolLayerV2::properties() const
{
  QgsStringMap map;
  map["rotate"] = ( mRotateMarker ? "1" : "0" );
  map["interval"] = QString::number( mInterval );
  map["offset"] = QString::number( mOffset );
  return map;
}

QgsSymbolV2* QgsMarkerLineSymbolLayerV2::subSymbol()
{
  return mMarker;
}

bool QgsMarkerLineSymbolLayerV2::setSubSymbol( QgsSymbolV2* symbol )
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

QgsSymbolLayerV2* QgsMarkerLineSymbolLayerV2::clone() const
{
  QgsMarkerLineSymbolLayerV2* x = new QgsMarkerLineSymbolLayerV2( mRotateMarker, mInterval );
  x->setSubSymbol( mMarker->clone() );
  x->setOffset( mOffset );
  return x;
}

/////////////

QgsLineDecorationSymbolLayerV2::QgsLineDecorationSymbolLayerV2( QColor color )
{
  mColor = color;
}

QgsLineDecorationSymbolLayerV2::~QgsLineDecorationSymbolLayerV2()
{
}

QgsSymbolLayerV2* QgsLineDecorationSymbolLayerV2::create( const QgsStringMap& props )
{
  QColor color = DEFAULT_LINEDECORATION_COLOR;

  if ( props.contains( "color" ) )
    color = QgsSymbolLayerV2Utils::decodeColor( props["color"] );

  return new QgsLineDecorationSymbolLayerV2( color );
}

QString QgsLineDecorationSymbolLayerV2::layerType() const
{
  return "LineDecoration";
}

void QgsLineDecorationSymbolLayerV2::startRender( QgsRenderContext& context )
{
  mPen.setColor( mColor );
}

void QgsLineDecorationSymbolLayerV2::stopRender( QgsRenderContext& context )
{
}

static double _calculateAngle( double x1, double y1, double x2, double y2 )
{
  // return angle (in radians) between two points
  if ( x1 == x2 )
    return  M_PI * ( y2 >= y1 ? 1 / 2 : 3 / 2 ); // angle is 90 or 270

  double t = ( y2 - y1 ) / ( x2 - x1 );
  if ( t >= 0 )
    return atan( t ) + ( y2 >= y1 ? 0 : M_PI );
  else // t < 0
    return atan( t ) + ( y2 >= y1 ? M_PI : 0 ); // atan is positive / negative
}

void QgsLineDecorationSymbolLayerV2::renderPolyline( const QPolygonF& points, QgsRenderContext& context )
{
  // draw arrow at the end of line

  int cnt = points.count();
  QPointF p1 = points.at( cnt - 2 );
  QPointF p2 = points.at( cnt - 1 );
  double angle = _calculateAngle( p1.x(), p1.y(), p2.x(), p2.y() );

  double size = 6;
  double angle1 = angle + M_PI / 6;
  double angle2 = angle - M_PI / 6;

  QPointF p2_1 = p2 - QPointF( size * cos( angle1 ), size * sin( angle1 ) );
  QPointF p2_2 = p2 - QPointF( size * cos( angle2 ), size * sin( angle2 ) );

  context.painter()->setPen( mPen );
  context.painter()->drawLine( p2, p2_1 );
  context.painter()->drawLine( p2, p2_2 );
}

QgsStringMap QgsLineDecorationSymbolLayerV2::properties() const
{
  QgsStringMap map;
  map["color"] = QgsSymbolLayerV2Utils::encodeColor( mColor );
  return map;
}

QgsSymbolLayerV2* QgsLineDecorationSymbolLayerV2::clone() const
{
  return new QgsLineDecorationSymbolLayerV2( mColor );
}

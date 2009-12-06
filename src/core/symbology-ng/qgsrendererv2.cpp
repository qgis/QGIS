
#include "qgsrendererv2.h"
#include "qgssymbolv2.h"
#include "qgssymbollayerv2utils.h"

#include "qgssinglesymbolrendererv2.h" // for default renderer

#include "qgsrendererv2registry.h"

#include "qgsrendercontext.h"
#include "qgsgeometry.h"
#include "qgsfeature.h"
#include "qgslogger.h"

#include <QDomElement>
#include <QDomDocument>
#include <QPolygonF>



static unsigned char* _getPoint( QPointF& pt, QgsRenderContext& context, unsigned char* wkb )
{
  wkb++; // jump over endian info
  unsigned int wkbType = *(( int* ) wkb );
  wkb += sizeof( unsigned int );

  double x = *(( double * ) wkb ); wkb += sizeof( double );
  double y = *(( double * ) wkb ); wkb += sizeof( double );

  if ( wkbType == QGis::WKBPolygon25D )
    wkb += sizeof( double );

  if ( context.coordinateTransform() )
  {
    double z = 0; // dummy variable for coordiante transform
    context.coordinateTransform()->transformInPlace( x, y, z );
  }

  context.mapToPixel().transformInPlace( x, y );

  pt = QPointF( x, y );
  return wkb;
}

static unsigned char* _getLineString( QPolygonF& pts, QgsRenderContext& context, unsigned char* wkb )
{
  wkb++; // jump over endian info
  unsigned int wkbType = *(( int* ) wkb );
  wkb += sizeof( unsigned int );
  unsigned int nPoints = *(( int* ) wkb );
  wkb += sizeof( unsigned int );

  bool hasZValue = ( wkbType == QGis::WKBLineString25D );
  double x, y;

  pts.resize( nPoints );

  const QgsCoordinateTransform* ct = context.coordinateTransform();
  const QgsMapToPixel& mtp = context.mapToPixel();
  double z = 0; // dummy variable for coordiante transform

  for ( unsigned int i = 0; i < nPoints; ++i )
  {
    x = *(( double * ) wkb );
    wkb += sizeof( double );
    y = *(( double * ) wkb );
    wkb += sizeof( double );

    if ( hasZValue ) // ignore Z value
      wkb += sizeof( double );

    // TODO: maybe to the transform at once (faster?)
    if ( ct )
      ct->transformInPlace( x, y, z );
    mtp.transformInPlace( x, y );

    pts[i] = QPointF( x, y );

  }

  return wkb;
}

static unsigned char* _getPolygon( QPolygonF& pts, QList<QPolygonF>& holes, QgsRenderContext& context, unsigned char* wkb )
{
  wkb++; // jump over endian info
  unsigned int wkbType = *(( int* ) wkb );
  wkb += sizeof( unsigned int ); // jump over wkb type
  unsigned int numRings = *(( int* ) wkb );
  wkb += sizeof( unsigned int );

  if ( numRings == 0 )  // sanity check for zero rings in polygon
    return wkb;

  bool hasZValue = ( wkbType == QGis::WKBPolygon25D );
  double x, y;
  holes.clear();

  const QgsCoordinateTransform* ct = context.coordinateTransform();
  const QgsMapToPixel& mtp = context.mapToPixel();
  double z = 0; // dummy variable for coordiante transform

  for ( unsigned int idx = 0; idx < numRings; idx++ )
  {
    unsigned int nPoints = *(( int* )wkb );
    wkb += sizeof( unsigned int );

    QPolygonF poly( nPoints );

    // Extract the points from the WKB and store in a pair of vectors.
    for ( unsigned int jdx = 0; jdx < nPoints; jdx++ )
    {
      x = *(( double * ) wkb ); wkb += sizeof( double );
      y = *(( double * ) wkb ); wkb += sizeof( double );

      // TODO: maybe to the transform at once (faster?)
      if ( ct )
        ct->transformInPlace( x, y, z );
      mtp.transformInPlace( x, y );

      poly[jdx] = QPointF( x, y );

      if ( hasZValue )
        wkb += sizeof( double );
    }

    if ( nPoints < 1 )
      continue;

    if ( idx == 0 )
      pts = poly;
    else
      holes.append( poly );
  }

  return wkb;
}


QgsFeatureRendererV2::QgsFeatureRendererV2( QString type )
    : mType( type ), mUsingSymbolLevels( false )
{
}

QgsFeatureRendererV2* QgsFeatureRendererV2::defaultRenderer( QGis::GeometryType geomType )
{
  return new QgsSingleSymbolRendererV2( QgsSymbolV2::defaultSymbol( geomType ) );
}


void QgsFeatureRendererV2::renderFeature( QgsFeature& feature, QgsRenderContext& context, int layer )
{
  QgsSymbolV2* symbol = symbolForFeature( feature );
  if ( symbol == NULL )
    return;

  QgsSymbolV2::SymbolType symbolType = symbol->type();

  QgsGeometry* geom = feature.geometry();
  switch ( geom->wkbType() )
  {
    case QGis::WKBPoint:
    {
      if ( symbolType != QgsSymbolV2::Marker )
      {
        QgsDebugMsg( "point can be drawn only with marker symbol!" );
        break;
      }
      QPointF pt;
      _getPoint( pt, context, geom->asWkb() );
      (( QgsMarkerSymbolV2* )symbol )->renderPoint( pt, context, layer );
    }
    break;

    case QGis::WKBLineString:
    {
      if ( symbolType != QgsSymbolV2::Line )
      {
        QgsDebugMsg( "linestring can be drawn only with line symbol!" );
        break;
      }
      QPolygonF pts;
      _getLineString( pts, context, geom->asWkb() );
      (( QgsLineSymbolV2* )symbol )->renderPolyline( pts, context, layer );
    }
    break;

    case QGis::WKBPolygon:
    {
      if ( symbolType != QgsSymbolV2::Fill )
      {
        QgsDebugMsg( "polygon can be drawn only with fill symbol!" );
        break;
      }
      QPolygonF pts;
      QList<QPolygonF> holes;
      _getPolygon( pts, holes, context, geom->asWkb() );
      (( QgsFillSymbolV2* )symbol )->renderPolygon( pts, ( holes.count() ? &holes : NULL ), context, layer );
    }
    break;

    case QGis::WKBMultiPoint:
    {
      if ( symbolType != QgsSymbolV2::Marker )
      {
        QgsDebugMsg( "multi-point can be drawn only with marker symbol!" );
        break;
      }

      unsigned char* wkb = geom->asWkb();
      unsigned int num = *(( int* )( wkb + 5 ) );
      unsigned char* ptr = wkb + 9;
      QPointF pt;

      for ( unsigned int i = 0; i < num; ++i )
      {
        ptr = _getPoint( pt, context, ptr );
        (( QgsMarkerSymbolV2* )symbol )->renderPoint( pt, context, layer );
      }
    }
    break;

    case QGis::WKBMultiLineString:
    {
      if ( symbolType != QgsSymbolV2::Line )
      {
        QgsDebugMsg( "multi-linestring can be drawn only with line symbol!" );
        break;
      }

      unsigned char* wkb = geom->asWkb();
      unsigned int num = *(( int* )( wkb + 5 ) );
      unsigned char* ptr = wkb + 9;
      QPolygonF pts;

      for ( unsigned int i = 0; i < num; ++i )
      {
        ptr = _getLineString( pts, context, ptr );
        (( QgsLineSymbolV2* )symbol )->renderPolyline( pts, context, layer );
      }
    }
    break;

    case QGis::WKBMultiPolygon:
    {
      if ( symbolType != QgsSymbolV2::Fill )
      {
        QgsDebugMsg( "multi-polygon can be drawn only with fill symbol!" );
        break;
      }

      unsigned char* wkb = geom->asWkb();
      unsigned int num = *(( int* )( wkb + 5 ) );
      unsigned char* ptr = wkb + 9;
      QPolygonF pts;
      QList<QPolygonF> holes;

      for ( unsigned int i = 0; i < num; ++i )
      {
        ptr = _getPolygon( pts, holes, context, ptr );
        (( QgsFillSymbolV2* )symbol )->renderPolygon( pts, ( holes.count() ? &holes : NULL ), context, layer );
      }
    }
    break;

    default:
      QgsDebugMsg( "unsupported wkb type for rendering" );
  }
}

QString QgsFeatureRendererV2::dump()
{
  return "UNKNOWN RENDERER\n";
}


QgsFeatureRendererV2* QgsFeatureRendererV2::load( QDomElement& element )
{
  // <renderer-v2 type=""> ... </renderer-v2>

  if ( element.isNull() )
    return NULL;

  // load renderer
  QString rendererType = element.attribute( "type" );

  QgsRendererV2CreateFunc pfCreate = QgsRendererV2Registry::instance()->rendererMetadata( rendererType ).createFunction();

  // unknown renderer type?
  if ( pfCreate == NULL )
    return NULL;

  return pfCreate( element );
}

QDomElement QgsFeatureRendererV2::save( QDomDocument& doc )
{
  // create empty renderer element
  return doc.createElement( RENDERER_TAG_NAME );
}

QgsLegendSymbologyList QgsFeatureRendererV2::legendSymbologyItems( QSize iconSize )
{
  // empty list by default
  return QgsLegendSymbologyList();
}

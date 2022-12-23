/***************************************************************************
 qgssymbol.cpp
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

#include <QColor>
#include <QImage>
#include <QPainter>
#include <QSize>
#include <QSvgGenerator>

#include <cmath>
#include <map>
#include <random>

#include "qgssymbol.h"
#include "qgssymbollayer.h"

#include "qgsgeometrygeneratorsymbollayer.h"
#include "qgsmaptopixelgeometrysimplifier.h"
#include "qgslogger.h"
#include "qgsrendercontext.h" // for bigSymbolPreview
#include "qgsproject.h"
#include "qgsprojectstylesettings.h"
#include "qgsstyle.h"
#include "qgspainteffect.h"
#include "qgsvectorlayer.h"
#include "qgsfeature.h"
#include "qgsgeometry.h"
#include "qgsmultipoint.h"
#include "qgsgeometrycollection.h"
#include "qgslinestring.h"
#include "qgspolygon.h"
#include "qgsclipper.h"
#include "qgsproperty.h"
#include "qgscolorschemeregistry.h"
#include "qgsapplication.h"
#include "qgsexpressioncontextutils.h"
#include "qgsrenderedfeaturehandlerinterface.h"
#include "qgslegendpatchshape.h"
#include "qgsgeos.h"
#include "qgsmarkersymbol.h"
#include "qgslinesymbol.h"
#include "qgsfillsymbol.h"

QgsPropertiesDefinition QgsSymbol::sPropertyDefinitions;

Q_NOWARN_DEPRECATED_PUSH // because of deprecated mLayer
QgsSymbol::QgsSymbol( Qgis::SymbolType type, const QgsSymbolLayerList &layers )
  : mType( type )
  , mLayers( layers )
{

  // check they're all correct symbol layers
  for ( int i = 0; i < mLayers.count(); i++ )
  {
    if ( !mLayers.at( i ) )
    {
      mLayers.removeAt( i-- );
    }
    else if ( !mLayers.at( i )->isCompatibleWithSymbol( this ) )
    {
      delete mLayers.at( i );
      mLayers.removeAt( i-- );
    }
  }
}
Q_NOWARN_DEPRECATED_POP

QPolygonF QgsSymbol::_getLineString( QgsRenderContext &context, const QgsCurve &curve, bool clipToExtent )
{
  if ( curve.is3D() )
    return _getLineString3d( context, curve, clipToExtent );
  else
    return _getLineString2d( context, curve, clipToExtent );
}

QPolygonF QgsSymbol::_getLineString3d( QgsRenderContext &context, const QgsCurve &curve, bool clipToExtent )
{
  const unsigned int nPoints = curve.numPoints();

  QgsCoordinateTransform ct = context.coordinateTransform();
  const QgsMapToPixel &mtp = context.mapToPixel();
  QVector< double > pointsX;
  QVector< double > pointsY;
  QVector< double > pointsZ;

  // apply clipping for large lines to achieve a better rendering performance
  if ( clipToExtent && nPoints > 1 && !( context.flags() & Qgis::RenderContextFlag::ApplyClipAfterReprojection ) )
  {
    const QgsRectangle e = context.extent();
    const double cw = e.width() / 10;
    const double ch = e.height() / 10;
    const QgsBox3d clipRect( e.xMinimum() - cw, e.yMinimum() - ch, -HUGE_VAL, e.xMaximum() + cw, e.yMaximum() + ch, HUGE_VAL ); // TODO also need to be clipped according to z axis

    const QgsLineString *lineString = nullptr;
    if ( const QgsLineString *ls = qgsgeometry_cast< const QgsLineString * >( &curve ) )
    {
      lineString = ls;
    }
    else
    {
      std::unique_ptr< QgsLineString > segmentized;
      segmentized.reset( qgsgeometry_cast< QgsLineString * >( curve.segmentize( ) ) );
      lineString = segmentized.get();
    }

    QgsClipper::clipped3dLine( lineString->xVector(), lineString->yVector(), lineString->zVector(), pointsX, pointsY, pointsZ, clipRect );
  }
  else
  {
    // clone...
    if ( const QgsLineString *ls = qgsgeometry_cast<const QgsLineString *>( &curve ) )
    {
      pointsX = ls->xVector();
      pointsY = ls->yVector();
      pointsZ = ls->zVector();
    }
    else
    {
      std::unique_ptr< QgsLineString > segmentized;
      segmentized.reset( qgsgeometry_cast< QgsLineString * >( curve.segmentize( ) ) );

      pointsX = segmentized->xVector();
      pointsY = segmentized->yVector();
      pointsZ = segmentized->zVector();
    }
  }

  // transform the points to screen coordinates
  const QVector< double > preTransformPointsZ = pointsZ;
  bool wasTransformed = false;
  if ( ct.isValid() )
  {
    //create x, y arrays
    const int nVertices = pointsX.size();
    wasTransformed = true;

    try
    {
      ct.transformCoords( nVertices, pointsX.data(), pointsY.data(), pointsZ.data(), Qgis::TransformDirection::Forward );
    }
    catch ( QgsCsException & )
    {
      // we don't abort the rendering here, instead we remove any invalid points and just plot those which ARE valid
    }
  }

  // remove non-finite points, e.g. infinite or NaN points caused by reprojecting errors
  {
    const int size = pointsX.size();

    const double *xIn = pointsX.data();
    const double *yIn = pointsY.data();
    const double *zIn = pointsZ.data();

    const double *preTransformZIn = wasTransformed ? preTransformPointsZ.constData() : nullptr;

    double *xOut = pointsX.data();
    double *yOut = pointsY.data();
    double *zOut = pointsZ.data();
    int outSize = 0;
    for ( int i = 0; i < size; ++i )
    {
      bool pointOk = std::isfinite( *xIn ) && std::isfinite( *yIn );

      // skip z points which have been made non-finite during transformations only. Ie if:
      // - we did no transformation, then always render even if non-finite z
      // - we did transformation and z is finite then render
      // - we did transformation and z is non-finite BUT input z was also non finite then render
      // - we did transformation and z is non-finite AND input z WAS finite then skip
      pointOk &= !wasTransformed || std::isfinite( *zIn ) || !std::isfinite( *preTransformZIn );

      if ( pointOk )
      {
        *xOut++ = *xIn++;
        *yOut++ = *yIn++;
        *zOut++ = *zIn++;
        outSize++;
      }
      else
      {
        xIn++;
        yIn++;
        zIn++;
      }

      if ( preTransformZIn )
        preTransformZIn++;
    }
    pointsX.resize( outSize );
    pointsY.resize( outSize );
    pointsZ.resize( outSize );
  }

  if ( clipToExtent && nPoints > 1 && context.flags() & Qgis::RenderContextFlag::ApplyClipAfterReprojection )
  {
    // early clipping was not possible, so we have to apply it here after transformation
    const QgsRectangle e = context.mapExtent();
    const double cw = e.width() / 10;
    const double ch = e.height() / 10;
    const QgsBox3d clipRect( e.xMinimum() - cw, e.yMinimum() - ch, -HUGE_VAL, e.xMaximum() + cw, e.yMaximum() + ch, HUGE_VAL ); // TODO also need to be clipped according to z axis

    QVector< double > tempX;
    QVector< double > tempY;
    QVector< double > tempZ;
    QgsClipper::clipped3dLine( pointsX, pointsY, pointsZ, tempX, tempY, tempZ, clipRect );
    pointsX = tempX;
    pointsY = tempY;
    pointsZ = tempZ;
  }

  const int polygonSize = pointsX.size();
  QPolygonF out( polygonSize );
  const double *x = pointsX.constData();
  const double *y = pointsY.constData();
  QPointF *dest = out.data();
  for ( int i = 0; i < polygonSize; ++i )
  {
    double screenX = *x++;
    double screenY = *y++;
    mtp.transformInPlace( screenX, screenY );
    *dest++ = QPointF( screenX, screenY );
  }

  return out;
}

QPolygonF QgsSymbol::_getLineString2d( QgsRenderContext &context, const QgsCurve &curve, bool clipToExtent )
{
  const unsigned int nPoints = curve.numPoints();

  QgsCoordinateTransform ct = context.coordinateTransform();
  const QgsMapToPixel &mtp = context.mapToPixel();
  QPolygonF pts;

  // apply clipping for large lines to achieve a better rendering performance
  if ( clipToExtent && nPoints > 1 && !( context.flags() & Qgis::RenderContextFlag::ApplyClipAfterReprojection ) )
  {
    const QgsRectangle e = context.extent();
    const double cw = e.width() / 10;
    const double ch = e.height() / 10;
    const QgsRectangle clipRect( e.xMinimum() - cw, e.yMinimum() - ch, e.xMaximum() + cw, e.yMaximum() + ch );
    pts = QgsClipper::clippedLine( curve, clipRect );
  }
  else
  {
    pts = curve.asQPolygonF();
  }

  // transform the QPolygonF to screen coordinates
  if ( ct.isValid() )
  {
    try
    {
      ct.transformPolygon( pts );
    }
    catch ( QgsCsException & )
    {
      // we don't abort the rendering here, instead we remove any invalid points and just plot those which ARE valid
    }
  }

  // remove non-finite points, e.g. infinite or NaN points caused by reprojecting errors
  pts.erase( std::remove_if( pts.begin(), pts.end(),
                             []( const QPointF point )
  {
    return !std::isfinite( point.x() ) || !std::isfinite( point.y() );
  } ), pts.end() );

  if ( clipToExtent && nPoints > 1 && context.flags() & Qgis::RenderContextFlag::ApplyClipAfterReprojection )
  {
    // early clipping was not possible, so we have to apply it here after transformation
    const QgsRectangle e = context.mapExtent();
    const double cw = e.width() / 10;
    const double ch = e.height() / 10;
    const QgsRectangle clipRect( e.xMinimum() - cw, e.yMinimum() - ch, e.xMaximum() + cw, e.yMaximum() + ch );
    pts = QgsClipper::clippedLine( pts, clipRect );
  }

  QPointF *ptr = pts.data();
  for ( int i = 0; i < pts.size(); ++i, ++ptr )
  {
    mtp.transformInPlace( ptr->rx(), ptr->ry() );
  }

  return pts;
}


QPolygonF QgsSymbol::_getPolygonRing( QgsRenderContext &context, const QgsCurve &curve, const bool clipToExtent, const bool isExteriorRing, const bool correctRingOrientation )
{
  if ( curve.is3D() )
    return _getPolygonRing3d( context, curve, clipToExtent, isExteriorRing, correctRingOrientation );
  else
    return _getPolygonRing2d( context, curve, clipToExtent, isExteriorRing, correctRingOrientation );
}

QPolygonF QgsSymbol::_getPolygonRing3d( QgsRenderContext &context, const QgsCurve &curve, const bool clipToExtent, const bool isExteriorRing, const bool correctRingOrientation )
{
  const QgsCoordinateTransform ct = context.coordinateTransform();
  const QgsMapToPixel &mtp = context.mapToPixel();

  QVector< double > pointsX;
  QVector< double > pointsY;
  QVector< double > pointsZ;

  if ( curve.numPoints() < 1 )
    return QPolygonF();

  bool reverseRing = false;
  if ( correctRingOrientation )
  {
    // ensure consistent polygon ring orientation
    if ( ( isExteriorRing && curve.orientation() != Qgis::AngularDirection::Clockwise ) || ( !isExteriorRing && curve.orientation() != Qgis::AngularDirection::CounterClockwise ) )
    {
      reverseRing = true;
    }
  }

  //clip close to view extent, if needed
  if ( clipToExtent && !( context.flags() & Qgis::RenderContextFlag::ApplyClipAfterReprojection ) && !context.extent().contains( curve.boundingBox() ) )
  {
    const QgsRectangle e = context.extent();
    const double cw = e.width() / 10;
    const double ch = e.height() / 10;
    const QgsBox3d clipRect( e.xMinimum() - cw, e.yMinimum() - ch, -HUGE_VAL, e.xMaximum() + cw, e.yMaximum() + ch, HUGE_VAL ); // TODO also need to be clipped according to z axis

    const QgsLineString *lineString = nullptr;
    std::unique_ptr< QgsLineString > segmentized;
    if ( const QgsLineString *ls = qgsgeometry_cast< const QgsLineString * >( &curve ) )
    {
      lineString = ls;
    }
    else
    {
      segmentized.reset( qgsgeometry_cast< QgsLineString * >( curve.segmentize( ) ) );
      lineString = segmentized.get();
    }

    pointsX = lineString->xVector();
    pointsY = lineString->yVector();
    pointsZ = lineString->zVector();

    QgsClipper::trimPolygon( pointsX, pointsY, pointsZ, clipRect );
  }
  else
  {
    // clone...
    if ( const QgsLineString *ls = qgsgeometry_cast<const QgsLineString *>( &curve ) )
    {
      pointsX = ls->xVector();
      pointsY = ls->yVector();
      pointsZ = ls->zVector();
    }
    else
    {
      std::unique_ptr< QgsLineString > segmentized;
      segmentized.reset( qgsgeometry_cast< QgsLineString * >( curve.segmentize( ) ) );

      pointsX = segmentized->xVector();
      pointsY = segmentized->yVector();
      pointsZ = segmentized->zVector();
    }
  }

  if ( reverseRing )
  {
    std::reverse( pointsX.begin(), pointsX.end() );
    std::reverse( pointsY.begin(), pointsY.end() );
    std::reverse( pointsZ.begin(), pointsZ.end() );
  }

  //transform the QPolygonF to screen coordinates
  const QVector< double > preTransformPointsZ = pointsZ;
  bool wasTransformed = false;
  if ( ct.isValid() )
  {
    const int nVertices = pointsX.size();
    wasTransformed = true;
    try
    {
      ct.transformCoords( nVertices, pointsX.data(), pointsY.data(), pointsZ.data(), Qgis::TransformDirection::Forward );
    }
    catch ( QgsCsException & )
    {
      // we don't abort the rendering here, instead we remove any invalid points and just plot those which ARE valid
    }
  }

  // remove non-finite points, e.g. infinite or NaN points caused by reprojecting errors
  {
    const int size = pointsX.size();

    const double *xIn = pointsX.data();
    const double *yIn = pointsY.data();
    const double *zIn = pointsZ.data();

    const double *preTransformZIn = wasTransformed ? preTransformPointsZ.constData() : nullptr;

    double *xOut = pointsX.data();
    double *yOut = pointsY.data();
    double *zOut = pointsZ.data();
    int outSize = 0;
    for ( int i = 0; i < size; ++i )
    {
      bool pointOk = std::isfinite( *xIn ) && std::isfinite( *yIn );
      // skip z points which have been made non-finite during transformations only. Ie if:
      // - we did no transformation, then always render even if non-finite z
      // - we did transformation and z is finite then render
      // - we did transformation and z is non-finite BUT input z was also non finite then render
      // - we did transformation and z is non-finite AND input z WAS finite then skip
      pointOk &= !wasTransformed || std::isfinite( *zIn ) || !std::isfinite( *preTransformZIn );

      if ( pointOk )
      {
        *xOut++ = *xIn++;
        *yOut++ = *yIn++;
        *zOut++ = *zIn++;
        outSize++;
      }
      else
      {
        xIn++;
        yIn++;
        zIn++;
      }

      if ( preTransformZIn )
        preTransformZIn++;
    }
    pointsX.resize( outSize );
    pointsY.resize( outSize );
    pointsZ.resize( outSize );
  }

  if ( clipToExtent && context.flags() & Qgis::RenderContextFlag::ApplyClipAfterReprojection && !context.mapExtent().contains( curve.boundingBox() ) )
  {
    // early clipping was not possible, so we have to apply it here after transformation
    const QgsRectangle e = context.mapExtent();
    const double cw = e.width() / 10;
    const double ch = e.height() / 10;
    const QgsBox3d clipRect( e.xMinimum() - cw, e.yMinimum() - ch, -HUGE_VAL, e.xMaximum() + cw, e.yMaximum() + ch, HUGE_VAL ); // TODO also need to be clipped according to z axis

    QgsClipper::trimPolygon( pointsX, pointsY, pointsZ, clipRect );
  }

  const int polygonSize = pointsX.size();
  QPolygonF out( polygonSize );
  const double *x = pointsX.constData();
  const double *y = pointsY.constData();
  QPointF *dest = out.data();
  for ( int i = 0; i < polygonSize; ++i )
  {
    double screenX = *x++;
    double screenY = *y++;
    mtp.transformInPlace( screenX, screenY );
    *dest++ = QPointF( screenX, screenY );
  }

  if ( !out.empty() && !out.isClosed() )
    out << out.at( 0 );

  return out;
}


QPolygonF QgsSymbol::_getPolygonRing2d( QgsRenderContext &context, const QgsCurve &curve, const bool clipToExtent, const bool isExteriorRing, const bool correctRingOrientation )
{
  const QgsCoordinateTransform ct = context.coordinateTransform();
  const QgsMapToPixel &mtp = context.mapToPixel();

  QPolygonF poly = curve.asQPolygonF();

  if ( curve.numPoints() < 1 )
    return QPolygonF();

  if ( correctRingOrientation )
  {
    // ensure consistent polygon ring orientation
    if ( isExteriorRing && curve.orientation() != Qgis::AngularDirection::Clockwise )
      std::reverse( poly.begin(), poly.end() );
    else if ( !isExteriorRing && curve.orientation() != Qgis::AngularDirection::CounterClockwise )
      std::reverse( poly.begin(), poly.end() );
  }

  //clip close to view extent, if needed
  if ( clipToExtent && !( context.flags() & Qgis::RenderContextFlag::ApplyClipAfterReprojection ) && !context.extent().contains( poly.boundingRect() ) )
  {
    const QgsRectangle e = context.extent();
    const double cw = e.width() / 10;
    const double ch = e.height() / 10;
    const QgsRectangle clipRect( e.xMinimum() - cw, e.yMinimum() - ch, e.xMaximum() + cw, e.yMaximum() + ch );
    QgsClipper::trimPolygon( poly, clipRect );
  }

  //transform the QPolygonF to screen coordinates
  if ( ct.isValid() )
  {
    try
    {
      ct.transformPolygon( poly );
    }
    catch ( QgsCsException & )
    {
      // we don't abort the rendering here, instead we remove any invalid points and just plot those which ARE valid
    }
  }

  // remove non-finite points, e.g. infinite or NaN points caused by reprojecting errors
  poly.erase( std::remove_if( poly.begin(), poly.end(),
                              []( const QPointF point )
  {
    return !std::isfinite( point.x() ) || !std::isfinite( point.y() );
  } ), poly.end() );

  if ( clipToExtent && context.flags() & Qgis::RenderContextFlag::ApplyClipAfterReprojection && !context.mapExtent().contains( poly.boundingRect() ) )
  {
    // early clipping was not possible, so we have to apply it here after transformation
    const QgsRectangle e = context.mapExtent();
    const double cw = e.width() / 10;
    const double ch = e.height() / 10;
    const QgsRectangle clipRect( e.xMinimum() - cw, e.yMinimum() - ch, e.xMaximum() + cw, e.yMaximum() + ch );
    QgsClipper::trimPolygon( poly, clipRect );
  }

  QPointF *ptr = poly.data();
  for ( int i = 0; i < poly.size(); ++i, ++ptr )
  {
    mtp.transformInPlace( ptr->rx(), ptr->ry() );
  }

  if ( !poly.empty() && !poly.isClosed() )
    poly << poly.at( 0 );

  return poly;
}

void QgsSymbol::_getPolygon( QPolygonF &pts, QVector<QPolygonF> &holes, QgsRenderContext &context, const QgsPolygon &polygon, const bool clipToExtent, const bool correctRingOrientation )
{
  holes.clear();

  pts = _getPolygonRing( context, *polygon.exteriorRing(), clipToExtent, true, correctRingOrientation );
  const int ringCount = polygon.numInteriorRings();
  holes.reserve( ringCount );
  for ( int idx = 0; idx < ringCount; idx++ )
  {
    const QPolygonF hole = _getPolygonRing( context, *( polygon.interiorRing( idx ) ), clipToExtent, false, correctRingOrientation );
    if ( !hole.isEmpty() )
      holes.append( hole );
  }
}

QString QgsSymbol::symbolTypeToString( Qgis::SymbolType type )
{
  switch ( type )
  {
    case Qgis::SymbolType::Marker:
      return QObject::tr( "Marker" );
    case Qgis::SymbolType::Line:
      return QObject::tr( "Line" );
    case Qgis::SymbolType::Fill:
      return QObject::tr( "Fill" );
    case Qgis::SymbolType::Hybrid:
      return QObject::tr( "Hybrid" );
  }
  return QString();
}

Qgis::SymbolType QgsSymbol::symbolTypeForGeometryType( QgsWkbTypes::GeometryType type )
{
  switch ( type )
  {
    case QgsWkbTypes::PointGeometry:
      return Qgis::SymbolType::Marker;
    case QgsWkbTypes::LineGeometry:
      return Qgis::SymbolType::Line;
    case QgsWkbTypes::PolygonGeometry:
      return Qgis::SymbolType::Fill;
    case QgsWkbTypes::UnknownGeometry:
    case QgsWkbTypes::NullGeometry:
      return Qgis::SymbolType::Hybrid;
  }
  return Qgis::SymbolType::Hybrid;
}

const QgsPropertiesDefinition &QgsSymbol::propertyDefinitions()
{
  QgsSymbol::initPropertyDefinitions();
  return sPropertyDefinitions;
}

QgsSymbol::~QgsSymbol()
{
  // delete all symbol layers (we own them, so it's okay)
  qDeleteAll( mLayers );
}

QgsUnitTypes::RenderUnit QgsSymbol::outputUnit() const
{
  if ( mLayers.empty() )
  {
    return QgsUnitTypes::RenderUnknownUnit;
  }

  QgsSymbolLayerList::const_iterator it = mLayers.constBegin();

  QgsUnitTypes::RenderUnit unit = ( *it )->outputUnit();

  for ( ; it != mLayers.constEnd(); ++it )
  {
    if ( ( *it )->outputUnit() != unit )
    {
      return QgsUnitTypes::RenderUnknownUnit;
    }
  }
  return unit;
}

bool QgsSymbol::usesMapUnits() const
{
  if ( mLayers.empty() )
  {
    return false;
  }

  for ( const QgsSymbolLayer *layer : mLayers )
  {
    if ( layer->usesMapUnits() )
    {
      return true;
    }
  }
  return false;
}

QgsMapUnitScale QgsSymbol::mapUnitScale() const
{
  if ( mLayers.empty() )
  {
    return QgsMapUnitScale();
  }

  QgsSymbolLayerList::const_iterator it = mLayers.constBegin();
  if ( it == mLayers.constEnd() )
    return QgsMapUnitScale();

  QgsMapUnitScale scale = ( *it )->mapUnitScale();
  ++it;

  for ( ; it != mLayers.constEnd(); ++it )
  {
    if ( ( *it )->mapUnitScale() != scale )
    {
      return QgsMapUnitScale();
    }
  }
  return scale;
}

void QgsSymbol::setOutputUnit( QgsUnitTypes::RenderUnit u ) const
{
  const auto constMLayers = mLayers;
  for ( QgsSymbolLayer *layer : constMLayers )
  {
    layer->setOutputUnit( u );
  }
}

void QgsSymbol::setMapUnitScale( const QgsMapUnitScale &scale ) const
{
  const auto constMLayers = mLayers;
  for ( QgsSymbolLayer *layer : constMLayers )
  {
    layer->setMapUnitScale( scale );
  }
}

QgsSymbolAnimationSettings &QgsSymbol::animationSettings()
{
  return mAnimationSettings;
}

const QgsSymbolAnimationSettings &QgsSymbol::animationSettings() const
{
  return mAnimationSettings;
}

void QgsSymbol::setAnimationSettings( const QgsSymbolAnimationSettings &settings )
{
  mAnimationSettings = settings;
}

QgsSymbol *QgsSymbol::defaultSymbol( QgsWkbTypes::GeometryType geomType )
{
  std::unique_ptr< QgsSymbol > s;

  // override global default if project has a default for this type
  switch ( geomType )
  {
    case QgsWkbTypes::PointGeometry:
      s.reset( QgsProject::instance()->styleSettings()->defaultSymbol( Qgis::SymbolType::Marker ) );
      break;
    case QgsWkbTypes::LineGeometry:
      s.reset( QgsProject::instance()->styleSettings()->defaultSymbol( Qgis::SymbolType::Line ) );
      break;
    case QgsWkbTypes::PolygonGeometry:
      s.reset( QgsProject::instance()->styleSettings()->defaultSymbol( Qgis::SymbolType::Fill ) );
      break;
    default:
      break;
  }

  // if no default found for this type, get global default (as previously)
  if ( !s )
  {
    switch ( geomType )
    {
      case QgsWkbTypes::PointGeometry:
        s = std::make_unique< QgsMarkerSymbol >();
        break;
      case QgsWkbTypes::LineGeometry:
        s = std::make_unique< QgsLineSymbol >();
        break;
      case QgsWkbTypes::PolygonGeometry:
        s = std::make_unique< QgsFillSymbol >();
        break;
      default:
        QgsDebugMsg( QStringLiteral( "unknown layer's geometry type" ) );
        return nullptr;
    }
  }

  // set opacity
  s->setOpacity( QgsProject::instance()->styleSettings()->defaultSymbolOpacity() );

  // set random color, it project prefs allow
  if ( QgsProject::instance()->styleSettings()->randomizeDefaultSymbolColor() )
  {
    s->setColor( QgsApplication::colorSchemeRegistry()->fetchRandomStyleColor() );
  }

  return s.release();
}

QgsSymbolLayer *QgsSymbol::symbolLayer( int layer )
{
  return mLayers.value( layer );
}

const QgsSymbolLayer *QgsSymbol::symbolLayer( int layer ) const
{
  return mLayers.value( layer );
}

bool QgsSymbol::insertSymbolLayer( int index, QgsSymbolLayer *layer )
{
  if ( index < 0 || index > mLayers.count() ) // can be added also after the last index
    return false;

  if ( !layer || !layer->isCompatibleWithSymbol( this ) )
    return false;

  mLayers.insert( index, layer );
  return true;
}


bool QgsSymbol::appendSymbolLayer( QgsSymbolLayer *layer )
{
  if ( !layer || !layer->isCompatibleWithSymbol( this ) )
    return false;

  mLayers.append( layer );
  return true;
}


bool QgsSymbol::deleteSymbolLayer( int index )
{
  if ( index < 0 || index >= mLayers.count() )
    return false;

  delete mLayers.at( index );
  mLayers.removeAt( index );
  return true;
}


QgsSymbolLayer *QgsSymbol::takeSymbolLayer( int index )
{
  if ( index < 0 || index >= mLayers.count() )
    return nullptr;

  return mLayers.takeAt( index );
}


bool QgsSymbol::changeSymbolLayer( int index, QgsSymbolLayer *layer )
{
  QgsSymbolLayer *oldLayer = mLayers.value( index );

  if ( oldLayer == layer )
    return false;

  if ( !layer || !layer->isCompatibleWithSymbol( this ) )
    return false;

  delete oldLayer; // first delete the original layer
  mLayers[index] = layer; // set new layer
  return true;
}


void QgsSymbol::startRender( QgsRenderContext &context, const QgsFields &fields )
{
  Q_ASSERT_X( !mStarted, "startRender", "Rendering has already been started for this symbol instance!" );
  mStarted = true;

  mSymbolRenderContext.reset( new QgsSymbolRenderContext( context, QgsUnitTypes::RenderUnknownUnit, mOpacity, false, mRenderHints, nullptr, fields ) );

  // Why do we need a copy here ? Is it to make sure the symbol layer rendering does not mess with the symbol render context ?
  // Or is there another profound reason ?
  QgsSymbolRenderContext symbolContext( context, QgsUnitTypes::RenderUnknownUnit, mOpacity, false, mRenderHints, nullptr, fields );

  std::unique_ptr< QgsExpressionContextScope > scope( QgsExpressionContextUtils::updateSymbolScope( this, new QgsExpressionContextScope() ) );

  if ( mAnimationSettings.isAnimated() )
  {
    const long long mapFrameNumber = context.currentFrame();
    double animationTimeSeconds = 0;
    if ( mapFrameNumber >= 0 && context.frameRate() > 0 )
    {
      // render is part of an animation, so we base the calculated frame on that
      animationTimeSeconds = mapFrameNumber / context.frameRate();
    }
    else
    {
      // render is outside of animation, so base the calculated frame on the current epoch
      animationTimeSeconds = QDateTime::currentMSecsSinceEpoch() / 1000.0;
    }

    const long long symbolFrame = static_cast< long long >( std::floor( animationTimeSeconds * mAnimationSettings.frameRate() ) );
    scope->setVariable( QStringLiteral( "symbol_frame" ), symbolFrame, true );
  }

  mSymbolRenderContext->setExpressionContextScope( scope.release() );

  mDataDefinedProperties.prepare( context.expressionContext() );

  const auto constMLayers = mLayers;
  for ( QgsSymbolLayer *layer : constMLayers )
  {
    if ( !layer->enabled() || !context.isSymbolLayerEnabled( layer ) )
      continue;

    layer->prepareExpressions( symbolContext );
    layer->prepareMasks( symbolContext );
    layer->startRender( symbolContext );
  }
}

void QgsSymbol::stopRender( QgsRenderContext &context )
{
  Q_ASSERT_X( mStarted, "startRender", "startRender was not called for this symbol instance!" );
  mStarted = false;

  Q_UNUSED( context )
  if ( mSymbolRenderContext )
  {
    const auto constMLayers = mLayers;
    for ( QgsSymbolLayer *layer : constMLayers )
    {
      if ( !layer->enabled()  || !context.isSymbolLayerEnabled( layer ) )
        continue;

      layer->stopRender( *mSymbolRenderContext );
    }
  }

  mSymbolRenderContext.reset( nullptr );

  Q_NOWARN_DEPRECATED_PUSH
  mLayer = nullptr;
  Q_NOWARN_DEPRECATED_POP
}

void QgsSymbol::setColor( const QColor &color ) const
{
  const auto constMLayers = mLayers;
  for ( QgsSymbolLayer *layer : constMLayers )
  {
    if ( !layer->isLocked() )
      layer->setColor( color );
  }
}

QColor QgsSymbol::color() const
{
  for ( const QgsSymbolLayer *layer : mLayers )
  {
    // return color of the first unlocked layer
    if ( !layer->isLocked() )
    {
      const QColor layerColor = layer->color();
      if ( layerColor.isValid() )
        return layerColor;
    }
  }
  return QColor( 0, 0, 0 );
}

void QgsSymbol::drawPreviewIcon( QPainter *painter, QSize size, QgsRenderContext *customContext, bool selected, const QgsExpressionContext *expressionContext, const QgsLegendPatchShape *patchShape )
{
  QgsRenderContext *context = customContext;
  std::unique_ptr< QgsRenderContext > tempContext;
  if ( !context )
  {
    tempContext.reset( new QgsRenderContext( QgsRenderContext::fromQPainter( painter ) ) );
    context = tempContext.get();
    context->setFlag( Qgis::RenderContextFlag::RenderSymbolPreview, true );
  }

  const bool prevForceVector = context->forceVectorOutput();
  context->setForceVectorOutput( true );

  const double opacity = expressionContext ? dataDefinedProperties().valueAsDouble( QgsSymbol::PropertyOpacity, *expressionContext, mOpacity ) : mOpacity;

  QgsSymbolRenderContext symbolContext( *context, QgsUnitTypes::RenderUnknownUnit, opacity, false, mRenderHints, nullptr );
  symbolContext.setSelected( selected );
  switch ( mType )
  {
    case Qgis::SymbolType::Marker:
      symbolContext.setOriginalGeometryType( QgsWkbTypes::PointGeometry );
      break;
    case Qgis::SymbolType::Line:
      symbolContext.setOriginalGeometryType( QgsWkbTypes::LineGeometry );
      break;
    case Qgis::SymbolType::Fill:
      symbolContext.setOriginalGeometryType( QgsWkbTypes::PolygonGeometry );
      break;
    case Qgis::SymbolType::Hybrid:
      symbolContext.setOriginalGeometryType( QgsWkbTypes::UnknownGeometry );
      break;
  }

  if ( patchShape )
    symbolContext.setPatchShape( *patchShape );

  if ( !customContext && expressionContext )
  {
    context->setExpressionContext( *expressionContext );
  }
  else if ( !customContext )
  {
    // if no render context was passed, build a minimal expression context
    QgsExpressionContext expContext;
    expContext.appendScopes( QgsExpressionContextUtils::globalProjectLayerScopes( nullptr ) );
    context->setExpressionContext( expContext );
  }

  for ( QgsSymbolLayer *layer : std::as_const( mLayers ) )
  {
    if ( !layer->enabled()  || ( customContext && !customContext->isSymbolLayerEnabled( layer ) ) )
      continue;

    if ( mType == Qgis::SymbolType::Fill && layer->type() == Qgis::SymbolType::Line )
    {
      // line symbol layer would normally draw just a line
      // so we override this case to force it to draw a polygon stroke
      QgsLineSymbolLayer *lsl = dynamic_cast<QgsLineSymbolLayer *>( layer );
      if ( lsl )
      {
        // from QgsFillSymbolLayer::drawPreviewIcon() -- would be nicer to add the
        // symbol type to QgsSymbolLayer::drawPreviewIcon so this logic could be avoided!

        // hmm... why was this using size -1 ??
        const QSizeF targetSize = QSizeF( size.width() - 1, size.height() - 1 );

        const QList< QList< QPolygonF > > polys = patchShape ? patchShape->toQPolygonF( Qgis::SymbolType::Fill, targetSize )
            : QgsStyle::defaultStyle()->defaultPatchAsQPolygonF( Qgis::SymbolType::Fill, targetSize );

        lsl->startRender( symbolContext );
        QgsPaintEffect *effect = lsl->paintEffect();

        std::unique_ptr< QgsEffectPainter > effectPainter;
        if ( effect && effect->enabled() )
          effectPainter = std::make_unique< QgsEffectPainter >( symbolContext.renderContext(), effect );

        for ( const QList< QPolygonF > &poly : polys )
        {
          QVector< QPolygonF > rings;
          rings.reserve( poly.size() );
          for ( int i = 1; i < poly.size(); ++i )
            rings << poly.at( i );
          lsl->renderPolygonStroke( poly.value( 0 ), &rings, symbolContext );
        }

        effectPainter.reset();
        lsl->stopRender( symbolContext );
      }
    }
    else
      layer->drawPreviewIcon( symbolContext, size );
  }

  context->setForceVectorOutput( prevForceVector );
}

void QgsSymbol::exportImage( const QString &path, const QString &format, QSize size )
{
  if ( format.compare( QLatin1String( "svg" ), Qt::CaseInsensitive ) == 0 )
  {
    QSvgGenerator generator;
    generator.setFileName( path );
    generator.setSize( size );
    generator.setViewBox( QRect( 0, 0, size.height(), size.height() ) );

    QPainter painter( &generator );
    drawPreviewIcon( &painter, size );
    painter.end();
  }
  else
  {
    QImage image = asImage( size );
    image.save( path );
  }
}

QImage QgsSymbol::asImage( QSize size, QgsRenderContext *customContext )
{
  QImage image( size, QImage::Format_ARGB32_Premultiplied );
  image.fill( 0 );

  QPainter p( &image );
  p.setRenderHint( QPainter::Antialiasing );
  p.setRenderHint( QPainter::SmoothPixmapTransform );

  drawPreviewIcon( &p, size, customContext );

  return image;
}


QImage QgsSymbol::bigSymbolPreviewImage( QgsExpressionContext *expressionContext, Qgis::SymbolPreviewFlags flags )
{
  QImage preview( QSize( 100, 100 ), QImage::Format_ARGB32_Premultiplied );
  preview.fill( 0 );

  QPainter p( &preview );
  p.setRenderHint( QPainter::Antialiasing );
  p.translate( 0.5, 0.5 ); // shift by half a pixel to avoid blurring due antialiasing

  if ( mType == Qgis::SymbolType::Marker && flags & Qgis::SymbolPreviewFlag::FlagIncludeCrosshairsForMarkerSymbols )
  {
    p.setPen( QPen( Qt::gray ) );
    p.drawLine( 0, 50, 100, 50 );
    p.drawLine( 50, 0, 50, 100 );
  }

  QgsRenderContext context = QgsRenderContext::fromQPainter( &p );
  context.setFlag( Qgis::RenderContextFlag::RenderSymbolPreview );
  context.setFlag( Qgis::RenderContextFlag::Antialiasing );
  context.setFlag( Qgis::RenderContextFlag::HighQualityImageTransforms );
  context.setPainterFlagsUsingContext( &p );
  if ( expressionContext )
    context.setExpressionContext( *expressionContext );

  context.setIsGuiPreview( true );
  startRender( context );

  if ( mType == Qgis::SymbolType::Line )
  {
    QPolygonF poly;
    poly << QPointF( 0, 50 ) << QPointF( 99, 50 );
    static_cast<QgsLineSymbol *>( this )->renderPolyline( poly, nullptr, context );
  }
  else if ( mType == Qgis::SymbolType::Fill )
  {
    QPolygonF polygon;
    polygon << QPointF( 20, 20 ) << QPointF( 80, 20 ) << QPointF( 80, 80 ) << QPointF( 20, 80 ) << QPointF( 20, 20 );
    static_cast<QgsFillSymbol *>( this )->renderPolygon( polygon, nullptr, nullptr, context );
  }
  else // marker
  {
    static_cast<QgsMarkerSymbol *>( this )->renderPoint( QPointF( 50, 50 ), nullptr, context );
  }

  stopRender( context );
  return preview;
}

QImage QgsSymbol::bigSymbolPreviewImage( QgsExpressionContext *expressionContext, int flags )
{
  return bigSymbolPreviewImage( expressionContext, static_cast< Qgis::SymbolPreviewFlags >( flags ) );
}

QString QgsSymbol::dump() const
{
  QString t;
  switch ( type() )
  {
    case Qgis::SymbolType::Marker:
      t = QStringLiteral( "MARKER" );
      break;
    case Qgis::SymbolType::Line:
      t = QStringLiteral( "LINE" );
      break;
    case Qgis::SymbolType::Fill:
      t = QStringLiteral( "FILL" );
      break;
    default:
      Q_ASSERT( false && "unknown symbol type" );
  }
  QString s = QStringLiteral( "%1 SYMBOL (%2 layers) color %3" ).arg( t ).arg( mLayers.count() ).arg( QgsSymbolLayerUtils::encodeColor( color() ) );

  for ( QgsSymbolLayerList::const_iterator it = mLayers.begin(); it != mLayers.end(); ++it )
  {
    // TODO:
  }
  return s;
}

void QgsSymbol::toSld( QDomDocument &doc, QDomElement &element, QVariantMap props ) const
{
  props[ QStringLiteral( "alpha" )] = QString::number( opacity() );
  double scaleFactor = 1.0;
  props[ QStringLiteral( "uom" )] = QgsSymbolLayerUtils::encodeSldUom( outputUnit(), &scaleFactor );
  props[ QStringLiteral( "uomScale" )] = ( !qgsDoubleNear( scaleFactor, 1.0 ) ? qgsDoubleToString( scaleFactor ) : QString() );

  for ( QgsSymbolLayerList::const_iterator it = mLayers.begin(); it != mLayers.end(); ++it )
  {
    ( *it )->toSld( doc, element, props );
  }
}

QgsSymbolLayerList QgsSymbol::cloneLayers() const
{
  QgsSymbolLayerList lst;
  for ( QgsSymbolLayerList::const_iterator it = mLayers.begin(); it != mLayers.end(); ++it )
  {
    QgsSymbolLayer *layer = ( *it )->clone();
    layer->setLocked( ( *it )->isLocked() );
    layer->setRenderingPass( ( *it )->renderingPass() );
    layer->setEnabled( ( *it )->enabled() );
    lst.append( layer );
  }
  return lst;
}

void QgsSymbol::renderUsingLayer( QgsSymbolLayer *layer, QgsSymbolRenderContext &context, QgsWkbTypes::GeometryType geometryType, const QPolygonF *points, const QVector<QPolygonF> *rings )
{
  Q_ASSERT( layer->type() == Qgis::SymbolType::Hybrid );

  if ( layer->dataDefinedProperties().hasActiveProperties() && !layer->dataDefinedProperties().valueAsBool( QgsSymbolLayer::PropertyLayerEnabled, context.renderContext().expressionContext(), true ) )
    return;

  QgsGeometryGeneratorSymbolLayer *generatorLayer = static_cast<QgsGeometryGeneratorSymbolLayer *>( layer );

  QgsPaintEffect *effect = generatorLayer->paintEffect();
  if ( effect && effect->enabled() )
  {
    QgsEffectPainter p( context.renderContext(), effect );
    generatorLayer->render( context, geometryType, points, rings );
  }
  else
  {
    generatorLayer->render( context, geometryType, points, rings );
  }
}

QSet<QString> QgsSymbol::usedAttributes( const QgsRenderContext &context ) const
{
  // calling referencedFields() with ignoreContext=true because in our expression context
  // we do not have valid QgsFields yet - because of that the field names from expressions
  // wouldn't get reported
  QSet<QString> attributes = mDataDefinedProperties.referencedFields( context.expressionContext(), true );
  QgsSymbolLayerList::const_iterator sIt = mLayers.constBegin();
  for ( ; sIt != mLayers.constEnd(); ++sIt )
  {
    if ( *sIt )
    {
      attributes.unite( ( *sIt )->usedAttributes( context ) );
    }
  }
  return attributes;
}

void QgsSymbol::setDataDefinedProperty( QgsSymbol::Property key, const QgsProperty &property )
{
  mDataDefinedProperties.setProperty( key, property );
}

bool QgsSymbol::hasDataDefinedProperties() const
{
  if ( mDataDefinedProperties.hasActiveProperties() )
    return true;

  for ( QgsSymbolLayer *layer : mLayers )
  {
    if ( layer->hasDataDefinedProperties() )
      return true;
  }
  return false;
}

bool QgsSymbol::canCauseArtifactsBetweenAdjacentTiles() const
{
  for ( QgsSymbolLayer *layer : mLayers )
  {
    if ( layer->canCauseArtifactsBetweenAdjacentTiles() )
      return true;
  }
  return false;
}

void QgsSymbol::setLayer( const QgsVectorLayer *layer )
{
  Q_NOWARN_DEPRECATED_PUSH
  mLayer = layer;
  Q_NOWARN_DEPRECATED_POP
}

const QgsVectorLayer *QgsSymbol::layer() const
{
  Q_NOWARN_DEPRECATED_PUSH
  return mLayer;
  Q_NOWARN_DEPRECATED_POP
}

///@cond PRIVATE

/**
 * RAII class to pop scope from an expression context on destruction
 */
class ExpressionContextScopePopper
{
  public:

    ExpressionContextScopePopper() = default;

    ~ExpressionContextScopePopper()
    {
      if ( context )
        context->popScope();
    }

    QgsExpressionContext *context = nullptr;
};

/**
 * RAII class to restore original geometry on a render context on destruction
 */
class GeometryRestorer
{
  public:
    GeometryRestorer( QgsRenderContext &context )
      : mContext( context ),
        mGeometry( context.geometry() )
    {}

    ~GeometryRestorer()
    {
      mContext.setGeometry( mGeometry );
    }

  private:
    QgsRenderContext &mContext;
    const QgsAbstractGeometry *mGeometry;
};
///@endcond PRIVATE

void QgsSymbol::renderFeature( const QgsFeature &feature, QgsRenderContext &context, int layer, bool selected, bool drawVertexMarker, Qgis::VertexMarkerType currentVertexMarkerType, double currentVertexMarkerSize )
{
  if ( context.renderingStopped() )
    return;

  const QgsGeometry geom = feature.geometry();
  if ( geom.isNull() )
  {
    return;
  }

  GeometryRestorer geomRestorer( context );

  bool usingSegmentizedGeometry = false;
  context.setGeometry( geom.constGet() );

  if ( geom.type() != QgsWkbTypes::PointGeometry && !geom.boundingBox().isNull() )
  {
    try
    {
      const QPointF boundsOrigin = _getPoint( context, QgsPoint( geom.boundingBox().xMinimum(), geom.boundingBox().yMinimum() ) );
      if ( std::isfinite( boundsOrigin.x() ) && std::isfinite( boundsOrigin.y() ) )
        context.setTextureOrigin( boundsOrigin );
    }
    catch ( QgsCsException & )
    {

    }
  }

  bool clippingEnabled = clipFeaturesToExtent();
  // do any symbol layers prevent feature clipping?
  for ( QgsSymbolLayer *layer : std::as_const( mLayers ) )
  {
    if ( layer->flags() & Qgis::SymbolLayerFlag::DisableFeatureClipping )
    {
      clippingEnabled = false;
      break;
    }
  }
  if ( clippingEnabled && context.testFlag( Qgis::RenderContextFlag::RenderMapTile ) )
  {
    // If the "avoid artifacts between adjacent tiles" flag is set (RenderMapTile), then we'll force disable
    // the geometry clipping IF (and only if) this symbol can potentially have rendering artifacts when rendered as map tiles.
    // If the symbol won't have any artifacts anyway, then it's pointless and incredibly expensive to skip the clipping!
    if ( canCauseArtifactsBetweenAdjacentTiles() )
    {
      clippingEnabled = false;
    }
  }
  if ( context.extent().isEmpty() )
    clippingEnabled = false;

  mSymbolRenderContext->setGeometryPartCount( geom.constGet()->partCount() );
  mSymbolRenderContext->setGeometryPartNum( 1 );

  const bool needsExpressionContext = hasDataDefinedProperties();
  ExpressionContextScopePopper scopePopper;
  if ( mSymbolRenderContext->expressionContextScope() )
  {
    if ( needsExpressionContext )
    {
      // this is somewhat nasty - by appending this scope here it's now owned
      // by both mSymbolRenderContext AND context.expressionContext()
      // the RAII scopePopper is required to make sure it always has ownership transferred back
      // from context.expressionContext(), even if exceptions of other early exits occur in this
      // function
      context.expressionContext().appendScope( mSymbolRenderContext->expressionContextScope() );
      scopePopper.context = &context.expressionContext();

      QgsExpressionContextUtils::updateSymbolScope( this, mSymbolRenderContext->expressionContextScope() );
      mSymbolRenderContext->expressionContextScope()->addVariable( QgsExpressionContextScope::StaticVariable( QgsExpressionContext::EXPR_GEOMETRY_PART_COUNT, mSymbolRenderContext->geometryPartCount(), true ) );
      mSymbolRenderContext->expressionContextScope()->addVariable( QgsExpressionContextScope::StaticVariable( QgsExpressionContext::EXPR_GEOMETRY_PART_NUM, 1, true ) );
    }
  }

  // Collection of markers to paint, only used for no curve types.
  QPolygonF markers;

  QgsGeometry renderedBoundsGeom;

  // Step 1 - collect the set of painter coordinate geometries to render.
  // We do this upfront, because we only want to ever do this once, regardless how many symbol layers we need to render.

  struct PointInfo
  {
    QPointF renderPoint;
    const QgsPoint *originalGeometry = nullptr;
  };
  QVector< PointInfo > pointsToRender;

  struct LineInfo
  {
    QPolygonF renderLine;
    const QgsCurve *originalGeometry = nullptr;
  };
  QVector< LineInfo > linesToRender;

  struct PolygonInfo
  {
    QPolygonF renderExterior;
    QVector< QPolygonF > renderRings;
    const QgsCurvePolygon *originalGeometry = nullptr;
    int originalPartIndex = 0;
  };
  QVector< PolygonInfo > polygonsToRender;

  std::function< void ( const QgsAbstractGeometry *, int partIndex )> getPartGeometry;
  getPartGeometry = [&pointsToRender, &linesToRender, &polygonsToRender, &getPartGeometry, &context, &clippingEnabled, &markers, &feature, &usingSegmentizedGeometry, this]( const QgsAbstractGeometry * part, int partIndex = 0 )
  {
    Q_UNUSED( feature )

    if ( !part )
      return;

    // geometry preprocessing
    QgsGeometry temporaryGeometryContainer;
    const QgsAbstractGeometry *processedGeometry = nullptr;

    const bool isMultiPart = qgsgeometry_cast< const QgsGeometryCollection * >( part ) && qgsgeometry_cast< const QgsGeometryCollection * >( part )->numGeometries() > 1;

    if ( !isMultiPart )
    {
      // segmentize curved geometries
      const bool needsSegmentizing = QgsWkbTypes::isCurvedType( part->wkbType() ) || part->hasCurvedSegments();
      if ( needsSegmentizing )
      {
        std::unique_ptr< QgsAbstractGeometry > segmentizedPart( part->segmentize( context.segmentationTolerance(), context.segmentationToleranceType() ) );
        if ( !segmentizedPart )
        {
          return;
        }
        temporaryGeometryContainer.set( segmentizedPart.release() );
        processedGeometry = temporaryGeometryContainer.constGet();
        usingSegmentizedGeometry = true;
      }
      else
      {
        // no segmentation required
        processedGeometry = part;
      }

      // Simplify the geometry, if needed.
      if ( context.vectorSimplifyMethod().forceLocalOptimization() )
      {
        const int simplifyHints = context.vectorSimplifyMethod().simplifyHints();
        const QgsMapToPixelSimplifier simplifier( simplifyHints, context.vectorSimplifyMethod().tolerance(),
            static_cast< QgsMapToPixelSimplifier::SimplifyAlgorithm >( context.vectorSimplifyMethod().simplifyAlgorithm() ) );

        std::unique_ptr< QgsAbstractGeometry > simplified( simplifier.simplify( processedGeometry ) );
        if ( simplified )
        {
          temporaryGeometryContainer.set( simplified.release() );
          processedGeometry = temporaryGeometryContainer.constGet();
        }
      }

      // clip geometry to render context clipping regions
      if ( !context.featureClipGeometry().isEmpty() )
      {
        // apply feature clipping from context to the rendered geometry only -- just like the render time simplification,
        // we should NEVER apply this to the geometry attached to the feature itself. Doing so causes issues with certain
        // renderer settings, e.g. if polygons are being rendered using a rule based renderer based on the feature's area,
        // then we need to ensure that the original feature area is used instead of the clipped area..
        QgsGeos geos( processedGeometry );
        std::unique_ptr< QgsAbstractGeometry > clippedGeom( geos.intersection( context.featureClipGeometry().constGet() ) );
        if ( clippedGeom )
        {
          temporaryGeometryContainer.set( clippedGeom.release() );
          processedGeometry = temporaryGeometryContainer.constGet();
        }
      }
    }
    else
    {
      // for multipart geometries, the processing is deferred till we're rendering the actual part...
      processedGeometry = part;
    }

    if ( !processedGeometry )
    {
      // shouldn't happen!
      QgsDebugMsg( QStringLiteral( "No processed geometry to render for part!" ) );
      return;
    }

    switch ( QgsWkbTypes::flatType( processedGeometry->wkbType() ) )
    {
      case QgsWkbTypes::Point:
      {
        if ( mType != Qgis::SymbolType::Marker )
        {
          QgsDebugMsgLevel( QStringLiteral( "point can be drawn only with marker symbol!" ), 2 );
          break;
        }

        PointInfo info;
        info.originalGeometry = qgsgeometry_cast< const QgsPoint * >( part );
        info.renderPoint = _getPoint( context, *info.originalGeometry );
        pointsToRender << info;
        break;
      }

      case QgsWkbTypes::LineString:
      {
        if ( mType != Qgis::SymbolType::Line )
        {
          QgsDebugMsgLevel( QStringLiteral( "linestring can be drawn only with line symbol!" ), 2 );
          break;
        }

        LineInfo info;
        info.originalGeometry = qgsgeometry_cast<const QgsCurve *>( part );
        info.renderLine = _getLineString( context, *qgsgeometry_cast<const QgsCurve *>( processedGeometry ), clippingEnabled );
        linesToRender << info;
        break;
      }

      case QgsWkbTypes::Polygon:
      case QgsWkbTypes::Triangle:
      {
        QPolygonF pts;
        if ( mType != Qgis::SymbolType::Fill )
        {
          QgsDebugMsgLevel( QStringLiteral( "polygon can be drawn only with fill symbol!" ), 2 );
          break;
        }

        PolygonInfo info;
        info.originalGeometry = qgsgeometry_cast<const QgsCurvePolygon *>( part );
        info.originalPartIndex = partIndex;
        if ( !qgsgeometry_cast<const QgsPolygon *>( processedGeometry )->exteriorRing() )
        {
          QgsDebugMsg( QStringLiteral( "cannot render polygon with no exterior ring" ) );
          break;
        }

        _getPolygon( info.renderExterior, info.renderRings, context, *qgsgeometry_cast<const QgsPolygon *>( processedGeometry ), clippingEnabled, mForceRHR );
        polygonsToRender << info;
        break;
      }

      case QgsWkbTypes::MultiPoint:
      {
        const QgsMultiPoint *mp = qgsgeometry_cast< const QgsMultiPoint * >( processedGeometry );
        markers.reserve( mp->numGeometries() );
      }
      FALLTHROUGH
      case QgsWkbTypes::MultiCurve:
      case QgsWkbTypes::MultiLineString:
      case QgsWkbTypes::GeometryCollection:
      {
        const QgsGeometryCollection *geomCollection = qgsgeometry_cast<const QgsGeometryCollection *>( processedGeometry );

        const unsigned int num = geomCollection->numGeometries();
        for ( unsigned int i = 0; i < num; ++i )
        {
          if ( context.renderingStopped() )
            break;

          getPartGeometry( geomCollection->geometryN( i ), i );
        }
        break;
      }

      case QgsWkbTypes::MultiSurface:
      case QgsWkbTypes::MultiPolygon:
      {
        if ( mType != Qgis::SymbolType::Fill )
        {
          QgsDebugMsgLevel( QStringLiteral( "multi-polygon can be drawn only with fill symbol!" ), 2 );
          break;
        }

        QPolygonF pts;

        const QgsGeometryCollection *geomCollection = dynamic_cast<const QgsGeometryCollection *>( processedGeometry );
        const unsigned int num = geomCollection->numGeometries();

        // Sort components by approximate area (probably a bit faster than using
        // area() )
        std::map<double, QList<unsigned int> > thisAreaToPartNum;
        for ( unsigned int i = 0; i < num; ++i )
        {
          const QgsRectangle r( geomCollection->geometryN( i )->boundingBox() );
          thisAreaToPartNum[ r.width() * r.height()] << i;
        }

        // Draw starting with larger parts down to smaller parts, so that in
        // case of a part being incorrectly inside another part, it is drawn
        // on top of it (#15419)
        std::map<double, QList<unsigned int> >::const_reverse_iterator iter = thisAreaToPartNum.rbegin();
        for ( ; iter != thisAreaToPartNum.rend(); ++iter )
        {
          const QList<unsigned int> &listPartIndex = iter->second;
          for ( int idx = 0; idx < listPartIndex.size(); ++idx )
          {
            const unsigned i = listPartIndex[idx];
            getPartGeometry( geomCollection->geometryN( i ), i );
          }
        }
        break;
      }

      default:
        QgsDebugMsg( QStringLiteral( "feature %1: unsupported wkb type %2/%3 for rendering" )
                     .arg( feature.id() )
                     .arg( QgsWkbTypes::displayString( part->wkbType() ) )
                     .arg( part->wkbType(), 0, 16 ) );
    }
  };

  // Use the simplified type ref when rendering -- this avoids some unnecessary cloning/geometry modification
  // (e.g. if the original geometry is a compound curve containing only a linestring curve, we don't have
  // to segmentize the geometry before rendering)
  getPartGeometry( geom.constGet()->simplifiedTypeRef(), 0 );

  // step 2 - determine which layers to render
  std::vector< int > layers;
  if ( layer == -1 )
  {
    layers.reserve( mLayers.count() );
    for ( int i = 0; i < mLayers.count(); ++i )
      layers.emplace_back( i );
  }
  else
  {
    layers.emplace_back( layer );
  }

  // step 3 - render these geometries using the desired symbol layers.

  if ( needsExpressionContext )
    mSymbolRenderContext->expressionContextScope()->addVariable( QgsExpressionContextScope::StaticVariable( QStringLiteral( "symbol_layer_count" ), mLayers.count(), true ) );

  for ( const int symbolLayerIndex : layers )
  {
    QgsSymbolLayer *symbolLayer = mLayers.value( symbolLayerIndex );
    if ( !symbolLayer || !symbolLayer->enabled() )
      continue;

    if ( needsExpressionContext )
      mSymbolRenderContext->expressionContextScope()->addVariable( QgsExpressionContextScope::StaticVariable( QStringLiteral( "symbol_layer_index" ), symbolLayerIndex + 1, true ) );

    symbolLayer->startFeatureRender( feature, context );

    switch ( mType )
    {
      case Qgis::SymbolType::Marker:
      {
        int geometryPartNumber = 0;
        for ( const PointInfo &point : std::as_const( pointsToRender ) )
        {
          if ( context.renderingStopped() )
            break;

          mSymbolRenderContext->setGeometryPartNum( geometryPartNumber + 1 );
          if ( needsExpressionContext )
            mSymbolRenderContext->expressionContextScope()->addVariable( QgsExpressionContextScope::StaticVariable( QgsExpressionContext::EXPR_GEOMETRY_PART_NUM, geometryPartNumber + 1, true ) );

          static_cast<QgsMarkerSymbol *>( this )->renderPoint( point.renderPoint, &feature, context, symbolLayerIndex, selected );
          geometryPartNumber++;
        }

        break;
      }

      case Qgis::SymbolType::Line:
      {
        if ( linesToRender.empty() )
          break;

        int geometryPartNumber = 0;
        for ( const LineInfo &line : std::as_const( linesToRender ) )
        {
          if ( context.renderingStopped() )
            break;

          mSymbolRenderContext->setGeometryPartNum( geometryPartNumber + 1 );
          if ( needsExpressionContext )
            mSymbolRenderContext->expressionContextScope()->addVariable( QgsExpressionContextScope::StaticVariable( QgsExpressionContext::EXPR_GEOMETRY_PART_NUM, geometryPartNumber + 1, true ) );

          context.setGeometry( line.originalGeometry );
          static_cast<QgsLineSymbol *>( this )->renderPolyline( line.renderLine, &feature, context, symbolLayerIndex, selected );
          geometryPartNumber++;
        }
        break;
      }

      case Qgis::SymbolType::Fill:
      {
        for ( const PolygonInfo &info : std::as_const( polygonsToRender ) )
        {
          if ( context.renderingStopped() )
            break;

          mSymbolRenderContext->setGeometryPartNum( info.originalPartIndex + 1 );
          if ( needsExpressionContext )
            mSymbolRenderContext->expressionContextScope()->addVariable( QgsExpressionContextScope::StaticVariable( QgsExpressionContext::EXPR_GEOMETRY_PART_NUM, info.originalPartIndex + 1, true ) );

          context.setGeometry( info.originalGeometry );
          static_cast<QgsFillSymbol *>( this )->renderPolygon( info.renderExterior, ( !info.renderRings.isEmpty() ? &info.renderRings : nullptr ), &feature, context, symbolLayerIndex, selected );
        }

        break;
      }

      case Qgis::SymbolType::Hybrid:
        break;
    }

    symbolLayer->stopFeatureRender( feature, context );
  }

  // step 4 - handle post processing steps
  switch ( mType )
  {
    case Qgis::SymbolType::Marker:
    {
      markers.reserve( pointsToRender.size() );
      for ( const PointInfo &info : std::as_const( pointsToRender ) )
      {
        if ( context.hasRenderedFeatureHandlers() || context.testFlag( Qgis::RenderContextFlag::DrawSymbolBounds ) )
        {
          const QRectF bounds = static_cast<QgsMarkerSymbol *>( this )->bounds( info.renderPoint, context, feature );
          if ( context.hasRenderedFeatureHandlers() )
          {
            renderedBoundsGeom = renderedBoundsGeom.isNull() ? QgsGeometry::fromRect( bounds )
                                 : QgsGeometry::collectGeometry( QVector< QgsGeometry>() << QgsGeometry::fromRect( QgsRectangle( bounds ) ) << renderedBoundsGeom );
          }
          if ( context.testFlag( Qgis::RenderContextFlag::DrawSymbolBounds ) )
          {
            //draw debugging rect
            context.painter()->setPen( Qt::red );
            context.painter()->setBrush( QColor( 255, 0, 0, 100 ) );
            context.painter()->drawRect( bounds );
          }
        }

        if ( drawVertexMarker && !usingSegmentizedGeometry )
        {
          markers.append( info.renderPoint );
        }
      }
      break;
    }

    case Qgis::SymbolType::Line:
    {
      for ( const LineInfo &info : std::as_const( linesToRender ) )
      {
        if ( context.hasRenderedFeatureHandlers() && !info.renderLine.empty() )
        {
          renderedBoundsGeom = renderedBoundsGeom.isNull() ? QgsGeometry::fromQPolygonF( info.renderLine )
                               : QgsGeometry::collectGeometry( QVector< QgsGeometry>() << QgsGeometry::fromQPolygonF( info.renderLine ) << renderedBoundsGeom );
        }

        if ( drawVertexMarker && !usingSegmentizedGeometry )
        {
          markers << info.renderLine;
        }
      }
      break;
    }

    case Qgis::SymbolType::Fill:
    {
      int i = 0;
      for ( const PolygonInfo &info : std::as_const( polygonsToRender ) )
      {
        if ( context.hasRenderedFeatureHandlers() && !info.renderExterior.empty() )
        {
          renderedBoundsGeom = renderedBoundsGeom.isNull() ? QgsGeometry::fromQPolygonF( info.renderExterior )
                               : QgsGeometry::collectGeometry( QVector< QgsGeometry>() << QgsGeometry::fromQPolygonF( info.renderExterior ) << renderedBoundsGeom );
          // TODO: consider holes?
        }

        if ( drawVertexMarker && !usingSegmentizedGeometry )
        {
          markers << info.renderExterior;

          for ( const QPolygonF &hole : info.renderRings )
          {
            markers << hole;
          }
        }
        i++;
      }
      break;
    }

    case Qgis::SymbolType::Hybrid:
      break;
  }

  if ( context.hasRenderedFeatureHandlers() && !renderedBoundsGeom.isNull() )
  {
    QgsRenderedFeatureHandlerInterface::RenderedFeatureContext featureContext( context );
    const QList< QgsRenderedFeatureHandlerInterface * > handlers = context.renderedFeatureHandlers();
    for ( QgsRenderedFeatureHandlerInterface *handler : handlers )
      handler->handleRenderedFeature( feature, renderedBoundsGeom, featureContext );
  }

  if ( drawVertexMarker )
  {
    if ( !markers.isEmpty() && !context.renderingStopped() )
    {
      const auto constMarkers = markers;
      for ( QPointF marker : constMarkers )
      {
        renderVertexMarker( marker, context, currentVertexMarkerType, currentVertexMarkerSize );
      }
    }
    else
    {
      QgsCoordinateTransform ct = context.coordinateTransform();
      const QgsMapToPixel &mtp = context.mapToPixel();

      QgsPoint vertexPoint;
      QgsVertexId vertexId;
      double x, y, z;
      QPointF mapPoint;
      while ( geom.constGet()->nextVertex( vertexId, vertexPoint ) )
      {
        //transform
        x = vertexPoint.x();
        y = vertexPoint.y();
        z = 0.0;
        if ( ct.isValid() )
        {
          ct.transformInPlace( x, y, z );
        }
        mapPoint.setX( x );
        mapPoint.setY( y );
        mtp.transformInPlace( mapPoint.rx(), mapPoint.ry() );
        renderVertexMarker( mapPoint, context, currentVertexMarkerType, currentVertexMarkerSize );
      }
    }
  }
}

QgsSymbolRenderContext *QgsSymbol::symbolRenderContext()
{
  return mSymbolRenderContext.get();
}

void QgsSymbol::renderVertexMarker( QPointF pt, QgsRenderContext &context, Qgis::VertexMarkerType currentVertexMarkerType, double currentVertexMarkerSize )
{
  int markerSize = context.convertToPainterUnits( currentVertexMarkerSize, QgsUnitTypes::RenderMillimeters );
  QgsSymbolLayerUtils::drawVertexMarker( pt.x(), pt.y(), *context.painter(), currentVertexMarkerType, markerSize );
}

void QgsSymbol::initPropertyDefinitions()
{
  if ( !sPropertyDefinitions.isEmpty() )
    return;

  QString origin = QStringLiteral( "symbol" );

  sPropertyDefinitions = QgsPropertiesDefinition
  {
    { QgsSymbol::PropertyOpacity, QgsPropertyDefinition( "alpha", QObject::tr( "Opacity" ), QgsPropertyDefinition::Opacity, origin )},
  };
}

void QgsSymbol::startFeatureRender( const QgsFeature &feature, QgsRenderContext &context, const int layer )
{
  if ( layer != -1 )
  {
    QgsSymbolLayer *symbolLayer = mLayers.value( layer );
    if ( symbolLayer && symbolLayer->enabled() )
    {
      symbolLayer->startFeatureRender( feature, context );
    }
    return;
  }
  else
  {
    const QList< QgsSymbolLayer * > layers = mLayers;
    for ( QgsSymbolLayer *symbolLayer : layers )
    {
      if ( !symbolLayer->enabled() )
        continue;

      symbolLayer->startFeatureRender( feature, context );
    }
  }
}

void QgsSymbol::stopFeatureRender( const QgsFeature &feature, QgsRenderContext &context, int layer )
{
  if ( layer != -1 )
  {
    QgsSymbolLayer *symbolLayer = mLayers.value( layer );
    if ( symbolLayer && symbolLayer->enabled() )
    {
      symbolLayer->stopFeatureRender( feature, context );
    }
    return;
  }
  else
  {
    const QList< QgsSymbolLayer * > layers = mLayers;
    for ( QgsSymbolLayer *symbolLayer : layers )
    {
      if ( !symbolLayer->enabled() )
        continue;

      symbolLayer->stopFeatureRender( feature, context );
    }
  }
}

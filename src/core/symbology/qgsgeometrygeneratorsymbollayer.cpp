/***************************************************************************
 qgsgeometrygeneratorsymbollayer.cpp
 ---------------------
 begin                : November 2015
 copyright            : (C) 2015 by Matthias Kuhn
 email                : matthias at opengis dot ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsgeometrygeneratorsymbollayer.h"
#include "qgsgeometry.h"
#include "qgsmarkersymbol.h"
#include "qgslinesymbol.h"
#include "qgsfillsymbol.h"
#include "qgspolygon.h"

#include "qgsexpressioncontextutils.h"

QgsGeometryGeneratorSymbolLayer::~QgsGeometryGeneratorSymbolLayer() = default;

QgsSymbolLayer *QgsGeometryGeneratorSymbolLayer::create( const QVariantMap &properties )
{
  QString expression = properties.value( QStringLiteral( "geometryModifier" ) ).toString();
  if ( expression.isEmpty() )
  {
    expression = QStringLiteral( "$geometry" );
  }
  QgsGeometryGeneratorSymbolLayer *symbolLayer = new QgsGeometryGeneratorSymbolLayer( expression );

  if ( properties.value( QStringLiteral( "SymbolType" ) ) == QLatin1String( "Marker" ) )
  {
    symbolLayer->setSubSymbol( QgsMarkerSymbol::createSimple( properties ) );
  }
  else if ( properties.value( QStringLiteral( "SymbolType" ) ) == QLatin1String( "Line" ) )
  {
    symbolLayer->setSubSymbol( QgsLineSymbol::createSimple( properties ) );
  }
  else
  {
    symbolLayer->setSubSymbol( QgsFillSymbol::createSimple( properties ) );
  }
  symbolLayer->setUnits( QgsUnitTypes::decodeRenderUnit( properties.value( QStringLiteral( "units" ), QStringLiteral( "mapunits" ) ).toString() ) );

  symbolLayer->restoreOldDataDefinedProperties( properties );

  return symbolLayer;
}

QgsGeometryGeneratorSymbolLayer::QgsGeometryGeneratorSymbolLayer( const QString &expression )
  : QgsSymbolLayer( Qgis::SymbolType::Hybrid )
  , mExpression( new QgsExpression( expression ) )
  , mSymbolType( Qgis::SymbolType::Marker )
{

}

QString QgsGeometryGeneratorSymbolLayer::layerType() const
{
  return QStringLiteral( "GeometryGenerator" );
}

void QgsGeometryGeneratorSymbolLayer::setSymbolType( Qgis::SymbolType symbolType )
{
  if ( symbolType == Qgis::SymbolType::Fill )
  {
    if ( !mFillSymbol )
      mFillSymbol.reset( QgsFillSymbol::createSimple( QVariantMap() ) );
    mSymbol = mFillSymbol.get();
  }
  else if ( symbolType == Qgis::SymbolType::Line )
  {
    if ( !mLineSymbol )
      mLineSymbol.reset( QgsLineSymbol::createSimple( QVariantMap() ) );
    mSymbol = mLineSymbol.get();
  }
  else if ( symbolType == Qgis::SymbolType::Marker )
  {
    if ( !mMarkerSymbol )
      mMarkerSymbol.reset( QgsMarkerSymbol::createSimple( QVariantMap() ) );
    mSymbol = mMarkerSymbol.get();
  }
  else
    Q_ASSERT( false );

  mSymbolType = symbolType;
}

void QgsGeometryGeneratorSymbolLayer::startRender( QgsSymbolRenderContext &context )
{
  mExpression->prepare( &context.renderContext().expressionContext() );

  subSymbol()->startRender( context.renderContext() );
}

void QgsGeometryGeneratorSymbolLayer::stopRender( QgsSymbolRenderContext &context )
{
  if ( mSymbol )
    mSymbol->stopRender( context.renderContext() );
}

void QgsGeometryGeneratorSymbolLayer::startFeatureRender( const QgsFeature &, QgsRenderContext & )
{
  mRenderingFeature = true;
  mHasRenderedFeature = false;
}

void QgsGeometryGeneratorSymbolLayer::stopFeatureRender( const QgsFeature &, QgsRenderContext & )
{
  mRenderingFeature = false;
}

bool QgsGeometryGeneratorSymbolLayer::usesMapUnits() const
{
  if ( mFillSymbol )
    return mFillSymbol->usesMapUnits();
  else if ( mLineSymbol )
    return mLineSymbol->usesMapUnits();
  else if ( mMarkerSymbol )
    return mMarkerSymbol->usesMapUnits();
  return false;
}

QColor QgsGeometryGeneratorSymbolLayer::color() const
{
  if ( mFillSymbol )
    return mFillSymbol->color();
  else if ( mLineSymbol )
    return mLineSymbol->color();
  else if ( mMarkerSymbol )
    return mMarkerSymbol->color();
  return QColor();
}

QgsUnitTypes::RenderUnit QgsGeometryGeneratorSymbolLayer::outputUnit() const
{
  if ( mFillSymbol )
    return mFillSymbol->outputUnit();
  else if ( mLineSymbol )
    return mLineSymbol->outputUnit();
  else if ( mMarkerSymbol )
    return mMarkerSymbol->outputUnit();
  return QgsUnitTypes::RenderUnknownUnit;
}

QgsMapUnitScale QgsGeometryGeneratorSymbolLayer::mapUnitScale() const
{
  if ( mFillSymbol )
    return mFillSymbol->mapUnitScale();
  else if ( mLineSymbol )
    return mLineSymbol->mapUnitScale();
  else if ( mMarkerSymbol )
    return mMarkerSymbol->mapUnitScale();
  return QgsMapUnitScale();
}

QgsSymbolLayer *QgsGeometryGeneratorSymbolLayer::clone() const
{
  QgsGeometryGeneratorSymbolLayer *clone = new QgsGeometryGeneratorSymbolLayer( mExpression->expression() );

  if ( mFillSymbol )
    clone->mFillSymbol.reset( mFillSymbol->clone() );
  if ( mLineSymbol )
    clone->mLineSymbol.reset( mLineSymbol->clone() );
  if ( mMarkerSymbol )
    clone->mMarkerSymbol.reset( mMarkerSymbol->clone() );

  clone->setSymbolType( mSymbolType );
  clone->setUnits( mUnits );

  copyDataDefinedProperties( clone );
  copyPaintEffect( clone );

  return clone;
}

QVariantMap QgsGeometryGeneratorSymbolLayer::properties() const
{
  QVariantMap props;
  props.insert( QStringLiteral( "geometryModifier" ), mExpression->expression() );
  switch ( mSymbolType )
  {
    case Qgis::SymbolType::Marker:
      props.insert( QStringLiteral( "SymbolType" ), QStringLiteral( "Marker" ) );
      break;
    case Qgis::SymbolType::Line:
      props.insert( QStringLiteral( "SymbolType" ), QStringLiteral( "Line" ) );
      break;
    default:
      props.insert( QStringLiteral( "SymbolType" ), QStringLiteral( "Fill" ) );
      break;
  }
  props.insert( QStringLiteral( "units" ), QgsUnitTypes::encodeUnit( mUnits ) );

  return props;
}

void QgsGeometryGeneratorSymbolLayer::drawPreviewIcon( QgsSymbolRenderContext &context, QSize size )
{
  if ( mSymbol )
    mSymbol->drawPreviewIcon( context.renderContext().painter(), size, nullptr, false, nullptr, context.patchShape() );
}

void QgsGeometryGeneratorSymbolLayer::setGeometryExpression( const QString &exp )
{
  mExpression.reset( new QgsExpression( exp ) );
}

bool QgsGeometryGeneratorSymbolLayer::setSubSymbol( QgsSymbol *symbol )
{
  switch ( symbol->type() )
  {
    case Qgis::SymbolType::Marker:
      mMarkerSymbol.reset( static_cast<QgsMarkerSymbol *>( symbol ) );
      break;

    case Qgis::SymbolType::Line:
      mLineSymbol.reset( static_cast<QgsLineSymbol *>( symbol ) );
      break;

    case Qgis::SymbolType::Fill:
      mFillSymbol.reset( static_cast<QgsFillSymbol *>( symbol ) );
      break;

    default:
      break;
  }

  setSymbolType( symbol->type() );

  return true;
}

QSet<QString> QgsGeometryGeneratorSymbolLayer::usedAttributes( const QgsRenderContext &context ) const
{
  return QgsSymbolLayer::usedAttributes( context )
         + mSymbol->usedAttributes( context )
         + mExpression->referencedColumns();
}

bool QgsGeometryGeneratorSymbolLayer::hasDataDefinedProperties() const
{
  // we treat geometry generator layers like they have data defined properties,
  // since the WHOLE layer is based on expressions and requires the full expression
  // context
  return true;
}

bool QgsGeometryGeneratorSymbolLayer::isCompatibleWithSymbol( QgsSymbol *symbol ) const
{
  Q_UNUSED( symbol )
  return true;
}
void QgsGeometryGeneratorSymbolLayer::render( QgsSymbolRenderContext &context, QgsWkbTypes::GeometryType geometryType, const QPolygonF *points, const QVector<QPolygonF> *rings )
{
  if ( mRenderingFeature && mHasRenderedFeature )
    return;

  QgsExpressionContext &expressionContext = context.renderContext().expressionContext();
  QgsFeature f = expressionContext.feature();

  if ( !context.feature() && points )
  {
    // oh dear, we don't have a feature to work from... but that's ok, we are probably being rendered as a plain old symbol!
    // in this case we need to build up a feature which represents the points being rendered
    QgsGeometry drawGeometry;

    // step 1 - convert points and rings to geometry
    switch ( geometryType )
    {
      case QgsWkbTypes::PointGeometry:
      {
        Q_ASSERT( points->size() == 1 );
        drawGeometry = QgsGeometry::fromPointXY( points->at( 0 ) );
        break;
      }
      case QgsWkbTypes::LineGeometry:
      {
        Q_ASSERT( !rings );
        std::unique_ptr < QgsLineString > ring( QgsLineString::fromQPolygonF( *points ) );
        drawGeometry = QgsGeometry( std::move( ring ) );
        break;
      }
      case QgsWkbTypes::PolygonGeometry:
      {
        std::unique_ptr < QgsLineString > exterior( QgsLineString::fromQPolygonF( *points ) );
        std::unique_ptr< QgsPolygon > polygon = std::make_unique< QgsPolygon >();
        polygon->setExteriorRing( exterior.release() );
        if ( rings )
        {
          for ( const QPolygonF &ring : *rings )
          {
            polygon->addInteriorRing( QgsLineString::fromQPolygonF( ring ) );
          }
        }
        drawGeometry = QgsGeometry( std::move( polygon ) );
        break;
      }

      case QgsWkbTypes::UnknownGeometry:
      case QgsWkbTypes::NullGeometry:
        return; // unreachable
    }

    // step 2 - scale the draw geometry from PAINTER units to target units (e.g. millimeters)
    const double scale = 1 / context.renderContext().convertToPainterUnits( 1, mUnits );
    const QTransform painterToTargetUnits = QTransform::fromScale( scale, scale );
    drawGeometry.transform( painterToTargetUnits );

    // step 3 - set the feature to use the new scaled geometry, and inject it into the expression context
    f.setGeometry( drawGeometry );
    QgsExpressionContextScope *generatorScope = new QgsExpressionContextScope();
    QgsExpressionContextScopePopper popper( expressionContext, generatorScope );
    generatorScope->setFeature( f );

    // step 4 - evaluate the new generated geometry.
    QgsGeometry geom = mExpression->evaluate( &expressionContext ).value<QgsGeometry>();

    // step 5 - transform geometry back from target units to MAP units. We transform to map units here
    // as we'll ultimately be calling renderFeature, which excepts the feature has a geometry in map units.
    // Here we also scale the transform by the target unit to painter units factor to reverse that conversion
    geom.transform( painterToTargetUnits.inverted( ) );
    QTransform mapToPixel = context.renderContext().mapToPixel().transform();
    geom.transform( mapToPixel.inverted() );
    // also need to apply the coordinate transform from the render context
    try
    {
      geom.transform( context.renderContext().coordinateTransform(), QgsCoordinateTransform::ReverseTransform );
    }
    catch ( QgsCsException & )
    {
      QgsDebugMsg( QStringLiteral( "Could no transform generated geometry to layer CRS" ) );
    }

    f.setGeometry( geom );
  }
  else if ( context.feature() )
  {
    switch ( mUnits )
    {
      case QgsUnitTypes::RenderMapUnits:
      case QgsUnitTypes::RenderUnknownUnit: // unsupported, not exposed as an option
      case QgsUnitTypes::RenderMetersInMapUnits: // unsupported, not exposed as an option
      case QgsUnitTypes::RenderPercentage: // unsupported, not exposed as an option
      {
        QgsGeometry geom = mExpression->evaluate( &expressionContext ).value<QgsGeometry>();
        f.setGeometry( geom );
        break;
      }

      case QgsUnitTypes::RenderMillimeters:
      case QgsUnitTypes::RenderPixels:
      case QgsUnitTypes::RenderPoints:
      case QgsUnitTypes::RenderInches:
      {
        // add a new scope for the transformed geometry
        QgsExpressionContextScope *generatorScope = new QgsExpressionContextScope();
        QgsExpressionContextScopePopper popper( expressionContext, generatorScope );

        QgsGeometry transformed = f.geometry();
        transformed.transform( context.renderContext().coordinateTransform() );
        QTransform mapToPixel = context.renderContext().mapToPixel().transform();

        // scale transform to target units
        const double scale = 1 / context.renderContext().convertToPainterUnits( 1, mUnits );
        mapToPixel.scale( scale, scale );

        transformed.transform( mapToPixel );
        f.setGeometry( transformed );
        generatorScope->setFeature( f );

        QgsGeometry geom = mExpression->evaluate( &expressionContext ).value<QgsGeometry>();

        // transform geometry back from screen units to layer crs
        geom.transform( mapToPixel.inverted() );
        geom.transform( context.renderContext().coordinateTransform(), Qgis::TransformDirection::Reverse );

        f.setGeometry( geom );
        break;
      }
    }
  }

  QgsExpressionContextScope *subSymbolExpressionContextScope = mSymbol->symbolRenderContext()->expressionContextScope();

  subSymbolExpressionContextScope->setFeature( f );

  mSymbol->renderFeature( f, context.renderContext(), -1, context.selected() );

  if ( mRenderingFeature )
    mHasRenderedFeature = true;
}

void QgsGeometryGeneratorSymbolLayer::setColor( const QColor &color )
{
  mSymbol->setColor( color );
}

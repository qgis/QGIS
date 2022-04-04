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
#include "qgslegendpatchshape.h"
#include "qgsstyle.h"

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

void QgsGeometryGeneratorSymbolLayer::startFeatureRender( const QgsFeature &, QgsRenderContext &context )
{
  if ( context.flags() & Qgis::RenderContextFlag::RenderingSubSymbol )
    return;

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
  {
    // evaluate expression
    QgsGeometry patchShapeGeometry;

    if ( context.patchShape() && !context.patchShape()->isNull() )
    {
      patchShapeGeometry = context.patchShape()->scaledGeometry( size );
    }
    if ( patchShapeGeometry.isEmpty() )
    {
      Qgis::SymbolType originalSymbolType = Qgis::SymbolType::Hybrid;
      switch ( context.originalGeometryType() )
      {
        case QgsWkbTypes::PointGeometry:
          originalSymbolType = Qgis::SymbolType::Marker;
          break;
        case QgsWkbTypes::LineGeometry:
          originalSymbolType = Qgis::SymbolType::Line;
          break;
        case QgsWkbTypes::PolygonGeometry:
          originalSymbolType = Qgis::SymbolType::Fill;
          break;
        case QgsWkbTypes::UnknownGeometry:
        case QgsWkbTypes::NullGeometry:
          originalSymbolType = mSymbol->type();
          break;
      }
      patchShapeGeometry = QgsStyle::defaultStyle()->defaultPatch( originalSymbolType, size ).scaledGeometry( size );
    }

    // evaluate geometry expression
    QgsFeature feature;
    if ( context.feature() )
      feature = *context.feature();
    else
      feature.setGeometry( patchShapeGeometry );
    const QgsGeometry iconGeometry = evaluateGeometryInPainterUnits( patchShapeGeometry, feature, context.renderContext(), context.renderContext().expressionContext() );

    QgsLegendPatchShape evaluatedPatchShape( mSymbol->type(), coerceToExpectedType( iconGeometry ) );
    // we don't want to rescale the patch shape to fit the legend symbol size -- we've already considered that here,
    // and we don't want to undo the effects of a geometry generator which modifies the symbol bounds
    evaluatedPatchShape.setScaleToOutputSize( false );
    mSymbol->drawPreviewIcon( context.renderContext().painter(), size, &context.renderContext(), false, &context.renderContext().expressionContext(), &evaluatedPatchShape );
  }
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

QgsGeometry QgsGeometryGeneratorSymbolLayer::evaluateGeometryInPainterUnits( const QgsGeometry &input, const QgsFeature &, const QgsRenderContext &renderContext, QgsExpressionContext &expressionContext ) const
{
  QgsGeometry drawGeometry( input );
  // step 1 - scale the draw geometry from PAINTER units to target units (e.g. millimeters)
  const double scale = 1 / renderContext.convertToPainterUnits( 1, mUnits );
  const QTransform painterToTargetUnits = QTransform::fromScale( scale, scale );
  drawGeometry.transform( painterToTargetUnits );

  // step 2 - set the feature to use the new scaled geometry, and inject it into the expression context
  QgsExpressionContextScope *generatorScope = new QgsExpressionContextScope();
  QgsExpressionContextScopePopper popper( expressionContext, generatorScope );
  generatorScope->setGeometry( drawGeometry );

  // step 3 - evaluate the new generated geometry.
  QgsGeometry geom = mExpression->evaluate( &expressionContext ).value<QgsGeometry>();

  // step 4 - transform geometry back from target units to painter units
  geom.transform( painterToTargetUnits.inverted( ) );

  return geom;
}

QgsGeometry QgsGeometryGeneratorSymbolLayer::coerceToExpectedType( const QgsGeometry &geometry ) const
{
  switch ( mSymbolType )
  {
    case Qgis::SymbolType::Marker:
      if ( geometry.type() != QgsWkbTypes::PointGeometry )
      {
        QVector< QgsGeometry > geoms = geometry.coerceToType( QgsWkbTypes::MultiPoint );
        if ( !geoms.empty() )
          return geoms.at( 0 );
      }
      break;
    case Qgis::SymbolType::Line:
      if ( geometry.type() != QgsWkbTypes::LineGeometry )
      {
        QVector< QgsGeometry > geoms = geometry.coerceToType( QgsWkbTypes::MultiLineString );
        if ( !geoms.empty() )
          return geoms.at( 0 );
      }
      break;
    case Qgis::SymbolType::Fill:
      if ( geometry.type() != QgsWkbTypes::PolygonGeometry )
      {
        QVector< QgsGeometry > geoms = geometry.coerceToType( QgsWkbTypes::MultiPolygon );
        if ( !geoms.empty() )
          return geoms.at( 0 );
      }
      break;
    case Qgis::SymbolType::Hybrid:
      break;
  }
  return geometry;
}

void QgsGeometryGeneratorSymbolLayer::render( QgsSymbolRenderContext &context, QgsWkbTypes::GeometryType geometryType, const QPolygonF *points, const QVector<QPolygonF> *rings )
{
  if ( mRenderingFeature && mHasRenderedFeature )
    return;

  QgsExpressionContext &expressionContext = context.renderContext().expressionContext();
  QgsFeature f = expressionContext.feature();

  if ( ( !context.feature() || context.renderContext().flags() & Qgis::RenderContextFlag::RenderingSubSymbol ) && points )
  {
    // oh dear, we don't have a feature to work from... but that's ok, we are probably being rendered as a plain old symbol!
    // in this case we need to build up a feature which represents the points being rendered.
    // note that we also do this same logic when we are rendering a subsymbol. In that case the $geometry part of the
    // expression should refer to the shape of the subsymbol being rendered, NOT the feature's original geometry
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

    // step 2 - evaluate the result
    QgsGeometry result = evaluateGeometryInPainterUnits( drawGeometry, f, context.renderContext(), expressionContext );

    // We transform back to map units here (from painter units)
    // as we'll ultimately be calling renderFeature, which excepts the feature has a geometry in map units.
    // Here we also scale the transform by the target unit to painter units factor to reverse that conversion
    QTransform mapToPixel = context.renderContext().mapToPixel().transform();
    result.transform( mapToPixel.inverted() );
    // also need to apply the coordinate transform from the render context
    try
    {
      result.transform( context.renderContext().coordinateTransform(), Qgis::TransformDirection::Reverse );
    }
    catch ( QgsCsException & )
    {
      QgsDebugMsg( QStringLiteral( "Could no transform generated geometry to layer CRS" ) );
    }

    f.setGeometry( coerceToExpectedType( result ) );
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
        f.setGeometry( coerceToExpectedType( geom ) );
        break;
      }

      case QgsUnitTypes::RenderMillimeters:
      case QgsUnitTypes::RenderPixels:
      case QgsUnitTypes::RenderPoints:
      case QgsUnitTypes::RenderInches:
      {
        // convert feature geometry to painter units
        QgsGeometry transformed = f.geometry();
        transformed.transform( context.renderContext().coordinateTransform() );
        const QTransform mapToPixel = context.renderContext().mapToPixel().transform();
        transformed.transform( mapToPixel );

        QgsGeometry result = evaluateGeometryInPainterUnits( transformed, f, context.renderContext(), expressionContext );

        // We transform back to map units here (from painter units)
        // as we'll ultimately be calling renderFeature, which excepts the feature has a geometry in map units.
        // Here we also scale the transform by the target unit to painter units factor to reverse that conversion
        result.transform( mapToPixel.inverted() );
        // also need to apply the coordinate transform from the render context
        try
        {
          result.transform( context.renderContext().coordinateTransform(), Qgis::TransformDirection::Reverse );
        }
        catch ( QgsCsException & )
        {
          QgsDebugMsg( QStringLiteral( "Could no transform generated geometry to layer CRS" ) );
        }
        f.setGeometry( coerceToExpectedType( result ) );
        break;
      }
    }
  }

  QgsExpressionContextScope *subSymbolExpressionContextScope = mSymbol->symbolRenderContext()->expressionContextScope();
  // override the $geometry value for all subsymbols -- this should be the generated geometry
  subSymbolExpressionContextScope->setGeometry( f.geometry() );

  const bool prevIsSubsymbol = context.renderContext().flags() & Qgis::RenderContextFlag::RenderingSubSymbol;
  context.renderContext().setFlag( Qgis::RenderContextFlag::RenderingSubSymbol );

  mSymbol->renderFeature( f, context.renderContext(), -1, context.selected() );

  context.renderContext().setFlag( Qgis::RenderContextFlag::RenderingSubSymbol, prevIsSubsymbol );

  if ( mRenderingFeature )
    mHasRenderedFeature = true;
}

void QgsGeometryGeneratorSymbolLayer::setColor( const QColor &color )
{
  mSymbol->setColor( color );
}

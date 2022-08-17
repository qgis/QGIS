/***************************************************************************
                             qgscallout.cpp
                             ----------------
    begin                : July 2019
    copyright            : (C) 2019 Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgscallout.h"
#include "qgsrendercontext.h"
#include "qgssymbol.h"
#include "qgslinesymbollayer.h"
#include "qgsfillsymbollayer.h"
#include "qgssymbollayerutils.h"
#include "qgsxmlutils.h"
#include "qgslinestring.h"
#include "qgsvariantutils.h"
#include "qgsgeos.h"
#include "qgsgeometryutils.h"
#include "qgscircularstring.h"
#include "qgsshapegenerator.h"
#include "qgspainting.h"
#include "qgsfillsymbol.h"
#include "qgslinesymbol.h"

#include <mutex>

QgsPropertiesDefinition QgsCallout::sPropertyDefinitions;

void QgsCallout::initPropertyDefinitions()
{
  const QString origin = QStringLiteral( "callouts" );

  sPropertyDefinitions = QgsPropertiesDefinition
  {
    { QgsCallout::MinimumCalloutLength, QgsPropertyDefinition( "MinimumCalloutLength", QObject::tr( "Minimum callout length" ), QgsPropertyDefinition::DoublePositive, origin ) },
    { QgsCallout::OffsetFromAnchor, QgsPropertyDefinition( "OffsetFromAnchor", QObject::tr( "Offset from feature" ), QgsPropertyDefinition::DoublePositive, origin ) },
    { QgsCallout::OffsetFromLabel, QgsPropertyDefinition( "OffsetFromLabel", QObject::tr( "Offset from label" ), QgsPropertyDefinition::DoublePositive, origin ) },
    { QgsCallout::DrawCalloutToAllParts, QgsPropertyDefinition( "DrawCalloutToAllParts", QObject::tr( "Draw lines to all feature parts" ), QgsPropertyDefinition::Boolean, origin ) },
    { QgsCallout::AnchorPointPosition, QgsPropertyDefinition( "AnchorPointPosition", QgsPropertyDefinition::DataTypeString, QObject::tr( "Feature's anchor point position" ), QObject::tr( "string " ) + "[<b>pole_of_inaccessibility</b>|<b>point_on_exterior</b>|<b>point_on_surface</b>|<b>centroid</b>]", origin ) },
    {
      QgsCallout::LabelAnchorPointPosition, QgsPropertyDefinition( "LabelAnchorPointPosition", QgsPropertyDefinition::DataTypeString, QObject::tr( "Label's anchor point position" ), QObject::tr( "string " ) + "[<b>point_on_exterior</b>|<b>centroid</b>|<b>TL</b>=Top left|<b>T</b>=Top middle|"
          "<b>TR</b>=Top right|<br>"
          "<b>L</b>=Left|<b>R</b>=Right|<br>"
          "<b>BL</b>=Bottom left|<b>B</b>=Bottom middle|"
          "<b>BR</b>=Bottom right]", origin )
    },
    { QgsCallout::OriginX, QgsPropertyDefinition( "OriginX", QObject::tr( "Callout origin (X)" ), QgsPropertyDefinition::Double, origin ) },
    { QgsCallout::OriginY, QgsPropertyDefinition( "OriginY", QObject::tr( "Callout origin (Y)" ), QgsPropertyDefinition::Double, origin ) },
    { QgsCallout::DestinationX, QgsPropertyDefinition( "DestinationX", QObject::tr( "Callout destination (X)" ), QgsPropertyDefinition::Double, origin ) },
    { QgsCallout::DestinationY, QgsPropertyDefinition( "DestinationY", QObject::tr( "Callout destination (Y)" ), QgsPropertyDefinition::Double, origin ) },
    { QgsCallout::Curvature, QgsPropertyDefinition( "Curvature", QObject::tr( "Callout line curvature" ), QgsPropertyDefinition::Double, origin ) },
    {
      QgsCallout::Orientation, QgsPropertyDefinition( "Orientation", QgsPropertyDefinition::DataTypeString, QObject::tr( "Callout curve orientation" ),  QObject::tr( "string " ) + "[<b>auto</b>|<b>clockwise</b>|<b>counterclockwise</b>]", origin )
    },
    {
      QgsCallout::Margins, QgsPropertyDefinition( "Margins", QgsPropertyDefinition::DataTypeString, QObject::tr( "Margins" ), QObject::tr( "string of four doubles '<b>top,right,bottom,left</b>' or array of doubles <b>[top, right, bottom, left]</b>" ) )
    },
    { QgsCallout::WedgeWidth, QgsPropertyDefinition( "WedgeWidth", QObject::tr( "Wedge width" ), QgsPropertyDefinition::DoublePositive, origin ) },
    { QgsCallout::CornerRadius, QgsPropertyDefinition( "CornerRadius", QObject::tr( "Corner radius" ), QgsPropertyDefinition::DoublePositive, origin ) },
    { QgsCallout::BlendMode, QgsPropertyDefinition( "BlendMode", QObject::tr( "Callout blend mode" ), QgsPropertyDefinition::BlendMode, origin ) },
  };
}


QgsCallout::QgsCallout()
{
}

QVariantMap QgsCallout::properties( const QgsReadWriteContext & ) const
{
  QVariantMap props;
  props.insert( QStringLiteral( "enabled" ), mEnabled ? "1" : "0" );
  props.insert( QStringLiteral( "anchorPoint" ), encodeAnchorPoint( mAnchorPoint ) );
  props.insert( QStringLiteral( "labelAnchorPoint" ), encodeLabelAnchorPoint( mLabelAnchorPoint ) );
  props.insert( QStringLiteral( "blendMode" ), QgsPainting::getBlendModeEnum( mBlendMode ) );
  props.insert( QStringLiteral( "ddProperties" ), mDataDefinedProperties.toVariant( propertyDefinitions() ) );
  return props;
}

void QgsCallout::readProperties( const QVariantMap &props, const QgsReadWriteContext & )
{
  mEnabled = props.value( QStringLiteral( "enabled" ), QStringLiteral( "0" ) ).toInt();
  mAnchorPoint = decodeAnchorPoint( props.value( QStringLiteral( "anchorPoint" ), QString() ).toString() );
  mLabelAnchorPoint = decodeLabelAnchorPoint( props.value( QStringLiteral( "labelAnchorPoint" ), QString() ).toString() );
  mBlendMode = QgsPainting::getCompositionMode(
                 static_cast< QgsPainting::BlendMode >( props.value( QStringLiteral( "blendMode" ), QString::number( QgsPainting::BlendNormal ) ).toUInt() ) );
  mDataDefinedProperties.loadVariant( props.value( QStringLiteral( "ddProperties" ) ), propertyDefinitions() );
}

bool QgsCallout::saveProperties( QDomDocument &doc, QDomElement &element, const QgsReadWriteContext &context ) const
{
  if ( element.isNull() )
  {
    return false;
  }

  const QDomElement calloutPropsElement = QgsXmlUtils::writeVariant( properties( context ), doc );

  QDomElement calloutElement = doc.createElement( QStringLiteral( "callout" ) );
  calloutElement.setAttribute( QStringLiteral( "type" ), type() );
  calloutElement.appendChild( calloutPropsElement );

  element.appendChild( calloutElement );
  return true;
}

void QgsCallout::restoreProperties( const QDomElement &element, const QgsReadWriteContext &context )
{
  const QVariantMap props = QgsXmlUtils::readVariant( element.firstChildElement() ).toMap();
  readProperties( props, context );
}

void QgsCallout::startRender( QgsRenderContext & )
{

}
void QgsCallout::stopRender( QgsRenderContext & )
{

}

bool QgsCallout::containsAdvancedEffects() const
{
  return mBlendMode != QPainter::CompositionMode_SourceOver || dataDefinedProperties().isActive( QgsCallout::BlendMode );
}

QSet<QString> QgsCallout::referencedFields( const QgsRenderContext &context ) const
{
  mDataDefinedProperties.prepare( context.expressionContext() );
  return mDataDefinedProperties.referencedFields( context.expressionContext() );
}

QgsCallout::DrawOrder QgsCallout::drawOrder() const
{
  return OrderBelowAllLabels;
}

void QgsCallout::render( QgsRenderContext &context, const QRectF &rect, const double angle, const QgsGeometry &anchor, QgsCalloutContext &calloutContext )
{
  QPainter *painter = context.painter();
  if ( context.useAdvancedEffects() )
  {

    const QPainter::CompositionMode blendMode = mBlendMode;
    if ( dataDefinedProperties().isActive( QgsCallout::BlendMode ) )
    {
      context.expressionContext().setOriginalValueVariable( QString() );
      mBlendMode = QgsSymbolLayerUtils::decodeBlendMode( dataDefinedProperties().valueAsString( QgsCallout::BlendMode, context.expressionContext(), QString() ) );
    }

    painter->setCompositionMode( blendMode );
  }

#if 0 // for debugging
  painter->save();
  painter->setRenderHint( QPainter::Antialiasing, false );
  painter->translate( rect.center() );
  painter->rotate( -angle );

  painter->setBrush( QColor( 255, 0, 0, 100 ) );
  painter->setPen( QColor( 255, 0, 0, 150 ) );

  painter->drawRect( rect.width() * -0.5, rect.height() * -0.5, rect.width(), rect.height() );
  painter->restore();

  painter->setBrush( QColor( 0, 255, 0, 100 ) );
  painter->setPen( QColor( 0, 255, 0, 150 ) );

  painter->drawRect( anchor.boundingBox( ).buffered( 30 ).toRectF() );
#endif

  draw( context, rect, angle, anchor, calloutContext );

  painter->setCompositionMode( QPainter::CompositionMode_SourceOver ); // just to be sure
}

void QgsCallout::setEnabled( bool enabled )
{
  mEnabled = enabled;
}

QgsPropertiesDefinition QgsCallout::propertyDefinitions()
{
  static std::once_flag initialized;
  std::call_once( initialized, [ = ]( )
  {
    initPropertyDefinitions();
  } );
  return sPropertyDefinitions;
}

QgsCallout::AnchorPoint QgsCallout::decodeAnchorPoint( const QString &name, bool *ok )
{
  if ( ok )
    *ok = true;
  const QString cleaned = name.toLower().trimmed();

  if ( cleaned == QLatin1String( "pole_of_inaccessibility" ) )
    return PoleOfInaccessibility;
  else if ( cleaned == QLatin1String( "point_on_exterior" ) )
    return PointOnExterior;
  else if ( cleaned == QLatin1String( "point_on_surface" ) )
    return PointOnSurface;
  else if ( cleaned == QLatin1String( "centroid" ) )
    return Centroid;

  if ( ok )
    *ok = false;
  return PoleOfInaccessibility;
}

QString QgsCallout::encodeAnchorPoint( AnchorPoint anchor )
{
  switch ( anchor )
  {
    case PoleOfInaccessibility:
      return QStringLiteral( "pole_of_inaccessibility" );
    case PointOnExterior:
      return QStringLiteral( "point_on_exterior" );
    case PointOnSurface:
      return QStringLiteral( "point_on_surface" );
    case Centroid:
      return QStringLiteral( "centroid" );
  }
  return QString();
}

QString QgsCallout::encodeLabelAnchorPoint( QgsCallout::LabelAnchorPoint anchor )
{
  switch ( anchor )
  {
    case LabelPointOnExterior:
      return QStringLiteral( "point_on_exterior" );
    case LabelCentroid:
      return QStringLiteral( "centroid" );
    case LabelTopLeft:
      return QStringLiteral( "tl" );
    case LabelTopMiddle:
      return QStringLiteral( "t" );
    case LabelTopRight:
      return QStringLiteral( "tr" );
    case LabelMiddleLeft:
      return QStringLiteral( "l" );
    case LabelMiddleRight:
      return QStringLiteral( "r" );
    case LabelBottomLeft:
      return QStringLiteral( "bl" );
    case LabelBottomMiddle:
      return QStringLiteral( "b" );
    case LabelBottomRight:
      return QStringLiteral( "br" );
  }

  return QString();
}

QgsCallout::LabelAnchorPoint QgsCallout::decodeLabelAnchorPoint( const QString &name, bool *ok )
{
  if ( ok )
    *ok = true;
  const QString cleaned = name.toLower().trimmed();

  if ( cleaned == QLatin1String( "point_on_exterior" ) )
    return LabelPointOnExterior;
  else if ( cleaned == QLatin1String( "centroid" ) )
    return LabelCentroid;
  else if ( cleaned == QLatin1String( "tl" ) )
    return LabelTopLeft;
  else if ( cleaned == QLatin1String( "t" ) )
    return LabelTopMiddle;
  else if ( cleaned == QLatin1String( "tr" ) )
    return LabelTopRight;
  else if ( cleaned == QLatin1String( "l" ) )
    return LabelMiddleLeft;
  else if ( cleaned == QLatin1String( "r" ) )
    return LabelMiddleRight;
  else if ( cleaned == QLatin1String( "bl" ) )
    return LabelBottomLeft;
  else if ( cleaned == QLatin1String( "b" ) )
    return LabelBottomMiddle;
  else if ( cleaned == QLatin1String( "br" ) )
    return LabelBottomRight;

  if ( ok )
    *ok = false;
  return LabelPointOnExterior;
}

QgsGeometry QgsCallout::labelAnchorGeometry( const QRectF &rect, const double angle, LabelAnchorPoint anchor ) const
{
  QgsGeometry label;
  switch ( anchor )
  {
    case LabelPointOnExterior:
      label = QgsGeometry::fromRect( rect );
      break;

    case LabelCentroid:
      label = QgsGeometry::fromRect( rect ).centroid();
      break;

    case LabelTopLeft:
      label = QgsGeometry::fromPointXY( QgsPointXY( rect.bottomLeft() ) );
      break;

    case LabelTopMiddle:
      label = QgsGeometry::fromPointXY( QgsPointXY( ( rect.left() + rect.right() ) / 2.0, rect.bottom() ) );
      break;

    case LabelTopRight:
      label = QgsGeometry::fromPointXY( QgsPointXY( rect.bottomRight() ) );
      break;

    case LabelMiddleLeft:
      label = QgsGeometry::fromPointXY( QgsPointXY( rect.left(), ( rect.top() + rect.bottom() ) / 2.0 ) );
      break;

    case LabelMiddleRight:
      label = QgsGeometry::fromPointXY( QgsPointXY( rect.right(), ( rect.top() + rect.bottom() ) / 2.0 ) );
      break;

    case LabelBottomLeft:
      label = QgsGeometry::fromPointXY( QgsPointXY( rect.topLeft() ) );
      break;

    case LabelBottomMiddle:
      label = QgsGeometry::fromPointXY( QgsPointXY( ( rect.left() + rect.right() ) / 2.0, rect.top() ) );
      break;

    case LabelBottomRight:
      label = QgsGeometry::fromPointXY( QgsPointXY( rect.topRight() ) );
      break;
  }

  label.rotate( angle, rect.topLeft() );
  return label;
}

QgsGeometry QgsCallout::calloutLabelPoint( const QRectF &rect, const double angle, QgsCallout::LabelAnchorPoint anchor, QgsRenderContext &context, const QgsCallout::QgsCalloutContext &calloutContext, bool &pinned ) const
{
  pinned = false;
  if ( dataDefinedProperties().isActive( QgsCallout::OriginX ) && dataDefinedProperties().isActive( QgsCallout::OriginY ) )
  {
    bool ok = false;
    const double x = dataDefinedProperties().valueAsDouble( QgsCallout::OriginX, context.expressionContext(), 0, &ok );
    if ( ok )
    {
      const double y = dataDefinedProperties().valueAsDouble( QgsCallout::OriginY, context.expressionContext(), 0, &ok );
      if ( ok )
      {
        pinned = true;
        // data defined label point, use it directly
        QgsGeometry labelPoint = QgsGeometry::fromPointXY( QgsPointXY( x, y ) );
        try
        {
          labelPoint.transform( calloutContext.originalFeatureToMapTransform( context ) );
          labelPoint.transform( context.mapToPixel().transform() );
        }
        catch ( QgsCsException & )
        {
          return QgsGeometry();
        }
        return labelPoint;
      }
    }
  }

  QgsGeometry label;
  switch ( anchor )
  {
    case LabelPointOnExterior:
      label = QgsGeometry::fromRect( rect );
      break;

    case LabelCentroid:
      label = QgsGeometry::fromRect( rect ).centroid();
      break;

    case LabelTopLeft:
      label = QgsGeometry::fromPointXY( QgsPointXY( rect.bottomLeft() ) );
      break;

    case LabelTopMiddle:
      label = QgsGeometry::fromPointXY( QgsPointXY( ( rect.left() + rect.right() ) / 2.0, rect.bottom() ) );
      break;

    case LabelTopRight:
      label = QgsGeometry::fromPointXY( QgsPointXY( rect.bottomRight() ) );
      break;

    case LabelMiddleLeft:
      label = QgsGeometry::fromPointXY( QgsPointXY( rect.left(), ( rect.top() + rect.bottom() ) / 2.0 ) );
      break;

    case LabelMiddleRight:
      label = QgsGeometry::fromPointXY( QgsPointXY( rect.right(), ( rect.top() + rect.bottom() ) / 2.0 ) );
      break;

    case LabelBottomLeft:
      label = QgsGeometry::fromPointXY( QgsPointXY( rect.topLeft() ) );
      break;

    case LabelBottomMiddle:
      label = QgsGeometry::fromPointXY( QgsPointXY( ( rect.left() + rect.right() ) / 2.0, rect.top() ) );
      break;

    case LabelBottomRight:
      label = QgsGeometry::fromPointXY( QgsPointXY( rect.topRight() ) );
      break;
  }

  label.rotate( angle, rect.topLeft() );
  return label;
}

QgsGeometry QgsCallout::calloutLineToPart( const QgsGeometry &labelGeometry, const QgsAbstractGeometry *partGeometry, QgsRenderContext &context, const QgsCalloutContext &calloutContext, bool &pinned ) const
{
  pinned = false;
  AnchorPoint anchor = anchorPoint();
  const QgsAbstractGeometry *evaluatedPartAnchor = partGeometry;
  std::unique_ptr< QgsAbstractGeometry > tempPartAnchor;

  if ( dataDefinedProperties().isActive( QgsCallout::DestinationX ) && dataDefinedProperties().isActive( QgsCallout::DestinationY ) )
  {
    bool ok = false;
    const double x = dataDefinedProperties().valueAsDouble( QgsCallout::DestinationX, context.expressionContext(), 0, &ok );
    if ( ok )
    {
      const double y = dataDefinedProperties().valueAsDouble( QgsCallout::DestinationY, context.expressionContext(), 0, &ok );
      if ( ok )
      {
        pinned = true;
        tempPartAnchor = std::make_unique< QgsPoint >( QgsWkbTypes::Point, x, y );
        evaluatedPartAnchor = tempPartAnchor.get();
        try
        {
          tempPartAnchor->transform( calloutContext.originalFeatureToMapTransform( context ) );
          tempPartAnchor->transform( context.mapToPixel().transform() );
        }
        catch ( QgsCsException & )
        {
          evaluatedPartAnchor = partGeometry;
        }
      }
    }
  }

  if ( dataDefinedProperties().isActive( QgsCallout::AnchorPointPosition ) )
  {
    const QString encodedAnchor = encodeAnchorPoint( anchor );
    context.expressionContext().setOriginalValueVariable( encodedAnchor );
    anchor = decodeAnchorPoint( dataDefinedProperties().valueAsString( QgsCallout::AnchorPointPosition, context.expressionContext(), encodedAnchor ) );
  }

  QgsGeometry line;
  const QgsGeos labelGeos( labelGeometry.constGet() );

  switch ( QgsWkbTypes::geometryType( evaluatedPartAnchor->wkbType() ) )
  {
    case QgsWkbTypes::PointGeometry:
    case QgsWkbTypes::LineGeometry:
    {
      line = labelGeos.shortestLine( evaluatedPartAnchor );
      break;
    }

    case QgsWkbTypes::PolygonGeometry:
    {
      if ( labelGeos.intersects( evaluatedPartAnchor ) )
        return QgsGeometry();

      // ideally avoid this unwanted clone in future. For now we need it because poleOfInaccessibility/pointOnSurface are
      // only available to QgsGeometry objects
      const QgsGeometry evaluatedPartAnchorGeom( evaluatedPartAnchor->clone() );
      switch ( anchor )
      {
        case QgsCallout::PoleOfInaccessibility:
          line = labelGeos.shortestLine( evaluatedPartAnchorGeom.poleOfInaccessibility( std::max( evaluatedPartAnchor->boundingBox().width(), evaluatedPartAnchor->boundingBox().height() ) / 20.0 ) ); // really rough (but quick) pole of inaccessibility
          break;
        case QgsCallout::PointOnSurface:
          line = labelGeos.shortestLine( evaluatedPartAnchorGeom.pointOnSurface() );
          break;
        case QgsCallout::PointOnExterior:
          line = labelGeos.shortestLine( evaluatedPartAnchor );
          break;
        case QgsCallout::Centroid:
          line = labelGeos.shortestLine( evaluatedPartAnchorGeom.centroid() );
          break;
      }
      break;
    }

    case QgsWkbTypes::NullGeometry:
    case QgsWkbTypes::UnknownGeometry:
      return QgsGeometry(); // shouldn't even get here..
  }
  return line;
}

//
// QgsCallout::QgsCalloutContext
//

QgsCoordinateTransform QgsCallout::QgsCalloutContext::originalFeatureToMapTransform( const QgsRenderContext &renderContext ) const
{
  if ( !mOriginalFeatureToMapTransform.isValid() )
  {
    // lazy initialization, only create if needed...
    mOriginalFeatureToMapTransform = QgsCoordinateTransform( originalFeatureCrs, renderContext.coordinateTransform().destinationCrs(), renderContext.transformContext() );
  }
  return mOriginalFeatureToMapTransform;
}


//
// QgsSimpleLineCallout
//

QgsSimpleLineCallout::QgsSimpleLineCallout()
{
  mLineSymbol = std::make_unique< QgsLineSymbol >( QgsSymbolLayerList() << new QgsSimpleLineSymbolLayer( QColor( 60, 60, 60 ), .3 ) );

}

QgsSimpleLineCallout::~QgsSimpleLineCallout() = default;

QgsSimpleLineCallout::QgsSimpleLineCallout( const QgsSimpleLineCallout &other )
  : QgsCallout( other )
  , mLineSymbol( other.mLineSymbol ? other.mLineSymbol->clone() : nullptr )
  , mMinCalloutLength( other.mMinCalloutLength )
  , mMinCalloutLengthUnit( other.mMinCalloutLengthUnit )
  , mMinCalloutLengthScale( other.mMinCalloutLengthScale )
  , mOffsetFromAnchorDistance( other.mOffsetFromAnchorDistance )
  , mOffsetFromAnchorUnit( other.mOffsetFromAnchorUnit )
  , mOffsetFromAnchorScale( other.mOffsetFromAnchorScale )
  , mOffsetFromLabelDistance( other.mOffsetFromLabelDistance )
  , mOffsetFromLabelUnit( other.mOffsetFromLabelUnit )
  , mOffsetFromLabelScale( other.mOffsetFromLabelScale )
  , mDrawCalloutToAllParts( other.mDrawCalloutToAllParts )
{

}

QgsCallout *QgsSimpleLineCallout::create( const QVariantMap &properties, const QgsReadWriteContext &context )
{
  std::unique_ptr< QgsSimpleLineCallout > callout = std::make_unique< QgsSimpleLineCallout >();
  callout->readProperties( properties, context );
  return callout.release();
}

QString QgsSimpleLineCallout::type() const
{
  return QStringLiteral( "simple" );
}

QgsSimpleLineCallout *QgsSimpleLineCallout::clone() const
{
  return new QgsSimpleLineCallout( *this );
}

QVariantMap QgsSimpleLineCallout::properties( const QgsReadWriteContext &context ) const
{
  QVariantMap props = QgsCallout::properties( context );

  if ( mLineSymbol )
  {
    props[ QStringLiteral( "lineSymbol" ) ] = QgsSymbolLayerUtils::symbolProperties( mLineSymbol.get() );
  }
  props[ QStringLiteral( "minLength" ) ] = mMinCalloutLength;
  props[ QStringLiteral( "minLengthUnit" ) ] = QgsUnitTypes::encodeUnit( mMinCalloutLengthUnit );
  props[ QStringLiteral( "minLengthMapUnitScale" ) ] = QgsSymbolLayerUtils::encodeMapUnitScale( mMinCalloutLengthScale );

  props[ QStringLiteral( "offsetFromAnchor" ) ] = mOffsetFromAnchorDistance;
  props[ QStringLiteral( "offsetFromAnchorUnit" ) ] = QgsUnitTypes::encodeUnit( mOffsetFromAnchorUnit );
  props[ QStringLiteral( "offsetFromAnchorMapUnitScale" ) ] = QgsSymbolLayerUtils::encodeMapUnitScale( mOffsetFromAnchorScale );
  props[ QStringLiteral( "offsetFromLabel" ) ] = mOffsetFromLabelDistance;
  props[ QStringLiteral( "offsetFromLabelUnit" ) ] = QgsUnitTypes::encodeUnit( mOffsetFromLabelUnit );
  props[ QStringLiteral( "offsetFromLabelMapUnitScale" ) ] = QgsSymbolLayerUtils::encodeMapUnitScale( mOffsetFromLabelScale );

  props[ QStringLiteral( "drawToAllParts" ) ] = mDrawCalloutToAllParts;

  return props;
}

void QgsSimpleLineCallout::readProperties( const QVariantMap &props, const QgsReadWriteContext &context )
{
  QgsCallout::readProperties( props, context );

  const QString lineSymbolDef = props.value( QStringLiteral( "lineSymbol" ) ).toString();
  QDomDocument doc( QStringLiteral( "symbol" ) );
  doc.setContent( lineSymbolDef );
  const QDomElement symbolElem = doc.firstChildElement( QStringLiteral( "symbol" ) );
  std::unique_ptr< QgsLineSymbol > lineSymbol( QgsSymbolLayerUtils::loadSymbol< QgsLineSymbol >( symbolElem, context ) );
  if ( lineSymbol )
    mLineSymbol = std::move( lineSymbol );

  mMinCalloutLength = props.value( QStringLiteral( "minLength" ), 0 ).toDouble();
  mMinCalloutLengthUnit = QgsUnitTypes::decodeRenderUnit( props.value( QStringLiteral( "minLengthUnit" ) ).toString() );
  mMinCalloutLengthScale = QgsSymbolLayerUtils::decodeMapUnitScale( props.value( QStringLiteral( "minLengthMapUnitScale" ) ).toString() );

  mOffsetFromAnchorDistance = props.value( QStringLiteral( "offsetFromAnchor" ), 0 ).toDouble();
  mOffsetFromAnchorUnit = QgsUnitTypes::decodeRenderUnit( props.value( QStringLiteral( "offsetFromAnchorUnit" ) ).toString() );
  mOffsetFromAnchorScale = QgsSymbolLayerUtils::decodeMapUnitScale( props.value( QStringLiteral( "offsetFromAnchorMapUnitScale" ) ).toString() );
  mOffsetFromLabelDistance = props.value( QStringLiteral( "offsetFromLabel" ), 0 ).toDouble();
  mOffsetFromLabelUnit = QgsUnitTypes::decodeRenderUnit( props.value( QStringLiteral( "offsetFromLabelUnit" ) ).toString() );
  mOffsetFromLabelScale = QgsSymbolLayerUtils::decodeMapUnitScale( props.value( QStringLiteral( "offsetFromLabelMapUnitScale" ) ).toString() );

  mDrawCalloutToAllParts = props.value( QStringLiteral( "drawToAllParts" ), false ).toBool();
}

void QgsSimpleLineCallout::startRender( QgsRenderContext &context )
{
  QgsCallout::startRender( context );
  if ( mLineSymbol )
    mLineSymbol->startRender( context );
}

void QgsSimpleLineCallout::stopRender( QgsRenderContext &context )
{
  QgsCallout::stopRender( context );
  if ( mLineSymbol )
    mLineSymbol->stopRender( context );
}

QSet<QString> QgsSimpleLineCallout::referencedFields( const QgsRenderContext &context ) const
{
  QSet<QString> fields = QgsCallout::referencedFields( context );
  if ( mLineSymbol )
    fields.unite( mLineSymbol->usedAttributes( context ) );
  return fields;
}

QgsLineSymbol *QgsSimpleLineCallout::lineSymbol()
{
  return mLineSymbol.get();
}

void QgsSimpleLineCallout::setLineSymbol( QgsLineSymbol *symbol )
{
  mLineSymbol.reset( symbol );
}

void QgsSimpleLineCallout::draw( QgsRenderContext &context, const QRectF &rect, const double angle, const QgsGeometry &anchor, QgsCalloutContext &calloutContext )
{
  LabelAnchorPoint labelAnchor = labelAnchorPoint();
  if ( dataDefinedProperties().isActive( QgsCallout::LabelAnchorPointPosition ) )
  {
    const QString encodedAnchor = encodeLabelAnchorPoint( labelAnchor );
    context.expressionContext().setOriginalValueVariable( encodedAnchor );
    labelAnchor = decodeLabelAnchorPoint( dataDefinedProperties().valueAsString( QgsCallout::LabelAnchorPointPosition, context.expressionContext(), encodedAnchor ) );
  }

  bool originPinned = false;
  const QgsGeometry label = calloutLabelPoint( rect, angle, labelAnchor, context, calloutContext, originPinned );
  if ( label.isNull() )
    return;

  auto drawCalloutLine = [this, &context, &calloutContext, &label, &rect, angle, &anchor, originPinned]( const QgsAbstractGeometry * partAnchor )
  {
    bool destinationPinned = false;
    const QgsGeometry line = calloutLineToPart( label, partAnchor, context, calloutContext, destinationPinned );
    if ( line.isEmpty() )
      return;

    const double lineLength = line.length();
    if ( qgsDoubleNear( lineLength, 0 ) )
      return;

    double minLength = mMinCalloutLength;
    if ( dataDefinedProperties().isActive( QgsCallout::MinimumCalloutLength ) )
    {
      context.expressionContext().setOriginalValueVariable( minLength );
      minLength = dataDefinedProperties().valueAsDouble( QgsCallout::MinimumCalloutLength, context.expressionContext(), minLength );
    }
    const double minLengthPixels = context.convertToPainterUnits( minLength, mMinCalloutLengthUnit, mMinCalloutLengthScale );
    if ( minLengthPixels > 0 && lineLength < minLengthPixels )
      return; // too small!

    std::unique_ptr< QgsCurve > calloutCurve( createCalloutLine( qgsgeometry_cast< const QgsLineString * >( line.constGet() )->startPoint(),
        qgsgeometry_cast< const QgsLineString * >( line.constGet() )->endPoint(), context, rect, angle, anchor, calloutContext ) );

    double offsetFromAnchor = mOffsetFromAnchorDistance;
    if ( dataDefinedProperties().isActive( QgsCallout::OffsetFromAnchor ) )
    {
      context.expressionContext().setOriginalValueVariable( offsetFromAnchor );
      offsetFromAnchor = dataDefinedProperties().valueAsDouble( QgsCallout::OffsetFromAnchor, context.expressionContext(), offsetFromAnchor );
    }
    const double offsetFromAnchorPixels = context.convertToPainterUnits( offsetFromAnchor, mOffsetFromAnchorUnit, mOffsetFromAnchorScale );

    double offsetFromLabel = mOffsetFromLabelDistance;
    if ( dataDefinedProperties().isActive( QgsCallout::OffsetFromLabel ) )
    {
      context.expressionContext().setOriginalValueVariable( offsetFromLabel );
      offsetFromLabel = dataDefinedProperties().valueAsDouble( QgsCallout::OffsetFromLabel, context.expressionContext(), offsetFromLabel );
    }
    const double offsetFromLabelPixels = context.convertToPainterUnits( offsetFromLabel, mOffsetFromLabelUnit, mOffsetFromLabelScale );
    if ( offsetFromAnchorPixels > 0 || offsetFromLabelPixels > 0 )
    {
      calloutCurve.reset( calloutCurve->curveSubstring( offsetFromLabelPixels, calloutCurve->length() - offsetFromAnchorPixels ) );
    }

    const QPolygonF points = calloutCurve->asQPolygonF();

    if ( points.empty() )
      return;

    QgsCalloutPosition position;
    position.setOrigin( context.mapToPixel().toMapCoordinates( points.at( 0 ).x(), points.at( 0 ).y() ).toQPointF() );
    position.setOriginIsPinned( originPinned );
    position.setDestination( context.mapToPixel().toMapCoordinates( points.constLast().x(), points.constLast().y() ).toQPointF() );
    position.setDestinationIsPinned( destinationPinned );
    calloutContext.addCalloutPosition( position );

    mLineSymbol->renderPolyline( points, nullptr, context );
  };

  bool toAllParts = mDrawCalloutToAllParts;
  if ( dataDefinedProperties().isActive( QgsCallout::DrawCalloutToAllParts ) )
  {
    context.expressionContext().setOriginalValueVariable( toAllParts );
    toAllParts = dataDefinedProperties().valueAsBool( QgsCallout::DrawCalloutToAllParts, context.expressionContext(), toAllParts );
  }

  if ( calloutContext.allFeaturePartsLabeled || !toAllParts )
    drawCalloutLine( anchor.constGet() );
  else
  {
    for ( auto it = anchor.const_parts_begin(); it != anchor.const_parts_end(); ++it )
      drawCalloutLine( *it );
  }
}

QgsCurve *QgsSimpleLineCallout::createCalloutLine( const QgsPoint &start, const QgsPoint &end, QgsRenderContext &, const QRectF &, const double, const QgsGeometry &, QgsCallout::QgsCalloutContext & ) const
{
  return new QgsLineString( start, end );
}

//
// QgsManhattanLineCallout
//

QgsManhattanLineCallout::QgsManhattanLineCallout()
{
}

QgsManhattanLineCallout::QgsManhattanLineCallout( const QgsManhattanLineCallout &other )
  : QgsSimpleLineCallout( other )
{

}


QgsCallout *QgsManhattanLineCallout::create( const QVariantMap &properties, const QgsReadWriteContext &context )
{
  std::unique_ptr< QgsManhattanLineCallout > callout = std::make_unique< QgsManhattanLineCallout >();
  callout->readProperties( properties, context );
  return callout.release();
}

QString QgsManhattanLineCallout::type() const
{
  return QStringLiteral( "manhattan" );
}

QgsManhattanLineCallout *QgsManhattanLineCallout::clone() const
{
  return new QgsManhattanLineCallout( *this );
}

QgsCurve *QgsManhattanLineCallout::createCalloutLine( const QgsPoint &start, const QgsPoint &end, QgsRenderContext &, const QRectF &, const double, const QgsGeometry &, QgsCallout::QgsCalloutContext & ) const
{
  const QgsPoint mid1 = QgsPoint( start.x(), end.y() );
  return new QgsLineString( QVector< QgsPoint >() << start << mid1 << end );
}


//
// QgsCurvedLineCallout
//

QgsCurvedLineCallout::QgsCurvedLineCallout()
{
}

QgsCurvedLineCallout::QgsCurvedLineCallout( const QgsCurvedLineCallout &other )
  : QgsSimpleLineCallout( other )
  , mOrientation( other.mOrientation )
  , mCurvature( other.mCurvature )
{

}

QgsCallout *QgsCurvedLineCallout::create( const QVariantMap &properties, const QgsReadWriteContext &context )
{
  std::unique_ptr< QgsCurvedLineCallout > callout = std::make_unique< QgsCurvedLineCallout >();
  callout->readProperties( properties, context );

  callout->setCurvature( properties.value( QStringLiteral( "curvature" ), 0.1 ).toDouble() );
  callout->setOrientation( decodeOrientation( properties.value( QStringLiteral( "orientation" ), QStringLiteral( "auto" ) ).toString() ) );

  return callout.release();
}

QString QgsCurvedLineCallout::type() const
{
  return QStringLiteral( "curved" );
}

QgsCurvedLineCallout *QgsCurvedLineCallout::clone() const
{
  return new QgsCurvedLineCallout( *this );
}

QVariantMap QgsCurvedLineCallout::properties( const QgsReadWriteContext &context ) const
{
  QVariantMap props = QgsSimpleLineCallout::properties( context );
  props.insert( QStringLiteral( "curvature" ), mCurvature );
  props.insert( QStringLiteral( "orientation" ), encodeOrientation( mOrientation ) );
  return props;
}

QgsCurve *QgsCurvedLineCallout::createCalloutLine( const QgsPoint &start, const QgsPoint &end, QgsRenderContext &context, const QRectF &rect, const double, const QgsGeometry &, QgsCallout::QgsCalloutContext & ) const
{
  double curvature = mCurvature * 100;
  if ( dataDefinedProperties().isActive( QgsCallout::Curvature ) )
  {
    context.expressionContext().setOriginalValueVariable( curvature );
    curvature = dataDefinedProperties().valueAsDouble( QgsCallout::Curvature, context.expressionContext(), curvature );
  }

  Orientation orientation = mOrientation;
  if ( dataDefinedProperties().isActive( QgsCallout::Orientation ) )
  {
    bool ok = false;
    const QString orientationString = dataDefinedProperties().property( QgsCallout::Orientation ).valueAsString( context.expressionContext(), QString(), &ok );
    if ( ok )
    {
      orientation = decodeOrientation( orientationString );
    }
  }

  if ( orientation == Automatic )
  {
    // to calculate automatically the best curve orientation, we first check which side of the label bounding box
    // the callout origin is nearest to
    switch ( QgsGeometryUtils::closestSideOfRectangle( rect.right(), rect.bottom(), rect.left(), rect.top(), start.x(), start.y() ) )
    {
      case 1:
        // closest to bottom
        if ( qgsDoubleNear( end.x(), start.x() ) )
        {
          // if vertical line, we bend depending on whether the line sits towards the left or right side of the label
          if ( start.x() < ( rect.left() + 0.5 * rect.width() ) )
            orientation = CounterClockwise;
          else
            orientation = Clockwise;
        }
        else if ( end.x() > start.x() )
          orientation = CounterClockwise;
        else
          orientation = Clockwise;
        break;

      case 2:
        // closest to bottom-right
        if ( end.x() < start.x() )
          orientation = Clockwise;
        else if ( end.y() < start.y() )
          orientation = CounterClockwise;
        else if ( end.x() - start.x() < end.y() - start.y() )
          orientation = Clockwise;
        else
          orientation = CounterClockwise;
        break;

      case 3:
        // closest to right
        if ( qgsDoubleNear( end.y(), start.y() ) )
        {
          // if horizontal line, we bend depending on whether the line sits towards the top or bottom side of the label
          if ( start.y() < ( rect.top() + 0.5 * rect.height() ) )
            orientation = Clockwise;
          else
            orientation = CounterClockwise;
        }
        else if ( end.y() < start.y() )
          orientation = CounterClockwise;
        else
          orientation = Clockwise;
        break;

      case 4:
        // closest to top-right
        if ( end.x() < start.x() )
          orientation = CounterClockwise;
        else if ( end.y() > start.y() )
          orientation = Clockwise;
        else if ( end.x() - start.x() < start.y() - end.y() )
          orientation = CounterClockwise;
        else
          orientation = Clockwise;
        break;

      case 5:
        // closest to top
        if ( qgsDoubleNear( end.x(), start.x() ) )
        {
          // if vertical line, we bend depending on whether the line sits towards the left or right side of the label
          if ( start.x() < ( rect.left() + 0.5 * rect.width() ) )
            orientation = Clockwise;
          else
            orientation = CounterClockwise;
        }
        else if ( end.x() < start.x() )
          orientation = CounterClockwise;
        else
          orientation = Clockwise;
        break;

      case 6:
        // closest to top-left
        if ( end.x() > start.x() )
          orientation = Clockwise;
        else if ( end.y() > start.y() )
          orientation = CounterClockwise;
        else if ( start.x() - end.x() < start.y() - end.y() )
          orientation = Clockwise;
        else
          orientation = CounterClockwise;
        break;

      case 7:
        //closest to left
        if ( qgsDoubleNear( end.y(), start.y() ) )
        {
          // if horizontal line, we bend depending on whether the line sits towards the top or bottom side of the label
          if ( start.y() < ( rect.top() + 0.5 * rect.height() ) )
            orientation = CounterClockwise;
          else
            orientation = Clockwise;
        }
        else if ( end.y() > start.y() )
          orientation = CounterClockwise;
        else
          orientation = Clockwise;
        break;

      case 8:
        //closest to bottom-left
        if ( end.x() > start.x() )
          orientation = CounterClockwise;
        else if ( end.y() < start.y() )
          orientation = Clockwise;
        else if ( start.x() - end.x() < end.y() - start.y() )
          orientation = CounterClockwise;
        else
          orientation = Clockwise;
        break;
    }
  }

  // turn the line into a curved line. We do this by creating a circular string from the callout line's
  // start to end point, where the curve point is in the middle of the callout line and perpendicularly offset
  // by a proportion of the overall callout line length
  const double distance = ( orientation == Clockwise ? 1 : -1 ) * start.distance( end ) * curvature / 100.0;
  double midX, midY;
  QgsGeometryUtils::perpendicularOffsetPointAlongSegment( start.x(), start.y(), end.x(), end.y(), 0.5, distance, &midX, &midY );

  return new QgsCircularString( start, QgsPoint( midX, midY ), end );
}

QgsCurvedLineCallout::Orientation QgsCurvedLineCallout::decodeOrientation( const QString &string )
{
  const QString cleaned = string.toLower().trimmed();
  if ( cleaned == QLatin1String( "auto" ) )
    return Automatic;
  if ( cleaned == QLatin1String( "clockwise" ) )
    return Clockwise;
  if ( cleaned == QLatin1String( "counterclockwise" ) )
    return CounterClockwise;
  return Automatic;
}

QString QgsCurvedLineCallout::encodeOrientation( QgsCurvedLineCallout::Orientation orientation )
{
  switch ( orientation )
  {
    case QgsCurvedLineCallout::Automatic:
      return QStringLiteral( "auto" );
    case QgsCurvedLineCallout::Clockwise:
      return QStringLiteral( "clockwise" );
    case QgsCurvedLineCallout::CounterClockwise:
      return QStringLiteral( "counterclockwise" );
  }
  return QString();
}

QgsCurvedLineCallout::Orientation QgsCurvedLineCallout::orientation() const
{
  return mOrientation;
}

void QgsCurvedLineCallout::setOrientation( Orientation orientation )
{
  mOrientation = orientation;
}

double QgsCurvedLineCallout::curvature() const
{
  return mCurvature;
}

void QgsCurvedLineCallout::setCurvature( double curvature )
{
  mCurvature = curvature;
}



//
// QgsBalloonCallout
//

QgsBalloonCallout::QgsBalloonCallout()
{
  mFillSymbol = std::make_unique< QgsFillSymbol >( QgsSymbolLayerList() << new QgsSimpleFillSymbolLayer( QColor( 255, 200, 60 ) ) );
}

QgsBalloonCallout::~QgsBalloonCallout() = default;

QgsBalloonCallout::QgsBalloonCallout( const QgsBalloonCallout &other )
  : QgsCallout( other )
  , mFillSymbol( other.mFillSymbol ? other.mFillSymbol->clone() : nullptr )
  , mOffsetFromAnchorDistance( other.mOffsetFromAnchorDistance )
  , mOffsetFromAnchorUnit( other.mOffsetFromAnchorUnit )
  , mOffsetFromAnchorScale( other.mOffsetFromAnchorScale )
  , mMargins( other.mMargins )
  , mMarginUnit( other.mMarginUnit )
  , mWedgeWidth( other.mWedgeWidth )
  , mWedgeWidthUnit( other.mWedgeWidthUnit )
  , mWedgeWidthScale( other.mWedgeWidthScale )
  , mCornerRadius( other.mCornerRadius )
  , mCornerRadiusUnit( other.mCornerRadiusUnit )
  , mCornerRadiusScale( other.mCornerRadiusScale )
{

}

QgsCallout *QgsBalloonCallout::create( const QVariantMap &properties, const QgsReadWriteContext &context )
{
  std::unique_ptr< QgsBalloonCallout > callout = std::make_unique< QgsBalloonCallout >();
  callout->readProperties( properties, context );
  return callout.release();
}

QString QgsBalloonCallout::type() const
{
  return QStringLiteral( "balloon" );
}

QgsBalloonCallout *QgsBalloonCallout::clone() const
{
  return new QgsBalloonCallout( *this );
}

QVariantMap QgsBalloonCallout::properties( const QgsReadWriteContext &context ) const
{
  QVariantMap props = QgsCallout::properties( context );

  if ( mFillSymbol )
  {
    props[ QStringLiteral( "fillSymbol" ) ] = QgsSymbolLayerUtils::symbolProperties( mFillSymbol.get() );
  }

  props[ QStringLiteral( "offsetFromAnchor" ) ] = mOffsetFromAnchorDistance;
  props[ QStringLiteral( "offsetFromAnchorUnit" ) ] = QgsUnitTypes::encodeUnit( mOffsetFromAnchorUnit );
  props[ QStringLiteral( "offsetFromAnchorMapUnitScale" ) ] = QgsSymbolLayerUtils::encodeMapUnitScale( mOffsetFromAnchorScale );

  props[ QStringLiteral( "margins" ) ] = mMargins.toString();
  props[ QStringLiteral( "marginsUnit" ) ] = QgsUnitTypes::encodeUnit( mMarginUnit );

  props[ QStringLiteral( "wedgeWidth" ) ] = mWedgeWidth;
  props[ QStringLiteral( "wedgeWidthUnit" ) ] = QgsUnitTypes::encodeUnit( mWedgeWidthUnit );
  props[ QStringLiteral( "wedgeWidthMapUnitScale" ) ] = QgsSymbolLayerUtils::encodeMapUnitScale( mWedgeWidthScale );

  props[ QStringLiteral( "cornerRadius" ) ] = mCornerRadius;
  props[ QStringLiteral( "cornerRadiusUnit" ) ] = QgsUnitTypes::encodeUnit( mCornerRadiusUnit );
  props[ QStringLiteral( "cornerRadiusMapUnitScale" ) ] = QgsSymbolLayerUtils::encodeMapUnitScale( mCornerRadiusScale );

  return props;
}

void QgsBalloonCallout::readProperties( const QVariantMap &props, const QgsReadWriteContext &context )
{
  QgsCallout::readProperties( props, context );

  const QString fillSymbolDef = props.value( QStringLiteral( "fillSymbol" ) ).toString();
  QDomDocument doc( QStringLiteral( "symbol" ) );
  doc.setContent( fillSymbolDef );
  const QDomElement symbolElem = doc.firstChildElement( QStringLiteral( "symbol" ) );
  std::unique_ptr< QgsFillSymbol > fillSymbol( QgsSymbolLayerUtils::loadSymbol< QgsFillSymbol >( symbolElem, context ) );
  if ( fillSymbol )
    mFillSymbol = std::move( fillSymbol );

  mOffsetFromAnchorDistance = props.value( QStringLiteral( "offsetFromAnchor" ), 0 ).toDouble();
  mOffsetFromAnchorUnit = QgsUnitTypes::decodeRenderUnit( props.value( QStringLiteral( "offsetFromAnchorUnit" ) ).toString() );
  mOffsetFromAnchorScale = QgsSymbolLayerUtils::decodeMapUnitScale( props.value( QStringLiteral( "offsetFromAnchorMapUnitScale" ) ).toString() );

  mMargins = QgsMargins::fromString( props.value( QStringLiteral( "margins" ) ).toString() );
  mMarginUnit = QgsUnitTypes::decodeRenderUnit( props.value( QStringLiteral( "marginsUnit" ) ).toString() );

  mWedgeWidth = props.value( QStringLiteral( "wedgeWidth" ), 2.64 ).toDouble();
  mWedgeWidthUnit = QgsUnitTypes::decodeRenderUnit( props.value( QStringLiteral( "wedgeWidthUnit" ) ).toString() );
  mWedgeWidthScale = QgsSymbolLayerUtils::decodeMapUnitScale( props.value( QStringLiteral( "wedgeWidthMapUnitScale" ) ).toString() );

  mCornerRadius = props.value( QStringLiteral( "cornerRadius" ), 0 ).toDouble();
  mCornerRadiusUnit = QgsUnitTypes::decodeRenderUnit( props.value( QStringLiteral( "cornerRadiusUnit" ) ).toString() );
  mCornerRadiusScale = QgsSymbolLayerUtils::decodeMapUnitScale( props.value( QStringLiteral( "cornerRadiusMapUnitScale" ) ).toString() );
}

void QgsBalloonCallout::startRender( QgsRenderContext &context )
{
  QgsCallout::startRender( context );
  if ( mFillSymbol )
    mFillSymbol->startRender( context );
}

void QgsBalloonCallout::stopRender( QgsRenderContext &context )
{
  QgsCallout::stopRender( context );
  if ( mFillSymbol )
    mFillSymbol->stopRender( context );
}

QSet<QString> QgsBalloonCallout::referencedFields( const QgsRenderContext &context ) const
{
  QSet<QString> fields = QgsCallout::referencedFields( context );
  if ( mFillSymbol )
    fields.unite( mFillSymbol->usedAttributes( context ) );
  return fields;
}

QgsFillSymbol *QgsBalloonCallout::fillSymbol()
{
  return mFillSymbol.get();
}

void QgsBalloonCallout::setFillSymbol( QgsFillSymbol *symbol )
{
  mFillSymbol.reset( symbol );
}

void QgsBalloonCallout::draw( QgsRenderContext &context, const QRectF &rect, const double, const QgsGeometry &anchor, QgsCalloutContext &calloutContext )
{
  bool destinationIsPinned = false;
  QgsGeometry line = calloutLineToPart( QgsGeometry::fromRect( rect ), anchor.constGet(), context, calloutContext, destinationIsPinned );

  double offsetFromAnchor = mOffsetFromAnchorDistance;
  if ( dataDefinedProperties().isActive( QgsCallout::OffsetFromAnchor ) )
  {
    context.expressionContext().setOriginalValueVariable( offsetFromAnchor );
    offsetFromAnchor = dataDefinedProperties().valueAsDouble( QgsCallout::OffsetFromAnchor, context.expressionContext(), offsetFromAnchor );
  }
  const double offsetFromAnchorPixels = context.convertToPainterUnits( offsetFromAnchor, mOffsetFromAnchorUnit, mOffsetFromAnchorScale );

  if ( offsetFromAnchorPixels > 0 )
  {
    if ( const QgsLineString *ls = qgsgeometry_cast< const QgsLineString * >( line.constGet() ) )
    {
      line = QgsGeometry( ls->curveSubstring( 0, ls->length() - offsetFromAnchorPixels ) );
    }
  }

  QgsPointXY destination;
  QgsPointXY origin;
  if ( const QgsLineString *ls = qgsgeometry_cast< const QgsLineString * >( line.constGet() ) )
  {
    origin = ls->startPoint();
    destination = ls->endPoint();
  }
  else
  {
    destination = QgsPointXY( rect.center() );
  }

  const QPolygonF points = getPoints( context, destination, rect );
  if ( points.empty() )
    return;

  if ( !origin.isEmpty() )
  {
    QgsCalloutPosition position;
    position.setOrigin( context.mapToPixel().toMapCoordinates( origin.x(), origin.y() ).toQPointF() );
    position.setOriginIsPinned( false );
    position.setDestination( context.mapToPixel().toMapCoordinates( destination.x(), destination.y() ).toQPointF() );
    position.setDestinationIsPinned( destinationIsPinned );
    calloutContext.addCalloutPosition( position );
  }

  mFillSymbol->renderPolygon( points, nullptr, nullptr, context );
}

QPolygonF QgsBalloonCallout::getPoints( QgsRenderContext &context, QgsPointXY origin, QRectF rect ) const
{
  double segmentPointWidth = mWedgeWidth;
  if ( dataDefinedProperties().isActive( QgsCallout::WedgeWidth ) )
  {
    context.expressionContext().setOriginalValueVariable( segmentPointWidth );
    segmentPointWidth = dataDefinedProperties().valueAsDouble( QgsCallout::WedgeWidth, context.expressionContext(), segmentPointWidth );
  }
  segmentPointWidth = context.convertToPainterUnits( segmentPointWidth, mWedgeWidthUnit, mWedgeWidthScale );

  double cornerRadius = mCornerRadius;
  if ( dataDefinedProperties().isActive( QgsCallout::CornerRadius ) )
  {
    context.expressionContext().setOriginalValueVariable( cornerRadius );
    cornerRadius = dataDefinedProperties().valueAsDouble( QgsCallout::CornerRadius, context.expressionContext(), cornerRadius );
  }
  cornerRadius = context.convertToPainterUnits( cornerRadius, mCornerRadiusUnit, mCornerRadiusScale );

  double left = mMargins.left();
  double right = mMargins.right();
  double top = mMargins.top();
  double bottom = mMargins.bottom();

  if ( dataDefinedProperties().isActive( QgsCallout::Margins ) )
  {
    const QVariant value = dataDefinedProperties().value( QgsCallout::Margins, context.expressionContext() );
    if ( !QgsVariantUtils::isNull( value ) )
    {
      if ( value.type() == QVariant::List )
      {
        const QVariantList list = value.toList();
        if ( list.size() == 4 )
        {
          bool topOk = false;
          bool rightOk = false;
          bool bottomOk = false;
          bool leftOk = false;
          const double evaluatedTop = list.at( 0 ).toDouble( &topOk );
          const double evaluatedRight = list.at( 1 ).toDouble( &rightOk );
          const double evaluatedBottom = list.at( 2 ).toDouble( &bottomOk );
          const double evaluatedLeft = list.at( 3 ).toDouble( &leftOk );
          if ( topOk && rightOk && bottomOk && leftOk )
          {
            left = evaluatedLeft;
            top = evaluatedTop;
            right = evaluatedRight;
            bottom = evaluatedBottom;
          }
        }
      }
      else
      {
        const QStringList list = value.toString().trimmed().split( ',' );
        if ( list.count() == 4 )
        {
          bool topOk = false;
          bool rightOk = false;
          bool bottomOk = false;
          bool leftOk = false;
          const double evaluatedTop = list.at( 0 ).toDouble( &topOk );
          const double evaluatedRight = list.at( 1 ).toDouble( &rightOk );
          const double evaluatedBottom = list.at( 2 ).toDouble( &bottomOk );
          const double evaluatedLeft = list.at( 3 ).toDouble( &leftOk );
          if ( topOk && rightOk && bottomOk && leftOk )
          {
            left = evaluatedLeft;
            top = evaluatedTop;
            right = evaluatedRight;
            bottom = evaluatedBottom;
          }
        }
      }
    }
  }

  const double marginLeft = context.convertToPainterUnits( left, mMarginUnit );
  const double marginRight = context.convertToPainterUnits( right, mMarginUnit );
  const double marginTop = context.convertToPainterUnits( top, mMarginUnit );
  const double marginBottom = context.convertToPainterUnits( bottom, mMarginUnit );

  const QRectF expandedRect = rect.height() < 0 ?
                              QRectF( rect.left() - marginLeft, rect.top() + marginBottom,
                                      rect.width() + marginLeft + marginRight,
                                      rect.height() - marginTop - marginBottom ) :
                              QRectF( rect.left() - marginLeft, rect.top() - marginTop,
                                      rect.width() + marginLeft + marginRight,
                                      rect.height() + marginTop + marginBottom );

  // IMPORTANT -- check for degenerate height is sometimes >=0, because QRectF are not normalized and we are using painter
  // coordinates with descending vertical axis!
  if ( expandedRect.width() <= 0 || ( rect.height() < 0 && expandedRect.height() >= 0 ) || ( rect.height() > 0 && expandedRect.height() <= 0 ) )
    return QPolygonF();

  const QPainterPath path = QgsShapeGenerator::createBalloon( origin, expandedRect, segmentPointWidth, cornerRadius );
  const QTransform t = QTransform::fromScale( 100, 100 );
  const QTransform ti = t.inverted();
  const QPolygonF poly = path.toFillPolygon( t );
  return ti.map( poly );
}

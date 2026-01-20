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

#include <mutex>

#include "qgscircularstring.h"
#include "qgsfillsymbol.h"
#include "qgsfillsymbollayer.h"
#include "qgsgeometryutils.h"
#include "qgsgeos.h"
#include "qgslinestring.h"
#include "qgslinesymbol.h"
#include "qgslinesymbollayer.h"
#include "qgsmarkersymbol.h"
#include "qgspainting.h"
#include "qgsrendercontext.h"
#include "qgsshapegenerator.h"
#include "qgssymbol.h"
#include "qgssymbollayerutils.h"
#include "qgsunittypes.h"
#include "qgsvariantutils.h"
#include "qgsxmlutils.h"

QgsPropertiesDefinition QgsCallout::sPropertyDefinitions;

void QgsCallout::initPropertyDefinitions()
{
  const QString origin = u"callouts"_s;

  sPropertyDefinitions = QgsPropertiesDefinition
  {
    { static_cast< int >( QgsCallout::Property::MinimumCalloutLength ), QgsPropertyDefinition( "MinimumCalloutLength", QObject::tr( "Minimum callout length" ), QgsPropertyDefinition::DoublePositive, origin ) },
    { static_cast< int >( QgsCallout::Property::OffsetFromAnchor ), QgsPropertyDefinition( "OffsetFromAnchor", QObject::tr( "Offset from feature" ), QgsPropertyDefinition::DoublePositive, origin ) },
    { static_cast< int >( QgsCallout::Property::OffsetFromLabel ), QgsPropertyDefinition( "OffsetFromLabel", QObject::tr( "Offset from label" ), QgsPropertyDefinition::DoublePositive, origin ) },
    { static_cast< int >( QgsCallout::Property::DrawCalloutToAllParts ), QgsPropertyDefinition( "DrawCalloutToAllParts", QObject::tr( "Draw lines to all feature parts" ), QgsPropertyDefinition::Boolean, origin ) },
    { static_cast< int >( QgsCallout::Property::AnchorPointPosition ), QgsPropertyDefinition( "AnchorPointPosition", QgsPropertyDefinition::DataTypeString, QObject::tr( "Feature's anchor point position" ), QObject::tr( "string " ) + "[<b>pole_of_inaccessibility</b>|<b>point_on_exterior</b>|<b>point_on_surface</b>|<b>centroid</b>]", origin ) },
    {
      static_cast< int >( QgsCallout::Property::LabelAnchorPointPosition ), QgsPropertyDefinition( "LabelAnchorPointPosition", QgsPropertyDefinition::DataTypeString, QObject::tr( "Label's anchor point position" ), QObject::tr( "string " ) + "[<b>point_on_exterior</b>|<b>centroid</b>|<b>TL</b>=Top left|<b>T</b>=Top middle|"
          "<b>TR</b>=Top right|<br>"
          "<b>L</b>=Left|<b>R</b>=Right|<br>"
          "<b>BL</b>=Bottom left|<b>B</b>=Bottom middle|"
          "<b>BR</b>=Bottom right]", origin )
    },
    { static_cast< int >( QgsCallout::Property::OriginX ), QgsPropertyDefinition( "OriginX", QObject::tr( "Callout origin (X)" ), QgsPropertyDefinition::Double, origin ) },
    { static_cast< int >( QgsCallout::Property::OriginY ), QgsPropertyDefinition( "OriginY", QObject::tr( "Callout origin (Y)" ), QgsPropertyDefinition::Double, origin ) },
    { static_cast< int >( QgsCallout::Property::DestinationX ), QgsPropertyDefinition( "DestinationX", QObject::tr( "Callout destination (X)" ), QgsPropertyDefinition::Double, origin ) },
    { static_cast< int >( QgsCallout::Property::DestinationY ), QgsPropertyDefinition( "DestinationY", QObject::tr( "Callout destination (Y)" ), QgsPropertyDefinition::Double, origin ) },
    { static_cast< int >( QgsCallout::Property::Curvature ), QgsPropertyDefinition( "Curvature", QObject::tr( "Callout line curvature" ), QgsPropertyDefinition::Double, origin ) },
    {
      static_cast< int >( QgsCallout::Property::Orientation ), QgsPropertyDefinition( "Orientation", QgsPropertyDefinition::DataTypeString, QObject::tr( "Callout curve orientation" ),  QObject::tr( "string " ) + "[<b>auto</b>|<b>clockwise</b>|<b>counterclockwise</b>]", origin )
    },
    {
      static_cast< int >( QgsCallout::Property::Margins ), QgsPropertyDefinition( "Margins", QgsPropertyDefinition::DataTypeString, QObject::tr( "Margins" ), QObject::tr( "string of four doubles '<b>top,right,bottom,left</b>' or array of doubles <b>[top, right, bottom, left]</b>" ) )
    },
    { static_cast< int >( QgsCallout::Property::WedgeWidth ), QgsPropertyDefinition( "WedgeWidth", QObject::tr( "Wedge width" ), QgsPropertyDefinition::DoublePositive, origin ) },
    { static_cast< int >( QgsCallout::Property::CornerRadius ), QgsPropertyDefinition( "CornerRadius", QObject::tr( "Corner radius" ), QgsPropertyDefinition::DoublePositive, origin ) },
    { static_cast< int >( QgsCallout::Property::BlendMode ), QgsPropertyDefinition( "BlendMode", QObject::tr( "Callout blend mode" ), QgsPropertyDefinition::BlendMode, origin ) },
  };
}


QgsCallout::QgsCallout()
{
}

QVariantMap QgsCallout::properties( const QgsReadWriteContext & ) const
{
  QVariantMap props;
  props.insert( u"enabled"_s, mEnabled ? "1" : "0" );
  props.insert( u"anchorPoint"_s, encodeAnchorPoint( mAnchorPoint ) );
  props.insert( u"labelAnchorPoint"_s, encodeLabelAnchorPoint( mLabelAnchorPoint ) );
  props.insert( u"blendMode"_s, static_cast< int >( QgsPainting::getBlendModeEnum( mBlendMode ) ) );
  props.insert( u"ddProperties"_s, mDataDefinedProperties.toVariant( propertyDefinitions() ) );
  return props;
}

void QgsCallout::readProperties( const QVariantMap &props, const QgsReadWriteContext & )
{
  mEnabled = props.value( u"enabled"_s, u"0"_s ).toInt();
  mAnchorPoint = decodeAnchorPoint( props.value( u"anchorPoint"_s, QString() ).toString() );
  mLabelAnchorPoint = decodeLabelAnchorPoint( props.value( u"labelAnchorPoint"_s, QString() ).toString() );
  mBlendMode = QgsPainting::getCompositionMode(
                 static_cast< Qgis::BlendMode >( props.value( u"blendMode"_s, QString::number( static_cast< int >( Qgis::BlendMode::Normal ) ) ).toUInt() ) );
  mDataDefinedProperties.loadVariant( props.value( u"ddProperties"_s ), propertyDefinitions() );
}

bool QgsCallout::saveProperties( QDomDocument &doc, QDomElement &element, const QgsReadWriteContext &context ) const
{
  if ( element.isNull() )
  {
    return false;
  }

  const QDomElement calloutPropsElement = QgsXmlUtils::writeVariant( properties( context ), doc );

  QDomElement calloutElement = doc.createElement( u"callout"_s );
  calloutElement.setAttribute( u"type"_s, type() );
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
  return mBlendMode != QPainter::CompositionMode_SourceOver || dataDefinedProperties().isActive( QgsCallout::Property::BlendMode );
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
  if ( context.rasterizedRenderingPolicy() != Qgis::RasterizedRenderingPolicy::ForceVector )
  {
    const QPainter::CompositionMode blendMode = mBlendMode;
    if ( dataDefinedProperties().isActive( QgsCallout::Property::BlendMode ) )
    {
      context.expressionContext().setOriginalValueVariable( QString() );
      mBlendMode = QgsSymbolLayerUtils::decodeBlendMode( dataDefinedProperties().valueAsString( QgsCallout::Property::BlendMode, context.expressionContext(), QString() ) );
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
  std::call_once( initialized, initPropertyDefinitions );
  return sPropertyDefinitions;
}

QgsCallout::AnchorPoint QgsCallout::decodeAnchorPoint( const QString &name, bool *ok )
{
  if ( ok )
    *ok = true;
  const QString cleaned = name.toLower().trimmed();

  if ( cleaned == "pole_of_inaccessibility"_L1 )
    return PoleOfInaccessibility;
  else if ( cleaned == "point_on_exterior"_L1 )
    return PointOnExterior;
  else if ( cleaned == "point_on_surface"_L1 )
    return PointOnSurface;
  else if ( cleaned == "centroid"_L1 )
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
      return u"pole_of_inaccessibility"_s;
    case PointOnExterior:
      return u"point_on_exterior"_s;
    case PointOnSurface:
      return u"point_on_surface"_s;
    case Centroid:
      return u"centroid"_s;
  }
  return QString();
}

QString QgsCallout::encodeLabelAnchorPoint( QgsCallout::LabelAnchorPoint anchor )
{
  switch ( anchor )
  {
    case LabelPointOnExterior:
      return u"point_on_exterior"_s;
    case LabelCentroid:
      return u"centroid"_s;
    case LabelTopLeft:
      return u"tl"_s;
    case LabelTopMiddle:
      return u"t"_s;
    case LabelTopRight:
      return u"tr"_s;
    case LabelMiddleLeft:
      return u"l"_s;
    case LabelMiddleRight:
      return u"r"_s;
    case LabelBottomLeft:
      return u"bl"_s;
    case LabelBottomMiddle:
      return u"b"_s;
    case LabelBottomRight:
      return u"br"_s;
  }

  return QString();
}

QgsCallout::LabelAnchorPoint QgsCallout::decodeLabelAnchorPoint( const QString &name, bool *ok )
{
  if ( ok )
    *ok = true;
  const QString cleaned = name.toLower().trimmed();

  if ( cleaned == "point_on_exterior"_L1 )
    return LabelPointOnExterior;
  else if ( cleaned == "centroid"_L1 )
    return LabelCentroid;
  else if ( cleaned == "tl"_L1 )
    return LabelTopLeft;
  else if ( cleaned == "t"_L1 )
    return LabelTopMiddle;
  else if ( cleaned == "tr"_L1 )
    return LabelTopRight;
  else if ( cleaned == "l"_L1 )
    return LabelMiddleLeft;
  else if ( cleaned == "r"_L1 )
    return LabelMiddleRight;
  else if ( cleaned == "bl"_L1 )
    return LabelBottomLeft;
  else if ( cleaned == "b"_L1 )
    return LabelBottomMiddle;
  else if ( cleaned == "br"_L1 )
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
  if ( dataDefinedProperties().isActive( QgsCallout::Property::OriginX ) && dataDefinedProperties().isActive( QgsCallout::Property::OriginY ) )
  {
    bool ok = false;
    const double x = dataDefinedProperties().valueAsDouble( QgsCallout::Property::OriginX, context.expressionContext(), 0, &ok );
    if ( ok )
    {
      const double y = dataDefinedProperties().valueAsDouble( QgsCallout::Property::OriginY, context.expressionContext(), 0, &ok );
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

  if ( dataDefinedProperties().isActive( QgsCallout::Property::DestinationX ) && dataDefinedProperties().isActive( QgsCallout::Property::DestinationY ) )
  {
    bool ok = false;
    const double x = dataDefinedProperties().valueAsDouble( QgsCallout::Property::DestinationX, context.expressionContext(), 0, &ok );
    if ( ok )
    {
      const double y = dataDefinedProperties().valueAsDouble( QgsCallout::Property::DestinationY, context.expressionContext(), 0, &ok );
      if ( ok )
      {
        pinned = true;
        tempPartAnchor = std::make_unique< QgsPoint >( Qgis::WkbType::Point, x, y );
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

  if ( dataDefinedProperties().isActive( QgsCallout::Property::AnchorPointPosition ) )
  {
    const QString encodedAnchor = encodeAnchorPoint( anchor );
    context.expressionContext().setOriginalValueVariable( encodedAnchor );
    anchor = decodeAnchorPoint( dataDefinedProperties().valueAsString( QgsCallout::Property::AnchorPointPosition, context.expressionContext(), encodedAnchor ) );
  }

  QgsGeometry line;
  const QgsGeos labelGeos( labelGeometry.constGet() );

  switch ( QgsWkbTypes::geometryType( evaluatedPartAnchor->wkbType() ) )
  {
    case Qgis::GeometryType::Point:
    case Qgis::GeometryType::Line:
    {
      line = QgsGeometry( labelGeos.shortestLine( evaluatedPartAnchor ) );
      break;
    }

    case Qgis::GeometryType::Polygon:
    {
      if ( labelGeos.intersects( evaluatedPartAnchor ) )
        return QgsGeometry();

      // ideally avoid this unwanted clone in future. For now we need it because poleOfInaccessibility/pointOnSurface are
      // only available to QgsGeometry objects
      const QgsGeometry evaluatedPartAnchorGeom( evaluatedPartAnchor->clone() );
      switch ( anchor )
      {
        case QgsCallout::PoleOfInaccessibility:
          line = QgsGeometry( labelGeos.shortestLine( evaluatedPartAnchorGeom.poleOfInaccessibility( std::max( evaluatedPartAnchor->boundingBox().width(), evaluatedPartAnchor->boundingBox().height() ) / 20.0 ) ) ); // really rough (but quick) pole of inaccessibility
          break;
        case QgsCallout::PointOnSurface:
          line = QgsGeometry( labelGeos.shortestLine( evaluatedPartAnchorGeom.pointOnSurface() ) );
          break;
        case QgsCallout::PointOnExterior:
          line = QgsGeometry( labelGeos.shortestLine( evaluatedPartAnchor ) );
          break;
        case QgsCallout::Centroid:
          line = QgsGeometry( labelGeos.shortestLine( evaluatedPartAnchorGeom.centroid() ) );
          break;
      }
      break;
    }

    case Qgis::GeometryType::Null:
    case Qgis::GeometryType::Unknown:
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
  auto callout = std::make_unique< QgsSimpleLineCallout >();
  callout->readProperties( properties, context );
  return callout.release();
}

QString QgsSimpleLineCallout::type() const
{
  return u"simple"_s;
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
    props[ u"lineSymbol"_s ] = QgsSymbolLayerUtils::symbolProperties( mLineSymbol.get() );
  }
  props[ u"minLength"_s ] = mMinCalloutLength;
  props[ u"minLengthUnit"_s ] = QgsUnitTypes::encodeUnit( mMinCalloutLengthUnit );
  props[ u"minLengthMapUnitScale"_s ] = QgsSymbolLayerUtils::encodeMapUnitScale( mMinCalloutLengthScale );

  props[ u"offsetFromAnchor"_s ] = mOffsetFromAnchorDistance;
  props[ u"offsetFromAnchorUnit"_s ] = QgsUnitTypes::encodeUnit( mOffsetFromAnchorUnit );
  props[ u"offsetFromAnchorMapUnitScale"_s ] = QgsSymbolLayerUtils::encodeMapUnitScale( mOffsetFromAnchorScale );
  props[ u"offsetFromLabel"_s ] = mOffsetFromLabelDistance;
  props[ u"offsetFromLabelUnit"_s ] = QgsUnitTypes::encodeUnit( mOffsetFromLabelUnit );
  props[ u"offsetFromLabelMapUnitScale"_s ] = QgsSymbolLayerUtils::encodeMapUnitScale( mOffsetFromLabelScale );

  props[ u"drawToAllParts"_s ] = mDrawCalloutToAllParts;

  return props;
}

void QgsSimpleLineCallout::readProperties( const QVariantMap &props, const QgsReadWriteContext &context )
{
  QgsCallout::readProperties( props, context );

  const QString lineSymbolDef = props.value( u"lineSymbol"_s ).toString();
  QDomDocument doc( u"symbol"_s );
  doc.setContent( lineSymbolDef );
  const QDomElement symbolElem = doc.firstChildElement( u"symbol"_s );
  std::unique_ptr< QgsLineSymbol > lineSymbol( QgsSymbolLayerUtils::loadSymbol< QgsLineSymbol >( symbolElem, context ) );
  if ( lineSymbol )
    mLineSymbol = std::move( lineSymbol );

  mMinCalloutLength = props.value( u"minLength"_s, 0 ).toDouble();
  mMinCalloutLengthUnit = QgsUnitTypes::decodeRenderUnit( props.value( u"minLengthUnit"_s ).toString() );
  mMinCalloutLengthScale = QgsSymbolLayerUtils::decodeMapUnitScale( props.value( u"minLengthMapUnitScale"_s ).toString() );

  mOffsetFromAnchorDistance = props.value( u"offsetFromAnchor"_s, 0 ).toDouble();
  mOffsetFromAnchorUnit = QgsUnitTypes::decodeRenderUnit( props.value( u"offsetFromAnchorUnit"_s ).toString() );
  mOffsetFromAnchorScale = QgsSymbolLayerUtils::decodeMapUnitScale( props.value( u"offsetFromAnchorMapUnitScale"_s ).toString() );
  mOffsetFromLabelDistance = props.value( u"offsetFromLabel"_s, 0 ).toDouble();
  mOffsetFromLabelUnit = QgsUnitTypes::decodeRenderUnit( props.value( u"offsetFromLabelUnit"_s ).toString() );
  mOffsetFromLabelScale = QgsSymbolLayerUtils::decodeMapUnitScale( props.value( u"offsetFromLabelMapUnitScale"_s ).toString() );

  mDrawCalloutToAllParts = props.value( u"drawToAllParts"_s, false ).toBool();
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
  if ( dataDefinedProperties().isActive( QgsCallout::Property::LabelAnchorPointPosition ) )
  {
    const QString encodedAnchor = encodeLabelAnchorPoint( labelAnchor );
    context.expressionContext().setOriginalValueVariable( encodedAnchor );
    labelAnchor = decodeLabelAnchorPoint( dataDefinedProperties().valueAsString( QgsCallout::Property::LabelAnchorPointPosition, context.expressionContext(), encodedAnchor ) );
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
    if ( dataDefinedProperties().isActive( QgsCallout::Property::MinimumCalloutLength ) )
    {
      context.expressionContext().setOriginalValueVariable( minLength );
      minLength = dataDefinedProperties().valueAsDouble( QgsCallout::Property::MinimumCalloutLength, context.expressionContext(), minLength );
    }
    const double minLengthPixels = context.convertToPainterUnits( minLength, mMinCalloutLengthUnit, mMinCalloutLengthScale );
    if ( minLengthPixels > 0 && lineLength < minLengthPixels )
      return; // too small!

    std::unique_ptr< QgsCurve > calloutCurve( createCalloutLine( qgsgeometry_cast< const QgsLineString * >( line.constGet() )->startPoint(),
        qgsgeometry_cast< const QgsLineString * >( line.constGet() )->endPoint(), context, rect, angle, anchor, calloutContext ) );

    double offsetFromAnchor = mOffsetFromAnchorDistance;
    if ( dataDefinedProperties().isActive( QgsCallout::Property::OffsetFromAnchor ) )
    {
      context.expressionContext().setOriginalValueVariable( offsetFromAnchor );
      offsetFromAnchor = dataDefinedProperties().valueAsDouble( QgsCallout::Property::OffsetFromAnchor, context.expressionContext(), offsetFromAnchor );
    }
    const double offsetFromAnchorPixels = context.convertToPainterUnits( offsetFromAnchor, mOffsetFromAnchorUnit, mOffsetFromAnchorScale );

    double offsetFromLabel = mOffsetFromLabelDistance;
    if ( dataDefinedProperties().isActive( QgsCallout::Property::OffsetFromLabel ) )
    {
      context.expressionContext().setOriginalValueVariable( offsetFromLabel );
      offsetFromLabel = dataDefinedProperties().valueAsDouble( QgsCallout::Property::OffsetFromLabel, context.expressionContext(), offsetFromLabel );
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
  if ( dataDefinedProperties().isActive( QgsCallout::Property::DrawCalloutToAllParts ) )
  {
    context.expressionContext().setOriginalValueVariable( toAllParts );
    toAllParts = dataDefinedProperties().valueAsBool( QgsCallout::Property::DrawCalloutToAllParts, context.expressionContext(), toAllParts );
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


QgsCallout *QgsManhattanLineCallout::create( const QVariantMap &properties, const QgsReadWriteContext &context ) // cppcheck-suppress duplInheritedMember
{
  auto callout = std::make_unique< QgsManhattanLineCallout >();
  callout->readProperties( properties, context );
  return callout.release();
}

QString QgsManhattanLineCallout::type() const
{
  return u"manhattan"_s;
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

QgsCallout *QgsCurvedLineCallout::create( const QVariantMap &properties, const QgsReadWriteContext &context ) // cppcheck-suppress duplInheritedMember
{
  auto callout = std::make_unique< QgsCurvedLineCallout >();
  callout->readProperties( properties, context );

  callout->setCurvature( properties.value( u"curvature"_s, 0.1 ).toDouble() );
  callout->setOrientation( decodeOrientation( properties.value( u"orientation"_s, u"auto"_s ).toString() ) );

  return callout.release();
}

QString QgsCurvedLineCallout::type() const
{
  return u"curved"_s;
}

QgsCurvedLineCallout *QgsCurvedLineCallout::clone() const
{
  return new QgsCurvedLineCallout( *this );
}

QVariantMap QgsCurvedLineCallout::properties( const QgsReadWriteContext &context ) const
{
  QVariantMap props = QgsSimpleLineCallout::properties( context );
  props.insert( u"curvature"_s, mCurvature );
  props.insert( u"orientation"_s, encodeOrientation( mOrientation ) );
  return props;
}

QgsCurve *QgsCurvedLineCallout::createCalloutLine( const QgsPoint &start, const QgsPoint &end, QgsRenderContext &context, const QRectF &rect, const double, const QgsGeometry &, QgsCallout::QgsCalloutContext & ) const
{
  double curvature = mCurvature * 100;
  if ( dataDefinedProperties().isActive( QgsCallout::Property::Curvature ) )
  {
    context.expressionContext().setOriginalValueVariable( curvature );
    curvature = dataDefinedProperties().valueAsDouble( QgsCallout::Property::Curvature, context.expressionContext(), curvature );
  }

  Orientation orientation = mOrientation;
  if ( dataDefinedProperties().isActive( QgsCallout::Property::Orientation ) )
  {
    bool ok = false;
    const QString orientationString = dataDefinedProperties().property( QgsCallout::Property::Orientation ).valueAsString( context.expressionContext(), QString(), &ok );
    if ( ok )
    {
      orientation = decodeOrientation( orientationString );
    }
  }

  if ( orientation == Automatic )
  {
    // to calculate automatically the best curve orientation, we first check which side of the label bounding box
    // the callout origin is nearest to
    switch ( QgsGeometryUtilsBase::closestSideOfRectangle( rect.right(), rect.bottom(), rect.left(), rect.top(), start.x(), start.y() ) )
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
  QgsGeometryUtilsBase::perpendicularOffsetPointAlongSegment( start.x(), start.y(), end.x(), end.y(), 0.5, distance, &midX, &midY );

  return new QgsCircularString( start, QgsPoint( midX, midY ), end );
}

QgsCurvedLineCallout::Orientation QgsCurvedLineCallout::decodeOrientation( const QString &string )
{
  const QString cleaned = string.toLower().trimmed();
  if ( cleaned == "auto"_L1 )
    return Automatic;
  if ( cleaned == "clockwise"_L1 )
    return Clockwise;
  if ( cleaned == "counterclockwise"_L1 )
    return CounterClockwise;
  return Automatic;
}

QString QgsCurvedLineCallout::encodeOrientation( QgsCurvedLineCallout::Orientation orientation )
{
  switch ( orientation )
  {
    case QgsCurvedLineCallout::Automatic:
      return u"auto"_s;
    case QgsCurvedLineCallout::Clockwise:
      return u"clockwise"_s;
    case QgsCurvedLineCallout::CounterClockwise:
      return u"counterclockwise"_s;
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
  , mMarkerSymbol( other.mMarkerSymbol ? other.mMarkerSymbol->clone() : nullptr )
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
  auto callout = std::make_unique< QgsBalloonCallout >();
  callout->readProperties( properties, context );
  return callout.release();
}

QString QgsBalloonCallout::type() const
{
  return u"balloon"_s;
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
    props[ u"fillSymbol"_s ] = QgsSymbolLayerUtils::symbolProperties( mFillSymbol.get() );
  }

  if ( mMarkerSymbol )
  {
    props[ u"markerSymbol"_s ] = QgsSymbolLayerUtils::symbolProperties( mMarkerSymbol.get() );
  }

  props[ u"offsetFromAnchor"_s ] = mOffsetFromAnchorDistance;
  props[ u"offsetFromAnchorUnit"_s ] = QgsUnitTypes::encodeUnit( mOffsetFromAnchorUnit );
  props[ u"offsetFromAnchorMapUnitScale"_s ] = QgsSymbolLayerUtils::encodeMapUnitScale( mOffsetFromAnchorScale );

  props[ u"margins"_s ] = mMargins.toString();
  props[ u"marginsUnit"_s ] = QgsUnitTypes::encodeUnit( mMarginUnit );

  props[ u"wedgeWidth"_s ] = mWedgeWidth;
  props[ u"wedgeWidthUnit"_s ] = QgsUnitTypes::encodeUnit( mWedgeWidthUnit );
  props[ u"wedgeWidthMapUnitScale"_s ] = QgsSymbolLayerUtils::encodeMapUnitScale( mWedgeWidthScale );

  props[ u"cornerRadius"_s ] = mCornerRadius;
  props[ u"cornerRadiusUnit"_s ] = QgsUnitTypes::encodeUnit( mCornerRadiusUnit );
  props[ u"cornerRadiusMapUnitScale"_s ] = QgsSymbolLayerUtils::encodeMapUnitScale( mCornerRadiusScale );

  return props;
}

void QgsBalloonCallout::readProperties( const QVariantMap &props, const QgsReadWriteContext &context )
{
  QgsCallout::readProperties( props, context );

  {
    const QString fillSymbolDef = props.value( u"fillSymbol"_s ).toString();
    QDomDocument doc( u"symbol"_s );
    doc.setContent( fillSymbolDef );
    const QDomElement symbolElem = doc.firstChildElement( u"symbol"_s );
    std::unique_ptr< QgsFillSymbol > fillSymbol( QgsSymbolLayerUtils::loadSymbol< QgsFillSymbol >( symbolElem, context ) );
    if ( fillSymbol )
      mFillSymbol = std::move( fillSymbol );
  }

  {
    const QString markerSymbolDef = props.value( u"markerSymbol"_s ).toString();
    QDomDocument doc( u"symbol"_s );
    doc.setContent( markerSymbolDef );
    const QDomElement symbolElem = doc.firstChildElement( u"symbol"_s );
    std::unique_ptr< QgsMarkerSymbol > markerSymbol( QgsSymbolLayerUtils::loadSymbol< QgsMarkerSymbol >( symbolElem, context ) );
    if ( markerSymbol )
      mMarkerSymbol = std::move( markerSymbol );
  }

  mOffsetFromAnchorDistance = props.value( u"offsetFromAnchor"_s, 0 ).toDouble();
  mOffsetFromAnchorUnit = QgsUnitTypes::decodeRenderUnit( props.value( u"offsetFromAnchorUnit"_s ).toString() );
  mOffsetFromAnchorScale = QgsSymbolLayerUtils::decodeMapUnitScale( props.value( u"offsetFromAnchorMapUnitScale"_s ).toString() );

  mMargins = QgsMargins::fromString( props.value( u"margins"_s ).toString() );
  mMarginUnit = QgsUnitTypes::decodeRenderUnit( props.value( u"marginsUnit"_s ).toString() );

  mWedgeWidth = props.value( u"wedgeWidth"_s, 2.64 ).toDouble();
  mWedgeWidthUnit = QgsUnitTypes::decodeRenderUnit( props.value( u"wedgeWidthUnit"_s ).toString() );
  mWedgeWidthScale = QgsSymbolLayerUtils::decodeMapUnitScale( props.value( u"wedgeWidthMapUnitScale"_s ).toString() );

  mCornerRadius = props.value( u"cornerRadius"_s, 0 ).toDouble();
  mCornerRadiusUnit = QgsUnitTypes::decodeRenderUnit( props.value( u"cornerRadiusUnit"_s ).toString() );
  mCornerRadiusScale = QgsSymbolLayerUtils::decodeMapUnitScale( props.value( u"cornerRadiusMapUnitScale"_s ).toString() );
}

void QgsBalloonCallout::startRender( QgsRenderContext &context )
{
  QgsCallout::startRender( context );
  if ( mFillSymbol )
    mFillSymbol->startRender( context );
  if ( mMarkerSymbol )
    mMarkerSymbol->startRender( context );
}

void QgsBalloonCallout::stopRender( QgsRenderContext &context )
{
  QgsCallout::stopRender( context );
  if ( mFillSymbol )
    mFillSymbol->stopRender( context );
  if ( mMarkerSymbol )
    mMarkerSymbol->stopRender( context );
}

QSet<QString> QgsBalloonCallout::referencedFields( const QgsRenderContext &context ) const
{
  QSet<QString> fields = QgsCallout::referencedFields( context );
  if ( mFillSymbol )
    fields.unite( mFillSymbol->usedAttributes( context ) );
  if ( mMarkerSymbol )
    fields.unite( mMarkerSymbol->usedAttributes( context ) );
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

QgsMarkerSymbol *QgsBalloonCallout::markerSymbol()
{
  return mMarkerSymbol.get();
}

void QgsBalloonCallout::setMarkerSymbol( QgsMarkerSymbol *symbol )
{
  mMarkerSymbol.reset( symbol );
}

void QgsBalloonCallout::draw( QgsRenderContext &context, const QRectF &rect, const double, const QgsGeometry &anchor, QgsCalloutContext &calloutContext )
{
  bool destinationIsPinned = false;
  QgsGeometry line = calloutLineToPart( QgsGeometry::fromRect( rect ), anchor.constGet(), context, calloutContext, destinationIsPinned );

  if ( mMarkerSymbol )
  {
    if ( const QgsLineString *ls = qgsgeometry_cast< const QgsLineString * >( line.constGet() ) )
    {
      QgsPoint anchorPoint = ls->endPoint();
      mMarkerSymbol->renderPoint( anchorPoint.toQPointF(), nullptr, context );
    }
  }

  double offsetFromAnchor = mOffsetFromAnchorDistance;
  if ( dataDefinedProperties().isActive( QgsCallout::Property::OffsetFromAnchor ) )
  {
    context.expressionContext().setOriginalValueVariable( offsetFromAnchor );
    offsetFromAnchor = dataDefinedProperties().valueAsDouble( QgsCallout::Property::OffsetFromAnchor, context.expressionContext(), offsetFromAnchor );
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
  if ( dataDefinedProperties().isActive( QgsCallout::Property::WedgeWidth ) )
  {
    context.expressionContext().setOriginalValueVariable( segmentPointWidth );
    segmentPointWidth = dataDefinedProperties().valueAsDouble( QgsCallout::Property::WedgeWidth, context.expressionContext(), segmentPointWidth );
  }
  segmentPointWidth = context.convertToPainterUnits( segmentPointWidth, mWedgeWidthUnit, mWedgeWidthScale );

  double cornerRadius = mCornerRadius;
  if ( dataDefinedProperties().isActive( QgsCallout::Property::CornerRadius ) )
  {
    context.expressionContext().setOriginalValueVariable( cornerRadius );
    cornerRadius = dataDefinedProperties().valueAsDouble( QgsCallout::Property::CornerRadius, context.expressionContext(), cornerRadius );
  }
  cornerRadius = context.convertToPainterUnits( cornerRadius, mCornerRadiusUnit, mCornerRadiusScale );

  double left = mMargins.left();
  double right = mMargins.right();
  double top = mMargins.top();
  double bottom = mMargins.bottom();

  if ( dataDefinedProperties().isActive( QgsCallout::Property::Margins ) )
  {
    const QVariant value = dataDefinedProperties().value( QgsCallout::Property::Margins, context.expressionContext() );
    if ( !QgsVariantUtils::isNull( value ) )
    {
      if ( value.userType() == QMetaType::Type::QVariantList )
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

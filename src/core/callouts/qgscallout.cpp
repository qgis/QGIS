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
#include "qgssymbollayerutils.h"
#include "qgsxmlutils.h"
#include "qgslinestring.h"
#include "qgslogger.h"
#include <QPainter>
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
  props.insert( QStringLiteral( "ddProperties" ), mDataDefinedProperties.toVariant( propertyDefinitions() ) );
  return props;
}

void QgsCallout::readProperties( const QVariantMap &props, const QgsReadWriteContext & )
{
  mEnabled = props.value( QStringLiteral( "enabled" ), QStringLiteral( "0" ) ).toInt();
  mAnchorPoint = decodeAnchorPoint( props.value( QStringLiteral( "anchorPoint" ), QString() ).toString() );
  mLabelAnchorPoint = decodeLabelAnchorPoint( props.value( QStringLiteral( "labelAnchorPoint" ), QString() ).toString() );
  mDataDefinedProperties.loadVariant( props.value( QStringLiteral( "ddProperties" ) ), propertyDefinitions() );
}

bool QgsCallout::saveProperties( QDomDocument &doc, QDomElement &element, const QgsReadWriteContext &context ) const
{
  if ( element.isNull() )
  {
    return false;
  }

  QDomElement calloutPropsElement = QgsXmlUtils::writeVariant( properties( context ), doc );

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

QSet<QString> QgsCallout::referencedFields( const QgsRenderContext &context ) const
{
  mDataDefinedProperties.prepare( context.expressionContext() );
  return mDataDefinedProperties.referencedFields( context.expressionContext() );
}

QgsCallout::DrawOrder QgsCallout::drawOrder() const
{
  return OrderBelowAllLabels;
}

void QgsCallout::render( QgsRenderContext &context, QRectF rect, const double angle, const QgsGeometry &anchor, QgsCalloutContext &calloutContext )
{
  if ( !mEnabled )
    return;

#if 0 // for debugging
  QPainter *painter = context.painter();
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
  QString cleaned = name.toLower().trimmed();

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
  QString cleaned = name.toLower().trimmed();

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

QgsGeometry QgsCallout::labelAnchorGeometry( QRectF rect, const double angle, LabelAnchorPoint anchor ) const
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

//
// QgsSimpleLineCallout
//

QgsSimpleLineCallout::QgsSimpleLineCallout()
{
  mLineSymbol = qgis::make_unique< QgsLineSymbol >( QgsSymbolLayerList() << new QgsSimpleLineSymbolLayer( QColor( 60, 60, 60 ), .3 ) );

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
  std::unique_ptr< QgsSimpleLineCallout > callout = qgis::make_unique< QgsSimpleLineCallout >();
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
  QDomElement symbolElem = doc.firstChildElement( QStringLiteral( "symbol" ) );
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

void QgsSimpleLineCallout::draw( QgsRenderContext &context, QRectF rect, const double angle, const QgsGeometry &anchor, QgsCalloutContext &calloutContext )
{
  LabelAnchorPoint labelAnchor = labelAnchorPoint();
  if ( dataDefinedProperties().isActive( QgsCallout::LabelAnchorPointPosition ) )
  {
    QString encodedAnchor = encodeLabelAnchorPoint( labelAnchor );
    context.expressionContext().setOriginalValueVariable( encodedAnchor );
    labelAnchor = decodeLabelAnchorPoint( dataDefinedProperties().valueAsString( QgsCallout::LabelAnchorPointPosition, context.expressionContext(), encodedAnchor ) );
  }
  QgsGeometry label = labelAnchorGeometry( rect, angle, labelAnchor );

  auto drawCalloutLine = [this, &context, &label]( const QgsGeometry & partAnchor )
  {
    QgsGeometry line;
    AnchorPoint anchor = anchorPoint();
    if ( dataDefinedProperties().isActive( QgsCallout::AnchorPointPosition ) )
    {
      QString encodedAnchor = encodeAnchorPoint( anchor );
      context.expressionContext().setOriginalValueVariable( encodedAnchor );
      anchor = decodeAnchorPoint( dataDefinedProperties().valueAsString( QgsCallout::AnchorPointPosition, context.expressionContext(), encodedAnchor ) );
    }
    switch ( partAnchor.type() )
    {
      case QgsWkbTypes::PointGeometry:
        line = label.shortestLine( partAnchor );
        break;

      case QgsWkbTypes::LineGeometry:
        line = label.shortestLine( partAnchor );
        break;

      case QgsWkbTypes::PolygonGeometry:
        if ( label.intersects( partAnchor ) )
          return;

        switch ( anchor )
        {
          case QgsCallout::PoleOfInaccessibility:
            line = label.shortestLine( partAnchor.poleOfInaccessibility( std::max( partAnchor.boundingBox().width(), partAnchor.boundingBox().height() ) / 20.0 ) ); // really rough (but quick) pole of inaccessibility
            break;
          case QgsCallout::PointOnSurface:
            line = label.shortestLine( partAnchor.pointOnSurface() );
            break;
          case QgsCallout::PointOnExterior:
            line = label.shortestLine( partAnchor );
            break;
          case QgsCallout::Centroid:
            line = label.shortestLine( partAnchor.centroid() );
            break;
        }
        break;

      case QgsWkbTypes::NullGeometry:
      case QgsWkbTypes::UnknownGeometry:
        return; // shouldn't even get here..
    }

    if ( qgsDoubleNear( line.length(), 0 ) )
      return;

    double minLength = mMinCalloutLength;
    if ( dataDefinedProperties().isActive( QgsCallout::MinimumCalloutLength ) )
    {
      context.expressionContext().setOriginalValueVariable( minLength );
      minLength = dataDefinedProperties().valueAsDouble( QgsCallout::MinimumCalloutLength, context.expressionContext(), minLength );
    }
    double minLengthPixels = context.convertToPainterUnits( minLength, mMinCalloutLengthUnit, mMinCalloutLengthScale );
    if ( minLengthPixels > 0 && line.length() < minLengthPixels )
      return; // too small!

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
      if ( QgsLineString *ls = qgsgeometry_cast< QgsLineString * >( line.get() ) )
      {
        line = QgsGeometry( ls->curveSubstring( offsetFromLabelPixels, ls->length() - offsetFromAnchorPixels ) );
      }
    }

    mLineSymbol->renderPolyline( line.asQPolygonF(), nullptr, context );
  };

  bool toAllParts = mDrawCalloutToAllParts;
  if ( dataDefinedProperties().isActive( QgsCallout::DrawCalloutToAllParts ) )
  {
    context.expressionContext().setOriginalValueVariable( toAllParts );
    toAllParts = dataDefinedProperties().valueAsBool( QgsCallout::DrawCalloutToAllParts, context.expressionContext(), toAllParts );
  }

  if ( calloutContext.allFeaturePartsLabeled || !toAllParts )
    drawCalloutLine( anchor );
  else
  {
    const QVector< QgsGeometry > parts = anchor.asGeometryCollection();
    for ( const QgsGeometry &part : parts )
      drawCalloutLine( part );
  }
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
  std::unique_ptr< QgsManhattanLineCallout > callout = qgis::make_unique< QgsManhattanLineCallout >();
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

void QgsManhattanLineCallout::draw( QgsRenderContext &context, QRectF rect, const double angle, const QgsGeometry &anchor, QgsCalloutContext &calloutContext )
{
  LabelAnchorPoint labelAnchor = labelAnchorPoint();
  if ( dataDefinedProperties().isActive( QgsCallout::LabelAnchorPointPosition ) )
  {
    QString encodedAnchor = encodeLabelAnchorPoint( labelAnchor );
    context.expressionContext().setOriginalValueVariable( encodedAnchor );
    labelAnchor = decodeLabelAnchorPoint( dataDefinedProperties().valueAsString( QgsCallout::LabelAnchorPointPosition, context.expressionContext(), encodedAnchor ) );
  }
  QgsGeometry label = labelAnchorGeometry( rect, angle, labelAnchor );

  auto drawCalloutLine = [this, &context, &label]( const QgsGeometry & partAnchor )
  {
    QgsGeometry line;
    AnchorPoint anchor = anchorPoint();
    if ( dataDefinedProperties().isActive( QgsCallout::AnchorPointPosition ) )
    {
      QString encodedAnchor = encodeAnchorPoint( anchor );
      context.expressionContext().setOriginalValueVariable( encodedAnchor );
      anchor = decodeAnchorPoint( dataDefinedProperties().valueAsString( QgsCallout::AnchorPointPosition, context.expressionContext(), encodedAnchor ) );
    }
    switch ( partAnchor.type() )
    {
      case QgsWkbTypes::PointGeometry:
        line = label.shortestLine( partAnchor );
        break;

      case QgsWkbTypes::LineGeometry:
        line = label.shortestLine( partAnchor );
        break;

      case QgsWkbTypes::PolygonGeometry:
        if ( label.intersects( partAnchor ) )
          return;

        switch ( anchor )
        {
          case QgsCallout::PoleOfInaccessibility:
            line = label.shortestLine( partAnchor.poleOfInaccessibility( std::max( partAnchor.boundingBox().width(), partAnchor.boundingBox().height() ) / 20.0 ) ); // really rough (but quick) pole of inaccessibility
            break;
          case QgsCallout::PointOnSurface:
            line = label.shortestLine( partAnchor.pointOnSurface() );
            break;
          case QgsCallout::PointOnExterior:
            line = label.shortestLine( partAnchor );
            break;
          case QgsCallout::Centroid:
            line = label.shortestLine( partAnchor.centroid() );
            break;
        }
        break;

      case QgsWkbTypes::NullGeometry:
      case QgsWkbTypes::UnknownGeometry:
        return; // shouldn't even get here..
    }

    if ( qgsDoubleNear( line.length(), 0 ) )
      return;

    double minLength = minimumLength();
    if ( dataDefinedProperties().isActive( QgsCallout::MinimumCalloutLength ) )
    {
      minLength = dataDefinedProperties().valueAsDouble( QgsCallout::MinimumCalloutLength, context.expressionContext(), minLength );
    }
    double minLengthPixels = context.convertToPainterUnits( minLength, minimumLengthUnit(), minimumLengthMapUnitScale() );
    if ( minLengthPixels > 0 && line.length() < minLengthPixels )
      return; // too small!

    const QgsPoint start = qgsgeometry_cast< const QgsLineString * >( line.constGet() )->startPoint();
    const QgsPoint end = qgsgeometry_cast< const QgsLineString * >( line.constGet() )->endPoint();
    QgsPoint mid1 = QgsPoint( start.x(), end.y() );

    line = QgsGeometry::fromPolyline( QgsPolyline() << start << mid1 << end );
    double offsetFromAnchorDist = offsetFromAnchor();
    if ( dataDefinedProperties().isActive( QgsCallout::OffsetFromAnchor ) )
    {
      offsetFromAnchorDist = dataDefinedProperties().valueAsDouble( QgsCallout::OffsetFromAnchor, context.expressionContext(), offsetFromAnchorDist );
    }
    const double offsetFromAnchorPixels = context.convertToPainterUnits( offsetFromAnchorDist, offsetFromAnchorUnit(), offsetFromAnchorMapUnitScale() );

    double offsetFromLabelDist = offsetFromLabel();
    if ( dataDefinedProperties().isActive( QgsCallout::OffsetFromLabel ) )
    {
      offsetFromLabelDist = dataDefinedProperties().valueAsDouble( QgsCallout::OffsetFromLabel, context.expressionContext(), offsetFromLabelDist );
    }
    const double offsetFromLabelPixels = context.convertToPainterUnits( offsetFromLabelDist, offsetFromAnchorUnit(), offsetFromAnchorMapUnitScale() );

    if ( offsetFromAnchorPixels > 0 || offsetFromLabelPixels > 0 )
    {
      if ( QgsLineString *ls = qgsgeometry_cast< QgsLineString * >( line.get() ) )
      {
        line = QgsGeometry( ls->curveSubstring( offsetFromLabelPixels, ls->length() - offsetFromAnchorPixels ) );
      }
    }

    lineSymbol()->renderPolyline( line.asQPolygonF(), nullptr, context );
  };

  bool toAllParts = drawCalloutToAllParts();
  if ( dataDefinedProperties().isActive( QgsCallout::DrawCalloutToAllParts ) )
  {
    context.expressionContext().setOriginalValueVariable( toAllParts );
    toAllParts = dataDefinedProperties().valueAsBool( QgsCallout::DrawCalloutToAllParts, context.expressionContext(), toAllParts );
  }

  if ( calloutContext.allFeaturePartsLabeled || !toAllParts )
    drawCalloutLine( anchor );
  else
  {
    const QVector< QgsGeometry > parts = anchor.asGeometryCollection();
    for ( const QgsGeometry &part : parts )
      drawCalloutLine( part );
  }
}

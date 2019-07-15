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
#include <QPainter>
#include <mutex>

QgsPropertiesDefinition QgsCallout::sPropertyDefinitions;

void QgsCallout::initPropertyDefinitions()
{
  const QString origin = QStringLiteral( "callouts" );

  sPropertyDefinitions = QgsPropertiesDefinition
  {
    { QgsCallout::MinimumCalloutLength, QgsPropertyDefinition( "MinimumCalloutLength", QObject::tr( "Minimum callout length" ), QgsPropertyDefinition::DoublePositive, origin ) },
  };
}


QgsCallout::QgsCallout()
{
}

QVariantMap QgsCallout::properties( const QgsReadWriteContext & ) const
{
  QVariantMap props;
  props.insert( QStringLiteral( "enabled" ), mEnabled ? "1" : "0" );
  props.insert( QStringLiteral( "ddProperties" ), mDataDefinedProperties.toVariant( propertyDefinitions() ) );
  return props;
}

void QgsCallout::readProperties( const QVariantMap &props, const QgsReadWriteContext & )
{
  mEnabled = props.value( QStringLiteral( "enabled" ), QStringLiteral( "0" ) ).toInt();
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

void QgsCallout::render( QgsRenderContext &context, QRectF rect, const double angle, const QgsGeometry &anchor )
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

  draw( context, rect, angle, anchor );
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

  mMinCalloutLength = props.value( QStringLiteral( "minLength" ), 0 ).toInt();
  mMinCalloutLengthUnit = QgsUnitTypes::decodeRenderUnit( props.value( QStringLiteral( "minLengthUnit" ) ).toString() );
  mMinCalloutLengthScale = QgsSymbolLayerUtils::decodeMapUnitScale( props.value( QStringLiteral( "minLengthMapUnitScale" ) ).toString() );
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

void QgsSimpleLineCallout::draw( QgsRenderContext &context, QRectF rect, const double, const QgsGeometry &anchor )
{
  QgsGeometry label( QgsGeometry::fromRect( rect ) );
  QgsGeometry line;
  switch ( anchor.type() )
  {
    case QgsWkbTypes::PointGeometry:
      line = label.shortestLine( anchor );
      break;

    case QgsWkbTypes::LineGeometry:
      line = label.shortestLine( anchor );
      break;

    case QgsWkbTypes::PolygonGeometry:
      if ( label.intersects( anchor ) )
        return;

      line = label.shortestLine( anchor.poleOfInaccessibility( std::max( anchor.boundingBox().width(), anchor.boundingBox().height() ) / 20.0 ) ); // really rough (but quick) pole of inaccessibility
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
    minLength = dataDefinedProperties().valueAsDouble( QgsCallout::MinimumCalloutLength, context.expressionContext(), minLength );
  }
  double minLengthPixels = context.convertToPainterUnits( minLength, mMinCalloutLengthUnit, mMinCalloutLengthScale );
  if ( minLengthPixels > 0 && line.length() < minLengthPixels )
    return; // too small!

  mLineSymbol->renderPolyline( line.asQPolygonF(), nullptr, context );
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

void QgsManhattanLineCallout::draw( QgsRenderContext &context, QRectF rect, const double, const QgsGeometry &anchor )
{
  QgsGeometry label( QgsGeometry::fromRect( rect ) );
  QgsGeometry line;
  switch ( anchor.type() )
  {
    case QgsWkbTypes::PointGeometry:
      line = label.shortestLine( anchor );
      break;

    case QgsWkbTypes::LineGeometry:
      line = label.shortestLine( anchor );
      break;

    case QgsWkbTypes::PolygonGeometry:
      if ( label.intersects( anchor ) )
        return;

      line = label.shortestLine( anchor.poleOfInaccessibility( std::max( anchor.boundingBox().width(), anchor.boundingBox().height() ) / 20.0 ) ); // really rough (but quick) pole of inaccessibility
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

  QPolygonF lineF = QPolygonF() << start.toQPointF() << mid1.toQPointF() << end.toQPointF();

  lineSymbol()->renderPolyline( lineF, nullptr, context );
}

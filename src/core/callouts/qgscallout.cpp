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
#include <QPainter>

QVariantMap QgsCallout::properties() const
{
  QVariantMap props;
  props.insert( QStringLiteral( "enabled" ), mEnabled ? "1" : "0" );
  return props;
}

void QgsCallout::readProperties( const QVariantMap &props, const QgsReadWriteContext & )
{
  mEnabled = props.value( QStringLiteral( "enabled" ), QStringLiteral( "0" ) ).toInt();
}

bool QgsCallout::saveProperties( QDomDocument &doc, QDomElement &element ) const
{
  if ( element.isNull() )
  {
    return false;
  }

  QDomElement calloutPropsElement = QgsXmlUtils::writeVariant( properties(), doc );

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

QSet<QString> QgsCallout::referencedFields( const QgsRenderContext & ) const
{
  return QSet< QString >();
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


//
// QgsSimpleLineCallout
//

QgsSimpleLineCallout::QgsSimpleLineCallout()
{
  mLineSymbol = qgis::make_unique< QgsLineSymbol >( QgsSymbolLayerList() << new QgsSimpleLineSymbolLayer( QColor( 60, 60, 60 ), .3 ) );

}

QgsSimpleLineCallout::QgsSimpleLineCallout( const QgsSimpleLineCallout &other )
  : QgsCallout( other )
  , mLineSymbol( other.mLineSymbol ? other.mLineSymbol->clone() : nullptr )
{

}

QgsSimpleLineCallout &QgsSimpleLineCallout::operator=( const QgsSimpleLineCallout &other )
{
  mLineSymbol.reset( other.mLineSymbol ? other.mLineSymbol->clone() : nullptr );
}

QString QgsSimpleLineCallout::type() const
{
  return QStringLiteral( "simple" );
}

QgsSimpleLineCallout *QgsSimpleLineCallout::clone() const
{
  return new QgsSimpleLineCallout( *this );
}

QVariantMap QgsSimpleLineCallout::properties() const
{
  QVariantMap props = QgsCallout::properties();

  if ( mLineSymbol )
  {
    props[ QStringLiteral( "lineSymbol" ) ] = QgsSymbolLayerUtils::symbolProperties( mLineSymbol.get() );
  }

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

  mLineSymbol->renderPolyline( line.asQPolygonF(), nullptr, context );
}

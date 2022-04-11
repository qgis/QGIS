/***************************************************************************
                         qgsvectorlayerelevationproperties.cpp
                         ---------------
    begin                : February 2022
    copyright            : (C) 2022 by Nyall Dawson
    email                : nyall dot dawson dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsvectorlayerelevationproperties.h"
#include "qgslinesymbol.h"
#include "qgsfillsymbol.h"
#include "qgsmarkersymbol.h"
#include "qgssymbollayerutils.h"
#include "qgslinesymbollayer.h"
#include "qgsfillsymbollayer.h"
#include "qgsmarkersymbollayer.h"
#include "qgsapplication.h"
#include "qgscolorschemeregistry.h"

QgsVectorLayerElevationProperties::QgsVectorLayerElevationProperties( QObject *parent )
  : QgsMapLayerElevationProperties( parent )
{
  const QColor color = QgsApplication::colorSchemeRegistry()->fetchRandomStyleColor();
  setDefaultProfileLineSymbol( color );
  setDefaultProfileFillSymbol( color );
  setDefaultProfileMarkerSymbol( color );
}

QgsVectorLayerElevationProperties::~QgsVectorLayerElevationProperties() = default;

bool QgsVectorLayerElevationProperties::hasElevation() const
{
  // layer is considered as having non-default elevation settings if we aren't clamping to terrain
  return mClamping != Qgis::AltitudeClamping::Terrain;
}

QDomElement QgsVectorLayerElevationProperties::writeXml( QDomElement &parentElement, QDomDocument &document, const QgsReadWriteContext &context )
{
  QDomElement element = document.createElement( QStringLiteral( "elevation" ) );
  element.setAttribute( QStringLiteral( "zoffset" ), qgsDoubleToString( mZOffset ) );
  element.setAttribute( QStringLiteral( "zscale" ), qgsDoubleToString( mZScale ) );

  element.setAttribute( QStringLiteral( "extrusionEnabled" ), mEnableExtrusion ? QStringLiteral( "1" ) : QStringLiteral( "0" ) );
  element.setAttribute( QStringLiteral( "extrusion" ), qgsDoubleToString( mExtrusionHeight ) );
  element.setAttribute( QStringLiteral( "clamping" ), qgsEnumValueToKey( mClamping ) );
  element.setAttribute( QStringLiteral( "binding" ), qgsEnumValueToKey( mBinding ) );

  element.setAttribute( QStringLiteral( "respectLayerSymbol" ), mRespectLayerSymbology ? QStringLiteral( "1" ) : QStringLiteral( "0" ) );

  QDomElement profileLineSymbolElement = document.createElement( QStringLiteral( "profileLineSymbol" ) );
  profileLineSymbolElement.appendChild( QgsSymbolLayerUtils::saveSymbol( QString(), mProfileLineSymbol.get(), document, context ) );
  element.appendChild( profileLineSymbolElement );

  QDomElement profileFillSymbolElement = document.createElement( QStringLiteral( "profileFillSymbol" ) );
  profileFillSymbolElement.appendChild( QgsSymbolLayerUtils::saveSymbol( QString(), mProfileFillSymbol.get(), document, context ) );
  element.appendChild( profileFillSymbolElement );

  QDomElement profileMarkerSymbolElement = document.createElement( QStringLiteral( "profileMarkerSymbol" ) );
  profileMarkerSymbolElement.appendChild( QgsSymbolLayerUtils::saveSymbol( QString(), mProfileMarkerSymbol.get(), document, context ) );
  element.appendChild( profileMarkerSymbolElement );

  parentElement.appendChild( element );
  return element;
}

bool QgsVectorLayerElevationProperties::readXml( const QDomElement &element, const QgsReadWriteContext &context )
{
  const QDomElement elevationElement = element.firstChildElement( QStringLiteral( "elevation" ) ).toElement();
  mZOffset = elevationElement.attribute( QStringLiteral( "zoffset" ), QStringLiteral( "0" ) ).toDouble();
  mZScale = elevationElement.attribute( QStringLiteral( "zscale" ), QStringLiteral( "1" ) ).toDouble();

  mClamping = qgsEnumKeyToValue( elevationElement.attribute( QStringLiteral( "clamping" ) ), Qgis::AltitudeClamping::Terrain );
  mBinding = qgsEnumKeyToValue( elevationElement.attribute( QStringLiteral( "binding" ) ), Qgis::AltitudeBinding::Centroid );
  mEnableExtrusion = elevationElement.attribute( QStringLiteral( "extrusionEnabled" ), QStringLiteral( "0" ) ).toInt();
  mExtrusionHeight = elevationElement.attribute( QStringLiteral( "extrusion" ), QStringLiteral( "0" ) ).toDouble();

  mRespectLayerSymbology = elevationElement.attribute( QStringLiteral( "respectLayerSymbol" ), QStringLiteral( "1" ) ).toInt();

  const QColor color = QgsApplication::colorSchemeRegistry()->fetchRandomStyleColor();

  const QDomElement profileLineSymbolElement = elevationElement.firstChildElement( QStringLiteral( "profileLineSymbol" ) ).firstChildElement( QStringLiteral( "symbol" ) );
  mProfileLineSymbol.reset( QgsSymbolLayerUtils::loadSymbol< QgsLineSymbol >( profileLineSymbolElement, context ) );
  if ( !mProfileLineSymbol )
    setDefaultProfileLineSymbol( color );

  const QDomElement profileFillSymbolElement = elevationElement.firstChildElement( QStringLiteral( "profileFillSymbol" ) ).firstChildElement( QStringLiteral( "symbol" ) );
  mProfileFillSymbol.reset( QgsSymbolLayerUtils::loadSymbol< QgsFillSymbol >( profileFillSymbolElement, context ) );
  if ( !mProfileFillSymbol )
    setDefaultProfileFillSymbol( color );

  const QDomElement profileMarkerSymbolElement = elevationElement.firstChildElement( QStringLiteral( "profileMarkerSymbol" ) ).firstChildElement( QStringLiteral( "symbol" ) );
  mProfileMarkerSymbol.reset( QgsSymbolLayerUtils::loadSymbol< QgsMarkerSymbol >( profileMarkerSymbolElement, context ) );
  if ( !mProfileMarkerSymbol )
    setDefaultProfileMarkerSymbol( color );

  return true;
}

QgsVectorLayerElevationProperties *QgsVectorLayerElevationProperties::clone() const
{
  std::unique_ptr< QgsVectorLayerElevationProperties > res = std::make_unique< QgsVectorLayerElevationProperties >( nullptr );
  res->setZOffset( mZOffset );
  res->setZScale( mZScale );
  res->setClamping( mClamping );
  res->setBinding( mBinding );
  res->setExtrusionEnabled( mEnableExtrusion );
  res->setExtrusionHeight( mExtrusionHeight );
  res->setProfileLineSymbol( mProfileLineSymbol->clone() );
  res->setProfileFillSymbol( mProfileFillSymbol->clone() );
  res->setProfileMarkerSymbol( mProfileMarkerSymbol->clone() );
  res->setRespectLayerSymbology( mRespectLayerSymbology );
  return res.release();
}

bool QgsVectorLayerElevationProperties::isVisibleInZRange( const QgsDoubleRange & ) const
{
  // TODO -- test actual layer z range
  return true;
}

QgsDoubleRange QgsVectorLayerElevationProperties::calculateZRange( QgsMapLayer * ) const
{
  // TODO -- determine actual z range from layer statistics
  return QgsDoubleRange();
}

QgsLineSymbol *QgsVectorLayerElevationProperties::profileLineSymbol() const
{
  return mProfileLineSymbol.get();
}

void QgsVectorLayerElevationProperties::setProfileLineSymbol( QgsLineSymbol *symbol )
{
  mProfileLineSymbol.reset( symbol );
}

QgsFillSymbol *QgsVectorLayerElevationProperties::profileFillSymbol() const
{
  return mProfileFillSymbol.get();
}

void QgsVectorLayerElevationProperties::setProfileFillSymbol( QgsFillSymbol *symbol )
{
  mProfileFillSymbol.reset( symbol );
}

QgsMarkerSymbol *QgsVectorLayerElevationProperties::profileMarkerSymbol() const
{
  return mProfileMarkerSymbol.get();
}

void QgsVectorLayerElevationProperties::setProfileMarkerSymbol( QgsMarkerSymbol *symbol )
{
  mProfileMarkerSymbol.reset( symbol );
}

void QgsVectorLayerElevationProperties::setDefaultProfileLineSymbol( const QColor &color )
{
  std::unique_ptr< QgsSimpleLineSymbolLayer > profileLineLayer = std::make_unique< QgsSimpleLineSymbolLayer >( color, 0.6 );
  mProfileLineSymbol = std::make_unique< QgsLineSymbol>( QgsSymbolLayerList( { profileLineLayer.release() } ) );
}

void QgsVectorLayerElevationProperties::setDefaultProfileMarkerSymbol( const QColor &color )
{
  std::unique_ptr< QgsSimpleMarkerSymbolLayer > profileMarkerLayer = std::make_unique< QgsSimpleMarkerSymbolLayer >( Qgis::MarkerShape::Diamond, 3 );
  profileMarkerLayer->setColor( color );
  profileMarkerLayer->setStrokeWidth( 0.2 );
  profileMarkerLayer->setStrokeColor( color.darker( 140 ) );
  mProfileMarkerSymbol = std::make_unique< QgsMarkerSymbol>( QgsSymbolLayerList( { profileMarkerLayer.release() } ) );
}

void QgsVectorLayerElevationProperties::setDefaultProfileFillSymbol( const QColor &color )
{
  std::unique_ptr< QgsSimpleFillSymbolLayer > profileFillLayer = std::make_unique< QgsSimpleFillSymbolLayer >( color );
  profileFillLayer->setStrokeWidth( 0.2 );
  profileFillLayer->setStrokeColor( color.darker( 140 ) );
  mProfileFillSymbol = std::make_unique< QgsFillSymbol>( QgsSymbolLayerList( { profileFillLayer.release() } ) );
}

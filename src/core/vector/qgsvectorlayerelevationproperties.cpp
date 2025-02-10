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
#include "moc_qgsvectorlayerelevationproperties.cpp"
#include "qgslinesymbol.h"
#include "qgsfillsymbol.h"
#include "qgsmarkersymbol.h"
#include "qgssymbollayerutils.h"
#include "qgslinesymbollayer.h"
#include "qgsfillsymbollayer.h"
#include "qgsmarkersymbollayer.h"
#include "qgsapplication.h"
#include "qgscolorschemeregistry.h"
#include "qgsvectorlayer.h"

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
  // layer is always considered as having elevation -- even if no z values are present or any
  // offset/extrusion etc is set, then we are still considering the features as sitting on the terrain
  // height
  return true;
}

QDomElement QgsVectorLayerElevationProperties::writeXml( QDomElement &parentElement, QDomDocument &document, const QgsReadWriteContext &context )
{
  QDomElement element = document.createElement( QStringLiteral( "elevation" ) );
  writeCommonProperties( element, document, context );

  element.setAttribute( QStringLiteral( "extrusionEnabled" ), mEnableExtrusion ? QStringLiteral( "1" ) : QStringLiteral( "0" ) );
  element.setAttribute( QStringLiteral( "extrusion" ), qgsDoubleToString( mExtrusionHeight ) );
  element.setAttribute( QStringLiteral( "customToleranceEnabled" ), mEnableCustomTolerance ? QStringLiteral( "1" ) : QStringLiteral( "0" ) );
  if ( mCustomTolerance != 0 )
    element.setAttribute( QStringLiteral( "customTolerance" ), qgsDoubleToString( mCustomTolerance ) );
  element.setAttribute( QStringLiteral( "clamping" ), qgsEnumValueToKey( mClamping ) );
  element.setAttribute( QStringLiteral( "binding" ), qgsEnumValueToKey( mBinding ) );
  element.setAttribute( QStringLiteral( "type" ), qgsEnumValueToKey( mType ) );
  element.setAttribute( QStringLiteral( "symbology" ), qgsEnumValueToKey( mSymbology ) );
  if ( !std::isnan( mElevationLimit ) )
    element.setAttribute( QStringLiteral( "elevationLimit" ), qgsDoubleToString( mElevationLimit ) );

  element.setAttribute( QStringLiteral( "respectLayerSymbol" ), mRespectLayerSymbology ? QStringLiteral( "1" ) : QStringLiteral( "0" ) );
  element.setAttribute( QStringLiteral( "showMarkerSymbolInSurfacePlots" ), mShowMarkerSymbolInSurfacePlots ? QStringLiteral( "1" ) : QStringLiteral( "0" ) );

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
  if ( elevationElement.isNull() )
    return false;

  readCommonProperties( elevationElement, context );

  mClamping = qgsEnumKeyToValue( elevationElement.attribute( QStringLiteral( "clamping" ) ), Qgis::AltitudeClamping::Terrain );
  mBinding = qgsEnumKeyToValue( elevationElement.attribute( QStringLiteral( "binding" ) ), Qgis::AltitudeBinding::Centroid );
  mType = qgsEnumKeyToValue( elevationElement.attribute( QStringLiteral( "type" ) ), Qgis::VectorProfileType::IndividualFeatures );
  mEnableExtrusion = elevationElement.attribute( QStringLiteral( "extrusionEnabled" ), QStringLiteral( "0" ) ).toInt();
  mExtrusionHeight = elevationElement.attribute( QStringLiteral( "extrusion" ), QStringLiteral( "0" ) ).toDouble();
  mEnableCustomTolerance = elevationElement.attribute( QStringLiteral( "customToleranceEnabled" ), QStringLiteral( "0" ) ).toInt();
  mCustomTolerance = elevationElement.attribute( QStringLiteral( "customTolerance" ), QStringLiteral( "0" ) ).toDouble();
  mSymbology = qgsEnumKeyToValue( elevationElement.attribute( QStringLiteral( "symbology" ) ), Qgis::ProfileSurfaceSymbology::Line );
  if ( elevationElement.hasAttribute( QStringLiteral( "elevationLimit" ) ) )
    mElevationLimit = elevationElement.attribute( QStringLiteral( "elevationLimit" ) ).toDouble();
  else
    mElevationLimit = std::numeric_limits< double >::quiet_NaN();

  mShowMarkerSymbolInSurfacePlots = elevationElement.attribute( QStringLiteral( "showMarkerSymbolInSurfacePlots" ), QStringLiteral( "0" ) ).toInt();

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

void QgsVectorLayerElevationProperties::setDefaultsFromLayer( QgsMapLayer *layer )
{
  QgsVectorLayer *vlayer = qobject_cast< QgsVectorLayer * >( layer );
  if ( !vlayer )
    return;

  mZOffset = 0;
  mZScale = 1;

  mEnableExtrusion = false;
  mExtrusionHeight = 0;

  // By default override default tolerance for Polygon and Line
  // to avoid unexpected behaviors.
  // For example, see: https://github.com/qgis/QGIS/issues/58016
  mEnableCustomTolerance = vlayer->geometryType() != Qgis::GeometryType::Point;
  mCustomTolerance = 0;

  mDataDefinedProperties.clear();

  mBinding = Qgis::AltitudeBinding::Centroid;

  if ( QgsWkbTypes::hasZ( vlayer->wkbType() ) )
  {
    mClamping = Qgis::AltitudeClamping::Absolute;
  }
  else
  {
    mClamping = Qgis::AltitudeClamping::Terrain;
  }
}

QgsVectorLayerElevationProperties *QgsVectorLayerElevationProperties::clone() const
{
  auto res = std::make_unique< QgsVectorLayerElevationProperties >( nullptr );
  res->setClamping( mClamping );
  res->setBinding( mBinding );
  res->setType( mType );
  res->setExtrusionEnabled( mEnableExtrusion );
  res->setExtrusionHeight( mExtrusionHeight );
  res->setCustomToleranceEnabled( mEnableCustomTolerance );
  res->setCustomTolerance( mCustomTolerance );
  res->setProfileLineSymbol( mProfileLineSymbol->clone() );
  res->setProfileFillSymbol( mProfileFillSymbol->clone() );
  res->setProfileMarkerSymbol( mProfileMarkerSymbol->clone() );
  res->setRespectLayerSymbology( mRespectLayerSymbology );
  res->setProfileSymbology( mSymbology );
  res->setElevationLimit( mElevationLimit );
  res->setShowMarkerSymbolInSurfacePlots( mShowMarkerSymbolInSurfacePlots );
  res->copyCommonProperties( this );
  return res.release();
}

QString QgsVectorLayerElevationProperties::htmlSummary() const
{
  QStringList properties;

  switch ( mClamping )
  {
    case Qgis::AltitudeClamping::Terrain:
      properties << tr( "Clamped to Terrain" );
      break;
    case Qgis::AltitudeClamping::Relative:
      properties << tr( "Relative to Terrain" );
      break;
    case Qgis::AltitudeClamping::Absolute:
      properties << tr( "Absolute" );
      break;
  }

  if ( mDataDefinedProperties.isActive( Property::ZOffset ) )
  {
    switch ( mDataDefinedProperties.property( Property::ZOffset ).propertyType() )
    {
      case Qgis::PropertyType::Invalid:
      case Qgis::PropertyType::Static:
        break;
      case Qgis::PropertyType::Field:
        properties << tr( "Offset: %1" ).arg( mDataDefinedProperties.property( Property::ZOffset ).field() );
        break;
      case Qgis::PropertyType::Expression:
        properties << tr( "Offset: %1" ).arg( mDataDefinedProperties.property( Property::ZOffset ).expressionString() );
        break;
    }
  }
  else
  {
    properties << tr( "Offset: %1" ).arg( mZOffset );
  }

  if ( mEnableExtrusion )
  {
    if ( mDataDefinedProperties.isActive( Property::ExtrusionHeight ) )
    {
      switch ( mDataDefinedProperties.property( Property::ExtrusionHeight ).propertyType() )
      {
        case Qgis::PropertyType::Invalid:
        case Qgis::PropertyType::Static:
          break;
        case Qgis::PropertyType::Field:
          properties << tr( "Extrusion: %1" ).arg( mDataDefinedProperties.property( Property::ExtrusionHeight ).field() );
          break;
        case Qgis::PropertyType::Expression:
          properties << tr( "Extrusion: %1" ).arg( mDataDefinedProperties.property( Property::ExtrusionHeight ).expressionString() );
          break;
      }
    }
    else
    {
      properties << tr( "Extrusion: %1" ).arg( mExtrusionHeight );
    }
  }

  if ( mEnableCustomTolerance )
  {
    properties << tr( "CustomTolerance: %1" ).arg( mCustomTolerance );
  }

  properties << tr( "Scale: %1" ).arg( mZScale );

  return QStringLiteral( "<li>%1</li>" ).arg( properties.join( QLatin1String( "</li><li>" ) ) );
}

bool QgsVectorLayerElevationProperties::isVisibleInZRange( const QgsDoubleRange &, QgsMapLayer * ) const
{
  // TODO -- test actual layer z range
  return true;
}

QgsDoubleRange QgsVectorLayerElevationProperties::calculateZRange( QgsMapLayer * ) const
{
  // TODO -- determine actual z range from layer statistics
  return QgsDoubleRange();
}

bool QgsVectorLayerElevationProperties::showByDefaultInElevationProfilePlots() const
{
  // show by default if the features aren't just directly clamped onto the terrain with
  // no other changes
  return !qgsDoubleNear( mZOffset, 0 )
         || !qgsDoubleNear( mZScale, 1 )
         || mEnableExtrusion
         || mClamping != Qgis::AltitudeClamping::Terrain;
}

void QgsVectorLayerElevationProperties::setClamping( Qgis::AltitudeClamping clamping )
{
  if ( mClamping == clamping )
    return;

  mClamping = clamping;
  emit changed();
  emit profileGenerationPropertyChanged();
}

void QgsVectorLayerElevationProperties::setBinding( Qgis::AltitudeBinding binding )
{
  if ( mBinding == binding )
    return;

  mBinding = binding;
  emit changed();
  emit profileGenerationPropertyChanged();
}

void QgsVectorLayerElevationProperties::setType( Qgis::VectorProfileType type )
{
  if ( type == mType )
    return;

  mType = type;
  emit changed();
  emit profileGenerationPropertyChanged();
}

void QgsVectorLayerElevationProperties::setExtrusionEnabled( bool enabled )
{
  if ( mEnableExtrusion == enabled )
    return;

  mEnableExtrusion = enabled;
  emit changed();
  emit profileGenerationPropertyChanged();
}

void QgsVectorLayerElevationProperties::setExtrusionHeight( double height )
{
  if ( mExtrusionHeight == height )
    return;

  mExtrusionHeight = height;
  emit changed();
  emit profileGenerationPropertyChanged();
}

void QgsVectorLayerElevationProperties::setCustomTolerance( double tolerance )
{
  if ( mCustomTolerance == tolerance )
    return;

  mCustomTolerance = tolerance;
  emit changed();
  emit profileGenerationPropertyChanged();
}

void QgsVectorLayerElevationProperties::setCustomToleranceEnabled( bool enabled )
{
  if ( mEnableCustomTolerance == enabled )
    return;

  mEnableCustomTolerance = enabled;
  emit changed();
  emit profileGenerationPropertyChanged();
}

void QgsVectorLayerElevationProperties::setRespectLayerSymbology( bool enabled )
{
  if ( mRespectLayerSymbology == enabled )
    return;

  mRespectLayerSymbology = enabled;
  emit changed();
  emit profileRenderingPropertyChanged();
}

QgsLineSymbol *QgsVectorLayerElevationProperties::profileLineSymbol() const
{
  return mProfileLineSymbol.get();
}

void QgsVectorLayerElevationProperties::setProfileLineSymbol( QgsLineSymbol *symbol )
{
  mProfileLineSymbol.reset( symbol );
  emit changed();
  emit profileRenderingPropertyChanged();
}

QgsFillSymbol *QgsVectorLayerElevationProperties::profileFillSymbol() const
{
  return mProfileFillSymbol.get();
}

void QgsVectorLayerElevationProperties::setProfileFillSymbol( QgsFillSymbol *symbol )
{
  mProfileFillSymbol.reset( symbol );
  emit changed();
  emit profileRenderingPropertyChanged();
}

QgsMarkerSymbol *QgsVectorLayerElevationProperties::profileMarkerSymbol() const
{
  return mProfileMarkerSymbol.get();
}

void QgsVectorLayerElevationProperties::setProfileMarkerSymbol( QgsMarkerSymbol *symbol )
{
  mProfileMarkerSymbol.reset( symbol );
  emit changed();
  emit profileRenderingPropertyChanged();
}

void QgsVectorLayerElevationProperties::setProfileSymbology( Qgis::ProfileSurfaceSymbology symbology )
{
  if ( mSymbology == symbology )
    return;

  mSymbology = symbology;
  emit changed();
  emit profileRenderingPropertyChanged();
}

double QgsVectorLayerElevationProperties::elevationLimit() const
{
  return mElevationLimit;
}

void QgsVectorLayerElevationProperties::setElevationLimit( double limit )
{
  if ( qgsDoubleNear( mElevationLimit, limit ) )
    return;

  mElevationLimit = limit;
  emit changed();
  emit profileRenderingPropertyChanged();
}

void QgsVectorLayerElevationProperties::setShowMarkerSymbolInSurfacePlots( bool show )
{
  if ( show == mShowMarkerSymbolInSurfacePlots )
    return;

  mShowMarkerSymbolInSurfacePlots = show;
  emit changed();
  emit profileRenderingPropertyChanged();
}

void QgsVectorLayerElevationProperties::setDefaultProfileLineSymbol( const QColor &color )
{
  auto profileLineLayer = std::make_unique< QgsSimpleLineSymbolLayer >( color, 0.6 );
  mProfileLineSymbol = std::make_unique< QgsLineSymbol>( QgsSymbolLayerList( { profileLineLayer.release() } ) );
}

void QgsVectorLayerElevationProperties::setDefaultProfileMarkerSymbol( const QColor &color )
{
  auto profileMarkerLayer = std::make_unique< QgsSimpleMarkerSymbolLayer >( Qgis::MarkerShape::Diamond, 3 );
  profileMarkerLayer->setColor( color );
  profileMarkerLayer->setStrokeWidth( 0.2 );
  profileMarkerLayer->setStrokeColor( color.darker( 140 ) );
  mProfileMarkerSymbol = std::make_unique< QgsMarkerSymbol>( QgsSymbolLayerList( { profileMarkerLayer.release() } ) );
}

void QgsVectorLayerElevationProperties::setDefaultProfileFillSymbol( const QColor &color )
{
  auto profileFillLayer = std::make_unique< QgsSimpleFillSymbolLayer >( color );
  profileFillLayer->setStrokeWidth( 0.2 );
  profileFillLayer->setStrokeColor( color.darker( 140 ) );
  mProfileFillSymbol = std::make_unique< QgsFillSymbol>( QgsSymbolLayerList( { profileFillLayer.release() } ) );
}

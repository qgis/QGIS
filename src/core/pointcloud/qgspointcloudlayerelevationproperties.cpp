/***************************************************************************
                         qgspointcloudlayerelevationproperties.cpp
                         ---------------
    begin                : November 2020
    copyright            : (C) 2020 by Nyall Dawson
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

#include "qgspointcloudlayerelevationproperties.h"

#include "qgsapplication.h"
#include "qgscolorschemeregistry.h"
#include "qgscolorutils.h"
#include "qgsfillsymbol.h"
#include "qgsfillsymbollayer.h"
#include "qgslinesymbol.h"
#include "qgslinesymbollayer.h"
#include "qgsmarkersymbol.h"
#include "qgsmarkersymbollayer.h"
#include "qgspointcloudlayer.h"
#include "qgssymbollayerutils.h"
#include "qgsvirtualpointcloudprovider.h"

#include "moc_qgspointcloudlayerelevationproperties.cpp"

QgsPointCloudLayerElevationProperties::QgsPointCloudLayerElevationProperties( QObject *parent )
  : QgsMapLayerElevationProperties( parent )
{
  mPointColor = QgsApplication::colorSchemeRegistry()->fetchRandomStyleColor();
  setDefaultProfileLineSymbol( mPointColor );
  setDefaultProfileFillSymbol( mPointColor );
  setDefaultProfileMarkerSymbol( mPointColor );

  if ( QgsPointCloudLayer *pcLayer = qobject_cast< QgsPointCloudLayer * >( parent ) )
  {
    connect( pcLayer, &QgsPointCloudLayer::rendererChanged, this, [this]
    {
      if ( mRespectLayerColors )
        emit profileGenerationPropertyChanged();
    } );
  }
}

QgsLineSymbol *QgsPointCloudLayerElevationProperties::profileLineSymbol() const
{
  return mProfileLineSymbol.get();
}

void QgsPointCloudLayerElevationProperties::setProfileLineSymbol( QgsLineSymbol *symbol )
{
  if ( !symbol )
    return;

  mProfileLineSymbol.reset( symbol );
  emit changed();
  emit profileRenderingPropertyChanged();
}

QgsFillSymbol *QgsPointCloudLayerElevationProperties::profileFillSymbol() const
{
  return mProfileFillSymbol.get();
}

void QgsPointCloudLayerElevationProperties::setProfileFillSymbol( QgsFillSymbol *symbol )
{
  if ( !symbol )
    return;

  mProfileFillSymbol.reset( symbol );
  emit changed();
  emit profileRenderingPropertyChanged();
}

QgsMarkerSymbol *QgsPointCloudLayerElevationProperties::profileMarkerSymbol() const
{
  return mProfileMarkerSymbol.get();
}

void QgsPointCloudLayerElevationProperties::setProfileMarkerSymbol( QgsMarkerSymbol *symbol )
{
  if ( !symbol )
    return;

  mProfileMarkerSymbol.reset( symbol );
  emit changed();
  emit profileRenderingPropertyChanged();
}

void QgsPointCloudLayerElevationProperties::setProfileSymbology( Qgis::ProfileSurfaceSymbology symbology )
{
  if ( mSymbology == symbology )
    return;

  mSymbology = symbology;
  emit changed();
  emit profileRenderingPropertyChanged();
}

double QgsPointCloudLayerElevationProperties::elevationLimit() const
{
  return mElevationLimit;
}

void QgsPointCloudLayerElevationProperties::setElevationLimit( double limit )
{
  if ( qgsDoubleNear( mElevationLimit, limit ) )
    return;

  mElevationLimit = limit;
  emit changed();
  emit profileRenderingPropertyChanged();
}

void QgsPointCloudLayerElevationProperties::setShowMarkerSymbolInSurfacePlots( bool show )
{
  if ( show == mShowMarkerSymbolInSurfacePlots )
    return;

  mShowMarkerSymbolInSurfacePlots = show;
  emit changed();
  emit profileRenderingPropertyChanged();
}

void QgsPointCloudLayerElevationProperties::setDefaultProfileLineSymbol( const QColor &color )
{
  auto profileLineLayer = std::make_unique< QgsSimpleLineSymbolLayer >( color, 0.6 );
  mProfileLineSymbol = std::make_unique< QgsLineSymbol>( QgsSymbolLayerList( { profileLineLayer.release() } ) );
}

void QgsPointCloudLayerElevationProperties::setDefaultProfileMarkerSymbol( const QColor &color )
{
  auto profileMarkerLayer = std::make_unique< QgsSimpleMarkerSymbolLayer >( Qgis::MarkerShape::Diamond, 3 );
  profileMarkerLayer->setColor( color );
  profileMarkerLayer->setStrokeWidth( 0.2 );
  profileMarkerLayer->setStrokeColor( color.darker( 140 ) );
  mProfileMarkerSymbol = std::make_unique< QgsMarkerSymbol>( QgsSymbolLayerList( { profileMarkerLayer.release() } ) );
}

void QgsPointCloudLayerElevationProperties::setDefaultProfileFillSymbol( const QColor &color )
{
  auto profileFillLayer = std::make_unique< QgsSimpleFillSymbolLayer >( color );
  profileFillLayer->setStrokeWidth( 0.2 );
  profileFillLayer->setStrokeColor( color.darker( 140 ) );
  mProfileFillSymbol = std::make_unique< QgsFillSymbol>( QgsSymbolLayerList( { profileFillLayer.release() } ) );
}


bool QgsPointCloudLayerElevationProperties::hasElevation() const
{
  return true;
}

QDomElement QgsPointCloudLayerElevationProperties::writeXml( QDomElement &parentElement, QDomDocument &document, const QgsReadWriteContext &context )
{
  QDomElement element = document.createElement( u"elevation"_s );
  writeCommonProperties( element, document, context );

  element.setAttribute( u"max_screen_error"_s, qgsDoubleToString( mMaximumScreenError ) );
  element.setAttribute( u"max_screen_error_unit"_s, QgsUnitTypes::encodeUnit( mMaximumScreenErrorUnit ) );
  element.setAttribute( u"point_size"_s, qgsDoubleToString( mPointSize ) );
  element.setAttribute( u"point_size_unit"_s, QgsUnitTypes::encodeUnit( mPointSizeUnit ) );
  element.setAttribute( u"point_symbol"_s, qgsEnumValueToKey( mPointSymbol ) );
  element.setAttribute( u"point_color"_s, QgsColorUtils::colorToString( mPointColor ) );
  element.setAttribute( u"respect_layer_colors"_s, mRespectLayerColors ? u"1"_s : u"0"_s );
  element.setAttribute( u"opacity_by_distance"_s, mApplyOpacityByDistanceEffect ? u"1"_s : u"0"_s );
  element.setAttribute( u"render_type"_s, qgsEnumValueToKey( mRenderType ) );
  if ( !std::isnan( mElevationLimit ) )
    element.setAttribute( u"elevationLimit"_s, qgsDoubleToString( mElevationLimit ) );
  element.setAttribute( u"showMarkerSymbolInSurfacePlots"_s, mShowMarkerSymbolInSurfacePlots ? u"1"_S : u"0"_s );

  QDomElement profileLineSymbolElement = document.createElement( u"profileLineSymbol"_s );
  profileLineSymbolElement.appendChild( QgsSymbolLayerUtils::saveSymbol( QString(), mProfileLineSymbol.get(), document, context ) );
  element.appendChild( profileLineSymbolElement );

  QDomElement profileFillSymbolElement = document.createElement( u"profileFillSymbol"_s );
  profileFillSymbolElement.appendChild( QgsSymbolLayerUtils::saveSymbol( QString(), mProfileFillSymbol.get(), document, context ) );
  element.appendChild( profileFillSymbolElement );

  QDomElement profileMarkerSymbolElement = document.createElement( u"profileMarkerSymbol"_s );
  profileMarkerSymbolElement.appendChild( QgsSymbolLayerUtils::saveSymbol( QString(), mProfileMarkerSymbol.get(), document, context ) );
  element.appendChild( profileMarkerSymbolElement );

  parentElement.appendChild( element );
  return element;
}

bool QgsPointCloudLayerElevationProperties::readXml( const QDomElement &element, const QgsReadWriteContext &context )
{
  const QDomElement elevationElement = element.firstChildElement( u"elevation"_s ).toElement();
  readCommonProperties( elevationElement, context );

  mMaximumScreenError = elevationElement.attribute( u"max_screen_error"_s, u"0.3"_s ).toDouble();
  bool ok = false;
  mMaximumScreenErrorUnit = QgsUnitTypes::decodeRenderUnit( elevationElement.attribute( u"max_screen_error_unit"_s ), &ok );
  if ( !ok )
    mMaximumScreenErrorUnit = Qgis::RenderUnit::Millimeters;
  mPointSize = elevationElement.attribute( u"point_size"_s, u"0.6"_s ).toDouble();
  mPointSizeUnit = QgsUnitTypes::decodeRenderUnit( elevationElement.attribute( u"point_size_unit"_s ), &ok );
  if ( !ok )
    mPointSizeUnit = Qgis::RenderUnit::Millimeters;
  mPointSymbol = qgsEnumKeyToValue( elevationElement.attribute( u"point_symbol"_s ), Qgis::PointCloudSymbol::Square );
  const QString colorString = elevationElement.attribute( u"point_color"_s );
  if ( !colorString.isEmpty() )
  {
    mPointColor = QgsColorUtils::colorFromString( elevationElement.attribute( u"point_color"_s ) );
  }
  else
  {
    mPointColor = QgsApplication::colorSchemeRegistry()->fetchRandomStyleColor();
  }

  mRespectLayerColors = elevationElement.attribute( u"respect_layer_colors"_s, u"1"_s ).toInt();
  mApplyOpacityByDistanceEffect = elevationElement.attribute( u"opacity_by_distance"_s ).toInt();
  mRenderType = qgsEnumKeyToValue( elevationElement.attribute( u"render_type"_s ), Qgis::PointCloudProfileType::IndividualPoints );
  if ( elevationElement.hasAttribute( u"elevationLimit"_s ) )
    mElevationLimit = elevationElement.attribute( u"elevationLimit"_s ).toDouble();
  else
    mElevationLimit = std::numeric_limits< double >::quiet_NaN();

  mShowMarkerSymbolInSurfacePlots = elevationElement.attribute( u"showMarkerSymbolInSurfacePlots"_s, u"0"_s ).toInt();

  const QColor color = QgsApplication::colorSchemeRegistry()->fetchRandomStyleColor();

  const QDomElement profileLineSymbolElement = elevationElement.firstChildElement( u"profileLineSymbol"_s ).firstChildElement( u"symbol"_s );
  mProfileLineSymbol = QgsSymbolLayerUtils::loadSymbol< QgsLineSymbol >( profileLineSymbolElement, context );
  if ( !mProfileLineSymbol )
    setDefaultProfileLineSymbol( color );

  const QDomElement profileFillSymbolElement = elevationElement.firstChildElement( u"profileFillSymbol"_s ).firstChildElement( u"symbol"_s );
  mProfileFillSymbol = QgsSymbolLayerUtils::loadSymbol< QgsFillSymbol >( profileFillSymbolElement, context );
  if ( !mProfileFillSymbol )
    setDefaultProfileFillSymbol( color );

  const QDomElement profileMarkerSymbolElement = elevationElement.firstChildElement( u"profileMarkerSymbol"_s ).firstChildElement( u"symbol"_s );
  mProfileMarkerSymbol = QgsSymbolLayerUtils::loadSymbol< QgsMarkerSymbol >( profileMarkerSymbolElement, context );
  if ( !mProfileMarkerSymbol )
    setDefaultProfileMarkerSymbol( color );

  return true;
}

QgsPointCloudLayerElevationProperties *QgsPointCloudLayerElevationProperties::clone() const
{
  auto res = std::make_unique< QgsPointCloudLayerElevationProperties >( nullptr );
  res->copyCommonProperties( this );

  res->mMaximumScreenError = mMaximumScreenError;
  res->mMaximumScreenErrorUnit = mMaximumScreenErrorUnit;
  res->mPointSize = mPointSize;
  res->mPointSizeUnit = mPointSizeUnit;
  res->mPointSymbol = mPointSymbol;
  res->mPointColor = mPointColor;
  res->mRespectLayerColors = mRespectLayerColors;
  res->mRenderType = mRenderType;
  res->mApplyOpacityByDistanceEffect = mApplyOpacityByDistanceEffect;

  res->setProfileLineSymbol( mProfileLineSymbol->clone() );
  res->setProfileFillSymbol( mProfileFillSymbol->clone() );
  res->setProfileMarkerSymbol( mProfileMarkerSymbol->clone() );

  return res.release();
}

QString QgsPointCloudLayerElevationProperties::htmlSummary() const
{
  QStringList properties;
  properties << tr( "Scale: %1" ).arg( mZScale );
  properties << tr( "Offset: %1" ).arg( mZOffset );
  return u"<ul><li>%1</li></ul>"_s.arg( properties.join( "</li><li>"_L1 ) );
}

bool QgsPointCloudLayerElevationProperties::isVisibleInZRange( const QgsDoubleRange &, QgsMapLayer * ) const
{
  // TODO -- test actual point cloud z range
  return true;
}

QgsDoubleRange QgsPointCloudLayerElevationProperties::calculateZRange( QgsMapLayer *layer ) const
{
  if ( QgsPointCloudLayer *pcLayer = qobject_cast< QgsPointCloudLayer * >( layer ) )
  {
    if ( pcLayer->dataProvider() )
    {
      double zMin = std::numeric_limits<double>::quiet_NaN();
      double zMax = std::numeric_limits<double>::quiet_NaN();
      const QgsPointCloudStatistics stats = pcLayer->statistics();
      if ( !stats.statisticsMap().isEmpty() )
      {
        // try to fetch z range from provider metadata
        zMin = stats.minimum( u"Z"_s );
        zMax = stats.maximum( u"Z"_s );
      }
      // try to fetch the elevation properties from virtual point cloud metadata
      else if ( QgsVirtualPointCloudProvider *virtualProvider = dynamic_cast< QgsVirtualPointCloudProvider * >( pcLayer->dataProvider() ) )
      {
        for ( QgsPointCloudSubIndex subIndex : virtualProvider->subIndexes() )
        {
          const QgsDoubleRange newRange = subIndex.zRange();
          if ( newRange.isInfinite() ) continue;
          zMin = std::isnan( zMin ) ? newRange.lower() : std::min( zMin, newRange.lower() );
          zMax = std::isnan( zMax ) ? newRange.upper() : std::max( zMax, newRange.upper() );
        }
      }

      if ( !std::isnan( zMin ) && !std::isnan( zMax ) )
      {
        return QgsDoubleRange( zMin * mZScale + mZOffset, zMax * mZScale + mZOffset );
      }
    }
  }

  return QgsDoubleRange();
}

QList<double> QgsPointCloudLayerElevationProperties::significantZValues( QgsMapLayer *layer ) const
{
  const QgsDoubleRange range = calculateZRange( layer );
  if ( !range.isInfinite() && range.lower() != range.upper() )
    return {range.lower(), range.upper() };
  else if ( !range.isInfinite() )
    return {range.lower() };
  else
    return {};
}

bool QgsPointCloudLayerElevationProperties::showByDefaultInElevationProfilePlots() const
{
  return true;
}

void QgsPointCloudLayerElevationProperties::setMaximumScreenError( double error )
{
  if ( qgsDoubleNear( error, mMaximumScreenError ) )
    return;

  mMaximumScreenError = error;
  emit changed();
  emit profileGenerationPropertyChanged();
}

void QgsPointCloudLayerElevationProperties::setMaximumScreenErrorUnit( Qgis::RenderUnit unit )
{
  if ( unit == mMaximumScreenErrorUnit )
    return;

  mMaximumScreenErrorUnit = unit;
  emit changed();
  emit profileGenerationPropertyChanged();
}

Qgis::PointCloudSymbol QgsPointCloudLayerElevationProperties::pointSymbol() const
{
  return mPointSymbol;
}

void QgsPointCloudLayerElevationProperties::setPointSymbol( Qgis::PointCloudSymbol symbol )
{
  if ( mPointSymbol == symbol )
    return;

  mPointSymbol = symbol;
  emit changed();
  emit profileRenderingPropertyChanged();
}

void QgsPointCloudLayerElevationProperties::setPointColor( const QColor &color )
{
  if ( color == mPointColor )
    return;

  mPointColor = color;
  emit changed();
  emit profileRenderingPropertyChanged();
}

void QgsPointCloudLayerElevationProperties::setApplyOpacityByDistanceEffect( bool apply )
{
  if ( apply == mApplyOpacityByDistanceEffect )
    return;

  mApplyOpacityByDistanceEffect = apply;
  emit changed();

  // turning ON opacity by distance requires a profile regeneration, turning it off does not.
  if ( mApplyOpacityByDistanceEffect )
    emit profileGenerationPropertyChanged();
  else
    emit profileRenderingPropertyChanged();
}

void QgsPointCloudLayerElevationProperties::setPointSize( double size )
{
  if ( qgsDoubleNear( size, mPointSize ) )
    return;

  mPointSize = size;
  emit changed();
  emit profileRenderingPropertyChanged();
}

void QgsPointCloudLayerElevationProperties::setPointSizeUnit( const Qgis::RenderUnit units )
{
  if ( mPointSizeUnit == units )
    return;

  mPointSizeUnit = units;
  emit changed();
  emit profileRenderingPropertyChanged();
}

void QgsPointCloudLayerElevationProperties::setRespectLayerColors( bool enabled )
{
  if ( mRespectLayerColors == enabled )
    return;

  mRespectLayerColors = enabled;
  emit changed();

  // turning ON respect layer colors requires a profile regeneration, turning it off does not.
  if ( mRespectLayerColors )
    emit profileGenerationPropertyChanged();
  else
    emit profileRenderingPropertyChanged();
}

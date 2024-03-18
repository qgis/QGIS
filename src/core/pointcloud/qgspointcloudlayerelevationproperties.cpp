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
#include "qgspointcloudlayer.h"
#include "qgsapplication.h"
#include "qgscolorschemeregistry.h"
#include "qgscolorutils.h"

QgsPointCloudLayerElevationProperties::QgsPointCloudLayerElevationProperties( QObject *parent )
  : QgsMapLayerElevationProperties( parent )
{
  mPointColor = QgsApplication::colorSchemeRegistry()->fetchRandomStyleColor();

  if ( QgsPointCloudLayer *pcLayer = qobject_cast< QgsPointCloudLayer * >( parent ) )
  {
    connect( pcLayer, &QgsPointCloudLayer::rendererChanged, this, [this]
    {
      if ( mRespectLayerColors )
        emit profileGenerationPropertyChanged();
    } );
  }
}

bool QgsPointCloudLayerElevationProperties::hasElevation() const
{
  return true;
}

QDomElement QgsPointCloudLayerElevationProperties::writeXml( QDomElement &parentElement, QDomDocument &document, const QgsReadWriteContext &context )
{
  QDomElement element = document.createElement( QStringLiteral( "elevation" ) );
  writeCommonProperties( element, document, context );

  element.setAttribute( QStringLiteral( "max_screen_error" ), qgsDoubleToString( mMaximumScreenError ) );
  element.setAttribute( QStringLiteral( "max_screen_error_unit" ), QgsUnitTypes::encodeUnit( mMaximumScreenErrorUnit ) );
  element.setAttribute( QStringLiteral( "point_size" ), qgsDoubleToString( mPointSize ) );
  element.setAttribute( QStringLiteral( "point_size_unit" ), QgsUnitTypes::encodeUnit( mPointSizeUnit ) );
  element.setAttribute( QStringLiteral( "point_symbol" ), qgsEnumValueToKey( mPointSymbol ) );
  element.setAttribute( QStringLiteral( "point_color" ), QgsColorUtils::colorToString( mPointColor ) );
  element.setAttribute( QStringLiteral( "respect_layer_colors" ), mRespectLayerColors ? QStringLiteral( "1" ) : QStringLiteral( "0" ) );
  element.setAttribute( QStringLiteral( "opacity_by_distance" ), mApplyOpacityByDistanceEffect ? QStringLiteral( "1" ) : QStringLiteral( "0" ) );

  parentElement.appendChild( element );
  return element;
}

bool QgsPointCloudLayerElevationProperties::readXml( const QDomElement &element, const QgsReadWriteContext &context )
{
  const QDomElement elevationElement = element.firstChildElement( QStringLiteral( "elevation" ) ).toElement();
  readCommonProperties( elevationElement, context );

  mMaximumScreenError = elevationElement.attribute( QStringLiteral( "max_screen_error" ), QStringLiteral( "0.3" ) ).toDouble();
  bool ok = false;
  mMaximumScreenErrorUnit = QgsUnitTypes::decodeRenderUnit( elevationElement.attribute( QStringLiteral( "max_screen_error_unit" ) ), &ok );
  if ( !ok )
    mMaximumScreenErrorUnit = Qgis::RenderUnit::Millimeters;
  mPointSize = elevationElement.attribute( QStringLiteral( "point_size" ), QStringLiteral( "0.6" ) ).toDouble();
  mPointSizeUnit = QgsUnitTypes::decodeRenderUnit( elevationElement.attribute( QStringLiteral( "point_size_unit" ) ), &ok );
  if ( !ok )
    mPointSizeUnit = Qgis::RenderUnit::Millimeters;
  mPointSymbol = qgsEnumKeyToValue( elevationElement.attribute( QStringLiteral( "point_symbol" ) ), Qgis::PointCloudSymbol::Square );
  const QString colorString = elevationElement.attribute( QStringLiteral( "point_color" ) );
  if ( !colorString.isEmpty() )
  {
    mPointColor = QgsColorUtils::colorFromString( elevationElement.attribute( QStringLiteral( "point_color" ) ) );
  }
  else
  {
    mPointColor = QgsApplication::colorSchemeRegistry()->fetchRandomStyleColor();
  }
  mRespectLayerColors = elevationElement.attribute( QStringLiteral( "respect_layer_colors" ), QStringLiteral( "1" ) ).toInt();
  mApplyOpacityByDistanceEffect = elevationElement.attribute( QStringLiteral( "opacity_by_distance" ) ).toInt();

  return true;
}

QgsPointCloudLayerElevationProperties *QgsPointCloudLayerElevationProperties::clone() const
{
  std::unique_ptr< QgsPointCloudLayerElevationProperties > res = std::make_unique< QgsPointCloudLayerElevationProperties >( nullptr );
  res->copyCommonProperties( this );

  res->mMaximumScreenError = mMaximumScreenError;
  res->mMaximumScreenErrorUnit = mMaximumScreenErrorUnit;
  res->mPointSize = mPointSize;
  res->mPointSizeUnit = mPointSizeUnit;
  res->mPointSymbol = mPointSymbol;
  res->mPointColor = mPointColor;
  res->mRespectLayerColors = mRespectLayerColors;
  res->mApplyOpacityByDistanceEffect = mApplyOpacityByDistanceEffect;

  return res.release();
}

QString QgsPointCloudLayerElevationProperties::htmlSummary() const
{
  QStringList properties;
  properties << tr( "Scale: %1" ).arg( mZScale );
  properties << tr( "Offset: %1" ).arg( mZOffset );
  return QStringLiteral( "<ul><li>%1</li></ul>" ).arg( properties.join( QLatin1String( "</li><li>" ) ) );
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
      const QgsPointCloudStatistics stats = pcLayer->statistics();

      // try to fetch z range from provider metadata
      const double zMin = stats.minimum( QStringLiteral( "Z" ) );
      const double zMax = stats.maximum( QStringLiteral( "Z" ) );
      if ( !std::isnan( zMin ) && !std::isnan( zMax ) )
      {
        return QgsDoubleRange( zMin * mZScale + mZOffset, zMax * mZScale + mZOffset );
      }
    }
  }

  return QgsDoubleRange();
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

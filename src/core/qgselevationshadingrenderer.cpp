/***************************************************************************
  qgselevationshadingrenderer.cpp - QgsElevationShadingRenderer

 ---------------------
 begin                : 4.12.2022
 copyright            : (C) 2022 by Vincent Cloarec
 email                : vcloarec at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "qgselevationshadingrenderer.h"

#include <QImage>

#include "qgselevationmap.h"
#include "qgsrendercontext.h"
#include "qgsunittypes.h"

QgsElevationShadingRenderer::QgsElevationShadingRenderer()
{
}

void QgsElevationShadingRenderer::renderShading( const QgsElevationMap &elevation, QImage &image,  const QgsRenderContext &context ) const
{
  if ( elevation.rawElevationImage().size() != image.size() )
    return;

  if ( mRenderEdl )
    renderEdl( elevation, image, context );

  if ( mRenderHillshading )
    renderHillshading( elevation, image, context );
}

void QgsElevationShadingRenderer::setActive( bool active )
{
  mIsActive = active;
}

bool QgsElevationShadingRenderer::isActive() const
{
  return mIsActive;
}

void QgsElevationShadingRenderer::setActiveEyeDomeLighting( bool active )
{
  mRenderEdl = active;
}

bool QgsElevationShadingRenderer::isActiveEyeDomeLighting() const
{
  return mRenderEdl;
}

void QgsElevationShadingRenderer::setActiveHillshading( bool active )
{
  mRenderHillshading = active;
}

bool QgsElevationShadingRenderer::isActiveHillshading() const
{
  return mRenderHillshading;
}

double QgsElevationShadingRenderer::hillshadingZFactor() const
{
  return mHillshadingZFactor;
}

void QgsElevationShadingRenderer::setHillshadingZFactor( double zFactor )
{
  mHillshadingZFactor = zFactor;
}

bool QgsElevationShadingRenderer::isHillshadingMultidirectional() const
{
  return mHillshadingMultiDir;
}

void QgsElevationShadingRenderer::setHillshadingMultidirectional( bool multiDirectional )
{
  mHillshadingMultiDir = multiDirectional;
}

double QgsElevationShadingRenderer::lightAltitude() const
{
  return mLightAltitude;
}

void QgsElevationShadingRenderer::setLightAltitude( double lightAltitude )
{
  mLightAltitude = lightAltitude;
}

double QgsElevationShadingRenderer::lightAzimuth() const
{
  return mLightAzimuth;
}

void QgsElevationShadingRenderer::setLightAzimuth( double lightAzimuth )
{
  mLightAzimuth = lightAzimuth;
}

void QgsElevationShadingRenderer::writeXml( QDomElement &elem, const QgsReadWriteContext & ) const
{
  elem.setAttribute( QStringLiteral( "is-active" ), mIsActive ? 1 : 0 );

  elem.setAttribute( QStringLiteral( "combined-method" ), static_cast<int>( mCombinedElevationMethod ) );

  elem.setAttribute( QStringLiteral( "edl-is-active" ), mRenderEdl ? 1 : 0 );
  elem.setAttribute( QStringLiteral( "edl-strength" ),  qgsDoubleToString( mEyeDomeLightingStrength ) );
  elem.setAttribute( QStringLiteral( "edl-distance" ),  qgsDoubleToString( mEyeDomeLightingDistance ) );
  elem.setAttribute( QStringLiteral( "edl-distance-unit" ), static_cast<int>( mEyeDomeLightingDistanceUnit ) );

  elem.setAttribute( QStringLiteral( "hillshading-is-active" ), mRenderHillshading ? 1 : 0 );
  elem.setAttribute( QStringLiteral( "hillshading-z-factor" ), qgsDoubleToString( mHillshadingZFactor ) );
  elem.setAttribute( QStringLiteral( "hillshading-is-multidirectional" ), mHillshadingMultiDir ? 1 : 0 );

  elem.setAttribute( QStringLiteral( "light-altitude" ),  qgsDoubleToString( mLightAltitude ) );
  elem.setAttribute( QStringLiteral( "light-azimuth" ),  qgsDoubleToString( mLightAzimuth ) );
}

void QgsElevationShadingRenderer::readXml( const QDomElement &element, const QgsReadWriteContext & )
{
  if ( element.hasAttribute( QStringLiteral( "is-active" ) ) )
    mIsActive = element.attribute( QStringLiteral( "is-active" ) ).toInt() == 1;

  if ( element.hasAttribute( QStringLiteral( "combined-method" ) ) )
    mCombinedElevationMethod = static_cast<Qgis::ElevationMapCombineMethod>( element.attribute( QStringLiteral( "combined-method" ) ).toInt() );

  if ( element.hasAttribute( QStringLiteral( "edl-is-active" ) ) )
    mRenderEdl = element.attribute( QStringLiteral( "edl-is-active" ) ).toInt() == 1;

  if ( element.hasAttribute( QStringLiteral( "edl-strength" ) ) )
    mEyeDomeLightingStrength = element.attribute( QStringLiteral( "edl-strength" ) ).toDouble();

  if ( element.hasAttribute( QStringLiteral( "edl-distance" ) ) )
    mEyeDomeLightingDistance = element.attribute( QStringLiteral( "edl-distance" ) ).toDouble();

  if ( element.hasAttribute( QStringLiteral( "edl-distance-unit" ) ) )
    mEyeDomeLightingDistanceUnit = static_cast<Qgis::RenderUnit>( element.attribute( QStringLiteral( "edl-distance-unit" ) ).toInt() );

  if ( element.hasAttribute( QStringLiteral( "hillshading-is-active" ) ) )
    mRenderHillshading = element.attribute( QStringLiteral( "hillshading-is-active" ) ).toInt() == 1;

  if ( element.hasAttribute( QStringLiteral( "hillshading-z-factor" ) ) )
    mHillshadingZFactor = element.attribute( QStringLiteral( "hillshading-z-factor" ) ).toDouble();

  if ( element.hasAttribute( QStringLiteral( "hillshading-is-multidirectional" ) ) )
    mHillshadingMultiDir = element.attribute( QStringLiteral( "hillshading-is-multidirectional" ) ).toInt() == 1;

  if ( element.hasAttribute( QStringLiteral( "light-altitude" ) ) )
    mLightAltitude = element.attribute( QStringLiteral( "light-altitude" ) ).toDouble();

  if ( element.hasAttribute( QStringLiteral( "light-azimuth" ) ) )
    mLightAzimuth = element.attribute( QStringLiteral( "light-azimuth" ) ).toDouble();
}

Qgis::ElevationMapCombineMethod QgsElevationShadingRenderer::combinedElevationMethod() const
{
  return mCombinedElevationMethod;
}

void QgsElevationShadingRenderer::setCombinedElevationMethod( Qgis::ElevationMapCombineMethod newCombinedElevationMethod )
{
  mCombinedElevationMethod = newCombinedElevationMethod;
}

Qgis::RenderUnit QgsElevationShadingRenderer::eyeDomeLightingDistanceUnit() const
{
  return mEyeDomeLightingDistanceUnit;
}

void QgsElevationShadingRenderer::setEyeDomeLightingDistanceUnit( Qgis::RenderUnit newEyeDomeLightingDistanceUnit )
{
  mEyeDomeLightingDistanceUnit = newEyeDomeLightingDistanceUnit;
}

double QgsElevationShadingRenderer::eyeDomeLightingDistance() const
{
  return mEyeDomeLightingDistance;
}

void QgsElevationShadingRenderer::setEyeDomeLightingDistance( double distance )
{
  mEyeDomeLightingDistance = distance;
}

double QgsElevationShadingRenderer::eyeDomeLightingStrength() const
{
  return mEyeDomeLightingStrength;
}

void QgsElevationShadingRenderer::setEyeDomeLightingStrength( double strength )
{
  mEyeDomeLightingStrength = strength;
}

void QgsElevationShadingRenderer::renderEdl( const QgsElevationMap &elevation, QImage &image, const QgsRenderContext &context ) const
{
  double strength = mEyeDomeLightingStrength;
  double distanceDouble = context.convertToPainterUnits( mEyeDomeLightingDistance, mEyeDomeLightingDistanceUnit );
  int distance = static_cast<int>( std::round( distanceDouble ) );

  elevation.applyEyeDomeLighting( image, distance, static_cast<float>( strength ), static_cast<float>( context.rendererScale() ) );
}

void QgsElevationShadingRenderer::renderHillshading( const QgsElevationMap &elevation, QImage &image, const QgsRenderContext &context ) const
{
  double pixelSize = context.mapToPixel().mapUnitsPerPixel();

  // We suppose that the elevation are in meter, so we need to use meter for pixel size if possible
  Qgis::DistanceUnit destinationUnit = context.distanceArea().lengthUnits();
  double effPixelSize;
  if ( destinationUnit != Qgis::DistanceUnit::Unknown )
    effPixelSize = QgsUnitTypes::fromUnitToUnitFactor( destinationUnit, Qgis::DistanceUnit::Meters ) * pixelSize;
  else
    effPixelSize = pixelSize;

  elevation.applyHillshading( image, mHillshadingMultiDir, mLightAltitude, mLightAzimuth, mHillshadingZFactor, effPixelSize, effPixelSize );
}

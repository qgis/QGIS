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

#include "qgselevationmap.h"
#include "qgsrendercontext.h"
#include "qgsunittypes.h"

#include <QImage>

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
  elem.setAttribute( u"is-active"_s, mIsActive ? 1 : 0 );

  elem.setAttribute( u"combined-method"_s, static_cast<int>( mCombinedElevationMethod ) );

  elem.setAttribute( u"edl-is-active"_s, mRenderEdl ? 1 : 0 );
  elem.setAttribute( u"edl-strength"_s,  qgsDoubleToString( mEyeDomeLightingStrength ) );
  elem.setAttribute( u"edl-distance"_s,  qgsDoubleToString( mEyeDomeLightingDistance ) );
  elem.setAttribute( u"edl-distance-unit"_s, static_cast<int>( mEyeDomeLightingDistanceUnit ) );

  elem.setAttribute( u"hillshading-is-active"_s, mRenderHillshading ? 1 : 0 );
  elem.setAttribute( u"hillshading-z-factor"_s, qgsDoubleToString( mHillshadingZFactor ) );
  elem.setAttribute( u"hillshading-is-multidirectional"_s, mHillshadingMultiDir ? 1 : 0 );

  elem.setAttribute( u"light-altitude"_s,  qgsDoubleToString( mLightAltitude ) );
  elem.setAttribute( u"light-azimuth"_s,  qgsDoubleToString( mLightAzimuth ) );
}

void QgsElevationShadingRenderer::readXml( const QDomElement &element, const QgsReadWriteContext & )
{
  if ( element.hasAttribute( u"is-active"_s ) )
    mIsActive = element.attribute( u"is-active"_s ).toInt() == 1;

  if ( element.hasAttribute( u"combined-method"_s ) )
    mCombinedElevationMethod = static_cast<Qgis::ElevationMapCombineMethod>( element.attribute( u"combined-method"_s ).toInt() );

  if ( element.hasAttribute( u"edl-is-active"_s ) )
    mRenderEdl = element.attribute( u"edl-is-active"_s ).toInt() == 1;

  if ( element.hasAttribute( u"edl-strength"_s ) )
    mEyeDomeLightingStrength = element.attribute( u"edl-strength"_s ).toDouble();

  if ( element.hasAttribute( u"edl-distance"_s ) )
    mEyeDomeLightingDistance = element.attribute( u"edl-distance"_s ).toDouble();

  if ( element.hasAttribute( u"edl-distance-unit"_s ) )
    mEyeDomeLightingDistanceUnit = static_cast<Qgis::RenderUnit>( element.attribute( u"edl-distance-unit"_s ).toInt() );

  if ( element.hasAttribute( u"hillshading-is-active"_s ) )
    mRenderHillshading = element.attribute( u"hillshading-is-active"_s ).toInt() == 1;

  if ( element.hasAttribute( u"hillshading-z-factor"_s ) )
    mHillshadingZFactor = element.attribute( u"hillshading-z-factor"_s ).toDouble();

  if ( element.hasAttribute( u"hillshading-is-multidirectional"_s ) )
    mHillshadingMultiDir = element.attribute( u"hillshading-is-multidirectional"_s ).toInt() == 1;

  if ( element.hasAttribute( u"light-altitude"_s ) )
    mLightAltitude = element.attribute( u"light-altitude"_s ).toDouble();

  if ( element.hasAttribute( u"light-azimuth"_s ) )
    mLightAzimuth = element.attribute( u"light-azimuth"_s ).toDouble();
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

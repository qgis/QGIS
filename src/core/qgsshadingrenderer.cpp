/***************************************************************************
  qgsshadingrenderer.cpp - QgsShadingRenderer

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
#include "qgsshadingrenderer.h"

#include <QImage>

#include "qgselevationmap.h"
#include "qgsrendercontext.h"

QgsShadingRenderer::QgsShadingRenderer()
{
  mRenderHillShading = false;
  mRenderEdl = true;
}

void QgsShadingRenderer::renderShading( const QgsElevationMap &elevation, QImage &image,  const QgsRenderContext &context ) const
{
  if ( elevation.rawElevationImage().size() != image.size() )
    return;

  if ( mRenderEdl )
    renderEdl( elevation, image, context );

  if ( mRenderHillShading )
    renderHillShading( elevation, image, context );
}

void QgsShadingRenderer::setActive( bool active )
{
  mIsActive = active;
}

bool QgsShadingRenderer::isActive() const
{
  return mIsActive;
}

void QgsShadingRenderer::setActiveEyeDomeLighting( bool active )
{
  mRenderEdl = active;
}

void QgsShadingRenderer::setActiveHillShading( bool active )
{
  mRenderHillShading = active;
}

double QgsShadingRenderer::hillShadingZFactor() const
{
  return mHillShadingZFactor;
}

void QgsShadingRenderer::setHillShadingZFactor( double zFactor )
{
  mHillShadingZFactor = zFactor;
}

bool QgsShadingRenderer::isHillShadingMultidirectional() const
{
  return mHillShadingMultiDir;
}

void QgsShadingRenderer::setHillShadingMultidirectional( bool multiDirectional )
{
  mHillShadingMultiDir = multiDirectional;
}

double QgsShadingRenderer::lightAltitude() const
{
  return mLightAltitude;
}

void QgsShadingRenderer::setLightAltitude( double lightAltitude )
{
  mLightAltitude = lightAltitude;
}

double QgsShadingRenderer::lightAzimuth() const
{
  return mLightAzimuth;
}

void QgsShadingRenderer::setLightAzimuth( double lightAzimuth )
{
  mLightAzimuth = lightAzimuth;
}

void QgsShadingRenderer::writeXml( QDomElement &elem, QDomDocument &doc, const QgsReadWriteContext & ) const
{
  elem.setAttribute( QStringLiteral( "is-active" ), mIsActive ? 1 : 0 );
}

void QgsShadingRenderer::readXml( const QDomElement &element, const QgsReadWriteContext & )
{
  mIsActive = element.attribute( QStringLiteral( "is-active" ) ).toInt() == 1;
}

const QgsUnitTypes::RenderUnit &QgsShadingRenderer::eyeDomeLightingDistanceUnit() const
{
  return mEyeDomeLightingDistanceUnit;
}

void QgsShadingRenderer::setEyeDomeLightingDistanceUnit( const QgsUnitTypes::RenderUnit &newEyeDomeLightingDistanceUnit )
{
  mEyeDomeLightingDistanceUnit = newEyeDomeLightingDistanceUnit;
}

double QgsShadingRenderer::eyeDomeLightingDistance() const
{
  return mEyeDomeLightingDistance;
}

void QgsShadingRenderer::setEyeDomeLightingDistance( double distance )
{
  mEyeDomeLightingDistance = distance;
}

double QgsShadingRenderer::eyeDomeLightingStrength() const
{
  return mEyeDomeLightingStrength;
}

void QgsShadingRenderer::setEyeDomeLightingStrength( double strength )
{
  mEyeDomeLightingStrength = strength;
}

void QgsShadingRenderer::renderEdl( const QgsElevationMap &elevation, QImage &image, const QgsRenderContext &context ) const
{
  double strength = mEyeDomeLightingStrength;
  double distanceDouble = context.convertToPainterUnits( mEyeDomeLightingDistance, mEyeDomeLightingDistanceUnit );
  int distance = static_cast<int>( std::round( distanceDouble ) );

  elevation.applyEyeDomeLighting( image, distance, strength, context.rendererScale() );
}

void QgsShadingRenderer::renderHillShading( const QgsElevationMap &elevation, QImage &image, const QgsRenderContext &context ) const
{
  double pixelSize = context.mapToPixel().mapUnitsPerPixel();
  elevation.applyHillShading( image, mHillShadingMultiDir, mLightAltitude, mLightAzimuth, mHillShadingZFactor, pixelSize, pixelSize );
}

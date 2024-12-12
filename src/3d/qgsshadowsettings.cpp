/***************************************************************************
  qgsshadowsettings.cpp
  --------------------------------------
  Date                 : September 2020
  Copyright            : (C) 2020 by Belgacem Nedjima
  Email                : gb uderscore nedjima at esi dot dz
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsshadowsettings.h"

#include <QDomDocument>

#include "qgsreadwritecontext.h"
#include "qgssymbollayerutils.h"

QgsShadowSettings::QgsShadowSettings( const QgsShadowSettings &other )
  : mRenderShadows( other.mRenderShadows )
  , mSelectedDirectionalLight( other.mSelectedDirectionalLight )
  , mMaximumShadowRenderingDistance( other.mMaximumShadowRenderingDistance )
  , mShadowBias( other.mShadowBias )
  , mShadowMapResolution( other.mShadowMapResolution )
{
}

QgsShadowSettings &QgsShadowSettings::operator=( QgsShadowSettings const &rhs )
{
  this->mRenderShadows = rhs.mRenderShadows;
  this->mSelectedDirectionalLight = rhs.mSelectedDirectionalLight;
  this->mMaximumShadowRenderingDistance = rhs.mMaximumShadowRenderingDistance;
  this->mShadowBias = rhs.mShadowBias;
  this->mShadowMapResolution = rhs.mShadowMapResolution;
  return *this;
}

void QgsShadowSettings::readXml( const QDomElement &element, const QgsReadWriteContext &context )
{
  Q_UNUSED( context );
  mRenderShadows = element.attribute( QStringLiteral( "shadow-rendering-enabled" ), QStringLiteral( "0" ) ).toInt();
  mSelectedDirectionalLight = element.attribute( QStringLiteral( "selected-directional-light" ), QStringLiteral( "-1" ) ).toInt();
  mMaximumShadowRenderingDistance = element.attribute( QStringLiteral( "max-shadow-rendering-distance" ), QStringLiteral( "1500" ) ).toInt();
  mShadowBias = element.attribute( QStringLiteral( "shadow-bias" ), QStringLiteral( "0.00001" ) ).toFloat();
  mShadowMapResolution = element.attribute( QStringLiteral( "shadow-map-resolution" ), QStringLiteral( "2048" ) ).toInt();
}

void QgsShadowSettings::writeXml( QDomElement &element, const QgsReadWriteContext &context ) const
{
  Q_UNUSED( context );
  element.setAttribute( QStringLiteral( "shadow-rendering-enabled" ), mRenderShadows );
  element.setAttribute( QStringLiteral( "selected-directional-light" ), mSelectedDirectionalLight );
  element.setAttribute( QStringLiteral( "max-shadow-rendering-distance" ), mMaximumShadowRenderingDistance );
  element.setAttribute( QStringLiteral( "shadow-bias" ), mShadowBias );
  element.setAttribute( QStringLiteral( "shadow-map-resolution" ), mShadowMapResolution );
}

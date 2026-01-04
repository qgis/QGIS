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

#include "qgsreadwritecontext.h"
#include "qgssymbollayerutils.h"

#include <QDomDocument>

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
  if ( &rhs == this )
    return *this;

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
  mRenderShadows = element.attribute( u"shadow-rendering-enabled"_s, u"0"_s ).toInt();
  mSelectedDirectionalLight = element.attribute( u"selected-directional-light"_s, u"-1"_s ).toInt();
  mMaximumShadowRenderingDistance = element.attribute( u"max-shadow-rendering-distance"_s, u"1500"_s ).toInt();
  mShadowBias = element.attribute( u"shadow-bias"_s, u"0.00001"_s ).toFloat();
  mShadowMapResolution = element.attribute( u"shadow-map-resolution"_s, u"2048"_s ).toInt();
}

void QgsShadowSettings::writeXml( QDomElement &element, const QgsReadWriteContext &context ) const
{
  Q_UNUSED( context );
  element.setAttribute( u"shadow-rendering-enabled"_s, mRenderShadows );
  element.setAttribute( u"selected-directional-light"_s, mSelectedDirectionalLight );
  element.setAttribute( u"max-shadow-rendering-distance"_s, mMaximumShadowRenderingDistance );
  element.setAttribute( u"shadow-bias"_s, mShadowBias );
  element.setAttribute( u"shadow-map-resolution"_s, mShadowMapResolution );
}
